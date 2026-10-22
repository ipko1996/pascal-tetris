/*
 * Tetris - C port of Tetris.pas (Free Pascal, crt/dos units).
 *
 * The port mirrors the Pascal program procedure by procedure, including its
 * quirks: byte/smallint wrap-around, the shared global loop counters, the
 * #0+scancode encoding of extended keys, the record file format and the
 * text-file read semantics of the FPC runtime.
 *
 * A small crt emulation layer is provided on top of an ANSI terminal: an
 * 80x25 virtual screen with the 16 VGA colours, the CP852 character set
 * (the code page the original was written in), gotoxy/textcolor/clrscr,
 * keypressed/readkey, delay and gettime.
 *
 * Build: gcc -O2 -o tetris tetris.c
 */

#define _DEFAULT_SOURCE
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

typedef uint8_t byte;
typedef uint16_t word;

/* Pascal "for v := a to b": bounds are evaluated once, the counter is
   re-read from the (global) variable on every step and keeps the last
   value after the loop, exactly like the FPC generated code. */
#define FOR_TO(v, a, b)                                                    \
    for (int lo_ = (a), hi_ = (b), go_ = (lo_ <= hi_) ? ((v) = lo_, 1) : 0; \
         go_; go_ = ((v) < hi_) ? ((v)++, 1) : 0)
#define FOR_DOWNTO(v, a, b)                                                \
    for (int lo_ = (a), hi_ = (b), go_ = (lo_ >= hi_) ? ((v) = lo_, 1) : 0; \
         go_; go_ = ((v) > hi_) ? ((v)--, 1) : 0)

/* ------------------------------------------------------------------ */
/* CP852                                                              */
/* ------------------------------------------------------------------ */

static const uint16_t cp852_hi[128] = {
    0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x016f, 0x0107, 0x00e7,
    0x0142, 0x00eb, 0x0150, 0x0151, 0x00ee, 0x0179, 0x00c4, 0x0106,
    0x00c9, 0x0139, 0x013a, 0x00f4, 0x00f6, 0x013d, 0x013e, 0x015a,
    0x015b, 0x00d6, 0x00dc, 0x0164, 0x0165, 0x0141, 0x00d7, 0x010d,
    0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x0104, 0x0105, 0x017d, 0x017e,
    0x0118, 0x0119, 0x00ac, 0x017a, 0x010c, 0x015f, 0x00ab, 0x00bb,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00c1, 0x00c2, 0x011a,
    0x015e, 0x2563, 0x2551, 0x2557, 0x255d, 0x017b, 0x017c, 0x2510,
    0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x0102, 0x0103,
    0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x00a4,
    0x0111, 0x0110, 0x010e, 0x00cb, 0x010f, 0x0147, 0x00cd, 0x00ce,
    0x011b, 0x2518, 0x250c, 0x2588, 0x2584, 0x0162, 0x016e, 0x2580,
    0x00d3, 0x00df, 0x00d4, 0x0143, 0x0144, 0x0148, 0x0160, 0x0161,
    0x0154, 0x00da, 0x0155, 0x0170, 0x00fd, 0x00dd, 0x0163, 0x00b4,
    0x00ad, 0x02dd, 0x02db, 0x02c7, 0x02d8, 0x00a7, 0x00f7, 0x00b8,
    0x00b0, 0x00a8, 0x02d9, 0x0171, 0x0158, 0x0159, 0x25a0, 0x00a0,
};

/* Glyphs the text-mode font shows for the control characters. */
static const uint16_t cp_ctrl[32] = {
    0x0020, 0x263a, 0x263b, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
    0x25d8, 0x25cb, 0x25d9, 0x2642, 0x2640, 0x266a, 0x266b, 0x263c,
    0x25ba, 0x25c4, 0x2195, 0x203c, 0x00b6, 0x00a7, 0x25ac, 0x21a8,
    0x2191, 0x2193, 0x2192, 0x2190, 0x221f, 0x2194, 0x25b2, 0x25bc,
};

static uint16_t glyph(byte c)
{
    if (c < 32)
        return cp_ctrl[c];
    if (c == 127)
        return 0x2302;
    if (c < 128)
        return c;
    if (c == 0xf0)
        return '-'; /* soft hyphen: zero width in many terminals */
    if (c == 0xff)
        return ' '; /* no-break space */
    return cp852_hi[c - 128];
}

static int to_cp852(uint32_t u)
{
    if (u < 128)
        return (int)u;
    for (int n = 0; n < 128; n++)
        if (cp852_hi[n] == u)
            return n + 128;
    return -1;
}

/* Decode the next UTF-8 code point of a literal into a CP852 byte. */
static byte lit_next(const char **sp)
{
    const unsigned char *s = (const unsigned char *)*sp;
    uint32_t u;
    int n;
    if (s[0] < 0x80) {
        u = s[0];
        n = 1;
    } else if ((s[0] & 0xe0) == 0xc0) {
        u = ((s[0] & 0x1fu) << 6) | (s[1] & 0x3fu);
        n = 2;
    } else {
        u = ((s[0] & 0x0fu) << 12) | ((s[1] & 0x3fu) << 6) | (s[2] & 0x3fu);
        n = 3;
    }
    *sp += n;
    int c = to_cp852(u);
    return c < 0 ? '?' : (byte)c;
}

/* Pascal ShortString: s[0] is the length byte. */
typedef struct {
    byte s[256];
} pstr;

static pstr P(const char *lit)
{
    pstr r;
    memset(&r, 0, sizeof r);
    while (*lit)
        r.s[++r.s[0]] = lit_next(&lit);
    return r;
}

/* String assignment only copies the length byte and the characters,
   the rest of the 256 byte buffer keeps its old contents. */
static void pcopy(pstr *dst, const pstr *src) { memcpy(dst->s, src->s, (size_t)src->s[0] + 1); }

static void pset(pstr *dst, const char *lit)
{
    pstr t = P(lit);
    pcopy(dst, &t);
}

/* ------------------------------------------------------------------ */
/* Terminal / crt emulation                                           */
/* ------------------------------------------------------------------ */

#define SCR_W 80
#define SCR_H 25

static const byte vga[16][3] = {
    {0, 0, 0},       {0, 0, 170},     {0, 170, 0},     {0, 170, 170},
    {170, 0, 0},     {170, 0, 170},   {170, 85, 0},    {170, 170, 170},
    {85, 85, 85},    {85, 85, 255},   {85, 255, 85},   {85, 255, 255},
    {255, 85, 85},   {255, 85, 255},  {255, 255, 85},  {255, 255, 255},
};

static byte scr_ch[SCR_H][SCR_W];
static byte scr_at[SCR_H][SCR_W];
static int curx = 1, cury = 1;
static byte textattr = 7;

static char obuf[1 << 16];
static size_t olen;
static int tx = -1, ty = -1; /* terminal cursor, -1 = unknown */
static int tattr = -1;       /* attribute last sent to the terminal */

static struct termios orig_tio;
static int tio_saved;
static volatile sig_atomic_t resized;

static void out_flush_raw(void)
{
    size_t off = 0;
    while (off < olen) {
        ssize_t n = write(STDOUT_FILENO, obuf + off, olen - off);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        off += (size_t)n;
    }
    olen = 0;
}

static void out(const char *s, size_t n)
{
    if (olen + n > sizeof obuf)
        out_flush_raw();
    memcpy(obuf + olen, s, n);
    olen += n;
}

static void outs(const char *s) { out(s, strlen(s)); }

static void out_attr(byte a)
{
    if (tattr == a)
        return;
    char b[64];
    const byte *f = vga[a & 15], *g = vga[(a >> 4) & 7];
    int n = snprintf(b, sizeof b, "\x1b[0;38;2;%d;%d;%d;48;2;%d;%d;%dm", f[0], f[1],
                     f[2], g[0], g[1], g[2]);
    out(b, (size_t)n);
    tattr = a;
}

static void out_goto(int x, int y)
{
    if (tx == x && ty == y)
        return;
    char b[32];
    int n = snprintf(b, sizeof b, "\x1b[%d;%dH", y, x);
    out(b, (size_t)n);
    tx = x;
    ty = y;
}

