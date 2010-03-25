--BRUSH Remap: Grayscale (desaturate)
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- This script was adopted from Evalion, a Javascript codecrafting/imageprocessing project
--http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 


percent = 100

--
function desaturate(percent,r,g,b) -- V1.0 by Richard Fhager
 p = percent / 100
 a = (math.min(math.max(r,g,b),255) + math.max(math.min(r,g,b),0)) * 0.5 * p
 r = math.min(math.max(r + (a-r*p),0),255) -- Capping may not be needed if mathcolor/setcolor is updated
 g = math.min(math.max(g + (a-g*p),0),255)
 b = math.min(math.max(b + (a-b*p),0),255)
 return r,g,b
end
--


w, h = getbrushsize()

for x = 0, w - 1, 1 do
 for y = 0, h - 1, 1 do
   putbrushpixel(x, y, matchcolor(desaturate(percent,getcolor(getbrushpixel(x,y)))));
 end
end
