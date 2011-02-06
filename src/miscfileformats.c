/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2011 Pawel Góralski
    Copyright 2009 Petter Lindquist
    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
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

///@file miscfileformats.c
/// Formats that aren't fully saving, either because of palette restrictions or other things

#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "errors.h"
#include "global.h"
#include "io.h"
#include "libraw2crtc.h"
#include "limits.h"
#include "loadsave.h"
#include "misc.h"
#include "sdlscreen.h"
#include "struct.h"
#include "windows.h"

//////////////////////////////////// PAL ////////////////////////////////////
//

// -- Tester si un fichier est au format PAL --------------------------------
void Test_PAL(T_IO_Context * context)
{
  FILE *file; // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  long file_size; // Taille du fichier

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error = 1;

  // Ouverture du fichier
  if ((file = fopen(filename, "rb")))
  {
    // Lecture de la taille du fichier
    file_size = File_length_file(file);
    // Le fichier ne peut être au format PAL que si sa taille vaut 768 octets
    if (file_size == sizeof(T_Palette))
      File_error = 0;
    else {
      // Sinon c'est peut être un fichier palette ASCII "Jasc"
      fread(filename, 1, 8, file);
      if (strncmp(filename,"JASC-PAL",8) == 0)
      {
        File_error = 0;
      }
    }
    fclose(file);
  }
}


// -- Lire un fichier au format PAL -----------------------------------------
void Load_PAL(T_IO_Context * context)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  //long  file_size;   // Taille du fichier


  Get_full_filename(filename, context->File_name, context->File_directory);
  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    long file_size = File_length_file(file);
    // Le fichier ne peut être au format PAL que si sa taille vaut 768 octets
    if (file_size == sizeof(T_Palette))
    {
      T_Palette palette_64;
      // Pre_load(context, ?); // Pas possible... pas d'image...

      // Lecture du fichier dans context->Palette
      if (Read_bytes(file, palette_64, sizeof(T_Palette)))
      {
        Palette_64_to_256(palette_64);
        memcpy(context->Palette, palette_64, sizeof(T_Palette));
        Palette_loaded(context);
      }
      else
        File_error = 2;
    } else {
      fread(filename, 1, 8, file);
      if (strncmp(filename,"JASC-PAL",8) == 0)
      {
        int i, n, r, g, b;
        fscanf(file, "%d",&n);
        if(n != 100) 
        {
          File_error = 2;
          fclose(file);
          return;
        }
        // Read color count
        fscanf(file, "%d",&n);
        for (i = 0; i < n; i++)
        {
          fscanf(file, "%d %d %d",&r, &g, &b);
          context->Palette[i].R = r;
          context->Palette[i].G = g;
          context->Palette[i].B = b;
        }
        Palette_loaded(context);
      } else File_error = 2;
    
    }
    
    // Fermeture du fichier
    fclose(file);
  }
  else
    // Si on n'a pas réussi à ouvrir le fichier, alors il y a eu une erreur
    File_error=1;
}


// -- Sauver un fichier au format PAL ---------------------------------------
void Save_PAL(T_IO_Context * context)
{
  FILE *file;
  char filename[MAX_PATH_CHARACTERS]; ///< full filename

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  // Open output file
  if ((file=fopen(filename,"w")))
  {
    int i;
    if (fputs("JASC-PAL\n0100\n256\n", file)==EOF)
      File_error=1;
    for (i = 0; i < 256 && File_error==0; i++)
    {
      if (fprintf(file,"%d %d %d\n",context->Palette[i].R, context->Palette[i].G, context->Palette[i].B) <= 0)
        File_error=1;
    }
    
    fclose(file);
    
    if (File_error)
      remove(filename);
  }
  else
  {
    // unable to open output file, nothing saved.
    File_error=1;
  }
}


//////////////////////////////////// PKM ////////////////////////////////////
typedef struct
{
  char Ident[3];    // String "PKM" }
  byte Method;      // Compression method
                    //   0 = per-line compression (c)KM
                    //   others = unknown at the moment
  byte Recog1;      // Recognition byte 1
  byte Recog2;      // Recognition byte 2
  word Width;       // Image width
  word Height;      // Image height
  T_Palette Palette;// RGB Palette 256*3, on a 1-64 scale for each component
  word Jump;        // Size of the jump between header and image:
                    //   Used to insert a comment
} T_PKM_Header;

// -- Tester si un fichier est au format PKM --------------------------------
void Test_PKM(T_IO_Context * context)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  T_PKM_Header header;


  Get_full_filename(filename, context->File_name, context->File_directory);
  
  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Lecture du header du fichier
    if (Read_bytes(file,&header.Ident,3) &&
        Read_byte(file,&header.Method) &&
        Read_byte(file,&header.Recog1) &&
        Read_byte(file,&header.Recog2) &&
        Read_word_le(file,&header.Width) &&
        Read_word_le(file,&header.Height) &&
        Read_bytes(file,&header.Palette,sizeof(T_Palette)) &&
        Read_word_le(file,&header.Jump))
    {
      // On regarde s'il y a la signature PKM suivie de la méthode 0.
      // La constante "PKM" étant un chaîne, elle se termine toujours par 0.
      // Donc pas la peine de s'emm...er à regarder si la méthode est à 0.
      if ( (!memcmp(&header,"PKM",4)) && header.Width && header.Height)
        File_error=0;
    }
    fclose(file);
  }
}


// -- Lire un fichier au format PKM -----------------------------------------
void Load_PKM(T_IO_Context * context)
{
  FILE *file;             // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  T_PKM_Header header;
  byte  color;
  byte  temp_byte;
  word  len;
  word  index;
  dword Compteur_de_pixels;
  dword Compteur_de_donnees_packees;
  dword image_size;
  dword Taille_pack;
  long  file_size;

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;
  
  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);

    if (Read_bytes(file,&header.Ident,3) &&
        Read_byte(file,&header.Method) &&
        Read_byte(file,&header.Recog1) &&
        Read_byte(file,&header.Recog2) &&
        Read_word_le(file,&header.Width) &&
        Read_word_le(file,&header.Height) &&
        Read_bytes(file,&header.Palette,sizeof(T_Palette)) &&
        Read_word_le(file,&header.Jump))
    {
      context->Comment[0]='\0'; // On efface le commentaire
      if (header.Jump)
      {
        index=0;
        while ( (index<header.Jump) && (!File_error) )
        {
          if (Read_byte(file,&temp_byte))
          {
            index+=2; // On rajoute le "Field-id" et "le Field-size" pas encore lu
            switch (temp_byte)
            {
              case 0 : // Commentaire
                if (Read_byte(file,&temp_byte))
                {
                  if (temp_byte>COMMENT_SIZE)
                  {
                    color=temp_byte;              // On se sert de color comme
                    temp_byte=COMMENT_SIZE;   // variable temporaire
                    color-=COMMENT_SIZE;
                  }
                  else
                    color=0;

                  if (Read_bytes(file,context->Comment,temp_byte))
                  {
                    index+=temp_byte;
                    context->Comment[temp_byte]='\0';
                    if (color)
                      if (fseek(file,color,SEEK_CUR))
                        File_error=2;
                  }
                  else
                    File_error=2;
                }
                else
                  File_error=2;
                break;

              case 1 : // Dimensions de l'écran d'origine
                if (Read_byte(file,&temp_byte))
                {
                  if (temp_byte==4)
                  {
                    index+=4;
                    if ( ! Read_word_le(file,(word *) &Original_screen_X)
                      || !Read_word_le(file,(word *) &Original_screen_Y) )
                      File_error=2;
                  }
                  else
                    File_error=2;
                }
                else
                  File_error=2;
                break;

              case 2 : // color de transparence
                if (Read_byte(file,&temp_byte))
                {
                  if (temp_byte==1)
                  {
                    index++;
                    if (! Read_byte(file,&Back_color))
                      File_error=2;
                  }
                  else
                    File_error=2;
                }
                else
                  File_error=2;
                break;

              default:
                if (Read_byte(file,&temp_byte))
                {
                  index+=temp_byte;
                  if (fseek(file,temp_byte,SEEK_CUR))
                    File_error=2;
                }
                else
                  File_error=2;
            }
          }
          else
            File_error=2;
        }
        if ( (!File_error) && (index!=header.Jump) )
          File_error=2;
      }

      /*Init_lecture();*/

      if (!File_error)
      {
        Pre_load(context, header.Width,header.Height,file_size,FORMAT_PKM,PIXEL_SIMPLE,0);
        if (File_error==0)
        {
          
          context->Width=header.Width;
          context->Height=header.Height;
          image_size=(dword)(context->Width*context->Height);
          // Palette lue en 64
          memcpy(context->Palette,header.Palette,sizeof(T_Palette));
          Palette_64_to_256(context->Palette);
          Palette_loaded(context);

          Compteur_de_donnees_packees=0;
          Compteur_de_pixels=0;
          Taille_pack=(file_size)-780-header.Jump;

          // Boucle de décompression:
          while ( (Compteur_de_pixels<image_size) && (Compteur_de_donnees_packees<Taille_pack) && (!File_error) )
          {
            if(Read_byte(file, &temp_byte)!=1) 
            {
              File_error=2;
              break;
            }

            // Si ce n'est pas un octet de reconnaissance, c'est un pixel brut
            if ( (temp_byte!=header.Recog1) && (temp_byte!=header.Recog2) )
            {
              Set_pixel(context, Compteur_de_pixels % context->Width,
                                  Compteur_de_pixels / context->Width,
                                  temp_byte);
              Compteur_de_donnees_packees++;
              Compteur_de_pixels++;
            }
            else // Sinon, On regarde si on va décompacter un...
            { // ... nombre de pixels tenant sur un byte
                if (temp_byte==header.Recog1)
                {
                  if(Read_byte(file, &color)!=1)
                {
                    File_error=2;
                    break;
                }
                if(Read_byte(file, &temp_byte)!=1)
                {
                    File_error=2;
                    break;
                }
                for (index=0; index<temp_byte; index++)
                  Set_pixel(context, (Compteur_de_pixels+index) % context->Width,
                                      (Compteur_de_pixels+index) / context->Width,
                                      color);
                Compteur_de_pixels+=temp_byte;
                Compteur_de_donnees_packees+=3;
              }
              else // ... nombre de pixels tenant sur un word
              {
                if(Read_byte(file, &color)!=1)
                {
                    File_error=2;
                    break;
        }
                Read_word_be(file, &len);
                for (index=0; index<len; index++)
                  Set_pixel(context, (Compteur_de_pixels+index) % context->Width,
                                      (Compteur_de_pixels+index) / context->Width,
                                      color);
                Compteur_de_pixels+=len;
                Compteur_de_donnees_packees+=4;
              }
            }
          }
        }
      }
      /*Close_lecture();*/
    }
    else // Lecture header impossible: Error ne modifiant pas l'image
      File_error=1;

    fclose(file);
  }
  else // Ouv. fichier impossible: Error ne modifiant pas l'image
    File_error=1;
}


