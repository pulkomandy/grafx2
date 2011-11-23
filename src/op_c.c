/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2010 Alexander Filyanov
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
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

#include "op_c.h"
#include "errors.h"
#include "colorred.h"

int Convert_24b_bitmap_to_256_fast(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette);

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

// Handlers for the occurences tables
// This table is used to count the occurence of an (RGB) pixel value in the
// source 24bit image. These count are then used by the median cut algorithm to
// decide which cluster to split.

/// Initialize an occurence table
void OT_init(T_Occurrence_table * t)
{
  int size;

  size=(t->rng_r)*(t->rng_g)*(t->rng_b)*sizeof(int);
  memset(t->table,0,size); // Set it to 0
}

/// Allocate an occurence table for given number of bits
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
    size=(n->rng_r)*(n->rng_g)*(n->rng_b)*sizeof(int);
    n->table=(int *)calloc(size, 1);
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


/// Get number of occurences for a given color
int OT_get(T_Occurrence_table * t, int r, int g, int b)
{
  int index;

  // Drop bits as needed
  index=(r<<t->dec_r) | (g<<t->dec_g) | (b<<t->dec_b);
  return t->table[index];
}


/// Add 1 to the count for a color
void OT_inc(T_Occurrence_table * t,int r,int g,int b)
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


/// Count the total number of pixels in an occurence table
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
void Cluster_pack(T_Cluster * c,T_Occurrence_table * to)
{
  int rmin,rmax,vmin,vmax,bmin,bmax;
  int r,g,b;

  // Find min. and max. values actually used for each component in this cluster

  // Pre-shift everything to avoid using OT_Get and be faster. This will only
  // work if the occurence table actually has full precision, that is a
  // 256^3*sizeof(int) = 64MB table. If your computer has less free ram and
  // malloc fails, this will not work at all !
  // GIMP use only 6 bits for G and B components in this table.
  rmin=c->rmax <<16; rmax=c->rmin << 16;
  vmin=c->vmax << 8; vmax=c->vmin << 8;
  bmin=c->bmax; bmax=c->bmin;
  c->occurences=0;

  // Unoptimized code kept here for documentation purpose because the optimized
  // one is unreadable : run over the whole cluster and find the min and max,
  // and count the occurences at the same time.
  /*
  for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
    for (g=c->vmin<<8;g<=c->vmax<<8;g+=1<<8)
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
          c->occurences+=nbocc;
        }
      }
  */
  
  // Optimized version : find the extremums one at a time, so we can reduce the
  // area to seek for the next one. Start at the edges of the cluster and go to
  // the center until we find a pixel.

  for(r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
      for(g=c->vmin<<8;g<=c->vmax<<8;g+=1<<8)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + g + b]) // OT_get
            {
                rmin=r;
                goto RMAX;
            }
          }
RMAX:
  for(r=c->rmax<<16;r>=rmin;r-=1<<16)
      for(g=c->vmin<<8;g<=c->vmax<<8;g+=1<<8)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + g + b]) // OT_get
            {
                rmax=r;
                goto VMIN;
            }
          }
VMIN:
  for(g=c->vmin<<8;g<=c->vmax<<8;g+=1<<8)
      for(r=rmin;r<=rmax;r+=1<<16)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + g + b]) // OT_get
            {
                vmin=g;
                goto VMAX;
            }
          }
VMAX:
  for(g=c->vmax<<8;g>=vmin;g-=1<<8)
      for(r=rmin;r<=rmax;r+=1<<16)
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
      for(r=rmin;r<=rmax;r+=1<<16)
          for(g=vmin;g<=vmax;g+=1<<8)
          {
            if(to->table[r + g + b]) // OT_get
            {
                bmin=b;
                goto BMAX;
            }
          }
BMAX:
  for(b=c->bmax;b>=bmin;b--)
      for(r=rmin;r<=rmax;r+=1<<16)
          for(g=vmin;g<=vmax;g+=1<<8)
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
  for(r=rmin;r<=rmax;r+=1<<16)
      for(g=vmin;g<=vmax;g+=1<<8)
          for(b=bmin;b<=bmax;b++)
          {
            c->occurences+=to->table[r + g + b]; // OT_get
          }

  // Unshift the values and put them in the cluster info
  c->rmin=rmin>>16; c->rmax=rmax>>16;
  c->vmin=vmin>>8;  c->vmax=vmax>>8;
  c->bmin=bmin;     c->bmax=bmax;
  
  // Find the longest axis to know which way to split the cluster
  // This multiplications are supposed to improve the result, but may or may not
  // work, actually.
  r=(c->rmax-c->rmin)*299;
  g=(c->vmax-c->vmin)*587;
  b=(c->bmax-c->bmin)*114;

  if (g>=r)
  {
    // G>=R
    if (g>=b)
    {
      // G>=R et G>=B
      c->plus_large=1;
    }
    else
    {
      // G>=R et G<B
      c->plus_large=2;
    }
  }
  else
  {
    // R>G
    if (r>=b)
    {
      // R>G et R>=B
      c->plus_large=0;
    }
    else
    {
      // R>G et R<B
      c->plus_large=2;
    }
  }
}


