-- drawgrid8x8.lua
w,h=getpicturesize();
for y=0,h-1,1 do
  for x=0,w-1,8 do
    putpicturepixel(x,y,1);
    end;end
for y=0,h-1,8 do
  for x=0,w-1,1 do
    putpicturepixel(x,y,1);
    end;end
