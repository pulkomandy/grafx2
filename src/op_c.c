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
#include <assert.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

#include "op_c.h"
#include "errors.h"
#include "colorred.h"

// If GRAFX2_QUANTIZE_CLUSTER_POPULATION_SPLIT is defined,
// the clusters are splitted in two half of equal (pixel) population.
// Otherwise, they are splitted in two half of equal volume.
#define GRAFX2_QUANTIZE_CLUSTER_POPULATION_SPLIT

// If GRAFX2_QUANTIZE_CLUSTER_SORT_BY_VOLUME is defined
// the clusters are sorted by volume. Otherwise, they
// are sorted by length of the diagonal
//#define GRAFX2_QUANTIZE_CLUSTER_SORT_BY_VOLUME

#if defined(__GP2X__) || defined(__gp2x__) || defined(__WIZ__) || defined(__CAANOO__)
static int Convert_24b_bitmap_to_256_fast(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette);
#endif

/// Convert RGB to HSL.
/// Both input and output are in the 0..255 range to use in the palette screen
void RGB_to_HSL(int r,int g,int b,byte * hr,byte * sr,byte* lr)
{
  double rd,gd,bd,h,s,l,max,min;

  // convert RGB to HSV
  rd = r / 255.0;            // rd,gd,bd range 0-1 instead of 0-255
  gd = g / 255.0;
  bd = b / 255.0;

  // compute maximum of rd,gd,bd
  if (rd>=gd)
  {
    if (rd>=bd)
      max = rd;
    else
      max = bd;
  }
  else
  {
    if (gd>=bd)
      max = gd;
    else
      max = bd;
  }

  // compute minimum of rd,gd,bd
  if (rd<=gd)
  {
    if (rd<=bd)
      min = rd;
    else
      min = bd;
  }
  else
  {
    if (gd<=bd)
      min = gd;
    else
      min = bd;
  }

  l = (max + min) / 2.0;

  if(max==min)
      s = h = 0;
  else
  {
    if (l<=0.5)
        s = (max - min) / (max + min);
    else
        s = (max - min) / (2 - (max + min));

    if (max == rd)
        h = 42.5 * (gd-bd)/(max-min);
    else if (max == gd)
        h = 42.5 * (bd-rd)/(max-min)+85;
    else
        h = 42.5 * (rd-gd)/(max-min)+170;
    if (h<0) h+=255;
  }

  *hr = h;
  *lr = (l*255.0);
  *sr = (s*255.0);
}

/// Convert HSL back to RGB
/// Input and output are all in range 0..255
void HSL_to_RGB(byte h,byte s,byte l, byte* r, byte* g, byte* b)
{
    float rf =0 ,gf = 0,bf = 0;
    float hf,lf,sf;
    float p,q;

    if(s==0)
    {
        *r=*g=*b=l;
        return;
    }

    hf = h / 255.0;
    lf = l / 255.0;
    sf = s / 255.0;

    if (lf<=0.5)
        q = lf*(1+sf);
    else
        q = lf+sf-lf*sf;
    p = 2*lf-q;

    rf = hf + (1 / 3.0);
    gf = hf;
    bf = hf - (1 / 3.0);

    if (rf < 0) rf+=1;
    if (rf > 1) rf-=1;
    if (gf < 0) gf+=1;
    if (gf > 1) gf-=1;
    if (bf < 0) bf+=1;
    if (bf > 1) bf-=1;

    if (rf < 1/6.0)
        rf = p + ((q-p)*6*rf);
    else if(rf < 0.5)
        rf = q;
    else if(rf < 2/3.0)
        rf = p + ((q-p)*6*(2/3.0-rf));
    else
        rf = p;

    if (gf < 1/6.0)
        gf = p + ((q-p)*6*gf);
    else if(gf < 0.5)
        gf = q;
    else if(gf < 2/3.0)
        gf = p + ((q-p)*6*(2/3.0-gf));
    else
        gf = p;

    if (bf < 1/6.0)
        bf = p + ((q-p)*6*bf);
    else if(bf < 0.5)
        bf = q;
    else if(bf < 2/3.0)
        bf = p + ((q-p)*6*(2/3.0-bf));
    else
        bf = p;

    *r = rf * (255);
    *g = gf * (255);
    *b = bf * (255);
}

///
/// Returns a value that is high when color is near white,
/// and low when it's darker. Used for sorting.
long Perceptual_lightness(T_Components *color)
{
  return 26*color->R*26*color->R +
         55*color->G*55*color->G +
         19*color->B*19*color->B;
}

// Handlers for the occurrences tables
// This table is used to count the occurrence of an (RGB) pixel value in the
// source 24bit image. These count are then used by the median cut algorithm to
// decide which cluster to split.

/// Initialize an occurrence table
void OT_init(T_Occurrence_table * t)
{
  int size;

  size=(t->rng_r)*(t->rng_g)*(t->rng_b)*sizeof(int);
  memset(t->table,0,size); // Set it to 0
}

/// Allocate an occurrence table for given number of bits
T_Occurrence_table * OT_new(int nbb_r,int nbb_g,int nbb_b)
{
  T_Occurrence_table * n;
  int size;

  n=(T_Occurrence_table *)malloc(sizeof(T_Occurrence_table));
  if (n!=0)
  {
    // Copy passed parameters
    n->nbb_r=nbb_r;
    n->nbb_g=nbb_g;
    n->nbb_b=nbb_b;

    // Compute others
    n->rng_r=(1<<nbb_r);
    n->rng_g=(1<<nbb_g);
    n->rng_b=(1<<nbb_b);
    n->dec_r=nbb_g+nbb_b;
    n->dec_g=nbb_b;
    n->dec_b=0;
    n->red_r=8-nbb_r;
    n->red_g=8-nbb_g;
    n->red_b=8-nbb_b;

    // Allocate the table
    size=(n->rng_r)*(n->rng_g)*(n->rng_b);
    n->table=(int *)calloc(size, sizeof(int));
    if (n->table == NULL)
    {
      // Not enough memory !
      free(n);
      n=NULL;
    }
  }

  return n;
}