// -- Sauver un fichier au format PKM ---------------------------------------

  // Trouver quels sont les octets de reconnaissance
  void Find_recog(byte * recog1, byte * recog2)
  {
    dword Find_recon[256]; // Table d'utilisation de couleurs
    byte  best;   // Meilleure couleur pour recon (recon1 puis recon2)
    dword NBest;  // Nombre d'occurences de cette couleur
    word  index;


    // On commence par compter l'utilisation de chaque couleurs
    Count_used_colors(Find_recon);

    // Ensuite recog1 devient celle la moins utilisée de celles-ci
    *recog1=0;
    best=1;
    NBest=INT_MAX; // Une même couleur ne pourra jamais être utilisée 1M de fois.
    for (index=1;index<=255;index++)
      if (Find_recon[index]<NBest)
      {
        best=index;
        NBest=Find_recon[index];
      }
    *recog1=best;

    // Enfin recog2 devient la 2ème moins utilisée
    *recog2=0;
    best=0;
    NBest=INT_MAX;
    for (index=0;index<=255;index++)
      if ( (Find_recon[index]<NBest) && (index!=*recog1) )
      {
        best=index;
        NBest=Find_recon[index];
      }
    *recog2=best;
  }


void Save_PKM(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_PKM_Header header;
  dword Compteur_de_pixels;
  dword image_size;
  word  repetitions;
  byte  last_color;
  byte  pixel_value;
  byte  comment_size;



  // Construction du header
  memcpy(header.Ident,"PKM",3);
  header.Method=0;
  Find_recog(&header.Recog1,&header.Recog2);
  header.Width=context->Width;
  header.Height=context->Height;
  memcpy(header.Palette,context->Palette,sizeof(T_Palette));
  Palette_256_to_64(header.Palette);

  // Calcul de la taille du Post-header
  header.Jump=9; // 6 pour les dimensions de l'ecran + 3 pour la back-color
  comment_size=strlen(context->Comment);
  if (comment_size)
    header.Jump+=comment_size+2;


  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;

  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    // Ecriture du header
    if (Write_bytes(file,&header.Ident,3) &&
        Write_byte(file,header.Method) &&
        Write_byte(file,header.Recog1) &&
        Write_byte(file,header.Recog2) &&
        Write_word_le(file,header.Width) &&
        Write_word_le(file,header.Height) &&
        Write_bytes(file,&header.Palette,sizeof(T_Palette)) &&
        Write_word_le(file,header.Jump))
    {
      Init_write_buffer();

      // Ecriture du commentaire
      // (Compteur_de_pixels est utilisé ici comme simple index de comptage)
      if (comment_size)
      {
        Write_one_byte(file,0);
        Write_one_byte(file,comment_size);
        for (Compteur_de_pixels=0; Compteur_de_pixels<comment_size; Compteur_de_pixels++)
          Write_one_byte(file,context->Comment[Compteur_de_pixels]);
      }
      // Ecriture des dimensions de l'écran
      Write_one_byte(file,1);
      Write_one_byte(file,4);
      Write_one_byte(file,Screen_width&0xFF);
      Write_one_byte(file,Screen_width>>8);
      Write_one_byte(file,Screen_height&0xFF);
      Write_one_byte(file,Screen_height>>8);
      // Ecriture de la back-color
      Write_one_byte(file,2);
      Write_one_byte(file,1);
      Write_one_byte(file,Back_color);

      // Routine de compression PKM de l'image
      image_size=(dword)(context->Width*context->Height);
      Compteur_de_pixels=0;
      pixel_value=Get_pixel(context, 0,0);

      while ( (Compteur_de_pixels<image_size) && (!File_error) )
      {
        Compteur_de_pixels++;
        repetitions=1;
        last_color=pixel_value;
        if(Compteur_de_pixels<image_size)
        {
          pixel_value=Get_pixel(context, Compteur_de_pixels % context->Width,Compteur_de_pixels / context->Width);
        }
        while ( (pixel_value==last_color)
             && (Compteur_de_pixels<image_size)
             && (repetitions<65535) )
        {
          Compteur_de_pixels++;
          repetitions++;
          if(Compteur_de_pixels>=image_size) break;
          pixel_value=Get_pixel(context, Compteur_de_pixels % context->Width,Compteur_de_pixels / context->Width);
        }

        if ( (last_color!=header.Recog1) && (last_color!=header.Recog2) )
        {
          if (repetitions==1)
            Write_one_byte(file,last_color);
          else
          if (repetitions==2)
          {
            Write_one_byte(file,last_color);
            Write_one_byte(file,last_color);
          }
          else
          if ( (repetitions>2) && (repetitions<256) )
          { // RECON1/couleur/nombre
            Write_one_byte(file,header.Recog1);
            Write_one_byte(file,last_color);
            Write_one_byte(file,repetitions&0xFF);
          }
          else
          if (repetitions>=256)
          { // RECON2/couleur/hi(nombre)/lo(nombre)
            Write_one_byte(file,header.Recog2);
            Write_one_byte(file,last_color);
            Write_one_byte(file,repetitions>>8);
            Write_one_byte(file,repetitions&0xFF);
          }
        }
        else
        {
          if (repetitions<256)
          {
            Write_one_byte(file,header.Recog1);
            Write_one_byte(file,last_color);
            Write_one_byte(file,repetitions&0xFF);
          }
          else
          {
            Write_one_byte(file,header.Recog2);
            Write_one_byte(file,last_color);
            Write_one_byte(file,repetitions>>8);
            Write_one_byte(file,repetitions&0xFF);
          }
        }
      }

      End_write(file);
    }
    else
      File_error=1;
    fclose(file);
  }
  else
  {
    File_error=1;
    fclose(file);
  }
  //   S'il y a eu une erreur de sauvegarde, on ne va tout de même pas laisser
  // ce fichier pourri traîner... Ca fait pas propre.
  if (File_error)
    remove(filename);
}


//////////////////////////////////// CEL ////////////////////////////////////
typedef struct
{
  word Width;              // width de l'image
  word Height;             // height de l'image
} T_CEL_Header1;

typedef struct
{
  byte Signature[4];           // Signature du format
  byte Kind;               // Type de fichier ($10=PALette $20=BitMaP)
  byte Nb_bits;             // Nombre de bits
  word Filler1;            // ???
  word Width;            // width de l'image
  word Height;            // height de l'image
  word X_offset;         // Offset en X de l'image
  word Y_offset;         // Offset en Y de l'image
  byte Filler2[16];        // ???
} T_CEL_Header2;

// -- Tester si un fichier est au format CEL --------------------------------