static void out_cell(int x, int y)
{
    out_goto(x, y);
    out_attr(scr_at[y - 1][x - 1]);
    uint16_t u = glyph(scr_ch[y - 1][x - 1]);
    char b[4];
    int n;
    if (u < 0x80) {
        b[0] = (char)u;
        n = 1;
    } else if (u < 0x800) {
        b[0] = (char)(0xc0 | (u >> 6));
        b[1] = (char)(0x80 | (u & 0x3f));
        n = 2;
    } else {
        b[0] = (char)(0xe0 | (u >> 12));
        b[1] = (char)(0x80 | ((u >> 6) & 0x3f));
        b[2] = (char)(0x80 | (u & 0x3f));
        n = 3;
    }
    out(b, (size_t)n);
    tx = (x < SCR_W) ? x + 1 : -1;
}

static void redraw_all(void)
{
    tattr = -1;
    out_attr(0);
    outs("\x1b[2J");
    tx = -1;
    for (int y = 1; y <= SCR_H; y++)
        for (int x = 1; x <= SCR_W; x++)
            out_cell(x, y);
}

/* Push pending output and place the (visible) cursor where crt has it. */
static void flush_screen(void)
{
    if (resized) {
        resized = 0;
        redraw_all();
    }
    out_goto(curx, cury);
    out_flush_raw();
}

static void restore_terminal(void)
{
    olen = 0;
    outs("\x1b[0m\x1b[?25h\x1b[?1049l");
    out_flush_raw();
    if (tio_saved)
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_tio);
}

static void on_signal(int sig)
{
    restore_terminal();
    signal(sig, SIG_DFL);
    raise(sig);
}

static void on_winch(int sig)
{
    (void)sig;
    resized = 1;
}

static void scroll_up(void)
{
    memmove(scr_ch[0], scr_ch[1], (SCR_H - 1) * SCR_W);
    memmove(scr_at[0], scr_at[1], (SCR_H - 1) * SCR_W);
    memset(scr_ch[SCR_H - 1], ' ', SCR_W);
    memset(scr_at[SCR_H - 1], textattr, SCR_W);
    redraw_all();
}

static void line_feed(void)
{
    if (cury < SCR_H)
        cury++;
    else
        scroll_up();
}

static void crt_init(void)
{
    struct winsize ws;
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        fprintf(stderr, "tetris: must be run in a terminal\n");
        exit(1);
    }
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && (ws.ws_col < SCR_W || ws.ws_row < SCR_H)) {
        fprintf(stderr, "tetris: the terminal must be at least %dx%d (it is %dx%d)\n", SCR_W,
                SCR_H, ws.ws_col, ws.ws_row);
        exit(1);
    }
    tcgetattr(STDIN_FILENO, &orig_tio);
    tio_saved = 1;
    struct termios t = orig_tio;
    t.c_iflag &= ~(unsigned)(ICRNL | INLCR | IGNCR | IXON | ISTRIP);
    t.c_lflag &= ~(unsigned)(ICANON | ECHO | IEXTEN);
    t.c_cc[VSUSP] = _POSIX_VDISABLE;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &t);

    atexit(restore_terminal);
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    signal(SIGQUIT, on_signal);
    signal(SIGHUP, on_signal);
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_winch;
    sigaction(SIGWINCH, &sa, NULL);

    outs("\x1b[?1049h\x1b[?25h");
    memset(scr_ch, ' ', sizeof scr_ch);
    memset(scr_at, 7, sizeof scr_at);
    redraw_all();
    flush_screen();
}

static void gotoxy(byte x, byte y)
{
    if (x >= 1 && x <= SCR_W && y >= 1 && y <= SCR_H) {
        curx = x;
        cury = y;
    }
}

static void textcolor(byte c) { textattr = (byte)((c & 0x8f) | (textattr & 0x70)); }

static void textbackground(byte c) { textattr = (byte)(((c << 4) & 0x70) | (textattr & 0x0f)); }

static void clrscr(void)
{
    memset(scr_ch, ' ', sizeof scr_ch);
    memset(scr_at, textattr, sizeof scr_at);
    curx = cury = 1;
    redraw_all();
}

/* Write one character through the crt text driver. */
static void wrc(byte c)
{
    switch (c) {
    case 7:
        outs("\a");
        return;
    case 8:
        if (curx > 1)
            curx--;
        return;
    case 10:
        line_feed();
        return;
    case 13:
        curx = 1;
        return;
    }
    scr_ch[cury - 1][curx - 1] = c;
    scr_at[cury - 1][curx - 1] = textattr;
    out_cell(curx, cury);
    if (++curx > SCR_W) {
        curx = 1;
        line_feed();
    }
}

static void wr(const char *lit)
{
    while (*lit)
        wrc(lit_next(&lit));
}

static void wrs(const pstr *s)
{
    for (int n = 1; n <= s->s[0]; n++)
        wrc(s->s[n]);
}

static void wrpad(int width, const byte *s, int len)
{
    for (int n = len; n < width; n++)
        wrc(' ');
    for (int n = 0; n < len; n++)
        wrc(s[n]);
}

static void wri_w(long v, int width)
{
    char b[32];
    int n = snprintf(b, sizeof b, "%ld", v);
    wrpad(width, (const byte *)b, n);
}

static void wri(long v) { wri_w(v, 0); }

static void wrlit_w(const char *lit, int width)
{
    pstr s = P(lit);
    wrpad(width, s.s + 1, s.s[0]);
}

static void wrs_w(const pstr *s, int width) { wrpad(width, s->s + 1, s->s[0]); }

static void delay(int ms)
{
    flush_screen();
    struct timespec ts = {ms / 1000, (long)(ms % 1000) * 1000000L};
    while (nanosleep(&ts, &ts) < 0 && errno == EINTR)
        flush_screen();
}

/* --- keyboard ----------------------------------------------------- */

static byte kq[512];
static int kq_head, kq_len;

static void kpush(byte c)
{
    if (kq_len < (int)sizeof kq)
        kq[(kq_head + kq_len++) % sizeof kq] = c;
}

static void kpush_ext(byte scan)
{
    kpush(0);
    kpush(scan);
}

static int read_byte(int timeout_ms)
{
    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};
    for (;;) {
        int r = poll(&pfd, 1, timeout_ms);
        if (r < 0 && errno == EINTR) {
            flush_screen();
            continue;
        }
        if (r <= 0)
            return -1;
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == 1)
            return c;
        if (n == 0)
            exit(0); /* stdin closed */
        if (errno != EINTR && errno != EAGAIN)
            return -1;
    }
}

static void parse_csi(void)
{
    int params[4] = {0}, np = 0, c;
    for (;;) {
        c = read_byte(30);
        if (c < 0)
            return;
        if (c >= '0' && c <= '9') {
            params[np] = params[np] * 10 + (c - '0');
        } else if (c == ';') {
            if (np < 3)
                np++;
        } else if (c >= 0x40 && c <= 0x7e) {
            break;
        }
    }
    switch (c) {
    case 'A': kpush_ext(72); return;
    case 'B': kpush_ext(80); return;
    case 'C': kpush_ext(77); return;
    case 'D': kpush_ext(75); return;
    case 'H': kpush_ext(71); return;
    case 'F': kpush_ext(79); return;
    case 'P': kpush_ext(59); return;
    case 'Q': kpush_ext(60); return;
    case 'R': kpush_ext(61); return;
    case 'S': kpush_ext(62); return;
    case '~':
        switch (params[0]) {
        case 1: case 7: kpush_ext(71); return;
        case 4: case 8: kpush_ext(79); return;
        case 2: kpush_ext(82); return;
        case 3: kpush_ext(83); return;
        case 5: kpush_ext(73); return;
        case 6: kpush_ext(81); return;
        case 11: case 12: case 13: case 14: case 15:
            kpush_ext((byte)(59 + params[0] - 11)); return;
        case 17: case 18: case 19: case 20: case 21:
            kpush_ext((byte)(64 + params[0] - 17)); return;
        case 23: kpush_ext(133); return;
        case 24: kpush_ext(134); return;
        }
        return;
    }
}