/// Delete a table and free the memory
void OT_delete(T_Occurrence_table * t)
{
  free(t->table);
  free(t);
  t = NULL;
}


/// Get number of occurrences for a given color
int OT_get(T_Occurrence_table * t, byte r, byte g, byte b)
{
  int index;

  // Drop bits as needed
  index=(r<<t->dec_r) | (g<<t->dec_g) | (b<<t->dec_b);
  return t->table[index];
}


/// Add 1 to the count for a color
void OT_inc(T_Occurrence_table * t,byte r,byte g,byte b)
{
  int index;

  // Drop bits as needed
  r=(r>>t->red_r);
  g=(g>>t->red_g);
  b=(b>>t->red_b);

  // Compute the address
  index=(r<<t->dec_r) | (g<<t->dec_g) | (b<<t->dec_b);
  t->table[index]++;
}


/// Count the use of each color in a 24bit picture and fill in the table
void OT_count_occurrences(T_Occurrence_table* t, T_Bitmap24B image, int size)
{
  T_Bitmap24B ptr;
  int index;

  for (index = size, ptr = image; index > 0; index--, ptr++)
    OT_inc(t, ptr->R, ptr->G, ptr->B);
}


/// Count the total number of pixels in an occurrence table
int OT_count_colors(T_Occurrence_table * t)
{
  int val; // Computed return value
  int nb; // Number of colors to test
  int i; // Loop index

  val = 0;
  nb=(t->rng_r)*(t->rng_g)*(t->rng_b);
  for (i = 0; i < nb; i++)
    if (t->table[i]>0)
      val++;

  return val;
}


// Cluster management
// Clusters are boxes in the RGB spaces, defined by 6 corner coordinates :
// Rmax, Rmin, Vmax (or Gmax), Vmin, Rmax, Rmin
// The median cut algorithm start with a single cluster covering the whole
// colorspace then split it in two smaller clusters on the longest axis until
// there are 256 non-empty clusters (with some tricks if the original image
// actually has less than 256 colors)
// Each cluster also store the number of pixels that are inside and the
// rmin, rmax, vmin, vmax, bmin, bmax values are the first/last values that
// actually are used by a pixel in the cluster
// When you split a big cluster there may be some space between the splitting
// plane and the first pixel actually in a cluster


/// Pack a cluster, ie compute its {r,v,b}{min,max} values
void Cluster_pack(T_Cluster * c,const T_Occurrence_table * const to)
{
  int rmin,rmax,vmin,vmax,bmin,bmax;
  int r,g,b;

  // Find min. and max. values actually used for each component in this cluster

  // Pre-shift everything to avoid using OT_Get and be faster.
  // GIMP use only 6 bits for G and B components in this table.
  rmin=c->rmax << to->dec_r; rmax=c->rmin << to->dec_r;
  vmin=c->vmax << to->dec_g; vmax=c->vmin << to->dec_g;
  bmin=c->bmax << to->dec_b; bmax=c->bmin << to->dec_b;
  c->occurences=0;

  // Unoptimized code kept here for documentation purpose because the optimized
  // one is unreadable : run over the whole cluster and find the min and max,
  // and count the occurrences at the same time.
  /*
  for (r=c->rmin<<to->dec_r;r<=c->rmax<<to->dec_r;r+=1<<to->dec_r)
    for (g=c->vmin<<to->dec_g;g<=c->vmax<<to->dec_g;g+=1<<to->dec_g)
      for (b=c->bmin;b<=c->bmax;b++)
      {
        nbocc=to->table[r + g + b]; // OT_get
        if (nbocc)
        {
          if (r<rmin) rmin=r;
          else if (r>rmax) rmax=r;
          if (g<vmin) vmin=g;
          else if (g>vmax) vmax=g;
          if (b<bmin) bmin=b;
          else if (b>bmax) bmax=b;
          c->occurrences+=nbocc;
        }
      }
  */
  
  // Optimized version : find the extremums one at a time, so we can reduce the
  // area to seek for the next one. Start at the edges of the cluster and go to
  // the center until we find a pixel.

  for(r=c->rmin<<to->dec_r;r<=c->rmax<<to->dec_r;r+=1<<to->dec_r)
      for(g=c->vmin<<to->dec_g;g<=c->vmax<<to->dec_g;g+=1<<to->dec_g)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + g + b]) // OT_get
            {
                rmin=r;
                goto RMAX;
            }
          }
RMAX:
  for(r=c->rmax<<to->dec_r;r>=rmin;r-=1<<to->dec_r)
      for(g=c->vmin<<to->dec_g;g<=c->vmax<<to->dec_g;g+=1<<to->dec_g)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + g + b]) // OT_get
            {
                rmax=r;
                goto VMIN;
            }
          }
VMIN:
  for(g=c->vmin<<to->dec_g;g<=c->vmax<<to->dec_g;g+=1<<to->dec_g)
      for(r=rmin;r<=rmax;r+=1<<to->dec_r)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + g + b]) // OT_get
            {
                vmin=g;
                goto VMAX;
            }
          }
VMAX:
  for(g=c->vmax<<to->dec_g;g>=vmin;g-=1<<to->dec_g)
      for(r=rmin;r<=rmax;r+=1<<to->dec_r)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + g + b]) // OT_get
            {
                vmax=g;
                goto BMIN;
            }
          }
BMIN:
  for(b=c->bmin;b<=c->bmax;b++)
      for(r=rmin;r<=rmax;r+=1<<to->dec_r)
          for(g=vmin;g<=vmax;g+=1<<to->dec_g)
          {
            if(to->table[r + g + b]) // OT_get
            {
                bmin=b;
                goto BMAX;
            }
          }
