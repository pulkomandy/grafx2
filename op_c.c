#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "op_c.h"
#include "op_asm.h"
#include "erreurs.h"

#undef OPTIMISATIONS_ASSEMBLEUR



void rgb2hl(int r,int g,int b,byte * hr,byte * lr)
{
  double rd,gd,bd,h,s,v,l,max,min,del,rc,gc,bc;

  // convert RGB to HSV
  rd = r / 255.0;            // rd,gd,bd range 0-1 instead of 0-255
  gd = g / 255.0;
  bd = b / 255.0;

  // compute L
  l=(rd*0.30)+(gd*0.59)+(bd*0.11);

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
  del = max - min;
  v = max;
  if (max != 0.0)
    s = (del) / max;
  else
    s = 0.0;

  h = -1;
  if (s != 0.0)
  {
    rc = (max - rd) / del;
    gc = (max - gd) / del;
    bc = (max - bd) / del;

    if      (rd==max) h = bc - gc;
    else if (gd==max) h = 2 + rc - bc;
    else if (bd==max) h = 4 + gc - rc;

    h = h * 60;
    if (h<0) h += 360;
  }

  *hr = (h*255.0)/360;
  *lr = (l*255.0);
}



/////////////////////////////////////////////////////////////////////////////
///////////////////////////// M‚thodes de gestion des tables de conversion //
/////////////////////////////////////////////////////////////////////////////

