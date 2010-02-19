-- Test LUA inputbox
--  this script only resizes the brush

w, h = getbrushsize()
--[[
messagebox(
  "Forecolor: " .. getforecolor() .. "\n" ..
  "Backcolor: " .. getbackcolor() .. "\n" ..
  "Transparent color: " .. gettranscolor() .. "\n" ..
  "Brush dimensions: " .. w .. "x" .. h
)
]]


ok, w, h = inputbox("Modify brush",
  "Width",  w, -100.0,100.0, 0,
  "Height", h, -200.0,100.0, 0,
  "X Flip", 0, 0, 1,  0,
  "Y Flip", 0, 0, 1,  0
);
if ok == true then
  setbrushsize(w,h)
  for y = 0, h-1, 1 do
    for x = 0, w-1, 1 do
      putbrushpixel(x,y,1);
    end
  end
end