BMAX:
  for(b=c->bmax;b>=bmin;b--)
      for(r=rmin;r<=rmax;r+=1<<to->dec_r)
          for(g=vmin;g<=vmax;g+=1<<to->dec_g)
          {
            if(to->table[r + g + b]) // OT_get
            {
                bmax=b;
                goto ENDCRUSH;
            }
          }
ENDCRUSH:
  // We still need to seek the internal part of the cluster to count pixels
  // inside it
  for(r=rmin;r<=rmax;r+=1<<to->dec_r)
      for(g=vmin;g<=vmax;g+=1<<to->dec_g)
          for(b=bmin;b<=bmax;b++)
          {
            c->occurences+=to->table[r + g + b]; // OT_get
          }

  // Unshift the values and put them in the cluster info
  c->rmin=rmin>>to->dec_r; c->rmax=rmax>>to->dec_r;
  c->vmin=vmin>>to->dec_g; c->vmax=vmax>>to->dec_g;
  c->bmin=bmin;     c->bmax=bmax;
  
  // Find the longest axis to know which way to split the cluster
  r = c->rmax-c->rmin;
  g = c->vmax-c->vmin;
  b = c->bmax-c->bmin;

  c->data.cut.sqdiag = r*r+g*g+b*b;
  c->data.cut.volume = (r+1)*(g+1)*(b+1);

  if (g>=r)
  {
    // G>=R
    if (g>=b)
    {
      // G>=R et G>=B
      c->data.cut.plus_large=1;
    }
    else
    {
      // G>=R et G<B
      c->data.cut.plus_large=2;
    }
  }
  else
  {
    // R>G
    if (r>=b)
    {
      // R>G et R>=B
      c->data.cut.plus_large=0;
    }
    else
    {
      // R>G et R<B
      c->data.cut.plus_large=2;
    }
  }
}


#ifndef GRAFX2_QUANTIZE_CLUSTER_POPULATION_SPLIT
/// Split a cluster on its longest axis.
/// c = source cluster, c1, c2 = output after split
/// the two output cluster have half volume (and not half population)
void Cluster_split_volume(T_Cluster * c, T_Cluster * c1, T_Cluster * c2, int hue)
{
  int r,g,b;
  if (hue == 0) // split on red
  {
    r = (c->rmin + c->rmax) / 2;
    c1->Rmin=c->Rmin; c1->Rmax=r;
    c1->rmin=c->rmin; c1->rmax=r;
    c1->Gmin=c->Gmin; c1->Vmax=c->Vmax;
    c1->vmin=c->vmin; c1->vmax=c->vmax;
    c1->Bmin=c->Bmin; c1->Bmax=c->Bmax;
    c1->bmin=c->bmin; c1->bmax=c->bmax;

    c2->Rmin=r+1;     c2->Rmax=c->Rmax;
    c2->rmin=r+1;     c2->rmax=c->rmax;
    c2->Gmin=c->Gmin; c2->Vmax=c->Vmax;
    c2->vmin=c->vmin; c2->vmax=c->vmax;
    c2->Bmin=c->Bmin; c2->Bmax=c->Bmax;
    c2->bmin=c->bmin; c2->bmax=c->bmax;
  }
  else if (hue==1) // split on green
  {
    g = (c->vmin + c->vmax) / 2;
    c1->Rmin=c->Rmin; c1->Rmax=c->Rmax;
    c1->rmin=c->rmin; c1->rmax=c->rmax;
    c1->Gmin=c->Gmin; c1->Vmax=g;
    c1->vmin=c->vmin; c1->vmax=g;
    c1->Bmin=c->Bmin; c1->Bmax=c->Bmax;
    c1->bmin=c->bmin; c1->bmax=c->bmax;

    c2->Rmin=c->Rmin; c2->Rmax=c->Rmax;
    c2->rmin=c->rmin; c2->rmax=c->rmax;
    c2->Gmin=g+1;     c2->Vmax=c->Vmax;
    c2->vmin=g+1;     c2->vmax=c->vmax;
    c2->Bmin=c->Bmin; c2->Bmax=c->Bmax;
    c2->bmin=c->bmin; c2->bmax=c->bmax;
  }
  else
  {
    b = (c->bmin + c->bmax) / 2;
    c1->Rmin=c->Rmin; c1->Rmax=c->Rmax;
    c1->rmin=c->rmin; c1->rmax=c->rmax;
    c1->Gmin=c->Gmin; c1->Vmax=c->Vmax;
    c1->vmin=c->vmin; c1->vmax=c->vmax;
    c1->Bmin=c->Bmin; c1->Bmax=b;
    c1->bmin=c->bmin; c1->bmax=b;

    c2->Rmin=c->Rmin; c2->Rmax=c->Rmax;
    c2->rmin=c->rmin; c2->rmax=c->rmax;
    c2->Gmin=c->Gmin; c2->Vmax=c->Vmax;
    c2->vmin=c->vmin; c2->vmax=c->vmax;
    c2->Bmin=b+1;     c2->Bmax=c->Bmax;
    c2->bmin=b+1;     c2->bmax=c->bmax;
  }
}

