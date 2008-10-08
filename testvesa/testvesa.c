/*  Grafx2 - The Ultimate 256-color bitmap paint program

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
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vesa.h"



char * Version_to_string(word Version)
{
  static char Chaine[20];

  sprintf(Chaine,"%d",(Version>>8));
  strcat(Chaine,".");
  sprintf(Chaine+strlen(Chaine),"%d",(Version & 0xFF));

  return Chaine;
}



void Lire_infos_VESA(void)
{
  struct S_Buffer_VESA      Buffer_VESA;
  struct S_Buffer_mode_VESA Buffer_mode_VESA;
  word   Retour;
  int    Nombre_de_modes;
  word * Liste_des_modes;
  int    Offset1;
  int    Offset2;
  char   Libelle_du_nombre_de_couleurs[16];
  int    Nombre_de_bits;

  Retour=Get_VESA_info(&Buffer_VESA);

  if (Retour==0x004F)
  {
    if (memcmp(Buffer_VESA.Signature,"VESA",4)==0)
    {
      printf("VESA version : %s\n",Version_to_string(Buffer_VESA.Version));
      printf("Builder      : %s\n",Buffer_VESA.Fabricant);
      printf("Capabilities : %d\n",Buffer_VESA.Capacite);
      printf("Video memory : %dK\n",Buffer_VESA.Memoire*64);

      if (Buffer_VESA.Version>=0x0200)
      {
        printf("OEM version  : %s\n",Version_to_string(Buffer_VESA.Version_bios));
        printf("Vendor       : %s\n",Buffer_VESA.Vendeur);
        printf("Product      : %s\n",Buffer_VESA.Produit);
        printf("Revision     : %s\n",Buffer_VESA.Revision);
      }

      // Comptage du nombre de modes:
      Nombre_de_modes=0;
      for (Offset1=0;Buffer_VESA.Liste_des_modes[Offset1]!=0xFFFF;Offset1++)
        Nombre_de_modes++;

      // Allocation d'une liste duplique des modes:
      Liste_des_modes=(word *)malloc(sizeof(word)*Nombre_de_modes);

      // Copie de la liste des modes originale vers la duplique:
      memcpy(Liste_des_modes,Buffer_VESA.Liste_des_modes,sizeof(word)*Nombre_de_modes);

      // Affichage de la liste des modes:
      for (Offset1=0;Offset1<Nombre_de_modes;Offset1++)
      {
        // Lecture des infos du mode:
        Retour=Get_VESA_mode_info(Liste_des_modes[Offset1],&Buffer_mode_VESA);

        // Affichage des infos de ce mode:

        if (Retour==0x004F)
        {
          if (Buffer_mode_VESA.Nb_bits_par_pixel<=8)
          {
            Nombre_de_bits=0;
            sprintf(Libelle_du_nombre_de_couleurs,"%d col.",1<<Buffer_mode_VESA.Nb_bits_par_pixel);
          }
          else
          {
            Nombre_de_bits=Buffer_mode_VESA.Taille_masque_rouge+
                           Buffer_mode_VESA.Taille_masque_vert+
                           Buffer_mode_VESA.Taille_masque_bleu;

            switch (Nombre_de_bits)
            {
              case 15:
                strcpy(Libelle_du_nombre_de_couleurs,"32K");
                break;
              case 16:
                strcpy(Libelle_du_nombre_de_couleurs,"64K");
                break;
              case 24:
                if (Buffer_mode_VESA.Nb_bits_par_pixel==24)
                  strcpy(Libelle_du_nombre_de_couleurs,"16M");
                else
                  strcpy(Libelle_du_nombre_de_couleurs,"16M 32-bits");
                break;
              default:
                sprintf(Libelle_du_nombre_de_couleurs,"%d col.",1<<Nombre_de_bits);
            }
          }

          printf("\nMode : %Xh\t(",Liste_des_modes[Offset1]);

          if (Buffer_mode_VESA.Attributs&16)
            printf("Graphmode: ");
          else
            printf("Textmode : ");

          printf("%dx%d - %s - ",Buffer_mode_VESA.Largeur,
                 Buffer_mode_VESA.Hauteur,Libelle_du_nombre_de_couleurs);

          switch (Buffer_mode_VESA.Attributs&192)
          {
            case   0 : printf("Banked)\n"); break;
            case 128 : printf("Banked/LFB)\n"); break;
            case 192 : printf("LFB)\n"); break;
            default  : printf("BUGGED!)\n");
          }

          printf("  Attributes          : %Xh\n",Buffer_mode_VESA.Attributs);
          printf("  Window A attributes : %Xh\n",Buffer_mode_VESA.Attributs_fenetre_A);
          printf("  Window B attributes : %Xh\n",Buffer_mode_VESA.Attributs_fenetre_B);
          printf("  Granularity         : %dK\n",Buffer_mode_VESA.Granularite);
          printf("  Window size         : %dK\n",Buffer_mode_VESA.Taille_fenetres);
          printf("  Window A segment    : %Xh\n",Buffer_mode_VESA.Segment_fenetre_A);
          printf("  Window B segment    : %Xh\n",Buffer_mode_VESA.Segment_fenetre_B);
          printf("  WinFunction pointer : %p\n",Buffer_mode_VESA.WinFuncPtr);
          printf("  Bytes per line      : %d\n",Buffer_mode_VESA.Octets_par_ligne);
          printf("  Width               : %d\n",Buffer_mode_VESA.Largeur);
          printf("  Height              : %d\n",Buffer_mode_VESA.Hauteur);
          printf("  Character width     : %d\n",Buffer_mode_VESA.Largeur_de_char);
          printf("  Character height    : %d\n",Buffer_mode_VESA.Hauteur_de_char);
          printf("  Number of planes    : %d\n",Buffer_mode_VESA.Nb_plans);
          printf("  Bits per pixel      : %d\n",Buffer_mode_VESA.Nb_bits_par_pixel);
          printf("  Number of banks     : %d\n",Buffer_mode_VESA.Nb_banques);
          printf("  Memory model        : %d\n",Buffer_mode_VESA.Modele_de_memoire);
          printf("  Bank size           : %d\n",Buffer_mode_VESA.Taille_des_banques);
          printf("  Number of pages     : %d\n",Buffer_mode_VESA.Nombre_de_pages);

          if (Nombre_de_bits)
          {
            printf("  Red mask size       : %d\n",Buffer_mode_VESA.Taille_masque_rouge);
            printf("  Red mask position   : %d\n",Buffer_mode_VESA.Pos_masque_rouge);
            printf("  Green mask size     : %d\n",Buffer_mode_VESA.Taille_masque_vert);
            printf("  Green mask position : %d\n",Buffer_mode_VESA.Pos_masque_vert);
            printf("  Blue mask size      : %d\n",Buffer_mode_VESA.Taille_masque_bleu);
            printf("  Blue mask position  : %d\n",Buffer_mode_VESA.Pos_masque_bleu);
            printf("  Reserved mask size  : %d\n",Buffer_mode_VESA.Taille_masque_res);
            printf("  Reserved mask pos.  : %d\n",Buffer_mode_VESA.Pos_masque_res);
          }

          printf("  Direct screen mode  : %d\n",Buffer_mode_VESA.Direct_screen_mode);
          printf("  LFB address         : %p\n",Buffer_mode_VESA.Adresse_LFB);
          printf("  Offscreen address   : %p\n",Buffer_mode_VESA.Adresse_offscreen);
          printf("  Offscreen size      : %dK\n",Buffer_mode_VESA.Taille_offscreen);
        }
        else
        {
          printf("\nMode : %Xh",Liste_des_modes[Offset1]);
          printf("  Mode not supported (not enough memory or bad monitor)\n");
        }
      }

      // Libration de la liste duplique:
      free(Liste_des_modes);
    }
    else
      printf("VESA bugged.\n");
  }
  else
  {
    if (Retour==0xFF00)
      printf("Memory allocation impossible.\n");
    else
      printf("No VESA supported: %d\n",Retour);
  }
}



int main(void)
{
  printf("\nษอออออออออออออออออออออออออออออออออออออออออออออออออออออออป");
  printf("\nบÛฒฑฐ VESA tester - Copyright (c)1997 Sunset Design ฐฑฒÛบ");
  printf("\nศอออออออออออออออออออออออออออออออออออออออออออออออออออออออผ\n\n");
  Lire_infos_VESA();
  return 0;
}
