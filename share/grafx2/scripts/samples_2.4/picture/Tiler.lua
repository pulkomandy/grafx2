--Picture Tiler by Adrien Destugues
--Extract unique tiles from the spare page
--to the main one. Main page is erased.
--
-- Copyright 2011 Adrien Destugues <pulkomandy@pulkomandy.ath.cx>
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

-- Copy palette from spare to main
-- TODO

-- Grid size
-- TODO : get it from GrafX2
xgrid = 8;
ygrid = 16;

-- picture size
w, h = getsparepicturesize();

-- We may need less if there are duplicates
setpicturesize(xgrid, w*h/xgrid);
clearpicture(255);

tileid = 0;

-- blit part of the spare to picture
function blitsparetopicture(srcx, srcy, dstx, dsty, width, height)
	local x,y;
	for y = 0, height - 1, 1 do
		for x = 0, width - 1, 1 do
			putpicturepixel(dstx+x, dsty+y, getsparepicturepixel(srcx + x, srcy + y));
		end
	end
end

function comparesparetopicture(srcx, srcy, dstx, dsty, width, height)
	local x,y,color
	for y = 0, height - 1, 1 do
		for x = 0, width - 1, 1 do
			color = getsparepicturepixel(srcx + x, srcy + y);
			if color ~= getpicturepixel(dstx+x, dsty+y) then
				-- they are different
				return false;
			end
		end
	end
	-- they are identical
	return true;
end

-- compute checksum of a picture area
-- it may not be unique, we use it as a key for an hashmap
function checksum(srcx, srcy, width, height)
	local sum,x,y
	sum = 0;
	for y = 0, height - 1, 1 do
		for x = 0, width - 1, 1 do
			sum = sum + getsparepicturepixel(srcx+x, srcy+y);
		end
	end

	return sum;
end

tilemap = {}

-- foreach tile
for y = 0, h-1, ygrid do
	for x = 0, w - 1, xgrid do
		-- existing one ?
		csum = checksum(x,y,xgrid,ygrid);
		if tilemap[csum] ~= nil then
			-- potential match
			-- Find matching tileid
			found = false;
			for id in pairs(tilemap[csum]) do
				-- is it a match ?
				if comparesparetopicture(x,y,0,id*ygrid, xgrid, ygrid) then
					-- found it !
					tilemap[csum][id] = tilemap[csum][id] + 1;
					found = true;
					break;
				end
			end
			-- Add tile anyway if needed
			if not found then
				desty = tileid * ygrid;
				blitsparetopicture(x, y, 0, desty, xgrid, ygrid);

				-- add it to the tilemap
				tilemap[csum][tileid] = 1;
				-- give it a tile id
				tileid = tileid + 1;
			end
		else
			-- Copy to spare
			desty = tileid * ygrid;
			blitsparetopicture(x, y, 0, desty, xgrid, ygrid);

			-- add it to the tilemap
			tilemap[csum] = {}
			tilemap[csum][tileid] = 1;
			-- give it a tile id
			tileid = tileid + 1;
		end
	end
end

finalizepicture(); -- resize from what we just did
setpicturesize(xgrid, (tileid-1)*ygrid)
updatescreen();
