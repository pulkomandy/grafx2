/* vim:expandtab:ts=2 sw=2:
 */
/**
 * Amstrad CPC Loader for overscan picture.
 * Locomotive BASIC + Z80 Code.
 * @author Ast/iMP4CT
 * @see https://amstradplus.forumforever.com/t462-iMPdraw-v2-0.htm?start=45
 */
static const unsigned char impdraw_loader[] = {
  0x0e, 0x00, 0x0a, 0x00, 0x01, 0xc0, 0x20, 0x69, // starts at &0170
  0x4d, 0x50, 0x20, 0x76, 0x32, 0x00, 0x0d, 0x00, // 10 ' iMP v2
  0x14, 0x00, 0xad, 0x20, 0x0e, 0x01, 0x83, 0x1c, // 20 MODE 0:CALL &1ad
  0xad, 0x01, 0x00, 0x00, 0x00,                   // &0184 : token 0x0e = 0, 0x0f = 1, 0x10 = 2
  0x01, 0x30, 0x02, 0x32, 0x06, 0x22, 0x07, 0x23, // From &018d CRTC registers
  0x0c, 0x0d, 0xd0, 0x00, 0x00, 0x3f,
  0xff, 0x00, 0xff, 0x77, 0xb3, 0x51, 0xa8, 0xd4, // From &019b CPC Plus ASIC unlocking sequence
  0x62, 0x39, 0x9c, 0x46, 0x2b, 0x15, 0x8a, 0xcd,
  0xee,
  0x00,                                           // &01ac = CPC PLUS flag (0=CPC old, 1=CPC+)
  0xf3,             // 01ad di
  0x21, 0x8d, 0x01, // 01ae ld hl,&018d
  0x3e, 0x06,       // 01b1 ld a,&06
  0x01, 0xbe, 0xbd, // 01b3 ld bc,BDBE
  0xed, 0xa3,       // 01b6 outi  ; first decrements b  <---+
  0x41,             // 01b8 ld b,c                          |
  0xed, 0xa3,       // 01b9 outi                            |
  0x3d,             // 01bb dec a                           |
  0x20, 0xf8,       // 01bc jz nz,01B6   ; loop ------------+
  // load colors into gate array (from 7F00 ;)
  0x01, 0x00, 0x7f, // 01be ld bc,7F00 ; Gate array
  0x1e, 0x10,       // 01c1 ld e,10
  0x0a,             // 01c3 ld a,(bc)               <---+
  0xed, 0x49,       // 01c4 out (c),c   ; select ink#   |
  0xed, 0x79,       // 01c6 out (c),a   ; set ink       |
  0x0c,             // 01c8 inc c                       |
  0x1d,             // 01c9 dec e                       |
  0x20, 0xf7,       // 01ca jz nz,01C3  ; loop ---------+

  0x3a, 0xac, 0x01, // 01cc ld a,01AC   ; CPC PLUS flag
  0xfe, 0x01,       // 01cf cp 01
  0x20, 0x22,       // 01d1 jz nz,01F5  ; flag != 1 => skip CPC+
  // CPC Plus ASIC unlock
  0x21, 0x9b, 0x01, // 01d3 ld hl,019B
  0x01, 0x11, 0xbc, // 01d6 ld bc,BC11
  0x7e,             // 01d9 ld a,(hl)       <---+
  0xed, 0x79,       // 01da out (c),a           |
  0x2c,             // 01dc inc l               |
  0x0d,             // 01dd dec c               |
  0x20, 0xf9,       // 01de jr nz,01D9 ; loop --+
  // set CPC Plus colors
  0x01, 0xb8, 0x7f, // 01e0 ld bc,7FB8
  0xed, 0x49,       // 01e3 out (c),c     ; open ASIC I/O
  0x21, 0x01, 0x08, // 01e5 ld hl,0801
  0x11, 0x00, 0x64, // 01e8 ld de,6400
  0x01, 0x20, 0x00, // 01eb ld bc,0020
  0xed, 0xb0,       // 01ee ldir          ; copy bc bytes from (hl) to (de)
  0x01, 0xa0, 0x7f, // 01f0 ld bc,7FA0
  0xed, 0x49,       // 01f3 out (c),c     ; close ASIC I/O

  0x21, 0xf9, 0xb7, // 01f5 ld hl,B7F9    ; SCR Event Block: Set Inks
  0xcd, 0xdd, 0xbc, // 01f8 call BCDD     ; remove event block for CRTC IRQ
  0xfb,             // 01fb ei
  0xc3, 0x18, 0xbb, // 01fc jp BB18       ; Wait Keypress
  0x00              // 01ff
};
