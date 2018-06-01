#!/usr/bin/env python
# (c) 2018 Thomas Bernard

#import sys

filename = '../src/keycodes.h'

keys = ['UNKNOWN',
        'ESCAPE', 'RETURN', 'BACKSPACE', 'TAB',
        'UP', 'DOWN', 'LEFT', 'RIGHT',
        'LEFTBRACKET', 'RIGHTBRACKET',
        'INSERT', 'DELETE', 'COMMA', 'BACKQUOTE',
        'PAGEUP', 'PAGEDOWN', 'HOME', 'END',
        'KP_PLUS', 'KP_MINUS', 'KP_MULTIPLY', 'KP_ENTER',
        'KP_DIVIDE', 'KP_PERIOD', 'KP_EQUALS',
        'EQUALS', 'MINUS', 'PERIOD',
        'CAPSLOCK', 'CLEAR', 'SPACE', 'PAUSE',
        'LSHIFT', 'RSHIFT', 'LCTRL', 'RCTRL',
        'LALT', 'RALT']

def keycode_def(section, key, index, sdl_key=None):
	if section == 'SDL and SDL2':
		if sdl_key is None:
			sdl_key = key
		return '#define KEY_%-12s K2K(SDLK_%s)\n' % (key, sdl_key)
	else:
		return '#define KEY_%-12s %d\n' % (key, index)

def add_keycodes_defs(section, lines):
	global keys
	i = 0
	for key in keys:
		lines.append(keycode_def(section, key, i))
		i = i + 1
	for j in range(10):
		lines.append(keycode_def(section, chr(ord('0') + j), i))
		i = i + 1
	for j in range(26):
		lines.append(keycode_def(section, chr(ord('a') + j), i))
		i = i + 1
	if section == 'SDL and SDL2':
		lines.append('#if defined(USE_SDL)\n')
	for j in range(10):
		key = "KP%d" % (j)
		lines.append(keycode_def(section, key, i))
		i = i + 1
	lines.append(keycode_def(section, 'SCROLLOCK', i))
	i = i + 1
	if section == 'SDL and SDL2':
		lines.append('#else\n')
		for j in range(10):
			key = "KP%d" % (j)
			sdl_key = 'KP_%d' % (j)
			lines.append(keycode_def(section, key, 0, sdl_key))
		lines.append(keycode_def(section, 'SCROLLOCK', i, 'SCROLLLOCK'))
		lines.append('#endif\n')
	for j in range(1,13):
		key = "F%d" % (j)
		lines.append(keycode_def(section, key, i))
		i = i + 1

def update_keycodes(filename):
	output = []
	with open(filename) as f:
		skipping = False
		for line in f:
			if skipping:
				if line.startswith('// end of KEY definitions'):
					output.append(line)
					skipping = False
			else:
				output.append(line)
				if line.startswith('// KEY definitions for '):
					section = line[23:].strip()
					print 'section "%s"' % (section)
					skipping = True
					add_keycodes_defs(section, output)
	with open(filename, "w") as f:
		for line in output:
			f.write(line)

update_keycodes(filename)
