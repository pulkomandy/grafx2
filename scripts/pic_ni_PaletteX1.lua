-- x1 palette to picture
w,h=getpicturesize();
for y1=0,7,1 do
  for x1=0,31,1 do
    putpicturepixel(x1,y1,y1+x1*8)
    end;end
