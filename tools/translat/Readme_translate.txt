List functions and global variables
nm -Cg --defined-only obj/win32/*.o | grep "^0" | cut -c 12- | sort -u

List variables, parameters, struct members. Requires french.txt to remove duplicates.
perl -ne 'while ($_ =~ /\b(int|char|word|byte|float|double|short|long|Fenetre_Bouton_dropdown|Composantes|S_Mode_video|Fenetre_Bouton_normal|Fenetre_Bouton_palette|Fenetre_Bouton_scroller|Fenetre_Bouton_special|T_Shade|T_Degrade_Tableau|Element_de_liste_de_fileselect|S_ConfigTouche|T_Format|Table_conversion|Table_occurence|cluster|ClusterSet|Degrade|DegradeSet|T_Palette|Bouton_dropdown_choix|T_TABLEAIDE|Section_d_aide|T_Degrade_Tableau|T_Shade|Config_Mode_video|Config_Header|Config_Chunk|Config_Infos_touche|S_Page|S_Liste_de_pages|T_FONTE|statfs|S_Libelle_touche|POLYGON_EDGE)\s+(\*)?(\s)*(\w+)/g) { print "$4\n";}' *.c *.h | sort -u | comm -2 -3 - french.txt

List macros (not starting with _)
perl -ne 'if ($_ =~ /^\s*#define ([^_]\w+)/) { print "$1\n"; }' *.h *.c | sort -u

Translate some source files
./translate.pl *.c *.h