void Test_CEL(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  int  size;
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  int file_size;

  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);
  file_size=File_length(filename);
  if (file_size==0)
  {
    File_error = 1; // Si on ne peut pas faire de stat il vaut mieux laisser tomber
    return;
  }
  
  if (! (file=fopen(filename, "rb")))
  {
    File_error = 1;
    return;
  }
  if (Read_word_le(file,&header1.Width) &&
      Read_word_le(file,&header1.Height) )
  {
      //   Vu que ce header n'a pas de signature, il va falloir tester la
      // cohérence de la dimension de l'image avec celle du fichier.
      
      size=file_size-4;
      if ( (!size) || ( (((header1.Width+1)>>1)*header1.Height)!=size ) )
      {
        // Tentative de reconnaissance de la signature des nouveaux fichiers

        fseek(file,0,SEEK_SET);        
        if (Read_bytes(file,&header2.Signature,4) &&
            !memcmp(header2.Signature,"KiSS",4) &&
            Read_byte(file,&header2.Kind) &&
            (header2.Kind==0x20) &&
            Read_byte(file,&header2.Nb_bits) &&
            Read_word_le(file,&header2.Filler1) &&
            Read_word_le(file,&header2.Width) &&
            Read_word_le(file,&header2.Height) &&
            Read_word_le(file,&header2.X_offset) &&
            Read_word_le(file,&header2.Y_offset) &&
            Read_bytes(file,&header2.Filler2,16))
        {
          // ok
        }
        else
          File_error=1;
      }
      else
        File_error=1;
  }
  else
  {
    File_error=1;
  }
  fclose(file);    
}


// -- Lire un fichier au format CEL -----------------------------------------

void Load_CEL(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  short x_pos;
  short y_pos;
  byte  last_byte=0;
  long  file_size;
  const long int header_size = 4;

  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);
  if ((file=fopen(filename, "rb")))
  {
    if (Read_word_le(file,&(header1.Width))
    &&  Read_word_le(file,&(header1.Height)))
    {
      file_size=File_length_file(file);
      if ( (file_size>header_size)
        && ( (((header1.Width+1)>>1)*header1.Height)==(file_size-header_size) ) )
      {
        // Chargement d'un fichier CEL sans signature (vieux fichiers)
        context->Width=header1.Width;
        context->Height=header1.Height;
        Original_screen_X=context->Width;
        Original_screen_Y=context->Height;
        Pre_load(context, context->Width,context->Height,file_size,FORMAT_CEL,PIXEL_SIMPLE,0);
        if (File_error==0)
        {
          // Chargement de l'image
          /*Init_lecture();*/
          for (y_pos=0;((y_pos<context->Height) && (!File_error));y_pos++)
            for (x_pos=0;((x_pos<context->Width) && (!File_error));x_pos++)
              if ((x_pos & 1)==0)
              {
                if(Read_byte(file,&last_byte)!=1) File_error = 2;
                Set_pixel(context, x_pos,y_pos,(last_byte >> 4));
              }
              else
                Set_pixel(context, x_pos,y_pos,(last_byte & 15));
          /*Close_lecture();*/
        }
      }
      else
      {
        // On réessaye avec le nouveau format

        fseek(file,0,SEEK_SET);
        if (Read_bytes(file,header2.Signature,4)
        && Read_byte(file,&(header2.Kind))
        && Read_byte(file,&(header2.Nb_bits))
        && Read_word_le(file,&(header2.Filler1))
        && Read_word_le(file,&(header2.Width))
        && Read_word_le(file,&(header2.Height))
        && Read_word_le(file,&(header2.X_offset))
        && Read_word_le(file,&(header2.Y_offset))
        && Read_bytes(file,header2.Filler2,16)
        )
        {
          // Chargement d'un fichier CEL avec signature (nouveaux fichiers)

          context->Width=header2.Width+header2.X_offset;
          context->Height=header2.Height+header2.Y_offset;
          Original_screen_X=context->Width;
          Original_screen_Y=context->Height;
          Pre_load(context, context->Width,context->Height,file_size,FORMAT_CEL,PIXEL_SIMPLE,0);
          if (File_error==0)
          {
            // Chargement de l'image
            /*Init_lecture();*/

            if (!File_error)
            {
              // Effacement du décalage
              for (y_pos=0;y_pos<header2.Y_offset;y_pos++)
                for (x_pos=0;x_pos<context->Width;x_pos++)
                  Set_pixel(context, x_pos,y_pos,0);
              for (y_pos=header2.Y_offset;y_pos<context->Height;y_pos++)
                for (x_pos=0;x_pos<header2.X_offset;x_pos++)
                  Set_pixel(context, x_pos,y_pos,0);

              switch(header2.Nb_bits)
              {
                case 4:
                  for (y_pos=0;((y_pos<header2.Height) && (!File_error));y_pos++)
                    for (x_pos=0;((x_pos<header2.Width) && (!File_error));x_pos++)
                      if ((x_pos & 1)==0)
                      {
                        if(Read_byte(file,&last_byte)!=1) File_error=2;
                        Set_pixel(context, x_pos+header2.X_offset,y_pos+header2.Y_offset,(last_byte >> 4));
                      }
                      else
                        Set_pixel(context, x_pos+header2.X_offset,y_pos+header2.Y_offset,(last_byte & 15));
                  break;

                case 8:
                  for (y_pos=0;((y_pos<header2.Height) && (!File_error));y_pos++)
                    for (x_pos=0;((x_pos<header2.Width) && (!File_error));x_pos++)
                    {
                      byte byte_read;
                      if(Read_byte(file,&byte_read)!=1) File_error = 2;
                      Set_pixel(context, x_pos+header2.X_offset,y_pos+header2.Y_offset,byte_read);
                      }
                  break;

                default:
                  File_error=1;
              }
            }
            /*Close_lecture();*/
          }
        }
        else
          File_error=1;
      }
      fclose(file);
    }
    else
      File_error=1;
  }
  else
    File_error=1;
}


// -- Ecrire un fichier au format CEL ---------------------------------------

void Save_CEL(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  short x_pos;
  short y_pos;
  byte  last_byte=0;
  dword Utilisation[256]; // Table d'utilisation de couleurs


  // On commence par compter l'utilisation de chaque couleurs
  Count_used_colors(Utilisation);

  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);
  if ((file=fopen(filename,"wb")))
  {
    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (x_pos=16;((x_pos<256) && (!Utilisation[x_pos]));x_pos++);

    if (x_pos==256)
    {
      // Cas d'une image 16 couleurs (écriture à l'ancien format)

      header1.Width =context->Width;
      header1.Height=context->Height;

      if (Write_word_le(file,header1.Width)
      && Write_word_le(file,header1.Height)
      )
      {
        // Sauvegarde de l'image
        Init_write_buffer();
        for (y_pos=0;((y_pos<context->Height) && (!File_error));y_pos++)
        {
          for (x_pos=0;((x_pos<context->Width) && (!File_error));x_pos++)
            if ((x_pos & 1)==0)
              last_byte=(Get_pixel(context, x_pos,y_pos) << 4);
            else
            {
              last_byte=last_byte | (Get_pixel(context, x_pos,y_pos) & 15);
              Write_one_byte(file,last_byte);
            }

          if ((x_pos & 1)==1)
            Write_one_byte(file,last_byte);
        }
        End_write(file);
      }
      else
        File_error=1;
      fclose(file);
    }
    else
    {
      // Cas d'une image 256 couleurs (écriture au nouveau format)

      // Recherche du décalage
      for (y_pos=0;y_pos<context->Height;y_pos++)
      {
        for (x_pos=0;x_pos<context->Width;x_pos++)
          if (Get_pixel(context, x_pos,y_pos)!=0)
            break;
        if (Get_pixel(context, x_pos,y_pos)!=0)
          break;
      }
      header2.Y_offset=y_pos;
      for (x_pos=0;x_pos<context->Width;x_pos++)
      {
        for (y_pos=0;y_pos<context->Height;y_pos++)
          if (Get_pixel(context, x_pos,y_pos)!=0)
            break;
        if (Get_pixel(context, x_pos,y_pos)!=0)
          break;
      }
      header2.X_offset=x_pos;

      memcpy(header2.Signature,"KiSS",4); // Initialisation de la signature
      header2.Kind=0x20;              // Initialisation du type (BitMaP)
      header2.Nb_bits=8;               // Initialisation du nombre de bits
      header2.Filler1=0;              // Initialisation du filler 1 (?)
      header2.Width=context->Width-header2.X_offset; // Initialisation de la largeur
      header2.Height=context->Height-header2.Y_offset; // Initialisation de la hauteur
      for (x_pos=0;x_pos<16;x_pos++)  // Initialisation du filler 2 (?)
        header2.Filler2[x_pos]=0;

      if (Write_bytes(file,header2.Signature,4)
      && Write_byte(file,header2.Kind)
      && Write_byte(file,header2.Nb_bits)
      && Write_word_le(file,header2.Filler1)
      && Write_word_le(file,header2.Width)
      && Write_word_le(file,header2.Height)
      && Write_word_le(file,header2.X_offset)
      && Write_word_le(file,header2.Y_offset)
      && Write_bytes(file,header2.Filler2,14)
      )
      {
        // Sauvegarde de l'image
        Init_write_buffer();
        for (y_pos=0;((y_pos<header2.Height) && (!File_error));y_pos++)
          for (x_pos=0;((x_pos<header2.Width) && (!File_error));x_pos++)
            Write_one_byte(file,Get_pixel(context, x_pos+header2.X_offset,y_pos+header2.Y_offset));
        End_write(file);
      }
      else
        File_error=1;
      fclose(file);
    }

    if (File_error)
      remove(filename);
  }
  else
    File_error=1;
}


