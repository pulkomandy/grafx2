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

///@file giformat.c
/// Saving and loading GIF

#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "global.h"
#include "oldies.h"
#include "io.h"
#include "loadsave.h"
#include "loadsavefuncs.h"
#include "gfx2mem.h"
#include "gfx2log.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

/**
 * @defgroup GIF GIF format
 * @ingroup loadsaveformats
 * Graphics Interchange Format
 *
 * The GIF format uses LZW compression and stores indexed color pictures
 * up to 256 colors. It has the ability to store several pictures in the same
 * file : GrafX2 takes advantage of this feature for storing layered images
 * and animations.
 *
 * GrafX2 implements GIF89a :
 * https://www.w3.org/Graphics/GIF/spec-gif89a.txt
 *
 * @{
 */

/// Logical Screen Descriptor Block
typedef struct
{
  word Width;   ///< Width of the complete image area
  word Height;  ///< Height of the complete image area
  byte Resol;   ///< Informations about the resolution (and other)
  byte Backcol; ///< Proposed background color
  byte Aspect;  ///< Informations about aspect ratio. Ratio = (Aspect + 15) / 64
} T_GIF_LSDB;

/// Image Descriptor Block
typedef struct
{
  word Pos_X;         ///< X offset where the image should be pasted
  word Pos_Y;         ///< Y offset where the image should be pasted
  word Image_width;   ///< Width of image
  word Image_height;  ///< Height of image
  byte Indicator;     ///< Misc image information
  byte Nb_bits_pixel; ///< Nb de bits par pixel
} T_GIF_IDB;

/// Graphic Control Extension
typedef struct
{
  byte Block_identifier;  ///< 0x21
  byte Function;          ///< 0xF9
  byte Block_size;        ///< 4
  byte Packed_fields;     ///< 11100000 : Reserved
                          ///< 00011100 : Disposal method
                          ///< 00000010 : User input flag
                          ///< 00000001 : Transparent flag
  word Delay_time;        ///< Time for this frame to stay displayed
  byte Transparent_color; ///< Which color index acts as transparent
  word Block_terminator;  ///< 0x00
} T_GIF_GCE;

enum DISPOSAL_METHOD
{
  DISPOSAL_METHOD_UNDEFINED = 0,
  DISPOSAL_METHOD_DO_NOT_DISPOSE = 1,
  DISPOSAL_METHOD_RESTORE_BGCOLOR = 2,
  DISPOSAL_METHOD_RESTORE_PREVIOUS = 3,
};


/// Test if a file is GIF format
void Test_GIF(T_IO_Context * context, FILE * file)
{
  char signature[6];

  (void)context;
  File_error=1;

  if (Read_bytes(file,signature,6))
  {
    /// checks if the signature (6 first bytes) is either GIF87a or GIF89a
    if ((!memcmp(signature,"GIF87a",6))||(!memcmp(signature,"GIF89a",6)))
      File_error=0;
  }
}


// -- Lire un fichier au format GIF -----------------------------------------

typedef struct {
  word nb_bits;        ///< bits for a code
  word remainder_bits; ///< available bits in @ref last_byte field
  byte remainder_byte; ///< Remaining bytes in current block
  word current_code;   ///< current code (generally the one just read)
  byte last_byte;      ///< buffer byte for reading bits for codes
  word pos_X;          ///< Current coordinates
  word pos_Y;
  word interlaced;     ///< interlaced flag
  word pass;           ///< current pass in interlaced decoding
  word stop;           ///< Stop flag (end of picture)
} T_GIF_context;


/// Reads the next code (GIF.nb_bits bits)
static word GIF_get_next_code(FILE * GIF_file, T_GIF_context * gif)
{
  word nb_bits_to_process = gif->nb_bits;
  word nb_bits_processed = 0;
  word current_nb_bits;

  gif->current_code = 0;

  while (nb_bits_to_process)
  {
    if (gif->remainder_bits == 0) // Il ne reste plus de bits...
    {
      // Lire l'octet suivant:

      // Si on a atteint la fin du bloc de Raster Data
      if (gif->remainder_byte == 0)
      {
        // Lire l'octet nous donnant la taille du bloc de Raster Data suivant
        if(Read_byte(GIF_file, &gif->remainder_byte)!=1)
        {
          File_error=2;
          return 0;
        }
        if (gif->remainder_byte == 0) // still nothing ? That is the end data block
        {
          File_error = 2;
          GFX2_Log(GFX2_WARNING, "GIF 0 sized data block\n");
          return gif->current_code;
        }
      }
      if(Read_byte(GIF_file,&gif->last_byte)!=1)
      {
        File_error = 2;
        GFX2_Log(GFX2_ERROR, "GIF failed to load data byte\n");
        return 0;
      }
      gif->remainder_byte--;
      gif->remainder_bits=8;
    }

    current_nb_bits=(nb_bits_to_process<=gif->remainder_bits)?nb_bits_to_process:gif->remainder_bits;

    gif->current_code |= (gif->last_byte & ((1<<current_nb_bits)-1))<<nb_bits_processed;
    gif->last_byte >>= current_nb_bits;
    nb_bits_processed += current_nb_bits;
    nb_bits_to_process -= current_nb_bits;
    gif->remainder_bits -= current_nb_bits;
  }

  return gif->current_code;
}

/// Put a new pixel
static void GIF_new_pixel(T_IO_Context * context, T_GIF_context * gif, T_GIF_IDB *idb, int is_transparent, byte color)
{
  if (!is_transparent || color!=context->Transparent_color)
    Set_pixel(context, idb->Pos_X+gif->pos_X, idb->Pos_Y+gif->pos_Y,color);

  gif->pos_X++;

  if (gif->pos_X >= idb->Image_width)
  {
    gif->pos_X=0;

    if (!gif->interlaced)
    {
      gif->pos_Y++;
      if (gif->pos_Y >= idb->Image_height)
        gif->stop = 1;
    }
    else
    {
      switch (gif->pass)
      {
        case 0 :
        case 1 : gif->pos_Y+=8;
                 break;
        case 2 : gif->pos_Y+=4;
                 break;
        default: gif->pos_Y+=2;
      }

      if (gif->pos_Y >= idb->Image_height)
      {
        switch(++(gif->pass))
        {
        case 1 : gif->pos_Y=4;
                 break;
        case 2 : gif->pos_Y=2;
                 break;
        case 3 : gif->pos_Y=1;
                 break;
        case 4 : gif->stop = 1;
        }
      }
    }
  }
}