/// Split a cluster on its longest axis.
/// c = source cluster, c1, c2 = output after split
void Cluster_split(T_Cluster * c, T_Cluster * c1, T_Cluster * c2, int hue,
  T_Occurrence_table * to)
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
    for (r = c->rmin<<16; r<=c->rmax<<16; r+=1<<16)
    {
      for (g = c->vmin<<8; g<=c->vmax<<8; g+=1<<8)
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

    r>>=16;
    g>>=8;

    // We tried to split on red, but found half of the pixels with r = rmin
    // so we enforce some split to happen anyway, instead of creating an empty
    // c2 and c1 == c
    if (r==c->rmin)
      r++;

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

    for (g=c->vmin<<8;g<=c->vmax<<8;g+=1<<8)
    {
      for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
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

    r>>=16; g>>=8;

    if (g==c->vmin)
      g++;

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
      for (g=c->vmin<<8;g<=c->vmax<<8;g+=1<<8)
      {
        for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
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

    r>>=16; g>>=8;

    if (b==c->bmin)
      b++;

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
  
  c->r=(cumul_r<<to->red_r)/c->occurences;
  c->g=(cumul_g<<to->red_g)/c->occurences;
  c->b=(cumul_b<<to->red_b)/c->occurences;
  RGB_to_HSL(c->r, c->g, c->b, &c->h, &s, &c->l);
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
void CS_Get(T_Cluster_set * cs, T_Cluster * c)
{
  T_Cluster* current = cs->clusters;
  T_Cluster* prev = NULL;

  // Search a cluster with at least 2 distinct colors so we can split it
  // Clusters are sorted by number of occurences, so a cluster may end up
  // with a lot of pixelsand on top of the list, but only one color. We can't
  // split it in that case. It should probably be stored on a list of unsplittable
  // clusters to avoid running on it again on each iteration.
  do
  {
    if ( (current->rmin < current->rmax) ||
         (current->vmin < current->vmax) ||
         (current->bmin < current->bmax) )
      break;

    prev = current;
    
  } while((current = current -> next));

  // copy it to c
  *c = *current;

  // remove it from the list
  cs->nb--;

  if(prev)
    prev->next = current->next;
  else
    cs->clusters = current->next;
  free(current);
  current = NULL;
}


/// Push a cluster in the list
void CS_Set(T_Cluster_set * cs,T_Cluster * c)
{
  T_Cluster* current = cs->clusters;
  T_Cluster* prev = NULL;

  // Search the first cluster that is smaller than ours (less pixels)
  while (current && current->occurences > c->occurences)
  {
    prev = current;
    current = current->next;
  }

  // Now insert our cluster just before the one we found
  c -> next = current;

  current = malloc(sizeof(T_Cluster));
  *current = *c ;

  if (prev) prev->next = current;
  else cs->clusters = current;

  cs->nb++;
}

/// This is the main median cut algorithm and the function actually called to
/// reduce the palette. We get the number of pixels for each collor in the
/// occurence table and generate the cluster set from it.
// 1) RGB space is a big box
// 2) We seek the pixels with extreme values
// 3) We split the box in 2 parts on its longest axis
// 4) We pack the 2 resulting boxes again to leave no empty space between the box border and the first pixel
// 5) We take the box with the biggest number of pixels inside and we split it again
// 6) Iterate until there are 256 boxes. Associate each of them to its middle color
// At the same time, put the split clusters in the color tree for later palette lookup
void CS_Generate(T_Cluster_set * cs, T_Occurrence_table * to, CT_Node** colorTree)
{
  T_Cluster current;
  T_Cluster Nouveau1;
  T_Cluster Nouveau2;

  // There are less than 256 boxes
  while (cs->nb<cs->nb_max)
  {
    // Get the biggest one
    CS_Get(cs,&current);
    
    // We're going to split, so add the cluster to the colortree
    CT_set(colorTree,current.Rmin, current.Gmin, current.Bmin,
       current.Rmax, current.Vmax, current.Bmax, 0);

    // Split it
    Cluster_split(&current, &Nouveau1, &Nouveau2, current.plus_large, to);

    // Pack the 2 new clusters (the split may leave some empty space between the
    // box border and the first actual pixel)
    Cluster_pack(&Nouveau1, to);
    Cluster_pack(&Nouveau2, to);

    // Put them back in the list
    CS_Set(cs,&Nouveau1);
    CS_Set(cs,&Nouveau2);
    
  }
}


/// Compute the color associated to each box in the list
void CS_Compute_colors(T_Cluster_set * cs, T_Occurrence_table * to)
{
  T_Cluster * c;

  for (c=cs->clusters;c!=NULL;c=c->next)
    Cluster_compute_hue(c,to);
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
      if (place->h > nc->h) break;
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
      if (place->l > nc->l) break;
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
void CS_Generate_color_table_and_palette(T_Cluster_set * cs,CT_Node** tc,T_Components * palette)
{
  int index;
  T_Cluster* current = cs->clusters;

  for (index=0;index<cs->nb;index++)
  {
    palette[index].R=current->r;
    palette[index].G=current->g;
    palette[index].B=current->b;

    CT_set(tc,current->Rmin, current->Gmin, current->Bmin,
       current->Rmax, current->Vmax, current->Bmax, index);
    current = current->next;
  }
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Méthodes de gestion des dégradés //
/////////////////////////////////////////////////////////////////////////////

void GS_Init(T_Gradient_set * ds,T_Cluster_set * cs)
{
    ds->gradients[0].nb_colors=1;
    ds->gradients[0].min=cs->clusters->h;
    ds->gradients[0].max=cs->clusters->h;
    ds->gradients[0].hue=cs->clusters->h;
    // Et hop : le 1er ensemble de d‚grad‚s est initialis‚
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
    int best_gradient; // Meilleur d‚grad‚
    int best_diff; // Meilleure diff‚rence de chrominance
    int diff;  // difference de chrominance courante
    T_Cluster * current = cs->clusters;

    // Pour chacun des clusters … traiter
    do
    {
        // On recherche le d‚grad‚ le plus proche de la chrominance du cluster
        best_gradient=-1;
        best_diff=99999999;
        for (id=0;id<ds->nb;id++)
        {
            diff=abs(current->h - ds->gradients[id].hue);
            if ((best_diff>diff) && (diff<16))
            {
                best_gradient=id;
                best_diff=diff;
            }
        }

        // Si on a trouv‚ un d‚grad‚ dans lequel inclure le cluster
        if (best_gradient!=-1)
        {
            // On met … jour le d‚grad‚
            if (current->h < ds->gradients[best_gradient].min)
                ds->gradients[best_gradient].min=current->h;
            if (current->h > ds->gradients[best_gradient].max)
                ds->gradients[best_gradient].max=current->h;
            ds->gradients[best_gradient].hue=((ds->gradients[best_gradient].hue*
                        ds->gradients[best_gradient].nb_colors)
                    +current->h)
                /(ds->gradients[best_gradient].nb_colors+1);
            ds->gradients[best_gradient].nb_colors++;
        }
        else
        {
            // On cr‚e un nouveau d‚grad‚
            best_gradient=ds->nb;
            ds->gradients[best_gradient].nb_colors=1;
            ds->gradients[best_gradient].min=current->h;
            ds->gradients[best_gradient].max=current->h;
            ds->gradients[best_gradient].hue=current->h;
            ds->nb++;
        }
        current->h=best_gradient;
    } while((current = current->next));

    // On redistribue les valeurs dans les clusters
    current = cs -> clusters;
    do
        current->h=ds->gradients[current->h].hue;
    while((current = current ->next));
}


/// Compute best palette for given picture.
CT_Node* Optimize_palette(T_Bitmap24B image, int size,
  T_Components * palette, int r, int g, int b)
{	
  T_Occurrence_table * to;
  CT_Node* tc;
  T_Cluster_set * cs;
  T_Gradient_set * ds;

  // Allocate all the elements
  to = 0; tc = 0; cs = 0; ds = 0;

  to = OT_new(r, g, b);
  if (to == NULL)
    return 0;

  tc = CT_new();
  /*
  if (tc == NULL)
  {
  	OT_delete(to);
  	return NULL;
  }
*/
  // Count pixels for each color
  OT_count_occurrences(to, image, size);

  cs = CS_New(256, to);
  if (cs == NULL)
  {
    CT_delete(tc);
    OT_delete(to);
    return 0;
  }
  //CS_Check(cs);
  // Ok, everything was allocated

  // Generate the cluster set with median cut algorithm
  CS_Generate(cs, to, &tc);
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
  CS_Generate_color_table_and_palette(cs, &tc, palette);
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


/// Convert a 24b image to 256 colors (with a given palette and conversion table)
/// This destroys the 24b picture !
/// Uses floyd steinberg dithering.
void Convert_24b_bitmap_to_256_Floyd_Steinberg(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette,CT_Node* tc)
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
  T_Bitmap24B source, int width, int height, __attribute__((unused)) T_Components * palette,
  CT_Node* tc)
{
  T_Bitmap24B current;
  T_Bitmap256 d;
  int x_pos, y_pos;
  int red, green, blue;

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


// Give this one a 24b source, get back the 256c bitmap and its palette
int Convert_24b_bitmap_to_256(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette)
{
  #if defined(__GP2X__) || defined(__gp2x__) || defined(__WIZ__) || defined(__CAANOO__)
  return Convert_24b_bitmap_to_256_fast(dest, source, width, height, palette);  

  #else
  CT_Node* table; // table de conversion
  int                ip;    // index de précision pour la conversion

  // On essaye d'obtenir une table de conversion qui loge en mémoire, avec la
  // meilleure précision possible
  for (ip=0;ip<(10*3);ip+=3)
  {
	table = Optimize_palette(source,width*height,palette,
		precision_24b[ip], precision_24b[ip+1], precision_24b[ip+2]);
	if (table != NULL)
		break;
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

/// Really small, fast and dirty convertor(just for handhelds)
int Convert_24b_bitmap_to_256_fast(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette)
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
