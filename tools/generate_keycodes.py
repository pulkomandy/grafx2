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
        'LALT', 'RALT', 'MENU']

win32vk = {
'BACKSPACE': 'BACK',
'PAGEUP': 'PRIOR',
'PAGEDOWN': 'NEXT',
'CAPSLOCK': 'CAPITAL',
'EQUALS': 'OEM_PLUS',  # =+ key on US keyboards
'COMMA': 'OEM_COMMA',
'MINUS': 'OEM_MINUS',
'PERIOD': 'OEM_PERIOD',
'BACKQUOTE': 'OEM_3',
'LEFTBRACKET': 'OEM_4',
'RIGHTBRACKET': 'OEM_6',
'KP_PLUS': 'ADD',
'KP_MULTIPLY': 'MULTIPLY',
'KP_MINUS': 'SUBTRACT',
'KP_DIVIDE': 'DIVIDE',
'KP_PERIOD': 'DECIMAL',
'KP_ENTER': 'RETURN',
'LCTRL': 'LCONTROL',
'RCTRL': 'RCONTROL',
'LALT': 'LMENU',
'RALT': 'RMENU',
'MENU': 'APPS',
}

x11xk = {
'BACKSPACE': 'BackSpace',
'CAPSLOCK': 'Caps_Lock',
'PAGEUP': 'Page_Up',
'PAGEDOWN': 'Page_Down',
'BACKQUOTE': 'grave',
'KP_PLUS': 'KP_Add',
'KP_MINUS': 'KP_Subtract',
'KP_EQUALS': 'KP_Equal',
'KP_PERIOD': 'KP_Decimal',
'COMMA': 'comma',
'SPACE': 'space',
'EQUALS': 'equal',
'MINUS': 'minus',
'PERIOD': 'period',
'LEFTBRACKET': 'bracketleft',
'RIGHTBRACKET': 'bracketright',
'LCTRL': 'Control_L',
'RCTRL': 'Control_R',
'LALT': 'Alt_L',
'RALT': 'Alt_R',
'LSHIFT': 'Shift_L',
'RSHIFT': 'Shift_R',
}

def keycode_def(section, key, index, native_key=None):
	if native_key is None:
		native_key = key
	if section == 'SDL and SDL2':
		return '#define KEY_%-12s K2K(SDLK_%s)\n' % (key, native_key)
	elif section == 'win32':
		if len(key) == 1 and key >= '0' and key <= '9':
			return '#define KEY_%-12s 0x%02x\n' % (key, ord(key))
		elif len(key) == 1 and key >= 'a' and key <= 'z':
			return '#define KEY_%-12s 0x%02x\n' % (key, ord(key)-32)
		elif key == 'UNKNOWN' or key == 'KP_EQUALS':
			return '#define KEY_%-12s 0\n' % (key)
		else:
			return '#define KEY_%-12s VK_%s\n' % (key, native_key)
	elif section == 'x11':
		if native_key[0:3] == 'Kp_':
			native_key = 'KP_' + native_key[3:]
		if key == 'UNKNOWN':
			return '#define KEY_%-12s 0\n' % (key)
		else:
			return '#define KEY_%-12s K2K(XK_%s)\n' % (key, native_key)
	else:
		return '#define KEY_%-12s %d\n' % (key, index)

def add_keycodes_defs(section, lines):
	global keys
	i = 0
	for key in keys:
		if section == 'win32' and key in win32vk:
			lines.append(keycode_def(section, key, i, win32vk[key]))
		elif section == 'x11':
			if key in x11xk:
				lines.append(keycode_def(section, key, i, x11xk[key]))
			else:
				lines.append(keycode_def(section, key, i, key.title()))
		else:
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
		if section == 'win32':
			lines.append(keycode_def(section, key, i, "NUMPAD%d" % (j)))
		elif section == 'x11':
			lines.append(keycode_def(section, key, i, "KP_%d" % (j)))
		else:
			lines.append(keycode_def(section, key, i))
		i = i + 1
	if section == 'win32':
		lines.append(keycode_def(section, 'SCROLLOCK', i, 'SCROLL'))
	elif section == 'x11':
		lines.append(keycode_def(section, 'SCROLLOCK', i, 'Scroll_Lock'))
	else:
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
