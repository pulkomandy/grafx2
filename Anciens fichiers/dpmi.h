/*  Grafx2 - The Ultimate 256-color bitmap paint program

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

#ifndef _DPMI_H_
#define _DPMI_H_



  word Physical_address_mapping(byte * Physical_address,dword Size,byte * * Linear_address_pointer);
  word Free_physical_address_mapping(byte * Linear_address);
  word Lock_linear_region(byte * Linear_address,dword Size);
  word Unlock_linear_region(byte * Linear_address,dword Size);
  word Allocate_ldt_descriptor(word Nombre_de_descripteurs,word * Base_selector_pointer);
  word Free_ldt_descriptor(word Selector);
  word Set_segment_base_address(word Selector,byte * Linear_base_address);
  word Set_segment_limit(word Selector,dword Segment_limit);
  word Set_descriptor_access_rights(word Selector,word Rights);
  word Get_segment_base_address(word Selector,byte * * Linear_base_address_pointer);



#endif