Table_conversion * TC_New(int nbb_r,int nbb_v,int nbb_b)
{
  Table_conversion * n;
  int taille;

  n=(Table_conversion *)malloc(sizeof(Table_conversion));
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
    taille=(n->rng_r)*(n->rng_v)*(n->rng_b);
    n->table=(byte *)malloc(taille);
    if (n->table!=0)
      // C'est bon!
      memset(n->table,0,taille); // Inutile, mais plus propre
    else
    {
      // Table impossible … allouer
      free(n);
      n=0;
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

  if(r!=0 && v != 0 && b != 0)
  	printf("%d %d %d %d\n",indice,r,v,b);
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

#ifdef OPTIMISATIONS_ASSEMBLEUR

void TO_Compter_occurences(Table_occurence * t,Bitmap24B image,int taille)
{
  OPASM_Compter_occurences(t->table,image,taille,
                           t->red_r,t->red_v,t->red_b,
                           t->nbb_v,t->nbb_b);
}

#else

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

#endif

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

#ifdef OPTIMISATIONS_ASSEMBLEUR

  rmin=(c->rmin << to->dec_r); rmax=(c->rmax << to->dec_r);
  vmin=(c->vmin << to->dec_v); vmax=(c->vmax << to->dec_v);
  bmin=(c->bmin << to->dec_b); bmax=(c->bmax << to->dec_b);
  OPASM_Analyser_cluster(to->table,&rmin,&vmin,&bmin,&rmax,&vmax,&bmax,
                         to->dec_r,to->dec_v,to->dec_b,
                         (1 << to->dec_r),(1 << to->dec_v),(1 << to->dec_b),
                         &c->occurences);

#else

  int nbocc;

  rmin=c->rmax; rmax=c->rmin;
  vmin=c->vmax; vmax=c->vmin;
  bmin=c->bmax; bmax=c->bmin;
  c->occurences=0;
  for (r=c->rmin;r<=c->rmax;r++)
    for (v=c->vmin;v<=c->vmax;v++)
      for (b=c->bmin;b<=c->bmax;b++)
      {
        nbocc=TO_Get(to,r,v,b);
        if (nbocc)
        {
          if (r<rmin) rmin=r;
          if (r>rmax) rmax=r;
          if (v<vmin) vmin=v;
          if (v>vmax) vmax=v;
          if (b<bmin) bmin=b;
          if (b>bmax) bmax=b;
          c->occurences+=nbocc;
        }
      }

#endif

  c->rmin=rmin; c->rmax=rmax;
  c->vmin=vmin; c->vmax=vmax;
  c->bmin=bmin; c->bmax=bmax;

  // On regarde la composante qui a la variation la plus grande
  r=((c->rmax-c->rmin)<<to->red_r)*299;
  v=((c->vmax-c->vmin)<<to->red_v)*587;
  b=((c->bmax-c->bmin)<<to->red_b)*114;

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

#ifdef OPTIMISATIONS_ASSEMBLEUR

    OPASM_Split_cluster_Rouge(to->table,
                              (c->rmin << to->dec_r),(c->vmin << to->dec_v),
                              (c->bmin << to->dec_b),(c->rmax << to->dec_r),
                              (c->vmax << to->dec_v),(c->bmax << to->dec_b),
                              (1 << to->dec_r),(1 << to->dec_v),
                              (1 << to->dec_b),limite,to->dec_r,&r);

#else

    for (r=c->rmin;r<=c->rmax;r++)
    {
      for (v=c->vmin;v<=c->vmax;v++)
      {
        for (b=c->bmin;b<=c->bmax;b++)
        {
          cumul+=TO_Get(to,r,v,b);
          if (cumul>=limite)
            break;
        }
        if (cumul>=limite)
          break;
      }
      if (cumul>=limite)
        break;
    }

#endif

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

#ifdef OPTIMISATIONS_ASSEMBLEUR

    OPASM_Split_cluster_Vert(to->table,
                             (c->rmin << to->dec_r),(c->vmin << to->dec_v),
                             (c->bmin << to->dec_b),(c->rmax << to->dec_r),
                             (c->vmax << to->dec_v),(c->bmax << to->dec_b),
                             (1 << to->dec_r),(1 << to->dec_v),
                             (1 << to->dec_b),limite,to->dec_v,&v);

#else

    for (v=c->vmin;v<=c->vmax;v++)
    {
      for (r=c->rmin;r<=c->rmax;r++)
      {
        for (b=c->bmin;b<=c->bmax;b++)
        {
          cumul+=TO_Get(to,r,v,b);
          if (cumul>=limite)
            break;
        }
        if (cumul>=limite)
          break;
      }
      if (cumul>=limite)
        break;
    }

#endif

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

#ifdef OPTIMISATIONS_ASSEMBLEUR

    OPASM_Split_cluster_Bleu(to->table,
                             (c->rmin << to->dec_r),(c->vmin << to->dec_v),
                             (c->bmin << to->dec_b),(c->rmax << to->dec_r),
                             (c->vmax << to->dec_v),(c->bmax << to->dec_b),
                             (1 << to->dec_r),(1 << to->dec_v),
                             (1 << to->dec_b),limite,to->dec_b,&b);

#else

    for (b=c->bmin;b<=c->bmax;b++)
    {
      for (v=c->vmin;v<=c->vmax;v++)
      {
        for (r=c->rmin;r<=c->rmax;r++)
        {
          cumul+=TO_Get(to,r,v,b);
          if (cumul>=limite)
            break;
        }
        if (cumul>=limite)
          break;
      }
      if (cumul>=limite)
        break;
    }

#endif

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
  rgb2hl(c->r,c->v,c->b,&c->h,&c->l);
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
    if (n->nbmax>nbmax)
      n->nbmax=nbmax;

    // On tente d'allouer la table
    n->clusters=(Cluster *)malloc(nbmax*sizeof(Cluster));
    if (n->clusters!=0)
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

  for (indice=0;indice<cs->nb;indice++)
    if ( (cs->clusters[indice].rmin<cs->clusters[indice].rmax) ||
         (cs->clusters[indice].vmin<cs->clusters[indice].vmax) ||
         (cs->clusters[indice].bmin<cs->clusters[indice].bmax) )
      break;

  *c=cs->clusters[indice];
  cs->nb--;
  memcpy((cs->clusters+indice),(cs->clusters+indice+1),(cs->nb-indice)*sizeof(Cluster));
}

void CS_Set(ClusterSet * cs,Cluster * c)
{
  int indice,decalage;

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
    // On doit donc d‚caler les ensembles suivants vers la fin pour se faire
    // une place dans la liste.

    for (decalage=cs->nb;decalage>indice;decalage--)
      memcpy((cs->clusters+decalage),(cs->clusters+decalage-1),sizeof(Cluster));
  }

  cs->clusters[indice]=*c;
  cs->nb++;
}