/// Load GIF file
void Load_GIF(T_IO_Context * context)
{
  FILE *GIF_file;
  int image_mode = -1;
  char signature[6];

  word * alphabet_stack;     // Pile de décodage d'une chaîne
  word * alphabet_prefix;  // Table des préfixes des codes
  word * alphabet_suffix;  // Table des suffixes des codes
  word   alphabet_free;     // Position libre dans l'alphabet
  word   alphabet_max;      // Nombre d'entrées possibles dans l'alphabet
  word   alphabet_stack_pos; // Position dans la pile de décodage d'un chaîne

  T_GIF_context GIF;
  T_GIF_LSDB LSDB;
  T_GIF_IDB IDB;
  T_GIF_GCE GCE;

  word nb_colors;       // Nombre de couleurs dans l'image
  word color_index; // index de traitement d'une couleur
  byte size_to_read; // Nombre de données à lire      (divers)
  byte block_identifier;  // Code indicateur du type de bloc en cours
  byte initial_nb_bits;   // Nb de bits au début du traitement LZW
  word special_case=0;       // Mémoire pour le cas spécial
  word old_code=0;       // Code précédent
  word byte_read;         // Sauvegarde du code en cours de lecture
  word value_clr;        // Valeur <=> Clear tables
  word value_eof;        // Valeur <=> End d'image
  long file_size;
  int number_LID; // Nombre d'images trouvées dans le fichier
  int current_layer = 0;
  int last_delay = 0;
  byte is_transparent = 0;
  byte is_looping=0;
  enum PIXEL_RATIO ratio;
  byte disposal_method = DISPOSAL_METHOD_RESTORE_BGCOLOR;

  byte previous_disposal_method = DISPOSAL_METHOD_RESTORE_BGCOLOR;
  word previous_width=0;
  word previous_height=0;
  word previous_pos_x=0;
  word previous_pos_y=0;

  /////////////////////////////////////////////////// FIN DES DECLARATIONS //


  number_LID=0;

  if ((GIF_file=Open_file_read(context)))
  {
    file_size=File_length_file(GIF_file);
    if ( (Read_bytes(GIF_file,signature,6)) &&
         ( (memcmp(signature,"GIF87a",6)==0) ||
           (memcmp(signature,"GIF89a",6)==0) ) )
    {

      // Allocation de mémoire pour les tables & piles de traitement:
      alphabet_stack  = (word *)GFX2_malloc(4096*sizeof(word));
      alphabet_prefix = (word *)GFX2_malloc(4096*sizeof(word));
      alphabet_suffix = (word *)GFX2_malloc(4096*sizeof(word));

      if (Read_word_le(GIF_file,&(LSDB.Width))
      && Read_word_le(GIF_file,&(LSDB.Height))
      && Read_byte(GIF_file,&(LSDB.Resol))
      && Read_byte(GIF_file,&(LSDB.Backcol))
      && Read_byte(GIF_file,&(LSDB.Aspect))
        )
      {
        // Lecture du Logical Screen Descriptor Block réussie:

        Original_screen_X=LSDB.Width;
        Original_screen_Y=LSDB.Height;

        ratio=PIXEL_SIMPLE;          //  (49 + 15) / 64 = 1:1
        if (LSDB.Aspect != 0) {
          if (LSDB.Aspect < 25)      //  (17 + 15) / 64 = 1:2
            ratio=PIXEL_TALL;
          else if (LSDB.Aspect < 41) //  (33 + 15) / 64 = 3:4
            ratio=PIXEL_TALL3;
          else if (LSDB.Aspect > 82) // (113 + 15) / 64 = 2:1
            ratio=PIXEL_WIDE;
        }

        Pre_load(context, LSDB.Width,LSDB.Height,file_size,FORMAT_GIF,ratio,(LSDB.Resol&7)+1);

        // Palette globale dispo = (LSDB.Resol  and $80)
        // Profondeur de couleur =((LSDB.Resol  and $70) shr 4)+1
        // Nombre de bits/pixel  = (LSDB.Resol  and $07)+1
        // Ordre de Classement   = (LSDB.Aspect and $80)

        nb_colors=(1 << ((LSDB.Resol & 0x07)+1));
        if (LSDB.Resol & 0x80)
        {
          // Palette globale dispo:

          if (Config.Clear_palette)
            memset(context->Palette,0,sizeof(T_Palette));

          // Load the palette
          for(color_index=0;color_index<nb_colors;color_index++)
          {
            Read_byte(GIF_file,&(context->Palette[color_index].R));
            Read_byte(GIF_file,&(context->Palette[color_index].G));
            Read_byte(GIF_file,&(context->Palette[color_index].B));
          }
        }

        // On lit un indicateur de block
        Read_byte(GIF_file,&block_identifier);
        while (block_identifier!=0x3B && !File_error)
        {
          switch (block_identifier)
          {
            case 0x21: // Bloc d'extension
            {
              byte function_code;
              // Lecture du code de fonction:
              Read_byte(GIF_file,&function_code);
              // Lecture de la taille du bloc:
              Read_byte(GIF_file,&size_to_read);
              while (size_to_read!=0 && !File_error)
              {
                switch(function_code)
                {
                  case 0xFE: // Comment Block Extension
                    // On récupère le premier commentaire non-vide,
                    // on jette les autres.
                    if (context->Comment[0]=='\0')
                    {
                      int nb_char_to_keep = MIN(size_to_read, COMMENT_SIZE);

                      Read_bytes(GIF_file,context->Comment,nb_char_to_keep);
                      context->Comment[nb_char_to_keep+1]='\0';
                      // Si le commentaire etait trop long, on fait avance-rapide
                      // sur la suite.
                      if (size_to_read>nb_char_to_keep)
                        fseek(GIF_file,size_to_read-nb_char_to_keep,SEEK_CUR);
                    }
                    // Lecture de la taille du bloc suivant:
                    Read_byte(GIF_file,&size_to_read);
                    break;
                  case 0xF9: // Graphics Control Extension
                    // Prévu pour la transparence
                    if ( Read_byte(GIF_file,&(GCE.Packed_fields))
                      && Read_word_le(GIF_file,&(GCE.Delay_time))
                      && Read_byte(GIF_file,&(GCE.Transparent_color)))
                    {
                      previous_disposal_method = disposal_method;
                      disposal_method = (GCE.Packed_fields >> 2) & 7;
                      last_delay = GCE.Delay_time;
                      context->Transparent_color= GCE.Transparent_color;
                      is_transparent = GCE.Packed_fields & 1;
                      GFX2_Log(GFX2_DEBUG, "GIF Graphics Control Extension : transp=%d (color #%u) delay=%ums disposal_method=%d\n", is_transparent, GCE.Transparent_color, 10*GCE.Delay_time, disposal_method);
                      if (number_LID == 0)
                        context->Background_transparent = is_transparent;
                      is_transparent &= is_looping;
                    }
                    else
                      File_error=2;
                    // Lecture de la taille du bloc suivant:
                    Read_byte(GIF_file,&size_to_read);
                    break;

                  case 0xFF: // Application Extension
                    // Normally, always a 11-byte block
                    if (size_to_read == 0x0B)
                    {
                      char aeb[0x0B];
                      Read_bytes(GIF_file,aeb, 0x0B);
                      GFX2_Log(GFX2_DEBUG, "GIF extension \"%.11s\"\n", aeb);
                      if (File_error)
                        ;
                      else if (!memcmp(aeb,"NETSCAPE2.0",0x0B))
                      {
                        is_looping=1;
                        // The well-known Netscape extension.
                        // Load as an animation
                        Set_image_mode(context, IMAGE_MODE_ANIMATION);
                        // Skip sub-block
                        do
                        {
                          if (! Read_byte(GIF_file,&size_to_read))
                            File_error=1;
                          fseek(GIF_file,size_to_read,SEEK_CUR);
                        } while (!File_error && size_to_read!=0);
                      }
                      else if (!memcmp(aeb,"GFX2PATH\x00\x00\x00",0x0B))
                      {
                        // Original file path
                        Read_byte(GIF_file,&size_to_read);
                        if (!File_error && size_to_read > 0)
                        {
                          free(context->Original_file_directory);
                          context->Original_file_directory = GFX2_malloc(size_to_read);
                          Read_bytes(GIF_file, context->Original_file_directory, size_to_read);
                          Read_byte(GIF_file, &size_to_read);
                          if (!File_error && size_to_read > 0)
                          {
                            free(context->Original_file_name);
                            context->Original_file_name = GFX2_malloc(size_to_read);
                            Read_bytes(GIF_file, context->Original_file_name, size_to_read);
                            Read_byte(GIF_file, &size_to_read); // Normally 0
                          }
                        }
                      }
                      else if (!memcmp(aeb,"CRNG\0\0\0\0" "1.0",0x0B))
                      {
                        // Color animation. Similar to a CRNG chunk in IFF file format.
                        word rate;
                        word flags;
                        byte col1;
                        byte col2;
                        //
                        Read_byte(GIF_file,&size_to_read);
                        for(;size_to_read>0 && !File_error;size_to_read-=6)
                        {
                          if ( (Read_word_be(GIF_file, &rate))
                            && (Read_word_be(GIF_file, &flags))
                            && (Read_byte(GIF_file, &col1))
                            && (Read_byte(GIF_file, &col2)))
                          {
                            if (col1 != col2)
                            {
                              // Valid cycling range
                              context->Cycle_range[context->Color_cycles].Start = MIN(col1, col2);
                              context->Cycle_range[context->Color_cycles].End = MAX(col1, col2);
                              context->Cycle_range[context->Color_cycles].Inverse = (flags&2)?1:0;
                              context->Cycle_range[context->Color_cycles].Speed = (flags&1)?rate/78:0;

                              context->Color_cycles++;
                            }
                          }
                          else
                          {
                            File_error=1;
                          }
                        }
                        // Read end-of-block delimiter
                        if (!File_error)
                          Read_byte(GIF_file,&size_to_read);
                        if (size_to_read!=0)
                          File_error=1;
                      }
                      else if (0 == memcmp(aeb, "GFX2MODE", 8))
                      {
                        Read_byte(GIF_file,&size_to_read);
                        if (size_to_read > 0)
                        { // read the image mode. We'll set it after having loaded all layers.
                          char * label = GFX2_malloc((size_t)size_to_read + 1);
                          Read_bytes(GIF_file, label, size_to_read);
                          label[size_to_read] = '\0';
                          image_mode = Constraint_mode_from_label(label);
                          GFX2_Log(GFX2_DEBUG, "    mode = %s (%d)\n", label, image_mode);
                          free(label);
                          Read_byte(GIF_file,&size_to_read);
                          // be future proof, skip following sub-blocks :
                          while (size_to_read!=0 && !File_error)
                          {
                            if (fseek(GIF_file,size_to_read,SEEK_CUR) < 0)
                              File_error = 1;
                            if (!Read_byte(GIF_file,&size_to_read))
                              File_error = 1;
                          }
                        }
                      }
                      else
                      {
                        // Unknown extension, skip.
                        Read_byte(GIF_file,&size_to_read);
                        while (size_to_read!=0 && !File_error)
                        {
                          if (fseek(GIF_file,size_to_read,SEEK_CUR) < 0)
                            File_error = 1;
                          if (!Read_byte(GIF_file,&size_to_read))
                            File_error = 1;
                        }
                      }
                    }
                    else
                    {
                      fseek(GIF_file,size_to_read,SEEK_CUR);
                      // Lecture de la taille du bloc suivant:
                      Read_byte(GIF_file,&size_to_read);
                    }
                    break;

                  default:
                    // On saute le bloc:
                    fseek(GIF_file,size_to_read,SEEK_CUR);
                    // Lecture de la taille du bloc suivant:
                    Read_byte(GIF_file,&size_to_read);
                    break;
                }
              }
            }
            break;
            case 0x2C: // Local Image Descriptor
            {
              if (number_LID!=0)
              {
                // This a second layer/frame, or more.
                // Attempt to add a layer to current image
                current_layer++;
                Set_loading_layer(context, current_layer);
                if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
                {
                  // Copy the content of previous layer.
                  memcpy(
                    Main.backups->Pages->Image[Main.current_layer].Pixels,
                    Main.backups->Pages->Image[Main.current_layer-1].Pixels,
                    Main.backups->Pages->Width*Main.backups->Pages->Height);
                }
                else
                {
                  Fill_canvas(context, is_transparent ? context->Transparent_color : LSDB.Backcol);
                }
              }
              else
              {
                // First frame/layer, fill canvas with backcolor
                Fill_canvas(context, is_transparent ? context->Transparent_color : LSDB.Backcol);
              }
              // Duration was set in the previously loaded GCE
              Set_frame_duration(context, last_delay*10);
              number_LID++;

              // lecture de 10 derniers octets
              if ( Read_word_le(GIF_file,&(IDB.Pos_X))
                && Read_word_le(GIF_file,&(IDB.Pos_Y))
                && Read_word_le(GIF_file,&(IDB.Image_width))
                && Read_word_le(GIF_file,&(IDB.Image_height))
                && Read_byte(GIF_file,&(IDB.Indicator))
                && IDB.Image_width && IDB.Image_height)
              {
                GFX2_Log(GFX2_DEBUG, "GIF Image descriptor %u Pos (%u,%u) %ux%u %s%slocal palette(%ubpp)\n",
                         number_LID, IDB.Pos_X, IDB.Pos_Y, IDB.Image_width, IDB.Image_height,
                         (IDB.Indicator & 0x40) ? "interlaced " : "", (IDB.Indicator & 0x80) ? "" : "no ",
                         (IDB.Indicator & 7) + 1);
                // Palette locale dispo = (IDB.Indicator and $80)
                // Image entrelacée     = (IDB.Indicator and $40)
                // Ordre de classement  = (IDB.Indicator and $20)
                // Nombre de bits/pixel = (IDB.Indicator and $07)+1 (si palette locale dispo)

                if (IDB.Indicator & 0x80)
                {
                  // Palette locale dispo

                  if (Config.Clear_palette)
                    memset(context->Palette,0,sizeof(T_Palette));

                  nb_colors=(1 << ((IDB.Indicator & 0x07)+1));
                  // Load the palette
                  for(color_index=0;color_index<nb_colors;color_index++)
                  {
                    Read_byte(GIF_file,&(context->Palette[color_index].R));
                    Read_byte(GIF_file,&(context->Palette[color_index].G));
                    Read_byte(GIF_file,&(context->Palette[color_index].B));
                  }

                }
                if (number_LID!=1)
                {
                  // This a second layer/frame, or more.
                  if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
                  {
                    // Need to clear previous image to back-color.
                    if (previous_disposal_method==DISPOSAL_METHOD_RESTORE_BGCOLOR)
                    {
                      int y;
                      for (y=0; y<previous_height; y++)
                        memset(
                          Main.backups->Pages->Image[Main.current_layer].Pixels
                           + (previous_pos_y+y)* Main.backups->Pages->Width+previous_pos_x,
                          is_transparent ? context->Transparent_color : LSDB.Backcol,
                          previous_width);
                    }
                  }
                }
                previous_height=IDB.Image_height;
                previous_width=IDB.Image_width;
                previous_pos_x=IDB.Pos_X;
                previous_pos_y=IDB.Pos_Y;

                File_error=0;
                if (!Read_byte(GIF_file,&(initial_nb_bits)))
                  File_error=1;

                value_clr    =(1<<initial_nb_bits)+0;
                value_eof    =(1<<initial_nb_bits)+1;
                alphabet_free=(1<<initial_nb_bits)+2;

                GIF.nb_bits  =initial_nb_bits + 1;
                alphabet_max      =((1 <<  GIF.nb_bits)-1);
                GIF.interlaced    =(IDB.Indicator & 0x40);
                GIF.pass         =0;

                /*Init_lecture();*/


                GIF.stop = 0;

                //////////////////////////////////////////// DECOMPRESSION LZW //

                GIF.pos_X=0;
                GIF.pos_Y=0;
                alphabet_stack_pos=0;
                GIF.last_byte    =0;
                GIF.remainder_bits    =0;
                GIF.remainder_byte    =0;

                while ( (GIF_get_next_code(GIF_file, &GIF)!=value_eof) && (!File_error) )
                {
                  if (GIF.current_code > alphabet_free)
                  {
                    GFX2_Log(GFX2_INFO, "Load_GIF() Invalid code %u (should be <=%u)\n", GIF.current_code, alphabet_free);
                    File_error=2;
                    break;
                  }
                  else if (GIF.current_code != value_clr)
                  {
                    byte_read = GIF.current_code;
                    if (alphabet_free == GIF.current_code)
                    {
                      GIF.current_code=old_code;
                      alphabet_stack[alphabet_stack_pos++]=special_case;
                    }

                    while (GIF.current_code > value_clr)
                    {
                      if (GIF.current_code >= 4096)
                      {
                        GFX2_Log(GFX2_ERROR, "Load_GIF() GIF.current_code = %u >= 4096\n", GIF.current_code);
                        File_error = 2;
                        break;
                      }
                      alphabet_stack[alphabet_stack_pos++] = alphabet_suffix[GIF.current_code];
                      GIF.current_code = alphabet_prefix[GIF.current_code];
                    }

                    special_case = alphabet_stack[alphabet_stack_pos++] = GIF.current_code;

                    do
                      GIF_new_pixel(context, &GIF, &IDB, is_transparent, alphabet_stack[--alphabet_stack_pos]);
                    while (alphabet_stack_pos!=0);

                    alphabet_prefix[alphabet_free  ]=old_code;
                    alphabet_suffix[alphabet_free++]=GIF.current_code;
                    old_code=byte_read;

                    if (alphabet_free>alphabet_max)
                    {
                      if (GIF.nb_bits<12)
                        alphabet_max      =((1 << (++GIF.nb_bits))-1);
                    }
                  }
                  else // Clear code
                  {
                    GIF.nb_bits   = initial_nb_bits + 1;
                    alphabet_max  = ((1 <<  GIF.nb_bits)-1);
                    alphabet_free = (1<<initial_nb_bits)+2;
                    special_case  = GIF_get_next_code(GIF_file, &GIF);
                    if (GIF.current_code >= value_clr)
                    {
                      GFX2_Log(GFX2_INFO, "Load_GIF() Invalid code %u just after clear (=%u)!\n",
                               GIF.current_code, value_clr);
                      File_error = 2;
                      break;
                    }
                    old_code      = GIF.current_code;
                    GIF_new_pixel(context, &GIF, &IDB, is_transparent, GIF.current_code);
                  }
                }

                if (File_error == 2 && GIF.pos_X == 0 && GIF.pos_Y == IDB.Image_height)
                  File_error=0;

                if (File_error >= 0 && !GIF.stop)
                  File_error=2;

                // No need to read more than one frame in animation preview mode
                if (context->Type == CONTEXT_PREVIEW && is_looping)
                {
                  goto early_exit;
                }
                // Same with brush
                if (context->Type == CONTEXT_BRUSH && is_looping)
                {
                  goto early_exit;
                }

              } // Le fichier contenait un IDB
              else
                File_error=2;
            }
            default:
            break;
          }
          // Lecture du code de fonction suivant:
          if (!Read_byte(GIF_file,&block_identifier))
            File_error=2;
        }

        // set the mode that have been read previously.
        if (image_mode > 0)
          Set_image_mode(context, image_mode);
      } // Le fichier contenait un LSDB
      else
        File_error=1;

      early_exit:

      // Libération de la mémoire utilisée par les tables & piles de traitement:
      free(alphabet_suffix);
      free(alphabet_prefix);
      free(alphabet_stack);
      alphabet_suffix = alphabet_prefix = alphabet_stack = NULL;
    } // Le fichier contenait au moins la signature GIF87a ou GIF89a
    else
      File_error=1;

    fclose(GIF_file);

  } // Le fichier était ouvrable
  else
    File_error=1;
}


