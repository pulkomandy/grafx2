--BRUSH Distortion: Waves v1.0
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- This script was adopted from Evalion, a Javascript codecrafting/imageprocessing project
-- http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 


--frq = 2
--amp = 0.3

-- Adjust power of frequency & amplitude
frq_adj = 2
amp_adj = 0.02

ok,frq,amp = inputbox("Settings",
                      "Frequency 1-10", 3, 1,10,0,
                      "Amplitude 1-10", 3, 1,10,0
);

w, h = getbrushsize()

for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

    ox = x / w;
    oy = y / h;
    ox = (1 + ox + math.sin(oy*math.pi*frq*frq_adj)*amp*amp_adj) % 1;
   
    c = getbrushbackuppixel(math.floor(ox*w),y);
    putbrushpixel(x, y, c);

  end
end