/* Translate one input byte (plus whatever belongs to it) into crt keys. */
static void decode_input(int c)
{
    if (c == 27) {
        int d = read_byte(25);
        if (d < 0) {
            kpush(27);
        } else if (d == '[' || d == 'O') {
            parse_csi();
        } else {
            kpush(27);
            decode_input(d);
        }
        return;
    }
    if (c == 127) {
        kpush(8);
        return;
    }
    if (c < 0x80) {
        kpush((byte)c);
        return;
    }
    uint32_t u;
    int more;
    if ((c & 0xe0) == 0xc0) {
        u = c & 0x1f;
        more = 1;
    } else if ((c & 0xf0) == 0xe0) {
        u = c & 0x0f;
        more = 2;
    } else if ((c & 0xf8) == 0xf0) {
        u = c & 0x07;
        more = 3;
    } else {
        return;
    }
    while (more--) {
        int d = read_byte(25);
        if (d < 0)
            return;
        u = (u << 6) | (d & 0x3f);
    }
    int b = to_cp852(u);
    if (b >= 0)
        kpush((byte)b);
}

static void poll_input(void)
{
    int c;
    while ((c = read_byte(0)) >= 0)
        decode_input(c);
}

static int keypressed(void)
{
    flush_screen();
    if (kq_len == 0)
        poll_input();
    return kq_len > 0;
}

static byte readkey(void)
{
    flush_screen();
    while (kq_len == 0) {
        int c = read_byte(-1);
        if (c >= 0)
            decode_input(c);
    }
    byte c = kq[kq_head];
    kq_head = (kq_head + 1) % (int)sizeof kq;
    kq_len--;
    return c;
}

/* ReadLn on the crt console: line editing with echo, then CR/LF. */
static void crt_readln(void)
{
    int len = 0;
    for (;;) {
        byte c = readkey();
        if (c == 0) {
            readkey();
        } else if (c == 13) {
            wrc(13);
            wrc(10);
            return;
        } else if (c == 8) {
            if (len > 0) {
                len--;
                wrc(8);
                wrc(' ');
                wrc(8);
            }
        } else if (c >= 32) {
            len++;
            wrc(c);
        }
    }
}

static void gettime(word *h, word *m, word *s, word *s100)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    *h = (word)tm.tm_hour;
    *m = (word)tm.tm_min;
    *s = (word)tm.tm_sec;
    *s100 = (word)(tv.tv_usec / 10000);
}

static int random_(int n) { return (int)((unsigned)rand() % (unsigned)n); }

/* ------------------------------------------------------------------ */
/* Text file I/O with FPC runtime semantics                           */
/* ------------------------------------------------------------------ */

enum { fm_closed, fm_input, fm_output };

typedef struct {
    const char *name;
    int mode;
    FILE *fp;
    byte *buf; /* whole file contents when open for input */
    size_t len, pos;
} textfile;

static void runtime_error(int code)
{
    restore_terminal();
    printf("Runtime error %d at $00000000\n", code);
    exit(code);
}

static void tf_close(textfile *f)
{
    if (f->mode == fm_output && f->fp)
        fclose(f->fp);
    free(f->buf);
    f->fp = NULL;
    f->buf = NULL;
    f->len = f->pos = 0;
    f->mode = fm_closed;
}

/* Returns the IOResult of the reset (0 = success). */
static int tf_reset(textfile *f)
{
    tf_close(f);
    FILE *fp = fopen(f->name, "rb");
    if (!fp)
        return 2;
    size_t cap = 4096;
    f->buf = malloc(cap);
    size_t n;
    while ((n = fread(f->buf + f->len, 1, cap - f->len, fp)) > 0) {
        f->len += n;
        if (f->len == cap)
            f->buf = realloc(f->buf, cap *= 2);
    }
    fclose(fp);
    f->pos = 0;
    f->mode = fm_input;
    return 0;
}

static void tf_rewrite(textfile *f)
{
    tf_close(f);
    f->fp = fopen(f->name, "wb");
    if (!f->fp)
        runtime_error(5);
    f->mode = fm_output;
}

static int tf_eof(textfile *f) { return f->pos >= f->len; }

static int is_ws(byte c) { return c == ' ' || c == 9 || c == 10 || c == 13; }

/* read(f, smallint) */
static int16_t tf_read_int(textfile *f)
{
    byte tok[256];
    int n = 0;
    while (!tf_eof(f) && is_ws(f->buf[f->pos]))
        f->pos++;
    if (tf_eof(f))
        return 0;
    while (!tf_eof(f) && n < 255 && !is_ws(f->buf[f->pos]))
        tok[n++] = f->buf[f->pos++];

    /* Val() */
    int k = 0, neg = 0, base = 10, any = 0;
    int64_t v = 0;
    if (k < n && (tok[k] == '+' || tok[k] == '-'))
        neg = tok[k++] == '-';
    if (k < n && tok[k] == '$') {
        base = 16;
        k++;
    }
    for (; k < n; k++) {
        int d;
        byte c = tok[k];
        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (base == 16 && c >= 'a' && c <= 'f')
            d = c - 'a' + 10;
        else if (base == 16 && c >= 'A' && c <= 'F')
            d = c - 'A' + 10;
        else
            runtime_error(106);
        v = v * base + d;
        any = 1;
    }
    if (!any)
        runtime_error(106);
    return (int16_t)(neg ? -v : v);
}

/* read(f, char) */
static byte tf_read_char(textfile *f)
{
    if (tf_eof(f))
        return 26;
    return f->buf[f->pos++];
}

/* readln(f, shortstring) */
static void tf_readln_str(textfile *f, pstr *s)
{
    s->s[0] = 0;
    while (!tf_eof(f) && s->s[0] < 255 && f->buf[f->pos] != 10 && f->buf[f->pos] != 13)
        s->s[++s->s[0]] = f->buf[f->pos++];
    while (!tf_eof(f)) {
        byte c = f->buf[f->pos++];
        if (c == 10)
            break;
        if (c == 13) {
            if (!tf_eof(f) && f->buf[f->pos] == 10)
                f->pos++;
            break;
        }
    }
}

/* ------------------------------------------------------------------ */
/* The program                                                        */
/* ------------------------------------------------------------------ */

typedef struct {
    byte oszlop;
    byte sor;
} koordinatak;

typedef struct {
    int16_t pont;
    int16_t sor;
    pstr nev;
} rec;

static textfile f = {"rekordok.txt", fm_closed, NULL, NULL, 0, 0};
static rec rekordok[4]; /* [1..3] */
static rec sv, semmi;
static byte most, valaszt;
static byte c;
static pstr mondat, nev;
static koordinatak alak[5];    /* [1..4] */
static int palya[40][25];       /* [28..39, 3..24] */
static byte i, j, k, l;
static byte melyik, wait, kov, sorok, szint, rszint;
static int nincsA, nincsM, alul, full;
static word o, p, s, ms, oo, pp, ss, msms;
static int16_t pont;

enum {
    black, blue, green, cyan, red, magenta, brown, lightgray,
    darkgray, lightblue, lightgreen, lightcyan, lightred, lightmagenta, yellow, white
};

static void sarokba(void) { gotoxy(80, 25); }

