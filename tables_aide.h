/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)

    Grafx2 is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2
    of the License.

    Grafx2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
    write to the Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
// Les tables suivantes servent de table de conversion entre un caractère ASCII
// et le sprite qui le représente dans la Fonte

// Table de caractères Normaux
static const int Caracteres_Aide_N[] = { // Commence à ' '
    0,     1,     2,     3,     4,     5,     6,     7,     8,     9, 
   10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
   20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
   30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
   40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
   50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
   60,    61,     0,    62,    63,    64,    65,    66,    67,    68,
   69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
   79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
   89,    90,    91,    92,     0,     0,    93,    94,    95,    96,
    0,    97,     0,    98,    99,   100,   101,     0,     0,     0,
    0,     0,     0,   102,     0,   103,     0,     0,   104,   105};


  
// Table de caractères Sous-Titres
static const int Caracteres_Aide_S[] = { // Commence à ' '
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 
    0,     0,     0,   132,   133,   134,   135,   136,   137,   138, 
  139,   140,   141,   142,   143,   144,   145,     0,     0,     0, 
    0,   146,     0,   106,   107,   108,   109,   110,   111,   112, 
  113,   114,   115,   116,   117,   118,   119,   120,   121,   122, 
  123,   124,   125,   126,   127,   128,   129,   130,   131 };
  // Table de caractères Titres (demi-ligne supérieure)
static const int Caracteres_Aide_Titre_haut[] = { // Commence à ' '
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
  126+147,
   96+147, // -
   98+147, // .
  100+147, // /
  102+147, // 0
  104+147, // 1
  106+147, // 2
  108+147, // 3
  110+147, // 4
  112+147, // 5
  114+147, // 6
  116+147, // 7
  118+147, // 8
  120+147, // 9
  122+147, // :
  126+147,
  126+147,
  126+147,
  126+147,
  124+147,  // ?// ?
  126+147,
    0+147, // A
    2+147, // B
    4+147, // C
    6+147, // D
    8+147, // E
   10+147, // F
   12+147, // G
   14+147, // H
   16+147, // I
   18+147, // J
   20+147, // K
   22+147, // L
   24+147, // M
   26+147, // N
   28+147, // O
   30+147, // P
   32+147, // Q
   34+147, // R
   36+147, // S
   38+147, // T
   40+147, // U
   42+147, // V
   88+147, // W
   90+147, // X
   92+147, // Y
   94+147  // Z
};
  // Table de caractères Titres (demi-ligne supérieure)
static const int Caracteres_Aide_Titre_bas[] = { // Commence à ' '
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
   96+147+40, // -
   98+147+40, // .
  100+147+40, // /
  102+147+40, // 0
  104+147+40, // 1
  106+147+40, // 2
  108+147+40, // 3
  110+147+40, // 4
  112+147+40, // 5
  114+147+40, // 6
  116+147+40, // 7
  118+147+40, // 8
  120+147+40, // 9
  122+147+40, // :
  126+147+40,
  126+147+40,
  126+147+40,
  126+147+40,
  124+147+40, // ?
  126+147+40,
    0+147+44, // A
    2+147+44, // B
    4+147+44, // C
    6+147+44, // D
    8+147+44, // E
   10+147+44, // F
   12+147+44, // G
   14+147+44, // H
   16+147+44, // I
   18+147+44, // J
   20+147+44, // K
   22+147+44, // L
   24+147+44, // M
   26+147+44, // N
   28+147+44, // O
   30+147+44, // P
   32+147+44, // Q
   34+147+44, // R
   36+147+44, // S
   38+147+44, // T
   40+147+44, // U
   42+147+44, // V
   88+147+40, // W
   90+147+40, // X
   92+147+40, // Y
   94+147+40  // Z
};

// Quelques formules magiques:
#define AIDE_TEXTE(x) "N" x,
// Génère une ligne "Ntexte",

#define AIDE_BOLD(x) "S" x,
// Génère une ligne "Stexte",

