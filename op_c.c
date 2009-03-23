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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

#include "op_c.h"
#include "erreurs.h"

void RGBtoHSL(int r,int g,int b,byte * hr,byte * sr,byte* lr)
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

void HSLtoRGB(byte h,byte s,byte l, byte* r, byte* g, byte* b)
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

Table_conversion * TC_New(int nbb_r,int nbb_g,int nbb_b)
{
  Table_conversion * n;
  int Taille;

  n=(Table_conversion *)malloc(sizeof(Table_conversion));
  if (n!=NULL)
  {
    // On recopie les paramŠtres demand‚s
    n->nbb_r=nbb_r;
    n->nbb_g=nbb_g;
    n->nbb_b=nbb_b;

    // On calcule les autres
    n->rng_r=(1<<nbb_r);
    n->rng_v=(1<<nbb_g);
    n->rng_b=(1<<nbb_b);
    n->dec_r=nbb_g+nbb_b;
    n->dec_v=nbb_b;
    n->dec_b=0;
    n->red_r=8-nbb_r;
    n->red_v=8-nbb_g;
    n->red_b=8-nbb_b;

    // On tente d'allouer la table
    Taille=(n->rng_r)*(n->rng_v)*(n->rng_b);
    n->table=(byte *)malloc(Taille);
    if (n->table!=NULL)
      // C'est bon!
      memset(n->table,0,Taille); // Inutile, mais plus propre
    else
    {
      // Table impossible … allouer
      free(n);
      n=NULL;
    }
  }

  return n;
}

void TC_Delete(Table_conversion * t)
{
  free(t->table);
  free(t);
}

byte TC_Get(Table_conversion * t,int r,int g,int b)
{
  int index;

  // On réduit le nombre de bits par couleur
  r=(r>>t->red_r);
  g=(g>>t->red_v);
  b=(b>>t->red_b);
  
  // On recherche la couleur la plus proche dans la table de conversion
  index=(r<<t->dec_r) | (g<<t->dec_v) | (b<<t->dec_b);

  return t->table[index];
}

