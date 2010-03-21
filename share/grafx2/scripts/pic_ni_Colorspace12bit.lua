-- 12bit colour space from palette
--

-- Copyright 2010 Paulo Silva
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

w,h=getpicturesize();
for y1=0,3,1 do
  for x1=0,3,1 do
    for y2=0,15,1 do
      for x2=0,15,1 do
        putpicturepixel(x1*16+x2,y1*16+y2,matchcolor((y2*255)/15,
((y1*4+x1)*255)/15,(x2*255)/15))
        end;end;end;end