static void intro(void)
{
    byte a, b;
    gotoxy(13, 1);   wr("┌──────────────────────────────────────────────┐");
    FOR_TO(i, 2, 14) {
        gotoxy(13, i); wr("│                                              │");
    }
    gotoxy(13, 15);  wr("└──────────────────────────────────────────────┘");

    a = 15; b = 2;
    while (!keypressed() && b != 14) {
        gotoxy(a, b); wr("                                          "); b++;
        gotoxy(a, b); textcolor(9); wr("   ■     "); textcolor(10); wr("■ ■ ■     "); textcolor(9); wr("■     "); textcolor(12); wr("■   ■"); textcolor(14); wr("  ■  "); textcolor(11); wr("■ ■ ■ ■");
        sarokba();
        delay(100);
    }
    a = 15; b = 2;
    while (!keypressed() && b != 13) {
        gotoxy(a, b); wr("                                           "); b++;
        gotoxy(a, b); textcolor(9); wr("   ■     "); textcolor(10); wr("■         "); textcolor(9); wr("■     "); textcolor(12); wr("■  ■"); textcolor(14); wr("   ■        "); textcolor(11); wr(" ■");
        sarokba();
        delay(100);
    }

    a = 15; b = 2;
    while (!keypressed() && b != 12) {
        gotoxy(a, b); wr("                                           "); b++;
        gotoxy(a, b); textcolor(9); wr("   ■     "); textcolor(10); wr("■ ■ ■     "); textcolor(9); wr("■     "); textcolor(12); wr("■ ■    "); textcolor(14); wr("■  "); textcolor(11); wr(" ■ ■ ■ ■");
        sarokba();
        delay(100);
    }

    a = 15; b = 2;
    while (!keypressed() && b != 11) {
        gotoxy(a, b); wr("                                           "); b++;
        gotoxy(a, b); textcolor(9); wr("   ■     "); textcolor(10); wr("■         "); textcolor(9); wr("■     "); textcolor(12); wr("■   ■"); textcolor(14); wr("  ■  "); textcolor(11); wr("■");
        sarokba();
        delay(100);
    }

    a = 15; b = 2;
    while (!keypressed() && b != 10) {
        gotoxy(a, b); wr("                                           "); b++;
        gotoxy(a, b); textcolor(9); wr("■ ■ ■ ■  "); textcolor(10); wr("■ ■ ■  "); textcolor(9); wr("■ ■ ■ ■  "); textcolor(12); wr("■ ■ ■  "); textcolor(14); wr("■  "); textcolor(11); wr("■ ■ ■ ■");
        sarokba();
        delay(100);
    }
    if (!keypressed())
        delay(2000);

    a = 15; b = 2;
    while (!keypressed() && b != 14) {
        gotoxy(a, b); wr("                                            "); b++;
        gotoxy(a, b); textcolor(5); wr("■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■ ■");
        sarokba();
        delay(200);
    }
    gotoxy(a, b); wr("                                            ");
    sarokba();

    a = 13; b = 1;
    while (!keypressed() && b != 16) {
        gotoxy(a, b); wr("                                                      "); delay(30); b++;
        sarokba();
    }
}

static void animacio(pstr szoveg, byte oszlop, byte sor, byte szin, byte hatter)
{
    i = 0;
    textcolor(szin); textbackground(hatter);
    while (!keypressed() && i != szoveg.s[0] - 1) {
        gotoxy(oszlop + i, sor - 1); wrc(szoveg.s[i + 1]); delay(40);
        gotoxy(oszlop + i, sor - 1); wrc(' '); delay(40);
        gotoxy(oszlop + i, sor); wrc(szoveg.s[i + 1]);
        i++;
    }
    gotoxy(oszlop, sor); wrs(&szoveg);
    textbackground(0); textcolor(7);
}

static void menu(void)
{
    byte elo;
    clrscr();
    textbackground(9); textcolor(15);
    gotoxy(15, 5);  wr("╔═══════════════════╗");
    gotoxy(15, 6);  wr("║                   ║");
    gotoxy(15, 6);  wr("║                   ║");
    gotoxy(15, 7);  wr("║                   ║");
    gotoxy(15, 8);  wr("╠═══════════════════╣");
    gotoxy(15, 9);  wr("║                   ║");
    gotoxy(15, 10); wr("║                   ║");
    gotoxy(15, 11); wr("║                   ║");
    gotoxy(15, 12); wr("║                   ║");
    gotoxy(15, 13); wr("║                   ║");
    gotoxy(15, 14); wr("║                   ║");
    gotoxy(15, 15); wr("║                   ║");
    gotoxy(15, 16); wr("║                   ║");
    gotoxy(15, 17); wr("║                   ║");
    gotoxy(15, 18); wr("║                   ║");
    gotoxy(15, 19); wr("║                   ║");
    gotoxy(15, 20); wr("║                   ║");
    gotoxy(15, 21); wr("║                   ║");
    gotoxy(15, 22); wr("╚═══════════════════╝");

    textbackground(0);
    gotoxy(47, 7);  wr("  _______   _        _     ");
    gotoxy(47, 8);  wr(" |__   __| | |      (_)    ");
    gotoxy(47, 9);  wr("    | | ___| |_ _ __ _ ___ ");
    gotoxy(47, 10); wr("    | |/ _ \\ __| '__| / __|");
    gotoxy(47, 11); wr("    | |  __/ |_| |  | \\__ \\");
    gotoxy(47, 12); wr("    |_|\\___|\\__|_|  |_|___/");

    textbackground(7);
    FOR_TO(i, 6, 23) {
        gotoxy(36, i); wr(" ");
    }

    FOR_TO(i, 16, 36) {
        gotoxy(i, 23); wr(" ");
    }

    animacio(P("Tetris"), 22, 7, 12, 9);
    textbackground(9); textcolor(15);
    gotoxy(17, 10); wr("[ ]  Játék");
    gotoxy(17, 12); wr("[ ]  Rekordok");
    gotoxy(17, 14); wr("[ ]  Útmutató");
    gotoxy(17, 16); wr("[ ]  Adatok");
    gotoxy(17, 18); wr("[ ]  Kilépés");
    sarokba();

    valaszt = 0;
    elo = 1;
    while (valaszt == 0) {
        switch (most) {
        case 1: gotoxy(18, 10); break;
        case 2: gotoxy(18, 12); break;
        case 3: gotoxy(18, 14); break;
        case 4: gotoxy(18, 16); break;
        case 5: gotoxy(18, 18); break;
        }
        textcolor(10);
        wrc(175);
        elo = most;
        sarokba();
        c = readkey();

        switch (c) {
        case 80: most++; break;
        case 72: most--; break;
        case 13: valaszt = most; break;
        }

        if (most == 6) most = 1;
        if (most == 0) most = 5;

        switch (elo) {
        case 1: gotoxy(18, 10); break;
        case 2: gotoxy(18, 12); break;
        case 3: gotoxy(18, 14); break;
        case 4: gotoxy(18, 16); break;
        case 5: gotoxy(18, 18); break;
        }

        wr(" ");
    }
    textbackground(0); textcolor(7);
}

static void check(byte m)
{
    byte a, b;
    a = alak[m].oszlop;
    b = alak[m].sor;
    b++;
    if (palya[a][b])
        nincsA = 0;
    if (b == 24)
        alul = 0;
}

static void kirajzol(void)
{
    FOR_TO(i, 1, 4) {
        gotoxy(alak[i].oszlop, alak[i].sor); wrc(254);
    }
    sarokba();
}

static void torol(void)
{
    FOR_TO(i, 1, 4) {
        gotoxy(alak[i].oszlop, alak[i].sor); wr(" ");
    }
    sarokba();
}

static void beallit(void)
{
    byte a, b;
    FOR_TO(i, 1, 4) {
        a = alak[i].oszlop; b = alak[i].sor;
        palya[a][b] = 1;
    }
}

static koordinatak elhelyez(byte a, byte b)
{
    koordinatak x;
    x.oszlop = a;
    x.sor = b;
    return x;
}

static void setform(byte m)
{
    switch (m) {
    case 1:
        textcolor(cyan);
        alak[1] = elhelyez(32, 5);
        alak[2] = elhelyez(33, 5);
        alak[3] = elhelyez(34, 5);
        alak[4] = elhelyez(35, 5);
        break;
    case 2:
        textcolor(lightblue);
        alak[1] = elhelyez(35, 6);
        alak[2] = elhelyez(35, 5);
        alak[3] = elhelyez(34, 5);
        alak[4] = elhelyez(33, 5);
        break;
    case 3:
        textcolor(magenta);
        alak[1] = elhelyez(32, 6);
        alak[2] = elhelyez(32, 5);
        alak[3] = elhelyez(33, 5);
        alak[4] = elhelyez(34, 5);
        break;
    case 4:
        textcolor(yellow);
        alak[1] = elhelyez(33, 4);
        alak[2] = elhelyez(34, 4);
        alak[3] = elhelyez(33, 5);
        alak[4] = elhelyez(34, 5);
        break;
    case 5:
        textcolor(lightgreen);
        alak[1] = elhelyez(32, 6);
        alak[2] = elhelyez(33, 6);
        alak[3] = elhelyez(33, 5);
        alak[4] = elhelyez(34, 5);
        break;
    case 6:
        textcolor(lightred);
        alak[1] = elhelyez(34, 6);
        alak[2] = elhelyez(33, 6);
        alak[3] = elhelyez(33, 5);
        alak[4] = elhelyez(32, 5);
        break;
    case 7:
        textcolor(brown);
        alak[1] = elhelyez(32, 5);
        alak[2] = elhelyez(34, 5);
        alak[3] = elhelyez(33, 5);
        alak[4] = elhelyez(33, 6);
        break;
    }
}

