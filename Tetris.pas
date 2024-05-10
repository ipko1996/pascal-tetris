uses crt,dos;

type
koordinatak=
record
  oszlop:byte;
  sor:byte;
end;

rec=
record
 pont:integer;
 sor:integer;
 nev:string;
end;

var
  f:text;
  rekordok:array [1..3] of rec;
  sv,semmi:rec;
  most,valaszt:byte;
  c:char;
  mondat,nev:string;
  alak : array [1..4] of koordinatak;
  palya : array [28..39,3..24] of boolean;
  i,j,k,l:byte;
  melyik,wait,kov,sorok,szint,rszint:byte;
  nincsA,nincsM,alul,full:boolean;
  o,p,s,ms,oo,pp,ss,msms:word;
  pont:integer;

procedure sarokba;
  begin
    gotoxy(80,25);
  end;

procedure intro;
var a,b:byte;
  begin
    gotoxy(13,1);   Write('ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ');
    for i:=2 to 14 do begin
      gotoxy(13,i); Write('ณ                                              ณ');
     end;
    gotoxy(13,15);  Write('ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู');


    a:=15; b:=2;
    while (not keypressed) and (b<>14)  do
      begin
        gotoxy(a,b); Write('                                          '); inc(b);
        gotoxy(a,b); textcolor(9); Write('   þ     ');textcolor(10);Write('þ þ þ     ');textcolor(9) ;Write('þ     ');textcolor(12);Write('þ   þ');textcolor(14);Write('  þ  ');textcolor(11);Write('þ þ þ þ');
        sarokba;
        delay(100);
      end;
    a:=15; b:=2;
    while (not keypressed) and (b<>13)  do
      begin
        gotoxy(a,b);Write('                                           '); inc(b);
        gotoxy(a,b);textcolor(9);Write('   þ     ');textcolor(10);Write('þ         ');textcolor(9);Write('þ     ');textcolor(12);Write('þ  þ');textcolor(14); Write('   þ        ');textcolor(11);Write(' þ');
        sarokba;
        delay(100);
      end;

    a:=15; b:=2;
    while (not keypressed) and (b<>12)  do
      begin
        gotoxy(a,b); Write('                                           '); inc(b);
        gotoxy(a,b); textcolor(9); Write('   þ     ');textcolor(10); Write('þ þ þ     ');textcolor(9);Write('þ     ');textcolor(12);Write('þ þ    ');textcolor(14); Write('þ  ');textcolor(11); Write(' þ þ þ þ');
        sarokba;
        delay(100);
      end;

    a:=15; b:=2;
    while (not keypressed) and (b<>11)  do
      begin
        gotoxy(a,b); Write('                                           '); inc(b);
        gotoxy(a,b);textcolor(9); Write('   þ     ');textcolor(10); Write('þ         ');textcolor(9);Write('þ     ');textcolor(12);Write('þ   þ');textcolor(14);Write('  þ  ');textcolor(11);Write('þ');
        sarokba;
        delay(100);
      end;

    a:=15; b:=2;
    while (not keypressed) and (b<>10)  do
      begin
        gotoxy(a,b); Write('                                           '); inc(b);
        gotoxy(a,b);textcolor(9); Write('þ þ þ þ  ');textcolor(10);Write('þ þ þ  ');textcolor(9);Write('þ þ þ þ  ');textcolor(12);Write('þ þ þ  ');textcolor(14);Write('þ  ');textcolor(11);Write('þ þ þ þ');
        sarokba;
        delay(100);
      end;
    if not keypressed
      then
        delay(2000);

    a:=15; b:=2;
    while (not keypressed) and (b<>14)  do
      begin
        gotoxy(a,b); Write('                                            '); inc(b);
        gotoxy(a,b);textcolor(5); Write('þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ');
        sarokba;
        delay(200);
      end;
      gotoxy(a,b); Write('                                            ');
      sarokba;

    a:=13; b:=1;
    while (not keypressed) and (b<>16) do begin
      gotoxy(a,b); Write('                                                      '); delay(30); inc(b);
      sarokba;
    end;

  end;

procedure animacio(szoveg:string;oszlop,sor,szin,hatter:byte);
  begin
    i:=0;
    textcolor(szin); textbackground(hatter);
    while (not keypressed) and (i<>length(szoveg)-1) do
      begin
        gotoxy(oszlop+i,sor-1); Write(szoveg[i+1]); Delay(40);
        gotoxy(oszlop+i,sor-1); Write(' '); Delay(40);
        gotoxy(oszlop+i,sor); Write(szoveg[i+1]); //Delay(20);
        inc(i);
      end;
  gotoxy(oszlop,sor); Write(szoveg);
  textbackground(0); textcolor(7);
  end;

