-- Picture distortion: Sine

w, h = getpicturesize()

frq = 2
amp = 0.3


for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

    ox = x / w;
    oy = y / h;
    ox = (1 + ox + math.sin(oy*math.pi*frq)*amp) % 1;
   
    c = getbackuppixel(math.floor(ox*w),y);
    --c = y % 16
    putpicturepixel(x, y, c);

  end
end