static void checkL(void)
{
    byte a, b;
    a = alak[1].oszlop; b = alak[1].sor;
    if (palya[a - 1][b] || a - 1 < 29)
        nincsM = 0;
    a = alak[2].oszlop; b = alak[2].sor;
    if (palya[a - 1][b] || a - 1 < 29)
        nincsM = 0;
    a = alak[3].oszlop; b = alak[3].sor;
    if (palya[a - 1][b] || a - 1 < 29)
        nincsM = 0;
    a = alak[4].oszlop; b = alak[4].sor;
    if (palya[a - 1][b] || a - 1 < 29)
        nincsM = 0;
}

static void checkR(void)
{
    byte a, b;
    a = alak[1].oszlop; b = alak[1].sor;
    if (palya[a + 1][b] || a + 1 > 38)
        nincsM = 0;
    a = alak[2].oszlop; b = alak[2].sor;
    if (palya[a + 1][b] || a + 1 > 38)
        nincsM = 0;
    a = alak[3].oszlop; b = alak[3].sor;
    if (palya[a + 1][b] || a + 1 > 38)
        nincsM = 0;
    a = alak[4].oszlop; b = alak[4].sor;
    if (palya[a + 1][b] || a + 1 > 38)
        nincsM = 0;
}

static void checkLines(void)
{
    byte db;
    FOR_TO(i, 4, 23) { /* sor */
        db = 0;
        FOR_TO(j, 29, 38) { /* oszlop */
            if (palya[j][i])
                db = db + 1;
            if (db == 10) {
                textcolor(11);
                pont = (int16_t)(pont + 50); gotoxy(70, 5); wr("   "); gotoxy(70, 5); wri(pont);
                textcolor(9);
                sorok = sorok + 1; gotoxy(72, 7); wr("   "); gotoxy(72, 7); wri(sorok);
                textcolor(7);
                gotoxy(29, i);
                wr("          ");

                FOR_TO(k, 29, 38)
                    palya[k][i] = 0;

                FOR_DOWNTO(k, i - 1, 4)
                    FOR_TO(l, 29, 38)
                        if (palya[l][k]) {
                            palya[l][k] = 0; gotoxy(l, k); wr(" ");
                            palya[l][k + 1] = 1; gotoxy(l, k + 1); wrc(254);
                        }
            }
        }
    }
}

static void fordul(byte m)
{
    koordinatak x;
    (void)m;
    torol();
    x = alak[2];

    if (x.oszlop - 1 >= 29 && alak[4].oszlop <= 38 && alak[2].oszlop < 37 && x.sor < 22 &&

        !palya[x.oszlop - 1][x.sor - 1] &&
        !palya[x.oszlop][x.sor - 1] &&
        !palya[x.oszlop + 1][x.sor - 1] &&
        !palya[x.oszlop + 2][x.sor - 1] &&

        !palya[x.oszlop - 1][x.sor] &&
        !palya[x.oszlop][x.sor] &&
        !palya[x.oszlop + 1][x.sor] &&
        !palya[x.oszlop + 2][x.sor] &&

        !palya[x.oszlop - 1][x.sor + 1] &&
        !palya[x.oszlop][x.sor + 1] &&
        !palya[x.oszlop + 1][x.sor + 1] &&
        !palya[x.oszlop + 2][x.sor + 1] &&

        !palya[x.oszlop - 1][x.sor + 2] &&
        !palya[x.oszlop][x.sor + 2] &&
        !palya[x.oszlop + 1][x.sor + 2] &&
        !palya[x.oszlop + 2][x.sor + 2]) {

        if (melyik == 1) {
            if (alak[1].sor == alak[2].sor) {
                alak[1].oszlop = alak[2].oszlop;
                alak[1].sor = alak[2].sor - 1;
                alak[3].oszlop = alak[2].oszlop;
                alak[3].sor = alak[2].sor + 1;
                alak[4].oszlop = alak[2].oszlop;
                alak[4].sor = alak[2].sor + 2;
            } else {
                alak[1].sor = alak[2].sor;
                alak[1].oszlop = alak[2].oszlop - 1;
                alak[3].sor = alak[2].sor;
                alak[3].oszlop = alak[2].oszlop + 1;
                alak[4].sor = alak[2].sor;
                alak[4].oszlop = alak[2].oszlop + 2;
            }
        }
    }

    x = alak[3];
    if (x.oszlop - 1 >= 29 && x.oszlop + 1 <= 38 && x.sor + 1 < 23 &&

        !palya[x.oszlop - 1][x.sor - 1] &&
        !palya[x.oszlop][x.sor - 1] &&
        !palya[x.oszlop + 1][x.sor - 1] &&

        !palya[x.oszlop - 1][x.sor] &&
        !palya[x.oszlop][x.sor] &&
        !palya[x.oszlop + 1][x.sor] &&

        !palya[x.oszlop - 1][x.sor + 1] &&
        !palya[x.oszlop][x.sor + 1] &&
        !palya[x.oszlop + 1][x.sor + 1]) {
        switch (melyik) {
        case 2:
            if (alak[2].oszlop - 1 == alak[3].oszlop && alak[2].sor == alak[3].sor) {
                alak[1].oszlop = alak[4].oszlop;
                alak[2].oszlop = alak[3].oszlop;
                alak[2].sor = alak[1].sor;
                alak[4].oszlop = alak[3].oszlop;
                alak[4].sor = alak[3].sor - 1;
            } else if (alak[2].oszlop == alak[3].oszlop && alak[2].sor - 1 == alak[3].sor) {
                alak[1].sor = alak[4].sor;
                alak[2].oszlop = alak[3].oszlop - 1;
                alak[2].sor = alak[3].sor;
                alak[4].oszlop = alak[3].oszlop + 1;
                alak[4].sor = alak[3].sor;
            } else if (alak[2].oszlop + 1 == alak[3].oszlop && alak[2].sor == alak[3].sor) {
                alak[1].oszlop = alak[4].oszlop;
                alak[2].oszlop = alak[3].oszlop;
                alak[2].sor = alak[3].sor - 1;
                alak[4].oszlop = alak[3].oszlop;
                alak[4].sor = alak[3].sor + 1;
            } else if (alak[2].sor + 1 == alak[3].sor && alak[2].oszlop == alak[3].oszlop) {
                alak[1].sor = alak[4].sor;
                alak[2].oszlop = alak[3].oszlop + 1;
                alak[2].sor = alak[3].sor;
                alak[4].oszlop = alak[3].oszlop - 1;
                alak[4].sor = alak[3].sor;
            }
            break;
        case 3:
            if (alak[2].oszlop + 1 == alak[3].oszlop && alak[2].sor == alak[3].sor) {
                alak[1].sor = alak[1].sor - 2;
                alak[2].oszlop = alak[3].oszlop;
                alak[2].sor = alak[1].sor;
                alak[4].oszlop = alak[3].oszlop;
                alak[4].sor = alak[3].sor + 1;
            } else if (alak[2].oszlop == alak[3].oszlop && alak[2].sor + 1 == alak[3].sor) {
                alak[1].oszlop = alak[1].oszlop + 2;
                alak[2].oszlop = alak[3].oszlop + 1;
                alak[2].sor = alak[3].sor;
                alak[4].oszlop = alak[3].oszlop - 1;
                alak[4].sor = alak[3].sor;
            } else if (alak[2].oszlop - 1 == alak[3].oszlop && alak[2].sor == alak[3].sor) {
                alak[1].sor = alak[1].sor + 2;
                alak[2].oszlop = alak[3].oszlop;
                alak[2].sor = alak[3].sor + 1;
                alak[4].oszlop = alak[3].oszlop;
                alak[4].sor = alak[3].sor - 1;
            } else if (alak[2].oszlop == alak[3].oszlop && alak[2].sor - 1 == alak[3].sor) {
                alak[1].oszlop = alak[1].oszlop - 2;
                alak[2].oszlop = alak[3].oszlop - 1;
                alak[2].sor = alak[3].sor;
                alak[4].oszlop = alak[3].oszlop + 1;
                alak[4].sor = alak[3].sor;
            }
            break;
        case 4:
            /* kocka :D */
            break;
        case 5:
            if (alak[2].oszlop == alak[3].oszlop) {
                alak[1].oszlop = alak[1].oszlop + 2;
                alak[2].oszlop = alak[3].oszlop + 1;
                alak[2].sor = alak[3].sor;
                alak[4].oszlop = alak[3].oszlop;
                alak[4].sor = alak[3].sor - 1;
            } else {
                alak[1].oszlop = alak[1].oszlop - 2;
                alak[2].oszlop = alak[3].oszlop;
                alak[2].sor = alak[3].sor + 1;
                alak[4].oszlop = alak[3].oszlop + 1;
                alak[4].sor = alak[3].sor;
            }
            break;
        case 6:
            if (alak[2].oszlop == alak[3].oszlop) {
                alak[1].oszlop = alak[1].oszlop - 2;
                alak[2].oszlop = alak[3].oszlop - 1;
                alak[2].sor = alak[3].sor;
                alak[4].oszlop = alak[3].oszlop;
                alak[4].sor = alak[3].sor - 1;
            } else {
                alak[1].oszlop = alak[1].oszlop + 2;
                alak[2].oszlop = alak[3].oszlop;
                alak[2].sor = alak[3].sor + 1;
                alak[4].oszlop = alak[3].oszlop - 1;
                alak[4].sor = alak[3].sor;
            }
            break;
        case 7:
            if (alak[4].oszlop > alak[1].oszlop && alak[4].sor > alak[1].sor) {
                alak[1] = elhelyez(alak[3].oszlop, alak[3].sor - 1);
                alak[2] = elhelyez(alak[3].oszlop, alak[3].sor + 1);
                alak[4] = elhelyez(alak[3].oszlop - 1, alak[3].sor);
            } else if (alak[4].oszlop < alak[1].oszlop && alak[4].sor > alak[1].sor) {
                alak[1] = elhelyez(alak[3].oszlop + 1, alak[3].sor);
                alak[2] = elhelyez(alak[3].oszlop - 1, alak[3].sor);
                alak[4] = elhelyez(alak[3].oszlop, alak[3].sor - 1);
            } else if (alak[4].oszlop < alak[1].oszlop && alak[4].sor < alak[1].sor) {
                alak[1] = elhelyez(alak[3].oszlop, alak[3].sor + 1);
                alak[2] = elhelyez(alak[3].oszlop, alak[3].sor - 1);
                alak[4] = elhelyez(alak[3].oszlop + 1, alak[3].sor);
            } else if (alak[4].oszlop > alak[1].oszlop && alak[4].sor < alak[1].sor) {
                alak[1] = elhelyez(alak[3].oszlop - 1, alak[3].sor);
                alak[2] = elhelyez(alak[3].oszlop + 1, alak[3].sor);
                alak[4] = elhelyez(alak[3].oszlop, alak[3].sor + 1);
            }
            break;
        }
    }
    kirajzol();
}

