-- drawisometricgrid.lua
w,h=getpicturesize();
for y=0,h-1,8 do
  for x=0,w-1,1 do
    putpicturepixel(x,y+(x/2)%8,1);
    end;end
for y=0,h-1,8 do
  for x=0,w-1,1 do
    putpicturepixel(x+3,y+7-((x/2)%8),1);
    end;end
