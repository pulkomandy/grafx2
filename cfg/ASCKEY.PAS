var touche:byte;
begin
  repeat
    asm
      mov ah,8
      int 21h
      mov touche,al
    end;
    write(touche,' ');
  until touche=27
end.