static void ablak(byte x, byte y, byte h, byte sz, byte cl, byte bc)
{
    textcolor(cl); textbackground(bc);

    gotoxy(x, y);
    FOR_TO(i, 1, sz)
        if (i == 1) wr("╔");
        else if (i == sz) wr("╗");
        else wr("═");

    FOR_TO(i, 1, h - 1) {
        gotoxy(x, y + i);
        FOR_TO(j, 1, sz)
            if (j == 1 || j == sz) wr("║");
            else wr(" ");
    }

    gotoxy(x, y + h);

    FOR_TO(i, 1, sz)
        if (i == 1) wr("╚");
        else if (i == sz) wr("╝");
        else wr("═");

    textcolor(7); textbackground(0);
}

static void porog(byte x, byte y, byte z)
{
    gotoxy(x, y); wr("|"); sarokba(); delay(z);
    gotoxy(x, y); wr("/"); sarokba(); delay(z);
    gotoxy(x, y); wr("-"); sarokba(); delay(z);
    gotoxy(x, y); wr("\\"); sarokba(); delay(z);
    gotoxy(x, y); wr("|"); sarokba(); delay(z);
    gotoxy(x, y); wr("/"); sarokba(); delay(z);
    gotoxy(x, y); wr("-"); sarokba(); delay(z);
    gotoxy(x, y); wr("\\"); sarokba(); delay(z);
    gotoxy(x, y); wr("|"); sarokba(); delay(z);
}

static void vporog(byte x, byte y, byte z)
{
    gotoxy(x, y); wr("| "); sarokba(); delay(z);
    gotoxy(x, y); wr("\\ "); sarokba(); delay(z);
    gotoxy(x, y); wr("- "); sarokba(); delay(z);
    gotoxy(x, y); wr("/ "); sarokba(); delay(z);
    gotoxy(x, y); wr("| "); sarokba(); delay(z);
    gotoxy(x, y); wr("\\ "); sarokba(); delay(z);
    gotoxy(x, y); wr("- "); sarokba(); delay(z);
    gotoxy(x, y); wr("/ "); sarokba(); delay(z);
    gotoxy(x, y); wr("| "); sarokba(); delay(z);
}

static void ipko(void)
{
    byte i;
    pstr s;
    s = P("Programmed by: ipko");
    i = 1;
    while (i < s.s[0] + 1) {
        textcolor(random_(14) + 1);
        gotoxy(5 + i, 5);
        wrc(s.s[i]);
        porog(5 + i + 1, 5, 15);
        i++;
    }
    delay(1000);
    textcolor(7);
    while (i > 0) {
        vporog(5 + i, 5, 10);
        i--;
    }
}

/* Sets the text colour of the falling piece (used after pause/soft drop). */
static void piece_color(void)
{
    switch (melyik) {
    case 1: textcolor(cyan); break;
    case 2: textcolor(lightblue); break;
    case 3: textcolor(magenta); break;
    case 4: textcolor(yellow); break;
    case 5: textcolor(lightgreen); break;
    case 6: textcolor(lightred); break;
    case 7: textcolor(brown); break;
    }
}

static void info_frame(void)
{
    gotoxy(5, 5);  wr("╔═══════════════════════════════════════════╗");
    gotoxy(5, 6);  wr("║                                           ║");
    gotoxy(5, 6);  wr("║                                           ║");
    gotoxy(5, 7);  wr("║                                           ║");
    gotoxy(5, 8);  wr("╠═══════════════════════════════════════════╣");
    gotoxy(5, 9);  wr("║                                           ║");
}

static void info_logo(void)
{
    gotoxy(52, 7);  wr("  _______   _        _     ");
    gotoxy(52, 8);  wr(" |__   __| | |      (_)    ");
    gotoxy(52, 9);  wr("    | | ___| |_ _ __ _ ___ ");
    gotoxy(52, 10); wr("    | |/ _ \\ __| '__| / __|");
    gotoxy(52, 11); wr("    | |  __/ |_| |  | \\__ \\");
    gotoxy(52, 12); wr("    |_|\\___|\\__|_|  |_|___/");

    textbackground(7);
    FOR_TO(i, 6, 23) {
        gotoxy(50, i); wr(" ");
    }

    FOR_TO(i, 6, 50) {
        gotoxy(i, 23); wr(" ");
    }
}

static void vissza(void)
{
    wr("║               "); textcolor(10); wr(" »"); textcolor(7); wr(" Vissza"); textcolor(15); wr("                   ║");
}