//////////////////////////////////// KCF ////////////////////////////////////
typedef struct
{
  struct
  {
    struct
    {
      byte Byte1;
      byte Byte2;
    } color[16];
  } Palette[10];
} T_KCF_Header;

// -- Tester si un fichier est au format KCF --------------------------------

void Test_KCF(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_KCF_Header buffer;
  T_CEL_Header2 header2;
  int pal_index;
  int color_index;

  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);
  if ((file=fopen(filename, "rb")))
  {
    if (File_length_file(file)==320)
    {
      Read_bytes(file,&buffer,320);
      // On vérifie une propriété de la structure de palette:
      for (pal_index=0;pal_index<10;pal_index++)
        for (color_index=0;color_index<16;color_index++)
          if ((buffer.Palette[pal_index].color[color_index].Byte2>>4)!=0)
            File_error=1;
    }
    else
    {
      if (Read_bytes(file,header2.Signature,4)
        && Read_byte(file,&(header2.Kind))
        && Read_byte(file,&(header2.Nb_bits))
        && Read_word_le(file,&(header2.Filler1))
        && Read_word_le(file,&(header2.Width))
        && Read_word_le(file,&(header2.Height))
        && Read_word_le(file,&(header2.X_offset))
        && Read_word_le(file,&(header2.Y_offset))
        && Read_bytes(file,header2.Filler2,14)
        )
      {
        if (memcmp(header2.Signature,"KiSS",4)==0)
        {
          if (header2.Kind!=0x10)
            File_error=1;
        }
        else
          File_error=1;
      }
      else
        File_error=1;
    }
    fclose(file);
  }
  else
    File_error=1;
}


// -- Lire un fichier au format KCF -----------------------------------------

void Load_KCF(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_KCF_Header buffer;
  T_CEL_Header2 header2;
  byte bytes[3];
  int pal_index;
  int color_index;
  int index;
  long  file_size;


  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);
  if ((file=fopen(filename, "rb")))
  {
    file_size=File_length_file(file);
    if (file_size==320)
    {
      // Fichier KCF à l'ancien format

      if (Read_bytes(file,&buffer,320))
      {
        // Pre_load(context, ?); // Pas possible... pas d'image...

        if (Config.Clear_palette)
          memset(context->Palette,0,sizeof(T_Palette));

        // Chargement de la palette
        for (pal_index=0;pal_index<10;pal_index++)
          for (color_index=0;color_index<16;color_index++)
          {
            index=16+(pal_index*16)+color_index;
            context->Palette[index].R=((buffer.Palette[pal_index].color[color_index].Byte1 >> 4) << 4);
            context->Palette[index].B=((buffer.Palette[pal_index].color[color_index].Byte1 & 15) << 4);
            context->Palette[index].G=((buffer.Palette[pal_index].color[color_index].Byte2 & 15) << 4);
          }

        for (index=0;index<16;index++)
        {
          context->Palette[index].R=context->Palette[index+16].R;
          context->Palette[index].G=context->Palette[index+16].G;
          context->Palette[index].B=context->Palette[index+16].B;
        }

        Palette_loaded(context);
      }
      else
        File_error=1;
    }
    else
    {
      // Fichier KCF au nouveau format

      if (Read_bytes(file,header2.Signature,4)
        && Read_byte(file,&(header2.Kind))
        && Read_byte(file,&(header2.Nb_bits))
        && Read_word_le(file,&(header2.Filler1))
        && Read_word_le(file,&(header2.Width))
        && Read_word_le(file,&(header2.Height))
        && Read_word_le(file,&(header2.X_offset))
        && Read_word_le(file,&(header2.Y_offset))
        && Read_bytes(file,header2.Filler2,14)
        )
      {
        // Pre_load(context, ?); // Pas possible... pas d'image...

        index=(header2.Nb_bits==12)?16:0;
        for (pal_index=0;pal_index<header2.Height;pal_index++)
        {
           // Pour chaque palette

           for (color_index=0;color_index<header2.Width;color_index++)
           {
             // Pour chaque couleur

             switch(header2.Nb_bits)
             {
               case 12: // RRRR BBBB | 0000 VVVV
                 Read_bytes(file,bytes,2);
                 context->Palette[index].R=(bytes[0] >> 4) << 4;
                 context->Palette[index].B=(bytes[0] & 15) << 4;
                 context->Palette[index].G=(bytes[1] & 15) << 4;
                 break;

               case 24: // RRRR RRRR | VVVV VVVV | BBBB BBBB
                 Read_bytes(file,bytes,3);
                 context->Palette[index].R=bytes[0];
                 context->Palette[index].G=bytes[1];
                 context->Palette[index].B=bytes[2];
             }

             index++;
           }
        }

        if (header2.Nb_bits==12)
          for (index=0;index<16;index++)
          {
            context->Palette[index].R=context->Palette[index+16].R;
            context->Palette[index].G=context->Palette[index+16].G;
            context->Palette[index].B=context->Palette[index+16].B;
          }

        Palette_loaded(context);
      }
      else
        File_error=1;
    }
    fclose(file);
  }
  else
    File_error=1;
}


// -- Ecrire un fichier au format KCF ---------------------------------------

void Save_KCF(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS];
  FILE *file;
  T_KCF_Header buffer;
  T_CEL_Header2 header2;
  byte bytes[3];
  int pal_index;
  int color_index;
  int index;
  dword Utilisation[256]; // Table d'utilisation de couleurs

  // On commence par compter l'utilisation de chaque couleurs
  Count_used_colors(Utilisation);

  File_error=0;
  Get_full_filename(filename, context->File_name, context->File_directory);
  if ((file=fopen(filename,"wb")))
  {
    // Sauvegarde de la palette

    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (index=16;((index<256) && (!Utilisation[index]));index++);

    if (index==256)
    {
      // Cas d'une image 16 couleurs (écriture à l'ancien format)

      for (pal_index=0;pal_index<10;pal_index++)
        for (color_index=0;color_index<16;color_index++)
        {
          index=16+(pal_index*16)+color_index;
          buffer.Palette[pal_index].color[color_index].Byte1=((context->Palette[index].R>>4)<<4) | (context->Palette[index].B>>4);
          buffer.Palette[pal_index].color[color_index].Byte2=context->Palette[index].G>>4;
        }

      if (! Write_bytes(file,&buffer,320))
        File_error=1;
    }
    else
    {
      // Cas d'une image 256 couleurs (écriture au nouveau format)

      memcpy(header2.Signature,"KiSS",4); // Initialisation de la signature
      header2.Kind=0x10;              // Initialisation du type (PALette)
      header2.Nb_bits=24;              // Initialisation du nombre de bits
      header2.Filler1=0;              // Initialisation du filler 1 (?)
      header2.Width=256;            // Initialisation du nombre de couleurs
      header2.Height=1;              // Initialisation du nombre de palettes
      header2.X_offset=0;           // Initialisation du décalage X
      header2.Y_offset=0;           // Initialisation du décalage Y
      for (index=0;index<16;index++) // Initialisation du filler 2 (?)
        header2.Filler2[index]=0;

      if (!Write_bytes(file,header2.Signature,4)
      || !Write_byte(file,header2.Kind)
      || !Write_byte(file,header2.Nb_bits)
      || !Write_word_le(file,header2.Filler1)
      || !Write_word_le(file,header2.Width)
      || !Write_word_le(file,header2.Height)
      || !Write_word_le(file,header2.X_offset)
      || !Write_word_le(file,header2.Y_offset)
      || !Write_bytes(file,header2.Filler2,14)
      )
        File_error=1;

      for (index=0;(index<256) && (!File_error);index++)
      {
        bytes[0]=context->Palette[index].R;
        bytes[1]=context->Palette[index].G;
        bytes[2]=context->Palette[index].B;
        if (! Write_bytes(file,bytes,3))
          File_error=1;
      }
    }

    fclose(file);

    if (File_error)
      remove(filename);
  }
  else
    File_error=1;
}


//////////////////////////////////// PI1 ////////////////////////////////////

//// DECODAGE d'une partie d'IMAGE ////

void PI1_8b_to_16p(byte * src,byte * dest)
{
  int  i;           // index du pixel à calculer
  word byte_mask;      // Masque de decodage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la source

  byte_mask=0x8000;
  w0=(((word)src[0])<<8) | src[1];
  w1=(((word)src[2])<<8) | src[3];
  w2=(((word)src[4])<<8) | src[5];
  w3=(((word)src[6])<<8) | src[7];
  for (i=0;i<16;i++)
  {
    // Pour décoder le pixel n°i, il faut traiter les 4 words sur leur bit
    // correspondant à celui du masque

    dest[i]=((w0 & byte_mask)?0x01:0x00) |
           ((w1 & byte_mask)?0x02:0x00) |
           ((w2 & byte_mask)?0x04:0x00) |
           ((w3 & byte_mask)?0x08:0x00);
    byte_mask>>=1;
  }
}

