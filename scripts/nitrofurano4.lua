-- drawgrid8x8matchcolor.lua
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
