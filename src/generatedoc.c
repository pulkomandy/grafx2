/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2011 Pawel Góralski
    Copyright 2009 Pasi Kallinen
    Copyright 2008 Peter Gordon
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
/**
 * @file generatedoc.c
 * HTML doc generator
 */
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "hotkeys.h"
#include "helpfile.h"
#include "keyboard.h" // for Key_Name()

///
/// Export the help to HTML files
static int Export_help(const char * path);

int main(int argc,char * argv[])
{
  int r;
  const char * path = ".";

  if (argc > 1) {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
      printf("Usage: %s [directory]\n", argv[0]);
      return 1;
    }
    path = argv[1];
  }
  r = Export_help(path);
  if (r < 0)
    return -r;
  return 0;
}

/// similar to Shortcut() from help.c, but relying only on default shortcuts
static word * Find_default_shortcut(word shortcut_number)
{
  short order_index;
  // Recherche dans hotkeys
  order_index=0;
  while (Ordering[order_index]!=shortcut_number)
  {
    order_index++;
    if (order_index>=NB_SHORTCUTS)
    {
      return NULL;
    }
  }
  return &(ConfigKey[order_index].Key);
}

/// Similar to Keyboard_shortcut_value() from help.c, but relying only on default shortcuts
static const char * Keyboard_default_shortcut(word shortcut_number)
{
  static char shortcuts_name[80];
  word * pointer = Find_default_shortcut(shortcut_number);
  if (pointer == NULL)
    return "(Problem)";
  else
  {
    if (pointer[0] == 0 && pointer[1] == 0)
      return "None";
    if (pointer[0] != 0 && pointer[1] == 0)
      return Key_name(pointer[0]);
    if (pointer[0] == 0 && pointer[1] != 0)
      return Key_name(pointer[1]);
      
    strcpy(shortcuts_name, Key_name(pointer[0]));
    strcat(shortcuts_name, " or ");
    strcat(shortcuts_name, Key_name(pointer[1]));
    return shortcuts_name;
  }
}

///
static const char * Export_help_table(FILE * f, unsigned int page)
{
  int hlevel = 1;
  static char title[256];
  word index;

  const T_Help_table * table = Help_section[page].Help_table;
  word length = Help_section[page].Length;

  // Line_type : 'N' for normal line
  //             'S' for a bold line
  //             'K' for a line with keyboard shortcut
  //             'T' and '-' for upper and lower titles.

  for (index = 0; index < length; index++)
  {
    if (table[index].Line_type == 'T')
    {
      strncpy(title, table[index].Text, sizeof(title));
      title[sizeof(title)-1] = '\0';
      while (index + 2 < length && table[index+2].Line_type == 'T')
      {
        size_t len = strlen(title);
        index += 2;
        if (table[index].Text[0] != ' ')
          title[len++] = ' ';
        if (len >= sizeof(title) - 1)
          break;
        strncpy(title + len, table[index].Text, sizeof(title)-len-1);
      }
      break;
    }
  }

  fprintf(f, "<!DOCTYPE html>\n");
  fprintf(f, "<html lang=\"en\">\n");
  fprintf(f, "<head>\n");
  fprintf(f, "<title>%s</title>\n", title);
  fprintf(f, "<meta charset=\"ISO8859-1\">\n");
  fprintf(f, "<link rel=\"stylesheet\" href=\"grafx2.css\" />\n");
  fprintf(f, "</head>\n");

  fprintf(f, "<body>\n");

  fprintf(f, "<div class=\"navigation\">");
  fprintf(f, "<table><tr>\n<td>");
  if (page > 0)
    fprintf(f, "<a href=\"grafx2_%02d.html\">Previous</a>\n", page - 1);
  fprintf(f, "</td><td><a href=\"index.html\">GrafX2 Help</a></td><td>");
  if (page < sizeof(Help_section)/sizeof(Help_section[0]) - 1)
    fprintf(f, "<a href=\"grafx2_%02d.html\">Next</a>\n", page + 1);
  fprintf(f, "</td></tr></table>");
  fprintf(f, "</div>\n");
  fprintf(f, "<div class=\"help\">");
  for (index = 0; index < length; index++)
  {
    if (table[index].Line_type == '-')
      continue;
//    if (table[index].Line_type != 'N') printf("%c %s\n", table[index].Line_type, table[index].Text);
    if (table[index].Line_type == 'S')
      fprintf(f, "<strong>");
    else if (table[index].Line_type == 'T' && !(index > 1 && table[index-2].Line_type == 'T'))
      fprintf(f, "<h%d>", hlevel);

    if (table[index].Line_type == 'K')
    {
      char keytmp[64];
      snprintf(keytmp, sizeof(keytmp), "<em>%s</em>", Keyboard_default_shortcut(table[index].Line_parameter));
      fprintf(f, table[index].Text, keytmp);
    }
    else
    {
      const char * prefix = "";
      char * link = strstr(table[index].Text, "http://");
      if (link == NULL)
      {
        link = strstr(table[index].Text, "www.");
        prefix = "http://";
      }
      if (link != NULL)
      {
        char * link_end = strchr(link, ' ');
        if (link_end == NULL)
        {
          link_end = strchr(link, ')');
          if (link_end == NULL)
            link_end = link + strlen(link);
        }
        fprintf(f, "%.*s", (int)(link - table[index].Text), table[index].Text);
        fprintf(f, "<a href=\"%s%.*s\">%.*s</a>%s", prefix,
                (int)(link_end - link), link, (int)(link_end - link), link, link_end);
      }
      else
      {
        // print the string and escape < and > characters
        const char * text = table[index].Text;
        while (text[0] != '\0')
        {
          size_t i = strcspn(text, "<>");
          if (text[i] == '\0')
          { // no character to escape
            fprintf(f, "%s", text);
            break;
          }
          if (i > 0)
            fprintf(f, "%.*s", (int)i, text);
          switch(text[i])
          {
            case '<':
              fprintf(f, "&lt;");
              break;
            case '>':
              fprintf(f, "&gt;");
              break;
          }
          // continue with the remaining of the string
          text += i + 1;
        }
      }
    }

    if (table[index].Line_type == 'S')
      fprintf(f, "</strong>");
    else if (table[index].Line_type == 'T')
    {
      if (index < length-2 && table[index+2].Line_type == 'T')
        fprintf(f, "\n");
      else
      {
        fprintf(f, "</h%d>", hlevel);
        if (hlevel == 1)
          hlevel++;
      }
    }
    if (table[index].Line_type != 'T')
      fprintf(f, "\n");
    //  fprintf(f, "<br>\n");
  }
  fprintf(f, "</div>");
  fprintf(f, "</body>\n");
  fprintf(f, "</html>\n");
  return title;
}