//// CODAGE d'une partie d'IMAGE ////

void PI1_16p_to_8b(byte * src,byte * dest)
{
  int  i;           // index du pixel à calculer
  word byte_mask;      // Masque de codage
  word w0,w1,w2,w3; // Les 4 words bien ordonnés de la destination

  byte_mask=0x8000;
  w0=w1=w2=w3=0;
  for (i=0;i<16;i++)
  {
    // Pour coder le pixel n°i, il faut modifier les 4 words sur leur bit
    // correspondant à celui du masque

    w0|=(src[i] & 0x01)?byte_mask:0x00;
    w1|=(src[i] & 0x02)?byte_mask:0x00;
    w2|=(src[i] & 0x04)?byte_mask:0x00;
    w3|=(src[i] & 0x08)?byte_mask:0x00;
    byte_mask>>=1;
  }
  dest[0]=w0 >> 8;
  dest[1]=w0 & 0x00FF;
  dest[2]=w1 >> 8;
  dest[3]=w1 & 0x00FF;
  dest[4]=w2 >> 8;
  dest[5]=w2 & 0x00FF;
  dest[6]=w3 >> 8;
  dest[7]=w3 & 0x00FF;
}

//// DECODAGE de la PALETTE ////

void PI1_decode_palette(byte * src,byte * palette)
{
  int i;  // Numéro de la couleur traitée
  int ip; // index dans la palette
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  //    Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321
  
  ip=0;
  for (i=0;i<16;i++)
  {
    #if SDL_BYTEORDER == SDL_LIL_ENDIAN 
   
      w=(((word)src[(i*2)+1]<<8) | (src[(i*2)+0]));
    
      // Traitement des couleurs rouge, verte et bleue:
      palette[ip++]=(((w & 0x0007) <<  1) | ((w & 0x0008) >>  3)) << 4;
      palette[ip++]=(((w & 0x7000) >> 11) | ((w & 0x8000) >> 15)) << 4;
      palette[ip++]=(((w & 0x0700) >>  7) | ((w & 0x0800) >> 11)) << 4;
   
    #else
      w=(((word)src[(i*2+1)])|(((word)src[(i*2)])<<8));
    
      palette[ip++] = (((w & 0x0700)>>7) | ((w & 0x0800) >> 7))<<4 ;
      palette[ip++]=(((w & 0x0070)>>3) | ((w & 0x0080) >> 3))<<4 ;
      palette[ip++] = (((w & 0x0007)<<1) | ((w & 0x0008)))<<4 ;
    #endif
    
    
  } 
}

//// CODAGE de la PALETTE ////

void PI1_code_palette(byte * palette,byte * dest)
{
  int i;  // Numéro de la couleur traitée
  int ip; // index dans la palette
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  // Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321
    
  ip=0;
  for (i=0;i<16;i++)
  {
    #if SDL_BYTEORDER == SDL_LIL_ENDIAN
   
    // Traitement des couleurs rouge, verte et bleue:
    w =(((word)(palette[ip]>>2) & 0x38) >> 3) | (((word)(palette[ip]>>2) & 0x04) <<  1); ip++;
    w|=(((word)(palette[ip]>>2) & 0x38) << 9) | (((word)(palette[ip]>>2) & 0x04) << 13); ip++;
    w|=(((word)(palette[ip]>>2) & 0x38) << 5) | (((word)(palette[ip]>>2) & 0x04) <<  9); ip++;
    
    dest[(i*2)+0]=w & 0x00FF;
    dest[(i*2)+1]=(w>>8);
    #else
   
     w=(((word)(palette[ip]<<3))&0x0700);ip++;
     w|=(((word)(palette[ip]>>1))&0x0070);ip++;
     w|=(((word)(palette[ip]>>5))&0x0007);ip++;
 
    dest[(i*2)+1]=w & 0x00FF;
    dest[(i*2)+0]=(w>>8);
    #endif
  }
}


// -- Tester si un fichier est au format PI1 --------------------------------
void Test_PI1(T_IO_Context * context)
{
  FILE * file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  int  size;              // Taille du fichier
  word resolution;                 // Résolution de l'image


  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Vérification de la taille
    size=File_length_file(file);
    if ((size==32034) || (size==32066))
    {
      // Lecture et vérification de la résolution
      if (Read_word_le(file,&resolution))
      {
        if (resolution==0x0000)
          File_error=0;
      }
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format PI1 -----------------------------------------
void Load_PI1(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  word x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;
  if ((file=fopen(filename, "rb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32034);
    if (buffer!=NULL)
    {
      // Lecture du fichier dans le buffer
      if (Read_bytes(file,buffer,32034))
      {
        // Initialisation de la preview
        Pre_load(context, 320,200,File_length_file(file),FORMAT_PI1,PIXEL_SIMPLE,0);
        if (File_error==0)
        {
          // Initialisation de la palette
          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));
          PI1_decode_palette(buffer+2,(byte *)context->Palette);
          Palette_loaded(context);

          context->Width=320;
          context->Height=200;

          // Chargement/décompression de l'image
          ptr=buffer+34;
          for (y_pos=0;y_pos<200;y_pos++)
          {
            for (x_pos=0;x_pos<(320>>4);x_pos++)
            {
              PI1_8b_to_16p(ptr,pixels+(x_pos<<4));
              ptr+=8;
            }
            for (x_pos=0;x_pos<320;x_pos++)
              Set_pixel(context, x_pos,y_pos,pixels[x_pos]);
          }
        }
      }
      else
        File_error=1;
      free(buffer);
      buffer = NULL;
    }
    else
      File_error=1;
    fclose(file);
  }
  else
    File_error=1;
}


// -- Sauver un fichier au format PI1 ---------------------------------------
void Save_PI1(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;
  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32066);
    // Codage de la résolution
    buffer[0]=0x00;
    buffer[1]=0x00;
    // Codage de la palette
    PI1_code_palette((byte *)context->Palette,buffer+2);
    // Codage de l'image
    ptr=buffer+34;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<context->Height)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<context->Width);x_pos++)
          pixels[x_pos]=Get_pixel(context, x_pos,y_pos);
      }

      for (x_pos=0;x_pos<(320>>4);x_pos++)
      {
        PI1_16p_to_8b(pixels+(x_pos<<4),ptr);
        ptr+=8;
      }
    }

    memset(buffer+32034,0,32); // 32 extra NULL bytes at the end of the file to make ST Deluxe Paint happy

    if (Write_bytes(file,buffer,32066))
    {
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
    // Libération du buffer mémoire
    free(buffer);
    buffer = NULL;
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}


//////////////////////////////////// PC1 ////////////////////////////////////

//// DECOMPRESSION d'un buffer selon la méthode PACKBITS ////

void PC1_uncompress_packbits(byte * src,byte * dest)
{
  int is,id; // Les indices de parcour des buffers
  int n;     // Octet de contrôle

  for (is=id=0;id<32000;)
  {
    n=src[is++];

    if (n & 0x80)
    {
      // Recopier src[is] -n+1 fois
      n=257-n;
      for (;(n>0) && (id<32000);n--)
        dest[id++]=src[is];
      is++;
    }
    else
    {
      // Recopier n+1 octets littéralement
      n=n+1;
      for (;(n>0) && (id<32000);n--)
        dest[id++]=src[is++];
    }

    // Contrôle des erreurs
    if (n>0)
      File_error=1;
  }
}

//// COMPRESSION d'un buffer selon la méthode PACKBITS ////

void PC1_compress_packbits(byte * src,byte * dest,int source_size,int * dest_size)
{
  int is; // index dans la source
  int id; // index dans la destination
  int ir; // index de   la répétition
  int n;  // Taille des séquences
  int repet; // "Il y a répétition"

  for (is=id=0;is<source_size;)
  {
    // On recherche le 1er endroit où il y a répétition d'au moins 3 valeurs
    // identiques

    repet=0;
    for (ir=is;ir<source_size-2;ir++)
    {
      if ((src[ir]==src[ir+1]) && (src[ir+1]==src[ir+2]))
      {
        repet=1;
        break;
      }
      if ((ir-is)+1==40)
        break;
    }

    // On code la partie sans répétitions
    if (ir!=is)
    {
      n=(ir-is)+1;
      dest[id++]=n-1;
      for (;n>0;n--)
        dest[id++]=src[is++];
    }

    // On code la partie sans répétitions
    if (repet)
    {
      // On compte la quantité de fois qu'il faut répéter la valeur
      for (ir+=3;ir<source_size;ir++)
      {
        if (src[ir]!=src[is])
          break;
        if ((ir-is)+1==40)
          break;
      }
      n=(ir-is);
      dest[id++]=257-n;
      dest[id++]=src[is];
      is=ir;
    }
  }

  // On renseigne la taille du buffer compressé
  *dest_size=id;
}

//// DECODAGE d'une partie d'IMAGE ////

