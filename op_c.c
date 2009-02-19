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

void HSLtoRGB(byte H,byte S,byte L, byte* R, byte* G, byte* B)
{
    float rf =0 ,gf = 0,bf = 0;
    float hf,lf,sf;
    float p,q;

    if(S==0)
    {
        *R=*G=*B=L;
        return;
    }

    hf = H / 255.0;
    lf = L / 255.0;
    sf = S / 255.0;

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

    *R = rf * (255);
    *G = gf * (255);
    *B = bf * (255);
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////// Méthodes de gestion des tables de conversion //
/////////////////////////////////////////////////////////////////////////////

Table_conversion * TC_New(int nbb_r,int nbb_v,int nbb_b)
{
  Table_conversion * n;
  int taille;

  n=(Table_conversion *)malloc(sizeof(Table_conversion));
  if (n!=NULL)
  {
    // On recopie les paramŠtres demand‚s
    n->nbb_r=nbb_r;
    n->nbb_v=nbb_v;
    n->nbb_b=nbb_b;

    // On calcule les autres
    n->rng_r=(1<<nbb_r);
    n->rng_v=(1<<nbb_v);
    n->rng_b=(1<<nbb_b);
    n->dec_r=nbb_v+nbb_b;
    n->dec_v=nbb_b;
    n->dec_b=0;
    n->red_r=8-nbb_r;
    n->red_v=8-nbb_v;
    n->red_b=8-nbb_b;

    // On tente d'allouer la table
    taille=(n->rng_r)*(n->rng_v)*(n->rng_b);
    n->table=(byte *)malloc(taille);
    if (n->table!=NULL)
      // C'est bon!
      memset(n->table,0,taille); // Inutile, mais plus propre
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

byte TC_Get(Table_conversion * t,int r,int v,int b)
{
  int indice;

  // On réduit le nombre de bits par couleur
  r=(r>>t->red_r);
  v=(v>>t->red_v);
  b=(b>>t->red_b);
  
  // On recherche la couleur la plus proche dans la table de conversion
  indice=(r<<t->dec_r) | (v<<t->dec_v) | (b<<t->dec_b);

  return t->table[indice];
}

void TC_Set(Table_conversion * t,int r,int v,int b,byte i)
{
  int indice;

  indice=(r<<t->dec_r) | (v<<t->dec_v) | (b<<t->dec_b);
  t->table[indice]=i;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// M‚thodes de gestion des tables d'occurence //
/////////////////////////////////////////////////////////////////////////////

void TO_Init(Table_occurence * t)
{
  int taille;

  taille=(t->rng_r)*(t->rng_v)*(t->rng_b)*sizeof(int);
  memset(t->table,0,taille); // On initialise … 0
}

Table_occurence * TO_New(int nbb_r,int nbb_v,int nbb_b)
{
  Table_occurence * n;
  int taille;

  n=(Table_occurence *)malloc(sizeof(Table_occurence));
  if (n!=0)
  {
    // On recopie les paramŠtres demand‚s
    n->nbb_r=nbb_r;
    n->nbb_v=nbb_v;
    n->nbb_b=nbb_b;

    // On calcule les autres
    n->rng_r=(1<<nbb_r);
    n->rng_v=(1<<nbb_v);
    n->rng_b=(1<<nbb_b);
    n->dec_r=nbb_v+nbb_b;
    n->dec_v=nbb_b;
    n->dec_b=0;
    n->red_r=8-nbb_r;
    n->red_v=8-nbb_v;
    n->red_b=8-nbb_b;

    // On tente d'allouer la table
    taille=(n->rng_r)*(n->rng_v)*(n->rng_b)*sizeof(int);
    n->table=(int *)malloc(taille);
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

int TO_Get(Table_occurence * t,int r,int v,int b)
{
  int indice;

  indice=(r<<t->dec_r) | (v<<t->dec_v) | (b<<t->dec_b);
  return t->table[indice];
}

void TO_Set(Table_occurence * t,int r,int v,int b,int i)
{
  int indice;

  r=(r>>t->red_r);
  v=(v>>t->red_v);
  b=(b>>t->red_b);
  indice=(r<<t->dec_r) | (v<<t->dec_v) | (b<<t->dec_b);
  t->table[indice]=i;
}

void TO_Inc(Table_occurence * t,int r,int v,int b)
{
  int indice;

  r=(r>>t->red_r);
  v=(v>>t->red_v);
  b=(b>>t->red_b);
  indice=(r<<t->dec_r) | (v<<t->dec_v) | (b<<t->dec_b);
  t->table[indice]++;
}

void TO_Compter_occurences(Table_occurence * t,Bitmap24B image,int taille)
{
  Bitmap24B ptr;
  int indice;

  for (indice=taille,ptr=image;indice>0;indice--,ptr++)
    TO_Inc(t,ptr->R,ptr->V,ptr->B);
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
  int r,v,b;

  // On cherche les mins et les maxs de chaque composante sur la couverture

  // int nbocc;

  // On prédécale tout pour éviter de faire trop de bazar en se forçant à utiliser TO_Get, plus rapide
  rmin=c->rmax <<16; rmax=c->rmin << 16;
  vmin=c->vmax << 8; vmax=c->vmin << 8;
  bmin=c->bmax; bmax=c->bmin;
  c->occurences=0;
  /*
  for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
    for (v=c->vmin<<8;v<=c->vmax<<8;v+=1<<8)
      for (b=c->bmin;b<=c->bmax;b++)
      {
        nbocc=to->table[r + v + b]; // TO_Get
        if (nbocc)
        {
          if (r<rmin) rmin=r;
          else if (r>rmax) rmax=r;
          if (v<vmin) vmin=v;
          else if (v>vmax) vmax=v;
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
      for(v=c->vmin<<8;v<=c->vmax<<8;v+=1<<8)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + v + b]) // TO_Get
            {
                rmin=r;
                goto RMAX;
            }
          }
RMAX:
  for(r=c->rmax<<16;r>=rmin;r-=1<<16)
      for(v=c->vmin<<8;v<=c->vmax<<8;v+=1<<8)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + v + b]) // TO_Get
            {
                rmax=r;
                goto VMIN;
            }
          }
VMIN:
  for(v=c->vmin<<8;v<=c->vmax<<8;v+=1<<8)
      for(r=rmin;r<=rmax;r+=1<<16)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + v + b]) // TO_Get
            {
                vmin=v;
                goto VMAX;
            }
          }
VMAX:
  for(v=c->vmax<<8;v>=vmin;v-=1<<8)
      for(r=rmin;r<=rmax;r+=1<<16)
          for(b=c->bmin;b<=c->bmax;b++)
          {
            if(to->table[r + v + b]) // TO_Get
            {
                vmax=v;
                goto BMIN;
            }
          }
BMIN:
  for(b=c->bmin;b<=c->bmax;b++)
      for(r=rmin;r<=rmax;r+=1<<16)
          for(v=vmin;v<=vmax;v+=1<<8)
          {
            if(to->table[r + v + b]) // TO_Get
            {
                bmin=b;
                goto BMAX;
            }
          }
BMAX:
  for(b=c->bmax;b>=bmin;b--)
      for(r=rmin;r<=rmax;r+=1<<16)
          for(v=vmin;v<=vmax;v+=1<<8)
          {
            if(to->table[r + v + b]) // TO_Get
            {
                bmax=b;
                goto ENDCRUSH;
            }
          }
ENDCRUSH:
  // Il faut quand même parcourir la partie utile du cluster, pour savoir combien il y a d'occurences
  for(r=rmin;r<=rmax;r+=1<<16)
      for(v=vmin;v<=vmax;v+=1<<8)
          for(b=bmin;b<=bmax;b++)
          {
            c->occurences+=to->table[r + v + b]; // TO_Get
          }

  c->rmin=rmin>>16; c->rmax=rmax>>16;
  c->vmin=vmin>>8;  c->vmax=vmax>>8;
  c->bmin=bmin;     c->bmax=bmax;

  // On regarde la composante qui a la variation la plus grande
  r=(c->rmax-c->rmin)*299;
  v=(c->vmax-c->vmin)*587;
  b=(c->bmax-c->bmin)*114;

  if (v>=r)
  {
    // V>=R
    if (v>=b)
    {
      // V>=R et V>=B
      c->plus_large=1;
    }
    else
    {
      // V>=R et V<B
      c->plus_large=2;
    }
  }
  else
  {
    // R>V
    if (r>=b)
    {
      // R>V et R>=B
      c->plus_large=0;
    }
    else
    {
      // R>V et R<B
      c->plus_large=2;
    }
  }
}

void Cluster_Split(Cluster * c,Cluster * c1,Cluster * c2,int teinte,Table_occurence * to)
{
  int limite;
  int cumul;
  int r,v,b;

  limite=(c->occurences)/2;
  cumul=0;
  if (teinte==0)
  {
    for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
    {
      for (v=c->vmin<<8;v<=c->vmax<<8;v+=1<<8)
      {
        for (b=c->bmin;b<=c->bmax;b++)
        {
          cumul+=to->table[r + v + b];
          if (cumul>=limite)
            break;
        }
        if (cumul>=limite)
          break;
      }
      if (cumul>=limite)
        break;
    }

    r>>=16;
    v>>=8;

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
  if (teinte==1)
  {

    for (v=c->vmin<<8;v<=c->vmax<<8;v+=1<<8)
    {
      for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
      {
        for (b=c->bmin;b<=c->bmax;b++)
        {
          cumul+=to->table[r + v + b];
          if (cumul>=limite)
            break;
        }
        if (cumul>=limite)
          break;
      }
      if (cumul>=limite)
        break;
    }

    r>>=16; v>>=8;

    if (v==c->vmin)
      v++;
    // V est la valeur de d‚but du 2nd cluster

    c1->Rmin=c->Rmin; c1->Rmax=c->Rmax;
    c1->rmin=c->rmin; c1->rmax=c->rmax;
    c1->Vmin=c->Vmin; c1->Vmax=v-1;
    c1->vmin=c->vmin; c1->vmax=v-1;
    c1->Bmin=c->Bmin; c1->Bmax=c->Bmax;
    c1->bmin=c->bmin; c1->bmax=c->bmax;
    c2->Rmin=c->Rmin; c2->Rmax=c->Rmax;
    c2->rmin=c->rmin; c2->rmax=c->rmax;
    c2->Vmin=v;       c2->Vmax=c->Vmax;
    c2->vmin=v;       c2->vmax=c->vmax;
    c2->Bmin=c->Bmin; c2->Bmax=c->Bmax;
    c2->bmin=c->bmin; c2->bmax=c->bmax;
  }
  else
  {

    for (b=c->bmin;b<=c->bmax;b++)
    {
      for (v=c->vmin<<8;v<=c->vmax<<8;v+=1<<8)
      {
        for (r=c->rmin<<16;r<=c->rmax<<16;r+=1<<16)
        {
          cumul+=to->table[r + v + b];
          if (cumul>=limite)
            break;
        }
        if (cumul>=limite)
          break;
      }
      if (cumul>=limite)
        break;
    }

    r>>=16; v>>=8;

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
  int r,v,b;
  int nbocc;

  byte s=0;

  cumulR=cumulV=cumulB=0;
  for (r=c->rmin;r<=c->rmax;r++)
    for (v=c->vmin;v<=c->vmax;v++)
      for (b=c->bmin;b<=c->bmax;b++)
      {
        nbocc=TO_Get(to,r,v,b);
        if (nbocc)
        {
          cumulR+=r*nbocc;
          cumulV+=v*nbocc;
          cumulB+=b*nbocc;
        }
      }
  
  c->r=(cumulR<<to->red_r)/c->occurences;
  c->v=(cumulV<<to->red_v)/c->occurences;
  c->b=(cumulB<<to->red_b)/c->occurences;
  RGBtoHSL(c->r,c->v,c->b,&c->h,&s,&c->l);
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
    n->nbmax=TO_Compter_couleurs(to);

    // On vient de compter le nombre de couleurs existantes, s'il est plus grand que 256 on limite à 256 (nombre de couleurs voulu au final)
    if (n->nbmax>nbmax)
    {
      n->nbmax=nbmax;
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
  int indice;

  // On cherche un cluster que l'on peut couper en deux, donc avec au moins deux valeurs
  // différentes sur l'une des composantes
  for (indice=0;indice<cs->nb;indice++)
    if ( (cs->clusters[indice].rmin<cs->clusters[indice].rmax) ||
         (cs->clusters[indice].vmin<cs->clusters[indice].vmax) ||
         (cs->clusters[indice].bmin<cs->clusters[indice].bmax) )
      break;

  // On le recopie dans c
  *c=cs->clusters[indice];

  // On décrémente le nombre et on décale tous les clusters suivants
  // Sachant qu'on va réinsérer juste après, il me semble que ça serait une bonne idée de gérer les clusters 
  // comme une liste chainée... on n'a aucun accès direct dedans, que des parcours ...
  cs->nb--;
  memcpy((cs->clusters+indice),(cs->clusters+indice+1),(cs->nb-indice)*sizeof(Cluster));
}

void CS_Set(ClusterSet * cs,Cluster * c)
{
  int indice;
  // int decalage;

  // Le tableau des clusters est trié par nombre d'occurences. Donc on cherche la position du premier cluster 
  // qui est plus grand que le notre
  for (indice=0;indice<cs->nb;indice++)
    if (cs->clusters[indice].occurences<c->occurences)
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

  if (indice<cs->nb)
  {
    // On distingue ici une insertion plutot qu'un placement en fin de liste.
    // On doit donc décaler les ensembles suivants vers la fin pour se faire
    // une place dans la liste.

    //for (decalage=cs->nb;decalage>indice;decalage--)
    //  memcpy((cs->clusters+decalage),(cs->clusters+decalage-1),sizeof(Cluster));
    memmove(cs->clusters+indice+1,cs->clusters+indice,(cs->nb-indice)*sizeof(Cluster));
  }

  cs->clusters[indice]=*c;
  cs->nb++;
}

// Détermination de la meilleure palette en utilisant l'algo Median Cut :
// 1) On considère l'espace (R,V,B) comme 1 boîte
// 2) On cherche les extrêmes de la boîte en (R,V,B)
// 3) On trie les pixels de l'image selon l'axe le plus long parmi (R,V,B)
// 4) On coupe la boîte en deux au milieu, et on compacte pour que chaque bord corresponde bien à un pixel extreme
// 5) On recommence à couper selon le plus grand axe toutes boîtes confondues
// 6) On s'arrête quand on a le nombre de couleurs voulu
void CS_Generer(ClusterSet * cs,Table_occurence * to)
{
  Cluster Courant;
  Cluster Nouveau1;
  Cluster Nouveau2;

  // Tant qu'on a moins de 256 clusters
  while (cs->nb<cs->nbmax)
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
  int indice;
  Cluster * c;

  for (indice=0,c=cs->clusters;indice<cs->nb;indice++,c++)
    Cluster_Calculer_teinte(c,to);
}

void CS_Trier_par_chrominance(ClusterSet * cs)
{
  int byte_used[256];
  int decalages[256];
  int index;
  Cluster * nc;

  nc=(Cluster *)malloc(cs->nbmax*sizeof(Cluster));

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

  nc=(Cluster *)malloc(cs->nbmax*sizeof(Cluster));

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

void CS_Generer_TC_et_Palette(ClusterSet * cs,Table_conversion * tc,struct Composantes * palette)
{
  int indice;
  int r,v,b;

  for (indice=0;indice<cs->nb;indice++)
  {
    palette[indice].R=cs->clusters[indice].r;
    palette[indice].V=cs->clusters[indice].v;
    palette[indice].B=cs->clusters[indice].b;

    for (r=cs->clusters[indice].Rmin;r<=cs->clusters[indice].Rmax;r++)
      for (v=cs->clusters[indice].Vmin;v<=cs->clusters[indice].Vmax;v++)
        for (b=cs->clusters[indice].Bmin;b<=cs->clusters[indice].Bmax;b++)
          TC_Set(tc,r,v,b,indice);
  }
}

/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Méthodes de gestion des dégradés //
/////////////////////////////////////////////////////////////////////////////

void DS_Init(DegradeSet * ds,ClusterSet * cs)
{
    ds->degrades[0].nbcouleurs=1;
    ds->degrades[0].min=cs->clusters[0].h;
    ds->degrades[0].max=cs->clusters[0].h;
    ds->degrades[0].hue=cs->clusters[0].h;
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
        n->nbmax=cs->nbmax;

        // On tente d'allouer la table
        n->degrades=(Degrade *)malloc((n->nbmax)*sizeof(Degrade));
        if (n->degrades!=0)
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
    free(ds->degrades);
    free(ds);
}

void DS_Generer(DegradeSet * ds,ClusterSet * cs)
{
    int ic,id; // Les indices de parcours des ensembles
    int mdegr; // Meilleur d‚grad‚
    int mdiff; // Meilleure diff‚rence de chrominance
    int diff;  // Diff‚rence de chrominance courante

    // Pour chacun des clusters … traiter
    for (ic=1;ic<cs->nb;ic++)
    {
        // On recherche le d‚grad‚ le plus proche de la chrominance du cluster
        mdegr=-1;
        mdiff=99999999;
        for (id=0;id<ds->nb;id++)
        {
            diff=abs(cs->clusters[ic].h - ds->degrades[id].hue);
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
      if (cs->clusters[ic].h < ds->degrades[mdegr].min)
        ds->degrades[mdegr].min=cs->clusters[ic].h;
      if (cs->clusters[ic].h > ds->degrades[mdegr].max)
        ds->degrades[mdegr].max=cs->clusters[ic].h;
      ds->degrades[mdegr].hue=((ds->degrades[mdegr].hue*
                                ds->degrades[mdegr].nbcouleurs)
                               +cs->clusters[ic].h)
                              /(ds->degrades[mdegr].nbcouleurs+1);
      ds->degrades[mdegr].nbcouleurs++;
    }
    else
    {
      // On cr‚e un nouveau d‚grad‚
      mdegr=ds->nb;
      ds->degrades[mdegr].nbcouleurs=1;
      ds->degrades[mdegr].min=cs->clusters[ic].h;
      ds->degrades[mdegr].max=cs->clusters[ic].h;
      ds->degrades[mdegr].hue=cs->clusters[ic].h;
      ds->nb++;
    }
    cs->clusters[ic].h=mdegr;
  }

  // On redistribue les valeurs dans les clusters
  for (ic=0;ic<cs->nb;ic++)
    cs->clusters[ic].h=ds->degrades[cs->clusters[ic].h].hue;
}




Table_conversion * Optimiser_palette(Bitmap24B image,int taille,struct Composantes * palette,int r,int v,int b)
{
  Table_occurence  * to;
  Table_conversion * tc;
  ClusterSet       * cs;
  DegradeSet       * ds;

  // Création des éléments nécessaires au calcul de palette optimisée:
  to=0; tc=0; cs=0; ds=0;

  to=TO_New(r,v,b);
  if (to!=0)
  {
    tc=TC_New(r,v,b);
    if (tc!=0)
    {

      // Première étape : on compte les pixels de chaque couleur pour pouvoir trier là dessus
      TO_Compter_occurences(to,image,taille);

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

int Valeur_modifiee(int valeur,int modif)
{
  valeur+=modif;
  if (valeur<0)
  {
    valeur=0;
  }
  else if (valeur>255)
  {
    valeur=255;
  }
  return valeur;
}

void Convert_bitmap_24B_to_256_Floyd_Steinberg(Bitmap256 Dest,Bitmap24B Source,int largeur,int hauteur,struct Composantes * palette,Table_conversion * tc)
// Cette fonction dégrade au fur et à mesure le bitmap source, donc soit on ne
// s'en ressert pas, soit on passe à la fonction une copie de travail du
// bitmap original.
{
  Bitmap24B Courant;
  Bitmap24B C_plus1;
  Bitmap24B S_moins1;
  Bitmap24B Suivant;
  Bitmap24B S_plus1;
  Bitmap256 D;
  int Pos_X,Pos_Y;
  int Rouge,Vert,Bleu;
  float ERouge,EVert,EBleu;

  // On initialise les variables de parcours:
  Courant =Source;      // Le pixel dont on s'occupe
  Suivant =Courant+largeur; // Le pixel en dessous
  C_plus1 =Courant+1;   // Le pixel à droite
  S_moins1=Suivant-1;   // Le pixel en bas à gauche
  S_plus1 =Suivant+1;   // Le pixel en bas à droite
  D       =Dest;

  // On parcours chaque pixel:
  for (Pos_Y=0;Pos_Y<hauteur;Pos_Y++)
  {
    for (Pos_X=0;Pos_X<largeur;Pos_X++)
    {
      // On prends la meilleure couleur de la palette qui traduit la couleur
      // 24 bits de la source:
      Rouge=Courant->R;
      Vert =Courant->V;
      Bleu =Courant->B;
      // Cherche la couleur correspondant dans la palette et la range dans l'image de destination
      *D=TC_Get(tc,Rouge,Vert,Bleu);

      // Puis on calcule pour chaque composante l'erreur dûe à l'approximation
      Rouge-=palette[*D].R;
      Vert -=palette[*D].V;
      Bleu -=palette[*D].B;

      // Et dans chaque pixel voisin on propage l'erreur
      // A droite:
        ERouge=(Rouge*7)/16.0;
        EVert =(Vert *7)/16.0;
        EBleu =(Bleu *7)/16.0;
        if (Pos_X+1<largeur)
        {
          // Valeur_modifiee fait la somme des 2 params en bornant sur [0,255]
          C_plus1->R=Valeur_modifiee(C_plus1->R,ERouge);
          C_plus1->V=Valeur_modifiee(C_plus1->V,EVert );
          C_plus1->B=Valeur_modifiee(C_plus1->B,EBleu );
        }
      // En bas à gauche:
      if (Pos_Y+1<hauteur)
      {
        ERouge=(Rouge*3)/16.0;
        EVert =(Vert *3)/16.0;
        EBleu =(Bleu *3)/16.0;
        if (Pos_X>0)
        {
          S_moins1->R=Valeur_modifiee(S_moins1->R,ERouge);
          S_moins1->V=Valeur_modifiee(S_moins1->V,EVert );
          S_moins1->B=Valeur_modifiee(S_moins1->B,EBleu );
        }
      // En bas:
        ERouge=(Rouge*5/16.0);
        EVert =(Vert*5 /16.0);
        EBleu =(Bleu*5 /16.0);
        Suivant->R=Valeur_modifiee(Suivant->R,ERouge);
        Suivant->V=Valeur_modifiee(Suivant->V,EVert );
        Suivant->B=Valeur_modifiee(Suivant->B,EBleu );
      // En bas à droite:
        if (Pos_X+1<largeur)
        {
        ERouge=(Rouge/16.0);
        EVert =(Vert /16.0);
        EBleu =(Bleu /16.0);
          S_plus1->R=Valeur_modifiee(S_plus1->R,ERouge);
          S_plus1->V=Valeur_modifiee(S_plus1->V,EVert );
          S_plus1->B=Valeur_modifiee(S_plus1->B,EBleu );
        }
      }

      // On passe au pixel suivant :
      Courant++;
      C_plus1++;
      S_moins1++;
      Suivant++;
      S_plus1++;
      D++;
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

int Convert_bitmap_24B_to_256(Bitmap256 Dest,Bitmap24B Source,int largeur,int hauteur,struct Composantes * palette)
{
  Table_conversion * table; // table de conversion
  int                ip;    // Indice de précision pour la conversion

  // On essaye d'obtenir une table de conversion qui loge en mémoire, avec la
  // meilleure précision possible
  for (ip=0;ip<(10*3);ip+=3)
  {
    table=Optimiser_palette(Source,largeur*hauteur,palette,precision_24b[ip+0],
                            precision_24b[ip+1],precision_24b[ip+2]);
    if (table!=0)
      break;
  }

  if (table!=0)
  {
    Convert_bitmap_24B_to_256_Floyd_Steinberg(Dest,Source,largeur,hauteur,palette,table);
    TC_Delete(table);
    return 0;
  }
  else
    return 1;
}



