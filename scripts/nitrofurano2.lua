-- x8palettetopicture.lua
setpicturesize(256,64);
for y1=0,7,1 do
  for x1=0,31,1 do
    for y2=0,7,1 do
      for x2=0,7,1 do
        putpicturepixel(x1*8+x2,y1*8+y2,y1+x1*8)
        end;end;end;end
