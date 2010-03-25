--PICTURE: Rainbow - Dark to Bright
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/
-- Email: dawnbringer@hem.utfors.se
-- MSN:   annassar@hotmail.com
--
-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

-- This script was adopted from Evalion, a Javascript codecrafting/imageprocessing project
-- http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 
--

--
function shiftHUE(r,g,b,deg) -- V1.3 R.Fhager 2007, adopted from Evalion
 local c,h,mi,mx,d,s,p,i,f,q,t
 c = {g,b,r}
 mi = math.min(r,g,b)
 mx = math.max(r,g,b); v = mx;
 d = mx - mi;
 s = 0; if mx ~= 0 then s = d/mx; end
 p = 1; if g ~= mx then p = 2; if b ~= mx then p = 0; end; end
 
 if s~=0 then
  h=(deg/60+(6+p*2+(c[1+p]-c[1+(p+1)%3])/d))%6;
  i=math.floor(h);
  f=h-i;
  p=v*(1-s);
  q=v*(1-s*f);
  t=v*(1-s*(1-f));
  c={v,q,p,p,t,v}
  r = c[1+i]
  g = c[1+(i+4)%6]
  b = c[1+(i+2)%6]
 end

 return r,g,b
end
--


w, h = getpicturesize()

for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

   -- Fractionalize image dimensions
   ox = x / w;
   oy = y / h;

   r = 255 * math.sin(oy * 2) 
   g = (oy-0.5)*512 * oy
   b = (oy-0.5)*512 * oy

   r, g, b = shiftHUE(r,g,b,ox * 360); 
 
   c = matchcolor(math.max(0,math.min(255,r)),math.max(0,math.min(255,g)),math.max(0,math.min(255,b)))
 
   putpicturepixel(x, y, c);

  end
end


