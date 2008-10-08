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
#include <string.h>
#include "vesalfb.h"


// - -- --- ----\/\ Detection du support VESA sur la machine: /\/---- --- -- -

void Support_VESA(void)
{
  struct S_Buffer_VESA      Buffer_VESA;
  word   Retour;

  VESA_Erreur=0;

  // Lecture des infos du VESA:
  Retour=Get_VESA_info(&Buffer_VESA);
  if (Retour==0x004F)
  {
    // Les interruptions VESA sont interprˆt‚es par la carte vid‚o

    if (memcmp(Buffer_VESA.Signature,"VESA",4)==0)
    {
      // La signature "VESA" est bien pr‚sente
      VESA_Liste_des_modes =Buffer_VESA.Liste_des_modes;
      VESA_Version_Unite   =(Buffer_VESA.Version >> 8);
      VESA_Version_Decimale=(Buffer_VESA.Version & 0xFF);
      strncpy(VESA_Constructeur,Buffer_VESA.Fabricant,TAILLE_NOM_CONSTRUCTEUR);
      VESA_Constructeur[TAILLE_NOM_CONSTRUCTEUR]='\0';
      VESA_Taille_memoire  =Buffer_VESA.Memoire;
    }
    else
    {
      // Pas de signature VESA sur la carte
      VESA_Erreur=1;
    }

  }
  else
  {
    // La carte vid‚o n'a jamais entendu parler du VESA
    VESA_Erreur=1;
  }
}



// - -- --- -----\/\ Detection du support VESA pour un mode: /\/----- --- -- -

void Mode_VESA_supporte(word Mode,int Indice_mode)
{
  struct S_Buffer_VESA      Buffer_VESA;
  struct S_Buffer_mode_VESA Buffer_mode_VESA;
  word   Retour;
  int    Code_fenetres=0;
  int    Granularite=0;
  int    Indice;
  int    Taille;

  VESA_Erreur=0;

  /*
    On va commencer par regarder si le mode n'est pas dans la liste de ceux
    support‚s par la carte. Pour cela, il nous faut redemander cette liste
    car certaines cartes la "perdent" d'un appel sur l'autre.
  */

  // Lecture des infos du VESA:
  Retour=Get_VESA_info(&Buffer_VESA);
  if (Retour==0x004F)
  {
    // Les interruptions VESA sont interprˆt‚es par la carte vid‚o

    Taille=65536L*(int)Buffer_VESA.Memoire;

    // On regarde si le mode se trouve dans la liste
    for (Indice=0;Buffer_VESA.Liste_des_modes[Indice]!=0xFFFF;Indice++)
      if (Buffer_VESA.Liste_des_modes[Indice]==Mode)
        break;

    if (Buffer_VESA.Liste_des_modes[Indice]==Mode)
    {
      // Le mode est dans la liste

      // Lecture des infos du mode VESA:
      Retour=Get_VESA_mode_info(Mode,&Buffer_mode_VESA);

      if (Retour==0x004F)
      {
        if ((Buffer_mode_VESA.Attributs & 1)==0)
          VESA_Erreur=1;

        // On calcule le facteur de granularit‚:
        switch(Buffer_mode_VESA.Granularite)
        {
          case 64 :
            Granularite=0;
            break;
          case 32 :
            Granularite=1;
            break;
          case 16 :
            Granularite=2;
            break;
          case 8 :
            Granularite=3;
            break;
          case 4 :
            Granularite=4;
            break;
          case 2 :
            Granularite=5;
            break;
          case 1 :
            Granularite=6;
            break;
          default :
            VESA_Erreur=1;
        };

        // On calcule le code des fenˆtres:
        //  0 = Lecture & ‚criture dans A
        //  1 = Lecture & ‚criture dans B
        //  2 = Lecture dans A et ‚criture dans B
        //  3 = Lecture dans B et ‚criture dans A

        if ((Buffer_mode_VESA.Attributs_fenetre_A & 7)==7)
          // La fenˆtre A suffit … tout faire (lecture & ‚criture)
          Code_fenetres=0;
        else
        {
          if ((Buffer_mode_VESA.Attributs_fenetre_B & 7)==7)
            // La fenˆtre B suffit … tout faire (lecture & ‚criture)
            Code_fenetres=1;
          else
          {
            // La fenˆtre B ne suffira pas … tout faire

            if ( ((Buffer_mode_VESA.Attributs_fenetre_A & 3)==3) &&
                 ((Buffer_mode_VESA.Attributs_fenetre_B & 5)==5) )
              // La fenˆtre A est lisible et la fenˆtre B inscriptible
              Code_fenetres=2;
            else
            {
              if ( ((Buffer_mode_VESA.Attributs_fenetre_B & 3)==3) &&
                   ((Buffer_mode_VESA.Attributs_fenetre_A & 5)==5) )
                // La fenˆtre B est lisible et la fenˆtre A inscriptible
                Code_fenetres=3;
              else
                VESA_Erreur=1;
            }
          }
        }

        // On v‚rifie que la taille des fenˆtres soit bien de 64Ko:
        if (Buffer_mode_VESA.Taille_fenetres!=64)
          VESA_Erreur=1;

        if ( (VESA_Erreur==0) ||
             ((Buffer_mode_VESA.Attributs & 128) &&
              (Buffer_mode_VESA.Adresse_LFB!=0)) )
        {
          VESA_Erreur=0;
          VESA_Mode_Infos[Indice_mode].Granularite=Granularite;
          VESA_Mode_Infos[Indice_mode].Code_fenetres=Code_fenetres;
          Get_VESA_protected_mode_WinFuncPtr();
          VESA_Mode_Infos[Indice_mode].WinFuncPtr=VESA_WinFuncPtr;
          if (Buffer_mode_VESA.Attributs & 128)
          {
            // LFB support‚
            VESA_Mode_Infos[Indice_mode].Adresse_physique_LFB=Buffer_mode_VESA.Adresse_LFB;
            VESA_Mode_Infos[Indice_mode].Taille_LFB=Taille;
          }
          else
            VESA_Mode_Infos[Indice_mode].Adresse_physique_LFB=0;
        }
      }
      else
      {
        // Le mode est renseign‚ dans la liste mais n'est pas support‚
        VESA_Erreur=1;
      }
    }
    else
    {
      // Le mode n'est pas dans la liste
      VESA_Erreur=1;
    }
  }
  else
  {
    // La carte vid‚o n'a jamais entendu parler du VESA
    VESA_Erreur=1;
  }

  // Si le mode n'est pas support‚, on disable les modes vid‚os bas‚s sur
  // ce mode VESA:
  if (VESA_Erreur!=0)
    for (Indice=0;Indice<NB_MODES_VIDEO;Indice++)
      if (Mode_video[Indice].Mode_VESA_de_base==Mode)
        Mode_video[Indice].Etat+=128;
}


