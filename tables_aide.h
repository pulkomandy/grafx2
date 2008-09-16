// Les tables suivantes servent de table de conversion entre un caractère ASCII
// et le sprite qui le représente dans la Fonte

// Table de caractères Normaux
static const int Caracteres_Aide_N[] = { // Commence à ' '
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
   10,
   11,
   12,
   13,
   14,
   15,
   16,
   17,
   18,
   19,
   20,
   21,
   22,
   23,
   24,
   25,
   26,
   27,
   28,
   29,
   30,
   31,
   32,
   33,
   34,
   35,
   36,
   37,
   38,
   39,
   40,
   41,
   42,
   43,
   44,
   45,
   46,
   47,
   48,
   49,
   50,
   51,
   52,
   53,
   54,
   55,
   56,
   57,
   58,
   59,
   60,
   61,
    0,
   62,
   63,
   64,
   65,
   66,
   67,
   68,
   69,
   70,
   71,
   72,
   73,
   74,
   75,
   76,
   77,
   78,
   79,
   80,
   81,
   82,
   83,
   84,
   85,
   86,
   87,
   88,
   89,
   90,
   91,
   92,
    0,
    0,
   93,
   94,
   95,
   96,
    0,
   97,
    0,
   98,
   99,
  100,
  101,
    0,
    0,
    0,
    0,
    0,
    0,
  102,
    0,
  103,
    0,
    0,
  104,
  105};


  
// Table de caractères Sous-Titres
static const int Caracteres_Aide_S[] = { // Commence à ' '
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
  132,
  133,
  134,
  135,
  136,
  137,
  138,
  139,
  140,
  141,
  142,
  143,
  144,
  145,
    0,
    0,
    0,
    0,
  146,
    0,
  106,
  107,
  108,
  109,
  110,
  111,
  112,
  113,
  114,
  115,
  116,
  117,
  118,
  119,
  120,
  121,
  122,
  123,
  124,
  125,
  126,
  127,
  128,
  129,
  130,
  131 };
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