void TC_Set(Table_conversion * t,int r,int g,int b,byte i)
{
  int index;

  index=(r<<t->dec_r) | (g<<t->dec_v) | (b<<t->dec_b);
  t->table[index]=i;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// M‚thodes de gestion des tables d'occurence //
/////////////////////////////////////////////////////////////////////////////

void TO_Init(Table_occurence * t)
{
  int Taille;

  Taille=(t->rng_r)*(t->rng_v)*(t->rng_b)*sizeof(int);
  memset(t->table,0,Taille); // On initialise … 0
}

Table_occurence * TO_New(int nbb_r,int nbb_g,int nbb_b)
{
  Table_occurence * n;
  int Taille;

  n=(Table_occurence *)malloc(sizeof(Table_occurence));
  if (n!=0)
  {
    // On recopie les paramŠtres demand‚s
    n->nbb_r=nbb_r;
    n->nbb_g=nbb_g;
    n->nbb_b=nbb_b;

    // On calcule les autres
    n->rng_r=(1<<nbb_r);
    n->rng_v=(1<<nbb_g);
    n->rng_b=(1<<nbb_b);
    n->dec_r=nbb_g+nbb_b;
    n->dec_v=nbb_b;
    n->dec_b=0;
    n->red_r=8-nbb_r;
    n->red_v=8-nbb_g;
    n->red_b=8-nbb_b;

    // On tente d'allouer la table
    Taille=(n->rng_r)*(n->rng_v)*(n->rng_b)*sizeof(int);
    n->table=(int *)malloc(Taille);
    if (n->table!=0)
      // C'est bon! On initialise … 0
      TO_Init(n);
    else
    {
      // Table impossible … allouer
      free(n);
      n=0;
    }
  }

  return n;
}

void TO_Delete(Table_occurence * t)
{
  free(t->table);
  free(t);
}

int TO_Get(Table_occurence * t,int r,int g,int b)
{
  int index;

  index=(r<<t->dec_r) | (g<<t->dec_v) | (b<<t->dec_b);
  return t->table[index];
}

void TO_Set(Table_occurence * t,int r,int g,int b,int i)
{
  int index;

  r=(r>>t->red_r);
  g=(g>>t->red_v);
  b=(b>>t->red_b);
  index=(r<<t->dec_r) | (g<<t->dec_v) | (b<<t->dec_b);
  t->table[index]=i;
}

void TO_Inc(Table_occurence * t,int r,int g,int b)
{
  int index;

  r=(r>>t->red_r);
  g=(g>>t->red_v);
  b=(b>>t->red_b);
  index=(r<<t->dec_r) | (g<<t->dec_v) | (b<<t->dec_b);
  t->table[index]++;
}

void TO_Compter_occurences(Table_occurence * t,Bitmap24B image,int Taille)
{
  Bitmap24B ptr;
  int index;

  for (index=Taille,ptr=image;index>0;index--,ptr++)
    TO_Inc(t,ptr->R,ptr->G,ptr->B);
}

int TO_Compter_couleurs(Table_occurence * t)
{
  int val; // Valeur de retour
  int nb;  // Nombre de couleurs … tester
  int i;   // Compteur de couleurs test‚es

  val=0;
  nb=(t->rng_r)*(t->rng_v)*(t->rng_b);
  for (i=0;i<nb;i++)
    if (t->table[i]>0)
      val++;

  return val;
}



/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// M‚thodes de gestion des clusters //
/////////////////////////////////////////////////////////////////////////////

void Cluster_Analyser(Cluster * c,Table_occurence * to)
{
  int rmin,rmax,vmin,vmax,bmin,bmax;
  int r,g,b;

  // On cherche les mins et les maxs de chaque composante sur la couverture

  // int nbocc;

  // On prédécale tout pour éviter de faire trop de bazar en se forçant à utiliser TO_Get, plus rapide
  rmin=c->rmax <<16; rmax=c->rmin << 16;
  vmin=c->vmax << 8; vmax=c->vmin << 8;
  bmin=c->bmax; bmax=c->bmin;
  c->occurences=0;
  /*
  for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
    for (g=c->vmin<<8;g<=c->vmax<<8;g+=1<<8)
      for (b=c->bmin;b<=c->bmax;b++)
      {
        nbocc=to->table[r + g + b]; // TO_Get
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
            if(to->table[r + g + b]) // TO_Get
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
            if(to->table[r + g + b]) // TO_Get
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
            if(to->table[r + g + b]) // TO_Get
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
            if(to->table[r + g + b]) // TO_Get
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
            if(to->table[r + g + b]) // TO_Get
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
            if(to->table[r + g + b]) // TO_Get
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
            c->occurences+=to->table[r + g + b]; // TO_Get
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

void Cluster_Split(Cluster * c,Cluster * c1,Cluster * c2,int Teinte,Table_occurence * to)
{
  int limit;
  int cumul;
  int r,g,b;

  limit=(c->occurences)/2;
  cumul=0;
  if (Teinte==0)
  {
    for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
    {
      for (g=c->vmin<<8;g<=c->vmax<<8;g+=1<<8)
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

    r>>=16;
    g>>=8;

    if (r==c->rmin)
      r++;
    // R est la valeur de d‚but du 2nd cluster

    c1->Rmin=c->Rmin; c1->Rmax=r-1;
    c1->rmin=c->rmin; c1->rmax=r-1;
    c1->Vmin=c->Vmin; c1->Vmax=c->Vmax;
    c1->vmin=c->vmin; c1->vmax=c->vmax;
    c1->Bmin=c->Bmin; c1->Bmax=c->Bmax;
    c1->bmin=c->bmin; c1->bmax=c->bmax;
    c2->Rmin=r;       c2->Rmax=c->Rmax;
    c2->rmin=r;       c2->rmax=c->rmax;
    c2->Vmin=c->Vmin; c2->Vmax=c->Vmax;
    c2->vmin=c->vmin; c2->vmax=c->vmax;
    c2->Bmin=c->Bmin; c2->Bmax=c->Bmax;
    c2->bmin=c->bmin; c2->bmax=c->bmax;
  }
  else
  if (Teinte==1)
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
    c1->Vmin=c->Vmin; c1->Vmax=g-1;
    c1->vmin=c->vmin; c1->vmax=g-1;
    c1->Bmin=c->Bmin; c1->Bmax=c->Bmax;
    c1->bmin=c->bmin; c1->bmax=c->bmax;
    c2->Rmin=c->Rmin; c2->Rmax=c->Rmax;
    c2->rmin=c->rmin; c2->rmax=c->rmax;
    c2->Vmin=g;       c2->Vmax=c->Vmax;
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
    c1->Vmin=c->Vmin; c1->Vmax=c->Vmax;
    c1->vmin=c->vmin; c1->vmax=c->vmax;
    c1->Bmin=c->Bmin; c1->Bmax=b-1;
    c1->bmin=c->bmin; c1->bmax=b-1;
    c2->Rmin=c->Rmin; c2->Rmax=c->Rmax;
    c2->rmin=c->rmin; c2->rmax=c->rmax;
    c2->Vmin=c->Vmin; c2->Vmax=c->Vmax;
    c2->vmin=c->vmin; c2->vmax=c->vmax;
    c2->Bmin=b;       c2->Bmax=c->Bmax;
    c2->bmin=b;       c2->bmax=c->bmax;
  }
}

void Cluster_Calculer_teinte(Cluster * c,Table_occurence * to)
{
  int cumulR,cumulV,cumulB;
  int r,g,b;
  int nbocc;

  byte s=0;

  cumulR=cumulV=cumulB=0;
  for (r=c->rmin;r<=c->rmax;r++)
    for (g=c->vmin;g<=c->vmax;g++)
      for (b=c->bmin;b<=c->bmax;b++)
      {
        nbocc=TO_Get(to,r,g,b);
        if (nbocc)
        {
          cumulR+=r*nbocc;
          cumulV+=g*nbocc;
          cumulB+=b*nbocc;
        }
      }
  
  c->r=(cumulR<<to->red_r)/c->occurences;
  c->g=(cumulV<<to->red_v)/c->occurences;
  c->b=(cumulB<<to->red_b)/c->occurences;
  RGBtoHSL(c->r,c->g,c->b,&c->h,&s,&c->l);
}



/////////////////////////////////////////////////////////////////////////////
//////////////////////////// M‚thodes de gestion des ensembles de clusters //
/////////////////////////////////////////////////////////////////////////////

void CS_Init(ClusterSet * cs,Table_occurence * to)
{
  cs->clusters[0].Rmin=cs->clusters[0].rmin=0;
  cs->clusters[0].Vmin=cs->clusters[0].vmin=0;
  cs->clusters[0].Bmin=cs->clusters[0].bmin=0;
  cs->clusters[0].Rmax=cs->clusters[0].rmax=to->rng_r-1;
  cs->clusters[0].Vmax=cs->clusters[0].vmax=to->rng_v-1;
  cs->clusters[0].Bmax=cs->clusters[0].bmax=to->rng_b-1;
  Cluster_Analyser(cs->clusters+0,to);
  // Et hop : le 1er ensemble de couleurs est initialis‚
  cs->nb=1;
}

ClusterSet * CS_New(int nbmax,Table_occurence * to)
{
  ClusterSet * n;

  n=(ClusterSet *)malloc(sizeof(ClusterSet));
  if (n!=0)
  {
    // On recopie les paramŠtres demand‚s
    n->nb_max=TO_Compter_couleurs(to);

    // On vient de compter le nombre de couleurs existantes, s'il est plus grand que 256 on limit à 256 (nombre de couleurs voulu au final)
    if (n->nb_max>nbmax)
    {
      n->nb_max=nbmax;
    }

    // On tente d'allouer la table
    n->clusters=(Cluster *)malloc(nbmax*sizeof(Cluster));
    if (n->clusters!=NULL)
      // C'est bon! On initialise
      CS_Init(n,to);
    else
    {
      // Table impossible … allouer
      free(n);
      n=0;
    }
  }

  return n;
}

void CS_Delete(ClusterSet * cs)
{
  free(cs->clusters);
  free(cs);
}

void CS_Get(ClusterSet * cs,Cluster * c)
{
  int index;

  // On cherche un cluster que l'on peut couper en deux, donc avec au moins deux valeurs
  // différentes sur l'une des composantes
  for (index=0;index<cs->nb;index++)
    if ( (cs->clusters[index].rmin<cs->clusters[index].rmax) ||
         (cs->clusters[index].vmin<cs->clusters[index].vmax) ||
         (cs->clusters[index].bmin<cs->clusters[index].bmax) )
      break;

  // On le recopie dans c
  *c=cs->clusters[index];

  // On décrémente le nombre et on décale tous les clusters suivants
  // Sachant qu'on va réinsérer juste après, il me semble que ça serait une bonne idée de gérer les clusters 
  // comme une liste chainée... on n'a aucun accès direct dedans, que des parcours ...
  cs->nb--;
  memcpy((cs->clusters+index),(cs->clusters+index+1),(cs->nb-index)*sizeof(Cluster));
}

void CS_Set(ClusterSet * cs,Cluster * c)
{
  int index;
  // int decalage;

  // Le tableau des clusters est trié par nombre d'occurences. Donc on cherche la position du premier cluster 
  // qui est plus grand que le notre
  for (index=0;index<cs->nb;index++)
    if (cs->clusters[index].occurences<c->occurences)
/*
    if (((OPTPAL_Cluster[index].rmax-OPTPAL_Cluster[index].rmin+1)*
         (OPTPAL_Cluster[index].gmax-OPTPAL_Cluster[index].gmin+1)*
         (OPTPAL_Cluster[index].bmax-OPTPAL_Cluster[index].bmin+1))
        <
        ((Set->rmax-Set->rmin+1)*
         (Set->gmax-Set->gmin+1)*
         (Set->bmax-Set->bmin+1))
       )
*/
      break;

  if (index<cs->nb)
  {
    // On distingue ici une insertion plutot qu'un placement en fin de liste.
    // On doit donc décaler les ensembles suivants vers la fin pour se faire
    // une place dans la liste.

    //for (decalage=cs->nb;decalage>index;decalage--)
    //  memcpy((cs->clusters+decalage),(cs->clusters+decalage-1),sizeof(Cluster));
    memmove(cs->clusters+index+1,cs->clusters+index,(cs->nb-index)*sizeof(Cluster));
  }

  cs->clusters[index]=*c;
  cs->nb++;
}

// Détermination de la meilleure palette en utilisant l'algo Median Cut :
// 1) On considère l'espace (R,G,B) comme 1 boîte
// 2) On cherche les extrêmes de la boîte en (R,G,B)
// 3) On trie les pixels de l'image selon l'axe le plus long parmi (R,G,B)
// 4) On coupe la boîte en deux au milieu, et on compacte pour que chaque bord corresponde bien à un pixel extreme
// 5) On recommence à couper selon le plus grand axe toutes boîtes confondues
// 6) On s'arrête quand on a le nombre de couleurs voulu
void CS_Generer(ClusterSet * cs,Table_occurence * to)
{
  Cluster Courant;
  Cluster Nouveau1;
  Cluster Nouveau2;

  // Tant qu'on a moins de 256 clusters
  while (cs->nb<cs->nb_max)
  {
    // On récupère le plus grand cluster
    CS_Get(cs,&Courant);

    // On le coupe en deux
    Cluster_Split(&Courant,&Nouveau1,&Nouveau2,Courant.plus_large,to);

    // On compacte ces deux nouveaux (il peut y avoir un espace entre l'endroit de la coupure et les premiers pixels du cluster)
    Cluster_Analyser(&Nouveau1,to);
    Cluster_Analyser(&Nouveau2,to);

    // On met ces deux nouveaux clusters dans le clusterSet... sauf s'ils sont vides
    if(Nouveau1.occurences>0)
        CS_Set(cs,&Nouveau1);
    if(Nouveau2.occurences>0)
        CS_Set(cs,&Nouveau2);
  }
}

void CS_Calculer_teintes(ClusterSet * cs,Table_occurence * to)
{
  int index;
  Cluster * c;

  for (index=0,c=cs->clusters;index<cs->nb;index++,c++)
    Cluster_Calculer_teinte(c,to);
}

void CS_Trier_par_chrominance(ClusterSet * cs)
{
  int byte_used[256];
  int decalages[256];
  int index;
  Cluster * nc;

  nc=(Cluster *)malloc(cs->nb_max*sizeof(Cluster));

  // Initialisation de la table d'occurence de chaque octet
  for (index=0;index<256;index++)
    byte_used[index]=0;

  // Comptage du nombre d'occurences de chaque octet
  for (index=0;index<cs->nb;index++)
    byte_used[cs->clusters[index].h]++;

  // Calcul de la table des d‚calages
  decalages[0]=0;
  for (index=1;index<256;index++)
    decalages[index]=decalages[index-1]+byte_used[index-1];

  // Copie r‚ordonn‚e dans la nouvelle liste
  for (index=0;index<cs->nb;index++)
  {
    nc[decalages[cs->clusters[index].h]]=cs->clusters[index];
    decalages[cs->clusters[index].h]++;
  }

  // Remplacement de la liste d‚sordonn‚e par celle tri‚e
  free(cs->clusters);
  cs->clusters=nc;
}

void CS_Trier_par_luminance(ClusterSet * cs)
{
  int byte_used[256];
  int decalages[256];
  int index;
  Cluster * nc;

  nc=(Cluster *)malloc(cs->nb_max*sizeof(Cluster));

  // Initialisation de la table d'occurence de chaque octet
  for (index=0;index<256;index++)
    byte_used[index]=0;

  // Comptage du nombre d'occurences de chaque octet
  for (index=0;index<cs->nb;index++)
    byte_used[cs->clusters[index].l]++;

  // Calcul de la table des d‚calages
  decalages[0]=0;
  for (index=1;index<256;index++)
    decalages[index]=decalages[index-1]+byte_used[index-1];

  // Copie r‚ordonn‚e dans la nouvelle liste
  for (index=0;index<cs->nb;index++)
  {
    nc[decalages[cs->clusters[index].l]]=cs->clusters[index];
    decalages[cs->clusters[index].l]++;
  }

  // Remplacement de la liste d‚sordonn‚e par celle tri‚e
  free(cs->clusters);
  cs->clusters=nc;
}

void CS_Generer_TC_et_Palette(ClusterSet * cs,Table_conversion * tc,Composantes * palette)
{
  int index;
  int r,g,b;

  for (index=0;index<cs->nb;index++)
  {
    palette[index].R=cs->clusters[index].r;
    palette[index].G=cs->clusters[index].g;
    palette[index].B=cs->clusters[index].b;

    for (r=cs->clusters[index].Rmin;r<=cs->clusters[index].Rmax;r++)
      for (g=cs->clusters[index].Vmin;g<=cs->clusters[index].Vmax;g++)
        for (b=cs->clusters[index].Bmin;b<=cs->clusters[index].Bmax;b++)
          TC_Set(tc,r,g,b,index);
  }
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Méthodes de gestion des dégradés //
/////////////////////////////////////////////////////////////////////////////

void DS_Init(DegradeSet * ds,ClusterSet * cs)
{
    ds->gradients[0].nb_colors=1;
    ds->gradients[0].min=cs->clusters[0].h;
    ds->gradients[0].max=cs->clusters[0].h;
    ds->gradients[0].hue=cs->clusters[0].h;
    // Et hop : le 1er ensemble de d‚grad‚s est initialis‚
    ds->nb=1;
}

DegradeSet * DS_New(ClusterSet * cs)
{
    DegradeSet * n;

    n=(DegradeSet *)malloc(sizeof(DegradeSet));
    if (n!=NULL)
    {
        // On recopie les paramŠtres demand‚s
        n->nb_max=cs->nb_max;

        // On tente d'allouer la table
        n->gradients=(Degrade *)malloc((n->nb_max)*sizeof(Degrade));
        if (n->gradients!=0)
            // C'est bon! On initialise
            DS_Init(n,cs);
        else
        {
            // Table impossible … allouer
            free(n);
            n=0;
        }
    }

    return n;
}

void DS_Delete(DegradeSet * ds)
{
    free(ds->gradients);
    free(ds);
}

void DS_Generer(DegradeSet * ds,ClusterSet * cs)
{
    int ic,id; // Les indexs de parcours des ensembles
    int mdegr; // Meilleur d‚grad‚
    int mdiff; // Meilleure diff‚rence de chrominance
    int diff;  // Difference de chrominance courante

    // Pour chacun des clusters … traiter
    for (ic=1;ic<cs->nb;ic++)
    {
        // On recherche le d‚grad‚ le plus proche de la chrominance du cluster
        mdegr=-1;
        mdiff=99999999;
        for (id=0;id<ds->nb;id++)
        {
            diff=abs(cs->clusters[ic].h - ds->gradients[id].hue);
            if ((mdiff>diff) && (diff<16))
            {
                mdegr=id;
                mdiff=diff;
            }
        }

        // Si on a trouv‚ un d‚grad‚ dans lequel inclure le cluster
        if (mdegr!=-1)
    {
      // On met … jour le d‚grad‚
      if (cs->clusters[ic].h < ds->gradients[mdegr].min)
        ds->gradients[mdegr].min=cs->clusters[ic].h;
      if (cs->clusters[ic].h > ds->gradients[mdegr].max)
        ds->gradients[mdegr].max=cs->clusters[ic].h;
      ds->gradients[mdegr].hue=((ds->gradients[mdegr].hue*
                                ds->gradients[mdegr].nb_colors)
                               +cs->clusters[ic].h)
                              /(ds->gradients[mdegr].nb_colors+1);
      ds->gradients[mdegr].nb_colors++;
    }
    else
    {
      // On cr‚e un nouveau d‚grad‚
      mdegr=ds->nb;
      ds->gradients[mdegr].nb_colors=1;
      ds->gradients[mdegr].min=cs->clusters[ic].h;
      ds->gradients[mdegr].max=cs->clusters[ic].h;
      ds->gradients[mdegr].hue=cs->clusters[ic].h;
      ds->nb++;
    }
    cs->clusters[ic].h=mdegr;
  }

  // On redistribue les valeurs dans les clusters
  for (ic=0;ic<cs->nb;ic++)
    cs->clusters[ic].h=ds->gradients[cs->clusters[ic].h].hue;
}




Table_conversion * Optimiser_palette(Bitmap24B image,int Taille,Composantes * palette,int r,int g,int b)
{
  Table_occurence  * to;
  Table_conversion * tc;
  ClusterSet       * cs;
  DegradeSet       * ds;

  // Création des éléments nécessaires au calcul de palette optimisée:
  to=0; tc=0; cs=0; ds=0;

  to=TO_New(r,g,b);
  if (to!=0)
  {
    tc=TC_New(r,g,b);
    if (tc!=0)
    {

      // Première étape : on compte les pixels de chaque couleur pour pouvoir trier là dessus
      TO_Compter_occurences(to,image,Taille);

      cs=CS_New(256,to);
      if (cs!=0)
      {
        // C'est bon, on a pu tout allouer

        // On génère les clusters (avec l'algo du median cut)
        CS_Generer(cs,to);

        // On calcule la teinte de chaque pixel (Luminance et chrominance)
        CS_Calculer_teintes(cs,to);

        ds=DS_New(cs);
        if (ds!=0)
        {
          DS_Generer(ds,cs);
          DS_Delete(ds);
        }

        // Enfin on trie les clusters (donc les couleurs de la palette) dans un ordre sympa : par couleur, et par luminosité pour chaque couleur
        CS_Trier_par_luminance(cs);
        CS_Trier_par_chrominance(cs);

        // Enfin on génère la palette et la table de correspondance entre chaque couleur 24b et sa couleur palette associée.
        CS_Generer_TC_et_Palette(cs,tc,palette);

        CS_Delete(cs);
        TO_Delete(to);
        return tc;
      }
      TC_Delete(tc);
    }
    TO_Delete(to);
  }
  // Si on arrive ici c'est que l'allocation n'a pas réussi,
  // l'appelant devra recommencer avec une précision plus faible (3 derniers paramètres)
  return 0;
}

int Valeur_modifiee(int Valeur,int modif)
{
  Valeur+=modif;
  if (Valeur<0)
  {
    Valeur=0;
  }
  else if (Valeur>255)
  {
    Valeur=255;
  }
  return Valeur;
}

void Convert_bitmap_24B_to_256_Floyd_Steinberg(Bitmap256 Dest,Bitmap24B Source,int width,int height,Composantes * palette,Table_conversion * tc)
// Cette fonction dégrade au fur et à mesure le bitmap source, donc soit on ne
// s'en ressert pas, soit on passe à la fonction une copie de travail du
// bitmap original.
{
  Bitmap24B Courant;
  Bitmap24B C_plus1;
  Bitmap24B S_moins1;
  Bitmap24B Suivant;
  Bitmap24B S_plus1;
  Bitmap256 d;
  int x_pos,y_pos;
  int Rouge,Vert,Bleu;
  float ERouge,EVert,EBleu;

  // On initialise les variables de parcours:
  Courant =Source;      // Le pixel dont on s'occupe
  Suivant =Courant+width; // Le pixel en dessous
  C_plus1 =Courant+1;   // Le pixel à droite
  S_moins1=Suivant-1;   // Le pixel en bas à gauche
  S_plus1 =Suivant+1;   // Le pixel en bas à droite
  d       =Dest;

  // On parcours chaque pixel:
  for (y_pos=0;y_pos<height;y_pos++)
  {
    for (x_pos=0;x_pos<width;x_pos++)
    {
      // On prends la meilleure couleur de la palette qui traduit la couleur
      // 24 bits de la source:
      Rouge=Courant->R;
      Vert =Courant->G;
      Bleu =Courant->B;
      // Cherche la couleur correspondant dans la palette et la range dans l'image de destination
      *d=TC_Get(tc,Rouge,Vert,Bleu);

      // Puis on calcule pour chaque composante l'erreur dûe à l'approximation
      Rouge-=palette[*d].R;
      Vert -=palette[*d].G;
      Bleu -=palette[*d].B;

      // Et dans chaque pixel voisin on propage l'erreur
      // A droite:
        ERouge=(Rouge*7)/16.0;
        EVert =(Vert *7)/16.0;
        EBleu =(Bleu *7)/16.0;
        if (x_pos+1<width)
        {
          // Valeur_modifiee fait la somme des 2 params en bornant sur [0,255]
          C_plus1->R=Valeur_modifiee(C_plus1->R,ERouge);
          C_plus1->G=Valeur_modifiee(C_plus1->G,EVert );
          C_plus1->B=Valeur_modifiee(C_plus1->B,EBleu );
        }
      // En bas à gauche:
      if (y_pos+1<height)
      {
        ERouge=(Rouge*3)/16.0;
        EVert =(Vert *3)/16.0;
        EBleu =(Bleu *3)/16.0;
        if (x_pos>0)
        {
          S_moins1->R=Valeur_modifiee(S_moins1->R,ERouge);
          S_moins1->G=Valeur_modifiee(S_moins1->G,EVert );
          S_moins1->B=Valeur_modifiee(S_moins1->B,EBleu );
        }
      // En bas:
        ERouge=(Rouge*5/16.0);
        EVert =(Vert*5 /16.0);
        EBleu =(Bleu*5 /16.0);
        Suivant->R=Valeur_modifiee(Suivant->R,ERouge);
        Suivant->G=Valeur_modifiee(Suivant->G,EVert );
        Suivant->B=Valeur_modifiee(Suivant->B,EBleu );
      // En bas à droite:
        if (x_pos+1<width)
        {
        ERouge=(Rouge/16.0);
        EVert =(Vert /16.0);
        EBleu =(Bleu /16.0);
          S_plus1->R=Valeur_modifiee(S_plus1->R,ERouge);
          S_plus1->G=Valeur_modifiee(S_plus1->G,EVert );
          S_plus1->B=Valeur_modifiee(S_plus1->B,EBleu );
        }
      }

      // On passe au pixel suivant :
      Courant++;
      C_plus1++;
      S_moins1++;
      Suivant++;
      S_plus1++;
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

// Cette fonction utilise l'algorithme "median cut" (Optimiser_palette) pour trouver la palette, et diffuse les erreurs avec floyd-steinberg.

int Convert_bitmap_24B_to_256(Bitmap256 Dest,Bitmap24B Source,int width,int height,Composantes * palette)
{
  Table_conversion * table; // table de conversion
  int                ip;    // index de précision pour la conversion

  // On essaye d'obtenir une table de conversion qui loge en mémoire, avec la
  // meilleure précision possible
  for (ip=0;ip<(10*3);ip+=3)
  {
    table=Optimiser_palette(Source,width*height,palette,precision_24b[ip+0],
                            precision_24b[ip+1],precision_24b[ip+2]);
    if (table!=0)
      break;
  }

  if (table!=0)
  {
    Convert_bitmap_24B_to_256_Floyd_Steinberg(Dest,Source,width,height,palette,table);
    TC_Delete(table);
    return 0;
  }
  else
    return 1;
}



