--BRUSH Remap: Grayscale (average)
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/


w, h = getbrushsize()

for x = 0, w - 1, 1 do
 for y = 0, h - 1, 1 do
   
   r, g, b = getcolor(getbrushpixel(x,y))

   a = (r+g+b)/3

   putbrushpixel(x, y, matchcolor(a,a,a));

 end
end
