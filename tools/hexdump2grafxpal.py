#!/usr/bin/python
# -*- coding: latin-1 -*-
import os,sys
def hexvl(a_st):
  a_st=a_st.lower();tmpr=0;hx_st="0123456789abcdef";hx_st=hx_st.lower()
  for i in range(0,len(a_st),1):
    tmpr=(tmpr*16)+hx_st.find(a_st[i])
  return tmpr
finp_st=sys.argv[1];fout_st=finp_st+".pal"
if finp_st.lower()=="--help".lower():
  print "hexdump2grafxpal - 201001261202 - Paulo Silva (GPL licence)"
  print "converts a text hexdump file into a .pal file used on Grafx2"
  print "usage: python hexdump2grafxpal.py yourfile.hex"
  print "the result will be a neighbour file named yourfile.hex.pal"
  print "bug: the result may not have 768 bytes, which may not be"
  print "loadable by Grafx2, if so please fill remaining '0x00' bytes"
  print "with a hex editor like KHexEdit or any other."
else:
  finp_fl=open(finp_st,"r");fout_fl=open(fout_st,"w");ctr=0
  while True:
    text_st=finp_fl.readline()
    if len(text_st)==0:break
    text_st=text_st.replace("\n","")
    while len(text_st)>0:
      bytecrop=hexvl(text_st[:2])
      bc2=int(bytecrop/4)
      fout_fl.write(chr(bc2));ctr+=1
      text_st=text_st[2:]
  for j in range(ctr,768,1):
    fout_fl.write(chr(0))
  finp_fl.close();fout_fl.close()

