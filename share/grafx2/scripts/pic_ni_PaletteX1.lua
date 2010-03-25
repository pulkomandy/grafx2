-- x1 palette to picture

-- Copyright 2010 Paulo Silva
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

w,h=getpicturesize();
for y1=0,7,1 do
  for x1=0,31,1 do
    putpicturepixel(x1,y1,y1+x1*8)
    end;end
