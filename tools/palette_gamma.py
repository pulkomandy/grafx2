#!/usr/bin/env python
# vim:expandtab:ts=2 sw=2:
#
# show the 8bit values of 16 color palette with gamma correction
# Used to build Thomson palettes
#
# (c) 2018 Thomas BERNARD

import sys

def palette_values(n, gamma):
  return [int(round(255.0*pow(i/(n-1.0), 1.0/gamma))) for i in range (0,n)]

def array_to_c(a):
  return '{ ' + ', '.join(a) + '};'

for i in range (0,22):
  gamma = 1.0 + i / 10.0
  p = palette_values(16, gamma)
  print gamma, array_to_c(map(str, p))
  print '   ', array_to_c(map(hex, p))