#define AIDE_TITRE(x) "T" x, "-",
// Génère une ligne "Ttexte",
// et une deuxième  "-",
// (pour gérer les gros titres qui occupent deux lignes)

static const char * TableAideAbout[] =
/*
  AIDE_TEXTE("--------------------------------------------")
*/
{
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TITRE("      GRAFX 2.00")
  AIDE_BOLD ("  THE ULTIMATE MULTI-RESOLUTION GFX EDITOR")
  AIDE_TEXTE("       http://grafx2.googlecode.com")
  AIDE_TEXTE("")
  AIDE_TEXTE("    Copyright 2007 by the contributors")
  AIDE_TEXTE("   Copyright 1996-2001 by SUNSET DESIGN")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
};
static const char * TableAideLicense[] =
{
  AIDE_TITRE("       LICENSE")
  AIDE_TEXTE("")
  AIDE_TEXTE("Grafx2 is FREE SOFTWARE, you can")
  AIDE_TEXTE("redistribute it and/or modify it under the")
  AIDE_TEXTE("terms of the GNU General Public License as")
  AIDE_TEXTE("published by the Free Software Foundation;")
  AIDE_TEXTE("version 2 of the License.")
  AIDE_TEXTE("")
  AIDE_TEXTE("Grafx2 is distributed in the hope that it")
  AIDE_TEXTE("will be useful, but WITHOUT ANY WARRANTY;")
  AIDE_TEXTE("without even the implied warranty of")
  AIDE_TEXTE("MERCHANTABILITY or FITNESS FOR A PARTICULAR")
  AIDE_TEXTE("PURPOSE.  See the GNU General Public License")
  AIDE_TEXTE("for more details.")
  AIDE_TEXTE("")
  AIDE_TEXTE("You should have received a copy of the GNU")
  AIDE_TEXTE("General Public License along with Grafx2;")
  AIDE_TEXTE("if not, see http://www.gnu.org/licenses/ or")
  AIDE_TEXTE("write to the Free Software Foundation, Inc.")
  AIDE_TEXTE(" 59 Temple Place - Suite 330, Boston,")
  AIDE_TEXTE(" MA  02111-1307, USA.")
  AIDE_TEXTE("")

};
static const char * TableAideHelp[] =
{
  AIDE_TITRE("HELP")
  AIDE_TEXTE("")
  AIDE_TEXTE("  Documentation is here.")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TEXTE("")
  AIDE_TEXTE("Or will be.")
};
static const char * TableAideCredits[] =
{
  AIDE_TITRE("GRAFX2.GOOGLECODE.COM")
  AIDE_BOLD ("            PROJECT CONTRIBUTORS")
  AIDE_TEXTE("")
  AIDE_TEXTE("  Adrien Destugues (pulkomandy@gmail.com)")
  AIDE_TEXTE("  franhec@gmail.com")
  AIDE_TEXTE("  martin.planes@gmail.com")
  AIDE_TEXTE("  PLume.Argentee@gmail.com")
  AIDE_TEXTE("  Yves Rizoud (yrizoud@gmail.com)")
  AIDE_TEXTE("")
  AIDE_BOLD ("                    ART")
  AIDE_TEXTE("")
  AIDE_TEXTE("  GrafX2 logo by Made (www.m4de.com)")
  AIDE_TEXTE("  Icons and fonts by X-Man ")
  AIDE_TEXTE("")
  AIDE_TITRE("    SUNSET DESIGN")
  AIDE_BOLD ("       AUTHORS OF GRAFX2.0 BETA 96.5%")
  AIDE_TEXTE("")
  AIDE_BOLD ("    GUILLAUME DORME       KARL MARITAUD")
  AIDE_TEXTE("   alias \"Robinson\"   &   alias \"X-Man\"")
  AIDE_TEXTE("         programmer       programmer")
  AIDE_TEXTE("                          GFX artist")
  AIDE_TEXTE("")
  AIDE_TEXTE("   Re-licensed GrafX2 under the GPL in 2001")
  AIDE_TEXTE("")
  AIDE_BOLD ("  SNAIL MAIL:")
  AIDE_TEXTE("  (From 2001, current status: unknown)")
  AIDE_TEXTE("")
  AIDE_TEXTE("    GUILLAUME DORME (Robinson)")
  AIDE_TEXTE("    15, rue de l'observatoire")
  AIDE_TEXTE("    87000 LIMOGES (FRANCE)")
  AIDE_TEXTE("")
  AIDE_TEXTE("    KARL MARITAUD (X-Man)")
  AIDE_TEXTE("    10, rue de la Brasserie")
  AIDE_TEXTE("    87000 LIMOGES (FRANCE)")
  AIDE_TEXTE("")
  AIDE_BOLD ("  WEBSITE:")
  AIDE_TEXTE("  (From 2001, current status: down)")
  AIDE_TEXTE("  http://w3.ensil.unilim.fr/~maritaud/sunset")
  AIDE_TEXTE("")
  AIDE_BOLD("  THANKS:")
  AIDE_TEXTE("")
  AIDE_TEXTE("    We were inspired by the XSetMode routine")
  AIDE_TEXTE("   from 'XLIB2' to create more amazing video")
  AIDE_TEXTE("   resolutions.")
  AIDE_TEXTE("")
  AIDE_TEXTE("    Thanks to Eclipse for their EOS 3.05 dos")
  AIDE_TEXTE("   extender (WEOSlite).")
  AIDE_TEXTE("")
  AIDE_TEXTE("    Some information taken from several docs")
  AIDE_TEXTE("   (PCGPE, Intervue, PC Interdit...) gave us")
  AIDE_TEXTE("   an invaluable help.")
  AIDE_TEXTE("")
  AIDE_TEXTE("    Thanks to Shawn Hargreaves for his")
  AIDE_TEXTE("   filled polygon routine from Allegro v2.2.")
  AIDE_TEXTE("")
  AIDE_TEXTE("    Thanks to Carlos \"Made\" Pardo for his")
  AIDE_TEXTE("   great GrafX2 logo.")
  AIDE_TEXTE("")
  AIDE_TEXTE("    This is our very first program compiled")
  AIDE_TEXTE("   in Watcom C (10.6).")
  AIDE_TEXTE("    Low-level routines were written in 80386")
  AIDE_TEXTE("   Assembler (optimized for 80486) and")
  AIDE_TEXTE("   compiled with Borland TASM 5 (4.1).")
  AIDE_TEXTE("    A thousand thanks to the authors of")
  AIDE_TEXTE("    these compilers.")
  AIDE_TEXTE("")
  AIDE_TEXTE("    We also would like to thank all the")
  AIDE_TEXTE("   people who gave us ideas to improve")
  AIDE_TEXTE("   GrafX2.")
  AIDE_TITRE("")
  AIDE_BOLD ("  FILE FORMATS CREDITS:")
  AIDE_TEXTE("")
  AIDE_TEXTE("   PKM : Sunset Design")
  AIDE_TEXTE("   LBM : Electronic Arts")
  AIDE_TEXTE("   GIF : Compuserve")
  AIDE_TEXTE("   BMP : Microsoft")
  AIDE_TEXTE("   PCX : Z-Soft")
  AIDE_TEXTE("   IMG : Bivas (W. Wiedmann?)")
  AIDE_TEXTE("   SCx : Colorix (?)")
  AIDE_TEXTE("   CEL,KCF : K.O.S. (KISekae Set system)")
  AIDE_TEXTE("   PI1,PC1 : Degas Elite")
  AIDE_TEXTE("   PAL : ermmh... nobody (?)")
  AIDE_TEXTE("")
  AIDE_TEXTE("We send our best regards to...")
  AIDE_TEXTE("")
  AIDE_TEXTE("  Access        Filter        Pink")
  AIDE_TEXTE("  Ace           Fiver         Pixel")
  AIDE_TEXTE("  AcidJam       Flan          Profil")
  AIDE_TEXTE("  Acryl         Fred          Prowler")
  AIDE_TEXTE("  Alexel        FreddyV       Puznik")
  AIDE_TEXTE("  Alias         Frost         Quick")
  AIDE_TEXTE("  Amiral        Ga‰l(GDC)     Ra")
  AIDE_TEXTE("  Arrakis       GainX         Raster")
  AIDE_TEXTE("  Avocado       Gandalf       Ravian")
  AIDE_TEXTE("  Baloo         Goblin        RedBug")
  AIDE_TEXTE("  Barti         Greenpix7     Rem")
  AIDE_TEXTE("  Bat           Grid          Rez")
  AIDE_TEXTE("  Biro          GrosQuick     Roudoudou")
  AIDE_TEXTE("  Bisounours    HackerCroll   Sacrilege")
  AIDE_TEXTE("  BlackAxe      Haplo         Sam")
  AIDE_TEXTE("  Bonnie        Hof           SandMan")
  AIDE_TEXTE("  Boo           Hornet        Scape")
  AIDE_TEXTE("  Boz           Hulud         S‚bastien")
  AIDE_TEXTE("  Carine        Java          Shodan")
  AIDE_TEXTE("  Chandra       JBT           Skal")
  AIDE_TEXTE("  Cheetah       J‚r“me        Skyfire")
  AIDE_TEXTE("  Chill         Julien(JCA)   Sphair")
  AIDE_TEXTE("  Cougar        KalMinDo      Sprocket")
  AIDE_TEXTE("  Cremax        KaneWood      Stef")
  AIDE_TEXTE("  Cyclone       Karma         Stony")
  AIDE_TEXTE("  Dake          Keith303      Sumaleth")
  AIDE_TEXTE("  Danny         Lazur         Sunday")
  AIDE_TEXTE("  Danube        LightShow     Suny")
  AIDE_TEXTE("  Darjul        Lluvia        Sybaris")
  AIDE_TEXTE("  Darwin        Louie         TBF")
  AIDE_TEXTE("  DarkAngel     Luk           Tempest")
  AIDE_TEXTE("  Das           Made          Thor")
  AIDE_TEXTE("  Decker        Mamos         TMK")
  AIDE_TEXTE("  DerPiipo      Mandrixx      TwoFace")
  AIDE_TEXTE("  Destop        Mangue        Underking")
  AIDE_TEXTE("  Diabolo       Mars          Unreal")
  AIDE_TEXTE("  DineS         Mephisto      VaeVictis")
  AIDE_TEXTE("  Drac          Mercure       Vastator")
  AIDE_TEXTE("  DrYes         Mirec         Vatin")
  AIDE_TEXTE("  Edyx          Moa           Veckman")
  AIDE_TEXTE("  Eller         Moxica        Wain")
  AIDE_TEXTE("  Ellyn         MRK           Wally")
  AIDE_TEXTE("  EOF           Nitch         WillBe")
  AIDE_TEXTE("  Fall          Noal          Xoomie")
  AIDE_TEXTE("  Fame          Nytrik        Xtrm")
  AIDE_TEXTE("  Fantom        Optic         YannSulu")
  AIDE_TEXTE("  Fear          Orome         Z")
  AIDE_TEXTE("  Feather       Pahladin      Zeb")
  AIDE_TEXTE("  Fennec        Phar          Zebig")
  AIDE_TEXTE("  and all #pixel, #demofr and #coders.")
};

#define DECLARATION_TABLE_AIDE(x) {x, sizeof(x)/sizeof(const char **)},

struct Section_d_aide Table_d_aide[] =
{
  DECLARATION_TABLE_AIDE(TableAideAbout)
  DECLARATION_TABLE_AIDE(TableAideLicense)
  DECLARATION_TABLE_AIDE(TableAideHelp)
  DECLARATION_TABLE_AIDE(TableAideCredits)
};