///
/// Export the help to HTML files
static int Export_help(const char * path)
{
  FILE * f;
  FILE * findex;
  char filename[MAX_PATH_CHARACTERS];
  unsigned int i;

  snprintf(filename, sizeof(filename), "%s/index.html", path);
  //GFX2_Log(GFX2_INFO, "Creating %s\n", filename);
  findex = fopen(filename, "w");
  if (findex == NULL)
  {
    fprintf(stderr, "Cannot save index HTML file %s\n", filename);
    return -1;
  }
  fprintf(findex, "<!DOCTYPE html>\n");
  fprintf(findex, "<html lang=\"en\">\n");
  fprintf(findex, "<head>\n");
  fprintf(findex, "<title>GrafX2 Help</title>\n");
  fprintf(findex, "<meta charset=\"ISO8859-1\">\n");
  fprintf(findex, "<link rel=\"stylesheet\" href=\"grafx2.css\" />\n");
  fprintf(findex, "</head>\n");

  fprintf(findex, "<body>\n");
  fprintf(findex, "<ul>\n");
  for (i = 0; i < sizeof(Help_section)/sizeof(Help_section[0]); i++)
  {
    const char * title;
    snprintf(filename, sizeof(filename), "%s/grafx2_%02d.html", path, i);
    f = fopen(filename, "w");
    if (f == NULL)
    {
      fprintf(stderr, "Cannot save HTML file %s\n", filename);
      fclose(findex);
      return -1;
    }
    //GFX2_Log(GFX2_INFO, "Saving %s\n", filename);
    title = Export_help_table(f, i);
    fclose(f);
    fprintf(findex, "<li><a href=\"grafx2_%02d.html\">%s</a></li>\n", i, title);
  }
  fprintf(findex, "</ul>\n");
  fprintf(findex, "</body>\n");
  fclose(findex);

  snprintf(filename, sizeof(filename), "%s/grafx2.css", path);
  f = fopen(filename, "w");
  if (f != NULL)
  {
    fprintf(f, ".help {\n");
    fprintf(f, "font-family: %s;\n", "monospace");
    fprintf(f, "white-space: %s;\n", "pre");
    fprintf(f, "}\n");
    fclose(f);
  }
  return 0;
}
