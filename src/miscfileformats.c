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

#include "global.h"
#include "io.h"
#include "loadsave.h"
#include "loadsavefuncs.h"
#include "misc.h"
#include "struct.h"
#include "windows.h"
#include "oldies.h"
#include "fileformats.h"
#include "gfx2mem.h"
#include "gfx2log.h"

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

    for (i = 0; i < 256; i++)
    {
      for (;;)
      {
        // skip comments
        int c = getc(file);
        if (c == '#')
        {
          if (!Read_byte_line(file, buffer, sizeof(buffer)))
            return;
          GFX2_Log(GFX2_DEBUG, "comment: %s", buffer);
        }
        else
        {
          fseek(file, -1, SEEK_CUR);
          break;
        }
      }
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
        Pre_load(context, header1.Width, header1.Height,
                 file_size, FORMAT_CEL, PIXEL_SIMPLE, 0);
        if (File_error==0)
        {
          Original_screen_X = context->Width;
          Original_screen_Y = context->Height;
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

          Pre_load(context,
                   header2.Width + header2.X_offset,
                   header2.Height + header2.Y_offset,
                   file_size, FORMAT_CEL, PIXEL_SIMPLE, 0);
          if (File_error==0)
          {
            Original_screen_X = context->Width;
            Original_screen_Y = context->Height;
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
    GFX2_Log(GFX2_WARNING, "Load_FLI(): file size mismatch in header %lu != %u\n", file_size, header.size);

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
          if (context->Type == CONTEXT_MAIN_IMAGE && Get_image_mode(context) == IMAGE_MODE_ANIMATION)
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
                  GFX2_Log(GFX2_WARNING, "Unsupported opcode %04x\n", opcode);
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
        GFX2_Log(GFX2_WARNING, "Load_FLI(): unrecognized chunk %04x\n", chunk_type);
    }
    if (chunk_size > 0 && header.size != 12)
    {
      fseek(file, chunk_size, SEEK_CUR);
    }
  }
  fclose(file);
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

  vram[0] = GFX2_malloc(8192);
  Read_bytes(file, vram[0], 8192);
  if (is_dhgr)
  {
    vram[1] = GFX2_malloc(8192);
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


///////////////////////////// HP-48 Grob Files ////////////////////////////

/**
 * HP48 addresses are 20bits (5 nibbles)
 * offset is in nibble (half byte)
 */
static dword Read_HP48Address(const byte * buffer, int offset)
{
  dword data = 0;
  int i = 4;
  do
  {
    byte nibble;
    nibble = buffer[(offset + i) >> 1];
    if ((offset + i) & 1)
      nibble >>= 4;
    nibble &= 15;
    data = (data << 4) | nibble;
  }
  while (i-- > 0);
  return data;
}

/**
 * Test for a HP-48 Grob file
 */
void Test_GRB(T_IO_Context * context, FILE * file)
{
  byte buffer[18];
  unsigned long file_size;
  dword prologue, size, width, height;

  (void)context;
  File_error = 1;
  file_size = File_length_file(file);
  if (!Read_bytes(file, buffer, 18))
    return;
  if(memcmp(buffer, "HPHP48-R", 8) != 0)
    return;
  prologue = Read_HP48Address(buffer+8, 0);
  size = Read_HP48Address(buffer+8, 5);
  GFX2_Log(GFX2_DEBUG, "HP48 File detected. %lu bytes prologue %05x %u nibbles\n",
           file_size, prologue, size);
  if (prologue != 0x02b1e)
    return;
  height = Read_HP48Address(buffer+8, 10);
  width = Read_HP48Address(buffer+8, 15);
  GFX2_Log(GFX2_DEBUG, " Grob dimensions : %ux%u\n", width, height);
  if ((file_size - 8) < ((size + 5) / 2))
    return;
  if (file_size < (18 + ((width + 7) >> 3) * height))
    return;
  File_error = 0;
}

void Load_GRB(T_IO_Context * context)
{
  byte buffer[18];
  byte * bitplane[4];
  unsigned long file_size;
  dword prologue, size, width, height;
  byte bp, bpp;
  FILE * file;
  unsigned x, y;

  File_error = 1;
  file = Open_file_read(context);
  if (file == NULL)
    return;
  file_size = File_length_file(file);
  if (!Read_bytes(file, buffer, 18))
  {
    fclose(file);
    return;
  }
  prologue = Read_HP48Address(buffer+8, 0);
  size = Read_HP48Address(buffer+8, 5);
  height = Read_HP48Address(buffer+8, 10);
  width = Read_HP48Address(buffer+8, 15);
  if (height >= 256)
    bpp = 4;
  else if (height >= 192)
    bpp = 3;
  else if (height >= 128)
    bpp = 2;
  else
    bpp = 1;

  GFX2_Log(GFX2_DEBUG, "HP48: %05X size=%u %ux%u\n", prologue, size, width, height);

  File_error = 0;
  Pre_load(context, width, height/bpp, file_size, FORMAT_GRB, PIXEL_SIMPLE, bpp);
  if (File_error == 0)
  {
    dword bytes_per_plane = ((width + 7) >> 3) * (height/bpp);
    dword offset = 0;

    if (Config.Clear_palette)
      memset(context->Palette, 0, sizeof(T_Palette));
    for (x = 0; x < ((unsigned)1 << bpp); x++)
    {
      context->Palette[x].R = context->Palette[x].G = (x * 255) / ((1 << bpp) - 1);
      context->Palette[x].B = 127;
    }

    // Load all bit planes
    for (bp = 0; bp < bpp; bp++)
    {
      bitplane[bp] = GFX2_malloc(bytes_per_plane);
      if (bitplane[bp])
      {
        if (!Read_bytes(file, bitplane[bp], bytes_per_plane))
          File_error = 1;
      }
    }
    // set pixels
    for (y = 0; y < (height/bpp) && File_error == 0; y++)
    {
      for (x = 0; x < width; x++)
      {
        byte b = 0;
        for (bp = 0; bp < bpp; bp++)
          b |= ((bitplane[bp][offset] >> (x & 7)) & 1) << bp;
        // invert because 1 is a black pixel on HP-48 LCD display
        Set_pixel(context, x, y, b ^ ((1 << bpp) - 1));
        if ((x & 7) == 7)
          offset++;
      }
      if ((x & 7) != 7)
        offset++;
    }
    // Free bit planes
    for (bp = 0; bp < bpp; bp++)
      free(bitplane[bp]);
  }
  fclose(file);
}
