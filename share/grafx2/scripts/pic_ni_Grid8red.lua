-- Draw red 8x8 grid

-- Copyright 2010 Paulo Silva
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

w,h=getpicturesize();
c=matchcolor(0xFF,0x00,0x00)
for y=0,h-1,1 do
  for x=0,w-1,8 do
    putpicturepixel(x,y,c);
    end;end
for y=0,h-1,8 do
  for x=0,w-1,1 do
    putpicturepixel(x,y,c);
    end;end