procedure menu;
var elo:byte;
  begin
    ClrScr;
    textbackground(9); textcolor(15);
    gotoxy(15,5);  Write('ษอออออออออออออออออออป');
    gotoxy(15,6);  Write('บ                   บ');
    gotoxy(15,6);  Write('บ                   บ');
    gotoxy(15,7);  Write('บ                   บ');
    gotoxy(15,8);  Write('ฬอออออออออออออออออออน');
    gotoxy(15,9);  Write('บ                   บ');
    gotoxy(15,10); Write('บ                   บ');
    gotoxy(15,11); Write('บ                   บ');
    gotoxy(15,12); Write('บ                   บ');
    gotoxy(15,13); Write('บ                   บ');
    gotoxy(15,14); Write('บ                   บ');
    gotoxy(15,15); Write('บ                   บ');
    gotoxy(15,16); Write('บ                   บ');
    gotoxy(15,17); Write('บ                   บ');
    gotoxy(15,18); Write('บ                   บ');
    gotoxy(15,19); Write('บ                   บ');
    gotoxy(15,20); Write('บ                   บ');
    gotoxy(15,21); Write('บ                   บ');
    gotoxy(15,22); Write('ศอออออออออออออออออออผ');

    textbackground(0);
    gotoxy(47,7);  Write('  _______   _        _     ');
    gotoxy(47,8);  Write(' |__   __| | |      (_)    ');
    gotoxy(47,9);  Write('    | | ___| |_ _ __ _ ___ ');
    gotoxy(47,10); Write('    | |/ _ \ __| ''__| / __|');
    gotoxy(47,11); Write('    | |  __/ |_| |  | \__ \');
    gotoxy(47,12); Write('    |_|\___|\__|_|  |_|___/');

    textbackground(7);
    for i:=6 to 23 do
      begin
        gotoxy(36,i); Write(' ');
      end;

    for i:=16 to 36 do
      begin
        gotoxy(i,23); Write(' ');
      end;

    animacio('Tetris',22,7,12,9);
    textbackground(9); textcolor(15);
    gotoxy(17,10); Write('[ ]  J tk');
    gotoxy(17,12); Write('[ ]  Rekordok');
    gotoxy(17,14); Write('[ ]  ้tmutatข');
    gotoxy(17,16); Write('[ ]  Adatok');
    gotoxy(17,18); Write('[ ]  Kilps');
    sarokba;

    valaszt:=0;
//    most:=1;
    elo:=1;
    while valaszt=0 do begin
      case most of
        1 : gotoxy(18,10);
        2 : gotoxy(18,12);
        3 : gotoxy(18,14);
        4 : gotoxy(18,16);
        5 : gotoxy(18,18);
       end;
       textcolor(10);
       Write(#175);
      elo:=most;
      sarokba;
      c:=readkey;

      case c of
        #80 : inc(most);
        #72 : dec(most);
        #13 : valaszt:=most;
       end;

      if most=6 then most:=1;
      if most=0 then most:=5;

      case elo of
      1 : gotoxy(18,10);
      2 : gotoxy(18,12);
      3 : gotoxy(18,14);
      4 : gotoxy(18,16);
      5 : gotoxy(18,18);
      end;

      Write(' ');

    end;
    textbackground(0); textcolor(7);
 end;

procedure check(m:byte);
var a,b:byte;
begin
  a:=alak[m].oszlop;
  b:=alak[m].sor;
  inc(b);
  if palya[a,b]=true
    then
      nincsA:=false;
   if b=24
    then
      alul:=false;
end;

procedure kirajzol;
begin
  for i:=1 to 4 do
    begin
      gotoxy(alak[i].oszlop,alak[i].sor);  Write(#254);
    end;
  sarokba;
end;

procedure torol;
begin
  for i:=1 to 4 do
    begin
      gotoxy(alak[i].oszlop,alak[i].sor);  Write(' ');
    end;
  sarokba;
end;

procedure beallit;
var a,b:byte;
begin
  for i:=1 to 4 do
    begin
      a:=alak[i].oszlop; b:=alak[i].sor;
      palya[a,b]:=true;
    end;
end;

function elhelyez(a,b:byte):koordinatak;
var x:koordinatak;
begin
  x.oszlop:=a;
  x.sor:=b;
  elhelyez:=x;
end;

procedure setform(m:byte);
begin
  case m of
    1:begin
        textcolor(cyan);
        alak[1]:=elhelyez(32,5);
        alak[2]:=elhelyez(33,5);
        alak[3]:=elhelyez(34,5);
        alak[4]:=elhelyez(35,5);
      end;
    2:begin
        textcolor(lightblue);
        alak[1]:=elhelyez(35,6);
        alak[2]:=elhelyez(35,5);
        alak[3]:=elhelyez(34,5);
        alak[4]:=elhelyez(33,5);
      end;
    3:begin
        textcolor(magenta);
        alak[1]:=elhelyez(32,6);
        alak[2]:=elhelyez(32,5);
        alak[3]:=elhelyez(33,5);
        alak[4]:=elhelyez(34,5);
      end;
    4:begin
        textcolor(yellow);
        alak[1]:=elhelyez(33,4);
        alak[2]:=elhelyez(34,4);
        alak[3]:=elhelyez(33,5);
        alak[4]:=elhelyez(34,5);
      end;
    5:begin
        textcolor(lightgreen);
        alak[1]:=elhelyez(32,6);
        alak[2]:=elhelyez(33,6);
        alak[3]:=elhelyez(33,5);
        alak[4]:=elhelyez(34,5);
      end;
    6:begin
        textcolor(lightred);
        alak[1]:=elhelyez(34,6);
        alak[2]:=elhelyez(33,6);
        alak[3]:=elhelyez(33,5);
        alak[4]:=elhelyez(32,5);
      end;
    7:begin
        textcolor(brown);
        alak[1]:=elhelyez(32,5);
        alak[2]:=elhelyez(34,5);
        alak[3]:=elhelyez(33,5);
        alak[4]:=elhelyez(33,6);
      end;
  end;
end;

procedure checkL;
var a,b:byte;
begin
  a:=alak[1].oszlop; b:=alak[1].sor;
  if (palya[a-1,b]) OR (a-1<29)
    then
      nincsM:=false;
  a:=alak[2].oszlop; b:=alak[2].sor;
  if (palya[a-1,b])  OR (a-1<29)
    then
      nincsM:=false;
  a:=alak[3].oszlop; b:=alak[3].sor;
  if (palya[a-1,b])  OR (a-1<29)
    then
      nincsM:=false;
  a:=alak[4].oszlop; b:=alak[4].sor;
  if (palya[a-1,b])  OR (a-1<29)
    then
      nincsM:=false;
end;

procedure checkR;
var a,b:byte;
begin
  a:=alak[1].oszlop; b:=alak[1].sor;
  if (palya[a+1,b]) OR (a+1>38)
    then
      nincsM:=false;
  a:=alak[2].oszlop; b:=alak[2].sor;
  if (palya[a+1,b])  OR (a+1>38)
    then
      nincsM:=false;
  a:=alak[3].oszlop; b:=alak[3].sor;
  if (palya[a+1,b])  OR (a+1>38)
    then
      nincsM:=false;
  a:=alak[4].oszlop; b:=alak[4].sor;
  if (palya[a+1,b])  OR (a+1>38)
    then
      nincsM:=false;
end;

procedure checkLines;
var db:byte;
begin
  for i:=4 to 23 do //sor
    begin
      db:=0;
      for j:=29 to 38 do //oszlop
        begin
          if palya[j,i]
            then
              db:=db+1;
          if db=10
            then
              begin
                textcolor(11);
                pont:=pont+50;  gotoxy(70,5); Write('   '); gotoxy(70,5); Write(pont);
                textcolor(9);
                sorok:=sorok+1; gotoxy(72,7); Write('   '); gotoxy(72,7); Write(sorok);
                textcolor(7);
                gotoxy(29,i);
                Write('          ');

                for k:=29 to 38 do
                  palya[k,i]:=false;

                for k:=i-1 downto 4 do
                  for l:=29 to 38 do
                    if palya[l,k]
                      then
                        begin
                          palya[l,k]:=false; gotoxy(l,k); Write(' ');
                          palya[l,k+1]:=true; gotoxy(l,k+1); Write(#254);
                        end;
              end;
        end;
    end;
end;

procedure fordul(m:byte);
var x:koordinatak;
begin
  torol;
  x:=alak[2];

  	if (x.oszlop-1>=29) AND (alak[4].oszlop<=38) AND (alak[2].oszlop<37) AND (x.sor<22) AND

	   (palya[x.oszlop-1,x.sor-1]=FALSE) AND
	   (palya[x.oszlop,x.sor-1]=FALSE) AND
	   (palya[x.oszlop+1,x.sor-1]=FALSE) AND
	   (palya[x.oszlop+2,x.sor-1]=FALSE) AND

	   (palya[x.oszlop-1,x.sor]=FALSE) AND
	   (palya[x.oszlop,x.sor]=FALSE) AND
	   (palya[x.oszlop+1,x.sor]=FALSE) AND
	   (palya[x.oszlop+2,x.sor]=FALSE) AND

	   (palya[x.oszlop-1,x.sor+1]=FALSE) AND
	   (palya[x.oszlop,x.sor+1]=FALSE) AND
	   (palya[x.oszlop+1,x.sor+1]=FALSE) AND
	   (palya[x.oszlop+2,x.sor+1]=FALSE) AND

	   (palya[x.oszlop-1,x.sor+2]=FALSE) AND
	   (palya[x.oszlop,x.sor+2]=FALSE) AND
	   (palya[x.oszlop+1,x.sor+2]=FALSE) AND
       (palya[x.oszlop+2,x.sor+2]=FALSE)

	   then begin

  if melyik=1 then begin
	    if alak[1].sor=alak[2].sor
		  then
		    begin
			  alak[1].oszlop:=alak[2].oszlop;
			  alak[1].sor:=alak[2].sor-1;
			  alak[3].oszlop:=alak[2].oszlop;
			  alak[3].sor:=alak[2].sor+1;
			  alak[4].oszlop:=alak[2].oszlop;
			  alak[4].sor:=alak[2].sor+2;
			end
		  else
		    begin
			  alak[1].sor:=alak[2].sor;
			  alak[1].oszlop:=alak[2].oszlop-1;
			  alak[3].sor:=alak[2].sor;
			  alak[3].oszlop:=alak[2].oszlop+1;
			  alak[4].sor:=alak[2].sor;
			  alak[4].oszlop:=alak[2].oszlop+2;
			end;
	  end;

	  end;

	x:=alak[3];
	if (x.oszlop-1>=29) AND (x.oszlop+1<=38) AND (x.sor+1<23) AND

	   (palya[x.oszlop-1,x.sor-1]=FALSE) AND
	   (palya[x.oszlop,x.sor-1]=FALSE) AND
	   (palya[x.oszlop+1,x.sor-1]=FALSE) AND

	   (palya[x.oszlop-1,x.sor]=FALSE) AND
	   (palya[x.oszlop,x.sor]=FALSE) AND
	   (palya[x.oszlop+1,x.sor]=FALSE) AND

	   (palya[x.oszlop-1,x.sor+1]=FALSE) AND
	   (palya[x.oszlop,x.sor+1]=FALSE) AND
	   (palya[x.oszlop+1,x.sor+1]=FALSE)

	   then begin
	case melyik of
	2:begin

	    if (alak[2].oszlop-1=alak[3].oszlop) AND (alak[2].sor=alak[3].sor)
		  then
		    begin
		      alak[1].oszlop:=alak[4].oszlop;
    	      alak[2].oszlop:=alak[3].oszlop;
	      	  alak[2].sor:=alak[1].sor;
		      alak[4].oszlop:=alak[3].oszlop;
			  alak[4].sor:=alak[3].sor-1;
			end
		else if (alak[2].oszlop=alak[3].oszlop) AND (alak[2].sor-1=alak[3].sor)
		  then
		    begin
			  alak[1].sor:=alak[4].sor;
			  alak[2].oszlop:=alak[3].oszlop-1;
			  alak[2].sor:=alak[3].sor;
			  alak[4].oszlop:=alak[3].oszlop+1;
			  alak[4].sor:=alak[3].sor
			end
	    else if (alak[2].oszlop+1=alak[3].oszlop) AND (alak[2].sor=alak[3].sor)
		  then
		    begin
			  alak[1].oszlop:=alak[4].oszlop;
			  alak[2].oszlop:=alak[3].oszlop;
			  alak[2].sor:=alak[3].sor-1;
			  alak[4].oszlop:=alak[3].oszlop;
			  alak[4].sor:=alak[3].sor+1;
			end
		else if (alak[2].sor+1=alak[3].sor) AND (alak[2].oszlop=alak[3].oszlop)
		  then
		    begin
			  alak[1].sor:=alak[4].sor;
			  alak[2].oszlop:=alak[3].oszlop+1;
			  alak[2].sor:=alak[3].sor;
			  alak[4].oszlop:=alak[3].oszlop-1;
			  alak[4].sor:=alak[3].sor;
			end;
	  end;
	3:begin
	    if (alak[2].oszlop+1=alak[3].oszlop) AND (alak[2].sor=alak[3].sor)
		  then
		    begin
			  alak[1].sor:=alak[1].sor-2;
			  alak[2].oszlop:=alak[3].oszlop;
			  alak[2].sor:=alak[1].sor;
			  alak[4].oszlop:=alak[3].oszlop;
			  alak[4].sor:=alak[3].sor+1;
			end
		else if (alak[2].oszlop=alak[3].oszlop) AND (alak[2].sor+1=alak[3].sor)
		  then
		    begin
			  alak[1].oszlop:=alak[1].oszlop+2;
			  alak[2].oszlop:=alak[3].oszlop+1;
			  alak[2].sor:=alak[3].sor;
			  alak[4].oszlop:=alak[3].oszlop-1;
			  alak[4].sor:=alak[3].sor;
			end
	    else if (alak[2].oszlop-1=alak[3].oszlop) AND (alak[2].sor=alak[3].sor)
		  then
		    begin
			  alak[1].sor:=alak[1].sor+2;
			  alak[2].oszlop:=alak[3].oszlop;
			  alak[2].sor:=alak[3].sor+1;
			  alak[4].oszlop:=alak[3].oszlop;
			  alak[4].sor:=alak[3].sor-1;
			end
		else if (alak[2].oszlop=alak[3].oszlop) AND (alak[2].sor-1=alak[3].sor)
		  then
		    begin
			  alak[1].oszlop:=alak[1].oszlop-2;
			  alak[2].oszlop:=alak[3].oszlop-1;
			  alak[2].sor:=alak[3].sor;
			  alak[4].oszlop:=alak[3].oszlop+1;
			  alak[4].sor:=alak[3].sor;
			end;
	  end;
	4:begin
	  //kocka :D
	  end;
	5:begin
	    if (alak[2].oszlop=alak[3].oszlop)
		  then
		    begin
			  alak[1].oszlop:=alak[1].oszlop+2;
			  alak[2].oszlop:=alak[3].oszlop+1;
			  alak[2].sor:=alak[3].sor;
			  alak[4].oszlop:=alak[3].oszlop;
			  alak[4].sor:=alak[3].sor-1;
			end
		  else
		    begin
			  alak[1].oszlop:=alak[1].oszlop-2;
			  alak[2].oszlop:=alak[3].oszlop;
			  alak[2].sor:=alak[3].sor+1;
			  alak[4].oszlop:=alak[3].oszlop+1;
			  alak[4].sor:=alak[3].sor;
			end;
	  end;
	6:begin
	    if (alak[2].oszlop=alak[3].oszlop)
		  then
		    begin
			  alak[1].oszlop:=alak[1].oszlop-2;
			  alak[2].oszlop:=alak[3].oszlop-1;
			  alak[2].sor:=alak[3].sor;
			  alak[4].oszlop:=alak[3].oszlop;
			  alak[4].sor:=alak[3].sor-1;
			end
		else
		  begin
		    alak[1].oszlop:=alak[1].oszlop+2;
			alak[2].oszlop:=alak[3].oszlop;
			alak[2].sor:=alak[3].sor+1;
			alak[4].oszlop:=alak[3].oszlop-1;
			alak[4].sor:=alak[3].sor;
		  end;
	  end;
	7:begin
	    if (alak[4].oszlop>alak[1].oszlop) AND (alak[4].sor>alak[1].sor)
		  then
		   begin
		     alak[1]:=elhelyez(alak[3].oszlop,alak[3].sor-1);
			 alak[2]:=elhelyez(alak[3].oszlop,alak[3].sor+1);
			 alak[4]:=elhelyez(alak[3].oszlop-1,alak[3].sor);
		   end
		else if (alak[4].oszlop<alak[1].oszlop) AND (alak[4].sor>alak[1].sor)
		  then
		    begin
		     alak[1]:=elhelyez(alak[3].oszlop+1,alak[3].sor);
			 alak[2]:=elhelyez(alak[3].oszlop-1,alak[3].sor);
			 alak[4]:=elhelyez(alak[3].oszlop,alak[3].sor-1);
			end
		else if (alak[4].oszlop<alak[1].oszlop) AND (alak[4].sor<alak[1].sor)
		  then
		    begin
		     alak[1]:=elhelyez(alak[3].oszlop,alak[3].sor+1);
			 alak[2]:=elhelyez(alak[3].oszlop,alak[3].sor-1);
			 alak[4]:=elhelyez(alak[3].oszlop+1,alak[3].sor);
			end
		else if (alak[4].oszlop>alak[1].oszlop) AND (alak[4].sor<alak[1].sor)
		  then
		    begin
		     alak[1]:=elhelyez(alak[3].oszlop-1,alak[3].sor);
			 alak[2]:=elhelyez(alak[3].oszlop+1,alak[3].sor);
			 alak[4]:=elhelyez(alak[3].oszlop,alak[3].sor+1);
			end;
	  end;
	end;
  end;
  kirajzol;
end;

procedure ablak(x,y,h,s,c,bc:byte);
  begin

    textcolor(c); textbackground(bc);

    gotoxy(x,y);
    for i:=1 to s do
      if i=1 then Write('ษ')
      else if i=s then Write('ป')
      else Write('อ');

    for i:=1 to h-1 do
      begin
      gotoxy(x,y+i);
      for j:=1 to s do
        if (j=1) or (j=s) then Write('บ')
        else Write(' ');
      end;

    gotoxy(x,y+h);

    for i:=1 to s do
      if i=1 then Write('ศ')
      else if i=s then Write('ผ')
      else Write('อ');

    textcolor(7); textbackground(0);

  end;

procedure porog(x,y,z:byte);
begin
        gotoxy(x,y); write('|'); sarokba; delay(z);
    gotoxy(x,y); write('/'); sarokba; delay(z);
    gotoxy(x,y); write('-'); sarokba; delay(z);
    gotoxy(x,y); write('\'); sarokba; delay(z);
    gotoxy(x,y); write('|'); sarokba; delay(z);
    gotoxy(x,y); write('/'); sarokba; delay(z);
    gotoxy(x,y); write('-'); sarokba; delay(z);
    gotoxy(x,y); write('\'); sarokba; delay(z);
    gotoxy(x,y); write('|'); sarokba; delay(z);
end;

procedure vporog(x,y,z:byte);
begin
  gotoxy(x,y); write('| '); sarokba; delay(z);
  gotoxy(x,y); write('\ '); sarokba; delay(z);
  gotoxy(x,y); write('- '); sarokba; delay(z);
  gotoxy(x,y); write('/ '); sarokba; delay(z);
  gotoxy(x,y); write('| '); sarokba; delay(z);
  gotoxy(x,y); write('\ '); sarokba; delay(z);
  gotoxy(x,y); write('- '); sarokba; delay(z);
  gotoxy(x,y); write('/ '); sarokba; delay(z);
  gotoxy(x,y); write('| '); sarokba; delay(z);
end;

procedure ipko;
var i:byte;
	s:string;
begin
  s:='Programmed by: ipko';
  i:=1;
  while (i<length(s)+1) do
    begin
	  textcolor(random(14)+1);
	  gotoxy(5+i,5);
	  Write(s[i]);
	  porog(5+i+1,5,15);
          inc(i);
	end;
	delay(1000);
    textcolor(7);
	while i>0 do
	  begin
	    vporog(5+i,5,10);
		dec(i);
	  end;

end;


begin
  ClrScr;
  Randomize;
  assign(f,'rekordok.txt');

  {$I-}
  reset(f);
  {$I+}
   if IOResult=0 then
      begin
        for i:=1 to 3 do
          begin
            read(f,rekordok[i].pont);
            read(f,c);
            readln(f,rekordok[i].nev);
          end;
        end
     else
       rewrite(f);

  intro;
  if keypressed
    then
      c:=readkey;
  most:=1;
  menu;
  ClrScr;
while valaszt<>6 do begin

  case valaszt of

  1:begin
  ClrScr;

  gotoxy(2,2); Write('Loading');
  gotoxy(2,3); Write('[          ]');
  textcolor(lightblue);
  for i:=1 to 10 do begin gotoxy(2+i,3); Write(#219); delay(40); end;

  textcolor(7);
  ClrScr;
  pont:=0;
  full:=false;
  alul:=true;
  nincsA:=true;

  for i:=28 to 39 do
    for j:=4 to 23 do
      palya[i,j]:=false;

  gotoxy(28,3);  Write(#175); for i:=1 to 10 do Write(#244); Write(#174);
  gotoxy(28,4);  Write(#175,'          '#174);
  gotoxy(28,5);  Write(#175,'          '#174);
  gotoxy(28,6);  Write(#175,'          '#174);
  gotoxy(28,7);  Write(#175,'          '#174);
  gotoxy(28,8);  Write(#175,'          '#174);
  gotoxy(28,9);  Write(#175,'          '#174);
  gotoxy(28,10); Write(#175,'          '#174);
  gotoxy(28,11); Write(#175,'          '#174);
  gotoxy(28,12); Write(#175,'          '#174);
  gotoxy(28,13); Write(#175,'          '#174);
  gotoxy(28,14); Write(#175,'          '#174);
  gotoxy(28,15); Write(#175,'          '#174);
  gotoxy(28,16); Write(#175,'          '#174);
  gotoxy(28,17); Write(#175,'          '#174);
  gotoxy(28,18); Write(#175,'          '#174);
  gotoxy(28,19); Write(#175,'          '#174);
  gotoxy(28,20); Write(#175,'          '#174);
  gotoxy(28,21); Write(#175,'          '#174);
  gotoxy(28,22); Write(#175,'          '#174);
  gotoxy(28,23); Write(#175,'          '#174);
  gotoxy(28,24); for i:=1 to 12 do Write(#94);
  textcolor(14);
  gotoxy(48,2); Write('K”vetkez:');
  gotoxy(50,3); Write('ฺฤฤฤฤฟ');
  gotoxy(50,4); Write('ณ    ณ');
  gotoxy(50,5); Write('ณ    ณ');
  gotoxy(50,6); Write('ณ    ณ');
  gotoxy(50,7); Write('ภฤฤฤฤู');
  textcolor(11);
  gotoxy(65,5); Write('Pont: ',pont);
  textcolor(9);
  gotoxy(65,7); Write('Sorok: ',sorok);

  textcolor(15);
  gotoxy(58,13); Write('Szint:');
  textcolor(8);
  gotoxy(5,7); Write(' Haszn ld a');
  gotoxy(5,8); Write('nyกl gombokat');
  gotoxy(3,9); Write('(',#27,' ',#26,') az alakzat');
  gotoxy(5,10); Write('mozgat s hoz.');
  gotoxy(4,12); Write('A le nyilat (',#25,')');
  gotoxy(2,13); Write('ha gyorsกtani, Space-t');
  gotoxy(3,14); Write('ha forgatni akarsz.');

  sarokba;
  kov:=random(6)+1;
  wait:=25; rszint:=0; sorok:=0;
  textcolor(12);
  gotoxy(2,18); Write('Nyomj Entert a kezdshez!');
  gotoxy(2,19); Write('ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ'); sarokba;
  repeat
  c:=Readkey;
  until (c=#13) or (c=#27);

  gotoxy(2,18); Write('                         ');
  gotoxy(2,19); Write('                         ');

  textcolor(4);
  gotoxy(6,18); Write('P - Pause');

  while not full do
    begin


	if sorok<3 then szint:=1;
	if (sorok>=3) AND (sorok<6)  then szint:=2;
	if (sorok>=6) AND (sorok<9)  then szint:=3;
        if (sorok>=9) AND (sorok<12) then szint:=4;
        if (sorok>=12) AND (sorok<15) then szint:=5;

	if szint<>rszint
	  then  begin
                textcolor(2);
		case szint of
		  1:begin
                    gotoxy(60,15); Write(' ÛÛ');
                    gotoxy(60,16); Write('ÛÛÛ');
                    gotoxy(60,17); Write(' ÛÛ');
                    gotoxy(60,18); Write(' ÛÛ');
                    gotoxy(60,19); Write(' ÛÛ');
                    rszint:=1;
		    end;
		  2:begin
                    wait:=wait-3;
                    gotoxy(60,15); Write('ÛÛÛÛÛÛ ');
                    gotoxy(60,16); Write('     ÛÛ');
                    gotoxy(60,17); Write(' ÛÛÛÛÛ ');
                    gotoxy(60,18); Write('ÛÛ     ');
                    gotoxy(60,19); Write('ÛÛÛÛÛÛÛ');
                    rszint:=2;
		    end;
                  3:begin
                    wait:=wait-3;
                    gotoxy(60,15); Write('ÛÛÛÛÛÛ ');
                    gotoxy(60,16); Write('     ÛÛ');
                    gotoxy(60,17); Write(' ÛÛÛÛÛ ');
                    gotoxy(60,18); Write('     ÛÛ');
                    gotoxy(60,19); Write('ÛÛÛÛÛÛ ');
                    rszint:=3;
                    end;
                  4:begin
                    wait:=wait-3;
                    gotoxy(60,15); Write('ÛÛ   ÛÛ');
                    gotoxy(60,16); Write('ÛÛ   ÛÛ');
                    gotoxy(60,17); Write('ÛÛÛÛÛÛÛ');
                    gotoxy(60,18); Write('     ÛÛ');
                    gotoxy(60,19); Write('     ÛÛ');
                    rszint:=4;
                    end;
                  5:begin
                    wait:=wait-3;
                    gotoxy(60,15); Write('ÛÛÛÛÛÛÛ');
                    gotoxy(60,16); Write('ÛÛ     ');
                    gotoxy(60,17); Write('ÛÛÛÛÛÛÛ');
                    gotoxy(60,18); Write('     ÛÛ');
                    gotoxy(60,19); Write('ÛÛÛÛÛÛÛ');
                    rszint:=5;
                    end;
		end;
                end;

      melyik:=kov;
	  kov:=random(6)+1;

      textcolor(7);

      case kov of
      1:begin
          gotoxy(51,5); Write('þþþþ');
          gotoxy(51,6); Write('    ');
        end;
      2:begin
          gotoxy(51,5); Write('þþþ ');
          gotoxy(51,6); Write('  þ ');
        end;
      3:begin
          gotoxy(51,5); Write('þþþ ');
          gotoxy(51,6); Write('þ   ');
        end;
      4:begin
          gotoxy(51,5); Write(' þþ ');
          gotoxy(51,6); Write(' þþ ');
        end;
      5:begin
          gotoxy(51,5); Write('  þþ');
          gotoxy(51,6); Write(' þþ ');
        end;
      6:begin
          gotoxy(51,5); Write('þþ  ');
          gotoxy(51,6); Write(' þþ ');
        end;
      7:begin
          gotoxy(51,5); Write(' þ  ');
          gotoxy(51,6); Write('þþþ ');
        end;
      end;

      setform(melyik);
      kirajzol;

      nincsA:=true; alul:=true; nincsM:=true;

      while (nincsA) AND (alul) do
        begin
          kirajzol;
          nincsM:=true;
		  gettime(o,p,s,ms);
          for i:=1 to 20 do
            begin
              if keypressed
                then
                  begin
                    c:=readkey;
					nincsM:=true;
                    case c of
                      #75:begin
                            checkL;
                            if nincsM
                              then
                                begin
                                  torol;
                                  for j:=1 to 4 do
                                    dec(alak[j].oszlop);
                                  kirajzol;
                                end;
                          end;
                      #77:begin
                            checkR;
                            if nincsM
                              then
                                begin
                                  torol;
                                  for j:=1 to 4 do
                                    inc(alak[j].oszlop);
                                  kirajzol;
                                end;
                          end;
					  #80:begin
                                              textcolor(11);
					      pont:=pont+1; gotoxy(70,5); write('  '); gotoxy(70,5); write(pont);
                                              case melyik of
                                                1:textcolor(cyan);
                                                2:textcolor(lightblue);
                                                3:textcolor(magenta);
                                                4:textcolor(yellow);
                                                5:textcolor(lightgreen);
                                                6:textcolor(lightred);
                                                7:textcolor(brown);
                                              end;
					      break;
						  end;
					  #32:fordul(melyik);
					  'p',#27:begin
                                                textcolor(4);
                                                gotoxy(6,18); Write('P - Play ');
                                                gotoxy(6,19); Write('K - Kilps');
                                                sarokba;
                                                repeat
                                                  c:=Readkey;
                                                until (c='p') or (c='P') or (c='k') or (c='K') or (c=#27);
                                                if (c='k') or (c='K')
                                                  then
                                                    begin
                                                      full:=true;
                                                      nincsA:=false;
                                                      alul:=false;
                                                    end;
                                                gotoxy(6,18); Write('P - Pause ');
                                                gotoxy(6,19); Write('           ');
                                              case melyik of
                                                1:textcolor(cyan);
                                                2:textcolor(lightblue);
                                                3:textcolor(magenta);
                                                4:textcolor(yellow);
                                                5:textcolor(lightgreen);
                                                6:textcolor(lightred);
                                                7:textcolor(brown);
                                              end;
                                              end;
                    end;
                  end;
              delay(wait);
              gettime(oo,pp,ss,msms);
			  if ms+(s*100)+wait*2<msms+(ss*100)
			    then
				  break;
            end;
          torol;
		  for i:=29 to 38 do if palya[i,5] then full:=true;
		  for i:=1 to 4 do check(i);
		  if nincsA=false
		    then
			  break;
          for i:=1 to 4 do inc(alak[i].sor);
          for i:=1 to 4 do check(i);
		  for i:=29 to 38 do if palya[i,5] then full:=true;
        end;
    textcolor(7);
    kirajzol;
    beallit;
    checkLines;

    end;


    {$-}
    reset(f);
    {$+}
    if IOResult=0
      then
        begin
          for i:=1 to 3 do
            begin
              read(f,rekordok[i].pont);
              read(f,c);
              readln(f,rekordok[i].nev);
            end;

          if pont>rekordok[3].pont
            then
              begin
                ablak(25,10,8,26,15,9);
                textcolor(15); textbackground(9);
                gotoxy(33,12); Write('้j rekord');
                gotoxy(27,15); Write('Neved (max 8):');
                i:=1;
                nev:='        ';
                while i<=8 do
                  begin
                    c:=readkey;
                    if c<>#13 then begin
                    if c=#8 then begin gotoxy(40+i-1,15); Write(' '); gotoxy(40+i-1,15); dec(i); end
                    else begin
                      nev[i]:=c;
                      Write(c);
                      inc(i);
                      end;
                    end
                    else i:=9;
                  end;
                  delay(50);
                textcolor(7); textbackground(0);
              end;

          if (pont>rekordok[3].pont) AND (pont>rekordok[2].pont) AND (pont>rekordok[1].pont)
            then
              begin
                 sv:=rekordok[2];
                 rekordok[2]:=rekordok[1];
                 rekordok[3]:=sv;
                 rekordok[1].pont:=pont;
                 rekordok[1].nev:=nev;
                 rekordok[1].sor:=sorok;
              end;

          if (pont>rekordok[3].pont) AND (pont>rekordok[2].pont) AND (pont<rekordok[1].pont)
            then
              begin
                rekordok[3]:=rekordok[2];
                rekordok[2].pont:=pont;
                rekordok[2].nev:=nev;
                rekordok[2].sor:=sorok;
              end;

         if (pont>rekordok[3].pont) AND (pont<rekordok[2].pont) AND (pont<rekordok[1].pont)
            then
              begin
                rekordok[3].pont:=pont;
                rekordok[3].nev:=nev;
                rekordok[3].sor:=sorok;
              end;


          rewrite(f);

          WriteLn(f,rekordok[1].pont,' ',rekordok[1].sor,' ',rekordok[1].nev);
          WriteLn(f,rekordok[2].pont,' ',rekordok[2].sor,' ',rekordok[2].nev);
          WriteLn(f,rekordok[3].pont,' ',rekordok[3].sor,' ',rekordok[3].nev);

          close(f);

      end;

    menu;
  end;

  2:begin
   clrscr;
   textbackground(9); textcolor(15);
    gotoxy(5,5);  Write('ษอออออออออออออออออออออออออออออออออออออออออออป');
    gotoxy(5,6);  Write('บ                                           บ');
    gotoxy(5,6);  Write('บ                                           บ');
    gotoxy(5,7);  Write('บ                                           บ');
    gotoxy(5,8);  Write('ฬอออออออออออออออออออออออออออออออออออออออออออน');
    gotoxy(5,9);  Write('บ                                           บ');
    gotoxy(5,10); Write('บ                                           บ');
    gotoxy(5,11); Write('บ                                           บ');
    gotoxy(5,12); Write('บ                                           บ');
    gotoxy(5,13); Write('บ                                           บ');
    gotoxy(5,14); Write('บ                                           บ');
    gotoxy(5,15); Write('บ                                           บ');
    gotoxy(5,16); Write('บ                                           บ');
    gotoxy(5,17); Write('บ                                           บ');
    gotoxy(5,18); Write('บ               ');textcolor(10);Write(' ฏ');textcolor(7);Write(' Vissza');textcolor(15);Write('                   บ');
    gotoxy(5,19); Write('บ                                           บ');
    gotoxy(5,20); Write('บ                                           บ');
    gotoxy(5,21); Write('บ                                           บ');
    gotoxy(5,22); Write('ศอออออออออออออออออออออออออออออออออออออออออออผ');

       textbackground(0); textcolor(7);
    gotoxy(52,7);  Write('  _______   _        _     ');
    gotoxy(52,8);  Write(' |__   __| | |      (_)    ');
    gotoxy(52,9);  Write('    | | ___| |_ _ __ _ ___ ');
    gotoxy(52,10); Write('    | |/ _ \ __| ''__| / __|');
    gotoxy(52,11); Write('    | |  __/ |_| |  | \__ \');
    gotoxy(52,12); Write('    |_|\___|\__|_|  |_|___/');

    textbackground(7);
    for i:=6 to 23 do
      begin
        gotoxy(50,i); Write(' ');
      end;

    for i:=6 to 50 do
      begin
        gotoxy(i,23); Write(' ');
      end;

   animacio('Rekordok',23,7,12,9); sarokba;


   {$I-}
   reset(f);
   {$I+}

   if IOResult<>0 then begin gotoxy(17,13); textcolor(white); textbackground(blue); Write('Mg nincsenek rekordok'); textbackground(black); sarokba; end

   else
      begin
        for i:=1 to 3 do
          begin
            read(f,rekordok[i].pont);
            read(f,c);
            read(f,rekordok[i].sor);
            read(f,c);
            readln(f,rekordok[i].nev);
          end;
        textcolor(white); textbackground(blue);
        gotoxy(7,9); Write('Helyezet','Nv':6,'Pont':11,'Sorok':6);
        gotoxy(10,11); Write('I.    '); Write(rekordok[1].nev:10); Write(rekordok[1].pont:6); Write(rekordok[1].sor:6);
        gotoxy(10,13); Write('II.   '); Write(rekordok[2].nev:10); Write(rekordok[2].pont:6); Write(rekordok[2].sor:6);
        gotoxy(10,15); Write('III.  '); Write(rekordok[3].nev:10); Write(rekordok[3].pont:6); Write(rekordok[3].sor:6);

        textbackground(black);
        sarokba;
        close(f);
      end;
   gotoxy(55,20); Write('T - Rekordok t”rlse'); sarokba;
   repeat
     c:=ReadKey;
   until (c=#13) or (c='t');
   if c='t' then
     begin
      rewrite(f);
      for i:=1 to 3 do
         rekordok[i]:=semmi;

     end;

  menu;
  end;

  3:begin
    ClrScr;
    textbackground(9); textcolor(15);
    gotoxy(5,5);  Write('ษอออออออออออออออออออออออออออออออออออออออออออป');
    gotoxy(5,6);  Write('บ                                           บ');
    gotoxy(5,6);  Write('บ                                           บ');
    gotoxy(5,7);  Write('บ                                           บ');
    gotoxy(5,8);  Write('ฬอออออออออออออออออออออออออออออออออออออออออออน');
    gotoxy(5,9);  Write('บ                                           บ');
    gotoxy(5,10); Write('บ       A j tk sor n fentrl lefel        บ');
    gotoxy(5,11); Write('บ  vletlenszer๛ alakzatok fognak potyogni. บ');
    gotoxy(5,12); Write('บ      A cl az hogy ฃgy kell felpกteni    บ');
    gotoxy(5,13); Write('บ         az elemeket, hogy a sor           บ');
    gotoxy(5,14); Write('บ         csak azokat tartalmazza.          บ');
    gotoxy(5,15); Write('บ                                           บ');
    gotoxy(5,16); Write('บ                                           บ');
    gotoxy(5,17); Write('บ                                           บ');
    gotoxy(5,18); Write('บ               ');textcolor(10);Write(' ฏ');textcolor(7);Write(' Vissza');textcolor(15);Write('                   บ');
    gotoxy(5,19); Write('บ                                           บ');
    gotoxy(5,20); Write('บ                                           บ');
    gotoxy(5,21); Write('บ                                           บ');
    gotoxy(5,22); Write('ศอออออออออออออออออออออออออออออออออออออออออออผ');

    textbackground(0); textcolor(7);
    gotoxy(52,7);  Write('  _______   _        _     ');
    gotoxy(52,8);  Write(' |__   __| | |      (_)    ');
    gotoxy(52,9);  Write('    | | ___| |_ _ __ _ ___ ');
    gotoxy(52,10); Write('    | |/ _ \ __| ''__| / __|');
    gotoxy(52,11); Write('    | |  __/ |_| |  | \__ \');
    gotoxy(52,12); Write('    |_|\___|\__|_|  |_|___/');

    textbackground(7);
    for i:=6 to 23 do
      begin
        gotoxy(50,i); Write(' ');
      end;

    for i:=6 to 50 do
      begin
        gotoxy(i,23); Write(' ');
      end;
    animacio('้tmutatข',25,7,12,9);
    sarokba;
    ReadLn;
    menu;
  end;
  4:begin
    ClrScr;
    textbackground(9); textcolor(15);
    gotoxy(5,5);  Write('ษอออออออออออออออออออออออออออออออออออออออออออป');
    gotoxy(5,6);  Write('บ                                           บ');
    gotoxy(5,6);  Write('บ                                           บ');
    gotoxy(5,7);  Write('บ                                           บ');
    gotoxy(5,8);  Write('ฬอออออออออออออออออออออออออออออออออออออออออออน');
    gotoxy(5,9);  Write('บ                                           บ');
    gotoxy(5,10); Write('บ    Verzข: 1.0                             บ');
    gotoxy(5,11); Write('บ                                           บ');
    gotoxy(5,12); Write('บ    Tesztelk: Dani, Joni                  บ');
    gotoxy(5,13); Write('บ               M rkข, Ulbi                 บ');
    gotoxy(5,14); Write('บ                                           บ');
    gotoxy(5,15); Write('บ    D tum: 2014.07.13                      บ');
    gotoxy(5,16); Write('บ                                           บ');
    gotoxy(5,17); Write('บ    Programot กrta: ');textcolor(lightgreen);Write('ipko');textcolor(white); Write('                   บ');
    gotoxy(5,18); Write('บ                                           บ');
    gotoxy(5,19); Write('บ                                           บ');
    gotoxy(5,20); Write('บ               ');textcolor(10);Write(' ฏ');textcolor(7);Write(' Vissza');textcolor(15);Write('                   บ');
    gotoxy(5,21); Write('บ                                           บ');
    gotoxy(5,22); Write('ศอออออออออออออออออออออออออออออออออออออออออออผ');

    textbackground(0); textcolor(7);
    gotoxy(52,7);  Write('  _______   _        _     ');
    gotoxy(52,8);  Write(' |__   __| | |      (_)    ');
    gotoxy(52,9);  Write('    | | ___| |_ _ __ _ ___ ');
    gotoxy(52,10); Write('    | |/ _ \ __| ''__| / __|');
    gotoxy(52,11); Write('    | |  __/ |_| |  | \__ \');
    gotoxy(52,12); Write('    |_|\___|\__|_|  |_|___/');

    textbackground(7);
    for i:=6 to 23 do
      begin
        gotoxy(50,i); Write(' ');
      end;

    for i:=6 to 50 do
      begin
        gotoxy(i,23); Write(' ');
      end;
    animacio('Adatok',25,7,12,9);
    sarokba;
    ReadLn;
    menu;
end;

  5:begin
  ClrScr;
  mondat:='E g y   l m  n y   v o l t : D  ! ! !  Szia';
  i:=1;
  while (not keypressed) and (i<length(mondat)) do begin
    gotoxy(4+i,5); Write(mondat[i]); inc(i); delay(20);

  end;
  i:=1;
  while (i<=40) and (not keypressed) do
    begin
      textcolor(random(14)+1);
      ClrScr;
      gotoxy(5,5);
      Write('E g y   l m  n y   v o l t : D  ! ! ! Szia'); Delay(100);
      inc(i);
    end;
  i:=48;
  textcolor(7);
  while (not keypressed) and (i<>0) do begin

  gotoxy(i,5); Write(' '); dec(i); delay(30);
  end;

  valaszt:=6;
end;

  end;

end;
  delay(50);
  gotoxy(5,5); Write('                                            ');
  ipko;
  delay(1000);
end.
