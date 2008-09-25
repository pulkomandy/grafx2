
byte Readline(word Pos_X,word Pos_Y,char * Chaine,byte Taille_maxi,byte Type_saisie);
// Paramètres:
//   Pos_X, Pos_Y : Coordonnées de la saisie dans la fenêtre
//   Chaine       : Chaîne recevant la saisie (et contenant éventuellement une valeur initiale)
//   Taille_maxi  : Nombre de caractères logeant dans la zone de saisie
//   Type_saisie  : 0=Chaîne, 1=Nombre, 2=Nom de fichier (255 caractères réels)
// Sortie:
//   0: Sortie par annulation (Esc.) / 1: sortie par acceptation (Return)