#else // GRAFX2_QUANTIZE_CLUSTER_POPULATION_SPLIT
/// Split a cluster on its longest axis.
/// c = source cluster, c1, c2 = output after split
/// the two output clusters have half population (and not half volume)
void Cluster_split(T_Cluster * c, T_Cluster * c1, T_Cluster * c2, int hue,
  const T_Occurrence_table * const to)
{
  int limit;
  int cumul;
  int r, g, b;

  // Split criterion: each of the cluster will have the same number of pixels
  limit = c->occurences / 2;
  cumul = 0;
  if (hue == 0) // split on red
  {
    // Run over the cluster until we reach the requested number of pixels
    for (r = c->rmin<<to->dec_r; r<=c->rmax<<to->dec_r; r+=1<<to->dec_r)
    {
      for (g = c->vmin<<to->dec_g; g<=c->vmax<<to->dec_g; g+=1<<to->dec_g)
      {
        for (b = c->bmin; b<=c->bmax; b++)
        {
          cumul+=to->table[r + g + b];
          if (cumul>=limit)
            break;
        }
        if (cumul>=limit)
          break;
      }
      if (cumul>=limit)
        break;
    }

    r>>=to->dec_r;
    g>>=to->dec_g;
    
    // More than half of the cluster pixel have r = rmin. Ensure we split somewhere anyway.
    if (r == c->rmin) r++;

    c1->Rmin=c->Rmin; c1->Rmax=r-1;
    c1->rmin=c->rmin; c1->rmax=r-1;
    c1->Gmin=c->Gmin; c1->Vmax=c->Vmax;
    c1->vmin=c->vmin; c1->vmax=c->vmax;
    c1->Bmin=c->Bmin; c1->Bmax=c->Bmax;
    c1->bmin=c->bmin; c1->bmax=c->bmax;

    c2->Rmin=r;       c2->Rmax=c->Rmax;
    c2->rmin=r;       c2->rmax=c->rmax;
    c2->Gmin=c->Gmin; c2->Vmax=c->Vmax;
    c2->vmin=c->vmin; c2->vmax=c->vmax;
    c2->Bmin=c->Bmin; c2->Bmax=c->Bmax;
    c2->bmin=c->bmin; c2->bmax=c->bmax;
  }
  else
  if (hue==1) // split on green
  {

    for (g=c->vmin<<to->dec_g;g<=c->vmax<<to->dec_g;g+=1<<to->dec_g)
    {
      for (r=c->rmin<<to->dec_r;r<=c->rmax<<to->dec_r;r+=1<<to->dec_r)
      {
        for (b=c->bmin;b<=c->bmax;b++)
        {
          cumul+=to->table[r + g + b];
          if (cumul>=limit)
            break;
        }
        if (cumul>=limit)
          break;
      }
      if (cumul>=limit)
        break;
    }

    r>>=to->dec_r; g>>=to->dec_g;
    
    if (g == c->vmin) g++;

    c1->Rmin=c->Rmin; c1->Rmax=c->Rmax;
    c1->rmin=c->rmin; c1->rmax=c->rmax;
    c1->Gmin=c->Gmin; c1->Vmax=g-1;
    c1->vmin=c->vmin; c1->vmax=g-1;
    c1->Bmin=c->Bmin; c1->Bmax=c->Bmax;
    c1->bmin=c->bmin; c1->bmax=c->bmax;

    c2->Rmin=c->Rmin; c2->Rmax=c->Rmax;
    c2->rmin=c->rmin; c2->rmax=c->rmax;
    c2->Gmin=g;       c2->Vmax=c->Vmax;
    c2->vmin=g;       c2->vmax=c->vmax;
    c2->Bmin=c->Bmin; c2->Bmax=c->Bmax;
    c2->bmin=c->bmin; c2->bmax=c->bmax;
  }
  else // split on blue
  {

    for (b=c->bmin;b<=c->bmax;b++)
    {
      for (g=c->vmin<<to->dec_g;g<=c->vmax<<to->dec_g;g+=1<<to->dec_g)
      {
        for (r=c->rmin<<to->dec_r;r<=c->rmax<<to->dec_r;r+=1<<to->dec_r)
        {
          cumul+=to->table[r + g + b];
          if (cumul>=limit)
            break;
        }
        if (cumul>=limit)
          break;
      }
      if (cumul>=limit)
        break;
    }

    r>>=to->dec_r; g>>=to->dec_g;
    
    if (b == c->bmin) b++;

    c1->Rmin=c->Rmin; c1->Rmax=c->Rmax;
    c1->rmin=c->rmin; c1->rmax=c->rmax;
    c1->Gmin=c->Gmin; c1->Vmax=c->Vmax;
    c1->vmin=c->vmin; c1->vmax=c->vmax;
    c1->Bmin=c->Bmin; c1->Bmax=b-1;
    c1->bmin=c->bmin; c1->bmax=b-1;

    c2->Rmin=c->Rmin; c2->Rmax=c->Rmax;
    c2->rmin=c->rmin; c2->rmax=c->rmax;
    c2->Gmin=c->Gmin; c2->Vmax=c->Vmax;
    c2->vmin=c->vmin; c2->vmax=c->vmax;
    c2->Bmin=b;       c2->Bmax=c->Bmax;
    c2->bmin=b;       c2->bmax=c->bmax;
  }
}
#endif // GRAFX2_QUANTIZE_CLUSTER_POPULATION_SPLIT


/// Compute the mean R, G, B (for palette generation) and H, L (for palette sorting)
void Cluster_compute_hue(T_Cluster * c,T_Occurrence_table * to)
{
  int cumul_r,cumul_g,cumul_b;
  int r,g,b;
  int nbocc;

  byte s=0;

  cumul_r=cumul_g=cumul_b=0;
  for (r=c->rmin;r<=c->rmax;r++)
    for (g=c->vmin;g<=c->vmax;g++)
      for (b=c->bmin;b<=c->bmax;b++)
      {
        nbocc=OT_get(to,r,g,b);
        if (nbocc)
        {
          cumul_r+=r*nbocc;
          cumul_g+=g*nbocc;
          cumul_b+=b*nbocc;
        }
      }
  
  c->data.pal.r=(cumul_r<<to->red_r)/c->occurences;
  c->data.pal.g=(cumul_g<<to->red_g)/c->occurences;
  c->data.pal.b=(cumul_b<<to->red_b)/c->occurences;
  RGB_to_HSL(c->data.pal.r, c->data.pal.g, c->data.pal.b, &c->data.pal.h, &s, &c->data.pal.l);
}


// Cluster set management
// A set of clusters in handled as a list, the median cut algorithm pops a
// cluster from the list, split it, and pushes back the two splitted clusters
// until the lit grows to 256 items