static void jatek(void)
{
    clrscr();

    gotoxy(2, 2); wr("Loading");
    gotoxy(2, 3); wr("[          ]");
    textcolor(lightblue);
    FOR_TO(i, 1, 10) { gotoxy(2 + i, 3); wrc(219); delay(40); }

    textcolor(7);
    clrscr();
    pont = 0;
    full = 0;
    alul = 1;
    nincsA = 1;

    FOR_TO(i, 28, 39)
        FOR_TO(j, 4, 23)
            palya[i][j] = 0;

    gotoxy(28, 3);  wrc(175); FOR_TO(i, 1, 10) wrc(244); wrc(174);
    for (int y = 4; y <= 23; y++) {
        gotoxy(28, y); wrc(175); wr("          "); wrc(174);
    }
    gotoxy(28, 24); FOR_TO(i, 1, 12) wrc(94);
    textcolor(14);
    gotoxy(48, 2); wr("Következő:");
    gotoxy(50, 3); wr("┌────┐");
    gotoxy(50, 4); wr("│    │");
    gotoxy(50, 5); wr("│    │");
    gotoxy(50, 6); wr("│    │");
    gotoxy(50, 7); wr("└────┘");
    textcolor(11);
    gotoxy(65, 5); wr("Pont: "); wri(pont);
    textcolor(9);
    gotoxy(65, 7); wr("Sorok: "); wri(sorok);

    textcolor(15);
    gotoxy(58, 13); wr("Szint:");
    textcolor(8);
    gotoxy(5, 7); wr(" Használd a");
    gotoxy(5, 8); wr("nyíl gombokat");
    gotoxy(3, 9); wr("("); wrc(27); wr(" "); wrc(26); wr(") az alakzat");
    gotoxy(5, 10); wr("mozgatásához.");
    gotoxy(4, 12); wr("A le nyilat ("); wrc(25); wr(")");
    gotoxy(2, 13); wr("ha gyorsítani, Space-t");
    gotoxy(3, 14); wr("ha forgatni akarsz.");

    sarokba();
    kov = (byte)(random_(6) + 1);
    wait = 25; rszint = 0; sorok = 0;
    textcolor(12);
    gotoxy(2, 18); wr("Nyomj Entert a kezdéshez!");
    gotoxy(2, 19); wr("────────────────────────"); sarokba();
    do {
        c = readkey();
    } while (!(c == 13 || c == 27));

    gotoxy(2, 18); wr("                         ");
    gotoxy(2, 19); wr("                         ");

    textcolor(4);
    gotoxy(6, 18); wr("P - Pause");

    while (!full) {

        if (sorok < 3) szint = 1;
        if (sorok >= 3 && sorok < 6) szint = 2;
        if (sorok >= 6 && sorok < 9) szint = 3;
        if (sorok >= 9 && sorok < 12) szint = 4;
        if (sorok >= 12 && sorok < 15) szint = 5;

        if (szint != rszint) {
            textcolor(2);
            switch (szint) {
            case 1:
                gotoxy(60, 15); wr(" ██");
                gotoxy(60, 16); wr("███");
                gotoxy(60, 17); wr(" ██");
                gotoxy(60, 18); wr(" ██");
                gotoxy(60, 19); wr(" ██");
                rszint = 1;
                break;
            case 2:
                wait = wait - 3;
                gotoxy(60, 15); wr("██████ ");
                gotoxy(60, 16); wr("     ██");
                gotoxy(60, 17); wr(" █████ ");
                gotoxy(60, 18); wr("██     ");
                gotoxy(60, 19); wr("███████");
                rszint = 2;
                break;
            case 3:
                wait = wait - 3;
                gotoxy(60, 15); wr("██████ ");
                gotoxy(60, 16); wr("     ██");
                gotoxy(60, 17); wr(" █████ ");
                gotoxy(60, 18); wr("     ██");
                gotoxy(60, 19); wr("██████ ");
                rszint = 3;
                break;
            case 4:
                wait = wait - 3;
                gotoxy(60, 15); wr("██   ██");
                gotoxy(60, 16); wr("██   ██");
                gotoxy(60, 17); wr("███████");
                gotoxy(60, 18); wr("     ██");
                gotoxy(60, 19); wr("     ██");
                rszint = 4;
                break;
            case 5:
                wait = wait - 3;
                gotoxy(60, 15); wr("███████");
                gotoxy(60, 16); wr("██     ");
                gotoxy(60, 17); wr("███████");
                gotoxy(60, 18); wr("     ██");
                gotoxy(60, 19); wr("███████");
                rszint = 5;
                break;
            }
        }

        melyik = kov;
        kov = (byte)(random_(6) + 1);

        textcolor(7);

        switch (kov) {
        case 1:
            gotoxy(51, 5); wr("■■■■");
            gotoxy(51, 6); wr("    ");
            break;
        case 2:
            gotoxy(51, 5); wr("■■■ ");
            gotoxy(51, 6); wr("  ■ ");
            break;
        case 3:
            gotoxy(51, 5); wr("■■■ ");
            gotoxy(51, 6); wr("■   ");
            break;
        case 4:
            gotoxy(51, 5); wr(" ■■ ");
            gotoxy(51, 6); wr(" ■■ ");
            break;
        case 5:
            gotoxy(51, 5); wr("  ■■");
            gotoxy(51, 6); wr(" ■■ ");
            break;
        case 6:
            gotoxy(51, 5); wr("■■  ");
            gotoxy(51, 6); wr(" ■■ ");
            break;
        case 7:
            gotoxy(51, 5); wr(" ■  ");
            gotoxy(51, 6); wr("■■■ ");
            break;
        }

        setform(melyik);
        kirajzol();

        nincsA = 1; alul = 1; nincsM = 1;

        while (nincsA && alul) {
            kirajzol();
            nincsM = 1;
            gettime(&o, &p, &s, &ms);
            FOR_TO(i, 1, 20) {
                if (keypressed()) {
                    c = readkey();
                    nincsM = 1;
                    switch (c) {
                    case 75:
                        checkL();
                        if (nincsM) {
                            torol();
                            FOR_TO(j, 1, 4)
                                alak[j].oszlop--;
                            kirajzol();
                        }
                        break;
                    case 77:
                        checkR();
                        if (nincsM) {
                            torol();
                            FOR_TO(j, 1, 4)
                                alak[j].oszlop++;
                            kirajzol();
                        }
                        break;
                    case 80:
                        textcolor(11);
                        pont = (int16_t)(pont + 1); gotoxy(70, 5); wr("  "); gotoxy(70, 5); wri(pont);
                        piece_color();
                        goto tick_done; /* break */
                    case 32:
                        fordul(melyik);
                        break;
                    case 'p':
                    case 27:
                        textcolor(4);
                        gotoxy(6, 18); wr("P - Play ");
                        gotoxy(6, 19); wr("K - Kilépés");
                        sarokba();
                        do {
                            c = readkey();
                        } while (!(c == 'p' || c == 'P' || c == 'k' || c == 'K' || c == 27));
                        if (c == 'k' || c == 'K') {
                            full = 1;
                            nincsA = 0;
                            alul = 0;
                        }
                        gotoxy(6, 18); wr("P - Pause ");
                        gotoxy(6, 19); wr("           ");
                        piece_color();
                        break;
                    }
                }
                delay(wait);
                gettime(&oo, &pp, &ss, &msms);
                if (ms + (s * 100) + wait * 2 < msms + (ss * 100))
                    break;
            }
        tick_done:
            torol();
            FOR_TO(i, 29, 38) if (palya[i][5]) full = 1;
            FOR_TO(i, 1, 4) check(i);
            if (nincsA == 0)
                break;
            FOR_TO(i, 1, 4) alak[i].sor++;
            FOR_TO(i, 1, 4) check(i);
            FOR_TO(i, 29, 38) if (palya[i][5]) full = 1;
        }
        textcolor(7);
        kirajzol();
        beallit();
        checkLines();
    }

    /* {$-} is not a valid directive, so this reset runs under {$I+} */
    if (tf_reset(&f) != 0)
        runtime_error(2);
    {
        FOR_TO(i, 1, 3) {
            rekordok[i].pont = tf_read_int(&f);
            c = tf_read_char(&f);
            tf_readln_str(&f, &rekordok[i].nev);
        }

        if (pont > rekordok[3].pont) {
            ablak(25, 10, 8, 26, 15, 9);
            textcolor(15); textbackground(9);
            gotoxy(33, 12); wr("Új rekord");
            gotoxy(27, 15); wr("Neved (max 8):");
            i = 1;
            pset(&nev, "        ");
            while (i <= 8) {
                c = readkey();
                if (c != 13) {
                    if (c == 8) {
                        gotoxy(40 + i - 1, 15); wr(" "); gotoxy(40 + i - 1, 15); i--;
                    } else {
                        nev.s[i] = c;
                        wrc(c);
                        i++;
                    }
                } else {
                    i = 9;
                }
            }
            delay(50);
            textcolor(7); textbackground(0);
        }

        if (pont > rekordok[3].pont && pont > rekordok[2].pont && pont > rekordok[1].pont) {
            sv = rekordok[2];
            rekordok[2] = rekordok[1];
            rekordok[3] = sv;
            rekordok[1].pont = pont;
            pcopy(&rekordok[1].nev, &nev);
            rekordok[1].sor = sorok;
        }

        if (pont > rekordok[3].pont && pont > rekordok[2].pont && pont < rekordok[1].pont) {
            rekordok[3] = rekordok[2];
            rekordok[2].pont = pont;
            pcopy(&rekordok[2].nev, &nev);
            rekordok[2].sor = sorok;
        }

        if (pont > rekordok[3].pont && pont < rekordok[2].pont && pont < rekordok[1].pont) {
            rekordok[3].pont = pont;
            pcopy(&rekordok[3].nev, &nev);
            rekordok[3].sor = sorok;
        }

        tf_rewrite(&f);

        FOR_TO(i, 1, 3) {
            fprintf(f.fp, "%d %d ", rekordok[i].pont, rekordok[i].sor);
            fwrite(rekordok[i].nev.s + 1, 1, rekordok[i].nev.s[0], f.fp);
            fputc('\n', f.fp);
        }

        tf_close(&f);
    }

    menu();
}

