--BRUSH Scene: Amigaball 1.0
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- This script was adopted from Evalion, a Javascript codecrafting/imageprocessing project
--http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 



w, h = getbrushsize()

for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

   -- Fractionalize image dimensions
   ox = x / w;
   oy = y / h;

   -- Ball
   Xr = ox-0.5; Yr = oy-0.5; 
   W = (1 - 2*math.sqrt(Xr*Xr + Yr*Yr)); 

   -- 'FishEye' distortion / Fake 3D
   F = (math.cos((ox-0.5)*math.pi)*math.cos((oy-0.5)*math.pi))*0.65;
   ox = ox - (ox-0.5)*F; 
   oy = oy - (oy-0.5)*F; 

   -- Checkers
   V = ((math.floor(0.25+ox*10)+math.floor(1+oy*10)) % 2) * 255 * W;

   -- Specularities
   SPEC1 = math.max(0,(1-5*math.sqrt((ox-0.45)*(ox-0.45)+(oy-0.45)*(oy-0.45)))*112);
   SPEC2 = math.max(0,(1-15*math.sqrt((ox-0.49)*(ox-0.49)+(oy-0.48)*(oy-0.48)))*255);

   r = W * 255 + SPEC1 + SPEC2
   g = V + SPEC1 + SPEC2
   b = V + SPEC1 + SPEC2

   putbrushpixel(x, y, matchcolor(r,g,b));

  end
end