// Transformation de 4 plans de bits en 1 ligne de pixels

void PC1_4bp_to_1line(byte * src0,byte * src1,byte * src2,byte * src3,byte * dest)
{
  int  i,j;         // Compteurs
  int  ip;          // index du pixel à calculer
  byte byte_mask;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    b0=src0[i];
    b1=src1[i];
    b2=src2[i];
    b3=src3[i];
    // Pour chacun des 8 bits des octets
    byte_mask=0x80;
    for (j=0;j<8;j++)
    {
      dest[ip++]=((b0 & byte_mask)?0x01:0x00) |
                ((b1 & byte_mask)?0x02:0x00) |
                ((b2 & byte_mask)?0x04:0x00) |
                ((b3 & byte_mask)?0x08:0x00);
      byte_mask>>=1;
    }
  }
}

//// CODAGE d'une partie d'IMAGE ////

// Transformation d'1 ligne de pixels en 4 plans de bits

void PC1_1line_to_4bp(byte * src,byte * dst0,byte * dst1,byte * dst2,byte * dst3)
{
  int  i,j;         // Compteurs
  int  ip;          // index du pixel à calculer
  byte byte_mask;      // Masque de decodage
  byte b0,b1,b2,b3; // Les 4 octets des plans bits sources

  ip=0;
  // Pour chacun des 40 octets des plans de bits
  for (i=0;i<40;i++)
  {
    // Pour chacun des 8 bits des octets
    byte_mask=0x80;
    b0=b1=b2=b3=0;
    for (j=0;j<8;j++)
    {
      b0|=(src[ip] & 0x01)?byte_mask:0x00;
      b1|=(src[ip] & 0x02)?byte_mask:0x00;
      b2|=(src[ip] & 0x04)?byte_mask:0x00;
      b3|=(src[ip] & 0x08)?byte_mask:0x00;
      ip++;
      byte_mask>>=1;
    }
    dst0[i]=b0;
    dst1[i]=b1;
    dst2[i]=b2;
    dst3[i]=b3;
  }
}


// -- Tester si un fichier est au format PC1 --------------------------------
void Test_PC1(T_IO_Context * context)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  int  size;              // Taille du fichier
  word resolution;                 // Résolution de l'image


  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Vérification de la taille
    size=File_length_file(file);
    if ((size<=32066))
    {
      // Lecture et vérification de la résolution
      if (Read_word_le(file,&resolution))
      {
        if (resolution==0x0080)
          File_error=0;
      }
    }
    // Fermeture du fichier
    fclose(file);
  }
}


// -- Lire un fichier au format PC1 -----------------------------------------
void Load_PC1(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  int  size;
  word x_pos,y_pos;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;
  if ((file=fopen(filename, "rb")))
  {
    size=File_length_file(file);
    // allocation des buffers mémoire
    buffercomp=(byte *)malloc(size);
    bufferdecomp=(byte *)malloc(32000);
    if ( (buffercomp!=NULL) && (bufferdecomp!=NULL) )
    {
      // Lecture du fichier dans le buffer
      if (Read_bytes(file,buffercomp,size))
      {
        // Initialisation de la preview
        Pre_load(context, 320,200,File_length_file(file),FORMAT_PC1,PIXEL_SIMPLE,0);
        if (File_error==0)
        {
          // Initialisation de la palette
          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));
          PI1_decode_palette(buffercomp+2,(byte *)context->Palette);
          Palette_loaded(context);

          context->Width=320;
          context->Height=200;

          // Décompression du buffer
          PC1_uncompress_packbits(buffercomp+34,bufferdecomp);

          // Décodage de l'image
          ptr=bufferdecomp;
          for (y_pos=0;y_pos<200;y_pos++)
          {
            // Décodage de la scanline
            PC1_4bp_to_1line(ptr,ptr+40,ptr+80,ptr+120,pixels);
            ptr+=160;
            // Chargement de la ligne
            for (x_pos=0;x_pos<320;x_pos++)
              Set_pixel(context, x_pos,y_pos,pixels[x_pos]);
          }
        }
      }
      else
        File_error=1;
      free(bufferdecomp);
      free(buffercomp);
      buffercomp = bufferdecomp = NULL;
    }
    else
    {
      File_error=1;
      free(bufferdecomp);
      free(buffercomp);
      buffercomp = bufferdecomp = NULL;
    }
    fclose(file);
  }
  else
    File_error=1;
}


// -- Sauver un fichier au format PC1 ---------------------------------------
void Save_PC1(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  int   size;
  short x_pos,y_pos;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;
  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    // Allocation des buffers mémoire
    bufferdecomp=(byte *)malloc(32000);
    buffercomp  =(byte *)malloc(64066);
    // Codage de la résolution
    buffercomp[0]=0x80;
    buffercomp[1]=0x00;
    // Codage de la palette
    PI1_code_palette((byte *)context->Palette,buffercomp+2);
    // Codage de l'image
    ptr=bufferdecomp;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<context->Height)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<context->Width);x_pos++)
          pixels[x_pos]=Get_pixel(context, x_pos,y_pos);
      }

      // Encodage de la scanline
      PC1_1line_to_4bp(pixels,ptr,ptr+40,ptr+80,ptr+120);
      ptr+=160;
    }

    // Compression du buffer
    PC1_compress_packbits(bufferdecomp,buffercomp+34,32000,&size);
    size+=34;
    for (x_pos=0;x_pos<16;x_pos++)
      buffercomp[size++]=0;

    if (Write_bytes(file,buffercomp,size))
    {
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
    // Libération des buffers mémoire
    free(bufferdecomp);
    free(buffercomp);
    buffercomp = bufferdecomp = NULL;
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}


//////////////////////////////////// NEO ////////////////////////////////////

void Test_NEO(T_IO_Context * context)
{
  FILE *file;              // Fichier du fichier
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  int  size;              // Taille du fichier
  word resolution;                 // Résolution de l'image


  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=1;

  // Ouverture du fichier
  if ((file=fopen(filename, "rb")))
  {
    // Vérification de la taille
    size=File_length_file(file);
    if ((size==32128))
    {
      // Flag word : toujours 0
      if (Read_word_le(file,&resolution))
      {
        if (resolution == 0)
          File_error = 0;
      }

      // Lecture et vérification de la résolution
      if (Read_word_le(file,&resolution))
      {
        if (resolution==0 || resolution==1 || resolution==2)
          File_error |= 0;
      }
    }
    // Fermeture du fichier
    fclose(file);
  }

}

void Load_NEO(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  word x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;
  if ((file=fopen(filename, "rb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32128);
    if (buffer!=NULL)
    {
      // Lecture du fichier dans le buffer
      if (Read_bytes(file,buffer,32128))
      {
        // Initialisation de la preview
        Pre_load(context, 320,200,File_length_file(file),FORMAT_NEO,PIXEL_SIMPLE,0);
        if (File_error==0)
        {
          // Initialisation de la palette
          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));
          // on saute la résolution et le flag, chacun 2 bits
          PI1_decode_palette(buffer+4,(byte *)context->Palette);
          Palette_loaded(context);

          context->Width=320;
          context->Height=200;

          // Chargement/décompression de l'image
          ptr=buffer+128;
          for (y_pos=0;y_pos<200;y_pos++)
          {
            for (x_pos=0;x_pos<(320>>4);x_pos++)
            {
              PI1_8b_to_16p(ptr,pixels+(x_pos<<4));
              ptr+=8;
            }
            for (x_pos=0;x_pos<320;x_pos++)
              Set_pixel(context, x_pos,y_pos,pixels[x_pos]);
          }
        }
      }
      else
        File_error=1;
      free(buffer);
      buffer = NULL;
    }
    else
      File_error=1;
    fclose(file);
  }
  else
    File_error=1;
}

void Save_NEO(T_IO_Context * context)
{
  char filename[MAX_PATH_CHARACTERS]; // Nom complet du fichier
  FILE *file;
  short x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  Get_full_filename(filename, context->File_name, context->File_directory);

  File_error=0;
  // Ouverture du fichier
  if ((file=fopen(filename,"wb")))
  {
    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32128);
    // Codage de la résolution
    buffer[0]=0x00;
    buffer[1]=0x00;
    buffer[2]=0x00;
    buffer[3]=0x00;
    // Codage de la palette
    PI1_code_palette((byte *)context->Palette,buffer+4);
    // Codage de l'image
    ptr=buffer+128;
    for (y_pos=0;y_pos<200;y_pos++)
    {
      // Codage de la ligne
      memset(pixels,0,320);
      if (y_pos<context->Height)
      {
        for (x_pos=0;(x_pos<320) && (x_pos<context->Width);x_pos++)
          pixels[x_pos]=Get_pixel(context, x_pos,y_pos);
      }

      for (x_pos=0;x_pos<(320>>4);x_pos++)
      {
        PI1_16p_to_8b(pixels+(x_pos<<4),ptr);
        ptr+=8;
      }
    }

    if (Write_bytes(file,buffer,32128))
    {
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      remove(filename);
      File_error=1;
    }
    // Libération du buffer mémoire
    free(buffer);
    buffer = NULL;
  }
  else
  {
    fclose(file);
    remove(filename);
    File_error=1;
  }
}

