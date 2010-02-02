-- 15bitcolourspacefrompalette.lua
w,h=getpicturesize();
for y1=0,3,1 do
  for x1=0,7,1 do
    for y2=0,31,1 do
      for x2=0,31,1 do
        putpicturepixel(x1*32+x2,y1*32+y2,matchcolor((y2*255)/31,
((y1*8+x1)*255)/31,(x2*255)/31))
        end;end;end;end
