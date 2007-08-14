#include "op_c.h"
#include "stdio.h"

void OPASM_DitherFS_6123(
  Bitmap256            Destination,  // Pointeur sur le 1er pixel de la ligne
  Bitmap24B            Source,       // Idem mais sur la source
  int                  Largeur,      // Largeur à traiter, =(largeur_image-2), (>0)
  struct Composantes * Palette,      // Palette de l'image destination
  byte *               TableC,       // Table de conversion 24b->8b
  byte                 ReducR,       // 8-Nb_bits_rouges
  byte                 ReducV,       // 8-Nb_bits_verts
  byte                 ReducB,       // 8-Nb_bits_bleus
  byte                 NbbV,         // Nb_bits_verts
  byte                 NbbB)         // Nb_bits_bleus
{
	puts("OPASM_DitherFS_6123 non implémenté!\n");
}
  
void OPASM_DitherFS_623(
  Bitmap256            Destination,  // Pointeur sur le 1er pixel de la ligne
  Bitmap24B            Source,       // Idem mais sur la source
  int                  Largeur,      // Largeur à traiter, =(largeur_image-2), (>0)
  struct Composantes * Palette,      // Palette de l'image destination
  byte *               TableC,       // Table de conversion 24b->8b
  byte                 ReducR,       // 8-Nb_bits_rouges
  byte                 ReducV,       // 8-Nb_bits_verts
  byte                 ReducB,       // 8-Nb_bits_bleus
  byte                 NbbV,         // Nb_bits_verts
  byte                 NbbB)         // Nb_bits_bleus
{
	puts("OPASM_DitherFS_623 non implémenté!\n");
}

void OPASM_DitherFS_12(
  Bitmap256            Destination,  // Pointeur sur le 1er pixel de la ligne
  Bitmap24B            Source,       // Idem mais sur la source
  int                  Largeur,      // Largeur à traiter, =(largeur_image-2), (>0)
  struct Composantes * Palette,      // Palette de l'image destination
  byte *               TableC,       // Table de conversion 24b->8b
  byte                 ReducR,       // 8-Nb_bits_rouges
  byte                 ReducV,       // 8-Nb_bits_verts
  byte                 ReducB,       // 8-Nb_bits_bleus
  byte                 NbbV,         // Nb_bits_verts
  byte                 NbbB)         // Nb_bits_bleus
{
	puts("OPASM_DitherFS_12 non implémenté!\n");
}

void OPASM_DitherFS_6(
  Bitmap256            Destination,  // Pointeur sur le 1er pixel de la ligne
  Bitmap24B            Source,       // Idem mais sur la source
  int                  Largeur,      // Largeur à traiter, =(largeur_image-1), (>0)
  struct Composantes * Palette,      // Palette de l'image destination
  byte *               TableC,       // Table de conversion 24b->8b
  byte                 ReducR,       // 8-Nb_bits_rouges
  byte                 ReducV,       // 8-Nb_bits_verts
  byte                 ReducB,       // 8-Nb_bits_bleus
  byte                 NbbV,         // Nb_bits_verts
  byte                 NbbB)         // Nb_bits_bleus
{
	puts("OPASM_DitherFS_6 non implémenté!\n");
}

  void OPASM_DitherFS(
  Bitmap256 Destination,  // Pointeur sur le pixel
  Bitmap24B Source,       // Idem mais sur la source
  byte *    TableC,       // Table de conversion 24b->8b
  byte      ReducR,       // 8-Nb_bits_rouges
  byte      ReducV,       // 8-Nb_bits_verts
  byte      ReducB,       // 8-Nb_bits_bleus
  byte      NbbV,         // Nb_bits_verts
  byte      NbbB)         // Nb_bits_bleus
{
	puts("OPASM_DitherFS non implémenté!\n");
}