static const char * TableAide1[] =
{
  AIDE_TITRE("      GRAFX 2.00")
  AIDE_BOLD ("  THE ULTIMATE MULTI-RESOLUTION GFX EDITOR")
  AIDE_TEXTE("   Copyright (c) 1996-98 by SUNSET DESIGN")
  AIDE_TEXTE("")
  AIDE_TITRE("CREDITS:")
  AIDE_TEXTE("")
  AIDE_BOLD ("  WRITTEN AND DESIGNED BY:")
  AIDE_TEXTE("")
  AIDE_BOLD ("    GUILLAUME DORME       KARL MARITAUD")
  AIDE_TEXTE("   alias \"Robinson\"   &   alias \"X-Man\"")
  AIDE_TEXTE("         programmer       programmer")
  AIDE_TEXTE("                          GFX artist")
  AIDE_TITRE("")
  AIDE_BOLD ("  MISCELLANEOUS:")
  AIDE_TEXTE("")
  AIDE_TEXTE("    This is our very first program compiled")
  AIDE_TEXTE("   in Watcom C (10.6).")
  AIDE_TEXTE("    Low-level routines were written in 80386")
  AIDE_TEXTE("   Assembler (optimized for 80486) and")
  AIDE_TEXTE("   compiled with Borland TASM 5 (4.1).")
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
  AIDE_TEXTE("    A thousand thanks to the authors of the")
  AIDE_TEXTE("   different things listed above.")
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
  AIDE_TITRE("")
  AIDE_TITRE("HOW TO CONTACT US ?")
  AIDE_TEXTE("")
  AIDE_BOLD ("  SNAIL MAIL:")
  AIDE_TEXTE("")
  AIDE_TEXTE("    GUILLAUME DORME (Robinson)")
  AIDE_TEXTE("    15, rue de l'observatoire")
  AIDE_TEXTE("    87000 LIMOGES (FRANCE)")
  AIDE_TEXTE("")
  AIDE_TEXTE("    KARL MARITAUD (X-Man)")
  AIDE_TEXTE("    10, rue de la Brasserie")
  AIDE_TEXTE("    87000 LIMOGES (FRANCE)")
  AIDE_TEXTE("")
  AIDE_BOLD ("  E-MAIL:")
  AIDE_TEXTE("")
  AIDE_TEXTE("    dorme@msi.unilim.fr")
  AIDE_TEXTE("    maritaud@ensil.unilim.fr")
  AIDE_TITRE("")
  AIDE_TITRE("USELESS INFORMATION:")
  AIDE_TEXTE("")
  AIDE_TEXTE("   Source size: about 1400 Kb")
  AIDE_TEXTE("   GrafX2's birth date: July 20th 1996")
  AIDE_TITRE("")
  AIDE_TITRE("KNOWN BUGS:")
  AIDE_TEXTE("")
  AIDE_TEXTE("   - <Ctrl>+<C> stops the program when you")
  AIDE_TEXTE("     are entering values. So don't press")
  AIDE_TEXTE("     these keys until we fix this bug.")
  AIDE_TEXTE("   - A few different key combinations return")
  AIDE_TEXTE("     the same code.")
  AIDE_TEXTE("   - A red flash appears at start up when an")
  AIDE_TEXTE("     empty Zip drive is found.")
  AIDE_TEXTE("   - Some functions aren't finished yet.")
  AIDE_TEXTE("     This is normal for a Beta version, so")
  AIDE_TEXTE("     don't worry.")
  AIDE_TEXTE("     But if you still find some bugs in the")
  AIDE_TEXTE("     next versions, then we'd appreciate")
  AIDE_TEXTE("     that you tell us what they exactly are")
  AIDE_TEXTE("     and how and when they occur.")
};
static const char * TableAide2[] =
{
  AIDE_TITRE("REGISTERING ?")
  AIDE_TEXTE("")
  AIDE_TEXTE("  You'll probably say:")
  AIDE_TEXTE("    \"Does this mean that I have to pay?\"")
  AIDE_TEXTE("")
  AIDE_TEXTE("  And we'll reply to you: No...")
  AIDE_TEXTE("")
  AIDE_TITRE("GRAFX 2.00 IS FREEWARE")
  AIDE_TEXTE("")
  AIDE_TEXTE("            ... But, but, but...")
  AIDE_TEXTE("")
  AIDE_TEXTE("  Considering the amount of work (and love)")
  AIDE_TEXTE(" we put in this program, we would extremely")
  AIDE_TEXTE(" appreciate that you \"give\" us something in")
  AIDE_TEXTE(" exchange for the use you make of it.")
  AIDE_TEXTE("  Let's say some money, or a nice picture")
  AIDE_TEXTE(" you drew with GrafX2, or a postcard...")
  AIDE_TEXTE("")
  AIDE_TEXTE("  If you make profit with an image you drew")
  AIDE_TEXTE(" with GrafX2, it would be great if you sent")
  AIDE_TEXTE(" us this picture. If you used it for a game,")
  AIDE_TEXTE(" would be wiser not to send all your graphs,")
  AIDE_TEXTE(" what about sending a copy of the game?")
  AIDE_TITRE("")
  AIDE_TITRE("FREEWARE CONVENTIONS:")
  AIDE_TEXTE("")
  AIDE_TEXTE("   This version of GrafX2 is being released")
  AIDE_TEXTE("  to you as is.")
  AIDE_TEXTE("   All bugs should be reported to either")
  AIDE_TEXTE("  Robinson or X-Man.")
  AIDE_TEXTE("   The authors take no responsibility for")
  AIDE_TEXTE("  lost work, or damaged equipment caused")
  AIDE_TEXTE("  directly or indirectly by this program.")
  AIDE_TEXTE("   GrafX2 cannot be modified in any way")
  AIDE_TEXTE("  without our written consent.")
  AIDE_TEXTE("   Finally, the charging of monetary fees by")
  AIDE_TEXTE("  any unauthorized party for the circulation")
  AIDE_TEXTE("  or use of this utility is expressly")
  AIDE_TEXTE("  forbidden.")
  AIDE_TEXTE("   This means that if you had to pay for")
  AIDE_TEXTE("  getting GrafX2, you have been swindled.")
};
static const char * TableAide3[] =
{
  AIDE_TITRE("GREETINGS:")
  AIDE_TEXTE("")
  AIDE_TEXTE("Our best regards go to...")
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

struct Section_d_aide Table_d_aide[NB_SECTIONS_AIDE] =
{
  {TableAide1, sizeof(TableAide1)/sizeof(const char **)},
  {TableAide2, sizeof(TableAide2)/sizeof(const char **)},
  {TableAide3, sizeof(TableAide3)/sizeof(const char **)}
};
