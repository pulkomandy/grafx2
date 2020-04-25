/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

	Copyright owned by various GrafX2 authors, see COPYRIGHT.txt for details.

    Grafx2 is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2
    of the License.

    Grafx2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/

//////////////////////////////////////////////////////////////////////////////
///@file op_c.h
/// Color reduction and color conversion (24b->8b, RGB<->HSL).
/// This is called op_c because half of the process was originally 
/// coded in op_asm, in assembler.
//////////////////////////////////////////////////////////////////////////////

#ifndef _OP_C_H_
#define _OP_C_H_

#include "struct.h"
#include "colorred.h"

//////////////////////////////////////////////// Définition des types de base

typedef T_Components * T_Bitmap24B;
typedef byte * T_Bitmap256;


///////////////////////////////////////// Définition d'une table d'occurences

/**
 * Occurence table.
 *
 * This table is used to count the occurrence of an (RGB) pixel value in the
 * source 24bit image. These count are then used by the median cut algorithm to
 * decide which cluster to split.
 */
typedef struct
{
  int nbb_r; // Nb de bits de précision sur les rouges
  int nbb_g; // Nb de bits de précision sur les verts
  int nbb_b; // Nb de bits de précision sur les bleu

  int rng_r; // Nb de valeurs sur les rouges (= 1<<nbb_r)
  int rng_g; // Nb de valeurs sur les verts  (= 1<<nbb_g)
  int rng_b; // Nb de valeurs sur les bleus  (= 1<<nbb_b)

  int dec_r; // Coefficient multiplicateur d'accès dans la table (= nbb_g+nbb_b)
  int dec_g; // Coefficient multiplicateur d'accès dans la table (= nbb_b)
  int dec_b; // Coefficient multiplicateur d'accès dans la table (= 0)

  int red_r; // Coefficient réducteur de traduction d'une couleur rouge (= 8-nbb_r)
  int red_g; // Coefficient réducteur de traduction d'une couleur verte (= 8-nbb_g)
  int red_b; // Coefficient réducteur de traduction d'une couleur bleue (= 8-nbb_b)

  int * table;
} T_Occurrence_table;



///////////////////////////////////////// Définition d'un ensemble de couleur

/**
 * informations used while median-cutting.
 */
struct S_Cluster_CutData
{
  int volume; ///< volume of narrow covering (without margins where there are no pixels)
  int sqdiag; ///< square of diagonal length (without margins)
 
  /// Widest component : 0 red, 1 green, 2 blue
  byte plus_large;
};

/**
 * information used while color reducing
 */
struct S_Cluster_PalData
{
  byte r,g,b;      ///< color synthétisant l'ensemble
  byte h;          ///< Hue / Chrominance
  byte l;          ///< Luminosity / Luminosité
};

union U_Cluster_Data
{
	struct S_Cluster_CutData cut;
	struct S_Cluster_PalData pal;
};

/**
 * Color cluster
 */
typedef struct S_Cluster
{
  struct S_Cluster* next;
  int occurences; // Numbers of pixels in picture part of this cluster
  
  // Narrow covering (remove margins that don't hold any pixel)
  byte rmin,rmax;
  byte vmin,vmax;
  byte bmin,bmax;
  
  // Wide covering (how far it extends before touching another cluster wide covering)
  byte Rmin,Rmax;
  byte Gmin,Vmax;
  byte Bmin,Bmax;
 
  union U_Cluster_Data data;
} T_Cluster;



//////////////////////////////////////// Définition d'un ensemble de clusters

/**
 * Cluster set
 */
typedef struct
{
  int       nb;
  int       nb_max;
  T_Cluster * clusters;
} T_Cluster_set;



///////////////////////////////////////////////////// Définition d'un dégradé

/**
 * Gradient
 */
typedef struct
{
  int   nb_colors; // Nombre de couleurs dans le dégradé
  float min;        // Chrominance minimale du dégradé
  float max;        // Chrominance maximale du dégradé
  float hue;        // Chrominance moyenne du dégradé
} T_Gradient;



///////////////////////////////////////// Définition d'un ensemble de dégradé

typedef struct
{
  int nb;             // Nombre de dégradés dans l'ensemble
  int nb_max;          // Nombre maximum de dégradés
  T_Gradient * gradients; // Les dégradés
} T_Gradient_set;

void RGB_to_HSL(int r, int g,int b, byte* h, byte*s, byte* l);
void HSL_to_RGB(byte h, byte s, byte l, byte* r, byte* g, byte* b);

long Perceptual_lightness(T_Components *color);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Méthodes de gestion des tables d'occurence //
/////////////////////////////////////////////////////////////////////////////

void OT_init(T_Occurrence_table * t);
T_Occurrence_table * OT_new(int nbb_r,int nbb_g,int nbb_b);
void OT_delete(T_Occurrence_table * t);
int OT_get(T_Occurrence_table * t,byte r,byte g,byte b);
void OT_inc(T_Occurrence_table * t,byte r,byte g,byte b);
void OT_count_occurrences(T_Occurrence_table * t,T_Bitmap24B image,int size);



/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Méthodes de gestion des clusters //
/////////////////////////////////////////////////////////////////////////////

void Cluster_pack(T_Cluster * c,const T_Occurrence_table * const to);
void Cluster_split(T_Cluster * c,T_Cluster * c1,T_Cluster * c2,int hue,const T_Occurrence_table * const to);
void Cluster_compute_hue(T_Cluster * c,T_Occurrence_table * to);



/////////////////////////////////////////////////////////////////////////////
//////////////////////////// Méthodes de gestion des ensembles de clusters //
/////////////////////////////////////////////////////////////////////////////

void CS_Init(T_Cluster_set * cs,T_Occurrence_table * to);
T_Cluster_set * CS_New(int nbmax,T_Occurrence_table * to);
void CS_Delete(T_Cluster_set * cs);
void CS_Get(T_Cluster_set * cs,T_Cluster ** c);
int CS_Set(T_Cluster_set * cs,T_Cluster * c);
int CS_Generate(T_Cluster_set * cs,const T_Occurrence_table * const to, CT_Tree* colorTree);
void CS_Compute_colors(T_Cluster_set * cs,T_Occurrence_table * to);
void CS_Generate_color_table_and_palette(T_Cluster_set * cs,CT_Tree* tc,T_Components * palette, T_Occurrence_table * to);

/////////////////////////////////////////////////////////////////////////////
//////////////////////////// Méthodes de gestion des ensembles de dégradés //
/////////////////////////////////////////////////////////////////////////////

void GS_Init(T_Gradient_set * ds,T_Cluster_set * cs);
T_Gradient_set * GS_New(T_Cluster_set * cs);
void GS_Delete(T_Gradient_set * ds);
void GS_Generate(T_Gradient_set * ds,T_Cluster_set * cs);

int Convert_24b_bitmap_to_256(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette);
#endif
