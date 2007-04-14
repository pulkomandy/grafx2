
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