// Debug helper : check if a cluster set has the right count value
/*
void CS_Check(T_Cluster_set* cs)
{
    int i;
    T_Cluster* c = cs->clusters;
    for (i = cs->nb; i > 0; i--)
    {
        assert( c != NULL);
        c = c->next;
    }

    assert(c == NULL);
}
*/

/*
void Cluster_Print(T_Cluster* node)
{
	printf("R %d %d\tG %d %d\tB %d %d\n",
		node->Rmin, node->Rmax, node->Gmin, node->Vmax,
		node->Bmin, node->Bmax);
}
*/

// translate R G B values to 8 8 8
void CT_set_trad(CT_Tree* colorTree, byte Rmin, byte Gmin, byte Bmin,
	byte Rmax, byte Gmax, byte Bmax, byte index, const T_Occurrence_table * to)
{
  Rmin <<= to->red_r;
  Rmax <<= to->red_r;
  Rmax += ((1 << to->red_r) - 1);
  Gmin <<= to->red_g;
  Gmax <<= to->red_g;
  Gmax += ((1 << to->red_g) - 1);
  Bmin <<= to->red_b;
  Bmax <<= to->red_b;
  Bmax += ((1 << to->red_b) - 1);
  CT_set(colorTree, Rmin, Gmin, Bmin,
	       Rmax, Gmax, Bmax, index);
}

/// Setup the first cluster before we start the operations
/// This one covers the full palette range
void CS_Init(T_Cluster_set * cs, T_Occurrence_table * to)
{
  cs->clusters->Rmin = cs->clusters->rmin = 0;
  cs->clusters->Gmin = cs->clusters->vmin = 0;
  cs->clusters->Bmin = cs->clusters->bmin = 0;
  cs->clusters->Rmax = cs->clusters->rmax = to->rng_r - 1;
  cs->clusters->Vmax = cs->clusters->vmax = to->rng_g - 1;
  cs->clusters->Bmax = cs->clusters->bmax = to->rng_b - 1;
  cs->clusters->next = NULL;
  Cluster_pack(cs->clusters, to);
  cs->nb = 1;
}

/// Allocate a new cluster set
T_Cluster_set * CS_New(int nbmax, T_Occurrence_table * to)
{
  T_Cluster_set * n;

  n=(T_Cluster_set *)malloc(sizeof(T_Cluster_set));
  if (n != NULL)
  {
    // Copy requested params
    n->nb_max = OT_count_colors(to);

    // If the number of colors asked is > 256, we ceil it because we know we
    // don't want more
    if (n->nb_max > nbmax)
    {
      n->nb_max = nbmax;
    }

    // Allocate the first cluster
    n->clusters=(T_Cluster *)malloc(sizeof(T_Cluster));
    if (n->clusters != NULL)
      CS_Init(n, to);
    else
    {
      // No memory free ! Sorry !
      free(n);
      n = NULL;
    }
  }
  return n;
}

/// Free a cluster set
void CS_Delete(T_Cluster_set * cs)
{
    T_Cluster* nxt;
    while (cs->clusters != NULL)
    {
        nxt = cs->clusters->next;
        free(cs->clusters);
        cs->clusters = nxt;
    }
    free(cs);
  cs = NULL;
}


/// Pop a cluster from the cluster list
void CS_Get(T_Cluster_set * cs, T_Cluster ** c)
{  
  // Just remove and return the first cluster, which has the biggest volume.
  // or the longest diagonal
  *c = cs->clusters;
  
  cs->clusters = (*c)->next;
  --cs->nb;
}


/// Push a copy of a cluster in the list
/// return -1 in case of error
int CS_Set(T_Cluster_set * cs,T_Cluster * c)
{
  T_Cluster* current = cs->clusters;
  T_Cluster* prev = NULL;

  // Search the first cluster that is smaller than ours
#ifdef GRAFX2_QUANTIZE_CLUSTER_SORT_BY_VOLUME
  while (current && current->data.cut.volume > c->data.cut.volume)
#else
  while (current && current->data.cut.sqdiag > c->data.cut.sqdiag)
#endif
  {
    prev = current;
    current = current->next;
  }

  // Now insert our cluster just before the one we found
  c -> next = current;

  current = malloc(sizeof(T_Cluster));
  if(current == NULL)
    return -1;
  *current = *c ;

  if (prev) prev->next = current;
  else cs->clusters = current;

  cs->nb++;
  return 0;
}

/// This is the main median cut algorithm and the function actually called to
/// reduce the palette. We get the number of pixels for each collor in the
/// occurrence table and generate the cluster set from it.
// 1) RGB space is a big box
// 2) We seek the pixels with extreme values
// 3) We split the box in 2 parts on its longest axis
// 4) We pack the 2 resulting boxes again to leave no empty space between the box border and the first pixel
// 5) We take the box with the biggest number of pixels inside and we split it again
// 6) Iterate until there are 256 boxes. Associate each of them to its middle color
// At the same time, put the split clusters in the color tree for later palette lookup
int CS_Generate(T_Cluster_set * cs, const T_Occurrence_table * const to, CT_Tree* colorTree)
{
  T_Cluster* current;
  T_Cluster Nouveau1;
  T_Cluster Nouveau2;

  // There are less than 256 boxes
  while (cs->nb<cs->nb_max)
  {
    // Get the biggest one
    CS_Get(cs,&current);
    //Cluster_Print(current);
    
    // We are going to split this cluster, so add it to the color tree. It is a split cluster,
    // not a final one. We KNOW its two child will get added later (either because they are split,
    // or because they are part of the final cluster set). So, we add thiscluster with a NULL index.
    CT_set_trad(colorTree,current->Rmin, current->Gmin, current->Bmin,
       current->Rmax, current->Vmax, current->Bmax, 0, to);

    // Split it
    if (current->data.cut.volume <= 1)
    {
    	// Sorry, but there's nothing more to split !
    	// The biggest cluster only has one color...
    	free(current);
    	break;
    }
#ifndef GRAFX2_QUANTIZE_CLUSTER_POPULATION_SPLIT
    Cluster_split_volume(current, &Nouveau1, &Nouveau2, current->data.cut.plus_large);
#else
    Cluster_split(current, &Nouveau1, &Nouveau2, current->data.cut.plus_large, to);
#endif
    free(current);

    // Pack the 2 new clusters (the split may leave some empty space between the
    // box border and the first actual pixel)
    Cluster_pack(&Nouveau1, to);
    Cluster_pack(&Nouveau2, to);

    // Put them back in the list
    if (Nouveau1.occurences != 0) {
      if(CS_Set(cs,&Nouveau1) < 0)
        return -1;
    }

    if (Nouveau2.occurences != 0) {
      if(CS_Set(cs,&Nouveau2) < 0)
        return -1;
    }
  }
  return 0;
}


