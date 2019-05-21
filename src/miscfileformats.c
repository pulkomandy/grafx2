/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2011 Pawel Góralski
    Copyright 2009 Petter Lindquist
    Copyright 2008 Yves Rizoud
    Copyright 2008 Franck Charlet
    Copyright 2007-2011 Adrien Destugues
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

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include <stdio.h>
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#define strdup _strdup
#endif

#include <zlib.h>

#include "engine.h"
#include "errors.h"
#include "global.h"
#include "io.h"
#include "libraw2crtc.h"
#include "loadsave.h"
#include "misc.h"
#include "screen.h"
#include "struct.h"
#include "windows.h"
#include "oldies.h"
#include "pages.h"
#include "keycodes.h"
#include "input.h"
#include "help.h"
#include "fileformats.h"

extern char Program_version[]; // generated in pversion.c
extern const char SVN_revision[]; // generated in version.c

//////////////////////////////////// PAL ////////////////////////////////////
//

// -- Test wether a file is in PAL format --------------------------------
void Test_PAL(T_IO_Context * context, FILE * file)
{
  char buffer[32];
  long file_size;

  (void)context;
  File_error = 1;

  file_size = File_length_file(file);
  // First check for GrafX2 legacy palette format. The simplest one, 768 bytes
  // of RGB data. It is a raw dump of the T_Palette structure. There is no
  // header at all, so we check for the file size.
  if (file_size == sizeof(T_Palette))
    File_error = 0;
  else if (file_size > 8)
  {
    // Bigger (or smaller ?) files may be in other formats. These have an
    // header, so look for it.
    if (!Read_bytes(file, buffer, 8))
      return;
    if (strncmp(buffer,"JASC-PAL",8) == 0)
    {
      // JASC file format, used by Paint Shop Pro and GIMP. This is also the
      // one used for saving, as it brings greater interoperability.
      File_error = 0;
    }
    else if(strncmp(buffer,"RIFF", 4) == 0)
    {
      // Microsoft RIFF file
      // This is a data container (similar to IFF). We only check the first
      // chunk header, and give up if that's not a palette.
      fseek(file, 8, SEEK_SET);
      if (!Read_bytes(file, buffer, 8))
        return;
      if (strncmp(buffer, "PAL data", 8) == 0)
      {
        File_error = 0;
      }
    }
  }
}

/// Test for GPL (Gimp Palette) file format
void Test_GPL(T_IO_Context * context, FILE * file)
{
  char buffer[16];
  long file_size;

  (void)context;
  File_error = 1;

  file_size = File_length_file(file);
  if (file_size > 33) {
    // minimum header length == 33
    // "GIMP Palette" == 12
    if (!Read_bytes(file, buffer, 12))
      return;
    if (strncmp(buffer,"GIMP Palette",12) == 0)
      File_error = 0;
  }
}

/// skip the padding before a space-padded field.
static int skip_padding(FILE *file, int max_chars)
{
  byte b;
  int chars_read = 0;

  do {
    if (chars_read == max_chars)
      return chars_read; // eof
    if (!Read_byte(file, &b))
      return chars_read;
    chars_read++;
  } while (b == ' ');

  fseek(file, -1, SEEK_CUR);
  return chars_read;
}

/// Load GPL (Gimp Palette) file format
void Load_GPL(T_IO_Context * context)
{
  FILE *file;
  char buffer[256];

  File_error = 1;

  file = Open_file_read(context);
  if (file == NULL)
    return;

  if (!Read_byte_line(file, buffer, sizeof(buffer)))
    return;

  if (memcmp(buffer,"GIMP Palette",12) == 0)
  {
    int i, r, g, b, columns;
    size_t len;
    // Name: xxxxx
    if (!Read_byte_line(file, buffer, sizeof(buffer)))
      return;
    len = strlen(buffer);
    while (len > 0)
    {
      len--;
      if (buffer[len] == '\r' || buffer[len] == '\n')
        buffer[len] = '\0';
    }
    GFX2_Log(GFX2_DEBUG, "GPL %s\n", buffer);
    if (0 == memcmp(buffer, "Name: ", 6))
      snprintf(context->Comment, sizeof(context->Comment), "GPL: %s", buffer + 6);

    // Columns: 16
    if (fscanf(file, "Columns: %d", &columns) != 1)
      return;
    Read_byte_line(file, buffer, sizeof(buffer));
    /// @todo set grafx2 columns setting to match.
    // #<newline>
    if (!Read_byte_line(file, buffer, sizeof(buffer)))
      return;

    for (i = 0; i < 256; i++)
    {
      skip_padding(file, 32);
      if (fscanf(file, "%d", &r) != 1)
        break;
      skip_padding(file, 32);
      if (fscanf(file, "%d", &g) != 1)
        break;
      skip_padding(file, 32);
      if (fscanf(file, "%d\t", &b) != 1)
        break;
      if (!Read_byte_line(file, buffer, sizeof(buffer)))
        break;
      len = strlen(buffer);
      while (len > 1)
      {
        len--;
        if (buffer[len] == '\r' || buffer[len] == '\n')
          buffer[len] = '\0';
      }
      /// @todo analyze color names to build shade table

      GFX2_Log(GFX2_DEBUG, "GPL: %3d: RGB(%3d,%3d,%3d) %s\n", i, r,g,b, buffer);
      context->Palette[i].R = r;
      context->Palette[i].G = g;
      context->Palette[i].B = b;
    }
    if (i > 0)  // at least one color was read
      File_error = 0;
  }
  else
    File_error = 2;

  // close the file
  fclose(file);
}


/// Save GPL (Gimp Palette) file format
void
Save_GPL (T_IO_Context * context)
{
  // Gimp is a unix program, so use Unix file endings (LF aka '\n')
  FILE *file;

  file = Open_file_write(context);

  if (file != NULL )
  {
    int i;

    File_error = 0;
    fprintf (file, "GIMP Palette\n");
    fprintf (file, "Name: %s\n", context->File_name);
    // TODO: use actual columns value
    fprintf (file, "Columns: %d\n#\n", 16);

    for (i = 0; i < 256 && File_error==0; i++)
    {
      // TODO: build names from shade table data
      if (fprintf(file,"%d %d %d\tUntitled\n",context->Palette[i].R, context->Palette[i].G, context->Palette[i].B) <= 0)
        File_error=1;
    }
    fclose(file);

    if (File_error)
      Remove_file(context);
  }
  else
  {
    // unable to open output file, nothing saved.
    File_error=1;
  }
}



// -- Lire un fichier au format PAL -----------------------------------------
void Load_PAL(T_IO_Context * context)
{
  FILE *file;              // Fichier du fichier

  File_error=0;

  // Ouverture du fichier
  if ((file=Open_file_read(context)))
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
      }
      else
        File_error = 2;
    } else {
      char buffer[16];
      if (!Read_bytes(file, buffer, 8))
      {
        File_error = 2;
        fclose(file);
        return;
      }
      buffer[8] = '\0';
      if (strncmp(buffer,"JASC-PAL",8) == 0)
      {
        int i, n, r, g, b;
        i = fscanf(file, "%d",&n);
        if(i != 1 || n != 100)
        {
          File_error = 2;
          fclose(file);
          return;
        }
        // Read color count
        if (fscanf(file, "%d",&n) == 1)
        {
          for (i = 0; i < n; i++)
          {
            if (fscanf(file, "%d %d %d",&r, &g, &b) == 3)
            {
              context->Palette[i].R = r;
              context->Palette[i].G = g;
              context->Palette[i].B = b;
            }
            else
              File_error = 2;
          }
        }
        else
          File_error = 2;
      }
      else if(strncmp(buffer, "RIFF", 4) == 0)
      {
        // Microsoft RIFF format.
        fseek(file, 8, SEEK_SET);
        Read_bytes(file, buffer, 8);
        if (strncmp(buffer, "PAL data", 8) == 0)
        {
          word color_count;
          word i = 0;

          fseek(file, 22, SEEK_SET);
          if (!Read_word_le(file, &color_count))
            File_error = 2;
          else
            for(i = 0; i < color_count && File_error == 0; i++)
            {
              byte colors[4];
              if (!Read_bytes(file, colors, 4))
                File_error = 2;
              context->Palette[i].R = colors[0];
              context->Palette[i].G = colors[1];
              context->Palette[i].B = colors[2];
            }
        } else File_error = 2;
      } else
        File_error = 2;
    }

    fclose(file);
  }
  else
    // Si on n'a pas réussi à ouvrir le fichier, alors il y a eu une erreur
    File_error=1;
}


