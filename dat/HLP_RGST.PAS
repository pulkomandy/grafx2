{ Grafx2 - The Ultimate 256-color bitmap paint program

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
}
{ Ce fichier contient la partie "Register?" de l'aide de GrafX2 }

procedure Generer_l_aide_Register;
begin
  Demarrer_section_d_aide;
                        {XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}
                        {XXXXXXXXXXXXXXXXXXXXXX}
  Ecrire_ligne_d_aide(T,'REGISTERING ?');
  Ecrire_ligne_d_aide(N,'');
  Ecrire_ligne_d_aide(N,'  You''ll probably say:');
  Ecrire_ligne_d_aide(N,'    "Does this mean that I have to pay?"');
  Ecrire_ligne_d_aide(N,'');
  Ecrire_ligne_d_aide(N,'  And we''ll reply to you: No...');
  Ecrire_ligne_d_aide(N,'');
  Ecrire_ligne_d_aide(T,'GRAFX 2.00 IS FREEWARE');
  Ecrire_ligne_d_aide(N,'');
  Ecrire_ligne_d_aide(N,'            ... But, but, but...');
  Ecrire_ligne_d_aide(N,'');
  Ecrire_ligne_d_aide(N,'  Considering the amount of work (and love)');
  Ecrire_ligne_d_aide(N,' we put in this program, we would extremely');
  Ecrire_ligne_d_aide(N,' appreciate that you "give" us something in');
  Ecrire_ligne_d_aide(N,' exchange for the use you make of it.');
  Ecrire_ligne_d_aide(N,'  Let''s say some money, or a nice picture');
  Ecrire_ligne_d_aide(N,' you drew with GrafX2, or a postcard...');
  Ecrire_ligne_d_aide(N,'');
  Ecrire_ligne_d_aide(N,'  If you make profit with an image you drew');
  Ecrire_ligne_d_aide(N,' with GrafX2, it would be great if you sent');
  Ecrire_ligne_d_aide(N,' us this picture. If you used it for a game,');
  Ecrire_ligne_d_aide(N,' would be wiser not to send all your graphs,');
  Ecrire_ligne_d_aide(N,' what about sending a copy of the game?');
  Ecrire_ligne_d_aide(T,'');
  Ecrire_ligne_d_aide(T,'FREEWARE CONVENTIONS:');
  Ecrire_ligne_d_aide(N,'');
  Ecrire_ligne_d_aide(N,'   This version of GrafX2 is being released');
  Ecrire_ligne_d_aide(N,'  to you as is.');
  Ecrire_ligne_d_aide(N,'   All bugs should be reported to either');
  Ecrire_ligne_d_aide(N,'  Robinson or X-Man.');
  Ecrire_ligne_d_aide(N,'   The authors take no responsibility for');
  Ecrire_ligne_d_aide(N,'  lost work, or damaged equipment caused');
  Ecrire_ligne_d_aide(N,'  directly or indirectly by this program.');
  Ecrire_ligne_d_aide(N,'   GrafX2 cannot be modified in any way');
  Ecrire_ligne_d_aide(N,'  without our written consent.');
  Ecrire_ligne_d_aide(N,'   Finally, the charging of monetary fees by');
  Ecrire_ligne_d_aide(N,'  any unauthorized party for the circulation');
  Ecrire_ligne_d_aide(N,'  or use of this utility is expressly');
  Ecrire_ligne_d_aide(N,'  forbidden.');
  Ecrire_ligne_d_aide(N,'   This means that if you had to pay for');
  Ecrire_ligne_d_aide(N,'  getting GrafX2, you have been swindled.');
                        {XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX}
                        {XXXXXXXXXXXXXXXXXXXXXX}
  Terminer_section_d_aide;
end;
