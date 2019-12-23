/* vim:expandtab:ts=2 sw=2:
 */
/**
 * CPC Z80 code to be put in a "gap" of an image (at address &C7D0).
 * Sets the screen mode (from &D7D0), then sets the border and the palette
 * (Firmware colors) at &D7D1
 * @author unknown
 */
static const unsigned char cpc_scr_simple_loader[] = {
 0x3a, 0xd0, 0xd7,  // C7D0 LD A,(D7D0)
 0xcd, 0x1c, 0xbd,  // C7D3 CALL BD1C ; Set screen mode
 0x21, 0xd1, 0xd7,  // C7D6 LD HL,D7D1
 0x46,              // C7D9 LD B,(HL)
 0x48,              // C7DA LD C,B
 0xcd, 0x38, 0xbc,  // C7DB CALL BC38 ; Set border
 0xaf,              // C7DE XOR A
 0x21, 0xd1, 0xd7,  // C7DF LD HL,D7D1
 0x46,              // C7E2 LD B,(HL)       <-------------------+
 0x48,              // C7E3 LD C,B                              |
 0xf5,              // C7E4 PUSH AF                             |
 0xe5,              // C7E5 PUSH HL                             |
 0xcd, 0x32, 0xbc,  // C7E6 CALL BC32 ; Set ink A to color B,C  |
 0xe1,              // C7E9 POP HL                              |
 0xf1,              // C7EA POP AF                              |
 0x23,              // C7EB INC HL                              |
 0x3c,              // C7EC INC A                               |
 0xfe, 0x10,        // C7ED CP &10                              |
 0x20, 0xf1,        // C7EF JZ NZ, C7E2      -------------------+
 0xc3, 0x18, 0xbb   // C7F1 JP BB18   ; Wait key

};
