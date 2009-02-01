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
void Demarrer_pile_operation(word Operation_demandee);
void Operation_PUSH(short Valeur);
void Operation_POP(short * Valeur);

//////////////////////////////////////////////////// OPERATION_DESSIN_CONTINU
void Freehand_Mode1_1_0(void);
void Freehand_Mode1_1_2(void);
void Freehand_Mode12_0_2(void);
void Freehand_Mode1_2_0(void);
void Freehand_Mode1_2_2(void);

///////////////////////////////////////////////// OPERATION_DESSIN_DISCONTINU
void Freehand_Mode2_1_0(void);
void Freehand_Mode2_1_2(void);
void Freehand_Mode2_2_0(void);
void Freehand_Mode2_2_2(void);

////////////////////////////////////////////////////// OPERATION_DESSIN_POINT
void Freehand_Mode3_1_0(void);
void Freehand_Mode3_2_0(void);
void Freehand_Mode3_0_1(void);

///////////////////////////////////////////////////////////// OPERATION_LIGNE

void Ligne_12_0(void);
void Ligne_12_5(void);
void Ligne_0_5(void);

///////////////////////////////////////////////////////////// OPERATION_LOUPE

void Loupe_12_0(void);

/////////////////////////////////////////////////// OPERATION_RECTANGLE_?????

void Rectangle_12_0(void);
void Rectangle_12_5(void);
void Rectangle_vide_0_5(void);
void Rectangle_plein_0_5(void);

////////////////////////////////////////////////////// OPERATION_CERCLE_?????

void Cercle_12_0(void);
void Cercle_12_5(void);
void Cercle_vide_0_5(void);
void Cercle_plein_0_5(void);

///////////////////////////////////////////////////// OPERATION_ELLIPSE_?????

void Ellipse_12_0(void);
void Ellipse_12_5(void);
void Ellipse_vide_0_5(void);
void Ellipse_pleine_0_5(void);

////////////////////////////////////////////////////// OPERATION_PRISE_BROSSE

void Brosse_12_0(void);
void Brosse_12_5(void);
void Brosse_0_5(void);

///////////////////////////////////////////////////// OPERATION_ETIRER_BROSSE

void Etirer_brosse_12_0(void);
void Etirer_brosse_1_7(void);
void Etirer_brosse_0_7(void);
void Etirer_brosse_2_7(void);

//////////////////////////////////////////////////// OPERATION_TOURNER_BROSSE

void Tourner_brosse_12_0(void);
void Tourner_brosse_1_5(void);
void Tourner_brosse_0_5(void);
void Tourner_brosse_2_5(void);

//////////////////////////////////////////////////////// OPERATION_POLYBROSSE

void Polybrosse_12_8(void);

////////////////////////////////////////////////////////////// OPERATION_FILL

void Fill_1_0(void);
void Fill_2_0(void);

///////////////////////////////////////////////////////// OPERATION_REMPLACER

void Remplacer_1_0(void);
void Remplacer_2_0(void);

/////////////////////////////////////////////////////////// OPERATION_PIPETTE

void Pipette_0_0(void);
void Pipette_12_0(void);
void Pipette_1_1(void);
void Pipette_2_1(void);
void Pipette_0_1(void);

/////////////////////////////////////////////////////////// OPERATION_K_LIGNE

void K_Ligne_12_0(void);
void K_Ligne_12_6(void);
void K_Ligne_0_6(void);
void K_Ligne_12_7(void);

/////////////////////////////////////////////////// OPERATION_COURBE_?_POINTS

void Courbe_34_points_1_0(void);
void Courbe_34_points_2_0(void);
void Courbe_34_points_1_5(void);
void Courbe_34_points_2_5(void);

void Courbe_4_points_0_5(void);
void Courbe_4_points_1_9(void);
void Courbe_4_points_2_9(void);

void Courbe_3_points_0_5(void);
void Courbe_3_points_0_11(void);
void Courbe_3_points_12_11(void);

///////////////////////////////////////////////////////////// OPERATION_SPRAY

void Spray_1_0(void);
void Spray_2_0(void);
void Spray_12_2(void);
void Spray_0_2(void);

//////////////////////////////////////////////////////////// OPERATION_*POLY*

void Polygone_12_0(void);
void Polygone_12_9(void);

void Polyfill_12_0(void);
void Polyfill_0_8(void);
void Polyfill_12_8(void);
void Polyfill_12_9(void);

void Polyform_12_0(void);
void Polyform_12_8(void);
void Polyform_0_8(void);

void Filled_polyform_12_0(void);
void Filled_polyform_12_8(void);
void Filled_polyform_0_8(void);

//////////////////////////////////////////////////////////// OPERATION_SCROLL

void Scroll_12_0(void);
void Scroll_12_4(void);
void Scroll_0_4(void);

//////////////////////////////////////////////////// OPERATION_CERCLE_DEGRADE

void Cercle_degrade_12_0(void);
void Cercle_degrade_12_6(void);
void Cercle_degrade_0_6(void);
void Cercle_degrade_12_8(void);
void Cercle_ou_ellipse_degrade_0_8(void);

////////////////////////////////////////////////// OPERATION_ELLIPSE_DEGRADEE

void Ellipse_degradee_12_0(void);
void Ellipse_degradee_12_6(void);
void Ellipse_degradee_0_6(void);
void Ellipse_degradee_12_8(void);

///////////////////////////////////////////////// OPERATION_RECTANGLE_DEGRADE

void Rectangle_Degrade_12_0(void);
void Rectangle_Degrade_12_5(void);
void Rectangle_Degrade_0_5(void);
void Rectangle_Degrade_0_7(void);
void Rectangle_Degrade_12_7(void);
void Rectangle_Degrade_12_9(void);
void Rectangle_Degrade_0_9(void);

/////////////////////////////////////////////////// OPERATION_LIGNES_CENTREES

void Lignes_centrees_12_0(void);
void Lignes_centrees_12_3(void);
void Lignes_centrees_0_3(void);
void Lignes_centrees_12_7(void);
void Lignes_centrees_0_7(void);

