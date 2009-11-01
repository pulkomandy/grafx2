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

void RGB_to_HSL(int r,int g,int b,byte * hr,byte * sr,byte* lr)
{
  double rd,gd,bd,h,s,l,max,min;

  // convert RGB to HSV
  rd = r / 255.0;            // rd,gd,bd range 0-1 instead of 0-255
  gd = g / 255.0;
  bd = b / 255.0;

  // compute L
//  l=(rd*0.30)+(gd*0.59)+(bd*0.11);

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

/////////////////////////////////////////////////////////////////////////////
///////////////////////////// Méthodes de gestion des tables de conversion //
/////////////////////////////////////////////////////////////////////////////

T_Conversion_table * CT_new(int nbb_r,int nbb_g,int nbb_b)
{
  T_Conversion_table * n;
  int size;

  n=(T_Conversion_table *)malloc(sizeof(T_Conversion_table));
  if (n!=NULL)
  {
    // On recopie les paramŠtres demand‚s
    n->nbb_r=nbb_r;
    n->nbb_g=nbb_g;
    n->nbb_b=nbb_b;

    // On calcule les autres
    n->rng_r=(1<<nbb_r);
    n->rng_g=(1<<nbb_g);
    n->rng_b=(1<<nbb_b);
    n->dec_r=nbb_g+nbb_b;
    n->dec_g=nbb_b;
    n->dec_b=0;
    n->red_r=8-nbb_r;
    n->red_g=8-nbb_g;
    n->red_b=8-nbb_b;

    // On tente d'allouer la table
    size=(n->rng_r)*(n->rng_g)*(n->rng_b);
    n->table=(byte *)malloc(size);
    if (n->table!=NULL)
      // C'est bon!
      memset(n->table,0,size); // Inutile, mais plus propre
    else
    {
      // Table impossible … allouer
      free(n);
      n=NULL;
    }
  }

  return n;
}

void CT_delete(T_Conversion_table * t)
{
  free(t->table);
  free(t);
}

byte CT_get(T_Conversion_table * t,int r,int g,int b)
{
  int index;

  // On réduit le nombre de bits par couleur
  r=(r>>t->red_r);
  g=(g>>t->red_g);
  b=(b>>t->red_b);
  
  // On recherche la couleur la plus proche dans la table de conversion
  index=(r<<t->dec_r) | (g<<t->dec_g) | (b<<t->dec_b);

  return t->table[index];
}

void CT_set(T_Conversion_table * t,int r,int g,int b,byte i)
{
  int index;

  index=(r<<t->dec_r) | (g<<t->dec_g) | (b<<t->dec_b);
  t->table[index]=i;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// M‚thodes de gestion des tables d'occurence //
/////////////////////////////////////////////////////////////////////////////

void OT_init(T_Occurrence_table * t)
{
  int size;

  size=(t->rng_r)*(t->rng_g)*(t->rng_b)*sizeof(int);
  memset(t->table,0,size); // On initialise … 0
}

T_Occurrence_table * OT_new(int nbb_r,int nbb_g,int nbb_b)
{
  T_Occurrence_table * n;
  int size;

  n=(T_Occurrence_table *)malloc(sizeof(T_Occurrence_table));
  if (n!=0)
  {
    // On recopie les paramŠtres demand‚s
    n->nbb_r=nbb_r;
    n->nbb_g=nbb_g;
    n->nbb_b=nbb_b;

    // On calcule les autres
    n->rng_r=(1<<nbb_r);
    n->rng_g=(1<<nbb_g);
    n->rng_b=(1<<nbb_b);
    n->dec_r=nbb_g+nbb_b;
    n->dec_g=nbb_b;
    n->dec_b=0;
    n->red_r=8-nbb_r;
    n->red_g=8-nbb_g;
    n->red_b=8-nbb_b;

    // On tente d'allouer la table
    size=(n->rng_r)*(n->rng_g)*(n->rng_b)*sizeof(int);
    n->table=(int *)malloc(size);
    if (n->table!=0)
      // C'est bon! On initialise … 0
      OT_init(n);
    else
    {
      // Table impossible … allouer
      free(n);
      n=0;
    }
  }

  return n;
}

void OT_delete(T_Occurrence_table * t)
{
  free(t->table);
  free(t);
}

int OT_get(T_Occurrence_table * t,int r,int g,int b)
{
  int index;

  index=(r<<t->dec_r) | (g<<t->dec_g) | (b<<t->dec_b);
  return t->table[index];
}

void OT_inc(T_Occurrence_table * t,int r,int g,int b)
{
  int index;

  r=(r>>t->red_r);
  g=(g>>t->red_g);
  b=(b>>t->red_b);
  index=(r<<t->dec_r) | (g<<t->dec_g) | (b<<t->dec_b);
  t->table[index]++;
}

void OT_count_occurrences(T_Occurrence_table* t, T_Bitmap24B image, int size)
{
  T_Bitmap24B ptr;
  int index;

  for (index = size, ptr = image; index > 0; index--, ptr++)
    OT_inc(t, ptr->R, ptr->G, ptr->B);
}

int OT_count_colors(T_Occurrence_table * t)
{
  int val; // Valeur de retour
  int nb; // Nombre de couleurs … tester
  int i; // Compteur de couleurs test‚es

  val = 0;
  nb=(t->rng_r)*(t->rng_g)*(t->rng_b);
  for (i = 0; i < nb; i++)
    if (t->table[i]>0)
      val++;

  return val;
}



/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// M‚thodes de gestion des clusters //
/////////////////////////////////////////////////////////////////////////////

void Cluster_pack(T_Cluster * c,T_Occurrence_table * to)
{
  int rmin,rmax,vmin,vmax,bmin,bmax;
  int r,g,b;

  // On cherche les mins et les maxs de chaque composante sur la couverture

  // int nbocc;

  // On prédécale tout pour éviter de faire trop de bazar en se forçant à utiliser OT_get, plus rapide
  rmin=c->rmax <<16; rmax=c->rmin << 16;
  vmin=c->vmax << 8; vmax=c->vmin << 8;
  bmin=c->bmax; bmax=c->bmin;
  c->occurences=0;
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

  // On recherche le minimum et le maximum en parcourant le cluster selon chaque composante, 
  // ça évite des accès mémoires inutiles, de plus chaque boucle est plus petite que la 
  // précédente puisqu'on connait une borne supplémentaire

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
  // Il faut quand même parcourir la partie utile du cluster, pour savoir combien il y a d'occurences
  for(r=rmin;r<=rmax;r+=1<<16)
      for(g=vmin;g<=vmax;g+=1<<8)
          for(b=bmin;b<=bmax;b++)
          {
            c->occurences+=to->table[r + g + b]; // OT_get
          }

  c->rmin=rmin>>16; c->rmax=rmax>>16;
  c->vmin=vmin>>8;  c->vmax=vmax>>8;
  c->bmin=bmin;     c->bmax=bmax;

  // On regarde la composante qui a la variation la plus grande
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

void Cluster_split(T_Cluster * c, T_Cluster * c1, T_Cluster * c2, int hue,
	T_Occurrence_table * to)
{
  int limit;
  int cumul;
  int r, g, b;

  limit = c->occurences / 2;
  cumul = 0;
  if (hue == 0)
  {
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

    if (r==c->rmin)
      r++;
    // R est la valeur de d‚but du 2nd cluster

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
  if (hue==1)
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
    // G est la valeur de d‚but du 2nd cluster

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
  else
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
    // B est la valeur de d‚but du 2nd cluster

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



/////////////////////////////////////////////////////////////////////////////
//////////////////////////// M‚thodes de gestion des ensembles de clusters //
/////////////////////////////////////////////////////////////////////////////

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
    // On recopie les paramŠtres demand‚s
    n->nb_max = OT_count_colors(to);

    // On vient de compter le nombre de couleurs existantes, s'il est plus grand
	// que 256 on limite à 256
	// (nombre de couleurs voulu au final)
    if (n->nb_max > nbmax)
    {
      n->nb_max = nbmax;
    }

    // On tente d'allouer le premier cluster
    n->clusters=(T_Cluster *)malloc(sizeof(T_Cluster));
    if (n->clusters != NULL)
      // C'est bon! On initialise
      CS_Init(n, to);
    else
    {
      // Table impossible … allouer
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
}

void CS_Get(T_Cluster_set * cs, T_Cluster * c)
{
  T_Cluster* current = cs->clusters;
  T_Cluster* prev = NULL;

  // Search a cluster with at least 2 distinct colors so we can split it
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

void CS_Set(T_Cluster_set * cs,T_Cluster * c)
{
  T_Cluster* current = cs->clusters;
  T_Cluster* prev = NULL;

  // Search the first cluster that is smaller than ours
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

// Détermination de la meilleure palette en utilisant l'algo Median Cut :
// 1) On considère l'espace (R,G,B) comme 1 boîte
// 2) On cherche les extrêmes de la boîte en (R,G,B)
// 3) On trie les pixels de l'image selon l'axe le plus long parmi (R,G,B)
// 4) On coupe la boîte en deux au milieu, et on compacte pour que chaque bord
// corresponde bien à un pixel extreme
// 5) On recommence à couper selon le plus grand axe toutes boîtes confondues
// 6) On s'arrête quand on a le nombre de couleurs voulu
void CS_Generate(T_Cluster_set * cs, T_Occurrence_table * to)
{
  T_Cluster current;
  T_Cluster Nouveau1;
  T_Cluster Nouveau2;

  // Tant qu'on a moins de 256 clusters
  while (cs->nb<cs->nb_max)
  {
    // On récupère le plus grand cluster
    CS_Get(cs,&current);

    // On le coupe en deux
    Cluster_split(&current, &Nouveau1, &Nouveau2, current.plus_large, to);

    // On compacte ces deux nouveaux (il peut y avoir un espace entre l'endroit
	// de la coupure et les premiers pixels du cluster)
    Cluster_pack(&Nouveau1, to);
    Cluster_pack(&Nouveau2, to);

	// On les remet dans le set
    CS_Set(cs,&Nouveau1);
    CS_Set(cs,&Nouveau2);
    
  }
}

void CS_Compute_colors(T_Cluster_set * cs, T_Occurrence_table * to)
{
  T_Cluster * c;

  for (c=cs->clusters;c!=NULL;c=c->next)
    Cluster_compute_hue(c,to);
}

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

	// Put the new list bavk in place
	cs->clusters = newlist;
}

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

void CS_Generate_color_table_and_palette(T_Cluster_set * cs,T_Conversion_table * tc,T_Components * palette)
{
  int index;
  int r,g,b;
  T_Cluster* current = cs->clusters;

  for (index=0;index<cs->nb;index++)
  {
    palette[index].R=current->r;
    palette[index].G=current->g;
    palette[index].B=current->b;

    for (r=current->Rmin; r<=current->Rmax; r++)
      for (g=current->Gmin;g<=current->Vmax;g++)
        for (b=current->Bmin;b<=current->Bmax;b++)
          CT_set(tc,r,g,b,index);
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
        // On recopie les paramŠtres demand‚s
        n->nb_max=cs->nb_max;

        // On tente d'allouer la table
        n->gradients=(T_Gradient *)malloc((n->nb_max)*sizeof(T_Gradient));
        if (n->gradients!=0)
            // C'est bon! On initialise
            GS_Init(n,cs);
        else
        {
            // Table impossible … allouer
            free(n);
            n=0;
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




T_Conversion_table * Optimize_palette(T_Bitmap24B image, int size,
	T_Components * palette, int r, int g, int b)
{
  T_Occurrence_table * to;
  T_Conversion_table * tc;
  T_Cluster_set * cs;
  T_Gradient_set * ds;

  // Création des éléments nécessaires au calcul de palette optimisée:
  to = 0; tc = 0; cs = 0; ds = 0;

  to = OT_new(r, g, b);
  if (to == NULL)
	return 0;

  tc = CT_new(r, g, b);
  if (tc == NULL)
  {
	OT_delete(to);
	return 0;
  }

  // Première étape : on compte les pixels de chaque couleur pour pouvoir trier là dessus
  OT_count_occurrences(to, image, size);

  cs = CS_New(256, to);
  if (cs == NULL)
  {
    CT_delete(tc);
    OT_delete(to);
	return 0;
  }
  //CS_Check(cs);
  // C'est bon, on a pu tout allouer

  // On génère les clusters (avec l'algo du median cut)
  CS_Generate(cs, to);
  //CS_Check(cs);

  // On calcule la teinte de chaque pixel (Luminance et chrominance)
  CS_Compute_colors(cs, to);
  //CS_Check(cs);

  ds = GS_New(cs);
  if (ds!= NULL)
  {
	  GS_Generate(ds, cs);
	  GS_Delete(ds);
  }
  // Enfin on trie les clusters (donc les couleurs de la palette) dans un ordre
  // sympa : par couleur, et par luminosité pour chaque couleur
  CS_Sort_by_luminance(cs);
  //CS_Check(cs);
  CS_Sort_by_chrominance(cs);
  //CS_Check(cs);

  // Enfin on génère la palette et la table de correspondance entre chaque
  // couleur 24b et sa couleur palette associée.
  CS_Generate_color_table_and_palette(cs, tc, palette);
  //CS_Check(cs);

  CS_Delete(cs);
  OT_delete(to);
  return tc;
}

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

void Convert_24b_bitmap_to_256_Floyd_Steinberg(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette,T_Conversion_table * tc)
// Cette fonction dégrade au fur et à mesure le bitmap source, donc soit on ne
// s'en ressert pas, soit on passe à la fonction une copie de travail du
// bitmap original.
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

void Convert_24b_bitmap_to_256_nearest_neighbor(T_Bitmap256 dest,
	T_Bitmap24B source, int width, int height, __attribute__((unused)) T_Components * palette,
	T_Conversion_table * tc)
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


// Convertie avec le plus de précision possible une image 24b en 256c
// Renvoie s'il y a eu une erreur ou pas..

// Cette fonction utilise l'algorithme "median cut" (Optimize_palette) pour trouver la palette, et diffuse les erreurs avec floyd-steinberg.

int Convert_24b_bitmap_to_256(T_Bitmap256 dest,T_Bitmap24B source,int width,int height,T_Components * palette)
{
  T_Conversion_table * table; // table de conversion
  int                ip;    // index de précision pour la conversion

  // On essaye d'obtenir une table de conversion qui loge en mémoire, avec la
  // meilleure précision possible
  for (ip=0;ip<(10*3);ip+=3)
  {
    table=Optimize_palette(source,width*height,palette,precision_24b[ip+0],
                            precision_24b[ip+1],precision_24b[ip+2]);
    if (table!=0)
      break;
  }
  if (table!=0)
  {
    //Convert_24b_bitmap_to_256_Floyd_Steinberg(dest,source,width,height,palette,table);
    Convert_24b_bitmap_to_256_nearest_neighbor(dest,source,width,height,palette,table);
    CT_delete(table);
    return 0;
  }
  else
    return 1;
}



