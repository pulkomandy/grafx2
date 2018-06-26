#!/usr/bin/env python
# vim:expandtab:ts=2 sw=2:
#
# Tool to convert 8bits C sources (with Latin1 chars in litterals)
# to 7bits sources using escape sequences in litterals
#
# Do not convert 8bits chars which are in comments !
#
# Copyright 2018 Thomas Bernard
#
import sys
import os
import re

#conv_format = '\\%03o'  # octal
conv_format = '\\x%02x' # hex

# for C++ style comments //
#commentre = re.compile('(.*)(\/\/.*)', re.MULTILINE | re.DOTALL)
commentre = re.compile('^(.*?)(\/\/.*)$')

def escape_8bit_chars(char):
  if ord(char) >= 0x7f:
    return (conv_format % ord(char))
  else:
    return char

# return True if there was any translation
# and a filename+'.out' file was written
def to_7bit(filename):
  with open(filename, "r") as f:
    has_changed = False
    output = ''
    in_comment = False  # C style comments /*  */
    lineindex = 0
    for line in f.readlines():
      line = line.rstrip()
      lineindex = lineindex + 1
      comment_start = 0
      comment_end = -1
      if not in_comment:
        comment_start = line.find('/*')
        if comment_start >= 0:
          in_comment = True
      if in_comment:
        comment_end = line.find('*/', comment_start)
        if comment_end >= 0:
          in_comment = False
      # skip all lines with C style comments
      if in_comment:
        (line, comment) = ('', line)
      elif comment_start < comment_end:
        # Note : the characters after the end of the comment are not translated
        (line, comment) = (line[0:comment_start], line[comment_start:])
      else:
        m = commentre.match(line)
        if m is not None:
          (line, comment) = m.group(1,2)
        else:
          comment = ''
      convlist = map(escape_8bit_chars, list(line))
      # check for problematic hex strings
      for i in range(len(convlist)-1):
        if convlist[i][0:2] == '\\x':
          c = convlist[i+1]
          if (c >= '0' and c <= '9') or (c >= 'a' and c <= 'f') or (c >= 'A' and c <= 'F'):
            print "WARNING %15s:%-4d %s followed by %s : Inserting \"\"" % (filename, lineindex, convlist[i], c)
            convlist[i] = convlist[i] + '""'
      conv = ''.join(convlist)
      if conv != line:
        has_changed = True
      output = output + conv + comment + '\n'
    if has_changed:
      with open(filename + '.out', "w") as fout:
        fout.write(output)
    return has_changed

# program entry point
if len(sys.argv) <= 1:
  print ('Usage: %s <file.c> ... <file.c>' % sys.argv[0])
  print 'Convert C sources files to 7bit ASCII'
else:
  for i in range(1,len(sys.argv)):
    if to_7bit(sys.argv[i]):
      print ("File %s translated" % sys.argv[i])
      os.rename(sys.argv[i], sys.argv[i]+'.orig')
      os.rename(sys.argv[i]+'.out', sys.argv[i])
