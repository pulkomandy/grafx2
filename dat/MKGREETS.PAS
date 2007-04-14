program mkgreets;

const
  NB_MAX_LIGNES=50;
  TAILLE_MAX_NICK=14;

var
  Fichier    : text;
  Greets     : array[0..3,0..NB_MAX_LIGNES-1] of string[TAILLE_MAX_NICK];
  Total_nicks: word;

{-----------------------------}

procedure Init_greets;
var
  i,j:word;
begin
  for i:=0 to 3 do
    for j:=0 to NB_MAX_LIGNES-1 do
      Greets[i,j]:='';
end;

{-----------------------------}

procedure Compter_greets;
var
  Temp:String;
begin
  Total_nicks:=0;
  while not eof(Fichier) do
  begin
    readln(Fichier,Temp);
    inc(Total_nicks);
  end;
end;

{-----------------------------}

procedure Lire_greets;
var
  Nb_nicks:word;
begin
  Nb_nicks:=0;
  while not eof(Fichier) do
  begin
    readln(Fichier,Greets[Nb_nicks div Total_nicks, Nb_nicks mod Total_nicks]);
    inc(Nb_nicks);
  end;
end;

{-----------------------------}

procedure Ecrire_greets;
var
  i:word;
begin
  writeln(Fichier,'{');
  writeln(Fichier,'  Ce fichier contient la partie "Greetings ..." de l''aide de GRAFX 2');
  writeln(Fichier,'}');
  writeln(Fichier,'');
  writeln(Fichier,'procedure Generer_l_aide_Greetings;');
  writeln(Fichier,'begin');
  writeln(Fichier,'  Demarrer_section_d_aide;');
  writeln(Fichier,'                        {XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}');
  writeln(Fichier,'                        {XXXXXXXXXXXXXXXXXXXXXX}');
  writeln(Fichier,'  Ecrire_ligne_d_aide(T,''GREETINGS:'');');
  writeln(Fichier,'  Ecrire_ligne_d_aide(N,'''');');
  writeln(Fichier,'  Ecrire_ligne_d_aide(N,''Our best regards go to...'');');
  writeln(Fichier,'  Ecrire_ligne_d_aide(N,'''');');

  for i:=0 to Total_nicks-1 do
  begin
    write(Fichier,'  Ecrire_ligne_d_aide(N,''  ',Greets[0,i]);
    if (Greets[1,i]<>'') then
      write(Fichier,' ':TAILLE_MAX_NICK-length(Greets[0,i]),Greets[1,i]);
    if (Greets[2,i]<>'') then
      write(Fichier,' ':TAILLE_MAX_NICK-length(Greets[1,i]),Greets[2,i]);
    writeln(Fichier,''');');
  end;

  writeln(Fichier,'  Ecrire_ligne_d_aide(N,''  and all #pixel, #demofr and #coders.'');');
  writeln(Fichier,'                        {XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}');
  writeln(Fichier,'                        {XXXXXXXXXXXXXXXXXXXXXX}');
  writeln(Fichier,'  Terminer_section_d_aide;');
  writeln(Fichier,'end;');
end;

{-----------------------------}

begin
  Init_greets;
  assign(Fichier,'greets.txt');
  reset(Fichier);
  Compter_greets;
  Total_nicks:=(Total_nicks+2) div 3;

  reset(Fichier);
  Lire_greets;
  close(Fichier);

  assign(Fichier,'hlp_gret.pas');
  rewrite(Fichier);
  Ecrire_greets;
  close(Fichier);
end.