void OPASM_DitherFS_2(
  Bitmap256            Destination,  // Pointeur sur le 1er pixel de la colonne
  Bitmap24B            Source,       // Idem mais sur la source
  int                  Hauteur,      // Hauteur à traiter, =(hauteur_image-1), (>0)
  struct Composantes * Palette,      // Palette de l'image destination
  byte *               TableC,       // Table de conversion 24b->8b
  byte                 ReducR,       // 8-Nb_bits_rouges
  byte                 ReducV,       // 8-Nb_bits_verts
  byte                 ReducB,       // 8-Nb_bits_bleus
  byte                 NbbV,         // Nb_bits_verts
  byte                 NbbB)         // Nb_bits_bleus
{
	puts("OPASM_DitherFS_2 non implémenté!\n");
}

  void OPASM_Split_cluster_Rouge(
  int * tableO, // Table d'occurences
  int   rmin,   // rmin << rdec
  int   vmin,   // vmin << vdec
  int   bmin,   // bmin << bdec
  int   rmax,   // rmax << rdec
  int   vmax,   // vmin << vdec
  int   bmax,   // bmin << bdec
  int   rinc,   // Incrémentation sur les rouges 1 << rdec
  int   vinc,   // Incrémentation sur les verts  1 << vdec
  int   binc,   // Incrémentation sur les bleus  1 << bdec
  int   limite, // Nombre d'occurences minimales
  int   rdec,   // rdec
  int * rouge)  // Valeur du rouge atteignant la limite
{
	puts("OPASM_Split_cluster_Rouge non implémenté!\n");
}

void OPASM_Split_cluster_Vert(
  int * tableO, // Table d'occurences
  int   rmin,   // rmin << rdec
  int   vmin,   // vmin << vdec
  int   bmin,   // bmin << bdec
  int   rmax,   // rmax << rdec
  int   vmax,   // vmin << vdec
  int   bmax,   // bmin << bdec
  int   rinc,   // Incrémentation sur les rouges 1 << rdec
  int   vinc,   // Incrémentation sur les verts  1 << vdec
  int   binc,   // Incrémentation sur les bleus  1 << bdec
  int   limite, // Nombre d'occurences minimales
  int   vdec,   // vdec
  int * vert)   // Valeur du vert atteignant la limite
{
	puts("OPASM_Split_cluster_Vert non implémenté!\n");
}

void OPASM_Split_cluster_Bleu(
  int * tableO, // Table d'occurences
  int   rmin,   // rmin << rdec
  int   vmin,   // vmin << vdec
  int   bmin,   // bmin << bdec
  int   rmax,   // rmax << rdec
  int   vmax,   // vmin << vdec
  int   bmax,   // bmin << bdec
  int   rinc,   // Incrémentation sur les rouges 1 << rdec
  int   vinc,   // Incrémentation sur les verts  1 << vdec
  int   binc,   // Incrémentation sur les bleus  1 << bdec
  int   limite, // Nombre d'occurences minimales
  int   bdec,   // bdec
  int * bleu)   // Valeur du bleu atteignant la limite
{
	puts("OPASM_Split_cluster_Bleu non implémenté!\n");
}

  void OPASM_Compter_occurences(
  int *     Destination,  // Pointeur sur la table d'occurences
  Bitmap24B Source,       // Pointeur sur l'image
  int       Taille,       // Nombre de pixels dans l'image
  byte      ReducR,       // 8-Nb_bits_rouges
  byte      ReducV,       // 8-Nb_bits_verts
  byte      ReducB,       // 8-Nb_bits_bleus
  byte      NbbV,         // Nb_bits_verts
  byte      NbbB)         // Nb_bits_bleus
{
	puts("OPASM_Compter_occurences non implémenté!\n");
}

void OPASM_Analyser_cluster(
  int * TableO, // Table d'occurences
  int * rmin,   // rmin << rdec
  int * vmin,   // vmin << vdec
  int * bmin,   // bmin << bdec
  int * rmax,   // rmax << rdec
  int * vmax,   // vmax << vdec
  int * bmax,   // bmax << bdec
  int   rdec,   // rdec
  int   vdec,   // vdec
  int   bdec,   // bdec
  int   rinc,   // Incrémentation sur les rouges 1 << rdec
  int   vinc,   // Incrémentation sur les verts  1 << vdec
  int   binc,   // Incrémentation sur les bleus  1 << bdec
  int * Nbocc)  // Nombre d'occurences
{
	puts("OPASM_Analyser_cluster non implémenté!\n");
}