/// Compute the color associated to each box in the list
void CS_Compute_colors(T_Cluster_set * cs, T_Occurrence_table * to)
{
  T_Cluster * c;

  for (c=cs->clusters;c!=NULL;c=c->next) {
    Cluster_compute_hue(c,to);
  }
}


// We sort the clusters on two criterions to get a somewhat coherent palette.
// TODO : It would be better to do this in one single pass.

/// Sort the clusters by chrominance value
void CS_Sort_by_chrominance(T_Cluster_set * cs)
{
  T_Cluster* nc;
  T_Cluster* prev = NULL;
  T_Cluster* place;
  T_Cluster* newlist = NULL;

  while (cs->clusters)
  {
    // Remove the first cluster from the original list
    nc = cs->clusters;
    cs->clusters = cs->clusters->next;

    // Find his position in the new list
    for (place = newlist; place != NULL; place = place->next)
    {
      if (place->data.pal.h > nc->data.pal.h) break;
      prev = place;
    }

    // Chain it there
    nc->next = place;
    if (prev) prev->next = nc;
    else newlist = nc;

    prev = NULL;
  }

  // Put the new list back in place
  cs->clusters = newlist;
}


/// Sort the clusters by luminance value
void CS_Sort_by_luminance(T_Cluster_set * cs)
{
  T_Cluster* nc;
  T_Cluster* prev = NULL;
  T_Cluster* place;
  T_Cluster* newlist = NULL;

  while (cs->clusters)
  {
    // Remove the first cluster from the original list
    nc = cs->clusters;
    cs->clusters = cs->clusters->next;

    // Find its position in the new list
    for (place = newlist; place != NULL; place = place->next)
    {
      if (place->data.pal.l > nc->data.pal.l) break;
      prev = place;
    }

    // Chain it there
    nc->next = place;
    if (prev) prev->next = nc;
    else newlist = nc;

    // reset prev pointer
    prev = NULL;
  }

  // Put the new list back in place
  cs->clusters = newlist;
}