// -- Sauver un fichier au format PAL ---------------------------------------
void Save_PAL(T_IO_Context * context)
{
  // JASC-PAL is a DOS/Windows format, so use CRLF line endings "\r\n"
  FILE *file;

  File_error=0;

  // Open output file
  if ((file=Open_file_write(context)) != NULL)
  {
    int i;

    setvbuf(file, NULL, _IOFBF, 64*1024);

    if (fputs("JASC-PAL\r\n0100\r\n256\r\n", file)==EOF)
      File_error=1;
    for (i = 0; i < 256 && File_error==0; i++)
    {
      if (fprintf(file,"%d %d %d\r\n",context->Palette[i].R, context->Palette[i].G, context->Palette[i].B) <= 0)
        File_error=1;
    }

    fclose(file);

    if (File_error)
      Remove_file(context);
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
void Test_PKM(T_IO_Context * context, FILE * file)
{
  T_PKM_Header header;

  (void)context;
  File_error=1;

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
}


// -- Lire un fichier au format PKM -----------------------------------------
void Load_PKM(T_IO_Context * context)
{
  FILE *file;             // Fichier du fichier
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

  File_error=0;

  if ((file=Open_file_read(context)))
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
                    else
                      GFX2_Log(GFX2_DEBUG, "PKM original screen %dx%d\n", (int)Original_screen_X, (int)Original_screen_Y);
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

          Compteur_de_donnees_packees=0;
          Compteur_de_pixels=0;
          // Header size is 780
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
  FILE *file;
  T_PKM_Header header;
  dword Compteur_de_pixels;
  dword image_size;
  word  repetitions;
  byte  last_color;
  byte  pixel_value;
  size_t comment_size;



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
  if (comment_size > 255) comment_size = 255;
  if (comment_size)
    header.Jump+=(word)comment_size+2;


  File_error=0;

  // Ouverture du fichier
  if ((file=Open_file_write(context)))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);

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

      // Ecriture du commentaire
      // (Compteur_de_pixels est utilisé ici comme simple index de comptage)
      if (comment_size > 0)
      {
        Write_one_byte(file,0);
        Write_one_byte(file,(byte)comment_size);
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
    Remove_file(context);
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

void Test_CEL(T_IO_Context * context, FILE * file)
{
  int  size;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  int file_size;

  (void)context;
  File_error=0;

  file_size = File_length_file(file);
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
}


// -- Lire un fichier au format CEL -----------------------------------------

void Load_CEL(T_IO_Context * context)
{
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  short x_pos;
  short y_pos;
  byte  last_byte=0;
  long  file_size;
  const long int header_size = 4;

  File_error=0;
  if ((file=Open_file_read(context)))
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
  FILE *file;
  T_CEL_Header1 header1;
  T_CEL_Header2 header2;
  short x_pos;
  short y_pos;
  byte  last_byte=0;
  dword color_usage[256]; // Table d'utilisation de couleurs


  // On commence par compter l'utilisation de chaque couleurs
  Count_used_colors(color_usage);

  File_error=0;
  if ((file=Open_file_write(context)))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);

    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (x_pos=16;((x_pos<256) && (!color_usage[x_pos]));x_pos++);

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
        for (y_pos=0;((y_pos<header2.Height) && (!File_error));y_pos++)
          for (x_pos=0;((x_pos<header2.Width) && (!File_error));x_pos++)
            Write_one_byte(file,Get_pixel(context, x_pos+header2.X_offset,y_pos+header2.Y_offset));
      }
      else
        File_error=1;
      fclose(file);
    }

    if (File_error)
      Remove_file(context);
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

void Test_KCF(T_IO_Context * context, FILE * file)
{
  T_KCF_Header header1;
  T_CEL_Header2 header2;
  int pal_index;
  int color_index;

  (void)context;
  File_error=0;
    if (File_length_file(file)==320)
    {
      for (pal_index=0;pal_index<10 && !File_error;pal_index++)
        for (color_index=0;color_index<16 && !File_error;color_index++)
          if (!Read_byte(file,&header1.Palette[pal_index].color[color_index].Byte1) ||
              !Read_byte(file,&header1.Palette[pal_index].color[color_index].Byte2))
            File_error=1;
      // On vérifie une propriété de la structure de palette:
      for (pal_index=0;pal_index<10;pal_index++)
        for (color_index=0;color_index<16;color_index++)
          if ((header1.Palette[pal_index].color[color_index].Byte2>>4)!=0)
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
}


// -- Lire un fichier au format KCF -----------------------------------------

void Load_KCF(T_IO_Context * context)
{
  FILE *file;
  T_KCF_Header header1;
  T_CEL_Header2 header2;
  byte bytes[3];
  int pal_index;
  int color_index;
  int index;
  long  file_size;


  File_error=0;
  if ((file=Open_file_read(context)))
  {
    file_size=File_length_file(file);
    if (file_size==320)
    {
      // Fichier KCF à l'ancien format
      for (pal_index=0;pal_index<10 && !File_error;pal_index++)
        for (color_index=0;color_index<16 && !File_error;color_index++)
          if (!Read_byte(file,&header1.Palette[pal_index].color[color_index].Byte1) ||
              !Read_byte(file,&header1.Palette[pal_index].color[color_index].Byte2))
            File_error=1;

      if (!File_error)
      {
        // Pre_load(context, ?); // Pas possible... pas d'image...

        if (Config.Clear_palette)
          memset(context->Palette,0,sizeof(T_Palette));

        // Chargement de la palette
        for (pal_index=0;pal_index<10;pal_index++)
          for (color_index=0;color_index<16;color_index++)
          {
            index=16+(pal_index*16)+color_index;
            context->Palette[index].R=((header1.Palette[pal_index].color[color_index].Byte1 >> 4) << 4);
            context->Palette[index].B=((header1.Palette[pal_index].color[color_index].Byte1 & 15) << 4);
            context->Palette[index].G=((header1.Palette[pal_index].color[color_index].Byte2 & 15) << 4);
          }

        for (index=0;index<16;index++)
        {
          context->Palette[index].R=context->Palette[index+16].R;
          context->Palette[index].G=context->Palette[index+16].G;
          context->Palette[index].B=context->Palette[index+16].B;
        }

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
  FILE *file;
  T_KCF_Header header1;
  T_CEL_Header2 header2;
  byte bytes[3];
  int pal_index;
  int color_index;
  int index;
  dword color_usage[256]; // Table d'utilisation de couleurs

  // On commence par compter l'utilisation de chaque couleurs
  Count_used_colors(color_usage);

  File_error=0;
  if ((file=Open_file_write(context)))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);
    // Sauvegarde de la palette

    // On regarde si des couleurs >16 sont utilisées dans l'image
    for (index=16;((index<256) && (!color_usage[index]));index++);

    if (index==256)
    {
      // Cas d'une image 16 couleurs (écriture à l'ancien format)

      for (pal_index=0;pal_index<10;pal_index++)
        for (color_index=0;color_index<16;color_index++)
        {
          index=16+(pal_index*16)+color_index;
          header1.Palette[pal_index].color[color_index].Byte1=((context->Palette[index].R>>4)<<4) | (context->Palette[index].B>>4);
          header1.Palette[pal_index].color[color_index].Byte2=context->Palette[index].G>>4;
        }

      // Write all
      for (pal_index=0;pal_index<10 && !File_error;pal_index++)
        for (color_index=0;color_index<16 && !File_error;color_index++)
          if (!Write_byte(file,header1.Palette[pal_index].color[color_index].Byte1) ||
              !Write_byte(file,header1.Palette[pal_index].color[color_index].Byte2))
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
      Remove_file(context);
  }
  else
    File_error=1;
}


/**
 * @defgroup atarist Atari ST picture formats
 * @ingroup loadsaveformats
 *
 * Support for Atari ST picture formats. The Atari ST has
 * 3 video modes :
 * - low res : 320x200 16 colors
 * - med res : 640x200 4 colors
 * - high res : 640x400 monochrome
 *
 * Supported formats :
 * - PI1 : Degas
 * - PC1 : Degas elite compressed
 * - NEO : Neochrome
 *
 * @{
 */
//////////////////////////////////// PI1 ////////////////////////////////////

//// DECODAGE d'une partie d'IMAGE ////

void PI1_8b_to_16p(const byte * src,byte * dest)
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

void PI2_8b_to_16p(const byte * src,byte * dest)
{
  int  i;           // index du pixel à calculer
  word mask;      // Masque de decodage
  word w0,w1;

  w0=(((word)src[0])<<8) | src[1];
  w1=(((word)src[2])<<8) | src[3];
  mask=0x8000;
  for (i = 0; i < 16; i++)
  {
    dest[i] = ((w0 & mask) ? 0x01 : 0) | ((w1 & mask) ? 0x02 : 0);
    mask >>= 1;
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

static void PI1_decode_palette(const byte * src, T_Components * palette)
{
  int i;  // Numéro de la couleur traitée
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  //    Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321

  for (i=0;i<16;i++)
  {
    w = (word)src[0] << 8 | (word)src[1];
    src += 2;

    palette[i].R = (((w & 0x0700)>>7) | ((w & 0x0800) >> 11)) * 0x11 ;
    palette[i].G = (((w & 0x0070)>>3) | ((w & 0x0080) >> 7)) * 0x11 ;
    palette[i].B = (((w & 0x0007)<<1) | ((w & 0x0008) >> 3)) * 0x11 ;
  }
}

//// CODAGE de la PALETTE ////

void PI1_code_palette(const T_Components * palette, byte * dest)
{
  int i;  // Numéro de la couleur traitée
  word w; // Word contenant le code

  // Schéma d'un word =
  //
  // Low        High
  // VVVV RRRR | 0000 BBBB
  // 0321 0321 |      0321

  for (i=0;i<16;i++)
  {
    w  = ((word)(palette[i].R & 0xe0) << 3) | ((word)(palette[i].R & 0x10) << 7);
    w |= ((word)(palette[i].G & 0xe0) >> 1) | ((word)(palette[i].G & 0x10) << 3);
    w |= ((word)(palette[i].B & 0xe0) >> 5) | ((word)(palette[i].B & 0x10) >> 1);

    *dest++ = (w >> 8);
    *dest++ = (w & 0xff);
  }
}

/// Load color cycling data from a PI1 or PC1 image (Degas Elite format)
static void PI1_load_ranges(T_IO_Context * context, const byte * buffer, int size)
{
  int range;

  if (buffer==NULL || size<32)
    return;

  for (range=0; range < 4; range ++)
  {
    word min_col, max_col, direction, delay;

    min_col   = (buffer[size - 32 + range*2 +  0] << 8) | buffer[size - 32 + range*2 +  1];
    max_col   = (buffer[size - 32 + range*2 +  8] << 8) | buffer[size - 32 + range*2 +  9];
    direction = (buffer[size - 32 + range*2 + 16] << 8) | buffer[size - 32 + range*2 + 17];
    delay     = (buffer[size - 32 + range*2 + 24] << 8) | buffer[size - 32 + range*2 + 25];

    if (max_col < min_col)
      SWAP_WORDS(min_col,max_col)

    GFX2_Log(GFX2_DEBUG, "Degas Color cycling : [#%d:#%d] direction=%d delay=%d\n", min_col, max_col, direction, delay);
    // Sanity checks
    if (min_col < 256 && max_col < 256 && direction < 3 && (direction == 1 || delay < 128))
    {
      int speed = 1;
      if (delay < 128)
        speed = 210/(128-delay);
      // Grafx2's slider has a limit of COLOR_CYCLING_SPEED_MAX
      if (speed > COLOR_CYCLING_SPEED_MAX)
        speed = COLOR_CYCLING_SPEED_MAX;
      context->Cycle_range[context->Color_cycles].Start=min_col;
      context->Cycle_range[context->Color_cycles].End=max_col;
      context->Cycle_range[context->Color_cycles].Inverse= (direction==0);
      context->Cycle_range[context->Color_cycles].Speed=direction == 1 ? 0 : speed;
      context->Color_cycles++;
    }
  }
}

/// Saves color ranges from a PI1 or PC1 image (Degas Elite format)
void PI1_save_ranges(T_IO_Context * context, byte * buffer, int size)
{
  // empty by default
  memset(buffer+size - 32, 0, 32);
  if (context->Color_cycles)
  {
    int i; // index in context->Cycle_range[] : < context->Color_cycles
    int saved_range; // index in resulting buffer : < 4

    for (i=0, saved_range=0; i<context->Color_cycles && saved_range<4; i++)
    {
      if (context->Cycle_range[i].Start < 16 && context->Cycle_range[i].End < 16)
      {
        int speed;
        if (context->Cycle_range[i].Speed == 0)
          speed = 0;
        else if (context->Cycle_range[i].Speed == 1)
          // has to "round" manually to closest valid number for this format
          speed = 1;
        else
          speed = 128 - 210 / context->Cycle_range[i].Speed;

        buffer[size - 32 + saved_range*2 +  1] = context->Cycle_range[i].Start;
        buffer[size - 32 + saved_range*2 +  9] = context->Cycle_range[i].End;
        buffer[size - 32 + saved_range*2 + 17] = (context->Cycle_range[i].Speed == 0) ? 1 : (context->Cycle_range[i].Inverse ? 0 : 2);
        buffer[size - 32 + saved_range*2 + 25] = speed;

        saved_range ++;
      }
    }
  }
}

/// Test for Degas file format
void Test_PI1(T_IO_Context * context, FILE * file)
{
  unsigned long size;              // Taille du fichier
  word resolution;                 // Résolution de l'image

  (void)context;
  File_error=1;

  if (!Read_word_be(file,&resolution))
    return;

  size = File_length_file(file);
  if ((size==32034) || (size==32066)) // size check
  {
    if (resolution < 3)
      File_error=0;
  }
}


/// Load Degas file format
void Load_PI1(T_IO_Context * context)
{
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  word resolution;
  word width, height;
  FILE *file;
  word x_pos,y_pos;
  byte buffer[160];
  byte * ptr;
  byte pixels[320];
  byte bpp;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;

  if (!Read_word_be(file, &resolution))
    return;
  GFX2_Log(GFX2_DEBUG, "Degas UnCompressed. Resolution = %04x\n", resolution);
  // Read palette
  if (!Read_bytes(file, buffer, 32))
  {
    fclose(file);
    return;
  }
  if (Config.Clear_palette)
    memset(context->Palette,0,sizeof(T_Palette));
  PI1_decode_palette(buffer, context->Palette);

  switch (resolution)
  {
    case 0:  // Low Res
      width = 320;
      height = 200;
      bpp = 4;
      break;
    case 1:  // Med Res
      width = 640;
      height = 200;
      bpp = 2;
      ratio = PIXEL_TALL;
      break;
    case 2:  // High Res
      width = 640;
      height = 400;
      bpp = 1;
      break;
    default:
      fclose(file);
      return;
  }
  Pre_load(context, width, height, File_length_file(file),FORMAT_PI1,ratio, bpp);

  for (y_pos=0;y_pos<height;y_pos++)
  {
    if (!Read_bytes(file, buffer, (resolution == 2) ? 80 : 160))
    {
      fclose(file);
      return;
    }
    ptr = buffer;
    for (x_pos=0; x_pos < width;)
    {
      int i;
      switch (resolution)
      {
        case 0:
          PI1_8b_to_16p(ptr, pixels);
          ptr += 8;
          break;
        case 1:
          PI2_8b_to_16p(ptr, pixels);
          ptr += 4;
          break;
        case 2:
          for (i = 0; i < 8; i++)
            pixels[i] = (ptr[0] & (0x80 >> i)) ? 1 : 0;
          for (; i < 16; i++)
            pixels[i] = (ptr[1] & (0x80 >> (i - 8))) ? 1 : 0;
          ptr += 2;
      }
      for (i = 0; i < 16; i++)
        Set_pixel(context, x_pos++, y_pos, pixels[i]);
    }
  }
  // load color cycling information
  if (Read_bytes(file, buffer, 32))
  {
    PI1_load_ranges(context, buffer, 32);
  }
  fclose(file);
  File_error = 0;
}


// -- Sauver un fichier au format PI1 ---------------------------------------
void Save_PI1(T_IO_Context * context)
{
  FILE *file;
  short x_pos,y_pos;
  byte * buffer;
  byte * ptr;
  byte pixels[320];

  File_error=0;
  // Ouverture du fichier
  if ((file=Open_file_write(context)))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);

    // allocation d'un buffer mémoire
    buffer=(byte *)malloc(32034);
    // Codage de la résolution
    buffer[0]=0x00;
    buffer[1]=0x00;
    // Codage de la palette
    PI1_code_palette(context->Palette, buffer+2);
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

    if (Write_bytes(file,buffer,32034))
    {
      if (context->Color_cycles)
      {
        PI1_save_ranges(context, buffer, 32);
        if (!Write_bytes(file,buffer,32))
          File_error=1;
      }
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      Remove_file(context);
      File_error=1;
    }
    // Libération du buffer mémoire
    free(buffer);
    buffer = NULL;
  }
  else
  {
    fclose(file);
    Remove_file(context);
    File_error=1;
  }
}


//////////////////////////////////// PC1 ////////////////////////////////////

/// uncompress degas elite compressed stream (PACKBITS)
/// @return 1 for success
/// @return 0 for failure
static int PC1_uncompress_packbits_file(FILE *f, byte * dest)
{
  int id = 0;
  unsigned count;
  byte code, value;

  while (id < 32000)
  {
    if (!Read_byte(f, &code))
      return 0;

    if (code & 0x80)
    {
      /// Code is negative :
      /// Repeat (1-code) times next byte
      count = 257 - code;
      if (id + count > 32000)
        return 0;
      if (!Read_byte(f, &value))
        return 0;
      while (count-- > 0)
        dest[id++] = value;
    }
    else
    {
      /// Code is positive :
      /// Copy (code+1) bytes
      count = code + 1;
      if (id + count > 32000)
        return 0;
      if (!Read_bytes(f, dest + id, count))
        return 0;
      id += count;
    }
  }
  return 1;
}

//// COMPRESSION d'un buffer selon la méthode PACKBITS ////

static void PC1_compress_packbits(byte * src,byte * dest,int source_size,int * dest_size)
{

  *dest_size = 0;
  while (source_size > 0)
  {
    int is = 0; // index dans la source
    int id = 0; // index dans la destination
    int ir; // index de   la répétition
    int n;  // Taille des séquences
    int repet; // "Il y a répétition"

    while(is<40)
    {
      // On recherche le 1er endroit où il y a répétition d'au moins 3 valeurs
      // identiques

      repet=0;
      for (ir=is;ir<40-2;ir++)
      {
        if ((src[ir]==src[ir+1]) && (src[ir+1]==src[ir+2]))
        {
          repet=1;
          break;
        }
      }

      // On code la partie sans répétitions
      if (!repet || ir!=is)
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
        for (ir+=3;ir<40;ir++)
        {
          if (src[ir]!=src[is])
            break;
        }
        n=(ir-is);
        dest[id++]=257-n;
        dest[id++]=src[is];
        is=ir;
      }
    }
    // On renseigne la taille du buffer compressé
    *dest_size+=id;
    // Move for next 40-byte block
    src += 40;
    dest += id;
    source_size -= 40;
  }
}

//// DECODAGE d'une partie d'IMAGE ////

// Transformation de 4 plans de bits en 1 ligne de pixels

static void PC1_4bp_to_1line(byte * src0,byte * src1,byte * src2,byte * src3,byte * dest)
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

static void PC1_1line_to_4bp(byte * src,byte * dst0,byte * dst1,byte * dst2,byte * dst3)
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


/// Test for Degas Elite Compressed format
void Test_PC1(T_IO_Context * context, FILE * file)
{
  int  size;              // Taille du fichier
  word resolution;        // Résolution de l'image

  (void)context;
  File_error=1;

  size = File_length_file(file);
  if (!Read_word_be(file,&resolution))
    return;

  if ((size <= 32066) && (resolution & 0x8000))
  {
    if ((resolution & 0x7fff) < 3)
      File_error=0;
  }
}


/// Load Degas Elite compressed files
void Load_PC1(T_IO_Context * context)
{
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  unsigned long size;
  word width, height;
  byte bpp;
  FILE *file;
  word x_pos,y_pos;
  byte buffer[32];
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[640];
  word resolution;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  size = File_length_file(file);

  if (!Read_word_be(file, &resolution))
    return;
  GFX2_Log(GFX2_DEBUG, "Degas Elite Compressed. Resolution = %04x\n", resolution);
  // Read palette
  if (!Read_bytes(file, buffer, 32))
  {
    fclose(file);
    return;
  }
  if (Config.Clear_palette)
    memset(context->Palette,0,sizeof(T_Palette));
  PI1_decode_palette(buffer, context->Palette);

  switch (resolution)
  {
    case 0x8000:  // Low Res
      width = 320;
      height = 200;
      bpp = 4;
      break;
    case 0x8001:  // Med Res
      width = 640;
      height = 200;
      bpp = 2;
      ratio = PIXEL_TALL;
      break;
    case 0x8002:  // High Res
      width = 640;
      height = 400;
      bpp = 1;
      break;
    default:
      fclose(file);
      return;
  }

  bufferdecomp=(byte *)malloc(32000);
  if (bufferdecomp == NULL)
  {
    fclose(file);
    return;
  }

  // Initialisation de la preview
  Pre_load(context, width, height, size, FORMAT_PC1, ratio, bpp);

  //PC1_uncompress_packbits(buffercomp, bufferdecomp);
  if (!PC1_uncompress_packbits_file(file, bufferdecomp))
  {
    GFX2_Log(GFX2_INFO, "PC1_uncompress_packbits_file() failed\n");
    free(bufferdecomp);
    fclose(file);
    return;
  }

  // Décodage de l'image
  ptr=bufferdecomp;
  for (y_pos = 0; y_pos < height; y_pos++)
  {
    // Décodage de la scanline
    switch (resolution)
    {
      case 0x8000:  // Low Res
        PC1_4bp_to_1line(ptr,ptr+40,ptr+80,ptr+120,pixels);
        ptr+=160;
        break;
      case 0x8001:  // Med Res
        x_pos = 0;
        while (x_pos < width)
        {
          int i;
          for (i = 7; i >= 0; i--, x_pos++)
            pixels[x_pos] =  ((ptr[0] >> i) & 1)
                            | (((ptr[80] >> i) << 1) & 2);
          ptr++;
        }
        ptr += 80;
        break;
      case 0x8002:  // High Res
        x_pos = 0;
        while (x_pos < width)
        {
          int i;
          for (i = 7; i >= 0; i--)
            pixels[x_pos++] = (*ptr >> i) & 1;
          ptr++;
        }
    }
    for (x_pos=0;x_pos<width;x_pos++)
      Set_pixel(context, x_pos, y_pos, pixels[x_pos]);
  }
  // Try to load color cycling information
  GFX2_Log(GFX2_DEBUG, "remaining bytes = %ld\n", size - ftell(file));
  if (Read_bytes(file, buffer, 32))
  {
    PI1_load_ranges(context, buffer, 32);
  }
  free(bufferdecomp);
  fclose(file);
  File_error = 0;
}


// -- Sauver un fichier au format PC1 ---------------------------------------
void Save_PC1(T_IO_Context * context)
{
  FILE *file;
  int   size;
  short x_pos,y_pos;
  byte * buffercomp;
  byte * bufferdecomp;
  byte * ptr;
  byte pixels[320];

  File_error=0;
  // Ouverture du fichier
  if ((file=Open_file_write(context)))
  {
    setvbuf(file, NULL, _IOFBF, 64*1024);

    // Allocation des buffers mémoire
    bufferdecomp=(byte *)malloc(32000);
    buffercomp  =(byte *)malloc(64066);
    // Codage de la résolution
    buffercomp[0]=0x80;
    buffercomp[1]=0x00;
    // Codage de la palette
    PI1_code_palette(context->Palette, buffercomp+2);
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
    size += 34;
    size += 32;
    PI1_save_ranges(context, buffercomp,size);

    if (Write_bytes(file,buffercomp,size))
    {
      fclose(file);
    }
    else // Error d'écriture (disque plein ou protégé)
    {
      fclose(file);
      Remove_file(context);
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
    Remove_file(context);
    File_error=1;
  }
}


//////////////////////////////////// NEO ////////////////////////////////////
/**
NeoChrome Format :
<pre>
1 word          flag word [always 0]
1 word          resolution [0 = low res, 1 = medium res, 2 = high res]
16 words        palette
12 bytes        filename [usually "        .   "]
1 word          color animation limits.  High bit (bit 15) set if color
                animation data is valid.  Low byte contains color animation
                limits (4 most significant bits are left/lower limit,
                4 least significant bits are right/upper limit).
1 word          color animation speed and direction.  High bit (bit 15) set
                if animation is on.  Low order byte is # vblanks per step.
                If negative, scroll is left (decreasing).  Number of vblanks
                between cycles is |x| - 1
1 word          # of color steps (as defined in previous word) to display
                picture before going to the next.  (For use in slide shows)
1 word          image X offset [unused, always 0]
1 word          image Y offset [unused, always 0]
1 word          image width [unused, always 320]
1 word          image height [unused, always 200]
33 words        reserved for future expansion
32000 bytes     pixel data
</pre>

Dali           *.SD0 (ST low resolution)
               *.SD1 (ST medium resolution)
               *.SD2 (ST high resolution)
        
Files do not seem to have any resolution or bit plane info stored in them. The file
extension seems to be the only way to determine the contents.
        
1 long         file id? [always 0]
16 words       palette
92 bytes       reserved? [usually 0]
*/
void Test_NEO(T_IO_Context * context, FILE * file)
{
  word flag;
  word resolution;   // Atari ST resolution

  (void)context;
  File_error=1;

  if (File_length_file(file) != 32128)
    return;

  if (!Read_word_be(file,&flag))
    return;
  // Flag word : always 0
  if (flag != 0)
    return;

  if (!Read_word_be(file,&resolution))
    return;
  // 0 = STlow, 1 = STmed, 2 = SThigh
  if (resolution==0 || resolution==1 || resolution==2)
    File_error = 0;
}


/// Load Neochrome file format
void Load_NEO(T_IO_Context * context)
{
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  word flag;
  word resolution;   // Atari ST resolution
  word width, height;
  byte bpp;
  word color_cycling_range, color_cycling_delay;
  word display_time;
  word image_width, image_height, image_X_pos, image_Y_pos;
  FILE *file;
  word x_pos,y_pos;
  byte * ptr;
  byte buffer[160];
  byte pixels[16];

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;

  if (!Read_word_be(file,&flag))
    goto error;
  // Flag word : always 0
  if (flag != 0)
    goto error;

  if (!Read_word_be(file,&resolution))
    goto error;

  switch (resolution)
  {
  case 0:
    width = 320;
    height = 200;
    bpp = 4;
    break;
  case 1:
    width = 640;
    height = 200;
    bpp = 2;
    ratio = PIXEL_TALL;
    break;
  case 2:
    width = 640;
    height = 400;
    bpp = 1;
    break;
  default:
    goto error;
  }

  Pre_load(context, width, height, File_length_file(file), FORMAT_NEO, ratio, bpp);

  if (!Read_bytes(file,buffer,32))
    goto error;

  // Initialisation de la palette
  if (Config.Clear_palette)
    memset(context->Palette, 0, sizeof(T_Palette));
  PI1_decode_palette(buffer, context->Palette);

  if (!Read_bytes(file, buffer, 12))
    goto error;
  buffer[12] = '\0';
  GFX2_Log(GFX2_DEBUG, "NEO resolution %u name=\"%s\"\n", resolution, (char *)buffer);
  if (!Read_word_be(file, &color_cycling_range)
     || !Read_word_be(file, &color_cycling_delay)
     || !Read_word_be(file, &display_time))
    goto error;
  GFX2_Log(GFX2_DEBUG, "  Color cycling : %04x %04x. Time to show %u\n", color_cycling_range, color_cycling_delay, display_time);
  if (color_cycling_range & 0x8000)
  {
    context->Cycle_range[context->Color_cycles].Start = (color_cycling_range & 0x00f0) >> 4;
    context->Cycle_range[context->Color_cycles].End = (color_cycling_range & 0x000f);
    if (color_cycling_delay & 0x8000)
    {
      // color cycling on
      color_cycling_delay &= 0xff;
      if (color_cycling_delay & 0x0080)
      {
        context->Cycle_range[context->Color_cycles].Inverse = 1;
        color_cycling_delay = 256 - color_cycling_delay;
      }
      else
        context->Cycle_range[context->Color_cycles].Inverse = 0;
      // Speed resolution is 0.2856Hz
      // NEO color_cycling_delay is in 50Hz VBL
      // Speed = (50/delay) / 0.2856 = 175 / delay
      if (color_cycling_delay != 0)
        context->Cycle_range[context->Color_cycles].Speed = 175 / color_cycling_delay;
      else
        context->Cycle_range[context->Color_cycles].Speed = COLOR_CYCLING_SPEED_MAX; // fastest
      if (context->Cycle_range[context->Color_cycles].Speed > COLOR_CYCLING_SPEED_MAX)
        context->Cycle_range[context->Color_cycles].Speed = COLOR_CYCLING_SPEED_MAX;
    }
    else
      context->Cycle_range[context->Color_cycles].Speed = 0;  // cycling off
    context->Color_cycles++;
  }

  if (!Read_word_be(file, &image_X_pos) || !Read_word_be(file, &image_Y_pos)
      || !Read_word_be(file, &image_width) || !Read_word_be(file, &image_height))
    goto error;
  GFX2_Log(GFX2_DEBUG, "  pos (%u,%u) size %ux%u\n", image_X_pos, image_Y_pos, image_width, image_height);
  if (!Read_bytes(file, buffer, 128-4-32-12-6-8))
    goto error;
  GFX2_LogHexDump(GFX2_DEBUG, "NEO ", buffer, 0, 128-4-32-12-6-8);

  // Chargement/décompression de l'image
  for (y_pos=0;y_pos<height;y_pos++)
  {
    if (!Read_bytes(file, buffer, (resolution==2) ? 80 : 160))
      goto error;
    
    ptr = buffer;
    for (x_pos = 0; x_pos < width; )
    {
      int i;
      switch (resolution)
      {
        case 0:
          PI1_8b_to_16p(ptr, pixels);
          ptr += 8;
          break;
        case 1:
          PI2_8b_to_16p(ptr, pixels);
          ptr += 4;
          break;
        case 2:
          for (i = 0; i < 8; i++)
            pixels[i] = (ptr[0] & (0x80 >> i)) ? 1 : 0;
          for (; i < 16; i++)
            pixels[i] = (ptr[1] & (0x80 >> (i - 8))) ? 1 : 0;
          ptr += 2;
          break;
        default:
          goto error;
      }
      for (i = 0; i < 16; i++)
        Set_pixel(context, x_pos++, y_pos, pixels[i]);
    }
  }
  File_error = 0; // everything was ok

error:
  fclose(file);
}

/// Save in NeoChrome format
void Save_NEO(T_IO_Context * context)
{
  word resolution = 0;
  FILE *file = NULL;
  short x_pos,y_pos;
  word color_cycling_range = 0, color_cycling_delay = 0;
  word display_time = 0;
  word image_width = 320, image_height = 200;
  byte buffer[32];
  byte pixels[320];
  char * ext;
  int i, j;

  File_error = 1;
  file = Open_file_write(context);
  if (file == NULL)
    return;

  // flags and resolution
  if (!Write_word_be(file, 0) || !Write_word_be(file, resolution))
    goto error;

  // palette
  PI1_code_palette(context->Palette, buffer);
  if (!Write_bytes(file, buffer, 16*2))
    goto error;

  // file name
  i = 0;
  j = 0;
  ext = strrchr(context->File_name, '.');
  while (j < 8 && ext != (context->File_name + i))
  {
    byte c = context->File_name[i++];
    if (c == 0)
      break;
    if (c >= 'a' && c <= 'z')
      c -= 32;
    if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'))
      buffer[j++] = c;
  }
  while (j < 8)
    buffer[j++] = ' ';
  buffer[j++] = '.';
  if (ext != NULL)
  {
    i = 0;
    while (j < 12)
    {
      byte c = ext[i++];
      if (c == 0)
        break;
      if (c >= 'a' && c <= 'z')
        c -= 32;
      if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'))
        buffer[j++] = c;
    }
  }
  while (j < 12)
    buffer[j++] = ' ';

  if (!Write_bytes(file, buffer, 12))
    goto error;

  // Save the 1st valid Color cycling range
  for (i = 0; i < context->Color_cycles; i++)
  {
    if (context->Cycle_range[i].Start < 16 && context->Cycle_range[i].End < 16)
    {
      color_cycling_range = 0x8000 | (context->Cycle_range[i].Start << 4) | context->Cycle_range[i].End;
      if (context->Cycle_range[i].Speed > 0)
      {
        color_cycling_delay = 175 / context->Cycle_range[i].Speed;
        if (color_cycling_delay > 0 && context->Cycle_range[i].Inverse)
          color_cycling_delay = 256 - color_cycling_delay;
        color_cycling_delay |= 0x8000;
      }
      break;
    }
  }
  if (!Write_word_be(file, color_cycling_range) || !Write_word_be(file, color_cycling_delay) || !Write_word_be(file, display_time))
    goto error;

  // Save image position and size
  if (!Write_word_be(file, 0) || !Write_word_be(file, 0)
      || !Write_word_be(file, image_width) || !Write_word_be(file, image_height))
    goto error;

  // Fill with 128 bytes header with 0's
  // a few files have the string "NEO!" at offset 124 (0x7C)
  for (i = ftell(file); i < 128; i++)
  {
    if (!Write_byte(file, 0))
      goto error;
  }

  // image coding
  for (y_pos=0;y_pos<200;y_pos++)
  {
    // Codage de la ligne
    memset(pixels, 0, 320);
    if (y_pos < context->Height)
    {
      for (x_pos = 0; (x_pos < 320) && (x_pos < context->Width); x_pos++)
        pixels[x_pos] = Get_pixel(context, x_pos, y_pos);
    }

    for (x_pos=0; x_pos < 320; x_pos += 16)
    {
      PI1_16p_to_8b(pixels + x_pos, buffer);
      if (!Write_bytes(file, buffer, 8))
        goto error;
    }
  }

  fclose(file);
  File_error = 0;
  return;

error:
  if (file != NULL)
    fclose(file);
  Remove_file(context);
}

/* @} */

//////////////////////////////////// C64 ////////////////////////////////////

/** C64 file formats
 */
enum c64_format
{
  F_invalid = -1,
  F_hires = 0,    ///< 320x200
  F_multi = 1,    ///< 160x200
  F_bitmap = 2,   ///< 320x200 monochrome
  F_fli = 3       ///< FLI (Flexible Line Interpretation)
};

/** C64 file formats names
 */
static const char *c64_format_names[] = {
  "Hires",
  "Multicolor",
  "Bitmap",
  "FLI"
};

static long C64_unpack_doodle(byte ** file_buffer, long file_size);

/**
 * Test for a C64 picture file
 *
 * Checks the file size and the load address
 *
 * References :
 * - http://unusedino.de/ec64/technical/formats/bitmap.html
 * - http://codebase64.org/doku.php?id=base:c64_grafix_files_specs_list_v0.03
 * - https://sourceforge.net/p/view64/code/HEAD/tree/trunk/libview64.c#l3737
 */
void Test_C64(T_IO_Context * context, FILE * file)
{
  unsigned long file_size;
  word load_addr;
  byte header[14];

  (void)context;
  File_error = 1;
  file_size = File_length_file(file);
  if (file_size < 16 || file_size > 48*1024)
    return; // File too short or too long, exit now
  // First test for formats without load address
  switch (file_size)
  {
    // case 1000: // screen or color
    case 8000: // raw bitmap
    case 9000: // bitmap + ScreenRAM
    case 10001: // multicolor
    case 17472: // FLI (BlackMail)
      File_error = 0;
      return;
    default: // then we don't know for now.
      if (!Read_word_le(file, &load_addr))
        return;
  }
  GFX2_Log(GFX2_DEBUG, "Test_C64() file_size=%ld LoadAddr=$%04X\n", file_size, load_addr);
  if (!Read_bytes(file, header, sizeof(header)))
    return;
  if (memcmp(header, "DRAZPAINT", 9) == 0)
  {
    GFX2_Log(GFX2_DEBUG, "Test_C64() header=%.13s RLE code = $%02X\n", header, header[13]);
    File_error = 0;
    return;
  }
  // check last 2 bytes
  if (fseek(file, -2, SEEK_END) < 0)
    return;
  if (!Read_bytes(file, header, 2))
    return;
  if (load_addr == 0x4000 && header[0] == 0xC2 && header[1] == 0x00) // Amica Paint EOF mark
  {
    File_error = 0;
    return;
  }
  switch (file_size)
  {
    // case 1002: // (screen or color) + loadaddr
    case 8002: // raw bitmap with loadaddr
    case 9002: // bitmap + ScreenRAM + loadaddr
      // $4000 => InterPaint Hi-Res (.iph)
    case 9003: // bitmap + ScreenRAM + loadaddr (+ border ?)
    case 9009: // bitmap + ScreenRAM + loadaddr
      // $2000 => Art Studio
    case 9218:
      // $5C00 => Doodle
    case 9332:
      // $3F8E => Paint Magic (.pmg) 'JEDI' at offset $0010 and $2010
    case 10003: // multicolor + loadaddr
      // $4000 => InterPaint multicolor
      // $6000 => Koala Painter
    case 10004:
      // $4000 => Face Paint (.fpt)
    case 10006:
      // $6000 => Run Paint (.rpm)
    case 10018:
      // $2000 => Advanced Art Studio
    case 10022:
      // $18DC => Micro Illustrator (uncompressed)
    case 10050:
      // $1800 => Picasso64
    case 10218:
      // $3C00 => Image System (.ism)
    case 10219:
      // $7800 => Saracen Paint (.sar)
      File_error = 0;
      break;
    case 10242:
      // $4000 => Artist 64 (.a64)
      // $A000 => Blazing paddles (.pi)
      // $5C00 => Rainbow Painter (.rp)
      if (load_addr != 0x4000 && load_addr != 0xa000 && load_addr != 0x5c00)
      {
        File_error = 1;
        return;
      }
      File_error = 0;
      break;
    case 17218:
    case 17409:
      // $3c00 => FLI-designer v1.1
      // ? $3ff0 => FLI designer 2 ?
    case 17410:
     // $3c00 => FLI MATIC
    case 17474: // FLI (BlackMail) + loadaddr
      // $3b00 => FLI Graph 2
    case 17665:
      // $3b00 => FLI editor
    case 17666:
      // $3b00 => FLI Graph
    case 10277: // multicolor CDU-Paint + loadaddr
      // $7EEF
      File_error = 0;
      break;
    default: // then we don't know for now.
      if (load_addr == 0x6000 || load_addr == 0x5c00)
      {
        long unpacked_size;
        byte * buffer = malloc(file_size);
        if (buffer == NULL)
          return;
        fseek(file, SEEK_SET, 0);
        if (!Read_bytes(file, buffer, file_size))
          return;
        unpacked_size = C64_unpack_doodle(&buffer, file_size);
        free(buffer);
        switch (unpacked_size)
        {
          case 9024:  // Doodle hi color
          case 9216:
          case 10001: // Koala painter 2
          case 10070:
            File_error = 0;
        }
      }
  }
}

/**
 * Load C64 hires (320x200)
 *
 * @param context	the IO context
 * @param bitmap the bitmap RAM (8000 bytes)
 * @param screen_ram the screen RAM (1000 bytes)
 */
static void Load_C64_hires(T_IO_Context *context, byte *bitmap, byte *screen_ram)
{
  int cx,cy,x,y,c[4],pixel,color;

  for(cy=0; cy<25; cy++)
  {
    for(cx=0; cx<40; cx++)
    {
      if(screen_ram != NULL)
      {
        c[0]=screen_ram[cy*40+cx]&15;
        c[1]=screen_ram[cy*40+cx]>>4;
      }
      else
      { /// If screen_ram is NULL, uses default C64 basic colors
        c[0] = 6;
        c[1] = 14;
      }
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

/**
 * Load C64 multicolor (160x200)
 *
 * @param context	the IO context
 * @param bitmap the bitmap RAM (8000 bytes)
 * @param screen_ram the screen RAM (1000 bytes)
 * @param color_ram the color RAM (1000 bytes)
 * @param background the background color
 */
static void Load_C64_multi(T_IO_Context *context, byte *bitmap, byte *screen_ram, byte *color_ram, byte background)
{
    int cx,cy,x,y,c[4],pixel,color;
    c[0]=background&15;
    for(cy=0; cy<25; cy++)
    {
        for(cx=0; cx<40; cx++)
        {
            c[1]=screen_ram[cy*40+cx]>>4;
            c[2]=screen_ram[cy*40+cx]&15;
            c[3]=color_ram[cy*40+cx]&15;

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

/**
 * Loads a C64 FLI (Flexible Line Interpretation) picture.
 * Sets 4 layers :
 *  - Layer 0 : filled with background colors (1 per line)
 *  - Layer 1 : "Color RAM" 4x8 blocks
 *  - Layer 2 : pixels (From Screen RAMs + Bitmap)
 *  - Layer 3 : Transparency layer filled with color 16
 *
 * @param context the IO context
 * @param bitmap 8000 bytes buffer
 * @param screen_ram 8 x 1024 bytes buffers
 * @param color_ram 1000 byte buffer
 * @param background 200 byte buffer
 */
void Load_C64_fli(T_IO_Context *context, byte *bitmap, byte *screen_ram, byte *color_ram, byte *background)
{
  // Thanks to MagerValp for complement of specifications.
  //
  // background : length: 200 (+ padding 56)
  //    These are the BG colors for lines 0-199 (top to bottom)
  //        Low nybble: the color.
  //        High nybble: garbage. ignore it.
  // color_ram  : length: 1000 (+ padding 24)
  //    Color RAM. Contains one color per 4x8 block.
  //    There are 40x25 such blocks, arranged from top left to bottom
  //    right, starting in right direction. For each block there is one byte.
  //        Low nybble: the color.
  //        High nybble: garbage. ignore it.
  // screen_ram : length: 8192
  //    Screen RAMs. The s is important.
  //    This is actually 8 blocks of 1000 bytes, each separated by a filler of
  //    24 bytes. Each byte contains data for a 4x1 pixel group, and a complete
  //    block will contain 40x25 of them. 40 is from left to right, and 25 is from
  //    top to bottom, spacing them 8 lines apart.
  //    The second block start at y=1, the third block starts at y=2, etc...
  //    Each byte contains 2 colors that *can* be used by the 4x1 pixel group:
  //        Low nybble: Color 1
  //        High nybble: Color 2
  //
  // bitmap     : length: 8000
  //    This is the final structure that refers to all others. It describes
  //    160x200 pixels linearly, from top left to bottom right, starting in
  //    right direction. For each pixel, two bits say which color is displayed
  //    (So 4 pixels are described by the same byte)
  //        00 Use the BG color of the current line (background[y])
  //        01 Use the Color 2 from the current 4x8 block of Screen RAM
  //           ((screen_ram[y/8][x/4] & 0xF0) >> 8)
  //        10 Use the Color 1 from the current 4x8 block of Screen RAM
  //           (screen_ram[y/8][x/4] & 0x0F)
  //        11 Use the color from Color RAM
  //           (color_ram[y/8][x/4] & 0x0F)
  //

  int cx,cy,x,y,c[4];

  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    // Fill layer 0 with background colors
    for(y=0; y<200; y++)
    {
      byte bg_color = 0;
      if (background != NULL)
        bg_color = background[y];
      for(x=0; x<160; x++)
        Set_pixel(context, x,y, bg_color);
    }

    // Fill layer 1 with color ram (1 color per 4x8 block)
    Set_loading_layer(context, 1);
    for(cy=0; cy<25; cy++)
    {
      for(cx=0; cx<40; cx++)
      {
        c[3]=color_ram[cy*40+cx]&15;
        for(y=0; y<8; y++)
        {
          for(x=0; x<4; x++)
          {
            Set_pixel(context, cx*4+x,cy*8+y,c[3]);
          }
        }
      }
    }
  }

  // Layer 2 are actual pixels
  Set_loading_layer(context, 2);
  for(cy=0; cy<25; cy++)
  {
    for(cx=0; cx<40; cx++)
    {
      c[3]=color_ram[cy*40+cx]&15;
      for(y=0; y<8; y++)
      {
        int pixel=bitmap[cy*320+cx*8+y];

        c[0] = 0;
        if(background != NULL)
          c[0] = background[cy*8+y]&15;
        c[1]=screen_ram[y*1024+cy*40+cx]>>4;
        c[2]=screen_ram[y*1024+cy*40+cx]&15;
        for(x=0; x<4; x++)
        {
          int color=c[(pixel&3)];
          pixel>>=2;
          Set_pixel(context, cx*4+(3-x),cy*8+y,color);
        }
      }
    }
  }
  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    // Fill layer 3 with color 16
    Set_loading_layer(context, 3);
    for(y=0; y<200; y++)
    {
      for(x=0; x<160; x++)
        Set_pixel(context, x,y,16);
    }
  }
}

/**
 * Count the length of the unpacked data
 *
 * RLE encoding is either ESCAPE CODE, COUNT, VALUE
 * or ESCAPE CODE, VALUE, COUNT
 *
 * @param buffer the packed data
 * @param input_size the packed data byte count
 * @param RLE_code the escape code
 * @param order 0 for ESCAPE, COUNT, VALUE, 1 for ESCAPE, VALUE, COUNT
 * @return the unpacked data byte count
 */
static long C64_unpack_get_length(const byte * buffer, long input_size, byte RLE_code, int order)
{
  const byte * end;
  long unpacked_size = 0;

  end = buffer + input_size;
  while(buffer < end)
  {
    if (*buffer == RLE_code)
    {
      if (order)
      { // ESCAPE, VALUE, COUNT
        buffer += 2;  // skip value
        unpacked_size += *buffer;
      }
      else
      { // ESCAPE, COUNT, VALUE
        buffer++;
        if (*buffer == 0)
          break;
        unpacked_size += *buffer++;
      }
    }
    else
      unpacked_size++;
    buffer++;
  }
  return unpacked_size;
}

/**
 * unpack RLE packed data
 *
 * RLE encoding is either ESCAPE CODE, COUNT, VALUE
 * or ESCAPE CODE, VALUE, COUNT
 *
 * @param unpacked buffer to received unpacked data
 * @param buffer the packed data
 * @param input_size the packed data byte count
 * @param RLE_code the escape code
 * @param order 0 for ESCAPE, COUNT, VALUE, 1 for ESCAPE, VALUE, COUNT
 */
static void C64_unpack(byte * unpacked, const byte * buffer, long input_size, byte RLE_code, int order)
{
  const byte * end;

  end = buffer + input_size;
  while(buffer < end)
  {
    if (*buffer == RLE_code)
    {
      byte count;
      byte value;
      buffer++;
      if (order)
      { // ESCAPE, VALUE, COUNT
        value = *buffer++;
        count = *buffer;
      }
      else
      { // ESCAPE, COUNT, VALUE
        count = *buffer++;
        value = *buffer;
      }
      if (count == 0)
        break;
      while (count-- > 0)
        *unpacked++ = value;
    }
    else
      *unpacked++ = *buffer;
    buffer++;
  }
}

/**
 * Unpack the Amica Paint RLE packing
 *
 * @param[in,out] file_buffer will contain the unpacked buffer on return
 * @param[in] file_size packed buffer size
 * @return the unpacked data size or -1 in case of error
 *
 * Ref:
 * - http://codebase64.org/doku.php?id=base:c64_grafix_files_specs_list_v0.03
 */
static long C64_unpack_amica(byte ** file_buffer, long file_size)
{
  long unpacked_size;
  byte * unpacked_buffer;
  const byte RLE_code = 0xC2;

  if (file_size <= 16 || file_buffer == NULL || *file_buffer == NULL)
    return -1;
  unpacked_size = C64_unpack_get_length(*file_buffer + 2, file_size - 2, RLE_code, 0);
  GFX2_Log(GFX2_DEBUG, "C64_unpack_amica() unpacked_size=%ld\n", unpacked_size);
   // 2nd pass to unpack
  unpacked_buffer = malloc(unpacked_size);
  if (unpacked_buffer == NULL)
    return -1;
  C64_unpack(unpacked_buffer, *file_buffer + 2, file_size - 2, RLE_code, 0);

  free(*file_buffer);
  *file_buffer = unpacked_buffer;
  return unpacked_size;
}

/**
 * Unpack the DRAZPAINT RLE packing
 *
 * @param[in,out] file_buffer will contain the unpacked buffer on return
 * @param[in] file_size packed buffer size
 * @return the unpacked data size or -1 in case of error
 *
 * Ref:
 * - https://www.godot64.de/german/l_draz.htm
 * - https://sourceforge.net/p/view64/code/HEAD/tree/trunk/libview64.c#l2805
 */
static long C64_unpack_draz(byte ** file_buffer, long file_size)
{
  long unpacked_size;
  byte * unpacked_buffer;
  byte RLE_code;

  if (file_size <= 16 || file_buffer == NULL || *file_buffer == NULL)
    return -1;
  RLE_code = (*file_buffer)[15];
  // First pass to know unpacked size
  unpacked_size = C64_unpack_get_length(*file_buffer + 16, file_size - 16, RLE_code, 0);
  GFX2_Log(GFX2_DEBUG, "C64_unpack_draz() \"%.13s\" RLE code=$%02X RLE data length=%ld unpacked_size=%ld\n",
           *file_buffer + 2, RLE_code, file_size - 16, unpacked_size);
   // 2nd pass to unpack
  unpacked_buffer = malloc(unpacked_size);
  if (unpacked_buffer == NULL)
    return -1;
  C64_unpack(unpacked_buffer, *file_buffer + 16, file_size - 16, RLE_code, 0);
  free(*file_buffer);
  *file_buffer = unpacked_buffer;
  return unpacked_size;
}

/**
 * Unpack doodle/koala painter 2 data
 *
 * @return the unpacked data size or -1 in case of error
 */
static long C64_unpack_doodle(byte ** file_buffer, long file_size)
{
  long unpacked_size;
  byte * unpacked_buffer;
  const byte RLE_code = 0xFE;

  if (file_size <= 16 || file_buffer == NULL || *file_buffer == NULL)
    return -1;
  // First pass to know unpacked size
  unpacked_size = C64_unpack_get_length(*file_buffer + 2, file_size - 2, RLE_code, 1);
  GFX2_Log(GFX2_DEBUG, "C64_unpack_doodle() unpacked_size=%ld\n", unpacked_size);
   // 2nd pass to unpack
  unpacked_buffer = malloc(unpacked_size);
  if (unpacked_buffer == NULL)
    return -1;
  C64_unpack(unpacked_buffer, *file_buffer + 2, file_size - 2, RLE_code, 1);
  free(*file_buffer);
  *file_buffer = unpacked_buffer;
  return unpacked_size;
}

/**
 * Load C64 pictures formats.
 *
 * Supports:
 * - Hires (with or without ScreenRAM)
 * - Multicolor (Koala or CDU-paint format)
 * - FLI
 *
 * see http://unusedino.de/ec64/technical/formats/bitmap.html
 *
 * @param context the IO context
 */
void Load_C64(T_IO_Context * context)
{
    FILE* file;
    long file_size;
    byte hasLoadAddr=0;
    word load_addr;
    enum c64_format loadFormat = F_invalid;

    byte *file_buffer;
    byte *bitmap, *screen_ram, *color_ram=NULL, *background=NULL; // Only pointers to existing data
    byte *temp_buffer = NULL;
    word width, height=200;

    file = Open_file_read(context);

    if (file)
    {
        File_error=0;
        file_size = File_length_file(file);

        // Load entire file in memory
        file_buffer=(byte *)malloc(file_size);
        if (!file_buffer)
        {
            File_error = 1;
            fclose(file);
            return;
        }
        if (!Read_bytes(file,file_buffer,file_size))
        {
            File_error = 1;
            free(file_buffer);
            fclose(file);
            return;
        }
        fclose(file);

        // get load address (valid only if hasLoadAddr = 1)
        load_addr = file_buffer[0] | (file_buffer[1] << 8);

        // Unpack if needed
        if (memcmp(file_buffer + 2, "DRAZPAINT", 9) == 0)
          file_size = C64_unpack_draz(&file_buffer, file_size);
        else if(load_addr == 0x4000 && file_buffer[file_size-2] == 0xC2 && file_buffer[file_size-1] == 0)
          file_size = C64_unpack_amica(&file_buffer, file_size);
        else if (file_size < 8000 && (load_addr == 0x6000 || load_addr == 0x5c00))
          file_size = C64_unpack_doodle(&file_buffer, file_size);

        switch (file_size)
        {
            case 8000: // raw bitmap
                hasLoadAddr=0;
                loadFormat=F_bitmap;
                bitmap=file_buffer+0; // length: 8000
                screen_ram=NULL;
                break;

            case 8002: // raw bitmap with loadaddr
                hasLoadAddr=1;
                loadFormat=F_bitmap;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=NULL;
                break;

            case 9000: // bitmap + ScreenRAM
                hasLoadAddr=0;
                loadFormat=F_hires;
                bitmap=file_buffer+0; // length: 8000
                screen_ram=file_buffer+8000; // length: 1000
                break;

            case 9003: // bitmap + ScreenRAM + loadaddr (+ border ?)
            case 9002: // bitmap + ScreenRAM + loadaddr
                hasLoadAddr=1;
                loadFormat=F_hires;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=file_buffer+8002; // length: 1000
                break;

            case 9009: // Art Studio (.aas)
                hasLoadAddr=1;
                loadFormat=F_hires;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=file_buffer+8002; // length: 1000
                break;

            case 9024:  // Doodle (unpacked from .jj)
            case 9216:
                hasLoadAddr=0;
                loadFormat=F_hires;
                screen_ram=file_buffer; // length: 1000 (+24 padding)
                bitmap=file_buffer+1024; // length: 8000
                break;

            case 9218: // Doodle (.dd)
                hasLoadAddr=1;
                loadFormat=F_hires;
                screen_ram=file_buffer+2; // length: 1000 (+24 padding)
                bitmap=file_buffer+1024+2; // length: 8000
                break;

            case 9332:  // Paint Magic .pmg
                hasLoadAddr=1;
                loadFormat=F_multi;
                // Display routine between offset $0002 and $0073 (114 bytes)
                // duplicated between offset      $2002 and $2073
                bitmap=file_buffer+114+2;         // $0074
                background=file_buffer+8000+114+2;// $1FB4
                temp_buffer=malloc(1000);
                memset(temp_buffer, file_buffer[3+8000+114+2], 1000); // color RAM Byte
                color_ram=temp_buffer;
                //border  byte = file_buffer[4+8000+114+2];
                screen_ram=file_buffer+8192+114+2;  // $2074
                break;

            case 10001: // multicolor
            case 10070: // unpacked file.
                hasLoadAddr=0;
                loadFormat=F_multi;
                bitmap=file_buffer+0; // length: 8000
                screen_ram=file_buffer+8000; // length: 1000
                color_ram=file_buffer+9000; // length: 1000
                background=file_buffer+10000; // only 1
                break;

            case 10003: // multicolor + loadaddr
            case 10004: // extra byte is border color
            case 10006: // Run Paint
                hasLoadAddr=1;
                loadFormat=F_multi;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=file_buffer+8002; // length: 1000
                color_ram=file_buffer+9002; // length: 1000
                background=file_buffer+10002; // only 1
                break;

            case 10018: // Advanced Art Studio (.ocp) + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                bitmap=file_buffer+2; // length: 8000
                screen_ram=file_buffer+8000+2; // length: 1000
                color_ram=file_buffer+9016+2; // length: 1000
                // filebuffer+9000+2 is border
                background=file_buffer+9001+2; // only 1
                break;

            case 10022: // Micro Illustrator (.mil)
                hasLoadAddr=1;
                loadFormat=F_multi;
                screen_ram=file_buffer+20+2;
                color_ram=file_buffer+1000+20+2;
                bitmap=file_buffer+2*1000+20+2;
                break;

            case 10049: // unpacked DrazPaint
                hasLoadAddr=1;
                loadFormat=F_multi;
                color_ram=file_buffer; // length: 1000 + (padding 24)
                screen_ram=file_buffer+1024; // length: 1000 + (padding 24)
                bitmap=file_buffer+1024*2; // length: 8000
                background=file_buffer+8000+1024*2;
                break;

            case 10050: // Picasso64 multicolor + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                color_ram=file_buffer+2; // length: 1000 + (padding 24)
                screen_ram=file_buffer+1024+2; // length: 1000 + (padding 24)
                bitmap=file_buffer+1024*2+2; // length: 8000
                background=file_buffer+1024*2+2-1; // only 1
                break;

            case 10218: // Image System
                hasLoadAddr=1;
                loadFormat=F_multi;
                color_ram=file_buffer+2; // Length: 1000 (+ padding 24)
                bitmap=file_buffer+1024+2; // Length: 8000 (+padding 192)
                screen_ram=file_buffer+8192+1024+2;  // Length: 1000 (no padding)
                background=file_buffer+8192+1024+2-1; // only 1
                break;

            case 10219: // Saracen Paint (.sar)
                hasLoadAddr=1;
                loadFormat=F_multi;
                screen_ram=file_buffer+2;  // Length: 1000 (+ padding24)
                background=file_buffer+1008+2; // offset 0x3F0 (only 1 byte)
                bitmap=file_buffer+1024+2; // Length: 8000 (+padding 192)
                color_ram=file_buffer+8192+1024+2; // Length: 1000 (+ padding 24)
                break;

            case 10242: // Artist 64/Blazing Paddles/Rainbow Painter multicolor + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                switch(load_addr)
                {
                  default:
                  case 0x4000:  // Artist 64
                    bitmap=file_buffer+2; // length: 8000 (+padding 192)
                    screen_ram=file_buffer+8192+2; // length: 1000 + (padding 24)
                    color_ram=file_buffer+1024+8192+2; // length: 1000 + (padding 24)
                    background=file_buffer+1024*2+8192+2-1; // only 1
                    break;
                  case 0xa000:  // Blazing Paddles
                    bitmap=file_buffer+2; // length: 8000 (+padding 192)
                    screen_ram=file_buffer+8192+2; // length: 1000 + (padding 24)
                    color_ram=file_buffer+1024+8192+2; // length: 1000 + (padding 24)
                    background=file_buffer+8064+2; // only 1
                    break;
                  case 0x5c00:  // Rainbow Painter
                    screen_ram=file_buffer+2; // length: 1000 + (padding 24)
                    bitmap=file_buffer+1024+2; // length: 8000 (+padding 192)
                    color_ram=file_buffer+1024+8192+2; // length: 1000 + (padding 24)
                    background=file_buffer; // only 1
                    break;
                }
                break;

            case 10257: // unpacked Amica Paint (.ami)
                hasLoadAddr=1;
                loadFormat=F_multi;
                bitmap=file_buffer; // length 8000
                screen_ram=file_buffer+8000;  // length: 1000
                color_ram=file_buffer+1000+8000;// length:1000
                background=file_buffer+2*1000+8000;//1
                // remaining bytes (offset 10001, length 256) are a "Color Rotation Table"
                // we should decode if we learn its format...
                break;

            case 10277: // multicolor CDU-Paint + loadaddr
                hasLoadAddr=1;
                loadFormat=F_multi;
                // 273 bytes of display routine
                bitmap=file_buffer+275; // length: 8000
                screen_ram=file_buffer+8275; // length: 1000
                color_ram=file_buffer+9275; // length: 1000
                background=file_buffer+10275; // only 1
                break;

            case 17472: // FLI (BlackMail)
                hasLoadAddr=0;
                loadFormat=F_fli;
                background=file_buffer+0; // length: 200 (+ padding 56)
                color_ram=file_buffer+256; // length: 1000 (+ padding 24)
                screen_ram=file_buffer+1280; // length: 8192
                bitmap=file_buffer+9472; // length: 8000
                break;

            case 17474: // FLI (BlackMail) + loadaddr
                hasLoadAddr=1;
                loadFormat=F_fli;
                background=file_buffer+2; // length: 200 (+ padding 56)
                color_ram=file_buffer+258; // length: 1000 (+ padding 24)
                screen_ram=file_buffer+1282; // length: 8192
                bitmap=file_buffer+9474; // length: 8000
                break;

            case 17218:
            case 17409: // FLI-Designer v1.1 (+loadaddr)
            case 17410: // => FLI MATIC (background at 2+1024+8192+8000+65 ?)
              hasLoadAddr=1;
              loadFormat=F_fli;
              background=NULL;
              color_ram=file_buffer+2; // length: 1000 (+ padding 24)
              screen_ram=file_buffer+1024+2; // length: 8192
              bitmap=file_buffer+8192+1024+2; // length: 8000
              break;

            case 17666: // FLI Graph
              hasLoadAddr=1;
              loadFormat=F_fli;
              background=file_buffer+2;
              color_ram=file_buffer+256+2; // length: 1000 (+ padding 24)
              screen_ram=file_buffer+1024+256+2; // length: 8192
              bitmap=file_buffer+8192+1024+256+2; // length: 8000
              break;

            case 17665: // FLI Editor
              hasLoadAddr=1;
              loadFormat=F_fli;
              background=file_buffer+8;
              color_ram=file_buffer+256+2; // length: 1000 (+ padding 24)
              screen_ram=file_buffer+1024+256+2; // length: 8192
              bitmap=file_buffer+8192+1024+256+2; // length: 8000
              break;

            default:
                File_error = 1;
                free(file_buffer);
                return;
        }

        if (loadFormat == F_invalid)
        {
          File_error = 1;
          free(file_buffer);
          return;
        }

        if (loadFormat == F_fli || loadFormat == F_multi)
        {
          context->Ratio = PIXEL_WIDE;
          width = 160;
        }
        else
        {
          context->Ratio = PIXEL_SIMPLE;
          width = 320;
        }

        // Write detailed format in comment
        if (hasLoadAddr)
          snprintf(context->Comment,COMMENT_SIZE+1,"%s, load at $%4.4X",c64_format_names[loadFormat],load_addr);
        else
          snprintf(context->Comment,COMMENT_SIZE+1,"%s, no addr",c64_format_names[loadFormat]);

        Pre_load(context, width, height, file_size, FORMAT_C64, context->Ratio, (loadFormat == F_bitmap) ? 1 : 4); // Do this as soon as you can

        if (Config.Clear_palette)
          memset(context->Palette,0, sizeof(T_Palette));
        C64_set_palette(context->Palette);
        context->Transparent_color=16;

        switch(loadFormat)
        {
          case F_fli:
            Load_C64_fli(context,bitmap,screen_ram,color_ram,background);
            Set_image_mode(context, IMAGE_MODE_C64FLI);
            break;
          case F_multi:
            Load_C64_multi(context,bitmap,screen_ram,color_ram,
                           (background==NULL) ? 0 : *background);
            Set_image_mode(context, IMAGE_MODE_C64MULTI);
            break;
          default:
            Load_C64_hires(context,bitmap,screen_ram);
            if (loadFormat == F_hires)
              Set_image_mode(context, IMAGE_MODE_C64HIRES);
        }

        free(file_buffer);
        if (temp_buffer)
          free(temp_buffer);
    }
    else
        File_error = 1;
}

/**
 * Display the dialog for C64 save parameters
 *
 * @param[in,out] saveFormat one of the C64 mode from @ref c64_format
 * @param[in,out] saveWhat 0=All, 1=Only bitmap, 2=Only Screen RAM, 3=Only color RAM
 * @param[in,out] loadAddr actual load address or 0 for "None"
 * @return true to proceed, false to abort
 */
static int Save_C64_window(enum c64_format *saveFormat, byte *saveWhat, word *loadAddr)
{
  int button;
  unsigned int i;
  T_Dropdown_button *what, *addr;
  T_Dropdown_button *format;
  static const char * what_label[] = {
    "All",
    "Bitmap",
    "Screen",
    "Color"
  };
  static const char * address_label[] = {
    "None",
    "$2000",
    "$4000",
    "$6000",
    "$8000",
    "$A000",
    "$C000",
    "$E000"
  };
  // default addresses :
  //  - FLI Fli Graph 2 (BlackMail) => $3b00
  //  - multicolor (Koala Painter) => $6000
  //  - hires (InterPaint) => $4000

  Open_window(200,120,"C64 saving settings");
  Window_set_normal_button(110,100,80,15,"Save",1,1,KEY_RETURN); // 1
  Window_set_normal_button(10,100,80,15,"Cancel",1,1,KEY_ESCAPE); // 2

  Print_in_window(13,18,"Data:",MC_Dark,MC_Light);
  what = Window_set_dropdown_button(10,28,90,15,70,what_label[*saveWhat],1, 0, 1, LEFT_SIDE,0); // 3
  Window_dropdown_clear_items(what);
  for (i=0; i<sizeof(what_label)/sizeof(what_label[0]); i++)
    Window_dropdown_add_item(what,i,what_label[i]);

  Print_in_window(113,18,"Address:",MC_Dark,MC_Light);
  addr = Window_set_dropdown_button(110,28,70,15,70,address_label[*loadAddr/0x2000],1, 0, 1, LEFT_SIDE,0); // 4
  Window_dropdown_clear_items(addr);
  for (i=0; i<sizeof(address_label)/sizeof(address_label[0]); i++)
    Window_dropdown_add_item(addr,i,address_label[i]);

  Print_in_window(13,46,"Format:",MC_Dark,MC_Light);
  format = Window_set_dropdown_button(10,56,90,15,88,c64_format_names[*saveFormat],1, 0, 1, LEFT_SIDE,0); // 5
  if (*saveFormat == F_hires || *saveFormat == F_bitmap)
  {
    Window_dropdown_add_item(format, F_hires, c64_format_names[F_hires]);
    Window_dropdown_add_item(format, F_bitmap, c64_format_names[F_bitmap]);
  }
  else
  {
    Window_dropdown_add_item(format, F_multi, c64_format_names[F_multi]);
    Window_dropdown_add_item(format, F_fli, c64_format_names[F_fli]);
  }

  Update_window_area(0,0,Window_width,Window_height);
  Display_cursor();

  do
  {
    button = Window_clicked_button();
    if (Is_shortcut(Key, 0x100+BUTTON_HELP))
    {
      Key = 0;
      Window_help(BUTTON_SAVE, "COMMODORE 64 FORMATS");
    }
    else switch(button)
    {
      case 3: // Save what
        *saveWhat = Window_attribute2;
        GFX2_Log(GFX2_DEBUG, "Save_C64_Window() : what=%d (%s)\n", *saveWhat, what_label[*saveWhat]);
        break;

      case 4: // Load addr
        *loadAddr = Window_attribute2*0x2000;
        GFX2_Log(GFX2_DEBUG, "Save_C64_Window() : addr=$%04x (%d)\n",*loadAddr,Window_attribute2);
        break;

      case 5:
        *saveFormat = Window_attribute2;
        GFX2_Log(GFX2_DEBUG, "Save_C64_Window() : format=%d\n", Window_attribute2);
        break;

      case 0: break;
    }
  } while(button!=1 && button!=2);

  Close_window();
  Display_cursor();
  return button==1;
}

/// Save a C64 hires picture
///
/// c64 hires is 320x200 with only 2 colors per 8x8 block.
static int Save_C64_hires(T_IO_Context *context, byte saveWhat, word loadAddr)
{
  int i, pos = 0;
  word cx, cy, x, y;
  byte screen_ram[1000],bitmap[8000];
  FILE *file;

  for(cy=0; cy<25; cy++) // Character line, 25 lines
  {
    for(cx=0; cx<40; cx++) // Character column, 40 columns
    {
      byte fg, bg;  // foreground and background colors for the 8x8 block
      byte c[2];
      int count = 0;
      // first pass : find colors used
      for(y=0; y<8; y++)
      {
        for(x=0; x<8; x++)
        {
          byte pixel = Get_pixel(context, x+cx*8,y+cy*8);
          if(pixel>15)
          {
            Warning_message("Color above 15 used");
            // TODO hilite offending block here too?
            // or make it smarter with color allocation?
            // However, the palette is fixed to the 16 first colors
            return 1;
          }
          for (i = 0; i < count; i++)
          {
            if (c[i] == pixel)
              break;
          }
          if (i >= 2)
          {
            Warning_with_format("More than 2 colors\nin 8x8 pixel cell: (%d, %d)\nRect: (%d, %d, %d, %d)", cx, cy, cx * 8, cy * 8, cx * 8 + 7, cy * 8 + 7);
            // TODO here we should hilite the offending block
            return 1;
          }
          if (i >= count)
            c[count++] = pixel;
        }
      }

      if (count == 1)
      {
        if (c[0] == 0)  // only black
          fg = 1; // white
        else
          fg = c[0];
        bg = 0; // black
      }
      else
      {
        // set lower color index as background
        if (c[0] < c[1])
        {
          fg = c[1];
          bg = c[0];
        }
        else
        {
          fg = c[0];
          bg = c[1];
        }
      }
      screen_ram[cx+cy*40] = (fg<<4) | bg;

      // 2nd pass : store bitmap (0 = background, 1 = foreground)
      for(y=0; y<8; y++)
      {
        byte bits = 0;
        for(x=0; x<8; x++)
        {
          bits <<= 1;
          if (Get_pixel(context, x+cx*8, y+cy*8) == fg)
            bits |= 1;
        }
        bitmap[pos++] = bits;
      }
    }
  }

  file = Open_file_write(context);

  if(!file)
  {
    Warning_message("File open failed");
    File_error = 1;
    return 1;
  }

  if (loadAddr)
    Write_word_le(file,loadAddr);

  if (saveWhat==0 || saveWhat==1)
    Write_bytes(file,bitmap,8000);
  if (saveWhat==0 || saveWhat==2)
    Write_bytes(file,screen_ram,1000);

  fclose(file);
  return 0;
}


/**
 * Save a C64 FLI (Flexible Line Interpretation) picture.
 *
 * This function is able to save a one layer picture, by finding
 * itself the background colors and color RAM value to be used.
 *
 * The algorithm is :
 * - first choose the lowest value for all possible background colors for each line
 * - first the lowest value from the possible colors for color RAM
 * - encode bitmap and screen RAMs
 *
 * The algorithm can fail by picking a "wrong" background color for a line,
 * that make the choice for the color RAM value of one of the 40 blocks impossible.
 *
 * @param context the IO context
 * @param saveWhat what part of the data to save
 * @param loadAddr The load address
 */
int Save_C64_fli_monolayer(T_IO_Context *context, byte saveWhat, word loadAddr)
{
  FILE * file;
  byte bitmap[8000],screen_ram[1024*8],color_ram[1024];
  byte background[256];

  memset(bitmap, 0, sizeof(bitmap));
  memset(screen_ram, 0, sizeof(screen_ram));
  memset(color_ram, 0, sizeof(color_ram));
  memset(background, 0, sizeof(background));

  memset(color_ram, 0xff, 40*25); // no hint
  memset(background, 0xff, 200);

  if (C64_pixels_to_FLI(bitmap, screen_ram, color_ram, background, context->Target_address, context->Pitch, 0) > 0)
    return 1;

  file = Open_file_write(context);

  if(!file)
  {
    Warning_message("File open failed");
    File_error = 1;
    return 1;
  }

  if (loadAddr)
    Write_word_le(file, loadAddr);

  if (saveWhat==0)
    Write_bytes(file,background,256);    // Background colors for lines 0-199 (+ 56bytes padding)

  if (saveWhat==0 || saveWhat==3)
    Write_bytes(file,color_ram,1024); // Color RAM (1000 bytes + padding 24)

  if (saveWhat==0 || saveWhat==1)
    Write_bytes(file,screen_ram,8192);  // Screen RAMs 8 x (1000 bytes + padding 24)

  if (saveWhat==0 || saveWhat==2)
    Write_bytes(file,bitmap,8000);  // BitMap

  fclose(file);

  return 0;
}

/**
 * Save a C64 multicolor picture
 *
 * @param context the IO context
 * @param saveWhat what part of the data to save
 * @param loadAddr The load address
 */
int Save_C64_multi(T_IO_Context *context, byte saveWhat, word loadAddr)
{
    /*
    BITS     COLOR INFORMATION COMES FROM
    00     Background color #0 (screen color)
    01     Upper 4 bits of Screen RAM
    10     Lower 4 bits of Screen RAM
    11     Color RAM nybble (nybble = 1/2 byte = 4 bits)
    */

    int cx,cy,x,y,c[4]={0,0,0,0},color,lut[16],bits,pixel,pos=0;
    int cand,n,used;
    word cols, candidates = 0, invalids = 0;

    // FIXME allocating this on the stack is not a good idea. On some platforms
    // the stack has a rather small size...
    byte bitmap[8000],screen_ram[1000],color_ram[1000];

    word numcolors;
    dword cusage[256];
    byte i,background=0;
    FILE *file;

    // Detect the background color the image should be using. It's the one that's
    // used on all tiles having 4 colors.
    for(y=0;y<200;y=y+8)
    {
        for (x = 0; x<160; x=x+4)
        {
            cols = 0;

            // Compute the usage count of each color in the tile
            for (cy=0;cy<8;cy++)
            for (cx=0;cx<4;cx++)
            {
              pixel=Get_pixel(context, x+cx,y+cy);
              if(pixel>15)
              {
                Warning_message("Color above 15 used");
                // TODO hilite as in hires, you should stay to
                // the fixed 16 color palette
                return 1;
              }
              cols |= (1 << pixel);
            }

            cand = 0;
            used = 0;
            // Count the number of used colors in the tile
            for (n = 0; n<16; n++)
            {
                if (cols & (1 << n))
                    used++;
            }

            if (used>3)
            {
                GFX2_Log(GFX2_DEBUG, "(%3d,%3d) used=%d cols=%04x\n", x, y, used,(unsigned)cols);
                // This is a tile that uses the background color (and 3 others)

                // Try to guess which color is most likely the background one
                for (n = 0; n<16; n++)
                {
                    if ((cols & (1 << n)) && !((candidates | invalids) & (1 << n))) {
                        // This color is used in this tile but
                        // was not used in any other tile yet,
                        // so it could be the background one.
                        candidates |= 1 << n;
                    }

                    if ((cols & (1 << n)) == 0 ) {
                        // This color isn't used at all in this tile:
                        // Can't be the global background
                        invalids |= 1 << n;
                        candidates &= ~(1 << n);
                    }

                    if (candidates & (1 << n)) {
                        // We have a candidate, mark it as such
                        cand++;
                    }
                }

                // After checking the constraints for this tile, do we have
                // candidate background colors left ?
                if (cand==0)
                {
                    Warning_message("No possible global background color");
                    return 1;
                }
            }
        }
    }

	// Now just pick the first valid candidate
	for (n = 0; n<16; n++)
	{
		if (candidates & (1 << n)) {
			background = n;
			break;
		}
	}
  GFX2_Log(GFX2_DEBUG, "Save_C64_multi() background=%d ($%x) candidates=%x invalid=%x\n",
           (int)background, (int)background, (unsigned)candidates, (unsigned)invalids);


  // Now that we know which color is the background, we can encode the cells
  for(cy=0; cy<25; cy++)
  {
    for(cx=0; cx<40; cx++)
    {
      numcolors=Count_used_colors_area(cusage,cx*4,cy*8,4,8);
      if(numcolors>4)
      {
        Warning_with_format("More than 4 colors\nin 4x8 pixel cell: (%d, %d)\nRect: (%d, %d, %d, %d)", cx, cy, cx * 4, cy * 8, cx * 4 + 3, cy * 8 + 7);
        // TODO hilite offending block
        return 1;
      }
      color=1;
      c[0]=background;
      for(i=0; i<16; i++)
      {
        lut[i]=0;
        if(cusage[i] && (i!=background))
        {
          lut[i]=color;
          c[color]=i;
          color++;
        }
      }
      // add to screen_ram and color_ram
      screen_ram[cx+cy*40]=c[1]<<4|c[2];
      color_ram[cx+cy*40]=c[3];

      for(y=0;y<8;y++)
      {
        bits=0;
        for(x=0;x<4;x++)
        {
          pixel = Get_pixel(context, cx*4+x,cy*8+y);
          bits = (bits << 2) | lut[pixel];
        }
        bitmap[pos++]=bits;
      }
    }
  }

  file = Open_file_write(context);

  if(!file)
  {
    Warning_message("File open failed");
    File_error = 2;
    return 2;
  }

  setvbuf(file, NULL, _IOFBF, 64*1024);

  if (loadAddr)
    Write_word_le(file,loadAddr);

  if (saveWhat==0 || saveWhat==1)
    Write_bytes(file,bitmap,8000);

  if (saveWhat==0 || saveWhat==2)
    Write_bytes(file,screen_ram,1000);

  if (saveWhat==0 || saveWhat==3)
    Write_bytes(file,color_ram,1000);

  if (saveWhat==0)
    Write_byte(file,background);

  fclose(file);
  return 0;
}

/**
 * Save a C64 FLI (Flexible Line Interpretation) picture.
 *
 * This function need a 3 layer image :
 * - layer 0 is background colors
 * - layer 1 is color RAM values (4x8 blocks)
 * - layer 2 is the actual picture
 *
 * @param context the IO context
 * @param saveWhat what part of the data to save
 * @param loadAddr The load address
 */
int Save_C64_fli(T_IO_Context * context, byte saveWhat, word loadAddr)
{
  FILE *file;
  byte file_buffer[17474];

  memset(file_buffer,0,sizeof(file_buffer));

  switch(C64_FLI(context, file_buffer+9474, file_buffer+1282, file_buffer+258, file_buffer+2))
  {
    case 0: // OK
      break;
    case 1:
      Warning_message("Less than 3 layers");
      File_error=1;
      return 1;
    case 2:
      Warning_message("Picture must be 160x200");
      File_error=1;
      return 1;
    default:
      File_error=1;
      return 1;
  }

  file = Open_file_write(context);

  if(!file)
  {
    Warning_message("File open failed");
    File_error = 1;
    return 1;
  }

  if (loadAddr)
    Write_word_le(file, loadAddr);

  if (saveWhat==0)
    Write_bytes(file,file_buffer+2,256);    // Background colors for lines 0-199 (+ 56bytes padding)

  if (saveWhat==0 || saveWhat==3)
    Write_bytes(file,file_buffer+258,1024); // Color RAM (1000 bytes + padding 24)

  if (saveWhat==0 || saveWhat==1)
    Write_bytes(file,file_buffer+1282,8192);  // Screen RAMs 8 x (1000 bytes + padding 24)

  if (saveWhat==0 || saveWhat==2)
    Write_bytes(file,file_buffer+9474,8000);  // BitMap

  fclose(file);
  return 0;
}

/**
 * Save C64 picture.
 *
 * Supports :
 * - HiRes (320x200)
 * - Multicolor
 * - FLI
 *
 * @param context the IO context
 */
void Save_C64(T_IO_Context * context)
{
  enum c64_format saveFormat = F_invalid;
  static byte saveWhat=0;
  static word loadAddr=0;

  if (((context->Width!=320) && (context->Width!=160)) || context->Height!=200)
  {
    Warning_message("must be 320x200 or 160x200");
    File_error = 1;
    return;
  }

  saveFormat = (context->Width == 320) ? F_hires : F_multi;

  GFX2_Log(GFX2_DEBUG, "Save_C64() extension : %s\n", context->File_name + strlen(context->File_name) - 4);
  if (strcasecmp(context->File_name + strlen(context->File_name) - 4, ".fli") == 0)
    saveFormat = F_fli;

  if(!Save_C64_window(&saveFormat, &saveWhat,&loadAddr))
  {
    File_error = 1;
    return;
  }

  Set_saving_layer(context, 0);
  switch (saveFormat)
  {
    case F_fli:
      if (context->Nb_layers < 3)
        File_error = Save_C64_fli_monolayer(context, saveWhat, loadAddr);
      else
        File_error = Save_C64_fli(context, saveWhat, loadAddr);
      break;
    case F_multi:
      File_error = Save_C64_multi(context, saveWhat, loadAddr);
      break;
    case F_bitmap:
      saveWhat = 1; // force save bitmap
#if defined(__GNUC__) && (__GNUC__ >= 7)
      __attribute__ ((fallthrough));
#endif
    case F_hires:
    default:
      File_error = Save_C64_hires(context, saveWhat, loadAddr);
  }
}


/////////////////////////// pixcen *.GPX ///////////////////////////
void Test_GPX(T_IO_Context * context, FILE * file)
{
  byte header[2];
  (void)context;

  // check for a Zlib compressed stream
  File_error = 1;
  if (!Read_bytes(file, header, 2))
    return;
  if ((header[0] & 0x0f) != 8)
    return;
  if (((header[0] << 8) + header[1]) % 31)
    return;
  File_error = 0;
}

void Load_GPX(T_IO_Context * context)
{
  FILE * file;
  unsigned long file_size;
  byte * buffer;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  file_size = File_length_file(file);
  buffer = malloc(file_size);
  if (buffer == NULL)
  {
    GFX2_Log(GFX2_ERROR, "Failed to allocate %lu bytes.\n", file_size);
    fclose(file);
    return;
  }
  if (Read_bytes(file, buffer, file_size))
  {
    byte * gpx = NULL;
    unsigned long gpx_size = 0;
    int r;

    do
    {
      free(gpx);
      gpx_size += 65536;
      gpx = malloc(gpx_size);
      if (gpx == NULL)
      {
        GFX2_Log(GFX2_ERROR, "Failed to allocate %lu bytes\n", gpx_size);
        break;
      }
      r = uncompress(gpx, &gpx_size, buffer, file_size);
      if (r != Z_BUF_ERROR && r != Z_OK)
        GFX2_Log(GFX2_ERROR, "uncompress() failed with error %d: %s\n", r, zError(r));
    }
    while (r == Z_BUF_ERROR); // there was not enough room in the output buffer
    if (r == Z_OK)
    {
      byte * p;
      dword version, mode;
/*
 mode :
0		BITMAP,
1		MC_BITMAP,
2		SPRITE,
3		MC_SPRITE,
4		CHAR,
5   MC_CHAR,
6		UNUSED1,
7		UNUSED2,
8		UNRESTRICTED,
9		W_UNRESTRICTED
*/
      GFX2_Log(GFX2_DEBUG, "inflated %lu bytes to %lu\n", file_size, gpx_size);
#define READU32LE(p) ((p)[0] | (p)[1] << 8 | (p)[2] << 16 | (p)[3] << 24)
      version = READU32LE(gpx);
      mode = READU32LE(gpx+4);
      GFX2_Log(GFX2_DEBUG, "gpx version %u mode %u\n", version, mode);
      snprintf(context->Comment, COMMENT_SIZE, "pixcen file version %u mode %u", version, mode);
      if (version >= 4)
      {
        dword count;
        const char * key;
        word value[256];
        int xsize = -1;
        int ysize = -1;
        int mapsize = -1;
        int screensize = -1;
        int colorsize = -1;
        int backbuffers = -1;

        count = READU32LE(gpx+8);
        p = gpx + 12;
        while (count--)
        {
          int i = 0;
          int int_value = 0;

          key = (const char *)p;
          while (*p++);
          for (;;)
          {
            value[i] = p[0] + (p[1] << 8);
            p += 2;
            if (value[i] == 0)
              break;
            int_value = int_value * 10 + (value[i] - '0');
            i++;
          }
          GFX2_Log(GFX2_DEBUG, "%s=%d\n", key, int_value);
          if (0 == strcmp(key, "xsize"))
            xsize = int_value;
          else if (0 == strcmp(key, "ysize"))
            ysize = int_value;
          else if (0 == strcmp(key, "mapsize"))
            mapsize = int_value;
          else if (0 == strcmp(key, "screensize"))
            screensize = int_value;
          else if (0 == strcmp(key, "colorsize"))
            colorsize = int_value;
          else if (0 == strcmp(key, "backbuffers"))
            backbuffers = int_value;
        }
//buffersize = 64 + (64 + mapsize + screensize + colorsize) * backbuffers;
        p += 64;  // 64 empty bytes ?
        File_error = 0;
        if (mode & 1)
          context->Ratio = PIXEL_WIDE;
        else
          context->Ratio = PIXEL_SIMPLE;
        Pre_load(context, xsize, ysize, file_size, FORMAT_GPX, context->Ratio, 4); // Do this as soon as you can
        if (Config.Clear_palette)
          memset(context->Palette,0, sizeof(T_Palette));
        C64_set_palette(context->Palette);
        context->Transparent_color=16;

        //foreach backbuffer
        if (backbuffers >= 1)
        {
          byte border, background;
          //byte ext0, ext1, ext2;
          byte * bitmap, * color, * screen;

          //GFX2_LogHexDump(GFX2_DEBUG, "GPX ", p, 0, 64);
          p += 47;  // Extra bytes
          //crippled = p;
          p += 6;
          //lock = p;
          p += 6;
          border = *p++;
          background = *p++;
          /*ext0 = *p++;
          ext1 = *p++;
          ext2 = *p++;*/
          p += 3;
          bitmap = p;
          p += mapsize;
          color = p;
          p += colorsize;
          screen = p;
          p += screensize;

          GFX2_Log(GFX2_DEBUG, "background color #%d, border color #%d\n", (int)background, (int)border);
          Load_C64_multi(context, bitmap, screen, color, background);
          Set_image_mode(context, (mode & 1) ? IMAGE_MODE_C64MULTI : IMAGE_MODE_C64HIRES);
        }
      }
      else
      {
        GFX2_Log(GFX2_ERROR, "GPX file version %d unsupported\n", version);
      }
    }
    free(gpx);
  }
  free(buffer);
  fclose(file);
}

/**
 * Test for SCR file (Amstrad CPC)
 *
 * SCR file format is from "Advanced OCP Art Studio" :
 * http://www.cpcwiki.eu/index.php/Format:Advanced_OCP_Art_Studio_File_Formats
 *
 * .WIN "window" format is also supported.
 *
 * For now we check the presence of a valid PAL file.
 * If the PAL file is not there the pixel data may still be valid.
 * The file size depends on the screen resolution.
 * An AMSDOS header would be a good indication but in some cases it may not
 * be there.
 */
void Test_SCR(T_IO_Context * context, FILE * file)
{
  FILE * pal_file;
  unsigned long pal_size, file_size;
  byte mode, color_anim_flag;

  File_error = 1;

  file_size = File_length_file(file);
  if (file_size > 16384+128)
    return;

  // requires the PAL file
  pal_file = Open_file_read_with_alternate_ext(context, "pal");
  if (pal_file == NULL)
    return;
  /** @todo the palette data can be hidden in the 48 "empty" bytes
   * every 2048 bytes of a standard resolution SCR file.
   * So we should detect the hidden Z80 code and load them.
   * Load address of file is C000. Z80 code :<br>
   * <tt>C7D0: 3a d0 d7 cd 1c bd 21 d1 d7 46 48 cd 38 bc af 21 | :.....!..FH.8..!</tt><br>
   * <tt>C7E0: d1 d7 46 48 f5 e5 cd 32 bc e1 f1 23 3c fe 10 20 | ..FH...2...#<.. </tt><br>
   * <tt>C7F0: f1 c3 18 bb 00 00 00 00 00 00 00 00 00 00 00 00 | ................</tt><br>
   * mode and palette :<br>
   * <tt>D7D0: 00 1a 00 0c 03 0b 01 0d 17 10 02 0f 09 19 06 00 | ................</tt><br>
   * https://gitlab.com/GrafX2/grafX2/merge_requests/121#note_119964168
   */


  pal_size = File_length_file(pal_file);
  if (pal_size == 239+128)
  {
    if (!CPC_check_AMSDOS(pal_file, NULL, NULL))
    {
      fclose(pal_file);
      return;
    }
    fseek(pal_file, 128, SEEK_SET); // right after AMSDOS header
  }
  else if (pal_size != 239)
  {
    fclose(pal_file);
    return;
  }

  if (!Read_byte(pal_file, &mode) || !Read_byte(pal_file, &color_anim_flag))
  {
    fclose(pal_file);
    return;
  }
  GFX2_Log(GFX2_DEBUG, "Test_SCR() mode=%d color animation flag %02X\n", mode, color_anim_flag);
  if (mode <= 2 && (color_anim_flag == 0 || color_anim_flag == 0xff))
    File_error = 0;
  fclose(pal_file);
}

/**
 * Load Advanced OCP Art Studio files (Amstrad CPC)
 *
 * Only standard resolution files (Mode 0 160x200, mode 1 320x200 and
 * mode 2 640x200) are supported. The .PAL file presence is required.
 * "MJH" RLE packing is supported.
 *
 * .WIN "window" format is also supported.
 *
 * @todo Ask user for screen size (or register values) in order to support
 * non standard resolutions.
 */
void Load_SCR(T_IO_Context * context)
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
  FILE * pal_file, * file;
  unsigned long real_file_size, file_size, amsdos_file_size = 0;
  byte mode, color_anim_flag, color_anim_delay;
  byte pal_data[236]; // 12 palettes of 16+1 colors + 16 excluded inks + 16 protected inks
  word width, height = 200;
  byte bpp;
  enum PIXEL_RATIO ratio;
  byte * pixel_data;
  word x, y;
  int i;
  byte sig[3];
  word block_length;
  word win_width, win_height;
  int is_win = 0;
  int columns = 80;

  File_error = 1;
  // requires the PAL file
  pal_file = Open_file_read_with_alternate_ext(context, "pal");
  if (pal_file == NULL)
    return;
  file_size = File_length_file(pal_file);
  if (file_size == 239+128)
  {
    if (!CPC_check_AMSDOS(pal_file, NULL, NULL))
    {
      fclose(pal_file);
      return;
    }
    fseek(pal_file, 128, SEEK_SET); // right after AMSDOS header
  }
  if (!Read_byte(pal_file, &mode) || !Read_byte(pal_file, &color_anim_flag)
      || !Read_byte(pal_file, &color_anim_delay) || !Read_bytes(pal_file, pal_data, 236))
  {
    GFX2_Log(GFX2_WARNING, "Load_SCR() failed to load .PAL file\n");
    fclose(pal_file);
    return;
  }
  fclose(pal_file);
  GFX2_Log(GFX2_DEBUG, "Load_SCR() mode=%d color animation flag=%02X delay=%u\n",
           mode, color_anim_flag, color_anim_delay);
  switch (mode)
  {
    case 0:
      width = 160;
      bpp = 4;
      ratio = PIXEL_WIDE;
      break;
    case 1:
      width = 320;
      bpp = 2;
      ratio = PIXEL_SIMPLE;
      break;
    case 2:
      width = 640;
      bpp = 1;
      ratio = PIXEL_TALL;
      break;
    default:
      return; // unsupported
  }

  if (Config.Clear_palette)
    memset(context->Palette,0,sizeof(T_Palette));
  // Setup the palette (amstrad hardware palette)
  CPC_set_HW_palette(context->Palette + 0x40);

  // Set the palette for this picture
  for (i = 0; i < 16; i++)
    context->Palette[i] = context->Palette[pal_data[12*i]];

  file = Open_file_read(context);
  if (file == NULL)
    return;
  file_size = File_length_file(file);
  real_file_size = file_size;
  if (CPC_check_AMSDOS(file, NULL, &amsdos_file_size))
  {
    if (file_size < (amsdos_file_size + 128))
    {
      GFX2_Log(GFX2_ERROR, "Load_SCR() mismatch in file size. AMSDOS file size %lu, should be %lu\n", amsdos_file_size, file_size - 128);
      fclose(file);
      return;
    }
    else if (file_size > (amsdos_file_size + 128))
      GFX2_Log(GFX2_INFO, "Load_SCR() %lu extra bytes at end of file\n", file_size - 128 - amsdos_file_size);
    fseek(file, 128, SEEK_SET); // right after AMSDOS header
    file_size = amsdos_file_size;
  }
  else
    fseek(file, 0, SEEK_SET);

  if (file_size > 16384)  // we don't support bigger files yet
  {
    fclose(file);
    return;
  }

  if (!Read_bytes(file, sig, 3) || !Read_word_le(file, &block_length))
  {
    fclose(file);
    return;
  }
  fseek(file, -5, SEEK_CUR);

  pixel_data = malloc(16384);
  memset(pixel_data, 0, 16384);

  if (0 != memcmp(sig, "MJH", 3) || block_length > 16384)
  {
    // raw data
    Read_bytes(file, pixel_data, file_size);
    i = file_size;
  }
  else
  {
    // MJH packed format
    i = 0;
    do
    {
      if (!Read_bytes(file, sig, 3) || !Read_word_le(file, &block_length))
        break;
      if (0 != memcmp(sig, "MJH", 3))
        break;
      GFX2_Log(GFX2_DEBUG, "  %.3s block %u\n", sig, block_length);
      file_size -= 5;
      while (block_length > 0)
      {
        byte code;
        if (!Read_byte(file, &code))
          break;
        file_size--;
        if (code == 1)
        {
          byte repeat, value;
          if (!Read_byte(file, &repeat) || !Read_byte(file, &value))
            break;
          file_size -= 2;
          do
          {
            pixel_data[i++] = value;
            block_length--;
          }
          while(--repeat != 0);
        }
        else
        {
          pixel_data[i++] = code;
          block_length--;
        }
      }
      GFX2_Log(GFX2_DEBUG, "  unpacked %d bytes. remaining bytes in file=%lu\n",
               i, file_size);
    }
    while(file_size > 0 && i < 16384);
  }
  fclose(file);

  if (i > 5)
  {
    win_width = pixel_data[i-4] + (pixel_data[i-3] << 8);  // in bits
    win_height = pixel_data[i-2];
    if (((win_width + 7) >> 3) * win_height + 5 == i) // that's a WIN file !
    {
      width = win_width >> (2 - mode);
      height = win_height;
      is_win = 1;
      columns = (win_width + 7) >> 3;
      GFX2_Log(GFX2_DEBUG, ".WIN file detected len=%d (%d,%d) %dcols %02X %02X %02X %02X %02X\n",
          i, width, height, columns,
          pixel_data[i-5], pixel_data[i-4], pixel_data[i-3],
          pixel_data[i-2], pixel_data[i-1]);
    }
    else
    {
      ///@todo guess the picture size, or ask the user
      GFX2_Log(GFX2_DEBUG, ".SCR file. Data length %d\n", i);
      // i <= 16384 && i >= 16336 => Standard resolution
      if (i <= 16384 && i >= 16336)
      {
        int j;
        // Standard resolution files have the 200 lines stored in block
        // of 25 lines of 80 bytes = 2000 bytes every 2048 bytes.
        // so there are 48 bytes unused every 2048 bytes...
        for (j = 0; j < i; j += 2048)
          GFX2_LogHexDump(GFX2_DEBUG, "SCR ", pixel_data, j+2000, 48);
      }
    }
  }

  Pre_load(context, width, height, real_file_size, FORMAT_SCR, ratio, bpp);

  for (y = 0; y < height; y++)
  {
    const byte * line;

    if (is_win)
      line = pixel_data + y * columns;
    else
      line = pixel_data + ((y & 7) << 11) + ((y >> 3) * columns);
    x = 0;
    for (i = 0; i < columns; i++)
    {
      byte pixels = line[i];
      switch (mode)
      {
        case 0:
          Set_pixel(context, x++, y, (pixels & 0x80) >> 7 | (pixels & 0x08) >> 2 | (pixels & 0x20) >> 3 | (pixels & 0x02) << 2);
          Set_pixel(context, x++, y, (pixels & 0x40) >> 6 | (pixels & 0x04) >> 1 | (pixels & 0x10) >> 2 | (pixels & 0x01) << 3);
          break;
        case 1:
          do {
            // upper nibble is 4 lower color bits, lower nibble is 4 upper color bits
            Set_pixel(context, x++, y, (pixels & 0x80) >> 7 | (pixels & 0x08) >> 2);
            pixels <<= 1;
          }
          while ((x & 3) != 0);
          break;
        case 2:
          do {
            Set_pixel(context, x++, y, (pixels & 0x80) >> 7);
            pixels <<= 1;
          }
          while ((x & 7) != 0);
      }
    }
  }

  free(pixel_data);

  File_error = 0;
}

/**
 * Save Amstrad SCR file
 *
 * guess mode from aspect ratio :
 * - normal pixels are mode 1
 * - wide pixels are mode 0
 * - tall pixels are mode 2
 *
 * Mode and palette are stored in a .PAL file.
 *
 * The picture color index should be 0-15,
 * The CPC Hardware palette is expected to be set (indexes 64 to 95)
 *
 * @todo Add possibility to set R9, R12, R13 values
 * @todo Add OCP packing support
 * @todo Add possibility to include AMSDOS header, with proper loading
 *       address guessed from r12/r13 values.
 */
void Save_SCR(T_IO_Context * context)
{
  int i, j;
  unsigned char* output;
  unsigned long outsize = 0;
  unsigned char r1 = 0;
  int cpc_mode;
  FILE* file;


  switch(Pixel_ratio)
  {
    case PIXEL_WIDE:
    case PIXEL_WIDE2:
      cpc_mode = 0;
      break;
    case PIXEL_TALL:
    case PIXEL_TALL2:
    case PIXEL_TALL3:
      cpc_mode = 2;
      break;
    default:
      cpc_mode = 1;
      break;
  }

  file = Open_file_write_with_alternate_ext(context, "pal");
  if (file == NULL)
    return;
  if (!Write_byte(file, cpc_mode) || !Write_byte(file, 0) || !Write_byte(file, 0))
  {
    fclose(file);
    return;
  }
  for (i = 0; i < 16; i++)
  {
    // search for the color in the HW palette (0x40-0x5F)
    byte index = 0x40;
    while ((index < 0x60) &&
        !CPC_compare_colors(context->Palette + i, context->Palette + index))
      index++;
    if (index >= 0x60)
    {
      GFX2_Log(GFX2_WARNING, "Save_SCR() color #%i not found in CPC HW palette.\n", i);
      index = 0x54 - i; // default
    }
    for (j = 0; j < 12; j++)  // write the same color for the 12 frames
    {
      Write_byte(file, index);
    }
  }
  // border
  for (j = 0; j < 12; j++)
  {
    Write_byte(file, 0x54); // black
  }
  // excluded inks
  for (i = 0; i < 16; i++)
  {
    Write_byte(file, 0);
  }
  // protected inks
  for (i = 0; i < 16; i++)
  {
    Write_byte(file, 0);
  }
  fclose(file);

  output = raw2crtc(context, cpc_mode, 7, &outsize, &r1, 0x0C, 0);
  GFX2_Log(GFX2_DEBUG, "Save_SCR() output=%p outsize=%lu r1=$%02X\n", output, outsize, r1);

  if (output == NULL)
    return;

  file = Open_file_write(context);
  if (file == NULL)
    File_error = 1;
  else
  {
    File_error = 0;
    if (!Write_bytes(file, output, outsize))
      File_error = 1;
    fclose(file);
  }
  free (output);
}

/**
 * Test for CM5 - Amstrad CPC "Mode 5" picture
 *
 * This is a format designed by SyX.
 * There is one .GFX file in the usual amstrad format
 * and a .CM5 file with the palette, which varies over time.
 *
 * CM5 file is 2049 bytes, GFX is 18432 bytes.
 *
 * @todo check CM5 contains only valid values [0x40-0x5f]
 */
void Test_CM5(T_IO_Context * context, FILE * file)
{
  // check cm5 file size == 2049 bytes
  FILE *file_gfx;
  long file_size;

  File_error = 1;

  file_size = File_length_file(file);
  if (file_size != 2049)
    return;

  // check existence of a .GFX file with the same name
  file_gfx = Open_file_read_with_alternate_ext(context, "gfx");
  if (file_gfx == NULL)
    return;
  file_size = File_length_file(file_gfx);
  fclose(file_gfx);
  if (file_size != 18432)
    return;

  File_error = 0;
}


/**
 * Load Amstrad CPC "Mode 5" picture
 *
 * Only support 288x256 resolution as the Mode 5 Viewer app only handles this
 * single resoltion.
 */
void Load_CM5(T_IO_Context* context)
{
  // Ensure "8bit" constraint mode is switched on
  // Set palette to the CPC hardware colors
  // Load the palette data to the 4 colorlayers
  FILE *file;
  byte value = 0;
  int mod=0;
  short line = 0;
  int tx, ty;
  // for preview :
  byte ink0;
  byte ink1[256];
  byte ink2[256];
  byte ink3[256*6];

  if (!(file = Open_file_read(context)))
  {
      File_error = 1;
      return;
  }

  Pre_load(context, 48*6, 256, 2049, FORMAT_CM5, PIXEL_SIMPLE, 0);

  if (Config.Clear_palette)
  {
    memset(context->Palette,0,sizeof(T_Palette));
    // setup colors 0,1,2,3 to see something in the thumbnail preview of layer 5
    context->Palette[1].R = 60;
    context->Palette[2].B = 60;
    context->Palette[3].G = 60;
  }

  // Setup the palette (amstrad hardware palette)
  CPC_set_HW_palette(context->Palette + 0x40);

  First_color_in_palette = 64;

  if (!Read_byte(file, &ink0))
    File_error = 2;

  // This forces the creation of 5 layers total :
  // Needed because the "pixel" functions will seek layer 4
  Set_loading_layer(context, 4);
  // Now select layer 1 again
  Set_loading_layer(context, 0);

  if (context->Type == CONTEXT_MAIN_IMAGE)
  {
    Set_image_mode(context, IMAGE_MODE_MODE5);

    // Fill layer with color we just read (Layer 1 - INK 0)
    for(ty=0; ty<context->Height; ty++)
      for(tx=0; tx<context->Width; tx++)
        Set_pixel(context, tx, ty, ink0);
  }

  while(Read_byte(file, &value))
  {
    switch(mod)
    {
      case 0:
        // This is color for layer 2 - INK 1
        Set_loading_layer(context, 1);
        for(tx=0; tx<context->Width; tx++)
          Set_pixel(context, tx, line, value);
        ink1[line] = value;
        break;
      case 1:
        // This is color for layer 3 - INK 2
        Set_loading_layer(context, 2);
        for(tx=0; tx<context->Width; tx++)
          Set_pixel(context, tx, line, value);
        ink2[line] = value;
        break;
      default:
        // This is color for a block in layer 4 - INK 3
        Set_loading_layer(context, 3);
        for(tx=(mod-2)*48; tx<(mod-1)*48; tx++)
          Set_pixel(context, tx, line, value);
        ink3[line*6+(mod-2)] = value;
        break;
    }
    mod++;
    if (mod > 7)
    {
      mod = 0;
      line++;
    }
  }

  fclose(file);

  // Load the pixeldata to the 5th layer
  file = Open_file_read_with_alternate_ext(context, "gfx");
  if (file == NULL)
  {
    File_error = 1;
    return;
  }
  Set_loading_layer(context, 4);

  if (context->Type == CONTEXT_PREVIEW)
    for (ty = 0; ty < 256; ty++)
      for (tx = 0; tx < 48*6; )
      {
        Read_byte(file, &value);
        for (mod = 0; mod < 4; mod++, tx++, value <<= 1)
        {
          switch(3 ^ (((value&0x80) >> 7) | ((value&0x8)>>2)))  // INK
          {
            case 0:
              Set_pixel(context, tx, ty, ink0);
              break;
            case 1:
              Set_pixel(context, tx, ty, ink1[ty]);
              break;
            case 2:
              Set_pixel(context, tx, ty, ink2[ty]);
              break;
            default:
              Set_pixel(context, tx, ty, ink3[ty*6+(tx/48)]);
          }
        }
      }
  else
    for (ty = 0; ty < 256; ty++)
      for (tx = 0; tx < 48*6; )
      {
        Read_byte(file, &value);
        Set_pixel(context, tx++, ty, 3 ^ (((value&0x80) >> 7) | ((value&0x8)>>2)));
        Set_pixel(context, tx++, ty, 3 ^ (((value&0x40) >> 6) | ((value&0x4)>>1)));
        Set_pixel(context, tx++, ty, 3 ^ (((value&0x20) >> 5) | ((value&0x2)>>0)));
        Set_pixel(context, tx++, ty, 3 ^ (((value&0x10) >> 4) | ((value&0x1)<<1)));
      }

  fclose(file);

}


void Save_CM5(T_IO_Context* context)
{
  FILE* file;
  int tx, ty;

  // TODO: Check picture has 5 layers
  // TODO: Check the constraints on the layers
  // Layer 1 : 1 color Only
  // Layer 2 and 3 : 1 color/line
  // Layer 4 : 1 color / 48x1 block
  // TODO: handle filesize

  if (!(file = Open_file_write(context)))
  {
    File_error = 1;
    return;
  }
  setvbuf(file, NULL, _IOFBF, 64*1024);

  // Write layer 0
  Set_saving_layer(context, 0);
  Write_byte(file, Get_pixel(context, 0, 0));
  for(ty = 0; ty < 256; ty++)
  {
    Set_saving_layer(context, 1);
    Write_byte(file, Get_pixel(context, 0, ty));
    Set_saving_layer(context, 2);
    Write_byte(file, Get_pixel(context, 0, ty));
    Set_saving_layer(context, 3);
    for(tx = 0; tx < 6; tx++)
    {
      Write_byte(file, Get_pixel(context, tx*48, ty));
    }
  }

  fclose(file);

  // Now the pixeldata
  if (!(file = Open_file_write_with_alternate_ext(context, "gfx")))
  {
    File_error = 2;
    return;
  }
  setvbuf(file, NULL, _IOFBF, 64*1024);

  Set_saving_layer(context, 4);

  for (ty = 0; ty < 256; ty++)
  {
    for (tx = 0; tx < 48*6; tx+=4)
    {
      byte code = 0;
      byte pixel;

      pixel = 3-Get_pixel(context, tx+3, ty);
      code |= (pixel&2)>>1 | ((pixel & 1)<<4);
      pixel = 3-Get_pixel(context, tx+2, ty);
      code |= ((pixel&2)<<0) | ((pixel & 1)<<5);
      pixel = 3-Get_pixel(context, tx+1, ty);
      code |= ((pixel&2)<<1) | ((pixel & 1)<<6);
      pixel = 3-Get_pixel(context, tx, ty);
      code |= ((pixel&2)<<2) | ((pixel & 1)<<7);
      Write_byte(file, code);
    }
  }

  fclose(file);
  File_error = 0;

}


/* Amstrad CPC 'PPH' for Perfect Pix.
// This is a format designed by Rhino.
// There are 3 modes:
// - Mode 'R': 1:1 pixels, 16 colors from the CPC 27 color palette.
//   (this is implemented on CPC as two pictures with wide pixels, the "odd" one
//   being shifted half a pixel to the right), and flipping)
// - Mode 'B0': wide pixels, up to 126 out of 378 colors.
//   (this is implemented as two pictures with wide pixels, sharing the same 16
//   color palette, and flipping)
// - Mode 'B1': 1:1 pixels, 1 fixed color, up to 34 palettes of 9 colors
//   (actually 4 colors + flipping)
//
// - The standard CPC formats can also be encapsulated into a PPH file.
//
// http://www.pouet.net/prod.php?which=67770#c766959
*/
void Test_PPH(T_IO_Context * context, FILE * file)
{
  FILE *file_oddeve;
  byte buffer[6];
  unsigned long file_size;
  unsigned int w, h;
  unsigned int expected;

  File_error = 1;

  // First check file size is large enough to hold the header
  file_size = File_length_file(file);
  if (file_size < 11) {
    File_error = 1;
    return;
  }

  // File is large enough for the header, now check if the data makes some sense
  if (!Read_bytes(file, buffer, 6))
    return;
  if (buffer[0] > 5) {
    // Unknown mode
    File_error = 2;
    return;
  }

  w = buffer[1] | (buffer[2] << 8);
  if (w < 2 || w > 384) {
    // Invalid width
    File_error = 3;
    return;
  }

  h = buffer[3] | (buffer[4] << 8);
  if (h < 1 || h > 272) {
    // Invalid height
    File_error = 4;
    return;
  }

  if (buffer[5] < 1 || buffer[5] > 28)
  {
    // Invalid palettes count
    File_error = 5;
    return;
  }
  expected = 6; // Size of header
  switch(buffer[0])
  {
    case 0:
    case 3:
    case 4:
      // Palette size should be 16 bytes, only 1 palette.
      if (buffer[5] != 1) {
        File_error = 7;
        return;
      }
      expected += 16;
      break;

    case 1:
    case 5:
      expected += buffer[5] * 5 - 1;
      break;

    case 2:
      // Palette size should be 2 bytes
      if (buffer[5] != 1) {
        File_error = 7;
        return;
      }
      expected += 2;
      break;
  }

  if (file_size != expected)
  {
    File_error = 6;
    return;
  }

  // check existence of .ODD/.EVE files with the same name
  // and the right size
  expected = w * h / 4;
  file_oddeve = Open_file_read_with_alternate_ext(context, "odd");
  if (file_oddeve == NULL)
    return;
  file_size = File_length_file(file_oddeve);
  fclose (file_oddeve);
  if (file_size != expected)
  {
    File_error = 8;
    return;
  }
  file_oddeve = Open_file_read_with_alternate_ext(context, "eve");
  if (file_oddeve == NULL)
    return;
  file_size = File_length_file(file_oddeve);
  fclose(file_oddeve);
  if (file_size != expected)
  {
    File_error = 8;
    return;
  }
  File_error = 0;
}


static uint8_t pph_blend(uint8_t a, uint8_t b)
{
	uint32_t h,l;
	if (a > b) { h = a; l = b; }
	else       { h = b; l = a; }

	return (23 * h + 9 * l) / 32;
}


void Load_PPH(T_IO_Context* context)
{
  FILE *file;
  FILE *feven;

  // Read in the header
  uint8_t mode;
  uint16_t width;
  uint16_t height;
  uint8_t npal;
  int i,j;
  uint8_t a,b,c,d;
  int file_size;
  uint8_t pl[16];

  static const T_Components CPCPAL[27] =
  {
      { 0x00, 0x02, 0x01 }, { 0x00, 0x02, 0x6B }, { 0x0C, 0x02, 0xF4 },
      { 0x6C, 0x02, 0x01 }, { 0x69, 0x02, 0x68 }, { 0x6C, 0x02, 0xF2 },
      { 0xF3, 0x05, 0x06 }, { 0xF0, 0x02, 0x68 }, { 0xF3, 0x02, 0xF4 },
      { 0x02, 0x78, 0x01 }, { 0x00, 0x78, 0x68 }, { 0x0C, 0x7B, 0xF4 },
      { 0x6E, 0x7B, 0x01 }, { 0x6E, 0x7D, 0x6B }, { 0x6E, 0x7B, 0xF6 },
      { 0xF3, 0x7D, 0x0D }, { 0xF3, 0x7D, 0x6B }, { 0xFA, 0x80, 0xF9 },
      { 0x02, 0xF0, 0x01 }, { 0x00, 0xF3, 0x6B }, { 0x0F, 0xF3, 0xF2 },
      { 0x71, 0xF5, 0x04 }, { 0x71, 0xF3, 0x6B }, { 0x71, 0xF3, 0xF4 },
      { 0xF3, 0xF3, 0x0D }, { 0xF3, 0xF3, 0x6D }, { 0xFF, 0xF3, 0xF9 }
  };

  if (!(file = Open_file_read(context)))
  {
      File_error = 1;
      return;
  }

  file_size=File_length_file(file);

  Read_byte(file, &mode);
  Read_word_le(file, &width);
  Read_word_le(file, &height);
  Read_byte(file, &npal);

  if (npal > 16)
      npal = 16;

  // Switch to the proper aspect ratio
  switch (mode)
  {
      case 0:
      case 4:
        context->Ratio = PIXEL_WIDE;
        width /= 2;
        break;

      case 2:
        context->Ratio = PIXEL_TALL;
        break;

      case 1:
      case 5:
      case 3:
        context->Ratio = PIXEL_SIMPLE;
        break;
  }

  Pre_load(context, width, height, file_size, FORMAT_PPH, context->Ratio, 0);

  context->Width = width;
  context->Height = height;

  // First of all, detect the mode
  // 0, 1, 2 > Load as with SCR files?
  // R(3)    > Load as single layer, square pixels, 16 colors
  // B0(4)   > Load as single layer, wide pixels, expand palette with colorcycling
  // B1(5)   > Load as ???
  //           Maybe special mode similar to mode5, with 2 layers + auto-flicker?

  switch (mode)
  {
      case 0:
      case 3: // R
          // 16-color palette
          for (i = 0; i < 16; i++)
          {
              uint8_t color;
              Read_byte(file, &color);
              context->Palette[i] = CPCPAL[color];
          }
          break;

      case 1:
      case 5: // B1
      {
          // Single or multiple 4-color palettes
          uint8_t base[4];
          for (j = 0; j < npal; j++)
          {
            for (i = 0; i < 4; i++)
            {
              Read_byte(file,&base[i]);
            }
            for (i = 0; i < 16; i++)
            {
              context->Palette[i + 16*j].R = pph_blend(
                  CPCPAL[base[i & 3]].R, CPCPAL[base[i >> 2]].R);
              context->Palette[i + 16*j].G = pph_blend(
                  CPCPAL[base[i & 3]].G, CPCPAL[base[i >> 2]].G);
              context->Palette[i + 16*j].B = pph_blend(
                  CPCPAL[base[i & 3]].B, CPCPAL[base[i >> 2]].B);
            }
            // TODO this byte marks where this palette stops being used and the
            // next starts. We must handle this!
            Read_byte(file,&pl[j]);
          }
          pl[npal - 1] = 255;
          break;
      }

      case 2:
          // Single 2-color palette
          break;

      case 4: // B0
      {
          // Single 16-color palette + flipping, need to expand palette and
          // setup colorcycling ranges.
          uint8_t base[16];
          for (i = 0; i < 16; i++)
          {
              Read_byte(file,&base[i]);
          }

          for (i = 0; i < 256; i++)
          {
              context->Palette[i].R = pph_blend(
                  CPCPAL[base[i & 15]].R, CPCPAL[base[i >> 4]].R);
              context->Palette[i].G = pph_blend(
                  CPCPAL[base[i & 15]].G, CPCPAL[base[i >> 4]].G);
              context->Palette[i].B = pph_blend(
                  CPCPAL[base[i & 15]].B, CPCPAL[base[i >> 4]].B);
          }
      }
      break;
  }

  fclose(file);

  // Load the picture data
  // There are two pages, each storing bytes in the CPC vram format but lines in
  // linear order.
  file = Open_file_read_with_alternate_ext(context, "odd");
  if (file == NULL)
  {
    File_error = 3;
    return;
  }
  feven = Open_file_read_with_alternate_ext(context, "eve");
  if (feven == NULL)
  {
    File_error = 4;
    fclose(file);
    return;
  }

  c = 0;
  d = 0;

  for (j = 0; j < height; j++)
  {
      for (i = 0; i < width;)
      {
          uint8_t even, odd;
          Read_byte(feven, &even);
          Read_byte(file, &odd);

          switch (mode)
          {
              case 4:
                a = ((even & 0x02) << 2) | ((even & 0x08) >> 2)
                  | ((even & 0x20) >> 3) | ((even & 0x80) >> 7);
                a <<= 4;
                a |= ((odd & 0x02) << 2) | (( odd & 0x08) >> 2)
                  | (( odd & 0x20) >> 3) | (( odd & 0x80) >> 7);

                b = ((even & 0x01) << 3) | ((even & 0x04) >> 1)
                  | ((even & 0x10) >> 2) | ((even & 0x40) >> 6);
                b <<= 4;
                b |= ((odd & 0x01) << 3) | (( odd & 0x04) >> 1)
                  | (( odd & 0x10) >> 2) | (( odd & 0x40) >> 6);

                Set_pixel(context, i++, j, a);
                Set_pixel(context, i++, j, b);
                break;

              case 3:
                a = ((even & 0x02) << 2) | ((even & 0x08) >> 2)
                  | ((even & 0x20) >> 3) | ((even & 0x80) >> 7);
                b = (( odd & 0x02) << 2) | (( odd & 0x08) >> 2)
                  | (( odd & 0x20) >> 3) | (( odd & 0x80) >> 7);
                c = ((even & 0x01) << 3) | ((even & 0x04) >> 1)
                  | ((even & 0x10) >> 2) | ((even & 0x40) >> 6);
                d = (( odd & 0x01) << 3) | (( odd & 0x04) >> 1)
                  | (( odd & 0x10) >> 2) | (( odd & 0x40) >> 6);
                Set_pixel(context, i++, j, j & 1 ? b : a);
                Set_pixel(context, i++, j, j & 1 ? a : b);
                Set_pixel(context, i++, j, j & 1 ? d : c);
                Set_pixel(context, i++, j, j & 1 ? c : d);
                break;

              case 5:
                if (d >= pl[c])
                {
                    d = 0;
                    c++;
                }
                a = ((even & 0x80) >> 6) | ((even & 0x08) >> 3);
                b = (( odd & 0x80) >> 6) | (( odd & 0x08) >> 3);
                Set_pixel(context, i++, j, a + (b << 2) + c * 16);
                a = ((even & 0x40) >> 5) | ((even & 0x04) >> 2);
                b = (( odd & 0x40) >> 5) | (( odd & 0x04) >> 2);
                Set_pixel(context, i++, j, a + (b << 2) + c * 16);
                a = ((even & 0x20) >> 4) | ((even & 0x02) >> 1);
                b = (( odd & 0x20) >> 4) | (( odd & 0x02) >> 1);
                Set_pixel(context, i++, j, a + (b << 2) + c * 16);
                a = ((even & 0x10) >> 3) | ((even & 0x01) >> 0);
                b = (( odd & 0x10) >> 3) | (( odd & 0x01) >> 0);
                Set_pixel(context, i++, j, a + (b << 2) + c * 16);

                break;

              default:
                File_error = 2;
                return;
          }

      }
      d++;
  }
  fclose(file);
  fclose(feven);

  File_error = 0;
}

void Save_PPH(T_IO_Context* context)
{
  (void)context; // unused
    // TODO

    // Detect mode
    // Wide pixels => B0 (4)
    // Square pixels:
    // - 16 colors used => R
    // - more colors used => B1 (if <16 colors per line)

    // Check palette
    // B0: use diagonal: 0, 17, 34, ... (assume the other are mixes)
    // R: use 16 used colors (or 16 first?)
    // B1: find the 16 colors used in a line? Or assume they are in-order already?
}


/////////////////////////////////// FLI/FLC /////////////////////////////////
typedef struct {
  dword size;          /* Size of FLIC including this header */
  word  type;          /* File type 0xAF11, 0xAF12, 0xAF30, 0xAF44, ... */
  word  frames;        /* Number of frames in first segment */
  word  width;         /* FLIC width in pixels */
  word  height;        /* FLIC height in pixels */
  word  depth;         /* Bits per pixel (usually 8) */
  word  flags;         /* Set to zero or to three */
  dword speed;         /* Delay between frames */
  word  reserved1;     /* Set to zero */
  dword created;       /* Date of FLIC creation (FLC only) */
  dword creator;       /* Serial number or compiler id (FLC only) */
  dword updated;       /* Date of FLIC update (FLC only) */
  dword updater;       /* Serial number (FLC only), see creator */
  word  aspect_dx;     /* Width of square rectangle (FLC only) */
  word  aspect_dy;     /* Height of square rectangle (FLC only) */
  word  ext_flags;     /* EGI: flags for specific EGI extensions */
  word  keyframes;     /* EGI: key-image frequency */
  word  totalframes;   /* EGI: total number of frames (segments) */
  dword req_memory;    /* EGI: maximum chunk size (uncompressed) */
  word  max_regions;   /* EGI: max. number of regions in a CHK_REGION chunk */
  word  transp_num;    /* EGI: number of transparent levels */
  byte  reserved2[24]; /* Set to zero */
  dword oframe1;       /* Offset to frame 1 (FLC only) */
  dword oframe2;       /* Offset to frame 2 (FLC only) */
  byte  reserved3[40]; /* Set to zero */
} T_FLIC_Header;

static void Load_FLI_Header(FILE * file, T_FLIC_Header * header)
{
  if (!(Read_dword_le(file,&header->size)
      && Read_word_le(file,&header->type)
      && Read_word_le(file,&header->frames)
      && Read_word_le(file,&header->width)
      && Read_word_le(file,&header->height)
      && Read_word_le(file,&header->depth)
      && Read_word_le(file,&header->flags)
      && Read_dword_le(file,&header->speed)
      && Read_word_le(file,&header->reserved1)
      && Read_dword_le(file,&header->created)
      && Read_dword_le(file,&header->creator)
      && Read_dword_le(file,&header->updated)
      && Read_dword_le(file,&header->updater)
      && Read_word_le(file,&header->aspect_dx)
      && Read_word_le(file,&header->aspect_dy)
      && Read_word_le(file,&header->ext_flags)
      && Read_word_le(file,&header->keyframes)
      && Read_word_le(file,&header->totalframes)
      && Read_dword_le(file,&header->req_memory)
      && Read_word_le(file,&header->max_regions)
      && Read_word_le(file,&header->transp_num)
      && Read_bytes(file,header->reserved2,24)
      && Read_dword_le(file,&header->oframe1)
      && Read_dword_le(file,&header->oframe2)
      && Read_bytes(file,header->reserved2,40) ))
  {
    File_error=1;
  }
}

/**
 * Test for the Autodesk Animator FLI/FLC format.
 *
 * Not to be confused with Commodore 64 FLI.
 */
void Test_FLI(T_IO_Context * context, FILE * file)
{
  T_FLIC_Header header;
  (void)context;

  File_error=0;
  Load_FLI_Header(file, &header);
  if (File_error != 0) return;

  switch (header.type)
  {
    case 0xAF11:  // standard FLI
    case 0xAF12:  // FLC (8bpp)
#if 0
    case 0xAF30:  // Huffman or BWT compression
    case 0xAF31:  // frame shift compression
    case 0xAF44:  // bpp != 8
#endif
      File_error=0;
      break;
    default:
      File_error=1;
  }
}

/**
 * Load file in the Autodesk Animator FLI/FLC format.
 *
 * Not to be confused with Commodore 64 FLI.
 */
void Load_FLI(T_IO_Context * context)
{
  FILE * file;
  unsigned long file_size;
  T_FLIC_Header header;
  dword chunk_size;
  word chunk_type;
  word sub_chunk_count, sub_chunk_index;
  dword sub_chunk_size;
  word sub_chunk_type;
  word frame_delay, frame_width, frame_height;
  int current_frame = 0;

  file = Open_file_read(context);
  if (file == NULL)
  {
    File_error=1;
    return;
  }
  File_error=0;
  file_size = File_length_file(file);
  Load_FLI_Header(file, &header);
  if (File_error != 0)
  {
    fclose(file);
    return;
  }
  if (header.size == 12)
  {
    // special "magic carpet" format
    header.depth = 8;
    header.speed = 66; // about 15fps
    fseek(file, 12, SEEK_SET);
  }
  else if (file_size != header.size)
    Warning("Load_FLI(): file size mismatch in header");

  if (header.speed == 0)
  {
    if (header.type == 0xAF11) // FLI
      header.speed = 1;   // 1/70th seconds
    else
      header.speed = 10;  // 10ms
  }

  while (File_error == 0
     && Read_dword_le(file,&chunk_size) && Read_word_le(file,&chunk_type))
  {
    chunk_size -= 6;
    switch (chunk_type)
    {
      case 0xf1fa:  // FRAME
        Read_word_le(file, &sub_chunk_count);
        Read_word_le(file, &frame_delay);
        fseek(file, 2, SEEK_CUR);
        Read_word_le(file, &frame_width);
        Read_word_le(file, &frame_height);
        if (frame_width == 0)
          frame_width = header.width;
        if (frame_height == 0)
          frame_height = header.height;
        if (frame_delay == 0)
          frame_delay = header.speed;
        chunk_size -= 10;

        if (current_frame == 0)
        {
          Pre_load(context, header.width,header.height,file_size,FORMAT_FLI,PIXEL_SIMPLE,header.depth);
          Set_image_mode(context, IMAGE_MODE_ANIMATION);
          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));
        }
        else
        {
          Set_loading_layer(context, current_frame);
          if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
          {
            // Copy the content of previous frame
            memcpy(
                Main.backups->Pages->Image[Main.current_layer].Pixels,
                Main.backups->Pages->Image[Main.current_layer-1].Pixels,
                Main.backups->Pages->Width*Main.backups->Pages->Height);
          }
        }
        if (header.type == 0xAF11) // FLI
          Set_frame_duration(context, (frame_delay * 100) / 7); // 1/70th sec
        else
          Set_frame_duration(context, frame_delay); // msec
        current_frame++;

        for (sub_chunk_index = 0; sub_chunk_index < sub_chunk_count; sub_chunk_index++)
        {
          if (!(Read_dword_le(file,&sub_chunk_size) && Read_word_le(file,&sub_chunk_type)))
            File_error = 1;
          else
          {
            chunk_size -= sub_chunk_size;
            sub_chunk_size -= 6;
            if (sub_chunk_type == 0x04 || sub_chunk_type == 0x0b)   // color map
            {
              word packet_count;
              int i = 0;
              sub_chunk_size -= 2;
              if (!Read_word_le(file, &packet_count))
                File_error = 1;
              else
                while (packet_count-- > 0 && File_error == 0)
                {
                  byte skip, count;
                  if (!(Read_byte(file, &skip) && Read_byte(file, &count)))
                    File_error = 1;
                  else
                  {
                    sub_chunk_size -= 2;
                    i += skip;  // count 0 means 256
                    do
                    {
                      byte r, g, b;
                      if (!(Read_byte(file, &r) && Read_byte(file, &g) && Read_byte(file, &b)))
                      {
                        File_error = 1;
                        break;
                      }
                      if (sub_chunk_type == 0x0b || header.size == 12) // 6bit per color
                      {
                        r = (r << 2) | (r >> 4);
                        g = (g << 2) | (g >> 4);
                        b = (b << 2) | (b >> 4);
                      }
                      context->Palette[i].R = r;
                      context->Palette[i].G = g;
                      context->Palette[i].B = b;
                      i++;
                      sub_chunk_size -= 3;
                    } while (--count != 0);
                  }
                }
            }
            else if (sub_chunk_type == 0x0f)  // full frame RLE
            {
              word x, y;
              for (y = 0; y < frame_height && File_error == 0; y++)
              {
                byte count, data;
                Read_byte(file, &count); // packet count, but dont rely on it
                sub_chunk_size--;
                for (x = 0; x < frame_width; )
                {
                  if (!Read_byte(file, &count))
                  {
                    File_error = 1;
                    break;
                  }
                  sub_chunk_size--;
                  if ((count & 0x80) == 0)
                  {
                    if (!Read_byte(file, &data))  // repeat data count times
                    {
                      File_error = 1;
                      break;
                    }
                    sub_chunk_size--;
                    while (count-- > 0 && x < frame_width)
                      Set_pixel(context, x++, y, data);
                  }
                  else
                    while (count++ != 0 && x < frame_width)  // copy count bytes
                    {
                      if (!Read_byte(file, &data))
                      {
                        File_error = 1;
                        break;
                      }
                      Set_pixel(context, x++, y, data);
                      sub_chunk_size--;
                    }
                }
              }
              if (context->Type == CONTEXT_PREVIEW || context->Type == CONTEXT_PREVIEW_PALETTE)
              { // load only 1st frame in preview
                fclose(file);
                return;
              }
            }
            else if (sub_chunk_type == 0x0c)  // delta image, RLE
            {
              word x, y, line_count;

              Read_word_le(file, &y);
              Read_word_le(file, &line_count);
              sub_chunk_size -= 4;
              while (sub_chunk_size > 0 && line_count > 0 && File_error == 0)
              {
                byte packet_count;

                x = 0;
                if (!Read_byte(file, &packet_count))
                  File_error = 1;
                else
                {
                  sub_chunk_size--;
                  while (packet_count-- > 0 && File_error == 0)
                  {
                    byte skip, count, data;
                    if (!(Read_byte(file, &skip) && Read_byte(file, &count)))
                      File_error = 1;
                    else
                    {
                      sub_chunk_size -= 2;
                      x += skip;
                      if (count & 0x80)
                      {
                        Read_byte(file, &data);
                        sub_chunk_size--;
                        while (count++ != 0)
                          Set_pixel(context, x++, y, data);
                      }
                      else
                        while (count-- > 0)
                        {
                          Read_byte(file, &data);
                          sub_chunk_size--;
                          Set_pixel(context, x++, y, data);
                        }
                    }
                  }
                }
                y++;
                line_count--;
              }
            }
            else if (sub_chunk_type == 0x07)  // FLC delta image
            {
              word opcode, y, line_count;

              y = 0;
              Read_word_le(file, &line_count);
              sub_chunk_size -= 2;
              while (line_count > 0)
              {
                Read_word_le(file, &opcode);
                sub_chunk_size -= 2;
                if ((opcode & 0xc000) == 0x0000) // packet count
                {
                  word x = 0;
                  while (opcode-- > 0)
                  {
                    byte skip, count, data1, data2;
                    if (!(Read_byte(file, &skip) && Read_byte(file, &count)))
                      File_error = 1;
                    else
                    {
                      sub_chunk_size -= 2;
                      x += skip;
                      if (count & 0x80)
                      {
                        Read_byte(file, &data1);
                        Read_byte(file, &data2);
                        sub_chunk_size -= 2;
                        while (count++ != 0)
                        {
                          Set_pixel(context, x++, y, data1);
                          Set_pixel(context, x++, y, data2);
                        }
                      }
                      else
                        while (count-- > 0)
                        {
                          Read_byte(file, &data1);
                          Set_pixel(context, x++, y, data1);
                          Read_byte(file, &data2);
                          Set_pixel(context, x++, y, data2);
                          sub_chunk_size -= 2;
                        }
                    }
                  }
                  y++;
                  line_count--;
                }
                else if ((opcode & 0xc000) == 0xc000)  // line skip
                {
                  y -= opcode;
                }
                else if ((opcode & 0xc000) == 0x8000)  // last byte
                {
                  Set_pixel(context, frame_width - 1, y, opcode & 0xff);
                }
                else
                {
                  Warning("Unsupported opcode");
                  File_error = 2;
                  break;
                }
              }
            }
            if (sub_chunk_size > 0)
            {
              fseek(file, sub_chunk_size, SEEK_CUR);
            }
          }
        }
        break;
      default:  // skip
        Warning("Load_FLI(): unrecognized chunk");
    }
    if (chunk_size > 0 && header.size != 12)
    {
      fseek(file, chunk_size, SEEK_CUR);
    }
  }
  fclose(file);
}

/////////////////////////////// Thomson Files ///////////////////////////////

/**
 * Test for Thomson file
 */
void Test_MOTO(T_IO_Context * context, FILE * file)
{
  long file_size;

  file_size = File_length_file(file);

  File_error = 1;
  if (file_size <= 10)
    return;
  switch (MOTO_Check_binary_file(file))
  {
    case 0: // Not Thomson binary format
      switch (file_size)
      {
        // Files in RAW formats (from TGA2teo)
        case 8004:  // 2 colors palette
        case 8008:  // 4 colors palette
        case 8032:  // 16 colors palette
          {
            char * filename;
            char * path;
            char * ext;

            // Check there are both FORME and COULEUR files
            filename = strdup(context->File_name);
            ext = strrchr(filename, '.');
            if (ext == NULL || ext == filename)
            {
              free(filename);
              return;
            }
            if ((ext[-1] | 32) == 'c')
              ext[-1] = (ext[-1] & 32) | 'P';
            else if ((ext[-1] | 32) == 'p')
              ext[-1] = (ext[-1] & 32) | 'C';
            else
            {
              free(filename);
              return;
            }
            path = Filepath_append_to_dir(context->File_directory, filename);
            if (File_exists(path))
              File_error = 0;
            free(path);
            free(filename);
          }
          return;
        default:
          break;
      }
      break;
    case 2: // MAP file (SAVEP/LOADP)
    case 3: // TO autoloading picture
    case 4: // MO autoloading picture
      File_error = 0;
      return;
  }
}

/**
 * Load a picture for Thomson TO8/TO8D/TO9/TO9+/MO6
 *
 * One of the supported format is the one produced by TGA2Teo :
 * - Picture data is splitted into 2 files, one for each VRAM bank :
 *   - The first VRAM bank is called "forme" (shape).
 *     In 40col mode it stores pixels.
 *   - The second VRAM bank is called "couleur" (color).
 *     In 40col mode it store color indexes for foreground and background.
 * - File extension is .BIN, character before extension is "P" for the first
 *   file, and "C" for the second.
 * - The color palette is stored in both files after the data.
 *
 * The mode is detected thanks to the number of color in the palette :
 * - 2 colors is 80col (640x200)
 * - 4 colors is bitmap4 (320x200 4 colors)
 * - 16 colors is either bitmap16 (160x200 16colors)
 *   or 40col (320x200 16 colors with 2 unique colors in each 8x1 pixels
 *   block).
 *
 * As it is not possible to disriminate bitmap16 and 40col, opening the "P"
 * file sets bitmap16, opening the "C" file sets 40col.
 *
 * This function also supports .MAP files (with optional TO-SNAP extension)
 * and our own "autoloading" BIN files.
 * See http://pulkomandy.tk/projects/GrafX2/wiki/Develop/FileFormats/MOTO for
 * a detailled description.
 */
void Load_MOTO(T_IO_Context * context)
{
  // FORME / COULEUR
  FILE * file;
  byte * vram_forme = NULL;
  byte * vram_couleur = NULL;
  long file_size;
  int file_type;
  int bx, x, y, i;
  byte bpp = 4;
  byte code;
  word length, address;
  int transpose = 1;  // transpose the upper bits of the color plane bytes
                      // FFFFBBBB becomes bfFFFBBB (for TO7 compatibility)
  enum MOTO_Graphic_Mode mode = MOTO_MODE_40col;
  enum PIXEL_RATIO ratio = PIXEL_SIMPLE;
  int width = 320, height = 200, columns = 40;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  file_size = File_length_file(file);
  // Load default palette
  if (Config.Clear_palette)
    memset(context->Palette,0,sizeof(T_Palette));
  MOTO_set_TO7_palette(context->Palette);

  file_type = MOTO_Check_binary_file(file);
  if (fseek(file, 0, SEEK_SET) < 0)
  {
    fclose(file);
    return;
  }

  if (file_type == 2) // MAP file
  {
    // http://collection.thomson.free.fr/code/articles/prehisto_bulletin/page.php?XI=0&XJ=13
    byte map_mode, col_count, line_count;
    byte * vram_current;
    int end_marks;

    if (!(Read_byte(file,&code) && Read_word_be(file,&length) && Read_word_be(file,&address)))
    {
      fclose(file);
      return;
    }
    if (length < 5 || !(Read_byte(file,&map_mode) && Read_byte(file,&col_count) && Read_byte(file,&line_count)))
    {
      fclose(file);
      return;
    }
    length -= 3;
    columns = col_count + 1;
    height = 8 * (line_count + 1);
    switch(map_mode)
    {
      default:
      case 0: // bitmap4 or 40col
        width = 8 * columns;
        mode = MOTO_MODE_40col; // default to 40col
        bpp = 4;
        break;
      case 0x40:  // bitmap16
        columns >>= 1;
        width = 4 * columns;
        mode = MOTO_MODE_bm16;
        bpp = 4;
        ratio = PIXEL_WIDE;
        break;
      case 0x80:  // 80col
        columns >>= 1;
        width = 16 * columns;
        mode = MOTO_MODE_80col;
        bpp = 1;
        ratio = PIXEL_TALL;
        break;
    }
    GFX2_Log(GFX2_DEBUG, "Map mode &H%02X row=%u line=%u (%dx%d) %d\n", map_mode, col_count, line_count, width, height, columns * height);
    vram_forme = malloc(columns * height);
    vram_couleur = malloc(columns * height);
    // Check extension (TO-SNAP / PPM / ???)
    if (length > 36)
    {
      long pos_backup;
      word data;

      pos_backup = ftell(file);
      fseek(file, length-2, SEEK_CUR);  // go to last word of chunk
      Read_word_be(file, &data);
      GFX2_Log(GFX2_DEBUG, "%04X\n", data);
      switch (data)
      {
      case 0xA55A:  // TO-SNAP
        fseek(file, -40, SEEK_CUR); // go to begin of extension
        Read_word_be(file, &data);  // SCRMOD. 0=>40col, 1=>bm4, $40=>bm16, $80=>80col
        GFX2_Log(GFX2_DEBUG, "SCRMOD=&H%04X ", data);
        Read_word_be(file, &data);  // Border color
        GFX2_Log(GFX2_DEBUG, "BORDER=%u ", data);
        Read_word_be(file, &data);  // Mode BASIC (CONSOLE,,,,X) 0=40col, 1=80col, 2=bm4, 3=bm16, etc.
        GFX2_Log(GFX2_DEBUG, "CONSOLE,,,,%u\n", data);
        if(data == 2)
        {
          mode = MOTO_MODE_bm4;
          bpp = 2;
        }
        for (i = 0; i < 16; i++)
        {
          Read_word_be(file, &data);  // Palette entry
          if (data & 0x8000) data = ~data;
          MOTO_gamma_correct_MOTO_to_RGB(&context->Palette[i], data);
        }
        snprintf(context->Comment, sizeof(context->Comment), "TO-SNAP .MAP file");
        break;
      case 0x484C:  // 'HL' PPM
        fseek(file, -36, SEEK_CUR); // go to begin of extension
        for (i = 0; i < 16; i++)
        {
          Read_word_be(file, &data);  // Palette entry
          if (data & 0x8000) data = ~data;
          MOTO_gamma_correct_MOTO_to_RGB(&context->Palette[i], data);
        }
        Read_word_be(file, &data);  // Mode BASIC (CONSOLE,,,,X) 0=40col, 1=80col, 2=bm4, 3=bm16, etc.
        GFX2_Log(GFX2_DEBUG, "CONSOLE,,,,%u\n", data);
        if(data == 2)
        {
          mode = MOTO_MODE_bm4;
          bpp = 2;
        }
        snprintf(context->Comment, sizeof(context->Comment), "PPM .MAP file");
        break;
      default:
        snprintf(context->Comment, sizeof(context->Comment), "standard .MAP file");
      }
      fseek(file, pos_backup, SEEK_SET);  // RESET Position
    }
    i = 0;
    vram_current = vram_forme;
    end_marks = 0;
    while (length > 1)
    {
      byte byte1, byte2;
      Read_byte(file,&byte1);
      Read_byte(file,&byte2);
      length-=2;
      if(byte1 == 0)
      {
        if (byte2 == 0)
        {
          // end of vram stream
          GFX2_Log(GFX2_DEBUG, "0000 i=%d length=%ld\n", i, length);
          if (end_marks == 1)
            break;
          i = 0;
          vram_current = vram_couleur;
          end_marks++;
        }
        else while(byte2-- > 0 && length > 0) // copy
        {
          Read_byte(file,vram_current + i);
          length--;
          i += columns; // to the next line
          if (i >= columns * height)
          {
            if (mode == MOTO_MODE_bm4 || mode == MOTO_MODE_40col)
              i -= (columns * height - 1);  // to the 1st line of the next column
            else
            {
              i -= columns * height;  // back to the 1st line of the current column
              if (vram_current == vram_forme)   // other VRAM
                vram_current = vram_couleur;
              else
              {
                vram_current = vram_forme;
                i++;  // next column
              }
            }
          }
        }
      }
      else while(byte1-- > 0) // run length
      {
        vram_current[i] = byte2;
        i += columns; // to the next line
        if (i >= columns * height)
        {
          if (mode == MOTO_MODE_bm4 || mode == MOTO_MODE_40col)
            i -= (columns * height - 1);  // to the 1st line of the next column
          else
          {
            i -= columns * height;  // back to the 1st line of the current column
            if (vram_current == vram_forme)   // other VRAM
              vram_current = vram_couleur;
            else
            {
              vram_current = vram_forme;
              i++;  // next column
            }
          }
        }
      }
    }
    fclose(file);
  }
  else if(file_type == 3 || file_type == 4)
  {
    if (file_type == 4) // MO file
    {
      transpose = 0;
      MOTO_set_MO5_palette(context->Palette);
    }

    do
    {
      if (!(Read_byte(file,&code) && Read_word_be(file,&length) && Read_word_be(file,&address)))
      {
        if (vram_forme)
          break;
        fclose(file);
        return;
      }
      // MO5/MO6 VRAM address is &H0000
      // TO7/TO8/TO9 VRAM addres is &H4000
      if (length >= 8000 && length <= 8192 && (address == 0x4000 || address == 0))
      {
        if (vram_forme == NULL)
        {
          vram_forme = calloc(8192, 1);
          Read_bytes(file, vram_forme, length);
          length = 0;
        }
        else if (vram_couleur == NULL)
        {
          vram_couleur = calloc(8192, 1);
          Read_bytes(file, vram_couleur, length);
          if (length >= 8032)
          {
            for (x = 0; x < 16; x++)
            {
              // 1 byte Blue (4 lower bits)
              // 1 byte Green (4 upper bits) / Red (4 lower bits)
              MOTO_gamma_correct_MOTO_to_RGB(&context->Palette[x],
                                             vram_couleur[8000+x*2]<<8 | vram_couleur[8000+x*2+1]);
            }
            if (length >= 8064)
            {
              memcpy(context->Comment, vram_couleur + 8032, 32);
              if (vram_couleur[8063] >= '0' && vram_couleur[8063] <= '3')
                mode = vram_couleur[8063] - '0';
            }
            context->Comment[COMMENT_SIZE] = '\0';
          }
          length = 0;
        }
      }
      if (length > 0)
        fseek(file, length, SEEK_CUR);
    } while(code == 0);
    fclose(file);
    switch (mode)
    {
      case MOTO_MODE_40col: // default
        break;
      case MOTO_MODE_bm4:
        bpp = 2;
        break;
      case MOTO_MODE_80col:
        bpp = 1;
        width = 640;
        ratio = PIXEL_TALL;
        break;
      case MOTO_MODE_bm16:
        width = 160;
        ratio = PIXEL_WIDE;
        break;
    }
  }
  else
  {
    char * filename;
    char * path;
    char * ext;
    int n_colors;

    vram_forme = malloc(file_size);
    if (vram_forme == NULL)
    {
      fclose(file);
      return;
    }
    if (!Read_bytes(file, vram_forme, file_size))
    {
      free(vram_forme);
      fclose(file);
      return;
    }
    n_colors = (file_size - 8000) / 2;
    switch(n_colors)
    {
      case 16:
        bpp = 4;
        // 16 colors : either 40col or bm16 mode !
        // select later
        break;
      case 4:
        bpp = 2;
        mode = MOTO_MODE_bm4;
        break;
      default:
        bpp = 1;
        mode = MOTO_MODE_80col;
        width = 640;
        ratio = PIXEL_TALL;
    }
    filename = strdup(context->File_name);
    ext = strrchr(filename, '.');
    if (ext == NULL || ext == filename)
    {
      free(vram_forme);
      free(filename);
      return;
    }
    if ((ext[-1] | 32) == 'c')
    {
      vram_couleur = vram_forme;
      vram_forme = NULL;
      ext[-1] = (ext[-1] & 32) | 'P';
    }
    else if ((ext[-1] | 32) == 'p')
    {
      ext[-1] = (ext[-1] & 32) | 'C';
      if (n_colors == 16)
      {
        mode = MOTO_MODE_bm16;
        width = 160;
        ratio = PIXEL_WIDE;
      }
    }
    else
    {
      free(vram_forme);
      free(filename);
      return;
    }
    path = Filepath_append_to_dir(context->File_directory, filename);
    file = fopen(path, "rb");
    if (file == NULL)
      GFX2_Log(GFX2_ERROR, "Failed to open %s\n", path);
    free(path);
    free(filename);
    if (vram_forme == NULL)
    {
      vram_forme = malloc(file_size);
      if (vram_forme == NULL)
      {
        free(vram_couleur);
        fclose(file);
        return;
      }
      Read_bytes(file,vram_forme,file_size);
    }
    else
    {
      vram_couleur = malloc(file_size);
      if (vram_couleur == NULL)
      {
        free(vram_forme);
        fclose(file);
        return;
      }
      Read_bytes(file,vram_couleur,file_size);
    }
    fclose(file);
    GFX2_Log(GFX2_DEBUG, "MO/TO: %s,%s file_size=%ld n_colors=%d\n", context->File_name, filename, file_size, n_colors);
    for (x = 0; x < n_colors; x++)
    {
      // 1 byte Blue (4 lower bits)
      // 1 byte Green (4 upper bits) / Red (4 lower bits)
      MOTO_gamma_correct_MOTO_to_RGB(&context->Palette[x],
                                     vram_couleur[8000+x*2]<<8 | vram_couleur[8000+x*2+1]);
    }
  }
  Pre_load(context, width, height, file_size, FORMAT_MOTO, ratio, bpp);
  if (mode == MOTO_MODE_40col)
    Set_image_mode(context, IMAGE_MODE_THOMSON);
  File_error = 0;
  i = 0;
  for (y = 0; y < height; y++)
  {
    for (bx = 0; bx < columns; bx++)
    {
      byte couleur_forme;
      byte couleur_fond;
      byte forme, couleurs;

      forme = vram_forme[i];
      if (vram_couleur)
        couleurs = vram_couleur[i];
      else
        couleurs = (mode == MOTO_MODE_40col) ? 0x01 : 0x00;
      i++;
      switch(mode)
      {
        case MOTO_MODE_bm4:
          for (x = bx*8; x < bx*8+8; x++)
          {
            Set_pixel(context, x, y, ((forme & 0x80) >> 6) | ((couleurs & 0x80) >> 7));
            forme <<= 1;
            couleurs <<= 1;
          }
#if 0     // the following would be for the alternate bm4 mode
          for (x = bx*8; x < bx*8+4; x++)
          {
            Set_pixel(context, x, y, couleurs >> 6);
            couleurs <<= 2;
          }
          for (x = bx*8 + 4; x < bx*8+8; x++)
          {
            Set_pixel(context, x, y, forme >> 6);
            forme <<= 2;
          }
#endif
          break;
        case MOTO_MODE_bm16:
          Set_pixel(context, bx*4, y, forme >> 4);
          Set_pixel(context, bx*4+1, y, forme & 0x0F);
          Set_pixel(context, bx*4+2, y, couleurs >> 4);
          Set_pixel(context, bx*4+3, y, couleurs & 0x0F);
          break;
        case MOTO_MODE_80col:
          for (x = bx*16; x < bx*16+8; x++)
          {
            Set_pixel(context, x, y, (forme & 0x80) >> 7);
            Set_pixel(context, x+8, y, (couleurs & 0x80) >> 7);
            forme <<= 1;
            couleurs <<= 1;
          }
          break;
        case MOTO_MODE_40col:
        default:
          if (transpose)
          {
            // the color plane byte is bfFFFBBB (for TO7 compatibility)
            // with the upper bits of both foreground (forme) and
            // background (fond) inverted.
            couleur_forme = ((couleurs & 0x78) >> 3) ^ 0x08;
            couleur_fond = ((couleurs & 7) | ((couleurs & 0x80) >> 4)) ^ 0x08;
          }
          else
          {
            // MO5 : the color plane byte is FFFFBBBB
            couleur_forme = couleurs >> 4;
            couleur_fond = couleurs & 0x0F;
          }
          for (x = bx*8; x < bx*8+8; x++)
          {
            Set_pixel(context, x, y, (forme & 0x80)?couleur_forme:couleur_fond);
            forme <<= 1;
          }
      }
    }
  }
  free(vram_forme);
  free(vram_couleur);
}

/**
 * Pack a stream of byte in the format used by Thomson MO/TO MAP files.
 *
 * - 00 cc xx yy .. : encodes a "copy run" (cc = bytes to copy)
 * - cc xx          : encodes a "repeat run" (cc > 0 : count)
 */
//#define MOTO_MAP_NOPACKING
static unsigned int MOTO_MAP_pack(byte * packed, const byte * unpacked, unsigned int unpacked_len)
{
  unsigned int src;
  unsigned int dst = 0;
  unsigned int count;
#ifndef MOTO_MAP_NOPACKING
  unsigned int repeat;
  unsigned int i;
  word * counts;
#endif

  GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack(%p, %p, %u)\n", packed, unpacked, unpacked_len);
  if (unpacked_len == 0)
    return 0;
  if (unpacked_len == 1)
  {
    packed[0] = 1;
    packed[1] = unpacked[0];
    return 2;
  }
#ifdef MOTO_MAP_NOPACKING
  // compression disabled
  src = 0;
  while ((unpacked_len - src) > 255)
  {
    packed[dst++] = 0;
    packed[dst++] = 255;
    memcpy(packed+dst, unpacked+src, 255);
    dst += 255;
    src += 255;
  }
  count = unpacked_len - src;
  packed[dst++] = 0;
  packed[dst++] = count;
  memcpy(packed+dst, unpacked+src, count);
  dst += count;
  src += count;
  return dst;
#else
  counts = malloc(sizeof(word) * (unpacked_len + 1));
  i = 0;
  repeat = (unpacked[0] == unpacked[1]);
  count = 2;
  src = 2;
  // 1st step : count lenght of the Copy runs and Repeat runs
  while (src < unpacked_len)
  {
    if (repeat)
    {
      if (unpacked[src-1] == unpacked[src])
        count++;
      else
      {
        // flush the repeat run
        counts[i++] = count | 0x8000; // 0x8000 is the marker for repeat runs
        count = 1;
        repeat = 0;
      }
    }
    else
    {
      if (unpacked[src-1] != unpacked[src])
        count++;
      else if (count == 1)
      {
        count++;
        repeat = 1;
      }
      else
      {
        // flush the copy run
        counts[i++] = (count-1) | (count == 2 ? 0x8000 : 0); // mark copy run of 1 as repeat of 1
        count = 2;
        repeat = 1;
      }
    }
    src++;
  }
  // flush the last run
  counts[i++] = ((repeat || count == 1) ? 0x8000 : 0) | count;
  counts[i++] = 0;  // end marker
  // check consistency of counts
  count = 0;
  for (i = 0; counts[i] != 0; i++)
    count += (counts[i] & ~0x8000);
  if (count != unpacked_len)
    GFX2_Log(GFX2_ERROR, "*** encoding error in MOTO_MAP_pack() *** count=%u unpacked_len=%u\n",
             count, unpacked_len);
  // output optimized packed stream
  // repeat run are encoded cc xx
  // copy run are encoded   00 cc xx xx xx xx
  i = 0;
  src = 0;
  while (counts[i] != 0)
  {
    while (counts[i] & 0x8000)  // repeat run
    {
      count = counts[i] & ~0x8000;
      GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() %4u %4u repeat %u times %02x\n", src, i, count, unpacked[src]);
      while(count > 255)
      {
        packed[dst++] = 255;
        packed[dst++] = unpacked[src];
        count -= 255;
        src += 255;
      }
      packed[dst++] = count;
      packed[dst++] = unpacked[src];
      src += count;
      i++;
    }
    while (counts[i] != 0 && !(counts[i] & 0x8000))  // copy run
    {
      // calculate the "savings" of repeat runs between 2 copy run
      int savings = 0;
      unsigned int j;
      GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() %4u %4u copy %u bytes\n", src, i, counts[i]);
      for (j = i + 1; counts[j] & 0x8000; j++) // check repeat runs until the next copy run
      {
        count = counts[j] & ~0x8000;
        if (savings < 0 && (savings + (int)count - 2) > 0)
          break;
        savings += count - 2; // a repeat run outputs 2 bytes for count bytes of input
      }
      count = counts[i];
GFX2_Log(GFX2_DEBUG, " savings=%d i=%u j=%u (counts[j]=0x%04x)\n", savings, i, j, counts[j]);
      if (savings < 2 && (j > i + 1))
      {
        unsigned int k;
        if (counts[j] == 0) // go to the end of stream
        {
          for (k = i + 1; k < j; k++)
            count += (counts[k] & ~0x8000);
          GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() src=%u extend copy from %u to %u\n", src, counts[i], count);
          i = j - 1;
        }
        else
        {
          for (k = i + 1; k < j; k++)
            count += (counts[k] & ~0x8000);
          if (!(counts[j] & 0x8000))
          { // merge with the next copy run (and the repeat runs between)
            GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() src=%u merge savings=%d\n", src, savings);
            i = j;
            counts[i] += count;
            continue;
          }
          else
          { // merge with the next few repeat runs
            GFX2_Log(GFX2_DEBUG, "MOTO_MAP_pack() src=%u extends savings=%d\n", src, savings);
            i = j - 1;
          }
        }
      }
      while (count > 255)
      {
        packed[dst++] = 0;
        packed[dst++] = 255;
        memcpy(packed+dst, unpacked+src, 255);
        dst += 255;
        src += 255;
        count -= 255;
      }
      packed[dst++] = 0;
      packed[dst++] = count;
      memcpy(packed+dst, unpacked+src, count);
      dst += count;
      src += count;
      i++;
    }
  }
  free(counts);
  return dst;
#endif
}


/**
 * GUI window to choose Thomson MO/TO saving parameters
 *
 * @param[out] machine target machine
 * @param[out] format file format (0 = BIN, 1 = MAP)
 * @param[in,out] mode video mode @ref MOTO_Graphic_Mode
 */
static int Save_MOTO_window(enum MOTO_Machine_Type * machine, int * format, enum MOTO_Graphic_Mode * mode)
{
  int button;
  T_Dropdown_button * machine_dd;
  T_Dropdown_button * format_dd;
  T_Dropdown_button * mode_dd;
  static const char * mode_list[] = { "40col", "80col", "bm4", "bm16" };
  char text_info[24];

  Open_window(200, 125, "Thomson MO/TO Saving");
  Window_set_normal_button(110,100,80,15,"Save",1,1,KEY_RETURN); // 1
  Window_set_normal_button(10,100,80,15,"Cancel",1,1,KEY_ESCAPE); // 2

  Print_in_window(13,18,"Target Machine:",MC_Dark,MC_Light);
  machine_dd = Window_set_dropdown_button(10,28,110,15,100,
                                          (*mode == MOTO_MODE_40col) ? "TO7/TO7-70" : "TO9/TO8/TO9+",
                                          1, 0, 1, LEFT_SIDE,0); // 3
  if (*mode == MOTO_MODE_40col)
    Window_dropdown_add_item(machine_dd, MACHINE_TO7, "TO7/TO7-70");
  Window_dropdown_add_item(machine_dd, MACHINE_TO8, "TO9/TO8/TO9+");
  if (*mode == MOTO_MODE_40col)
    Window_dropdown_add_item(machine_dd, MACHINE_MO5, "MO5");
  Window_dropdown_add_item(machine_dd, MACHINE_MO6, "MO6");

  Print_in_window(13,46,"Format:",MC_Dark,MC_Light);
  format_dd = Window_set_dropdown_button(10,56,110,15,92,"BIN",1, 0, 1, LEFT_SIDE,0); // 4
  Window_dropdown_add_item(format_dd, 0, "BIN");
  Window_dropdown_add_item(format_dd, 1, "MAP/TO-SNAP");

  Print_in_window(136,46,"Mode:",MC_Dark,MC_Light);
  mode_dd = Window_set_dropdown_button(136,56,54,15,44,mode_list[*mode],1, 0, 1, LEFT_SIDE,0); // 5
  if (*mode == MOTO_MODE_40col)
    Window_dropdown_add_item(mode_dd, *mode, mode_list[*mode]);
  if (*mode == MOTO_MODE_80col)
    Window_dropdown_add_item(mode_dd, *mode, mode_list[*mode]);
  if (*mode == MOTO_MODE_40col)
    Window_dropdown_add_item(mode_dd, MOTO_MODE_bm4, mode_list[MOTO_MODE_bm4]);
  if (*mode == MOTO_MODE_bm16)
    Window_dropdown_add_item(mode_dd, *mode, mode_list[*mode]);

  Update_window_area(0,0,Window_width,Window_height);
  Display_cursor();
  do
  {
    button = Window_clicked_button();
    if (Is_shortcut(Key, 0x100+BUTTON_HELP))
    {
      Key = 0;
      Window_help(BUTTON_SAVE, "THOMSON MO/TO FORMAT");
    }
    else switch (button)
    {
      case 3:
        *machine = (enum MOTO_Machine_Type)Window_attribute2;
        break;
      case 4:
        *format = Window_attribute2;
        break;
      case 5:
        *mode = (enum MOTO_Graphic_Mode)Window_attribute2;
        break;
    }
    Hide_cursor();
    //"ABCDEFGHIJKLMNOPQRSTUVW"
    memset(text_info, ' ', 23);
    text_info[23] = '\0';
    if (*machine == MACHINE_TO7 || *machine == MACHINE_TO770 || *machine == MACHINE_MO5)
    {
      if (*mode != MOTO_MODE_40col)
        snprintf(text_info, sizeof(text_info), "%s only supports 40col",
                 (*machine == MACHINE_MO5) ? "MO5" : "TO7");
      else if (*format == 1)
        strncpy(text_info, "No TO-SNAP extension.  ", sizeof(text_info));
      else
        strncpy(text_info, "No palette to save.    ", sizeof(text_info));
    }
    Print_in_window(9, 80, text_info, MC_Dark, MC_Light);
    Display_cursor();
  } while(button!=1 && button!=2);

  Close_window();
  Display_cursor();
  return button==1;
}

/**
 * Save a picture in MAP or BIN Thomson MO/TO file format.
 *
 * File format details :
 * http://pulkomandy.tk/projects/GrafX2/wiki/Develop/FileFormats/MOTO
 */
void Save_MOTO(T_IO_Context * context)
{
  int transpose = 1;  // transpose upper bits in "couleur" vram
  enum MOTO_Machine_Type target_machine = MACHINE_TO7;
  int format = 0; // 0 = BIN, 1 = MAP
  enum MOTO_Graphic_Mode mode;
  FILE * file = NULL;
  byte * vram_forme;
  byte * vram_couleur;
  int i, x, y, bx;
  word reg_prc = 0xE7C3; // PRC : TO7/8/9 0xE7C3 ; MO5/MO6 0xA7C0
  byte prc_value = 0x65;// Value to write to PRC to select VRAM bank
  // MO5 : 0x51
  word vram_address = 0x4000; // 4000 on TO7/TO8/TO9, 0000 on MO5/MO6

  File_error = 1;

  /**
   * In the future we could support other resolution for .MAP
   * format.
   * And even in .BIN format, we could store less lines. */
  if (context->Height != 200)
  {
    Warning_message("must be 640x200, 320x200 or 160x200");
    return;
  }

  switch (context->Width)
  {
    case 160:
      mode = MOTO_MODE_bm16;
      target_machine = MACHINE_TO8;
      break;
    case 640:
      mode = MOTO_MODE_80col;
      target_machine = MACHINE_TO8;
      break;
    case 320:
      mode = MOTO_MODE_40col; // or bm4
      break;
    default:
      Warning_message("must be 640x200, 320x200 or 160x200");
      return;
  }

  if (!Save_MOTO_window(&target_machine, &format, &mode))
    return;

  if (target_machine == MACHINE_MO5 || target_machine == MACHINE_MO6)
  {
    reg_prc = 0xA7C0; // PRC : MO5/MO6 0xA7C0
    prc_value = 0x51;
    vram_address = 0;
    transpose = 0;
  }

  vram_forme = malloc(8192);
  vram_couleur = malloc(8192);
  switch (mode)
  {
  case MOTO_MODE_40col:
    {
      /**
       * The 40col encoding algorithm is optimized for further vertical
       * RLE packing. The "attibute" byte is kept as constant as possible
       * between adjacent blocks.
       */
      unsigned color_freq[16];
      unsigned max_freq = 0;
      byte previous_fond = 0, previous_forme = 0;
      byte most_used_color = 0;

      // search for most used color to prefer it as background color
      for (i = 0; i < 16; i++)
        color_freq[i] = 0;
      for (y = 0; y < context->Height; y++)
      {
        for (x = 0; x < context->Width; x++)
        {
          byte col = Get_pixel(context, x, y);
          if (col > 15)
          {
            Warning_with_format("color %u > 15 at pixel (%d,%d)", col, x, y);
            goto error;
          }
          color_freq[col]++;
        }
      }
      for (i = 0; i < 16; i++)
      {
        if (color_freq[i] > max_freq)
        {
          max_freq = color_freq[i];
          most_used_color = (byte)i;  // most used color
        }
      }
      previous_fond = most_used_color;
      max_freq = 0;
      for (i = 0; i < 16; i++)
      {
        if (i != most_used_color && color_freq[i] > max_freq)
        {
          max_freq = color_freq[i];
          previous_forme = (byte)i;  // second most used color
        }
      }
      GFX2_Log(GFX2_DEBUG, "Save_MOTO() most used color index %u, 2nd %u\n", previous_fond, previous_forme);

      if (target_machine == MACHINE_MO5)
      {
        /**
         * For MO5 we use a different 40col algorithm
         * to make sure the last pixel of a GPL and the first the next
         * are both FORME or both FOND, else we get an ugly glitch on the
         * EFGJ033 Gate Array MO5!
         */
        byte forme_byte = 0;
        byte couleur_byte = 0x10;
        GFX2_Log(GFX2_DEBUG, "Save_MOTO() 40col using MO5 algo\n");
        for (y = 0; y < context->Height; y++)
        {
          for (bx = 0; bx < 40; bx++)
          {
            byte fond = 0xff, forme = 0xff;
            forme_byte &= 1;  // Last bit of the previous FORME byte
            x = bx*8;
            if (forme_byte)
              forme = Get_pixel(context, x, y);
            else
              fond = Get_pixel(context, x, y);
            while (++x < bx * 8 + 8)
            {
              byte col = Get_pixel(context, x, y);
              forme_byte <<= 1;
              if (col == forme)
                forme_byte |= 1;
              else if (col != fond)
              {
                if (forme == 0xff)
                {
                  forme_byte |= 1;
                  forme = col;
                }
                else if (fond == 0xff)
                  fond = col;
                else
                {
                  Warning_with_format("Constraint error at pixel (%d,%d)", x, y);
                  goto error;
                }
              }
            }
            if (forme != 0xff)
              couleur_byte = (forme << 4) | (couleur_byte & 0x0f);
            if (fond != 0xff)
              couleur_byte = (couleur_byte & 0xf0) | fond;
            vram_forme[bx+y*40] = forme_byte;
            vram_couleur[bx+y*40] = couleur_byte;
          }
        }
      }
      else
      {
        GFX2_Log(GFX2_DEBUG, "Save_MOTO() 40col using optimized algo\n");
        // encoding of each 8x1 block
        for (bx = 0; bx < 40; bx++)
        {
          for (y = 0; y < context->Height; y++)
          {
            byte forme_byte = 1;
            byte col;
            byte c1, c1_count = 1;
            byte c2 = 0xff, c2_count = 0;
            byte fond, forme;
            x = bx * 8;
            c1 = Get_pixel(context, x, y);
            while (++x < bx * 8 + 8)
            {
              forme_byte <<= 1;
              col = Get_pixel(context, x, y);
              if (col > 15)
              {
                Warning_with_format("color %d > 15 at pixel (%d,%d)", col, x, y);
                goto error;
              }
              if (col == c1)
              {
                forme_byte |= 1;
                c1_count++;
              }
              else
              {
                c2_count++;
                if (c2 == 0xff)
                  c2 = col;
                else if (col != c2)
                {
                  Warning_with_format("constraint error at pixel (%d,%d)", x, y);
                  goto error;
                }
              }
            }
            if (c2 == 0xff)
            {
              // Only one color in the 8x1 block
              if (c1 == previous_fond)
                c2 = previous_forme;
              else
                c2 = previous_fond;
            }
            // select background color (fond)
            // and foreground color (forme)
            if (c1 == previous_fond)
            {
              fond = c1;
              forme = c2;
              forme_byte = ~forme_byte;
            }
            else if (c2 == previous_fond)
            {
              fond = c2;
              forme = c1;
            }
            else if (c1 == most_used_color)
            {
              fond = c1;
              forme = c2;
              forme_byte = ~forme_byte;
            }
            else if (c2 == most_used_color)
            {
              fond = c2;
              forme = c1;
            }
            else if (c1_count >= c2_count)
            {
              fond = c1;
              forme = c2;
              forme_byte = ~forme_byte;
            }
            else
            {
              fond = c2;
              forme = c1;
            }
            // write to VRAM
            vram_forme[bx+y*40] = forme_byte;
            // transpose for TO7 compatibility
            if (transpose)
              vram_couleur[bx+y*40] = ((fond & 7) | ((fond & 8) << 4) | (forme << 3)) ^ 0xC0;
            else
              vram_couleur[bx+y*40] = fond | (forme << 4);
            previous_fond = fond;
            previous_forme = forme;
          }
          if (transpose)
          {
            previous_fond = (vram_couleur[bx] & 7) | (~vram_couleur[bx] & 0x80) >> 4;
            previous_forme = ((vram_couleur[bx] & 0x78) >> 3) ^ 8;
          }
          else
          {
            previous_fond = vram_couleur[bx] & 15;
            previous_forme = vram_couleur[bx] >> 4;
          }
        }
      }
    }
    break;
  case MOTO_MODE_80col:
    for (bx = 0; bx < context->Width / 16; bx++)
    {
      for (y = 0; y < context->Height; y++)
      {
        byte val = 0;
        for (x = bx * 16; x < bx*16 + 8; x++)
          val = (val << 1) | Get_pixel(context, x, y);
        vram_forme[y*(context->Width/16)+bx] = val;
        for (; x < bx*16 + 16; x++)
          val = (val << 1) | Get_pixel(context, x, y);
        vram_couleur[y*(context->Width/16)+bx] = val;
      }
    }
    break;
  case MOTO_MODE_bm4:
    for (y = 0; y < context->Height; y++)
    {
      for (bx = 0; bx < context->Width / 8; bx++)
      {
        byte val1 = 0, val2 = 0, pixel;
        for (x = bx * 8; x < bx*8 + 8; x++)
        {
          pixel = Get_pixel(context, x, y);
          if (pixel > 3)
          {
            Warning_with_format("color %d > 3 at pixel (%d,%d)", pixel, x, y);
            goto error;
          }
          val1 = (val1 << 1) | (pixel >> 1);
          val2 = (val2 << 1) | (pixel & 1);
        }
        vram_forme[y*(context->Width/8)+bx] = val1;
        vram_couleur[y*(context->Width/8)+bx] = val2;
      }
    }
    break;
  case MOTO_MODE_bm16:
    for (bx = 0; bx < context->Width / 4; bx++)
    {
      for (y = 0; y < context->Height; y++)
      {
         vram_forme[y*(context->Width/4)+bx] = (Get_pixel(context, bx*4, y) << 4) | Get_pixel(context, bx*4+1, y);
         vram_couleur[y*(context->Width/4)+bx] = (Get_pixel(context, bx*4+2, y) << 4) | Get_pixel(context, bx*4+3, y);
      }
    }
    break;
  }
  // palette
  for (i = 0; i < 16; i++)
  {
    word to8color = MOTO_gamma_correct_RGB_to_MOTO(context->Palette + i);
    vram_forme[8000+i*2] = to8color >> 8;
    vram_forme[8000+i*2+1] = to8color & 0xFF;
  }

  file = Open_file_write(context);
  if (file == NULL)
    goto error;

  if (format == 0)  // BIN
  {
    word chunk_length;

    if (target_machine == MACHINE_TO7 || target_machine == MACHINE_TO770 || target_machine == MACHINE_MO5)
      chunk_length = 8000;  // Do not save palette
    else
    {
      chunk_length = 8000 + 32 + 32;  // data + palette + comment
      // Commentaire
      if (context->Comment[0] != '\0')
        strncpy((char *)vram_forme + 8032, context->Comment, 32);
      else
        snprintf((char *)vram_forme + 8032, 32, "GrafX2 %s.%s", Program_version, SVN_revision);
      // also saves the video mode
      vram_forme[8063] = '0' + mode;
      memcpy(vram_couleur + 8000, vram_forme + 8000, 64);
    }

    // Format BIN
    // TO8/TO9 : set LGAMOD 0xE7DC  40col=0 bm4=0x21 80col=0x2a bm16=0x7b
    if (!DECB_BIN_Add_Chunk(file, 1, reg_prc, &prc_value))
      goto error;
    if (!DECB_BIN_Add_Chunk(file, chunk_length, vram_address, vram_forme))
      goto error;
    prc_value &= 0xFE; // select color data
    if (!DECB_BIN_Add_Chunk(file, 1, reg_prc, &prc_value))
      goto error;
    if (!DECB_BIN_Add_Chunk(file, chunk_length, vram_address, vram_couleur))
      goto error;
    if (!DECB_BIN_Add_End(file, 0x0000))
      goto error;
  }
  else
  {
    // format MAP with TO-SNAP extensions
    byte * unpacked_data;
    byte * packed_data;

    unpacked_data = malloc(16*1024);
    packed_data = malloc(16*1024);
    if (packed_data == NULL || unpacked_data == NULL)
    {
      GFX2_Log(GFX2_ERROR, "Failed to allocate 2x16kB of memory\n");
      free(packed_data);
      free(unpacked_data);
      goto error;
    }
    switch (mode)
    {
      case MOTO_MODE_40col:
      case MOTO_MODE_bm4:
        packed_data[0] = 0;  // mode
        packed_data[1] = (context->Width / 8) - 1;
        break;
      case MOTO_MODE_80col:
        packed_data[0] = 0x80; // mode
        packed_data[1] = (context->Width / 8) - 1;
        break;
      case MOTO_MODE_bm16:
        packed_data[0] = 0x40; // mode
        packed_data[1] = (context->Width / 2) - 1;
        break;
    }
    packed_data[2] = (context->Height / 8) - 1;
    // 1st step : put data to pack in a linear buffer
    // 2nd step : pack data
    i = 0;
    switch (mode)
    {
      case MOTO_MODE_40col:
      case MOTO_MODE_bm4:
        for (bx = 0; bx <= packed_data[1]; bx++)
        {
          for (y = 0; y < context->Height; y++)
          {
            unpacked_data[i] = vram_forme[bx + y*(packed_data[1]+1)];
            unpacked_data[i+8192] = vram_couleur[bx + y*(packed_data[1]+1)];
            i++;
          }
        }
        i = 3;
        i += MOTO_MAP_pack(packed_data+3, unpacked_data, context->Height * (packed_data[1]+1));
        packed_data[i++] = 0; // ending of VRAM forme packing
        packed_data[i++] = 0;
        i += MOTO_MAP_pack(packed_data+i, unpacked_data + 8192, context->Height * (packed_data[1]+1));
        packed_data[i++] = 0; // ending of VRAM couleur packing
        packed_data[i++] = 0;
        break;
      case MOTO_MODE_80col:
      case MOTO_MODE_bm16:
        for (bx = 0; bx < (packed_data[1] + 1) / 2; bx++)
        {
          for (y = 0; y < context->Height; y++)
            unpacked_data[i++] = vram_forme[bx + y*(packed_data[1]+1)/2];
          for (y = 0; y < context->Height; y++)
            unpacked_data[i++] = vram_couleur[bx + y*(packed_data[1]+1)/2];
        }
        i = 3;
        i += MOTO_MAP_pack(packed_data+3, unpacked_data, context->Height * (packed_data[1]+1));
        packed_data[i++] = 0; // ending of VRAM forme packing
        packed_data[i++] = 0;
        packed_data[i++] = 0; // ending of VRAM couleur packing
        packed_data[i++] = 0;
        break;
    }
    if (i&1)  // align
      packed_data[i++] = 0;
    if (target_machine != MACHINE_TO7 && target_machine != MACHINE_TO770 && target_machine != MACHINE_MO5)
    {
      // add TO-SNAP extension
      // see http://collection.thomson.free.fr/code/articles/prehisto_bulletin/page.php?XI=0&XJ=13
      // bytes 0-1 : Hardware video mode (value of SCRMOD 0x605F)
      packed_data[i++] = 0;
      switch (mode)
      {
        case MOTO_MODE_40col:
          packed_data[i++] = 0;
          break;
        case MOTO_MODE_bm4:
          packed_data[i++] = 0x01;
          break;
        case MOTO_MODE_80col:
          packed_data[i++] = 0x80;
          break;
        case MOTO_MODE_bm16:
          packed_data[i++] = 0x40;
          break;
      }
      // bytes 2-3 : Border color
      packed_data[i++] = 0;
      packed_data[i++] = 0;
      // bytes 4-5 : BASIC video mode (CONSOLE,,,,X)
      packed_data[i++] = 0;
      switch (mode)
      {
        case MOTO_MODE_40col:
          packed_data[i++] = 0;
          break;
        case MOTO_MODE_bm4:
          packed_data[i++] = 2;
          break;
        case MOTO_MODE_80col:
          packed_data[i++] = 1;
          break;
        case MOTO_MODE_bm16:
          packed_data[i++] = 3;
          break;
      }
      // bytes 6-37 : BGR palette
      for (x = 0; x < 16; x++)
      {
        word bgr = MOTO_gamma_correct_RGB_to_MOTO(context->Palette + x);
        packed_data[i++] = bgr >> 8;
        packed_data[i++] = bgr & 0xff;
      }
      // bytes 38-39 : TO-SNAP signature
      packed_data[i++] = 0xA5;
      packed_data[i++] = 0x5A;
    }

    free(unpacked_data);

    if (!DECB_BIN_Add_Chunk(file, i, 0, packed_data) ||
        !DECB_BIN_Add_End(file, 0x0000))
    {
      free(packed_data);
      goto error;
    }
    free(packed_data);
  }
  fclose(file);
  File_error = 0;
  return;

error:
  free(vram_forme);
  free(vram_couleur);
  if (file)
    fclose(file);
  File_error = 1;
}

/////////////////////////////// Apple II Files //////////////////////////////

/**
 * Test for an Apple II HGR or DHGR raw file
 */
void Test_HGR(T_IO_Context * context, FILE * file)
{
  long file_size;

  (void)context;
  File_error = 1;

  file_size = File_length_file(file);
  if (file_size == 8192)  // HGR
    File_error = 0;
  else if(file_size == 16384) // DHGR
    File_error = 0;
}

/**
 * Load HGR (280x192) or DHGR (560x192) Apple II pictures
 *
 * Creates 2 layers :
 * 1. Monochrome
 * 2. Color
 */
void Load_HGR(T_IO_Context * context)
{
  unsigned long file_size;
  FILE * file;
  byte * vram[2];
  int bank;
  int x, y;
  int is_dhgr = 0;

  file = Open_file_read(context);
  if (file == NULL)
  {
    File_error = 1;
    return;
  }
  file_size = File_length_file(file);
  if (file_size == 16384)
    is_dhgr = 1;

  vram[0] = malloc(8192);
  Read_bytes(file, vram[0], 8192);
  if (is_dhgr)
  {
    vram[1] = malloc(8192);
    Read_bytes(file, vram[1], 8192);
  }
  else
    vram[1] = NULL;
  fclose(file);

  if (Config.Clear_palette)
    memset(context->Palette,0,sizeof(T_Palette));
  if (is_dhgr)
  {
    DHGR_set_palette(context->Palette);
    Pre_load(context, 560, 192, file_size, FORMAT_HGR, PIXEL_TALL, 4);
  }
  else
  {
    HGR_set_palette(context->Palette);
    Pre_load(context, 280, 192, file_size, FORMAT_HGR, PIXEL_SIMPLE, 2);
  }
  for (y = 0; y < 192; y++)
  {
    byte palette = 0, color = 0;
    byte previous_palette = 0;  // palette for the previous pixel pair
    int column, i;
    int offset = ((y & 7) << 10) + ((y & 070) << 4) + ((y >> 6) * 40);
    x = 0;
    for (column = 0; column < 40; column++)
    for (bank = 0; bank <= is_dhgr; bank++)
    {
      byte b = vram[bank][offset+column];
      if (!is_dhgr)
        palette = (b & 0x80) ? 4 : 0;
      else
        palette = (b & 0x80) ? 0 : 16;
      for (i = 0; i < 7; i++)
      {
        if (context->Type == CONTEXT_MAIN_IMAGE)
        {
          // monochrome
          Set_loading_layer(context, 0);
          Set_pixel(context, x, y, ((b & 1) * (is_dhgr ? 15 : 3)) + palette);
          Set_loading_layer(context, 1);
        }
        // color
        color = (color << 1) | (b & 1);
        if (is_dhgr)
        {
          if ((x & 3) == 0)
            previous_palette = palette; // what is important is the value when the 1st bit was read...
          /// emulate "chat mauve" DHGR mixed mode.
          /// see http://boutillon.free.fr/Underground/Anim_Et_Graph/Extasie_Chat_Mauve_Reloaded/Extasie_Chat_Mauve_Reloaded.html
          if (previous_palette) // BW
            Set_pixel(context, x, y, ((b & 1) * 15) + palette);
          else if ((x & 3) == 3)
          {
            Set_pixel(context, x - 3, y, (color & 15) + palette);
            Set_pixel(context, x - 2, y, (color & 15) + palette);
            Set_pixel(context, x - 1, y, (color & 15) + palette);
            Set_pixel(context, x, y, (color & 15) + palette);
          }
        }
        else
        {
          /// HGR emulation following the behaviour of a "Le Chat Mauve"
          /// RVB adapter for the Apple //c.
          /// Within the bit stream, the color of the middle pixel is :<br>
          /// <tt>
          /// 111 \          <br>
          /// 110  }- white  <br>
          /// 011 /          <br>
          /// 010 \ _ color  <br>
          /// 101 /          <br>
          /// 000 \          <br>
          /// 001  }- black  <br>
          /// 100 /          <br>
          /// </tt>
          /// Color depends on the selected palette for the current byte
          /// and the position of the pixel (odd or even).
          if ((color & 3) == 3) // 11 => white
          {
            Set_pixel(context, x - 1, y, 3 + previous_palette);
            Set_pixel(context, x, y, 3 + palette);
          }
          else if ((color & 1) == 0) // 0 => black
            Set_pixel(context, x, y, palette);
          else // 01 => color
          {
            if ((color & 7) == 5) // 101
              Set_pixel(context, x - 1, y, 2 - (x & 1) + previous_palette);
            Set_pixel(context, x, y, 2 - (x & 1) + palette);
          }
          previous_palette = palette;
        }
        b >>= 1;
        x++;
      }
    }
  }
  // show hidden data in HOLES
  for (y = 0; y < 64; y++)
  for (bank = 0; bank < 1; bank++)
  {
    byte b = 0;
    for (x = 0; x < 8; x++)
      b |= vram[bank][x + (y << 7) + 120];
    if (b != 0)
      GFX2_LogHexDump(GFX2_DEBUG, bank ? "AUX " : "MAIN", vram[bank], (y << 7) + 120, 8);
  }
  free(vram[0]);
  free(vram[1]);
  File_error = 0;

  Set_image_mode(context, is_dhgr ? IMAGE_MODE_DHGR : IMAGE_MODE_HGR);
}

/**
 * Save HGR (280x192) or DHGR (560x192) Apple II pictures
 *
 * The data saved is the "monochrome" data from layer 1
 */
void Save_HGR(T_IO_Context * context)
{
  FILE * file;
  byte * vram[2];
  int bank;
  int x, y;
  int is_dhgr = 0;

  File_error = 1;
  if (context->Height != 192 || (context->Width != 280 && context->Width != 560))
  {
    Warning_message("Picture must be 280x192 (HGR) or 560x192 (DHGR)");
    return;
  }
  if (context->Width == 560)
    is_dhgr = 1;
  
  file = Open_file_write(context);
  if (file == NULL)
    return;
  vram[0] = calloc(8192, 1);
  if (vram[0] == NULL)
  {
    fclose(file);
    return;
  }
  if (is_dhgr)
  {
    vram[1] = calloc(8192, 1);
    if (vram[1] == NULL)
    {
      free(vram[0]);
      fclose(file);
      return;
    }
  }
  else
    vram[1] = NULL;

  Set_saving_layer(context, 0); // "monochrome" layer
  for (y = 0; y < 192; y++)
  {
    int i, column = 0;
    int offset = ((y & 7) << 10) + ((y & 070) << 4) + ((y >> 6) * 40);
    x = 0;
    bank = 0;
    while (x < context->Width)
    {
      byte b;
      if (is_dhgr)
        b = (Get_pixel(context, x, y) & 16) ? 0 : 0x80;
      else
        b = (Get_pixel(context, x, y) & 4) ? 0x80 : 0;
      for (i = 0; i < 7; i++)
      {
        b = b | ((Get_pixel(context, x++, y) & 1) << i);
      }
      vram[bank][offset + column] = b;
      if (is_dhgr)
      {
        if (++bank > 1)
        {
          bank = 0;
          column++;
        }
      }
      else
        column++;
    }
  }

  if (Write_bytes(file, vram[0], 8192))
  {
    if (is_dhgr)
    {
      if (Write_bytes(file, vram[1], 8192))
        File_error = 0;
    }
    else
      File_error = 0;
  }

  free(vram[0]);
  free(vram[1]);
  fclose(file);
}
