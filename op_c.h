#ifndef _OP_C_H_
#define _OP_C_H_

#include "struct.h"

//////////////////////////////////////////////// D‚finition des types de base

typedef struct Composantes * Bitmap24B;
typedef byte * Bitmap256;



//////////////////////////////////////// D‚finition d'une table de conversion

typedef struct
{
  int nbb_r; // Nb de bits de pr‚cision sur les rouges
  int nbb_v; // Nb de bits de pr‚cision sur les verts
  int nbb_b; // Nb de bits de pr‚cision sur les bleu

  int rng_r; // Nb de valeurs sur les rouges (= 1<<nbb_r)
  int rng_v; // Nb de valeurs sur les verts  (= 1<<nbb_v)
  int rng_b; // Nb de valeurs sur les bleus  (= 1<<nbb_b)

  int dec_r; // Coefficient multiplicateur d'accŠs dans la table (= nbb_v+nbb_b)
  int dec_v; // Coefficient multiplicateur d'accŠs dans la table (= nbb_b)
  int dec_b; // Coefficient multiplicateur d'accŠs dans la table (= 0)

  int red_r; // Coefficient r‚ducteur de traduction d'une couleur rouge (= 8-nbb_r)
  int red_v; // Coefficient r‚ducteur de traduction d'une couleur verte (= 8-nbb_v)
  int red_b; // Coefficient r‚ducteur de traduction d'une couleur bleue (= 8-nbb_b)

  byte * table;
} Table_conversion;



///////////////////////////////////////// D‚finition d'une table d'occurences

typedef struct
{
  int nbb_r; // Nb de bits de pr‚cision sur les rouges
  int nbb_v; // Nb de bits de pr‚cision sur les verts
  int nbb_b; // Nb de bits de pr‚cision sur les bleu

  int rng_r; // Nb de valeurs sur les rouges (= 1<<nbb_r)
  int rng_v; // Nb de valeurs sur les verts  (= 1<<nbb_v)
  int rng_b; // Nb de valeurs sur les bleus  (= 1<<nbb_b)

  int dec_r; // Coefficient multiplicateur d'accŠs dans la table (= nbb_v+nbb_b)
  int dec_v; // Coefficient multiplicateur d'accŠs dans la table (= nbb_b)
  int dec_b; // Coefficient multiplicateur d'accŠs dans la table (= 0)

  int red_r; // Coefficient r‚ducteur de traduction d'une couleur rouge (= 8-nbb_r)
  int red_v; // Coefficient r‚ducteur de traduction d'une couleur verte (= 8-nbb_v)
  int red_b; // Coefficient r‚ducteur de traduction d'une couleur bleue (= 8-nbb_b)

  int * table;
} Table_occurence;



///////////////////////////////////////// D‚finition d'un ensemble de couleur

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
  byte r,v,b;      // Couleur synth‚tisant l'ensemble
  byte h;          // Chrominance
  byte l;          // Luminosit‚
} Cluster;



//////////////////////////////////////// D‚finition d'un ensemble de clusters

typedef struct
{
  int       nb;
  int       nbmax;
  Cluster * clusters;
} ClusterSet;



///////////////////////////////////////////////////// D‚finition d'un d‚grad‚

typedef struct
{
  int   nbcouleurs; // Nombre de couleurs dans le d‚grad‚
  float min;        // Chrominance minimale du d‚grad‚
  float max;        // Chrominance maximale du d‚grad‚
  float hue;        // Chrominance moyenne du d‚grad‚
} Degrade;



///////////////////////////////////////// D‚finition d'un ensemble de d‚grad‚

typedef struct
{
  int nb;             // Nombre de d‚grad‚s dans l'ensemble
  int nbmax;          // Nombre maximum de d‚grad‚s
  Degrade * degrades; // Les d‚grad‚s
} DegradeSet;



/////////////////////////////////////////////////////////////////////////////
///////////////////////////// M‚thodes de gestion des tables de conversion //
/////////////////////////////////////////////////////////////////////////////

Table_conversion * TC_New(int nbb_r,int nbb_v,int nbb_b);
void TC_Delete(Table_conversion * t);
byte TC_Get(Table_conversion * t,int r,int v,int b);
void TC_Set(Table_conversion * t,int r,int v,int b,byte i);



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
void CS_Generer_TC_et_Palette(ClusterSet * cs,Table_occurence * to,Table_conversion * tc,struct Composantes * palette);



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