/// Generates the palette from the clusters, then the conversion table to map (RGB) to a palette index
void CS_Generate_color_table_and_palette(T_Cluster_set * cs,CT_Tree* tc,T_Components * palette, T_Occurrence_table * to)
{
  int index;
  T_Cluster* current = cs->clusters;

  for (index=0;index<cs->nb;index++)
  {
    palette[index].R=current->data.pal.r;
    palette[index].G=current->data.pal.g;
    palette[index].B=current->data.pal.b;

    CT_set_trad(tc,current->Rmin, current->Gmin, current->Bmin,
       current->Rmax, current->Vmax, current->Bmax,
       index, to);
    current = current->next;
  }
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Méthodes de gestion des dégradés //
/////////////////////////////////////////////////////////////////////////////

void GS_Init(T_Gradient_set * ds,T_Cluster_set * cs)
{
    ds->gradients[0].nb_colors=1;
    ds->gradients[0].min=cs->clusters->data.pal.h;
    ds->gradients[0].max=cs->clusters->data.pal.h;
    ds->gradients[0].hue=cs->clusters->data.pal.h;
    // Et hop : le 1er ensemble de dégradés est initialisé
    ds->nb=1;
}

T_Gradient_set * GS_New(T_Cluster_set * cs)
{
    T_Gradient_set * n;

    n=(T_Gradient_set *)malloc(sizeof(T_Gradient_set));
    if (n!=NULL)
    {
        // On recopie les paramètres demandés
        n->nb_max=cs->nb_max;

        // On tente d'allouer la table
        n->gradients=(T_Gradient *)malloc((n->nb_max)*sizeof(T_Gradient));
        if (n->gradients!=0)
            // C'est bon! On initialise
            GS_Init(n,cs);
        else
        {
            // Table impossible à allouer
            free(n);
            n=NULL;
        }
    }

    return n;
}

void GS_Delete(T_Gradient_set * ds)
{
    free(ds->gradients);
    free(ds);
}

void GS_Generate(T_Gradient_set * ds,T_Cluster_set * cs)
{
    int id; // Les indexs de parcours des ensembles
    int best_gradient; // Meilleur dégradé
    int best_diff; // Meilleure différence de chrominance
    int diff;  // difference de chrominance courante
    T_Cluster * current = cs->clusters;

    // Pour chacun des clusters … traiter
    do
    {
        // On recherche le dégradé le plus proche de la chrominance du cluster
        best_gradient=-1;
        best_diff=99999999;
        for (id=0;id<ds->nb;id++)
        {
            diff=abs(current->data.pal.h - (int)ds->gradients[id].hue);
            if ((best_diff>diff) && (diff<16))
            {
                best_gradient=id;
                best_diff=diff;
            }
        }

        // Si on a trouvé un dégradé dans lequel inclure le cluster
        if (best_gradient!=-1)
        {
            // On met à jour le dégradé
            if (current->data.pal.h < ds->gradients[best_gradient].min)
                ds->gradients[best_gradient].min=current->data.pal.h;
            if (current->data.pal.h > ds->gradients[best_gradient].max)
                ds->gradients[best_gradient].max=current->data.pal.h;
            ds->gradients[best_gradient].hue=((ds->gradients[best_gradient].hue*
                        ds->gradients[best_gradient].nb_colors)
                    +current->data.pal.h)
                /(ds->gradients[best_gradient].nb_colors+1);
            ds->gradients[best_gradient].nb_colors++;
        }
        else
        {
            // On crée un nouveau dégradé
            best_gradient=ds->nb;
            ds->gradients[best_gradient].nb_colors=1;
            ds->gradients[best_gradient].min=current->data.pal.h;
            ds->gradients[best_gradient].max=current->data.pal.h;
            ds->gradients[best_gradient].hue=current->data.pal.h;
            ds->nb++;
        }
        current->data.pal.h=best_gradient;
    } while((current = current->next));

    // On redistribue les valeurs dans les clusters
    current = cs -> clusters;
    do
        current->data.pal.h=ds->gradients[current->data.pal.h].hue;
    while((current = current ->next));
}


/// Compute best palette for given picture.
///
/// The picture is first depth-reduced to the given
/// r,g,b resolution, then the median cut algorithm is used to find 256 colors which are suitable
/// for the given picture.
///
/// @returns a conversion tree to be used for converting the picture to indexed with the generated palette (with or without dithering).
///
/// @param image The true-color image for which the palette needs to be optimized
/// @param size in pixels (number of pixels, the height/width doesn't matter)
/// @param palette pointer to the space where the palette will be stored (256 entries at most)
/// @param r Resolution for red
/// @param g Resolution for green
/// @param b Resolution for blue
CT_Tree* Optimize_palette(T_Bitmap24B image, int size,
  T_Components * palette, int r, int g, int b)
{
  T_Occurrence_table * to;
  CT_Tree* tc;
  T_Cluster_set * cs;
  T_Gradient_set * ds;

  // Allocate all the elements
  to = 0; tc = 0; cs = 0; ds = 0;

  to = OT_new(r, g, b);
  if (to == NULL)
    return 0;

  tc = CT_new();
  
  if (tc == NULL)
  {
  	OT_delete(to);
  	return NULL;
  }

  // Count pixels for each color
  OT_count_occurrences(to, image, size);

  cs = CS_New(256, to);
  if (cs == NULL)
  {
    CT_delete(tc);
    OT_delete(to);
    return NULL;
  }
  //CS_Check(cs);
  // Ok, everything was allocated

  // Generate the cluster set with median cut algorithm
  if(CS_Generate(cs, to, tc) < 0) {
    CS_Delete(cs);
    CT_delete(tc);
    OT_delete(to);
    return NULL;
  }
  //CS_Check(cs);

  // Compute the color data for each cluster (palette entry + HL)
  CS_Compute_colors(cs, to);
  //CS_Check(cs);

  ds = GS_New(cs);
  if (ds!= NULL)
  {
    GS_Generate(ds, cs);
    GS_Delete(ds);
  }
  // Sort the clusters on L and H to get a nice palette
  CS_Sort_by_luminance(cs);
  //CS_Check(cs);
  CS_Sort_by_chrominance(cs);
  //CS_Check(cs);
  
  // And finally generate the conversion table to map RGB > pal. index
  CS_Generate_color_table_and_palette(cs, tc, palette, to);
  //CS_Check(cs);
  
  CS_Delete(cs);
  OT_delete(to);
  return tc;
}


/// Change a value with proper ceiling and flooring
int Modified_value(int value,int modif)
{
  value+=modif;
  if (value<0)
  {
    value=0;
  }
  else if (value>255)
  {
    value=255;
  }
  return value;
}


/// Convert a 24b image to 256 colors (with a given palette and conversion table).
/// This destroys the 24b picture !
/// Uses floyd steinberg dithering.
void Convert_24b_bitmap_to_256_Floyd_Steinberg(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette,CT_Tree* tc)
{
  T_Bitmap24B current;
  T_Bitmap24B c_plus1;
  T_Bitmap24B u_minus1;
  T_Bitmap24B next;
  T_Bitmap24B u_plus1;
  T_Bitmap256 d;
  int x_pos,y_pos;
  int red,green,blue;
  float e_red,e_green,e_blue;

  // On initialise les variables de parcours:
  current =source;      // Le pixel dont on s'occupe
  next =current+width; // Le pixel en dessous
  c_plus1 =current+1;   // Le pixel à droite
  u_minus1=next-1;   // Le pixel en bas à gauche
  u_plus1 =next+1;   // Le pixel en bas à droite
  d       =dest;

  // On parcours chaque pixel:
  for (y_pos=0;y_pos<height;y_pos++)
  {
    for (x_pos=0;x_pos<width;x_pos++)
    {
      // On prends la meilleure couleur de la palette qui traduit la couleur
      // 24 bits de la source:
      red=current->R;
      green =current->G;
      blue =current->B;
      // Cherche la couleur correspondant dans la palette et la range dans l'image de destination
      *d=CT_get(tc,red,green,blue);

      // Puis on calcule pour chaque composante l'erreur dûe à l'approximation
      red-=palette[*d].R;
      green -=palette[*d].G;
      blue -=palette[*d].B;

      // Et dans chaque pixel voisin on propage l'erreur
      // A droite:
        e_red=(red*7)/16.0;
        e_green =(green *7)/16.0;
        e_blue =(blue *7)/16.0;
        if (x_pos+1<width)
        {
          // Modified_value fait la somme des 2 params en bornant sur [0,255]
          c_plus1->R=Modified_value(c_plus1->R,e_red);
          c_plus1->G=Modified_value(c_plus1->G,e_green );
          c_plus1->B=Modified_value(c_plus1->B,e_blue );
        }
      // En bas à gauche:
      if (y_pos+1<height)
      {
        e_red=(red*3)/16.0;
        e_green =(green *3)/16.0;
        e_blue =(blue *3)/16.0;
        if (x_pos>0)
        {
          u_minus1->R=Modified_value(u_minus1->R,e_red);
          u_minus1->G=Modified_value(u_minus1->G,e_green );
          u_minus1->B=Modified_value(u_minus1->B,e_blue );
        }
      // En bas:
        e_red=(red*5/16.0);
        e_green =(green*5 /16.0);
        e_blue =(blue*5 /16.0);
        next->R=Modified_value(next->R,e_red);
        next->G=Modified_value(next->G,e_green );
        next->B=Modified_value(next->B,e_blue );
      // En bas à droite:
        if (x_pos+1<width)
        {
        e_red=(red/16.0);
        e_green =(green /16.0);
        e_blue =(blue /16.0);
          u_plus1->R=Modified_value(u_plus1->R,e_red);
          u_plus1->G=Modified_value(u_plus1->G,e_green );
          u_plus1->B=Modified_value(u_plus1->B,e_blue );
        }
      }

      // On passe au pixel suivant :
      current++;
      c_plus1++;
      u_minus1++;
      next++;
      u_plus1++;
      d++;
    }
  }
}


/// Converts from 24b to 256c without dithering, using given conversion table
void Convert_24b_bitmap_to_256_nearest_neighbor(T_Bitmap256 dest,
  T_Bitmap24B source, int width, int height, T_Components * palette,
  CT_Tree* tc)
{
  T_Bitmap24B current;
  T_Bitmap256 d;
  int x_pos, y_pos;
  int red, green, blue;
  (void)palette; // unused

  // On initialise les variables de parcours:
  current =source; // Le pixel dont on s'occupe

  d =dest;

  // On parcours chaque pixel:
  for (y_pos = 0; y_pos < height; y_pos++)
  {
    for (x_pos = 0 ;x_pos < width; x_pos++)
    {
      // On prends la meilleure couleur de la palette qui traduit la couleur
      // 24 bits de la source:
      red = current->R;
      green = current->G;
      blue = current->B;
      // Cherche la couleur correspondant dans la palette et la range dans
      // l'image de destination
      *d = CT_get(tc, red, green, blue);

      // On passe au pixel suivant :
      current++;
      d++;
    }
  }
}


// Count colors and convert if 256 colors or less are used
// return 0 for success
int Try_Convert_to_256_Without_Loss(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette)
{
  int i;
  int n = 0;  // number of colors
  long index;
  T_Bitmap24B ptr;

  for (index = width * height, ptr = source; index > 0; index--, ptr++)
  {
    // look for the color in the table
    for (i = 0; i < n; i++) {
      if(palette[i].R == ptr->R && palette[i].G == ptr->G && palette[i].B == ptr->B)
        break;  // found !
    }
    if (i >= n) {
      if (n > 255) {
        // there are more than 256 colors
        return 1;
      }
      // need to add the color in the palette
      palette[n].R = ptr->R;
      palette[n].G = ptr->G;
      palette[n].B = ptr->B;
      n++;
    }
    *dest = (byte)i;
    dest++;
  }
  // TODO : Sort the palette ?
  return 0;
}


// These are the allowed precisions for all the tables.
// For some of them only the first one may work because of ugly optimizations
static const byte precision_24b[]=
{
 8,8,8,
 6,6,6,
 6,6,5,
 5,6,5,
 5,5,5,
 5,5,4,
 4,5,4,
 4,4,4,
 4,4,3,
 3,4,3,
 3,3,3,
 3,3,2};


/**
 * Converts a 24 bit picture to 256 color (color reduction)
 * @param[out] dest The converted 8bpp picture
 * @param[in] source the 24bpp picture
 * @param[in] width the width of the picture
 * @param[in] height the height of the picture
 * @param[out] palette the palette of the converted 8bpp picture
 * @return 0 for OK, 1 for error
 */
int Convert_24b_bitmap_to_256(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette)
{
#if !(defined(__GP2X__) || defined(__gp2x__) || defined(__WIZ__) || defined(__CAANOO__))
  CT_Tree* table; // table de conversion
  int                ip;    // index de précision pour la conversion
#endif

  if (Try_Convert_to_256_Without_Loss(dest, source, width, height, palette) == 0)
    return 0;

  #if defined(__GP2X__) || defined(__gp2x__) || defined(__WIZ__) || defined(__CAANOO__)
  return Convert_24b_bitmap_to_256_fast(dest, source, width, height, palette);  

  #else
  // On essaye d'obtenir une table de conversion qui loge en mémoire, avec la
  // meilleure précision possible
  for (ip=0;ip<(10*3);ip+=3)
  {
    table = Optimize_palette(source,width*height,palette,
                             precision_24b[ip], precision_24b[ip+1], precision_24b[ip+2]);
    if (table != NULL) {
      break;
    }
  }

  if (table!=NULL)
  {
    //Convert_24b_bitmap_to_256_Floyd_Steinberg(dest,source,width,height,palette,table);
    Convert_24b_bitmap_to_256_nearest_neighbor(dest,source,width,height,palette,table);
    CT_delete(table);
    return 0;
  }
  else
    return 1;

  #endif
}


//Really small, fast and ugly converter(just for handhelds)
#include "global.h"
#include <limits.h>
#include "engine.h"
#include "windows.h"

extern void Set_palette_fake_24b(T_Palette palette);

#if defined(__GP2X__) || defined(__gp2x__) || defined(__WIZ__) || defined(__CAANOO__)
/// Really small, fast and dirty convertor(just for handhelds)
static int Convert_24b_bitmap_to_256_fast(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette)
{
  int size;

  Set_palette_fake_24b(palette);

  size = width*height;

  while(size--)
  {
    //Set palette color index to destination bitmap
    *dest = ((source->R >> 5) << 5) |
            ((source->G >> 5) << 2) |
            ((source->B >> 6));
    source++;
    dest++;
  }
  return 0;
}
#endif