//////////////////////////////////// C64 ////////////////////////////////////
void Test_C64(T_IO_Context * context)
{  
    FILE* file;
    char filename[MAX_PATH_CHARACTERS];
    long file_size;
  
    Get_full_filename(filename, context->File_name, context->File_directory);
  
    file = fopen(filename,"rb");
  
    if (file)
    {
        file_size = File_length_file(file);
        switch (file_size)
        {
            case 1000: // screen or color
            case 1002: // (screen or color) + loadaddr
            case 8000: // raw bitmap
            case 8002: // raw bitmap with loadaddr
            case 9000: // bitmap + screen
            case 9002: // bitmap + screen + loadaddr
            case 10001: // multicolor
            case 10003: // multicolor + loadaddr
                File_error = 0;
                break;
            default: // then we don't know for now.
            File_error = 1;
        }
        fclose (file);
    }
    else
    {
        File_error = 1;
    }
}

void Load_C64_hires(T_IO_Context *context, byte *bitmap, byte *colors)
{
    int cx,cy,x,y,c[4],pixel,color;
  
    for(cy=0; cy<25; cy++)
    {
        for(cx=0; cx<40; cx++)
        {
            c[0]=colors[cy*40+cx]&15;
            c[1]=colors[cy*40+cx]>>4;
            for(y=0; y<8; y++)
            {
                pixel=bitmap[cy*320+cx*8+y];
                for(x=0; x<8; x++)
                {
                    color=c[pixel&(1<<(7-x))?1:0];
                    Set_pixel(context, cx*8+x,cy*8+y,color);
                }
            }
        }
    }
}

void Load_C64_multi(T_IO_Context *context, byte *bitmap, byte *colors, byte *nybble, byte background)
{
    int cx,cy,x,y,c[4],pixel,color;
    c[0]=background&15;
    for(cy=0; cy<25; cy++)
    {
        for(cx=0; cx<40; cx++)
        {
            c[1]=colors[cy*40+cx]>>4;
            c[2]=colors[cy*40+cx]&15;
            c[3]=nybble[cy*40+cx]&15;
                
            for(y=0; y<8; y++)
            {
                pixel=bitmap[cy*320+cx*8+y];
                for(x=0; x<4; x++)
                {
                    color=c[(pixel&3)];
                    pixel>>=2;
                    Set_pixel(context, cx*4+(3-x),cy*8+y,color);
                }
            }
        }
    }
}

void Load_C64(T_IO_Context * context)
{    
    FILE* file;
    char filename[MAX_PATH_CHARACTERS];
    long file_size;
    int i;
    byte background,hasLoadAddr=0;
    int loadFormat=0;
    enum c64_format {F_hires,F_multi,F_bitmap,F_screen,F_color};
    const char *c64_format_names[]={"hires","multicolor","bitmap","screen","color"};
    
    // Palette from http://www.pepto.de/projects/colorvic/
    byte pal[48]={
      0x00, 0x00, 0x00, 
      0xFF, 0xFF, 0xFF, 
      0x68, 0x37, 0x2B, 
      0x70, 0xA4, 0xB2, 
      0x6F, 0x3D, 0x86, 
      0x58, 0x8D, 0x43, 
      0x35, 0x28, 0x79, 
      0xB8, 0xC7, 0x6F, 
      0x6F, 0x4F, 0x25, 
      0x43, 0x39, 0x00, 
      0x9A, 0x67, 0x59, 
      0x44, 0x44, 0x44, 
      0x6C, 0x6C, 0x6C, 
      0x9A, 0xD2, 0x84, 
      0x6C, 0x5E, 0xB5, 
      0x95, 0x95, 0x95};
  
    byte bitmap[8000],colors[1000],nybble[1000];
    word width=320, height=200;
    
    Get_full_filename(filename, context->File_name, context->File_directory);
    file = fopen(filename,"rb");
  
    if (file)
    {
    File_error=0;
    file_size = File_length_file(file);

    switch (file_size)
        {
            case 1000: // screen or color
                hasLoadAddr=0;
                loadFormat=F_screen;
                break;
                
            case 1002: // (screen or color) + loadaddr
                hasLoadAddr=1;
                loadFormat=F_screen;
                break;
                    
            case 8000: // raw bitmap
                hasLoadAddr=0;
                loadFormat=F_bitmap;
                break;
                    
            case 8002: // raw bitmap with loadaddr
                hasLoadAddr=1;
                loadFormat=F_bitmap;
                break;
                    
            case 9000: // bitmap + screen
                hasLoadAddr=0;
                loadFormat=F_hires;
                break;
                    
            case 9002: // bitmap + screen + loadaddr
                hasLoadAddr=1;
                loadFormat=F_hires;
                break;
                    
            case 10001: // multicolor
                hasLoadAddr=0;
                loadFormat=F_multi;
                break;
                    
            case 10003: // multicolor + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                break;
                    
            default: // then we don't know what it is.
                File_error = 1;

        }
        
        memcpy(context->Palette,pal,48); // this set the software palette for grafx2
        Palette_loaded(context); // Always call it if you change the palette
                
    if (file_size>9002)
        width=160;
                
    if (hasLoadAddr)
    {
        // get load address
        Read_byte(file,&background);
        Read_byte(file,&background);
        sprintf(filename,"load at $%02x00",background);
    }
    else
    {
        sprintf(filename,"no addr");
    }
       
        if(file_size>9002)
        {
            context->Ratio = PIXEL_WIDE;
        }
        sprintf(context->Comment,"C64 %s, %s",
            c64_format_names[loadFormat],filename);
        Pre_load(context, width, height, file_size, FORMAT_C64, context->Ratio,0); // Do this as soon as you can
                     
        context->Width = width ;                
        context->Height = height;
                
        Read_bytes(file,bitmap,8000);

        if (file_size>8002) 
            Read_bytes(file,colors,1000);
        else
        {
            for(i=0;i<1000;i++)
            {
                colors[i]=1;
            }
        }

        if(width==160)
        {
            Read_bytes(file,nybble,1000);
            Read_byte(file,&background);
            Load_C64_multi(context,bitmap,colors,nybble,background);
        }
        else
        {
            Load_C64_hires(context,bitmap,colors);
        }
        
        File_error = 0;
        fclose(file);
    }
    else
        File_error = 1;
}

int Save_C64_window(byte *saveWhat, byte *loadAddr)
{
    int button;
    unsigned int i;
    T_Dropdown_button *what, *addr;
    char * what_label[] = {
        "All",
        "Bitmap",
        "Screen",
        "Color"
    };
    char * address_label[] = {
        "None",
        "$2000",
        "$4000",
        "$6000",
        "$8000",
        "$A000",
        "$C000",
        "$E000"
    };
       
    Open_window(200,120,"c64 settings");
    Window_set_normal_button(110,100,80,15,"Save",1,1,SDLK_RETURN);
    Window_set_normal_button(10,100,80,15,"Cancel",1,1,SDLK_ESCAPE);
    
    Print_in_window(13,18,"Data:",MC_Dark,MC_Light);
    what=Window_set_dropdown_button(10,28,90,15,70,what_label[*saveWhat],1, 0, 1, LEFT_SIDE,0);
    Window_dropdown_clear_items(what);
    for (i=0; i<sizeof(what_label)/sizeof(what_label[0]); i++)
        Window_dropdown_add_item(what,i,what_label[i]);
    
    Print_in_window(113,18,"Address:",MC_Dark,MC_Light);
    addr=Window_set_dropdown_button(110,28,70,15,70,address_label[*loadAddr/32],1, 0, 1, LEFT_SIDE,0);
    Window_dropdown_clear_items(addr);
    for (i=0; i<sizeof(address_label)/sizeof(address_label[0]); i++)
        Window_dropdown_add_item(addr,i,address_label[i]); 
    
    Update_window_area(0,0,Window_width,Window_height); 
    Display_cursor();

    do
    {
        button = Window_clicked_button();
        switch(button)
        {
            case 3: // Save what
                *saveWhat=Window_attribute2;
                //printf("what=%d\n",Window_attribute2);
                break;
            
            case 4: // Load addr
                *loadAddr=Window_attribute2*32;
                //printf("addr=$%02x00 (%d)\n",loadAddr,Window_attribute2);
                break;
            
            case 0: break;
        }
    }while(button!=1 && button!=2);
    
    Close_window();
    Display_cursor();
    return button==1;
}

