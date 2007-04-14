
byte Readline(word Pos_X,word Pos_Y,char * Chaine,byte Taille_maxi,byte Type_saisie);
// Paramätres:
//   Pos_X, Pos_Y : CoordonnÇes de la saisie dans la fenàtre
//   Chaine       : Chaåne recevant la saisie (et contenant Çventuellement une valeur initiale)
//   Taille_maxi  : Nombre de caractäres logeant dans la zone de saisie
//   Type_saisie  : 0=Chaåne, 1=Nombre, 2=Nom de fichier (12 caractäres)
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
