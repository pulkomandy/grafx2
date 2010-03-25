-- Glass grid filter

-- Copyright 2010 Paulo Silva
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

w,h=getpicturesize();
for y1=0,h-1,8 do
  for x1=0,w-1,8 do
    for y2=0,3,1 do
      for x2=0,7,1 do
        c1=getpicturepixel(x1+x2,y1+y2)
        c2=getpicturepixel(x1+7-x2,y1+7-y2)
        putpicturepixel(x1+x2,y1+y2,c2)
        putpicturepixel(x1+7-x2,y1+7-y2,c1)
        end;end;end;end