int Save_C64_hires(T_IO_Context *context, char *filename, byte saveWhat, byte loadAddr)
{
    int cx,cy,x,y,c1,c2=0,i,pixel,bits,pos=0;
    word numcolors;
    dword cusage[256];
    byte colors[1000],bitmap[8000];
    FILE *file;
    
    for(x=0;x<1000;x++)colors[x]=1; // init colormem to black/white
  
    for(cy=0; cy<25; cy++) // Character line, 25 lines
    {
        for(cx=0; cx<40; cx++) // Character column, 40 columns
        {
            for(i=0;i<256;i++)
                cusage[i]=0;
            
            numcolors=Count_used_colors_area(cusage,cx*8,cy*8,8,8);
            if (numcolors>2)
            {
                Warning_message("More than 2 colors in 8x8 pixels");
                // TODO here we should hilite the offending block
                printf("\nerror at %dx%d (%d colors)\n",cx*8,cy*8,numcolors);
                return 1;
            }
            c1 = 0; c2 = 0;
            for(i=0;i<16;i++)
            {
                if(cusage[i])
                {
                    c2=i;
                    break;
                }
            }
            c1=c2+1;
            for(i=c2;i<16;i++)
            {
                if(cusage[i])
                {
                  c1=i;
                }
            }            
            colors[cx+cy*40]=(c2<<4)|c1;
      
            for(y=0; y<8; y++)
            {
                bits=0;
                for(x=0; x<8; x++)
                {
                    pixel=Get_pixel(context, x+cx*8,y+cy*8);
                    if(pixel>15) 
                    { 
                        Warning_message("Color above 15 used"); 
                        // TODO hilite offending block here too?
                        // or make it smarter with color allocation?
                        // However, the palette is fixed to the 16 first colors
                        return 1;
                    }
                    bits=bits<<1;
                    if (pixel==c2) bits|=1;
                }
                bitmap[pos++]=bits;
                //Write_byte(file,bits&255);
            }
        }
    }
  
    file = fopen(filename,"wb");
  
    if(!file)
    {
        Warning_message("File open failed");
        File_error = 1;
        return 1;
    }
    
    if (loadAddr)
    {
        Write_byte(file,0);
        Write_byte(file,loadAddr);
    }
    if (saveWhat==0 || saveWhat==1)
        Write_bytes(file,bitmap,8000);
    if (saveWhat==0 || saveWhat==2)
        Write_bytes(file,colors,1000);
    
    fclose(file);
    return 0;
}

int Save_C64_multi(T_IO_Context *context, char *filename, byte saveWhat, byte loadAddr)
{
    /* 
    BITS     COLOR INFORMATION COMES FROM
    00     Background color #0 (screen color)
    01     Upper 4 bits of screen memory
    10     Lower 4 bits of screen memory
    11     Color nybble (nybble = 1/2 byte = 4 bits)
    */

    int cx,cy,x,y,c[4]={0,0,0,0},color,lut[16],bits,pixel,pos=0;
    byte bitmap[8000],screen[1000],nybble[1000];
    word numcolors,count;
    dword cusage[256];
    byte i,background=0;
    FILE *file;
    
    numcolors=Count_used_colors(cusage);
  
    count=0;
    for(x=0;x<16;x++)
    {
        //printf("color %d, pixels %d\n",x,cusage[x]);
        if(cusage[x]>count)
        {
            count=cusage[x];
            background=x;
        }
    }
  
    for(cy=0; cy<25; cy++)
    {
        //printf("\ny:%2d ",cy);
        for(cx=0; cx<40; cx++)
        {
            numcolors=Count_used_colors_area(cusage,cx*4,cy*8,4,8);
            if(numcolors>4)
            {
                Warning_message("More than 4 colors in 4x8");
                // TODO hilite offending block
                return 1;
            }
            color=1;
            c[0]=background;
            for(i=0; i<16; i++)
            {
                lut[i]=0;
                if(cusage[i])
                {
                    if(i!=background)
                    {
                        lut[i]=color;
                        c[color]=i;
                        color++;
                    }
                    else
                    {
                        lut[i]=0;
                    }
                }
            }
            // add to screen and nybble
            screen[cx+cy*40]=c[1]<<4|c[2];
            nybble[cx+cy*40]=c[3];
            //printf("%x%x%x ",c[1],c[2],c[3]);
            for(y=0;y<8;y++)
            {
                bits=0;
                for(x=0;x<4;x++)
                {                    
                    pixel=Get_pixel(context, cx*4+x,cy*8+y);
                    if(pixel>15) 
                    { 
                        Warning_message("Color above 15 used"); 
                        // TODO hilite as in hires, you should stay to 
                        // the fixed 16 color palette
                        return 1;
                    }
                    bits=bits<<2;
                    bits|=lut[pixel];
        
                }
                //Write_byte(file,bits&255);
                bitmap[pos++]=bits;
            }
        }
    }
  
    file = fopen(filename,"wb");
    
    if(!file)
    {
        Warning_message("File open failed");
        File_error = 1;
        return 1;
    }

    if (loadAddr)
    {
        Write_byte(file,0);
        Write_byte(file,loadAddr);
    }

    if (saveWhat==0 || saveWhat==1)
        Write_bytes(file,bitmap,8000);
        
    if (saveWhat==0 || saveWhat==2)
        Write_bytes(file,screen,1000);
        
    if (saveWhat==0 || saveWhat==3)
        Write_bytes(file,nybble,1000);
        
    if (saveWhat==0)
        Write_byte(file,background);
    
    fclose(file);
    //printf("\nbg:%d\n",background);
    return 0;
}

void Save_C64(T_IO_Context * context)
{
    char filename[MAX_PATH_CHARACTERS];
    static byte saveWhat=0, loadAddr=0;
    dword numcolors,cusage[256];
    numcolors=Count_used_colors(cusage);
  
    Get_full_filename(filename, context->File_name, context->File_directory);
  
    if (numcolors>16)
    {
        Warning_message("Error: Max 16 colors");
        File_error = 1;
        return;
    }
    if (((context->Width!=320) && (context->Width!=160)) || context->Height!=200)
    {
        Warning_message("must be 320x200 or 160x200");
        File_error = 1;
        return;
    } 
    
    if(!Save_C64_window(&saveWhat,&loadAddr))
    {
        File_error = 1;
        return;
    }
    //printf("saveWhat=%d, loadAddr=%d\n",saveWhat,loadAddr);
    
    if (context->Width==320)
        File_error = Save_C64_hires(context,filename,saveWhat,loadAddr);
    else
        File_error = Save_C64_multi(context,filename,saveWhat,loadAddr);
}


// SCR (Amstrad CPC)

void Test_SCR(__attribute__((unused)) T_IO_Context * context)
{
    // Mmh... not sure what we could test. Any idea ?
    // The palette file can be tested, if it exists and have the right size it's
    // ok. But if it's not there the pixel data may still be valid. And we can't
    // use the filesize as this depends on the screen format.
    
    // An AMSDOS header would be a good indication but in some cases it may not
    // be there
}

void Load_SCR(__attribute__((unused)) T_IO_Context * context)
{
    // The Amstrad CPC screen memory is mapped in a weird mode, somewhere
    // between bitmap and textmode. Basically the only way to decode this is to
    // emulate the video chip and read the bytes as needed...
    // Moreover, the hardware allows the screen to have any size from 8x1 to
    // 800x273 pixels, and there is no indication of that in the file besides
    // its size. It can also use any of the 3 screen modes. Fortunately this
    // last bit of information is stored in the palette file.
    // Oh, and BTW, the picture can be offset, and it's even usual to do it,
    // because letting 128 pixels unused at the beginning of the file make it a
    // lot easier to handle screens using more than 16K of VRam.
    // The pixel encoding change with the video mode so we have to know that
    // before attempting to load anything...
    // As if this wasn't enough, Advanced OCP Art Studio, the reference tool on
    // Amstrad, can use RLE packing when saving files, meaning we also have to
    // handle that.
    
    // All this mess enforces us to load (and unpack if needed) the file to a
    // temporary 32k buffer before actually decoding it.
    
    // 1) Seek for a palette
    // 2) If palette found get screenmode from there, else ask user
    // 3) ask user for screen size (or register values)
    // 4) Load color data from palette (if found)
    // 5) Close palette
    // 6) Open the file
    // 7) Run around the screen to untangle the pixeldata
    // 8) Close the file
}

void Save_SCR(T_IO_Context * context)
{
    // TODO : Add possibility to set R9, R12, R13 values
    // TODO : Add OCP packing support
    // TODO : Add possibility to include AMSDOS header, with proper loading
    // address guessed from r12/r13 values.
    
    unsigned char* output;
    unsigned long outsize;
    unsigned char r1;
    int cpc_mode;
    FILE* file;
    char filename[MAX_PATH_CHARACTERS];

    Get_full_filename(filename, context->File_name, context->File_directory);


    switch(Pixel_ratio)
    {
        case PIXEL_WIDE:
        case PIXEL_WIDE2:
            cpc_mode = 0;
            break;
        case PIXEL_TALL:
        case PIXEL_TALL2:
            cpc_mode = 2;
            break;
        default:
            cpc_mode = 1;
            break;
    }

    output = raw2crtc(context->Width,context->Height,cpc_mode,7,&outsize,&r1,0,0);

    file = fopen(filename,"wb");
    Write_bytes(file, output, outsize);
    fclose(file);

    free (output);
  output = NULL;

    File_error = 0;
}
