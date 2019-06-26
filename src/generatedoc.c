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
#include "windows.h" // for T_Toolbar_button

#if defined(USE_X11)
word X11_key_mod = 0;
#endif

static T_Toolbar_button Buttons[NB_BUTTONS];

/// available skins
const char * const skins[] = {
  "classic",
  "modern",
  "DPaint",
  "scenish",
  "Aurora",
  NULL
};

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

#define Init_button(btn, tooltip, x_offset, y_offset, width, height, shape, \
                    left_action, right_action, left_instant, right_instant, \
                    unselect_action, family) \
        Buttons[btn].Tooltip = tooltip; \
        Buttons[btn].X_offset = x_offset; \
        Buttons[btn].Y_offset = y_offset; \
        Buttons[btn].Width = width; \
        Buttons[btn].Height = height; \
        Buttons[btn].Shape = shape; \
        Buttons[btn].Family = family;
  #include "init_buttons.h"
#undef Init_button

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

static void Print_button(FILE * f, int btn)
{
  int x_offset, y_offset;

  if (btn < 0 || btn >= NB_BUTTONS)
    return;
  if (Buttons[btn].Width <= 1)
    return;
  x_offset = Buttons[btn].X_offset;
  y_offset = Buttons[btn].Y_offset;
  if (btn <= BUTTON_HIDE) // Status
    y_offset += 67;
  else if (btn <= BUTTON_ANIM_PLAY) // Animation
    y_offset += 53;
  else if (btn <= BUTTON_LAYER_SELECT) // Layers
    y_offset += 43;
  else // Main
    y_offset += 8;

  fprintf(f, "<div class=\"button\" style=\"background-position: -%dpx -%dpx",
          x_offset, y_offset);
  if (Buttons[btn].Width != 16)
    fprintf(f, "; width: %dpx", Buttons[btn].Width);
  if (Buttons[btn].Height != 16)
    fprintf(f, "; height: %dpx", Buttons[btn].Height);
  fprintf(f, "\"></div>&nbsp;");
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
      const char * p = table[index].Text;
      while (*p == ' ')
        p++;
      strncpy(title, p, sizeof(title));
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
  fprintf(f, "<script type=\"text/javascript\" src=\"grafx2.js\"></script>\n");
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
  fprintf(f, "<div class=\"help skin skin_classic\">");
  for (index = 0; index < length; index++)
  {
    if (table[index].Line_type == '-')
      continue;
//    if (table[index].Line_type != 'N') printf("%c %s\n", table[index].Line_type, table[index].Text);
    if (table[index].Line_type == 'S')
      fprintf(f, "<strong>");
    else if (table[index].Line_type == 'T' && !(index > 1 && table[index-2].Line_type == 'T'))
    {
      fprintf(f, "<h%d>", hlevel);

      if (hlevel == 1 && page >= 4)
        Print_button(f, page-4);
    }

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
        link = strstr(table[index].Text, "https://");
        if (link == NULL)
        {
          link = strstr(table[index].Text, "www.");
          prefix = "http://";
        }
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
        {
          hlevel++;
          if (page >= 4 && page < NB_BUTTONS + 4 && Buttons[page-4].Tooltip != NULL)
            fprintf(f, "\n<em>%s</em>", Buttons[page-4].Tooltip);
        }
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
  fprintf(findex, "<noscript>\n"); /* hide the skin selector when JS is disabled */
  fprintf(findex, "  <style>.skinselector { display: none; }</style>\n");
  fprintf(findex, "</noscript>\n");
  fprintf(findex, "<script type=\"text/javascript\" src=\"grafx2.js\"></script>\n");
  fprintf(findex, "</head>\n");

  fprintf(findex, "<body>\n");
  fprintf(findex, "<div class=\"skin skin_classic\">\n");
  fprintf(findex, "<div class=\"skinselector center\">\n");
  fprintf(findex, "Choose your skin :\n");
  for (i = 0; skins[i] != NULL; i++)
  {
    fprintf(findex, "<a href=\"javascript:choose_skin('%s');\">%s</a>\n", skins[i], skins[i]);
  }
  fprintf(findex, "</div>\n");
  fprintf(findex, "<div class=\"button center\" "
                  "style=\"width: 231px; height: 56px; "
                  "background-position: 0px -336px; "
                  "display: block;\"></div>");
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
    // Button = i - 4
    fclose(f);
    fprintf(findex, "<li>");
    fprintf(findex, "<a href=\"grafx2_%02d.html\">", i);
    if (i >= 4)
      Print_button(findex, i-4);
    fprintf(findex, "%s</a>", title);
    if (i >= 4 && Buttons[i-4].Tooltip != NULL)
      fprintf(findex, " %s", Buttons[i-4].Tooltip);
    fprintf(findex, "</li>\n");
  }
  fprintf(findex, "</ul>\n");
  fprintf(findex, "</div>\n");
  fprintf(findex, "</body>\n");
  fclose(findex);

  snprintf(filename, sizeof(filename), "%s/grafx2.css", path);
  f = fopen(filename, "w");
  if (f != NULL)
  {
    fprintf(f, "body {\n");
    fprintf(f, "font-family: sans-serif;\n");
    fprintf(f, "color: #222;\n");
    fprintf(f, "max-width: 40rem;\n");
    fprintf(f, "margin: auto;\n");
    fprintf(f, "}\n");
    fprintf(f, ".center {\n");
    fprintf(f, "margin: auto;\n");
    fprintf(f, "text-align: center;\n");
    fprintf(f, "}\n");
    fprintf(f, ".help {\n");
    fprintf(f, "font-family: %s;\n", "monospace");
    fprintf(f, "white-space: %s;\n", "pre");
    fprintf(f, "}\n");
    fprintf(f, "div.button {\n");
    fprintf(f, "display: inline-block;\n");
    fprintf(f, "width: 16px;\n");
    fprintf(f, "height: 16px;\n");
    fprintf(f, "}\n");
    for (i = 0; skins[i] != NULL; i++)
    {
      fprintf(f, ".skin_%s .button {\n", skins[i]);
      fprintf(f, "background-image: url(\"skin_%s.png\");\n", skins[i]);
      fprintf(f, "}\n");
    }
    fprintf(f, "@media print {\n");
    fprintf(f, " .skinselector {\n");
    fprintf(f, "  display: none;\n");
    fprintf(f, " }\n");
    fprintf(f, " body {\n");
    fprintf(f, "  -webkit-print-color-adjust: exact; /*Chrome, Safari */\n");
    fprintf(f, "  color-adjust: exact; /*Firefox*/\n");
    fprintf(f, " }\n");
    fprintf(f, "}\n");
    fclose(f);
  }

  snprintf(filename, sizeof(filename), "%s/grafx2.js", path);
  f = fopen(filename, "w");
  if (f != NULL)
  {
    fprintf(f, "function setCookie(cname, cvalue) {\n"
               "  document.cookie = cname + '=' + cvalue /*+ ';path=/'*/;\n"
               "}\n");
    fprintf(f, "function getCookie(cname) {\n"
               "  var name = cname + '=';\n"
               "  var ca = decodeURIComponent(document.cookie).split(';');\n"
               "  for (var i = 0; i < ca.length; i++) {\n"
               "    var c = ca[i];\n"
               "    while (c.charAt(0) == ' ') c = c.substring(1);\n"
               "    if (c.indexOf(name) == 0) {\n"
               "      return c.substring(name.length);\n"
               "    }\n"
               "  }\n"
               "  return '';\n"
               "}\n");
    fprintf(f, "function change_skin(newskin) {\n"
               "  var elts = document.getElementsByClassName('skin');\n"
               "  for (var i = 0; i < elts.length; i++) {\n"
               "    elts[0].className = elts[0].className.replace(/skin_.*/, newskin);\n"
               "  }\n"
               "}\n");
    fprintf(f, "function choose_skin(skin) {\n"
               "  setCookie('skin', skin);\n"
               "  change_skin('skin_' + skin);\n"
               "}\n");
    fprintf(f, "window.onload = function () {\n"
               "  var skin = getCookie('skin');\n"
               "  if (skin != '') choose_skin(skin);\n"
               "}\n");
    fclose(f);
  }
  return 0;
}
