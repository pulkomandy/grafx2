/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2007 Adrien Destugues
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
#ifndef _OP_C_H_
#define _OP_C_H_

#include "struct.h"

//////////////////////////////////////////////// Définition des types de base

typedef struct Composantes * Bitmap24B;
typedef byte * Bitmap256;



//////////////////////////////////////// Définition d'une table de conversion

typedef struct
{
  int nbb_r; // Nb de bits de précision sur les rouges
  int nbb_v; // Nb de bits de précision sur les verts
  int nbb_b; // Nb de bits de précision sur les bleu

  int rng_r; // Nb de valeurs sur les rouges (= 1<<nbb_r)
  int rng_v; // Nb de valeurs sur les verts  (= 1<<nbb_v)
  int rng_b; // Nb de valeurs sur les bleus  (= 1<<nbb_b)

  int dec_r; // Coefficient multiplicateur d'accès dans la table (= nbb_v+nbb_b)
  int dec_v; // Coefficient multiplicateur d'accès dans la table (= nbb_b)
  int dec_b; // Coefficient multiplicateur d'accès dans la table (= 0)

  int red_r; // Coefficient réducteur de traduction d'une couleur rouge (= 8-nbb_r)
  int red_v; // Coefficient réducteur de traduction d'une couleur verte (= 8-nbb_v)
  int red_b; // Coefficient réducteur de traduction d'une couleur bleue (= 8-nbb_b)

  byte * table;
} Table_conversion;



///////////////////////////////////////// Définition d'une table d'occurences

typedef struct
{
  int nbb_r; // Nb de bits de précision sur les rouges
  int nbb_v; // Nb de bits de précision sur les verts
  int nbb_b; // Nb de bits de précision sur les bleu

  int rng_r; // Nb de valeurs sur les rouges (= 1<<nbb_r)
  int rng_v; // Nb de valeurs sur les verts  (= 1<<nbb_v)
  int rng_b; // Nb de valeurs sur les bleus  (= 1<<nbb_b)

  int dec_r; // Coefficient multiplicateur d'accès dans la table (= nbb_v+nbb_b)
  int dec_v; // Coefficient multiplicateur d'accès dans la table (= nbb_b)
  int dec_b; // Coefficient multiplicateur d'accès dans la table (= 0)

  int red_r; // Coefficient réducteur de traduction d'une couleur rouge (= 8-nbb_r)
  int red_v; // Coefficient réducteur de traduction d'une couleur verte (= 8-nbb_v)
  int red_b; // Coefficient réducteur de traduction d'une couleur bleue (= 8-nbb_b)

  int * table;
} Table_occurence;



///////////////////////////////////////// Définition d'un ensemble de couleur

typedef struct
{
  int occurences; // Nb total d'occurences des couleurs de l'ensemble

  // Grande couverture
  byte Rmin,Rmax;
  byte Vmin,Vmax;
  byte Bmin,Bmax;

  // Couverture minimale
  byte rmin,rmax;
  byte vmin,vmax;
  byte bmin,bmax;

  byte plus_large; // Composante ayant la plus grande variation (0=Rouge,1=Vert,2=Bleu)
  byte r,v,b;      // Couleur synthétisant l'ensemble
  byte h;          // Chrominance
  byte l;          // Luminosité
} Cluster;



//////////////////////////////////////// Définition d'un ensemble de clusters

typedef struct
{
  int       nb;
  int       nbmax;
  Cluster * clusters;
} ClusterSet;



///////////////////////////////////////////////////// Définition d'un dégradé

typedef struct
{
  int   nbcouleurs; // Nombre de couleurs dans le dégradé
  float min;        // Chrominance minimale du dégradé
  float max;        // Chrominance maximale du dégradé
  float hue;        // Chrominance moyenne du dégradé
} Degrade;



///////////////////////////////////////// Définition d'un ensemble de dégradé

typedef struct
{
  int nb;             // Nombre de dégradés dans l'ensemble
  int nbmax;          // Nombre maximum de dégradés
  Degrade * degrades; // Les dégradés
} DegradeSet;



/////////////////////////////////////////////////////////////////////////////
///////////////////////////// Méthodes de gestion des tables de conversion //
/////////////////////////////////////////////////////////////////////////////

Table_conversion * TC_New(int nbb_r,int nbb_v,int nbb_b);
void TC_Delete(Table_conversion * t);
byte TC_Get(Table_conversion * t,int r,int v,int b);
void TC_Set(Table_conversion * t,int r,int v,int b,byte i);

void rgb2hl(int r, int v,int b, byte* h, byte*l, byte* s);
void HLStoRGB(byte h, byte l, byte s, byte* r, byte* g, byte* b);


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// M‚thodes de gestion des tables d'occurence //
/////////////////////////////////////////////////////////////////////////////

void TO_Init(Table_occurence * t);
Table_occurence * TO_New(int nbb_r,int nbb_v,int nbb_b);
void TO_Delete(Table_occurence * t);
int  TO_Get(Table_occurence * t,int r,int v,int b);
void TO_Set(Table_occurence * t,int r,int v,int b,int i);
void TO_Inc(Table_occurence * t,int r,int v,int b);
void TO_Compter_occurences(Table_occurence * t,Bitmap24B image,int taille);



/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// M‚thodes de gestion des clusters //
/////////////////////////////////////////////////////////////////////////////

void Cluster_Analyser(Cluster * c,Table_occurence * to);
void Cluster_Split(Cluster * c,Cluster * c1,Cluster * c2,int teinte,Table_occurence * to);
void Cluster_Calculer_teinte(Cluster * c,Table_occurence * to);



/////////////////////////////////////////////////////////////////////////////
//////////////////////////// M‚thodes de gestion des ensembles de clusters //
/////////////////////////////////////////////////////////////////////////////

void CS_Init(ClusterSet * cs,Table_occurence * to);
ClusterSet * CS_New(int nbmax,Table_occurence * to);
void CS_Delete(ClusterSet * cs);
void CS_Get(ClusterSet * cs,Cluster * c);
void CS_Set(ClusterSet * cs,Cluster * c);
void CS_Generer(ClusterSet * cs,Table_occurence * to);
void CS_Calculer_teintes(ClusterSet * cs,Table_occurence * to);
void CS_Generer_TC_et_Palette(ClusterSet * cs,Table_conversion * tc,struct Composantes * palette);

/////////////////////////////////////////////////////////////////////////////
//////////////////////////// M‚thodes de gestion des ensembles de d‚grad‚s //
/////////////////////////////////////////////////////////////////////////////

void DS_Init(DegradeSet * ds,ClusterSet * cs);
DegradeSet * DS_New(ClusterSet * cs);
void DS_Delete(DegradeSet * ds);
void DS_Generer(DegradeSet * ds,ClusterSet * cs);



// Convertie avec le plus de pr‚cision possible une image 24b en 256c
// Renvoie s'il y a eu une erreur ou pas..
int Convert_bitmap_24B_to_256(Bitmap256 Dest,Bitmap24B Source,int largeur,int hauteur,struct Composantes * palette);



#endif