static void rekordok_kepernyo(void)
{
    clrscr();
    textbackground(9); textcolor(15);
    info_frame();
    for (int y = 10; y <= 17; y++) {
        gotoxy(5, y); wr("║                                           ║");
    }
    gotoxy(5, 18); vissza();
    for (int y = 19; y <= 21; y++) {
        gotoxy(5, y); wr("║                                           ║");
    }
    gotoxy(5, 22); wr("╚═══════════════════════════════════════════╝");

    textbackground(0); textcolor(7);
    info_logo();

    animacio(P("Rekordok"), 23, 7, 12, 9); sarokba();

    if (tf_reset(&f) != 0) {
        gotoxy(17, 13); textcolor(white); textbackground(blue); wr("Még nincsenek rekordok"); textbackground(black); sarokba();
    } else {
        FOR_TO(i, 1, 3) {
            rekordok[i].pont = tf_read_int(&f);
            c = tf_read_char(&f);
            rekordok[i].sor = tf_read_int(&f);
            c = tf_read_char(&f);
            tf_readln_str(&f, &rekordok[i].nev);
        }
        textcolor(white); textbackground(blue);
        gotoxy(7, 9); wr("Helyezet"); wrlit_w("Név", 6); wrlit_w("Pont", 11); wrlit_w("Sorok", 6);
        gotoxy(10, 11); wr("I.    "); wrs_w(&rekordok[1].nev, 10); wri_w(rekordok[1].pont, 6); wri_w(rekordok[1].sor, 6);
        gotoxy(10, 13); wr("II.   "); wrs_w(&rekordok[2].nev, 10); wri_w(rekordok[2].pont, 6); wri_w(rekordok[2].sor, 6);
        gotoxy(10, 15); wr("III.  "); wrs_w(&rekordok[3].nev, 10); wri_w(rekordok[3].pont, 6); wri_w(rekordok[3].sor, 6);

        textbackground(black);
        sarokba();
        tf_close(&f);
    }
    gotoxy(55, 20); wr("T - Rekordok törlése"); sarokba();
    do {
        c = readkey();
    } while (!(c == 13 || c == 't'));
    if (c == 't') {
        tf_rewrite(&f);
        FOR_TO(i, 1, 3)
            rekordok[i] = semmi;
    }

    menu();
}

static void utmutato(void)
{
    clrscr();
    textbackground(9); textcolor(15);
    info_frame();
    gotoxy(5, 10); wr("║       A játék során fentről lefelé        ║");
    gotoxy(5, 11); wr("║  véletlenszerű alakzatok fognak potyogni. ║");
    gotoxy(5, 12); wr("║      A cél az hogy úgy kell felépíteni    ║");
    gotoxy(5, 13); wr("║         az elemeket, hogy a sor           ║");
    gotoxy(5, 14); wr("║         csak azokat tartalmazza.          ║");
    gotoxy(5, 15); wr("║                                           ║");
    gotoxy(5, 16); wr("║                                           ║");
    gotoxy(5, 17); wr("║                                           ║");
    gotoxy(5, 18); vissza();
    gotoxy(5, 19); wr("║                                           ║");
    gotoxy(5, 20); wr("║                                           ║");
    gotoxy(5, 21); wr("║                                           ║");
    gotoxy(5, 22); wr("╚═══════════════════════════════════════════╝");

    textbackground(0); textcolor(7);
    info_logo();
    animacio(P("Útmutató"), 25, 7, 12, 9);
    sarokba();
    crt_readln();
    menu();
}

static void adatok(void)
{
    clrscr();
    textbackground(9); textcolor(15);
    info_frame();
    gotoxy(5, 10); wr("║    Verzó: 1.0                             ║");
    gotoxy(5, 11); wr("║                                           ║");
    gotoxy(5, 12); wr("║    Tesztelők: Dani, Joni                  ║");
    gotoxy(5, 13); wr("║               Márkó, Ulbi                 ║");
    gotoxy(5, 14); wr("║                                           ║");
    gotoxy(5, 15); wr("║    Dátum: 2014.07.13                      ║");
    gotoxy(5, 16); wr("║                                           ║");
    gotoxy(5, 17); wr("║    Programot írta: "); textcolor(lightgreen); wr("ipko"); textcolor(white); wr("                   ║");
    gotoxy(5, 18); wr("║                                           ║");
    gotoxy(5, 19); wr("║                                           ║");
    gotoxy(5, 20); vissza();
    gotoxy(5, 21); wr("║                                           ║");
    gotoxy(5, 22); wr("╚═══════════════════════════════════════════╝");

    textbackground(0); textcolor(7);
    info_logo();
    animacio(P("Adatok"), 25, 7, 12, 9);
    sarokba();
    crt_readln();
    menu();
}

static void kilepes(void)
{
    clrscr();
    pset(&mondat, "E g y  é l m é n y   v o l t : D  ! ! !  Szia");
    i = 1;
    while (!keypressed() && i < mondat.s[0]) {
        gotoxy(4 + i, 5); wrc(mondat.s[i]); i++; delay(20);
    }
    i = 1;
    while (i <= 40 && !keypressed()) {
        textcolor(random_(14) + 1);
        clrscr();
        gotoxy(5, 5);
        wr("E g y  é l m é n y   v o l t : D  ! ! ! Szia"); delay(100);
        i++;
    }
    i = 48;
    textcolor(7);
    while (!keypressed() && i != 0) {
        gotoxy(i, 5); wr(" "); i--; delay(30);
    }

    valaszt = 6;
}

int main(void)
{
    crt_init();
    clrscr();
    srand((unsigned)time(NULL) ^ (unsigned)getpid());

    if (tf_reset(&f) == 0) {
        FOR_TO(i, 1, 3) {
            rekordok[i].pont = tf_read_int(&f);
            c = tf_read_char(&f);
            tf_readln_str(&f, &rekordok[i].nev);
        }
    } else {
        tf_rewrite(&f);
    }

    intro();
    if (keypressed())
        c = readkey();
    most = 1;
    menu();
    clrscr();
    while (valaszt != 6) {
        switch (valaszt) {
        case 1: jatek(); break;
        case 2: rekordok_kepernyo(); break;
        case 3: utmutato(); break;
        case 4: adatok(); break;
        case 5: kilepes(); break;
        }
    }
    delay(50);
    gotoxy(5, 5); wr("                                            ");
    ipko();
    delay(1000);
    tf_close(&f);
    return 0;
}
