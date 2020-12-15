# picview.asm (c) 2019 Thomas Bernard

This is a C64 image depacker and viewer designed to build self-displaying
.prg pictures files.
One has to concatenate the data to the viewer machine code to build a .prg
file.

## Data format
-  1 byte for mode
-  then one or several blocks, composed of :
   1.  1 byte TP header :
       - upper nibble T is type
       - lower nibble P is packing
   2.  n bytes data load, depending of type and packing

### Mode byte
0EBM0000 E=Extended Background mode. B=Bitmap Mode. M=Multicolor.

### TP byte
- T = type
  - 0 - end of file
  - 1 - bitmap                  8000 bytes
  - 2 - screen ram              1000 bytes
  - 3 - color ram               1000 bytes at $D800
  - 4 - border/background etc.  15 bytes at $D020
- P = packing
  - 0 - no packing
  - 1 - RLE packing
  - 2 - color ram RLE packing

All other values are reserved.


## border/backgrounds at $D020, 15 bytes
- BO BORDER
- BA BACKGROUND
- E1 Extra background color #1
- E2 Extra background color #2
- E3 Extra background color #3
- SE1 sprite extra color #1
- SE2 sprite extra color #2
- S0 sprite #0
- S1 sprite #1
- S2 sprite #2
- S3 sprite #3
- S4 sprite #4
- S5 sprite #5
- S6 sprite #6
- S7 sprite #7