// -- Sauver un fichier au format GIF ---------------------------------------

/// Flush the buffer
static void GIF_empty_buffer(FILE * file, T_GIF_context *gif, byte * GIF_buffer)
{
  if (gif->remainder_byte)
  {
    GIF_buffer[0] = gif->remainder_byte;

    if (!Write_bytes(file, GIF_buffer, (size_t)gif->remainder_byte + 1))
      File_error = 1;

    gif->remainder_byte = 0;
  }
}

/// Write a code (GIF_nb_bits bits)
static void GIF_set_code(FILE * GIF_file, T_GIF_context * gif, byte * GIF_buffer, word Code)
{
  word nb_bits_to_process = gif->nb_bits;
  word nb_bits_processed  =0;
  word current_nb_bits;

  while (nb_bits_to_process)
  {
    current_nb_bits = (nb_bits_to_process <= (8-gif->remainder_bits)) ?
                          nb_bits_to_process: (8-gif->remainder_bits);

    gif->last_byte |= (Code & ((1<<current_nb_bits)-1))<<gif->remainder_bits;
    Code>>=current_nb_bits;
    gif->remainder_bits    +=current_nb_bits;
    nb_bits_processed  +=current_nb_bits;
    nb_bits_to_process-=current_nb_bits;

    if (gif->remainder_bits==8) // Il ne reste plus de bits à coder sur l'octet courant
    {
      // Ecrire l'octet à balancer:
      GIF_buffer[++(gif->remainder_byte)] = gif->last_byte;

      // Si on a atteint la fin du bloc de Raster Data
      if (gif->remainder_byte==255)
        // On doit vider le buffer qui est maintenant plein
        GIF_empty_buffer(GIF_file, gif, GIF_buffer);

      gif->last_byte=0;
      gif->remainder_bits=0;
    }
  }
}


