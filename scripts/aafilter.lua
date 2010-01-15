-- Get the picture size
w, h = getpicturesize();

-- Here is the filtering matrix
 matrix = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };

-- Loop trough all the pixels
-- To make this script simpler we don't handle the picture borders
-- (the matrix would get pixels outside the picture space)
-- for var = start_value, end_value, step do ...
for y = 1, h - 2, 1 do
	for x = 1, w - 2, 1 do
		filtered =
			matrix[1][1] * getpicturepixel(x - 1, y - 1) +
			matrix[1][2] * getpicturepixel(x    , y - 1) +
			matrix[1][3] * getpicturepixel(x + 1, y - 1) +
			matrix[2][1] * getpicturepixel(x - 1, y    ) +
			matrix[2][2] * getpicturepixel(x    , y    ) +
			matrix[2][3] * getpicturepixel(x + 1, y    ) +
			matrix[3][1] * getpicturepixel(x - 1, y + 1) +
			matrix[3][2] * getpicturepixel(x    , y + 1) +
			matrix[3][3] * getpicturepixel(x + 1, y + 1);
		putpicturepixel(x,y,filtered);
	end
end
