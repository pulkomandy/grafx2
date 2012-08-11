--DawnBringer function library v1.1
--** THIS IS NOT A RUNNABLE SCRIPT! **
--by Richard Fhager 
-- http://hem.fyristorg.com/dawnbringer/
-- Email: dawnbringer@hem.utfors.se
-- MSN:   annassar@hotmail.com
--
-- Many functions in here was adopted from Evalion, a Javascript codecrafting/imageprocessing project
-- http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 
--
--
-- You may access these functions in your own scripts by loading this library,
-- just add the follwing line as one of the first instructions:
--
-- run("dawnbringer_lib")
--
-- If the script is not in the same directory, use a relative path
-- like run("../libs/dawnbringer_lib.lua")
--
-- Note that the functions must be called with the full library object-name, "db.function_name..."
--

-- Global library object
db = {} 


-- *************************************
-- ***      Text & Conversions       ***
-- *************************************
--
--

function db.rgb2HEX(r,g,b,prefix)
  local c,n,s,t,z
  c = {r,g,b}
  z = {"0",""}
  t = ""
  for n = 1, 3, 1 do
  s = string.upper(string.format("%x",c[n]))
  t = t..z[#s]..s
     --s = tonumber(c[n],16)
     --t = t..s
  end
  return prefix..t
end


--
-- ... eof Text & Conversions ...
--



-- *************************************
-- ***    Custom Math Functions     ***
-- *************************************
--
--
function db.sign(v)
    local s
    s = 0
    if v > 0 then s = 1; end
    if v < 0 then s = -1; end
    return s 
end
--

--
function db.rotation (rot_ang,hub_x,hub_y,x,y) -- Rotate coordinates x & y relative hub
  local new_ang,dist,m,xd,yd,v; m = math 
  xd=hub_x-x; 
  yd=hub_y-y;
  if (not(xd==0 and yd==0)) then
   v = -90; if xd < 0 then v = 90; end
   new_ang = m.atan(yd/xd) - (v+rot_ang) * m.pi/180;
   dist = m.sqrt(xd*xd+yd*yd);
   x = hub_x - m.sin(new_ang)*dist;
   y = hub_y + m.cos(new_ang)*dist;
  end
  return math.floor(x),math.floor(y) -- For drawing purposes
end
--

--
-- ... eof Custom Math Functions ...
--

-- *************************************
-- ***     Fractional Scenery        ***
-- *************************************

--
function db.setSceneryPalette()
 db.colorCigarr(10,28,true) -- 250 colors
 setcolor(250, 208,48,48)
 setcolor(251, 48,208,48)
 setcolor(252, 48,48,208)
 setcolor(253, 224,224,64)
 setcolor(254, 224,64,224)
 setcolor(255, 64,224,224)
end
--

--
function db.star(xf,yf,sx,sy,rgb,haz,out,lum)
 local n,c,dist; c={}
 dist = haz + out * math.sqrt((xf-sx)^2+(yf-sy)^2);
 for n = 1, 3, 1 do c[n] = (rgb[n] * lum) / dist; end
 return c;
end
--

--
function db.zoom(xf,yf,zoom,panx,pany) -- Zoom and Pan in a fractional coord-system
  xf = (xf-0.5)/zoom + 0.5 + panx;
  yf = (yf-0.5)/zoom + 0.5 + pany;
  return xf,yf
end
--

--
function db.rotationFrac(rot_ang,hub_x,hub_y,x,y) -- Rotate coordinates x & y relative hub
  local new_ang,dist,m,xd,yd,v; m = math 
  xd=hub_x-x; 
  yd=hub_y-y;
  if (not(xd==0 and yd==0)) then
   v = -90; if xd < 0 then v = 90; end
   new_ang = m.atan(yd/xd) - (v+rot_ang) * m.pi/180;
   dist = m.sqrt(xd*xd+yd*yd);
   x = hub_x - m.sin(new_ang)*dist;
   y = hub_y + m.cos(new_ang)*dist;
  end
  return x,y
end
--

--
function db.twirl(x,y,arms,trot,tpow,tang)
 local b,ang,vx,vy,vr,m,deg,tw
 m=math; deg=math.pi/180; tw=.5;
 if (not(x==.5 and y==.5)) then
  ang = m.atan((.5-y)/(.5-x)); 
  b = 0; if (x>.5) then b = m.pi; end
  vx = .5-x; vy = .5-y; vr = m.pow(m.sqrt(vx*vx+vy*vy),tpow);
  tw = .5+m.sin(-tang*deg+vr*trot+(ang + b)*arms)*.5;
 end
 return tw;
end
--

--- Alpha filters
--
function db.alpha1(x,y,amp) -- Coord, Amplify: 0..n
 local p,a,xh,yh,m
 xh=0.5-x; yh=0.5-y; m = math
 p = m.pow(xh*xh+yh*yh,0.7);
 a = m.cos(32*m.pi*p)*m.sin(8*m.pi*(xh+yh));
 return 1 + (a * amp)
end
--

--
-- ... eof Fractional Scenery ...
--

-- *************************************
-- ***    Custom Array Functions     ***
-- *************************************
--
-- Ok, I don't know Lua that well (still unsure about some scopes & refs etc.)
-- And some features may not be active in Grafx2. So, some of the follwing functions
-- may exist in Lua/Grafx2...but since I'm not sure if and how they work - I'll prefer
-- to add a set of my own of known performance.

--
function db.newArrayInit2Dim(xs,ys,val) 
  local x,y,ary; ary = {}
  for y = 1, ys, 1 do
   ary[y] = {}
   for x = 1, xs, 1 do
     ary[y][x] = val
   end
  end
  return ary
end
--

--
-- Merge two arrays into a NEW one: array_c = db.newArrayMerge(array_b,array_b) 
--
function db.newArrayMerge(a,b) 
  local n,ary; ary = {}
  for n = 1, #a, 1 do
   ary[n] = a[n]
  end
  for n = 1, #b, 1 do
   ary[n+#a] = b[n]
  end
  return ary
end
--

--
-- Generate a copy of an array with a new value added Last
--
function db.newArrayInsertLast(a,val) 
  local n,ary; ary = {}
  for n = 1, #a, 1 do
   ary[n] = a[n]
  end
  ary[#a+1] = val
  return ary
end
--

--
-- Generate a copy of an array with a new value added First
--
function db.newArrayInsertFirst(a,val) 
  local n,ary; ary = {}
  ary[1] = val
  for n = 2, #a+1, 1 do
   ary[n] = a[n-1]
  end
  return ary
end
--

--
function db.ary2txt(ary) -- One & two dimensions supported [a,b] -> "a,b". [[a,b],[c,d]] -> "a-b, c-d"
 local t,n,m,v
 t = ""
 for n = 1, #ary, 1 do
   if type(ary[n]) == "table" then 
     t = t..ary[n][1]
     for m = 2, #ary[n], 1 do
       t = t.."-"..ary[n][m]
     end
   else t = t..ary[n]; 
   end
   t = t..", "
 end
 return t 
end
--


--
-- ... eof Custom Array Functions ...
--


-- *************************************
-- ***   Misc. Logical Operations    ***
-- *************************************

--
-- palList [r,g,b,palindex] is expected only to contain unique colors 
-- index = -1 --> index of list
--
function db.makeIndexList(list,index)
  local n,ilist
  ilist = {}
  for n = 1, #list, 1 do
   if (index > 0) then ilist[n] = list[n][index]; end
   if (index == -1) then ilist[n] = n; end
  end
  return ilist
end
--

--
-- Return a list of all possible (non-same) pairs from the entries in a list 
-- [a,b,c] --> [[a,b],[a,c],[b,c]]
-- (All entries are treated as unique. i.e it's only the INDEX that counts)
-- mode = 0: Only unique pairs (m = (n^2 - n)/2), [a,b] --> [[a,b]]
-- mode = 1: All pairs, i.e mirror versions as well. (m = n^2 - n), [a,b] --> [[a,b], [b,a]]
--
function db.pairsFromList(list,mode)
 local a,b,l,n,pairs
 pairs = {}
 l = #list
 n = 1
 for a = 1, l, 1 do
   for b = a+1, l, 1 do
     pairs[n] = {list[a],list[b]}; n = n + 1
     if mode == 1 then pairs[n] = {list[b],list[a]}; n = n + 1; end
   end
 end
 return pairs
end
--

function db.valueInArray(ary,val)
 local n,res
 res = false
 for n = 1, #ary, 1 do
   if ary[n] == val then res = true; break; end
 end
 return res
end

-- RAMP specific

-- Remove initial pair (palList) colors from pallist
function db.initiateRamp(pair,pallist,pal_index)
  local n,found,plist
  plist = {}

  found = 1 
  for n = 1, #pallist, 1 do
   if db.valueInArray(pair,pallist[n]) == false then
    plist[found] = pallist[n]; found = found + 1;
   end
  end

  pair[pal_index] = plist -- ex: ["pal"]

  return pair -- Is now a 2 color RAMP
end
--

-- Remove new col entry from ramp's pallist and add it to the ramp, returns an updated ramp
-- RampList = [1,2] ["pal"] = palList = [3,4,5], addindex = 3
-- --> [1,2,3] palList = [4,5]
function db.updateRamp(ramp,addindex,pal_index) 
  local n,found,pallist,plist,newramp
  plist = {}
  pallist = ramp[pal_index]

  -- New palList without added color to IndexList
  found = 1 
  for n = 1, #pallist, 1 do
   if pallist[n] ~= addindex then
    plist[found] = pallist[n]; found = found + 1;
   end
  end

  newramp = db.newArrayInsertLast(ramplist,addindex) 
  newramp[pal_index] = plist
 
  return rlist
end

--
-- Returns a list of all inital ramps from color pairs
--
-- Weeds out bad pairs, attaches remaining palette colors and the first rgb-vector
--
--
function db.initiateRampList(pairs,pallist,pal_index,vec_index,min,maxmult,rw,gw,bw) 
 local n,ramplist,newpairs,accept,dist,c1,c2,max,rD,gD,bD
 ramplist = {}
 max = min + (142 / math.sqrt(#pallist)) * maxmult -- min ex: 8-12
 accept = 0

 for n = 1, #pairs, 1 do
  c1 = pallist[pairs[n][1]]
  c2 = pallist[pairs[n][2]]
  rD = c2[1] - c1[1]
  gD = c2[2] - c1[2]
  bD = c2[3] - c1[3]
  dist = math.sqrt( (rw*rD)^2 + (gw*gD)^2 + (bw*bD)^2 ) 

  if dist >= min and dist <= max then
    accept = accept + 1; ramplist[accept] = db.initiateRamp(pairs[n],pallist,pal_index);
    ramplist[accept][vec_index] = {rD, gD, bD, dist}; -- Add first color vector, ONLY KEEP DISTANCE?
  end
 end

 return ramplist
end


function db.findRampExpansionColors(ramp)
 local clist
 clist = {}
 -- Calculate vectors here?
 return clist
end


function db.findRAMPS(min_len, max_len)
 local i,n,c,pallist,ramp,ramplist,pairs,spairs,palindex,vecindex,found,donelist,newlist,dones
 local colorlist
 palindex = "pal"
 vecindex = "vector"
 pallist =  db.fixPalette(db.makePalList(256), 0)
 pairs =    db.pairsFromList(db.makeIndexList(pallist,-1), 0)
 ramplist = db.initiateRampList(pairs,pallist,palindex,vecindex, 8,0.75, 0.26,0.55,0.19) 

 -- MIN_LEN = 5
 -- MAX_LEN = 10

 -- Split Ramp-build into two parts: 
 -- 1. Build ramps >= MIN_LEN, NONE added to 'Done'
 -- 2. Run til no more ramps can be expanded or reaches MAX_LEN, ALL ramps added to 'Done'

 for i = 1, (min_len - 2), 1 do -- Assuming 2 for inital pairs (2 color ramps)
  newlist = {}
  found = 0
  for n = 1, #ramplist, 1 do
    ramp = ramplist[n]
    colorlist = db.findRampExpansionColors(ramp) -- Colors that can split the current ramp into new expanded ramps
    for c = 1, #colorlist, 1 do
     found = found + 1; newlist[found] = db.updateRamp(ramp,colorlist[c],palindex); -- Ramp is expanded by 1 color
    end
  end
  ramplist = newlist
 end


 donelist = {}; dones = 0

 repeat
  newlist = {}
  found = 0
  for n = 1, #ramplist, 1 do
    ramp = ramplist[n]
    if true == false then
     found = found + 1; newlist[found] = db.updateRamp(ramp,color,palindex); 
      else
      dones = dones + 1; donelist[dones] = ramp;
    end
  end
  --ramplist = newlist
 until found == 0

 return #pairs.." - "..#ramplist
end

--
-- ... eof Misc. Logical Operations ...
--


-- ***************************************
-- *** General RGB-Color Modifications ***
-- ***************************************


--
function db.makeComplimentaryColor(r,g,b,brikeeplev) -- Lev: 0 = Normal, 1 = Loose, 2 = Strict

 local bri_o,bri_n,bdiff

 function cap(v) return math.max(0,math.min(v,255)); end

 bri_o = db.getBrightness(r,g,b)
 r,g,b = db.shiftHUE(r,g,b,180)

 if brikeeplev > 0 then

  for n = 0, brikeeplev*3-1, 1 do -- Must iterate to reduce brightness error
    bri_n = db.getBrightness(r,g,b)
    bdiff = (bri_o - bri_n) / 2 * brikeeplev
    r = cap(r + bdiff)
    g = cap(g + bdiff)
    b = cap(b + bdiff)
  end

 end

 return r,g,b

end
--


-- *** Color balance ***
--
-- bri_flag:   Preserve brightness
-- loose_flag: Loose preservation restrictions for brightness and balance
--
-- Jeez, was this a tricky sucker; color-balance is just adding and capping...
-- but trying color-balance with preserved perceptual brightness is a different monster...
-- ...so bad I could only solve it by iterative error correction.
--
function db.ColorBalance(r,g,b,rd,gd,bd,bri_flag,loose_flag) -- preserve brightness
 local rw,gw,bw,ri,gi,bi,itot,rni,gni,bni,ro,go,bo,ovscale,lev,count,rt,gt,bt,rf,gf,bf,bri

 -- Dawn 3.0, [0.26,0.55,0.19], 0-255 bri-colorscale adjust = 1.56905
 rw,gw,bw = 0.26, 0.55, 0.19

 function cap(v) return math.min(255,math.max(v,0)); end
 
  bri = db.getBrightness(r,g,b)


 -- Loose brightness & balance preservation, a good compromise.
 if bri_flag == true and loose_flag == true then

   lev = (rd + gd + bd) / 3
   rd = rd - lev
   gd = gd - lev
   bd = bd - lev

   brin = db.getBrightness(cap(r+rd),cap(g+gd),cap(b+bd))
   itot = brin - bri
   rd = rd - itot
   gd = gd - itot
   bd = bd - itot

 end


 if bri_flag == true and loose_flag == false then

  itot = 255
  count = 0

   -- Normalize (Yup, it's right only to normalize once first..cont.norm. will have some counter-effect)
  lev = (rd + gd + bd) / 3
  rd = rd - lev
  gd = gd - lev
  bd = bd - lev

 repeat

  --messagebox("Norm:"..rd..", "..gd..", "..bd)

  -- Calculate total brightness change
  -- Note: Perceptual Brightness is exponential, and can't be delta-adjusted for anything other than greyscales.
  -- Although the formula for the new brightness corrected normalization level can can be derived...
  -- ...it doesn't do much good since the bigger problem is overflow outside the 0-255 boundary.
  -- As for now, I see no other means to solve this issue than with iterative error-correction.

  rt = r+rd 
  gt = g+gd
  bt = b+bd
 
  itot = 9e99
  rni = rd 
  gni = gd 
  bni = bd
 
  -- We can get brightness of negative values etc. So bri-correction is put on hold until values are scaled down
  if (rt>=0 and gt>=0 and bt>=0) and (rt<256 and gt<256 and bt<256) then
    brin = db.getBrightness(rt,gt,bt)
    itot = brin - bri
    --messagebox("Bri Diff: "..itot)
    -- Brightness adjusted balance
     rni = rd - itot
     gni = gd - itot
     bni = bd - itot
  end

  --messagebox("Bri Adj Bal:"..rni..", "..gni..", "..bni)

   -- Apply balance to find overflow (as fraction of the channel change)
   ro = math.max( math.max((r + rni)-255,0), math.abs(math.min((r + rni),0)) ) / math.max(math.abs(rni),1)
   go = math.max( math.max((g + gni)-255,0), math.abs(math.min((g + gni),0)) ) / math.max(math.abs(gni),1)
   bo = math.max( math.max((b + bni)-255,0), math.abs(math.min((b + bni),0)) ) / math.max(math.abs(bni),1)

  ovscale = 1 - math.max(ro,go,bo)

  -- Scaling balances might be logically incorrect (as they can be seen as constant differences)
  -- But scaling DOWN is quite harmless and I don't see how it could be done otherwise...
  -- ex: +10 red, +5 blue: Scale x2   = +20 red, +10 blue -> More red over blue than ordered, a contrast behaviour.
  --     +10 red, +5 blue: Scale x0.5 = +5 red, +2.5 blue -> Less of everything, but a part of the order. Harmless?
  --
  rd = rni * ovscale
  gd = gni * ovscale  
  bd = bni * ovscale

  count = count + 1 

  --messagebox("Final bal:"..rd..", "..gd..", "..bd)

 until math.abs(itot) < 1 or count > 5

 end 

 rf = r + rd
 gf = g + gd
 bf = b + bd

 --messagebox("Result color:"..rf..", "..gf..", "..bf)

 return rf,gf,bf
end
--



--
-- bri_flag: Preserve brightness
-- cap_flag: Cap new color at 0-255, has a desaturating effect for large values.
--
function db.ColorBalanceXXX(r,g,b,rd,gd,bd,bri_flag,cap_flag) -- preserve brightness
 local rf,gf,bf

 if cap_flag == true then
  rd = math.min(255,math.max(0, r+rd)) - r
  gd = math.min(255,math.max(0, g+gd)) - g
  bd = math.min(255,math.max(0, b+bd)) - b
 end

 local rw,gw,bw,ri,gi,bi,itot,rni,gni,bni,ro,go,bo,ovscale


 -- Dawn 3.0, [0.26,0.55,0.19], 0-255 bri-colorscale adjust = 1.56905
 rw,gw,bw = 0.26, 0.55, 0.19

 if bri_flag == true then

  -- Calculate total brightness change
  --ri = rd * rw
  --gi = gd * gw
  --bi = bd * bw
  --itot = math.sqrt(ri^2 + gi^2 + bi^2) 

  bri  = db.getBrightness(r,g,b)
  brin = db.getBrightness(r+rd,g+gd,b+bd)
  itot = brin - bri

 
  -- Normalized and Brightness adjusted balance
  rni = rd - itot
  gni = gd - itot
  bni = bd - itot

  -- Apply balance to find overflow (as fraction of the channel change)
  ro = math.max( math.max((r + rni)-255,0), math.abs(math.min((r + rni),0)) ) / math.max(math.abs(rni),1)
  go = math.max( math.max((g + gni)-255,0), math.abs(math.min((g + gni),0)) ) / math.max(math.abs(gni),1)
  bo = math.max( math.max((b + bni)-255,0), math.abs(math.min((b + bni),0)) ) / math.max(math.abs(bni),1)

  ovscale = 1 - math.max(ro,go,bo)

  rd = rni * ovscale
  gd = gni * ovscale  
  bd = bni * ovscale

 end 

 rf = r + rd
 gf = g + gd
 bf = b + bd

 return rf,gf,bf
end
--

--
function db.getContrast(ch) -- Channel, returns fraction -1..0..1, negative for ch < 127.5
 --return math.abs((ch / 127.5) - 1)
 return (ch / 127.5) - 1
end
--

--
function db.getAvgContrast(r,g,b)
 return (math.abs(db.getContrast(r)) + math.abs(db.getContrast(g)) + math.abs(db.getContrast(b))) / 3   
end
--

--
-- Mode = 0: Proportional - all colors reach max contrast at 100%
--
-- Mode = 1: Linear - percentage simply added
-- 
function db.changeContrastOLD(r,g,b,prc,mode)

 local m,rd,gd,bd,rv,gv,bv,rc,gc,bc,base,sign

 base = 1; sign = 1
 if prc < 0 then base = 0; sign = -1; end -- decontrast

 m = prc / 100 * sign

 -- mode 0 
 rc = db.getContrast(r)
 rd = (base - math.abs(rc)) * m  * db.sign(rc)
 rv = (rc+rd+1) * 127.5

 gc = db.getContrast(g)
 gd = (base - math.abs(gc)) * m  * db.sign(gc)
 gv = (gc+gd+1) * 127.5

 bc = db.getContrast(b)
 bd = (base - math.abs(bc)) * m  * db.sign(bc)
 bv = (bc+bd+1) * 127.5

 return rv,gv,bv

end
--

function db.changeContrast(r,g,b,prc) -- Photoshop style

 local m,rd,gd,bd,rv,gv,bv,rc,gc,bc

 m = 1 + math.pow((255 / 100 * prc),3) / (255*255)

 -- decontrast
 if prc < 0 then
  m = 1 - math.abs(prc)/100
 end 
  
 rc = db.getContrast(r)
 rd = rc * m 
 rv = (rd+1) * 127.5

 gc = db.getContrast(g)
 gd = gc * m 
 gv = (gd+1) * 127.5

 bc = db.getContrast(b)
 bd = bc * m 
 bv = (bd+1) * 127.5

 return rv,gv,bv

end



--
function db.getBrightness(r,g,b) -- 0-255
 local bri
 --bri = (r+g+b)/3
 --bri = r*0.3 + g*0.59 + b*0.11 -- Luma Y'601
 --bri = math.sqrt((r*0.3)^2 + (g*0.59)^2 + (b*0.11)^2) -- Luma Y'601
 --bri = r*0.245 + g*0.575 + b*0.18 -- Dawn 2.0

 bri = math.sqrt((r*0.26)^2 + (g*0.55)^2 + (b*0.19)^2) * 1.56905 -- Dawn 3.0
 return bri  
end
--


--
-- Note on desaturation: These functions are all junk, the only way to desaturate
--                       is to fade a color into it's corresponding greyscale.
--

--
function db.desaturate(percent,r,g,b) -- V1.0 by Richard Fhager
 local a,p
 p = percent / 100
 a = (math.min(math.max(r,g,b),255) + math.max(math.min(r,g,b),0)) * 0.5 * p
 r = r + (a-r*p) -- a+r*(1-p)
 g = g + (a-g*p)
 b = b + (a-b*p)
 return r,g,b
end
--

--
function db.desaturateA(percent,c) -- array version
 local r,g,b,a
 r = c[1]
 g = c[2]
 b = c[3]
 p = percent / 100
 a = (math.min(math.max(r,g,b),255) + math.max(math.min(r,g,b),0)) * 0.5 * p
 r = r + (a-r*p)
 g = g + (a-g*p)
 b = b + (a-b*p)
 return {r,g,b}
end
--

--
function db.desatAVG(desat,c) -- Desaturation, simpe average
 r = c[1]
 g = c[2]
 b = c[3]
 p = desat / 100
 a = (r+g+b)/3
 r = r + p*(a-r) 
 g = g + p*(a-g)
 b = b + p*(a-b) 
 return {r,g,b}
end
--


--
function db.getSaturation(r,g,b) -- HSL
  local M,m,c,s,l
  M = math.max(r,g,b)
  m = math.min(r,g,b)
  c = (M - m)/255
  s = 0
  if c ~= 0 then
    --l = (0.3*r + 0.59*g + 0.11*b)/255 -- HSLuma: Y'601
    l = (M+m)/510 -- This produces a quite "correct looking" divison of saturation
    if l <= 0.5 then s = c / (2*l); end
    if l  > 0.5 then s = c / (2-2*l); end
  end
  return math.min(255,s * 255)
end
--

--
function db.getTrueSaturationX(r,g,b) -- Distance from grayscale axis. Not HSV/HSL 
 local sat,bri
 bri = (r+g+b) / 3
 sat = math.min(255, math.sqrt((r-bri)^2 + (g-bri)^2 + (b-bri)^2) * 1.224744875)
 return sat
end
--

-- WIP. Trying to find a more natural model for estimating Saturation
-- Current: (HSL + True) / 2
function db.getAppSaturation(r,g,b)
  return  math.min(255, (db.getSaturation(r,g,b) + db.getTrueSaturationX(r,g,b)) / 2)
end
--

--
function db.saturate(percent,r,g,b) 
  local a,m,p,mc
  a = (math.min(math.max(r,g,b),255) + math.max(math.min(r,g,b),0)) * 0.5
  m = math.min(255-math.max(r,g,b), math.min(r,g,b))
  p = percent * (m / 100)
  mc = math.max((r-a),(g-a),(b-a)) -- Can this be derived elsewhere?
  if mc ~= 0 then
   r = r + (r-a) * p / mc
   g = g + (g-a) * p / mc
   b = b + (b-a) * p / mc
  end
  return r,g,b
end
--

--
-- Super Saturate: Better than Photoshop etc.
--
-- Higher than 100% power is ok 
--
function db.saturateAdv(percent,r,g,b,brikeeplev,greydamp) -- brikeep = 0 - 2 
  local a,m,p,mc,bri_o,bri_n,bdiff,mx,mi,adj,q,n
   function cap(v) return math.max(0,math.min(v,255)); end
  mx = math.max(r,g,b)
  mi = math.min(r,g,b)
  bri_o = db.getBrightness(r,g,b)
  a = (math.min(mx,255) + math.max(mi,0)) * 0.5
  m = math.min(255-mx, mi)
  p = percent * (m / 100)
  mc = math.max((r-a),(g-a),(b-a)) -- Can this be derived elsewhere?
  if mc ~= 0 and m ~= 0 then
   adj = math.min(1,(mx - mi) / m) -- Reduce effect on low saturation
   if greydamp == false then adj = 1; end
   q = p / mc * adj
   r = cap( r + (r-a) * q )
   g = cap( g + (g-a) * q )
   b = cap( b + (b-a) * q )
  end
  for n = 0, brikeeplev*2, 1 do -- Must iterate to reduce brightness error
    bri_n = db.getBrightness(r,g,b)
    bdiff = (bri_o - bri_n) / 2 * brikeeplev
    r = cap(r + bdiff)
    g = cap(g + bdiff)
    b = cap(b + bdiff)
  end
  return r,g,b
end
--


--
-- Lightness: Darken / Brighten color (Argument and returnvalue is a rgb-list)
--            Rate of change is inversely proportional to the distance of the max/min. 
--            i.e. all colors/channels will reach max/min at the same time (at 0 or 100 %)
--            (As opposed to 'Brightness' where all channels are changed by a constant value)
--
function db.lightness(percent,c)
 local v,r,g,b,p
 r = c[1]
 g = c[2]
 b = c[3]
 p = math.abs(percent/100)
 v = 255
 if percent < 0 then v = 0; end
 r = r + (v - r)*p
 g = g + (v - g)*p 
 b = b + (v - b)*p 
 return {r,g,b}
end
--

--
function db.changeLightness(r,g,b,percent)
 local v
 v = db.lightness(percent,{r,g,b})
 return v[1],v[2],v[3]
end
--

--
function db.getLightness(r,g,b) -- HSL bi-hexcone
  return (math.max(r,g,b) + math.min(r,g,b)) / 2
end
--

--
function db.shiftHUE(r,g,b,deg) -- V1.3 R.Fhager 2007, (Heavily derived code, hehe...)
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

--
function db.getHUE(r,g,b,greytol) -- 0-6 (6.5 = Greyscale), mult. with 60 for degrees
 -- 1 Color diff is roughly detected by Tolerance = 0.0078125 (Tol. incr. with lightness etc.)
 local c,h,mi,mx,d,s,p,i,f,q,t
 c = {g,b,r}
 mi = math.min(r,g,b)
 mx = math.max(r,g,b); v = mx;
 d = mx - mi;
 s = 0; if mx ~= 0 then s = d/mx; end
 p = 1; if g ~= mx then p = 2; if b ~= mx then p = 0; end; end

 h = 6.5 -- for custom graphical purposes
 if s>greytol then -- can't use >=
  h=(6+p*2+(c[1+p]-c[1+(p+1)%3])/d)%6; 
 end

 return h
end
--

--
-- ... eof RGB color modifications ...
--



-- ****************************************
-- *** Custom Color / Palette functions ***
-- ****************************************


--
function db.rgbcap(r,g,b,mx,mi)
 local m = math
 return m.max(mi,m.min(r,mx)), m.max(mi,m.min(g,mx)), m.max(mi,m.min(b,mx))
end
--

--
function db.makePalList(cols)
 local pal,n,r,g,b
 pal = {}
 for n = 0, cols-1, 1 do
   r,g,b = getcolor(n)
   pal[n+1] = {r,g,b,n}
 end
 return pal
end
--

--
function db.makeSparePalList(cols)
 local pal,n,r,g,b
 pal = {}
 for n = 0, cols-1, 1 do
   r,g,b = getsparecolor(n)
   pal[n+1] = {r,g,b,n}
 end
 return pal
end
--


--
-- Use to remove the black colors (marks unused colors) from palette-list
-- if it's known that no black color exists in the image.
function db.stripBlackFromPalList(pallist)
   local i,u,c,dummy; i = 257 -- Do 'nothing' If using a full 256 col palette with no blacks
   for u = 1, #pallist, 1 do
     c = pallist[u]
     if (c[1]+c[2]+c[3]) == 0 then i = u; end
   end
   dummy = table.remove(pallist,i)
   return pallist
end
--

--
function db.stripIndexFromPalList(pallist,colindex)
   local i,u,c,dummy
   for u = 1, #pallist, 1 do
     c = pallist[u]
     if c[4] == colindex then i = u; end
   end
   dummy = table.remove(pallist,i)
   return pallist
end
--

--
function db.addHSBtoPalette(pallist)
 local n,hue,sat,rgb
 for n=1, #pallist, 1 do
   rgb = pallist[n]
   pallist[n][5] =        db.getHUE(rgb[1],rgb[2],rgb[3],0)
   pallist[n][6] = db.getSaturation(rgb[1],rgb[2],rgb[3])
   pallist[n][7] = db.getBrightness(rgb[1],rgb[2],rgb[3])
 end
 return pallist -- {r,g,b,n,bri,hue,sat}
end
--

--
function db.makePalListRange(start,ends)
 local pal,n,r,g,b,a
 pal = {}
 a = 1
 for n = start, ends, 1 do
   r,g,b = getcolor(n)
   pal[a] = {r,g,b,n}; a = a + 1;
 end
 return pal
end
--


--
function db.makePalListShade(cols,sha) -- Convert colors to less bits, colorcube operations etc.
 local pal,n,r,g,b,mf,div
 mf = math.floor
 div = 256 / sha
 pal = {}
 for n = 0, cols-1, 1 do
   r,g,b = getcolor(n)
   pal[n+1] = {mf(r/div),mf(g/div),mf(b/div),n}
 end
 return pal
end
--
--
function db.makePalListShadeSPARE(cols,sha) -- Convert colors to less bits, colorcube operations etc.
 local pal,n,r,g,b,mf,div
 mf = math.floor
 div = 256 / sha
 pal = {}
 for n = 0, cols-1, 1 do
   r,g,b = getsparecolor(n)
   pal[n+1] = {mf(r/div),mf(g/div),mf(b/div),n}
 end
 return pal
end
--



--
function db.getColorDistance_weight(r1,g1,b1,r2,g2,b2,rw,gw,bw)
 return math.sqrt( (rw*(r1-r2))^2 + (gw*(g1-g2))^2 + (bw*(b1-b2))^2 ) 
end
--

--
function db.getBestPalMatch(r,g,b,pal,index_flag) -- pal = [r,g,b,palindex], index_flag -> return palindex if pal is sorted or reduced
 local diff,best,bestcol,cols,n,c,p
 cols = #pal
 bestcol = -1
 best = 9e99

 for n=1, cols, 1 do
  p = pal[n]
  diff = db.getColorDistance_weight(r,g,b,p[1],p[2],p[3],0.26,0.55,0.19)  * 1.569
  if diff < best then bestcol = n; best = diff; end
 end 

 if index_flag == true then
  bestcol = pal[bestcol][4] + 1 
 end

 return bestcol-1 -- palList index start at 1, image-palette at 0
end
--


-- Normally this function will return the (image)palette index of best color
-- ...but if the palette has been sorted with 'fixPalette' it will return the index
-- of the custom palList, setting index_flag will convert this value to image-palette index
--
-- HYBRID means the colormatch is a combo of color and (perceptual)brightness
--
--
function db.getBestPalMatchHYBRID(rgb,pal,briweight,index_flag) -- Now correctly balanced
 local diff,diffC,diffB,best,bestcol,cols,n,c,r,g,b,p,obri,pbri
 cols = #pal
 bestcol = -1
 best = 9e99

 --messagebox(briweight)

 -- Note: Not secured against negative values (this algorithm is SLOW, we cannot afford it)
 r = rgb[1]
 g = rgb[2] 
 b = rgb[3]

 obri = db.getBrightness(r,g,b) -- 0-255

 for n=1, cols, 1 do
  p = pal[n]
  pbri = db.getBrightness(p[1],p[2],p[3])
  diffB = math.abs(obri - pbri)
  -- we need to normalize the distance by the weights
  diffC = db.getColorDistance_weight(r,g,b,p[1],p[2],p[3],0.26,0.55,0.19)  * 1.569

  diff = briweight * (diffB - diffC) + diffC
  if diff < best then bestcol = n; best = diff; end
 end 

 if index_flag == true then
  bestcol = pal[bestcol][4] + 1 -- Since we detract 1 on return, God Lua is stupid 
 end

 return bestcol-1 -- palList index start at 1, image-palette at 0
end
--



--
-- Special version of Hybrid-remapping for mixPalette list
--
-- mixpal: {score,col#1,col#2,dist,rm,gm,bm, c1_r,c1_g,c1_b, c2_r,c2_g,c2_b}
--
-- returns: {col#1,col#2} (index of palette)
--
function db.getBestPalMatchHybridMIX(rgb,mixpal,briweight,mixreduction)
 local diff,diffC,diffB,best,bestcol,cols,n,c,r,g,b,p,obri,pbri, distmult
 cols = #mixpal
 bestcol = -1
 best = 9e99

 -- We will simply add the the distance to the mix with the distance between the mixcolors and
 -- employ a user tolerance to much the latter will matter.
  --distmult = 255 / 9.56 / 100 * mixreduction -- 16 shades
 distmult = 1.56902 / 100 * mixreduction  -- 24-bit, Dawn3.0 colormodel

 -- Note: Not secured against negative values (this algorithm is SLOW, we cannot afford it)
 r = rgb[1]
 g = rgb[2] 
 b = rgb[3]

 obri = db.getBrightness(r,g,b) -- 0-255

 for n=1, cols, 1 do
  p = mixpal[n]
  --pbri = db.getBrightness(p[5],p[6],p[7])

 -- *** DawnBringer's exponetial color brightness dither resolution phenomena theorem ***
 -- Bri = color value ^ 2
 -- Two adjacent pixels displayed with "normal high resolution" will NOT have the perceptual
 -- brightness of the resulting mixcolor. The brightness lies closer to that of the brightest pixel.
 -- Bri[(C1+C2)/2] = SQRT( (C1bri^2 + C2bri^2) / 2 )
 -- (Brightness according to Dawn-model: bri = SQRT( (r*.26)^2 + (g*.55)^2 + (b*.19)^2 ) )

  pbri = math.sqrt((db.getBrightness(p[8],p[9],p[10])^2 + db.getBrightness(p[11],p[12],p[13])^2) / 2)

  diffB = math.abs(obri - pbri)
  -- we need to normalize the distance by the weights
  diffC = db.getColorDistance_weight(r,g,b,p[5],p[6],p[7],0.26,0.55,0.19)  * 1.569 + p[4]*distmult

  diff = briweight * (diffB - diffC) + diffC
  if diff <= best then bestcol = n; best = diff; end
 end 

 return {mixpal[bestcol][2], mixpal[bestcol][3]}
--return {mixpal[bestcol][2], 0}



end
--



--
function db.matchcolorHSB(h,s,b,pallist,index_flag)
 --
 -- why don't we just convert HSB-diagram to RGB and do normal colormatching?
 -- Not the same...
 --
 local n,c,best,bestcol,pb,ph,ps,diff,huediff,huecorr,hue_adj,sat_adj,bri_adj
 bestcol = -1
 best = 9e99
 
 -- higher adjust means more impact (higher hue gives more interpolation )
 hue_adj = 4
 sat_adj = 0.075
 bri_adj = 2

 huecorr = 255 / 6 -- Our Hue goes from 0.0 - 5.999 

 for n=1, #pallist, 1 do
  c = pallist[n]
  ph = c[5]
  ps = c[6]
  pb = c[7]

  huediff = math.abs(h-ph*huecorr)
  if huediff > 127 then huediff = huediff - (huediff % 127) * 2; end

  --if ph == 6.5 then huediff = 0; end 

  -- With less saturation, exact hue becomes less important and brightness more usefull
  -- This allows for greyscales and low saturation colors to work smoothly.
  huediff = huediff * (ps /255)

  diff = hue_adj*huediff^2 + (s-ps)^2 * sat_adj + (b-pb)^2 * bri_adj
  
  if diff <= best then bestcol = n; best = diff; end
 end

 if index_flag == true then
  bestcol = palList[bestcol][4] + 1 -- Since we detract 1 on return, God Lua is stupid 
 end

 return bestcol-1

end
--

--
-- Used by PaletteAnalysis.lua, FindRamps(), MixColors()
--
function db.fixPalette(pal,sortflag) -- Arrange palette & only keep unique colors

 local n,l,rgb,i,unique,bri,hue,sat,ulist,indexpal,newpal,dtot
 ulist = {}
 indexpal = {}
 newpal = {}
  local doubles; doubles = {}

 l = #pal

 unique = 1 -- ok, see how stupid lua is
 dtot = 0
 for n=1, l, 1 do
   rgb = pal[n]; -- actually rgbn
   i = 1 + rgb[1] * 65536 + rgb[2] * 256 + rgb[3];
   bri = db.getBrightness(rgb[1],rgb[2],rgb[3])
   if indexpal[i] == nil then 
      indexpal[i] = rgb; ulist[unique] = {i,bri}; unique = unique+1;
      else
        doubles[rgb[4]] = true; -- Mark as double
        dtot = dtot + 1
   end
 end

 -- sort ulist
 if sortflag == 1 then db.sorti(ulist,2); end -- sort by brightness

 l = #ulist
 for n=1, l, 1 do
  newpal[n] = indexpal[ulist[n][1]]
 end

 newpal["doubles"] = doubles
 newpal.double_total = dtot

  --messagebox("unique colors", unique-1)

 return newpal

end
--

--
-- InsertionSort Array, this is chaos...I'm confused and stomped...don't understand how Lua works...
-- ...sorting seem be to ok but this code is ugly...
-- Sort LO-HI
--
-- Screwed up or confused thing here I think, perhaps lo-hi/hi-lo. This is working lo-hi but the code
-- looks like hi-lo...edit this some day
-- 
function db.sorti(d,idx) 
   local a,j,tmp,l,e
   l = #d

   for a=2, l, 1 do
    tmp = d[a];
    e = a
    for j=a, 2, -1 do  
      e = j
      if d[j-1][idx] > tmp[idx] then d[j] = d[j-1]; e = j-1; else break; end;
    end;
    d[e] = tmp; -- WHY THE F**K CAN'T YOU READ j HERE!?! STUPID ASSUCKING LANGUAGE 
      
   end;
   --return d
end
--

--
function db.drawColorspace12bit(x,y,cols,size)
 local r,g,b,c,rows,row,col,s16,rx,ry,xx,yy
 s16 = size*16
 rows = math.floor(16/cols)

 for g = 0, 15, 1 do
  col = g % cols
  row = math.floor(g / cols)
  for r = 0, 15, 1 do
   for b = 0, 15, 1 do
    c  = matchcolor(r*17,g*17,b*17)
    xx = x+col*s16+r*size
    yy = y+row*s16+b*size
     for ry = 0, size-1, 1 do
      for rx = 0, size-1, 1 do
       putpicturepixel(xx+rx,yy+ry,c)
     end;end
   end
  end
 end
end
--

--
function db.drawHSBdiagram(pallist,posx,posy,width,height,size,sat)
 --db.addHSBtoPalette(palList)
 local x,y,c
 for y = 0, height-1, 1 do
  for x = 0, width-1, 1 do
   hue = 255/width  * x
   bri = 255/height * y
   c = db.matchcolorHSB(hue,sat,bri,pallist,true)
   db.drawRectangle(posx + x*size, posy + y*size,size,size, c)
  end
 end
end
--

--
-- Histograms, remapping etc.
--

--
function db.makeHistogram() 
  local n,y,x,c,w,h,list; list = {}
  w, h = getpicturesize()
  for n = 1, 256, 1 do list[n] = 0; end
  for y = 0, h - 1, 1 do
    for x = 0, w - 1, 1 do
      c = getpicturepixel(x,y)
      list[c+1] = list[c+1] + 1
    end
  end
  return list
end
--

--
function db.makeSpareHistogram() 
  local n,y,x,c,w,h,list; list = {}
  w, h = getsparepicturesize()
  --w,h = 512,360
  for n = 1, 256, 1 do list[n] = 0; end
  for y = 0, h - 1, 1 do
    for x = 0, w - 1, 1 do
      c = getsparepicturepixel(x,y)
      list[c+1] = list[c+1] + 1
    end
  end
  return list
end
--


--
-- Makes a palette-list from only the colors (histogram) that occurs in the image
-- Assumes image/palette has not changed since histogram was created
function db.makePalListFromHistogram(hist) 
  local n,r,g,b,list,count
  list = {}
  count = 1
  for n = 1, #hist, 1 do
    if hist[n] > 0 then
      r,g,b = getcolor(n-1)
      list[count] = {r,g,b,n-1}
      count = count + 1
    end
  end
  return list
end
--

function db.makePalListFromSpareHistogram(hist) 
  local n,r,g,b,list,count
  list = {}
  count = 1
  for n = 1, #hist, 1 do
    if hist[n] > 0 then
      r,g,b = getsparecolor(n-1)
      list[count] = {r,g,b,n-1}
      count = count + 1
    end
  end
  return list
end
--


--
function db.remap(org) -- Working with a remap-list there's no need of reading backuppixel
  --messagebox("Remapping")
  local x,y,c,i,w,h,s,f,col
  f = getpicturepixel
  s = false
  w, h = getpicturesize()
  for y = 0, h - 1, 1 do
   for x = 0, w - 1, 1 do
    c = f(x,y)
    i = org[c+1]
    if i == null then i = matchcolor(getbackupcolor(getbackuppixel(x,y))); s = true; col = c; end -- Find color for a removed double
    putpicturepixel(x,y,i)
   end
  end
  if s then messagebox("Remapping: Not all image colors were found in remap-list (re-assign), probably due to duplicate removal. Matchcolor was used, ex: col# "..col); 
  end
end
--

--
-- Palette DeCluster: Color-reduction by fusing similar colors into new ones, using a desired tolerance.
--                    This is a method similar to Median-Cut, but more surgical.
--
-- pallist:   Palette list {r,g,b,palette_index}
-- hist:      Histogram {color 0 pixels, color 1 pixels...etc} always a full 256 color list
-- crad:      Cluster radius treshold in % of distance between black & white
--            A value of 0 will only remove identical colors
--            A value of 3-4 will usally fuse redundant colors without causing notice 
-- prot_pow:  (0..10) Protect common colors in histogram. Distances are increased by ocurrence. 
--            Also gives protection to fused colors even if not using histogram (combined nominal weights)
-- pixels:    Pixels in image (so protection can be calculated)
-- rw,gw,bw:  Color weights (rw+gw+bw = 1, 0.33,0.33,0.33 is nominal)
--
-- Returns:
-- a new (c)palette list {r,g,b,{original palette_indices},fused flag, histogram_weight}
-- a remap list (org) [image color + 1] = remap color (in the new palette)
function db.deCluster(pallist, hist, crad, prot_pow, pixels, rw,gw,bw)

 --messagebox(pixels)

 local u,c,a,i,o,j,n,c1,c2,r,g,b,r1,g1,b1,r2,g2,b2,wt,rt,gt,bt,tot,pd
 local worst,wtot,maxdist,maxDist,distfrac,clusterExists,clustVal,count,crad1
 local cList,cPalList,clusterList,fuseCol,orgcols,newPalList,org

 maxdist = math.sqrt(rw*rw*65025 + gw*gw*65025 + bw*bw*65025)
 distfrac = 100 / maxdist

 -- Let's just make a slightly more suitable format of the pallist (List for original color(s))
 cPalList = {}
 for u = 1, #pallist, 1 do
  c = pallist[u]
  cPalList[u] = {c[1],c[2],c[3],{c[4]},false,hist[c[4]+1]} -- r,g,b,{original colors},fuse_marker,histogram_weight
 end

 --table.insert(cPalList,{255,255,0,{257},false,1})

 clusterExists = true
 while clusterExists do
  clusterExists = false
  clusterList = {}

  crad1 = crad + 1 -- avoid divison by zero
  worst = 9999
  for a = 1, #cPalList, 1 do 
    c1 = cPalList[a]
    r1,g1,b1 = c1[1],c1[2],c1[3]
    wtot = c1[6]
    cList = {a}
    maxDist = 0
    for b = 1, #cPalList, 1 do
      if (b ~= a) then
        c2 = cPalList[b]
        r2,g2,b2 = c2[1],c2[2],c2[3]
        wt = c2[6]
        pd = math.pow((1 + wt / pixels),  prot_pow) -- Protection, increase distance
        dist = db.getColorDistance_weight(r1,g1,b1,r2,g2,b2,rw,gw,bw) * distfrac * pd
        if dist <= crad then 
           wtot = wtot + wt
          table.insert(cList,b)
          maxDist = math.max(dist,maxDist)
        end
      end
    end -- b
    if #cList > 1 then 
      clustVal = maxDist / (crad1 * #cList) * (wtot / #cList)   
      if clustVal < worst then
        worst = clustVal
        clusterList = cList
      end
    end
  end -- a
  
  --t = db.ary2txt(clusterList)
  --messagebox("Worst cluster is "..t)
  
   -- Fuse
  if #clusterList > 1 then
    clusterExists = true -- Run another iteration and look for more clusters
    fuseCol = {0,0,0,{}}
    rt,gt,bt,tot = 0,0,0,0
    for n = 1, #clusterList, 1 do
     i = clusterList[n]
     c = cPalList[i]
       --o = c[4][1] -- Original color (always #1 in list since fused colors can't re-fuse) 
     o = c[4] -- Original color list
     --if c[5] == true then messagebox("Re-Fusing..."); end
     r,g,b = c[1],c[2],c[3]
       --wt = hist[o+1] -- Org. colors are 0-255
     wt = c[6]
     rt = rt + r * wt
     gt = gt + g * wt
     bt = bt + b * wt
     tot = tot + wt
     cPalList[i] = -1 -- Erase color
       --table.insert(fuseCol[4],o)
     orgcols = fuseCol[4]
     for j = 1, #o, 1 do
      table.insert(orgcols,o[j])
     end
     fuseCol[4] = orgcols
    end

    rt = rt / tot
    gt = gt / tot
    bt = bt / tot
    fuseCol[1] = rt
    fuseCol[2] = gt
    fuseCol[3] = bt
    fuseCol[5] = true -- fusecol marker
    fuseCol[6] = tot
    table.insert(cPalList,fuseCol)
    --messagebox(#clusterList.." Colors was fused, resulting in "..rt..", "..gt..", "..bt)
    newPalList = {}
    for n = 1, #cPalList, 1 do
     if cPalList[n] ~= -1 then
       table.insert(newPalList,cPalList[n])
      --newPalList = db.newArrayInsertLast(newPalList,cPalList[n])  
     end
    end
    cPalList = newPalList
    --messagebox("Pal length: "..#cPalList)
   statusmessage("DeCluster - Image colors:"..#cPalList.."                       "); waitbreak(0)
  end -- fuse

 end -- while

    -- Create remap-list
    org = {}
    count = 0
    for u = 1, #cPalList, 1 do
       c = cPalList[u]
       for n = 1, #c[4], 1 do
         i = c[4][n]
         org[i+1] = count -- quick way to remap without matchcolor
       end
       count = count + 1
    end

 return org,cPalList

end; -- decluster


--
-- ... eof Custom Color / Palette functions ...
--


-- *****************************
-- *** Custom Draw functions ***
-- *****************************

--
function db.lineTransp(x1,y1,x2,y2,c,amt) -- amt: 0-1, 1 = Full color
 local n,st,m,x,y,r,g,b,r1,g1,b1,c2,org; m = math
 org = 1 - amt
 st = m.max(1,m.abs(x2-x1),m.abs(y2-y1));
 for n = 0, st, 1 do
   x = m.floor(x1+n*(x2-x1)/st)
   y = m.floor(y1+n*(y2-y1)/st)
   r,g,b = getcolor(getpicturepixel(x,y))
   r1,g1,b1 = getcolor(c)
   c2 = matchcolor(r1*amt+r*org, g1*amt+g*org, b1*amt+b*org) 
   putpicturepixel(x, y, c2 );
 end
end
--

--
function db.drawBrushRectangle(x1,y1,w,h,c)
   local x,y
   for y = y1, y1+h-1, 1 do
    for x = x1, x1+w-1, 1 do
       putbrushpixel(x,y,c);
    end
   end
end
--

--
function db.drawRectangle(x1,y1,w,h,c)
	drawfilledrect(x1, y1, x1+w, y1+w, c);
end
--

--
function db.drawRectangleNeg(x1,y1,w,h,c)
   local x,y,xs,ys
   xs = db.sign(w)
   ys = db.sign(h)
   if xs == 0 then xs = 1; end
   if ys == 0 then ys = 1; end
   for y = y1, y1+h-1, ys do
    for x = x1, x1+w-1, xs do
       putpicturepixel(x,y,c);
    end
   end
end
--

--
function db.drawRectangleLine(x,y,w,h,c)
 w = w-1
 h = h-1
 drawline(x,y,x+w,y,c)
 drawline(x,y,x,y+h,c)
 drawline(x,y+h,x+w,y+h,c)
 drawline(x+w,y,x+w,y+h,c)
end
--


--
function db.drawRectangleMix(x1,y1,w,h,c1,c2)
   local x,y,c,n
   c = {c1,c2}
   n = 0
   for y = y1, y1+h-1, 1 do
     n = n + 1
    for x = x1, x1+w-1, 1 do
       n = n + 1
       putpicturepixel(x,y,c[n%2+1]);
    end
   end
end
--

--
function db.drawBrushCircle(x1,y1,r,c) -- ok, lottsa weird adjustments here, can probably be optimized...
   local x,y,d
   for y = 0, r*2, 1 do
    for x = 0, r*2, 1 do
       d = math.sqrt((x-r-0.5)^2 + (y-r-0.5)^2)
       if d < r-0.25 then putbrushpixel(x1+x-r-0.5,y1+y-r-0.5,c); end
    end
   end
end
--

--
-- Rotation in degrees
-- Step is # of line segments (more is "better")
-- a & b are axis-radius
function db.ellipse2(x,y,a,b,stp,rot,col) 
 local n,m=math,rad,al,sa,ca,sb,cb,ox,oy,x1,y1,ast
 m = math; rad = m.pi/180; ast = rad * 360/stp;
 sb = m.sin(-rot * rad); cb = m.cos(-rot * rad)
 for n = 0, stp, 1 do
  ox = x1; oy = y1;
  sa = m.sin(ast*n) * b; ca = m.cos(ast*n) * a
  x1 = x + ca * cb - sa * sb
  y1 = y + ca * sb + sa * cb
  if (n > 0) then drawline(ox,oy,x1,y1,col); end
 end
end
--



--[[
var ER = 0.3
var DR = 0.15

ellipse(0.5*xx,0.5*yy,DR*xx,6,Math.PI*0)

function ellipse(x,y,r,stp,rot){
 var n,deg=360,m=Math,rad=Math.PI/180,rn
 var ox,oy,x1,y1,x2,y2,d1,r1 = ER * xx
 
 for (n=0; n<=deg; n+=stp){

  ox = x2; oy = y2, rn = rad * n
  d1 = rn - rot
  x1 = x + m.sin(d1) * r
  y1 = y + m.cos(d1) * r

  x2 = x1 + m.sin(-rn) * r1
  y2 = y1 + m.cos(-rn) * r1
  if (n > 0){ line_rgb(MX,[0,0,0],0,ox,oy,x2,y2) }
 }
}

}

ellipse2(0.5*xx,0.5*yy,15,8,200,22,[0,0,0],0.5)

function ellipse2(x,y,a,b,stp,rot,rgb,transp){
 var n,m=Math,rad=m.PI/180,al,sa,ca,sb,cb,ox,oy,x1,y1
 sb = m.sin(-rot * rad); cb = m.cos(-rot * rad)
 for (n=0; n<=stp; n++){
  ox = x1; oy = y1; al = rad * 360/stp * n 
  sa = m.sin(al) * b; ca = m.cos(al) * a
  x1 = x + ca * cb - sa * sb
  y1 = y + ca * sb + sa * cb
  if (n > 0){ line_rgb(MX,rgb,transp,ox,oy,x1,y1) }
 }
}


]]



function db.obliqueCube(side,x,y,r,g,b,bri)
 local n,c,depth,x1,y1,x2,y2,f

  f = matchcolor
  c = f(r,g,b)
  cP50 =  f(r+bri*0.5,g+bri*0.5,b+bri*0.5)
  cP75 =  f(r+bri*0.75,g+bri*0.75,b+bri*0.75)
  cM50 =  f(r-bri*0.5,g-bri*0.5,b-bri*0.5)
  cM100 = f(r-bri,g-bri,b-bri)

 depth = math.floor(side / 2)

 for n = 0, depth-1, 1 do
  drawline(x+side+n,y-1-n,x+side+n,y+side-n-1,cM50)
 end

 for n = 0, depth-1, 1 do
  drawline(x+n,y-1-n,x+side+n-1,y-1-n,cP50)
 end

 --  /
 --   
 --drawline(x+side,y-1,x+side+depth-1,y-depth,c)

 -- Smoothing & Shade

 --
 --  /
 --drawline(x+side,y+side-1,x+side+depth-1,y+side-depth,cM100)

 --drawline(x,y,x+side-2,y,cP75)
 --drawline(x,y,x,y+side-2,cP75)

 db.drawRectangle(x,y,side,side,c)

end


function db.obliqueCubeBRI(side,x,y,r,g,b,bri,pallist,briweight,index_flag)
 local n,c,depth,x1,y1,x2,y2

  --f = db.getBestPalMatchHYBRID
  c =  db.getBestPalMatchHYBRID({r,g,b},    pallist, briweight, index_flag)
  cP50 =  db.getBestPalMatchHYBRID({r+bri*0.5,g+bri*0.5,b+bri*0.5},    pallist, briweight, index_flag)
  cP75 =  db.getBestPalMatchHYBRID({r+bri*0.75,g+bri*0.75,b+bri*0.75}, pallist, briweight, index_flag)
  cM50 =  db.getBestPalMatchHYBRID({r-bri*0.5,g-bri*0.5,b-bri*0.5},    pallist, briweight, index_flag)
  cM100 = db.getBestPalMatchHYBRID({r-bri,g-bri,b-bri},                pallist, briweight, index_flag)

 depth = math.floor(side / 2)

 db.drawRectangle(x,y,side,side,c)

 for n = 0, depth-1, 1 do
  drawline(x+side+n,y-1-n,x+side+n,y+side-n-1,cM50)
 end

 for n = 0, depth-1, 1 do
  drawline(x+n,y-1-n,x+side+n-1,y-1-n,cP50)
 end

 --  /
 --   
 drawline(x+side,y-1,x+side+depth-1,y-depth,c)

 -- Smoothing & Shade

 --
 --  /
 --drawline(x+side,y+side-1,x+side+depth-1,y+side-depth,cM100)

 --drawline(x,y,x+side-2,y,cP75)
 --drawline(x,y,x,y+side-2,cP75)


end


--
-- ... eof Custom Draw functions ...
--


-- ******************************
-- *** Filters & Convolutions ***
-- ******************************


function db.applyConvolution2Pic(convmx,divisor,bias,neg,amt)
 local r,g,b,mx,my,cx,cy,mxh,myh,mp,rb,gb,bb,xx,yy,x,y,w,h,div,n1,n2,amtr,ro,go,bo

 n1 = 1
 n2 = bias
 if neg == 1 then
  n1 = -1
  n2 = 255 + bias
 end
 
 amtr = 1 - amt
 w, h = getpicturesize()
 cy = #convmx
 cx = #convmx[1]
 mxh = math.floor(cx / 2) + 1
 myh = math.floor(cy / 2) + 1

  for y = 0, h-1, 1 do
  for x = 0, w-1, 1 do
   r,g,b = 0,0,0
   ro,go,bo = getcolor(getbackuppixel(x,y))
   div = divisor
   for my = 1, cy, 1 do
    for mx = 1, cx, 1 do
     xp = mx-mxh
     yp = my-myh
     mp = convmx[my][mx]
     xx = x + xp
     yy = y + yp
      if yy>=0 and yy<h and xx>=0 and xx<w then
       rb,gb,bb = getcolor(getbackuppixel(xx,yy)) 
       r = r + rb * mp
       g = g + gb * mp
       b = b + bb * mp
        else div = div - mp -- Assumes divisor is the sum of the convolution matrix
      end
    end
   end
   r = ro*amtr + (n2 + (n1 * r) / div)*amt -- +bias
   g = go*amtr + (n2 + (n1 * g) / div)*amt 
   b = bo*amtr + (n2 + (n1 * b) / div)*amt 
   putpicturepixel(x,y,matchcolor(r,g,b))
 end;end;

end

--
-- ... eof Filters & Convolutions ...
--



-- *****************************
-- *** Fractals etc.         ***
-- *****************************

-- Fractal Pattern V1.0 by Richard Fhager (mod allows for wrapping)
--
-- Pattern matrix example: {{1,1,1},{1,0,1},{1,1,1}}
--
function db.pattern(x,y,p,n,i) -- coord as fraction of 1, pattern, offset(0), iterations (1-15)
 local px,py
 py = #p
 px = #p[1]
 while ((p[1+math.abs(math.floor(y*py))%py][1+math.abs(math.floor(x*px))%px]) > 0 and n<i) do
  x=x*px-math.floor(x*px); 
  y=y*py-math.floor(y*py);
  n = n+1
 end
 return 1 - n/i;
end
--

--
function db.patternDec(x,y,p,n,i) -- coord as fraction of 1, pattern, offset(0), iterations (1-15)
 local px,py,spfrac,nfrac,fx,fy
spfrac = 1
nfrac = 0 
 py = #p
 px = #p[1]
 while (spfrac > 0 and n<i) do
   fy = math.floor(math.abs(y*py))%py
   fx = math.floor(math.abs(x*px))%px
   spfrac = p[fy+1][fx+1]
   if (spfrac>0) then 
    x = x*px - fx
    y = y*py - fy
    nfrac = nfrac + spfrac
   end
  n = n+1
 end
 --return 1 - n/i;
 return 1 - nfrac/i
end
--


--
function db.mandel(x,y,l,r,o,i) -- pos. as fraction of 1, left coord, right coord, y coord, iterations 

  local w,s,a,p,q,n,v,w

  s=math.abs(r-l);

  a = l + s*x;
  p = a;
  b = o - s*(y-0.5);
  q = b;
  n = 1;
  v = 0;
  w = 0; 

  while (v+w<4 and n<i) do n=n+1; v=p*p; w=q*q; q=2*p*q+b; p=v-w+a; end;

  return n
end
--

--
-- ... eof Fractals etc. ...
--


-- ********************************************
-- *** Color Cube / Space, Custom Functions ***
-- ********************************************
--
-- SHADES (sha): 24bit colors is too complex so we operate with less shades/bits/colors. 
-- 16 shades = 12bit = 16*16*16 = 4096 colors (or cubic-elements)
-- 32 shades = 15bit = 32*32*32 = 32768 colors

--
-- data: {color available flag, 0 (gravity/distance, calculated, init as zero)}
--
function db.initColorCube(sha,data)
  local ary,z,y,x,n 
  ary = {}
  for z = 0, sha-1, 1 do
   ary[z+1] = {}
   for y = 0, sha-1, 1 do
    ary[z+1][y+1] = {}
      if data ~= nil then
        for x = 0, sha-1, 1 do
           -- This is silly stupid, if you know how to assign one array to another (no ref), plz help!
           -- i.e. how to: data = {false,0}; myArray[z][y][x] = data. 
           ary[z+1][y+1][x+1] = {}  
           for n = 1, #data, 1 do
             ary[z+1][y+1][x+1][n] = data[n] 
           end
        end
      end
   end
  end
  return ary
end
--

--
function db.findVoid(cube,sha) -- void is point with longest distance to closest color
  local weakest,weak_i,x,y,z,c,w
  weakest = -1
   weak_i = {-1,-1,-1}
  for z = 0, sha-1, 1 do
   for y = 0, sha-1, 1 do
    for x = 0, sha-1, 1 do
 
      c = cube[z+1][y+1][x+1]
      if c[1] == true then
        w = c[2]
        if w > weakest then weakest = w; weak_i = {z,y,x}; end
      end 
    
  end;end;end
  return weak_i[1],weak_i[2],weak_i[3]
end
--

--
--
-- Nearest color version: void is selected by the point that has the greatest distance
-- to the nearest color. Higher value means greater void.
--
function db.addColor2Cube(cube,sha,r,g,b,rw,gw,bw)
  local star,x,y,z,d,rd,gd,bd,cu1,cu2
  star = 0
  cube[r+1][g+1][b+1] = {false, star}
  for z = 0, sha-1, 1 do
   rd = (rw*(z-r))^2
   cu2 = cube[z+1]
   for y = 0, sha-1, 1 do
    gd = (gw*(y-g))^2
    cu1 = cu2[y+1]
    for x = 0, sha-1, 1 do
  
      d = rd + gd + (bw*(x-b))^2 

      --cube[z+1][y+1][x+1][2] = math.min(d, cube[z+1][y+1][x+1][2]) -- Don't add, use nearest color

      cu1[x+1][2] = math.min(d, cu1[x+1][2]) 

  end;end;end
end
--

-- Should be same as original, but not 100% verified. Using a rgb+1 trick to speed up handling
--
function db.addColor2Cube_test(cube,sha,r,g,b,rw,gw,bw)
  local star,x,y,z,d,rd,gd,bd,cu1,cu2
  star = 0
  r = r+1; g = g+1; b = b+1
  cube[r][g][b] = {false, star}
  for z = 1, sha, 1 do
   rd = (rw*(z-r))^2
   cu2 = cube[z]
   for y = 1, sha, 1 do
    gd = (gw*(y-g))^2
    cu1 = cu2[y]
    for x = 1, sha, 1 do 
      cu1[x][2] = math.min(rd+gd+(bw*(x-b))^2, cu1[x][2]) 
  end;end;end
end
--



-- Create new allowed colorlines in colorspace (ramps from which colors can be picked)
function db.enableRangeColorsInCube(cube,sha,r1,g1,b1,r2,g2,b2) 

    local div,r,g,b,n,rs,gs,bs
    div = 256 / sha
    rs = (r2 - r1) / sha / div 
    gs = (g2 - g1) / sha / div
    bs = (b2 - b1) / sha / div

    for n = 0, sha-1, 1 do

     r = math.floor(r1/div + rs * n)
     g = math.floor(g1/div + gs * n)
     b = math.floor(b1/div + bs * n)

     cube[r+1][g+1][b+1][1] = true

    end
end
--


function db.colorCigarr(shades,radius,fill_flag)
 local s,rad,radsq,step,shalf,bas,cols,found,x,y,z,bri,con,d,n
 radius = radius / 100
 step = math.floor(255 / (shades-1))
 shalf = math.floor(shades / 2)
 s = shades - 1
 rad = math.floor(shades / 2 * radius) 
 radsq = rad^2

 bas = 0
 cols = {}
 found = 0

 for z = 0, s, 1 do 
  for y = 0, s, 1 do
   for x = 0, s, 1 do

  --0.26,0.55,0.19
  bri = (x + y + z ) / 3
  --bri = math.sqrt(((x*0.26)^2 + (y*0.55)^2 + (z*0.19)^2)) * 1.5609
  con = math.floor((shades - math.abs(bri - shalf)*2) * radius)

  d = math.floor(math.sqrt((bri-x)^2 + (bri-y)^2 + (bri-z)^2))
  --d = math.floor(math.sqrt(((bri-x)*0.26)^2 + ((bri-y)*0.55)^2 + ((bri-z)*0.19)^2)) * 1.5609

  -- Filled cigarr: Less or Equal, cigarr shell: Equal
   if d == con or (d < con and fill_flag) then 
      found = found + 1
      r = bas + x * step
      g = bas + y * step
      b = bas + z * step
      cols[found] = {r,g,b}
   end

  end; end; end

  --messagebox("Colors found: "..found.."\n\n".."Run AnalyzePalette to examine")

  for n = 0, 255, 1 do
   if n < found then
    c = cols[n+1]
    setcolor(n,c[1],c[2],c[3]) 
     else
     setcolor(n,0,0,0) 
    end
  end
end -- eof colorcigarr


--
-- ... eof Color Cube ...
--



-- COLORMIX --
--
-- Returns a list of mixcolors palette entries, that are ranked by by quality & usefulness
--
-- This whole junk my partly locked on 16 shades (4096 colors/ 12bit palette precision) so don't use anything else...
--
--
function db.colormixAnalysis(sha,spare_flag,cust_dist) -- Interface
  local shades,pallist,ilist,custom_max_distance

  shades = sha -- 16 is good
  --messagebox(shades)

  custom_max_distance = -1
  if cust_dist ~= null then
    custom_max_distance = cust_dist -- in % 
  end

  if spare_flag == true then -- No shades here for now
    --pallist = db.makePalListShadeSPARE(256,shades)     -- 16 shades so Colorcube processes is possible
    pallist = db.makeSparePalList(256)
    pallist = db.fixPalette(pallist,0)            -- Remove doubles, No need to sort?
    ilist = db.makeIndexList(pallist, -1) 	  -- -1, use list order as index
   else
     pallist = db.makePalListShade(256,shades)       -- 16 shades so Colorcube processes is possible
     pallist = db.fixPalette(pallist,0)              -- Remove doubles, No need to sort?
     ilist = db.makeIndexList(pallist, -1) 	     -- -1, use list order as index
  end 

   if shades > 0 then
    return db.colormixAnalysisEXT(shades,pallist,ilist,custom_max_distance) -- max distance in %
   end
   if shades == -1 then
     return db.colormixAnalysisEXTnoshade(pallist,ilist,custom_max_distance) -- max distance in %
   end
end
--
--
function db.colormixAnalysisEXT(SHADES,pallist,ilist,custom_max_distance) -- Shades, most number of mixes returned
 local n,m,c1,c2,pairs,cube,rm,gm,bm
 local mix,total,found,dist,void,ideal,mini,maxi,bestmix,bestscore
 
 --messagebox("will now make pairs")

  pairs = db.pairsFromList(ilist,0)            -- 0 for unique pairs only, pairs are entries in pallist

  --messagebox(#pairs.." will now add colors to cube")

 cube = db.initColorCube(SHADES,{true,9999})
 for n = 1, #pallist, 1 do
   c1 = pallist[n]
   db.addColor2Cube_test(cube,SHADES,c1[1],c1[2],c1[3],0.26,0.55,0.19)
 end

 -- these values are adjusted for a 12bit palette (0-15) and perceptual weight where r+g+b = 1.0
 -- Ideal distance = 2.5 Green steps = 1.375
 -- Minimum distance = 1 Green step  = 0.55

 --messagebox("colorcube done")

 VACT = 1
 DACT = 1

 total = 9.56 -- Max distance possible with 16 shades
 ideal = 0.45  -- 1 step = 0.637
 mini = 0.35
 maxi = ideal + (total - ideal) / math.max(1, #pallist / 16)
  if custom_max_distance ~= -1 then
   maxi = total * (custom_max_distance / 100)
  end
 mix = {}
 --mix[1] =  {9e99,0,0,9e99,0,0,0}
 bestmix = -1
 bestscore = 9e99
 found = 0
 for n = 1, #pairs, 1 do

   c1 = pallist[pairs[n][1]]
   c2 = pallist[pairs[n][2]]
   --0.26,0.55,0.19
   dist = db.getColorDistance_weight(c1[1],c1[2],c1[3],c2[1],c2[2],c2[3],0.26,0.55,0.19) -- Not normalized

   rm = math.floor((c1[1]+c2[1])/2) 
   gm = math.floor((c1[2]+c2[2])/2)
   bm = math.floor((c1[3]+c2[3])/2)

  -- Mix color adjustment (perhaps less than perfect, but probably good enough)
   mixbri = db.getBrightness(rm,gm,bm)
   truebri = math.sqrt((db.getBrightness(c1[1],c1[2],c1[3])^2 + db.getBrightness(c2[1],c2[2],c2[3])^2) / 2)
   diff = truebri - mixbri
   rm = math.max(0,math.min(15,math.floor(rm + diff)))
   gm = math.max(0,math.min(15,math.floor(gm + diff)))
   bm = math.max(0,math.min(15,math.floor(bm + diff)))
    newbri = db.getBrightness(rm,gm,bm)
    delta =  math.abs(newbri - truebri)
    --if delta > 0.9 then 
    -- messagebox(pallist[pairs[n][1]][4]..", "..pallist[pairs[n][2]][4].." delta = "..delta) 
    --end
  --

   --rm = math.floor(math.sqrt((c1[1]^2 + c2[1]^2) / 2))
   --gm = math.floor(math.sqrt((c1[2]^2 + c2[2]^2) / 2))
   --bm = math.floor(math.sqrt((c1[3]^2 + c2[3]^2) / 2))

   void = cube[rm+1][gm+1][bm+1][2]

   if dist >= mini and dist <= maxi then
     found = found + 1
     score = ((1+DACT*(dist - ideal)^2) / (1+void*VACT)) -- Lowest is best
     mix[found] = {score,pallist[pairs[n][1]][4],pallist[pairs[n][2]][4],dist,rm*SHADES,gm*SHADES,bm*SHADES,c1[1]*SHADES,c1[2]*SHADES,c1[3]*SHADES,c2[1]*SHADES,c2[2]*SHADES,c2[3]*SHADES} -- mix holds palette entry
     if score < bestscore then bestscore = score; bestmix = found; end
   end

 end


 if true == false then
 -- 2nd pass, add bestmix to colorspace. This reduces many similar mixes.
 m = mix[bestmix]
 db.addColor2Cube(cube,SHADES,m[5],m[6],m[7],0.26,0.55,0.19)
 for n = 1, #mix, 1 do
  if n ~= bestmix then
   m = mix[n]
   dist = m[4]
   void = cube[m[5]+1][m[6]+1][m[7]+1][2]
   score = ((1+DACT*(dist - ideal)^2) / (1+void*VACT)) 
   m[1] = score
  end
 end
 end

 c1,c2 = -1,-1
 if found > 0 then
  db.sorti(mix,1) 
  best = mix[1]
  c1 = best[2]
  c2 = best[3]
 end

 --return found,c1,c2
 return mix,found,c1,c2
end
--


--
-- Mixcolor without colorcube - no scoring or sorting, 24bit colors, faster...
--
function db.colormixAnalysisEXTnoshade(pallist,ilist,custom_max_distance)
 local n,m,c1,c2,pairs,cube,rm,gm,bm
 local mix,total,found,dist,void,ideal,mini,maxi,bestmix,bestscore

  pairs = db.pairsFromList(ilist,0)            -- 0 for unique pairs only, pairs are entries in pallist

 total = 162.53 -- Max distance possible with 24-bit palette ad Dawn3.0 color weights  162.53
 ideal = 0
 mini  = 0
 maxi  = ideal + (total - ideal) / math.max(1, #pallist / 16)

  if custom_max_distance ~= -1 then
   maxi = total * (custom_max_distance / 100)
  end

 mix = {}
 found = 0
 for n = 1, #pairs, 1 do

   c1 = pallist[pairs[n][1]]
   c2 = pallist[pairs[n][2]]
   --0.26,0.55,0.19
   dist = db.getColorDistance_weight(c1[1],c1[2],c1[3],c2[1],c2[2],c2[3],0.26,0.55,0.19) -- Not normalized

   rm = math.floor((c1[1]+c2[1])/2) 
   gm = math.floor((c1[2]+c2[2])/2)
   bm = math.floor((c1[3]+c2[3])/2)

  -- Mix color adjustment
   mixbri = db.getBrightness(rm,gm,bm)
   truebri = math.sqrt((db.getBrightness(c1[1],c1[2],c1[3])^2 + db.getBrightness(c2[1],c2[2],c2[3])^2) / 2)
   diff = truebri - mixbri
   rm = math.max(0,math.min(255,math.floor(rm + diff)))
   gm = math.max(0,math.min(255,math.floor(gm + diff)))
   bm = math.max(0,math.min(255,math.floor(bm + diff)))
    newbri = db.getBrightness(rm,gm,bm)
    delta =  math.abs(newbri - truebri)
    --if delta > 0.9 then 
    --  messagebox(pallist[pairs[n][1]][4]..", "..pallist[pairs[n][2]][4].." delta = "..delta) 
    --end
  --

   if dist >= mini and dist <= maxi then
     found = found + 1
     score = 1
     mix[found] = {score,pallist[pairs[n][1]][4],pallist[pairs[n][2]][4],dist,rm,gm,bm,c1[1],c1[2],c1[3],c2[1],c2[2],c2[3]} -- mix holds palette entry
   end

 end

 --messagebox(#mix)

 return mix,found,-1,-1
end
--



-- Fuse a palettelist into an extended mix-anlysis list
function db.fusePALandMIX(pal,mix,max_score,max_dist)
 local n,c,mixlist,tot,score,dist,c1,c2,rm,gm,bm

  mixlist = {}
  tot = 0

  -- {r,g,b,n}
  for n = 1, #pal, 1 do
   tot = tot + 1
   c = pal[n]
   mixlist[tot] = {0,c[4],c[4],0,c[1],c[2],c[3],c[1],c[2],c[3],c[1],c[2],c[3]}
  end

  -- {score,col#1,col#2,dist,rm,gm,bm} low score is best
  for n = 1, #mix, 1 do
   score = mix[n][1]
   dist  = mix[n][4]
   if score <= max_score and dist <= max_dist then
     tot = tot + 1
      mixlist[tot] = mix[n]
   end
  end

 return mixlist
end
--


-- ********************************************
-- *** L-system (fractal curves & "plants") ***
-- ********************************************
--

--
function db.Lsys_makeData(a)
  local n,i; i = {}
  for n = 1, #a, 1 do i[a[n][2]] = a[n]; end
  return i
end
--

--
function db.Lsys_makeSet(seed,iter,data)
  local s,n,i,nset,set
  set = seed
  for n = 1, iter, 1 do
   nset = ''
   for i = 1, #set, 1 do
    s = string.sub(set,i,i)
    nset = nset..data[s][3]
   end
   set = nset
  end
  return set
end
--

function db.Lsys_draw(set,data,cx,cy,size,rot,rgb,rng,transp, speed)
  local p,M,DEG,l,n,d,i,v,q,c,tx,ty,posx,posy,dval,col,w,h,s,cl,count

  if speed == nil then speed = 50; end -- speed is drawing operations per update

  function ang(d) return (d % 360 + 360) * DEG; end
  
  w,h = getpicturesize()

  p   = 0
  M   = math
  DEG = math.pi/180
  l   = #set

  posx={}; posy={}; dval={}

  if (rgb == null) then rgb = {0,0,0}; end
  if (transp == null) then transp = 0; end
  col = db.newArrayMerge(rgb,{})
  q = 255 / l

  count = 0
  for n = 1, l, 1 do
    s = string.sub(set,n,n)
    d = data[s]
    i = d[1]
    v = d[4]

    --messagebox(i)

    if (i == 'Left')  then rot = rot - v; end
    if (i == 'Right') then rot = rot + v; end
 
    if (i == 'Save') then p=p+1; posx[p] = cx; posy[p] = cy; dval[p] = rot; end
    if (i == 'Load') then cx = posx[p]; cy = posy[p]; rot = dval[p]; p=p-1; end

    if (i == 'Draw') then
      tx = cx +  M.sin(ang(rot)) * size
      ty = cy + -M.cos(ang(rot)) * size
         for c = 1, 3, 1 do
          if (rng[c] > 0) then col[c] = rgb[c] + (n * q) * rng[c]; end
          if (rng[c] < 0) then col[c] = rgb[c] + (n * q) * rng[c]; end
         end 

      cl = matchcolor(col[1],col[2],col[3])
      --putpicturepixel(cx*w,cy*h,cl); 
      --drawline(cx*w,cy*h,tx*w,ty*h,cl)
      db.lineTransp(cx*w,cy*h,tx*w,ty*h,cl,transp) 
  
      cx = tx; cy = ty
    end
   count = count + 1
   if count == speed then count = 0; updatescreen(); if (waitbreak(0)==1) then return end; end
  end

  return {cx,cy,rot}
end -- draw


--
-- eof L-system
--








