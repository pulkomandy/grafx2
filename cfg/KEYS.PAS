var
    Touche:word;

procedure Get_Key; assembler;
asm

    mov  Touche,0


    mov  ah,11h
    int  16h



    jz   @Get_input_Pas_de_touche


    mov  ah,10h
    int  16h


    mov  byte ptr[Touche],ah


    mov  ah,02h
    int  16h


    and  al,00001111b


    mov  ah,al
    and  ah,00000001b
    shr  al,1
    or   al,ah


    mov  byte ptr[Touche+1],al

    @Get_input_Pas_de_touche:
end;

const HEXA_ORDER:string='0123456789ABCDEF';
function Byte2Hexa(Valeur:byte):string;
var Temp:string;
begin
  Temp:='';
  while length(Temp)<2 do
  begin
    Temp:=HEXA_ORDER[(Valeur mod 16)+1]+Temp;
    Valeur:=Valeur div 16;
  end;
  Byte2Hexa:=Temp;
end;

BEGIN
  repeat
    Get_Key;
    if (Touche<>0)
    then write('(',hi(Touche),',',Byte2Hexa(lo(Touche)),') ');
  until Touche=$0001;
END.