/// Read the next pixel
static byte GIF_next_pixel(T_IO_Context *context, T_GIF_context *gif, T_GIF_IDB *idb)
{
  byte temp;

  temp = Get_pixel(context, gif->pos_X, gif->pos_Y);

  if (++gif->pos_X >= (idb->Image_width + idb->Pos_X))
  {
    gif->pos_X = idb->Pos_X;
    if (++gif->pos_Y >= (idb->Image_height + idb->Pos_Y))
      gif->stop = 1;
  }

  return temp;
}

struct gif_alphabet {
  word prefix[4096];    // code prefix array
  word suffix[4096];    // code suffix array
  word daughter[4096];  // daughter strings array (greater length)
  word sister[4096];    // sister strings array (same length)
  word free;            // first free slot in the alphabet
  word max;             // maximum number of entry in the alphabet
};

/// Save a GIF file
void Save_GIF(T_IO_Context * context)
{
  FILE * GIF_file;
  byte GIF_buffer[256];   // buffer d'écriture de bloc de données compilées

  struct gif_alphabet * alphabet;
  word   start;            // Code précédent (sert au linkage des chaînes)
  int    descend;          // Booléen "On vient de descendre"

  T_GIF_context GIF;
  T_GIF_LSDB LSDB;
  T_GIF_IDB IDB;


  byte block_identifier;  // Code indicateur du type de bloc en cours
  word current_string;   // Code de la chaîne en cours de traitement
  byte current_char;         // Caractère à coder
  word index;            // index de recherche de chaîne
  int current_layer;

  word clear;   // LZW clear code
  word eof;     // End of image code

  /////////////////////////////////////////////////// FIN DES DECLARATIONS //

  File_error=0;

  if ((GIF_file=Open_file_write(context)))
  {
    // On écrit la signature du fichier
    if (Write_bytes(GIF_file,"GIF89a",6))
    {
      // La signature du fichier a été correctement écrite.

      // Allocation de mémoire pour les tables
      alphabet = (struct gif_alphabet *)GFX2_malloc(sizeof(struct gif_alphabet));
      if (alphabet == NULL)
      {
        File_error = 1;
        fclose(GIF_file);
        return;
      }

      // On initialise le LSDB du fichier
      if (Config.Screen_size_in_GIF && Screen_width >= context->Width && Screen_height >= context->Height)
      {
        // Canvas bigger than the image
        LSDB.Width=Screen_width;
        LSDB.Height=Screen_height;
      }
      else
      {
        LSDB.Width=context->Width;
        LSDB.Height=context->Height;
      }
      LSDB.Resol  = 0xF7;  // Global palette of 256 entries, 256 color image
      // 0xF7 = 1111 0111
      // <Packed Fields>  =      Global Color Table Flag       1 Bit
      //                         Color Resolution              3 Bits
      //                         Sort Flag                     1 Bit
      //                         Size of Global Color Table    3 Bits
      LSDB.Backcol=context->Transparent_color;
      switch(context->Ratio)
      {
        case PIXEL_TALL:
        case PIXEL_TALL2:
          LSDB.Aspect = 17; // 1:2 = 2:4
          break;
        case PIXEL_TALL3:
          LSDB.Aspect = 33; // 3:4
          break;
        case PIXEL_WIDE:
        case PIXEL_WIDE2:
          LSDB.Aspect = 113; // 2:1 = 4:2
          break;
        default:
          LSDB.Aspect = 0; // undefined, which is most frequent.
          // 49 would be 1:1 ratio
      }

      // On sauve le LSDB dans le fichier

      if (Write_word_le(GIF_file,LSDB.Width) &&
          Write_word_le(GIF_file,LSDB.Height) &&
          Write_byte(GIF_file,LSDB.Resol) &&
          Write_byte(GIF_file,LSDB.Backcol) &&
          Write_byte(GIF_file,LSDB.Aspect) )
      {
        // Le LSDB a été correctement écrit.
        int i;
        // On sauve la palette
        for(i=0;i<256 && !File_error;i++)
        {
          if (!Write_byte(GIF_file,context->Palette[i].R)
            ||!Write_byte(GIF_file,context->Palette[i].G)
            ||!Write_byte(GIF_file,context->Palette[i].B))
            File_error=1;
        }
        if (!File_error)
        {
          // La palette a été correctement écrite.

          /// - "Netscape" animation extension :
          /// <pre>
          ///   0x21       Extension Label
          ///   0xFF       Application Extension Label
          ///   0x0B       Block Size
          ///   "NETSCAPE" Application Identifier (8 bytes)
          ///   "2.0"      Application Authentication Code (3 bytes)
          ///   0x03       Sub-block Data Size
          ///   0xLL       01 to loop
          ///   0xSSSS     (little endian) number of loops, 0 means infinite loop
          ///   0x00 Block terminator </pre>
          /// see http://www.vurdalakov.net/misc/gif/netscape-looping-application-extension
          if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
          {
            if (context->Nb_layers>1)
              Write_bytes(GIF_file,"\x21\xFF\x0BNETSCAPE2.0\x03\x01\x00\x00\x00",19);
          }
          else if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode > IMAGE_MODE_ANIMATION)
          {
            /// - GrafX2 extension to store ::IMAGE_MODES :
            /// <pre>
            ///   0x21       Extension Label
            ///   0xFF       Application Extension Label
            ///   0x0B       Block Size
            ///   "GFX2MODE" Application Identifier (8 bytes)
            ///   "2.6"      Application Authentication Code (3 bytes)
            ///   0xll       Sub-block Data Size
            ///   string     label
            ///   0x00 Block terminator </pre>
            /// @see Constraint_mode_label()
            const char * label = Constraint_mode_label(Main.backups->Pages->Image_mode);
            if (label != NULL)
            {
              size_t len = strlen(label);
              // Write extension for storing IMAGE_MODE
              Write_byte(GIF_file,0x21);  // Extension Introducer
              Write_byte(GIF_file,0xff);  // Extension Label
              Write_byte(GIF_file,  11);  // Block size
              Write_bytes(GIF_file, "GFX2MODE2.6", 11); // Application Identifier + Appl. Authentication Code
              Write_byte(GIF_file, (byte)len);    // Block size
              Write_bytes(GIF_file, label, len);  // Data
              Write_byte(GIF_file, 0);    // Block terminator
            }
          }

          // Ecriture du commentaire
          if (context->Comment[0])
          {
            Write_bytes(GIF_file,"\x21\xFE",2);
            Write_byte(GIF_file, (byte)strlen(context->Comment));
            Write_bytes(GIF_file,context->Comment,strlen(context->Comment)+1);
          }
          /// - "CRNG" Color cycing extension :
          /// <pre>
          ///   0x21       Extension Label
          ///   0xFF       Application Extension Label
          ///   0x0B       Block Size
          ///   "CRNG\0\0\0\0" "CRNG" Application Identifier (8 bytes)
          ///   "1.0"      Application Authentication Code (3 bytes)
          ///   0xll       Sub-block Data Size (6 bytes per color cycle)
          ///   For each color cycle :
          ///     0xRRRR   (big endian) Rate
          ///     0xFFFF   (big endian) Flags
          ///     0xSS     start (lower color index)
          ///     0xEE     end (higher color index)
          ///   0x00       Block terminator </pre>
          if (context->Color_cycles)
          {
            int i;

            Write_bytes(GIF_file,"\x21\xff\x0B" "CRNG\0\0\0\0" "1.0",14);
            Write_byte(GIF_file,context->Color_cycles*6);
            for (i=0; i<context->Color_cycles; i++)
            {
              word flags=0;
              flags|= context->Cycle_range[i].Speed?1:0; // Cycling or not
              flags|= context->Cycle_range[i].Inverse?2:0; // Inverted

              Write_word_be(GIF_file,context->Cycle_range[i].Speed*78); // Rate
              Write_word_be(GIF_file,flags); // Flags
              Write_byte(GIF_file,context->Cycle_range[i].Start); // Min color
              Write_byte(GIF_file,context->Cycle_range[i].End); // Max color
            }
            Write_byte(GIF_file,0);
          }

          // Loop on all layers
          for (current_layer=0;
            current_layer < context->Nb_layers && !File_error;
            current_layer++)
          {
            // Write a Graphic Control Extension
            T_GIF_GCE GCE;
            byte disposal_method;

            Set_saving_layer(context, current_layer);

            GCE.Block_identifier = 0x21;
            GCE.Function = 0xF9;
            GCE.Block_size=4;

            if (context->Type == CONTEXT_MAIN_IMAGE && Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
            {
              // Animation frame
              int duration;
              if(context->Background_transparent)
                disposal_method = DISPOSAL_METHOD_RESTORE_BGCOLOR;
              else
                disposal_method = DISPOSAL_METHOD_DO_NOT_DISPOSE;
              GCE.Packed_fields=(disposal_method<<2)|(context->Background_transparent);
              duration=Get_frame_duration(context)/10;
              GCE.Delay_time=duration<0xFFFF?duration:0xFFFF;
            }
            else
            {
              // Layered image or brush
              disposal_method = DISPOSAL_METHOD_DO_NOT_DISPOSE;
              if (current_layer==0)
                GCE.Packed_fields=(disposal_method<<2)|(context->Background_transparent);
              else
                GCE.Packed_fields=(disposal_method<<2)|(1);
              GCE.Delay_time=5; // Duration 5/100s (minimum viable value for current web browsers)
              if (current_layer == context->Nb_layers -1)
                GCE.Delay_time=0xFFFF; // Infinity (10 minutes)
            }
            GCE.Transparent_color=context->Transparent_color;
            GCE.Block_terminator=0x00;

            if (Write_byte(GIF_file,GCE.Block_identifier)
             && Write_byte(GIF_file,GCE.Function)
             && Write_byte(GIF_file,GCE.Block_size)
             && Write_byte(GIF_file,GCE.Packed_fields)
             && Write_word_le(GIF_file,GCE.Delay_time)
             && Write_byte(GIF_file,GCE.Transparent_color)
             && Write_byte(GIF_file,GCE.Block_terminator)
             )
            {
              byte temp, max = 0;

              IDB.Pos_X=0;
              IDB.Pos_Y=0;
              IDB.Image_width=context->Width;
              IDB.Image_height=context->Height;
              if(current_layer > 0)
              {
                word min_X, max_X, min_Y, max_Y;
                // find bounding box of changes for Animated GIFs
                min_X = min_Y = 0xffff;
                max_X = max_Y = 0;
                for(GIF.pos_Y = 0; GIF.pos_Y < context->Height; GIF.pos_Y++) {
                  for(GIF.pos_X = 0; GIF.pos_X < context->Width; GIF.pos_X++) {
                    if (GIF.pos_X >= min_X && GIF.pos_X <= max_X && GIF.pos_Y >= min_Y && GIF.pos_Y <= max_Y)
                      continue; // already in the box
                    if(disposal_method == DISPOSAL_METHOD_DO_NOT_DISPOSE)
                    {
                      // if that pixel has same value in previous layer, no need to save it
                      Set_saving_layer(context, current_layer - 1);
                      temp = Get_pixel(context, GIF.pos_X, GIF.pos_Y);
                      Set_saving_layer(context, current_layer);
                      if(temp == Get_pixel(context, GIF.pos_X, GIF.pos_Y))
                        continue;
                    }
                    if (disposal_method == DISPOSAL_METHOD_RESTORE_BGCOLOR
                      || context->Background_transparent
                      || Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
                    {
                      // if that pixel is Backcol, no need to save it
                      if (LSDB.Backcol == Get_pixel(context, GIF.pos_X, GIF.pos_Y))
                        continue;
                    }
                    if(GIF.pos_X < min_X) min_X = GIF.pos_X;
                    if(GIF.pos_X > max_X) max_X = GIF.pos_X;
                    if(GIF.pos_Y < min_Y) min_Y = GIF.pos_Y;
                    if(GIF.pos_Y > max_Y) max_Y = GIF.pos_Y;
                  }
                }
                if((min_X <= max_X) && (min_Y <= max_Y))
                {
                  IDB.Pos_X = min_X;
                  IDB.Pos_Y = min_Y;
                  IDB.Image_width = max_X + 1 - min_X;
                  IDB.Image_height = max_Y + 1 - min_Y;
                }
                else
                {
                  // if no pixel changes, store a 1 pixel image
                  IDB.Image_width = 1;
                  IDB.Image_height = 1;
                }
              }

              // look for the maximum pixel value
              // to decide how many bit per pixel are needed.
              for(GIF.pos_Y = IDB.Pos_Y; GIF.pos_Y < IDB.Image_height + IDB.Pos_Y; GIF.pos_Y++) {
                for(GIF.pos_X = IDB.Pos_X; GIF.pos_X < IDB.Image_width + IDB.Pos_X; GIF.pos_X++) {
                  temp=Get_pixel(context, GIF.pos_X, GIF.pos_Y);
                  if(temp > max) max = temp;
                }
              }
              IDB.Nb_bits_pixel=2;  // Find the minimum bpp value to fit all pixels
              while((int)max >= (1 << IDB.Nb_bits_pixel)) {
                IDB.Nb_bits_pixel++;
              }
              GFX2_Log(GFX2_DEBUG, "GIF image #%d %ubits (%u,%u) %ux%u\n",
                       current_layer, IDB.Nb_bits_pixel, IDB.Pos_X, IDB.Pos_Y,
                       IDB.Image_width, IDB.Image_height);

              // On va écrire un block indicateur d'IDB et l'IDB du fichier
              block_identifier=0x2C;
              IDB.Indicator=0x07;    // Image non entrelacée, pas de palette locale.
              clear = 1 << IDB.Nb_bits_pixel; // Clear Code
              eof = clear + 1;                // End of Picture Code

              if ( Write_byte(GIF_file,block_identifier) &&
                   Write_word_le(GIF_file,IDB.Pos_X) &&
                   Write_word_le(GIF_file,IDB.Pos_Y) &&
                   Write_word_le(GIF_file,IDB.Image_width) &&
                   Write_word_le(GIF_file,IDB.Image_height) &&
                   Write_byte(GIF_file,IDB.Indicator) &&
                   Write_byte(GIF_file,IDB.Nb_bits_pixel))
              {
                //   Le block indicateur d'IDB et l'IDB ont étés correctements
                // écrits.

                GIF.pos_X=IDB.Pos_X;
                GIF.pos_Y=IDB.Pos_Y;
                GIF.last_byte=0;
                GIF.remainder_bits=0;
                GIF.remainder_byte=0;

#define GIF_INVALID_CODE (65535)
                index=GIF_INVALID_CODE;
                File_error=0;
                GIF.stop=0;

                // Réintialisation de la table:
                alphabet->free = clear + 2;  // 258 for 8bpp
                GIF.nb_bits = IDB.Nb_bits_pixel + 1; // 9 for 8 bpp
                alphabet->max = clear+clear-1;  // 511 for 8bpp
                GIF_set_code(GIF_file, &GIF, GIF_buffer, clear);  //256 for 8bpp
                for (start=0; start<4096; start++)
                {
                  alphabet->daughter[start] = GIF_INVALID_CODE;
                  alphabet->sister[start] = GIF_INVALID_CODE;
                }

                ////////////////////////////////////////////// COMPRESSION LZW //

                start=current_string=GIF_next_pixel(context, &GIF, &IDB);
                descend=1;

                while ((!GIF.stop) && (!File_error))
                {
                  current_char=GIF_next_pixel(context, &GIF, &IDB);

                  // look for (current_string,current_char) in the alphabet
                  while ( (index != GIF_INVALID_CODE) &&
                          ( (current_string != alphabet->prefix[index]) ||
                            (current_char   != alphabet->suffix[index]) ) )
                  {
                    descend = 0;
                    start = index;
                    index = alphabet->sister[index];
                  }

                  if (index != GIF_INVALID_CODE)
                  {
                    // (current_string,current_char) == (alphabet_prefix,alphabet_suffix)[index]
                    // We have found (current_string,current_char) in the alphabet
                    // at the index position. So go on and prepare for then next character

                    descend = 1;
                    start = current_string = index;
                    index = alphabet->daughter[index];
                  }
                  else
                  {
                    // (current_string,current_char) was not found in the alphabet
                    // so write current_string to the Gif stream
                    GIF_set_code(GIF_file, &GIF, GIF_buffer, current_string);

                    if(alphabet->free < 4096) {
                      // link current_string and the new one
                      if (descend)
                        alphabet->daughter[start] = alphabet->free;
                      else
                        alphabet->sister[start] = alphabet->free;

                      // add (current_string,current_char) to the alphabet
                      alphabet->prefix[alphabet->free] = current_string;
                      alphabet->suffix[alphabet->free] = current_char;
                      alphabet->free++;
                    }

                    if (alphabet->free >= 4096)
                    {
                      // clear alphabet
                      GIF_set_code(GIF_file, &GIF, GIF_buffer, clear);    // 256 for 8bpp
                      alphabet->free=clear+2;  // 258 for 8bpp
                      GIF.nb_bits = IDB.Nb_bits_pixel + 1;  // 9 for 8bpp
                      alphabet->max = clear+clear-1;    // 511 for 8bpp
                      for (start=0;start<4096;start++)
                      {
                        alphabet->daughter[start] = GIF_INVALID_CODE;
                        alphabet->sister[start] = GIF_INVALID_CODE;
                      }
                    }
                    else if (alphabet->free > (alphabet->max + 1))
                    {
                      // On augmente le nb de bits

                      GIF.nb_bits++;
                      alphabet->max = (1<<GIF.nb_bits)-1;
                    }

                    // initialize current_string as the string "current_char"
                    index = alphabet->daughter[current_char];
                    start = current_string = current_char;
                    descend = 1;
                  }
                }

                if (!File_error)
                {
                  // Write the last code (before EOF)
                  GIF_set_code(GIF_file, &GIF, GIF_buffer, current_string);

                  // we need to update alphabet->free / GIF.nb_bits here because
                  // the decoder will update them after each code,
                  // so in very rare cases there might be a problem if we
                  // don't do it.
                  // see http://pulkomandy.tk/projects/GrafX2/ticket/125
                  if(alphabet->free < 4096)
                  {
                    alphabet->free++;
                    if ((alphabet->free > alphabet->max+1) && (GIF.nb_bits < 12))
                    {
                      GIF.nb_bits++;
                      alphabet->max = (1 << GIF.nb_bits) - 1;
                    }
                  }

                  GIF_set_code(GIF_file, &GIF, GIF_buffer, eof);  // 257 for 8bpp    // Code de End d'image
                  if (GIF.remainder_bits!=0)
                  {
                    // Write last byte (this is an incomplete byte)
                    GIF_buffer[++GIF.remainder_byte]=GIF.last_byte;
                    GIF.last_byte=0;
                    GIF.remainder_bits=0;
                  }
                  GIF_empty_buffer(GIF_file, &GIF, GIF_buffer); // On envoie les dernières données du buffer GIF dans le buffer KM

                  // On écrit un \0
                  if (! Write_byte(GIF_file,'\x00'))
                    File_error=1;
                }

              } // On a pu écrire l'IDB
              else
                File_error=1;
            }
            else
              File_error=1;
          }

          // After writing all layers
          if (!File_error)
          {
            /// - If requested, write a specific extension for storing
            /// original file path.
            /// This is used by the backup system.
            /// The format is :
            /// <pre>
            ///   0x21       Extension Label
            ///   0xFF       Application Extension Label
            ///   0x0B       Block Size
            ///   "GFX2PATH" "GFX2PATH" Application Identifier (8 bytes)
            ///   "\0\0\0"   Application Authentication Code (3 bytes)
            ///   0xll       Sub-block Data Size : path size (including null)
            ///   "..path.." path (null-terminated)
            ///   0xll       Sub-block Data Size : filename size (including null)
            ///   "..file.." file name (null-terminated)
            ///   0x00       Block terminator </pre>
            if (context->Original_file_name != NULL
             && context->Original_file_directory != NULL)
            {
              long name_size = 1+strlen(context->Original_file_name);
              long dir_size = 1+strlen(context->Original_file_directory);
              if (name_size<256 && dir_size<256)
              {
                if (! Write_bytes(GIF_file,"\x21\xFF\x0BGFX2PATH\x00\x00\x00", 14)
                || ! Write_byte(GIF_file,dir_size)
                || ! Write_bytes(GIF_file, context->Original_file_directory, dir_size)
                || ! Write_byte(GIF_file,name_size)
                || ! Write_bytes(GIF_file, context->Original_file_name, name_size)
                || ! Write_byte(GIF_file,0))
                  File_error=1;
              }
            }

            // On écrit un GIF TERMINATOR, exigé par SVGA et SEA.
            if (! Write_byte(GIF_file,'\x3B'))
              File_error=1;
          }

        } // On a pu écrire la palette
        else
          File_error=1;

      } // On a pu écrire le LSDB
      else
        File_error=1;

      // Libération de la mémoire utilisée par les tables
      free(alphabet);

    } // On a pu écrire la signature du fichier
    else
      File_error=1;

    fclose(GIF_file);
    if (File_error)
      Remove_file(context);

  } // On a pu ouvrir le fichier en écriture
  else
    File_error=1;

}

/** @} */