void CS_Generer(ClusterSet * cs,Table_occurence * to)
{
  Cluster Courant;
  Cluster Nouveau1;
  Cluster Nouveau2;

  while (cs->nb<cs->nbmax)
  {
    CS_Get(cs,&Courant);
    Cluster_Split(&Courant,&Nouveau1,&Nouveau2,Courant.plus_large,to);
    Cluster_Analyser(&Nouveau1,to);
    Cluster_Analyser(&Nouveau2,to);
    CS_Set(cs,&Nouveau1);
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

void CS_Generer_TC_et_Palette(ClusterSet * cs,Table_occurence * to,Table_conversion * tc,struct Composantes * palette)
{
  int indice;
  int r,v,b;

  for (indice=0;indice<cs->nb;indice++)
  {
    palette[indice].R=cs->clusters[indice].r;
    palette[indice].V=cs->clusters[indice].v;
    palette[indice].B=cs->clusters[indice].b;

    for (r=cs->clusters[indice].Rmin;r</*=*/cs->clusters[indice].Rmax;r++)
      for (v=cs->clusters[indice].Vmin;v</*=*/cs->clusters[indice].Vmax;v++)
        for (b=cs->clusters[indice].Bmin;b</*=*/cs->clusters[indice].Bmax;b++)
          TC_Set(tc,r,v,b,indice);
  }
}



/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// M‚thodes de gestion des d‚grad‚s //
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
  if (n!=0)
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

  DEBUG("START OPTIMIZING",1);

  to=TO_New(r,v,b);
  if (to!=0)
  {
    tc=TC_New(r,v,b);
    if (tc!=0)
    {
      TO_Compter_occurences(to,image,taille);

      cs=CS_New(256,to);
      if (cs!=0)
      {
        // C'est bon, on a pu tout allouer

        CS_Generer(cs,to);
        CS_Calculer_teintes(cs,to);

        ds=DS_New(cs);
        if (ds!=0)
        {
          DS_Generer(ds,cs);
          DS_Delete(ds);
        }

        CS_Trier_par_luminance(cs);
        CS_Trier_par_chrominance(cs);
        CS_Generer_TC_et_Palette(cs,to,tc,palette);

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






#ifdef OPTIMISATIONS_ASSEMBLEUR

void Convert_bitmap_24B_to_256_Floyd_Steinberg(Bitmap256 dest,Bitmap24B source,int largeur,int hauteur,struct Composantes * palette,Table_conversion * tc)
// Cette fonction dégrade au fur et à mesure le bitmap source, donc soit on ne
// s'en ressert pas, soit on passe à la fonction une copie de travail du
// bitmap original.
{
  Bitmap24B courant;
  Bitmap256 d;
  int y;


  // On initialise les variables de parcours:
  courant=source;
  d      =dest;

  if ((largeur>0) && (hauteur>0))
  { 
    if (hauteur>1)
    {
      // Traitement de la 1ère ligne à l'avant-dernière

      if (largeur>1)
      {
        // Il y a plusieurs colonnes
        for (y=0;y<hauteur-1;y++)
        {
          // Premier pixel de la ligne
          OPASM_DitherFS_623(d,courant,largeur-2,palette,tc->table,
                             tc->red_r,tc->red_v,tc->red_b,
                             tc->nbb_v,tc->nbb_b);
          courant++;
          d++;
          // Pixels interm‚diaires de la ligne
          if (largeur>2)
          {
            OPASM_DitherFS_6123(d,courant,largeur-2,palette,tc->table,
                                tc->red_r,tc->red_v,tc->red_b,
                                tc->nbb_v,tc->nbb_b);
            courant+=largeur-2;
            d+=largeur-2;
          }
          // Dernier pixel de la ligne
          OPASM_DitherFS_12(d,courant,largeur-2,palette,tc->table,
                            tc->red_r,tc->red_v,tc->red_b,
                            tc->nbb_v,tc->nbb_b);
          courant++;
          d++;
        }
      }
      else
      {
        OPASM_DitherFS_2(d,courant,hauteur-1,palette,tc->table,
                         tc->red_r,tc->red_v,tc->red_b,
                         tc->nbb_v,tc->nbb_b);
        courant+=hauteur-1;
        d+=hauteur-1;
      }
    }

    // Traitement de la derniŠre ligne

    if (largeur>1)
    {
      // Il y a plusieurs colonnes
      OPASM_DitherFS_6(d,courant,largeur-1,palette,tc->table,
                       tc->red_r,tc->red_v,tc->red_b,
                       tc->nbb_v,tc->nbb_b);
      courant+=largeur-1;
      d+=largeur-1;
    }
    // Le dernier pixel
    OPASM_DitherFS(d,courant,tc->table,
                   tc->red_r,tc->red_v,tc->red_b,
                   tc->nbb_v,tc->nbb_b);
  }
}

#else

int Valeur_modifiee(int valeur,int modif)
{
  valeur+=modif;
  if (valeur<0)
    valeur=0;
  else if (valeur>255)
    valeur=255;
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
  int DRouge,DVert,DBleu;
  int ERouge,EVert,EBleu;

  // On initialise les variables de parcours:
  Courant =Source;	// Le pixel dont on s'occupe
  Suivant =Courant+largeur; // Le pixel en dessous
  C_plus1 =Courant+1;	// Le pixel à droite
  S_moins1=Suivant-1;	// Le pixel en bas à gauche
  S_plus1 =Suivant+1;	// Le pixel en bas à droite
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
      *D=TC_Get(tc,Rouge,Vert,Bleu);
/*
      // Puis on calcule pour chaque composante l'erreur dûe à l'approximation
      Rouge=Rouge-palette[*D].R;
      Vert =Vert -palette[*D].V;
      Bleu =Bleu -palette[*D].B;

      // On initialise la quantité d'erreur diffusée
      DRouge=Rouge;
      DVert =Vert;
      DBleu =Bleu;

      // Et dans chaque pixel voisin on propage l'erreur
      // A droite:
        ERouge=(Rouge*7)/16;
        EVert =(Vert *7)/16;
        EBleu =(Bleu *7)/16;
        if (Pos_X+1<largeur)
        {
          C_plus1->R=Valeur_modifiee(C_plus1->R,ERouge);
          C_plus1->V=Valeur_modifiee(C_plus1->V,EVert );
          C_plus1->B=Valeur_modifiee(C_plus1->B,EBleu );
        }
        DRouge-=ERouge;
        DVert -=EVert;
        DBleu -=EBleu;
      // En bas à gauche:
      if (Pos_Y+1<hauteur)
      {
        ERouge=(Rouge*3)/16;
        EVert =(Vert *3)/16;
        EBleu =(Bleu *3)/16;
        if (Pos_X>0)
        {
          S_moins1->R=Valeur_modifiee(S_moins1->R,ERouge);
          S_moins1->V=Valeur_modifiee(S_moins1->V,EVert );
          S_moins1->B=Valeur_modifiee(S_moins1->B,EBleu );
        }
        DRouge-=ERouge;
        DVert -=EVert;
        DBleu -=EBleu;
      // En bas:
        ERouge=(Rouge*4)/16;
        EVert =(Vert *4)/16;
        EBleu =(Bleu *4)/16;
        Suivant->R=Valeur_modifiee(Suivant->R,ERouge);
        Suivant->V=Valeur_modifiee(Suivant->V,EVert );
        Suivant->B=Valeur_modifiee(Suivant->B,EBleu );
        DRouge-=ERouge;
        DVert -=EVert;
        DBleu -=EBleu;
      // En bas à droite:
        if (Pos_X+1<largeur)
        {
          S_plus1->R=Valeur_modifiee(S_plus1->R,DRouge);
          S_plus1->V=Valeur_modifiee(S_plus1->V,DVert );
          S_plus1->B=Valeur_modifiee(S_plus1->B,DBleu );
        }
      }
*/
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

#endif



static const byte precision_24b[]=
{6,6,5,
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



void Load_RAW_24B(int Largeur,int Hauteur,Bitmap24B Source)
{
  int Fichier;

  Fichier=open("TEST.RAW",O_RDONLY);
  if (read(Fichier,Source,Largeur*Hauteur*sizeof(struct Composantes))!=Largeur*Hauteur*sizeof(struct Composantes))
    exit(3);
  close(Fichier);
}

void Load_TGA(char * nom,Bitmap24B * dest,int * larg,int * haut)
{
  int fichier;
  struct
  {
    byte Id_field_size;        // Taille des donn‚es sp‚cifiques plac‚es aprŠs le header
    byte Color_map_type;       // Pr‚sence d'une palette
    byte Image_type_code;      // Type d'image
    word Color_map_origin;     // Indice de d‚part de la palette
    word Color_map_length;     // Taille de la palette
    byte Color_map_entry_size; // Palette sur 16, 24 ou 32 bits
    word X_origin;             // Coordonn‚es de d‚part
    word Y_origin;
    word Width;                // Largeur de l'image
    word Height;               // Hauteur de l'image
    byte Pixel_size;           // Pixels sur 16, 24 ou 32 bits
    byte Descriptor;           // ParamŠtres divers
  } TGA_Header;
  int x,y,py,skip,t;
  byte * buffer;

  fichier=open(nom,O_RDONLY);
  read(fichier,&TGA_Header,sizeof(TGA_Header));
  if (TGA_Header.Image_type_code==2)
  {
    *larg=TGA_Header.Width;
    *haut=TGA_Header.Height;
    *dest=(Bitmap24B)malloc((*larg)*(*haut)*3);

    // On saute l'ID field
    lseek(fichier,TGA_Header.Id_field_size,SEEK_CUR);

    // On saute la palette
    if (TGA_Header.Color_map_type==0)
      skip=0;
    else
    {
      skip=TGA_Header.Color_map_length;
      if (TGA_Header.Color_map_entry_size==16)
        skip*=2;
      else
      if (TGA_Header.Color_map_entry_size==24)
        skip*=3;
      else
      if (TGA_Header.Color_map_entry_size==32)
        skip*=4;
    }
    lseek(fichier,skip,SEEK_CUR);

    // Lecture des pixels
    skip=(*larg);
    if (TGA_Header.Pixel_size==16)
      skip*=2;
    else
    if (TGA_Header.Pixel_size==24)
      skip*=3;
    else
    if (TGA_Header.Pixel_size==32)
      skip*=4;

    buffer=(byte *)malloc(skip);
    for (y=0;y<(*haut);y++)
    {
      read(fichier,buffer,skip);

      // Inversion du rouge et du bleu
      for (x=0;x<(*larg);x++)
      {
        t=buffer[(x*3)+0];
        buffer[(x*3)+0]=buffer[(x*3)+2];
        buffer[(x*3)+2]=t;
      }

      // Prise en compte du sens d'‚criture verticale
      if (TGA_Header.Descriptor & 0x20)
        py=y;
      else
        py=(*haut)-y-1;

      // Prise en compte de l'interleave verticale
      if (TGA_Header.Descriptor & 0xC0)
        py=((py % (*haut))*2)+(py/(*haut));

      memcpy((*dest)+(py*(*larg)),buffer,skip);
    }
    free(buffer);
  }
  close(fichier);
}
