/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _MACINPUT_H_
#define _MACINPUT_H_

/**> HEADER FILES <**/
#include <stdlib.h>
#include <stdio.h>
#include <CursorDevices.h> //Mouse

/**> CONSTANT DECLARATIONS <**/
// Mac Keyboard Codes
#define	MAC_1_KEY				0x12
#define	MAC_2_KEY				0x13
#define	MAC_3_KEY				0x14
#define	MAC_4_KEY				0x15
#define	MAC_5_KEY				0x17
#define	MAC_6_KEY				0x16
#define	MAC_7_KEY				0x1A
#define	MAC_8_KEY				0x1C
#define	MAC_9_KEY				0x19
#define	MAC_0_KEY				0x1D
#define	MAC_NUMPAD_1_KEY		0x53
#define	MAC_NUMPAD_2_KEY		0x54
#define	MAC_NUMPAD_3_KEY		0x55
#define	MAC_NUMPAD_4_KEY		0x56
#define	MAC_NUMPAD_5_KEY		0x57
#define	MAC_NUMPAD_6_KEY		0x58
#define	MAC_NUMPAD_7_KEY		0x59
#define	MAC_NUMPAD_8_KEY		0x5B
#define	MAC_NUMPAD_9_KEY		0x5C
#define	MAC_NUMPAD_0_KEY		0x52
#define	MAC_A_KEY				0x00
#define	MAC_B_KEY				0x0B
#define	MAC_C_KEY				0x08
#define	MAC_D_KEY				0x02
#define	MAC_E_KEY				0x0E
#define	MAC_F_KEY				0x03
#define	MAC_G_KEY				0x05
#define	MAC_H_KEY				0x04
#define	MAC_I_KEY				0x22
#define	MAC_J_KEY				0x26
#define	MAC_K_KEY				0x28
#define	MAC_L_KEY				0x25
#define	MAC_M_KEY				0x2E
#define	MAC_N_KEY				0x2D
#define	MAC_O_KEY				0x1F
#define	MAC_P_KEY				0x23
#define	MAC_Q_KEY				0x0C
#define	MAC_R_KEY				0x0F
#define	MAC_S_KEY				0x01
#define	MAC_T_KEY				0x11
#define	MAC_U_KEY				0x20
#define	MAC_V_KEY				0x09
#define	MAC_W_KEY				0x0D
#define	MAC_X_KEY				0x07
#define	MAC_Y_KEY				0x10
#define	MAC_Z_KEY				0x06
#define	MAC_F1_KEY				0x7A
#define	MAC_F2_KEY				0x78
#define	MAC_F3_KEY				0x63
#define	MAC_F4_KEY				0x76
#define	MAC_F5_KEY				0x60
#define	MAC_F6_KEY				0x61
#define	MAC_F7_KEY				0x62
#define	MAC_F8_KEY				0x64
#define	MAC_F9_KEY				0x65
#define	MAC_F10_KEY				0x6D
#define	MAC_F11_KEY				0x67
#define	MAC_F12_KEY				0x6F
#define	MAC_RETURN_KEY			0x24
#define	MAC_ENTER_KEY			0x4C
#define	MAC_TAB_KEY				0x30
#define	MAC_SPACE_KEY			0x31
#define	MAC_DELETE_KEY			0x33
#define	MAC_ESCAPE_KEY			0x35
#define	MAC_COMMAND_KEY			0x37
#define	MAC_SHIFT_KEY			0x38
#define	MAC_CAPS_LOCK_KEY		0x39
#define	MAC_OPTION_KEY			0x3A
#define	MAC_CONTROL_KEY			0x3B
#define	MAC_PAGE_UP_KEY			0x74
#define	MAC_PAGE_DOWN_KEY		0x79
#define	MAC_INSERT_KEY			0x72
#define	MAC_DEL_KEY				0x75
#define	MAC_HOME_KEY			0x73
#define	MAC_END_KEY				0x77
#define	MAC_LEFT_BRACKET_KEY	0x21
#define	MAC_RIGHT_BRACKET_KEY	0x1E
#define	MAC_ARROW_UP_KEY		0x7E
#define	MAC_ARROW_DOWN_KEY		0x7D
#define	MAC_ARROW_LEFT_KEY		0x7B
#define	MAC_ARROW_RIGHT_KEY		0x7C
#define	MAC_TILDE_KEY			0x32
#define MAC_MINUS_KEY			0x1B
#define MAC_PLUS_KEY			0x18
#define MAC_SLASH_KEY			0x2C
#define MAC_PERIOD_KEY			0x2F
#define MAC_COMMA_KEY			0x2B
#define MAC_BACKSLASH_KEY		0x2A
#define MAC_LEFTBRACKET_KEY		0x21
#define MAC_RIGHTBRACKET_KEY	0x1E
#define MAC_NUMPAD_CLEAR_KEY	0x47#define MAC_NUMPAD_MINUS_KEY	0x4E
#define MAC_NUMPAD_EQUALS_KEY	0x51
#define MAC_NUMPAD_PLUS_KEY		0x45
#define MAC_NUMPAD_SLASH_KEY	0x4B
#define MAC_NUMPAD_ASTERISK_KEY	0x43
#define MAC_NUMPAD_ENTER_KEY	0x4C
#define MAC_NUMPAD_PERIOD_KEY	0x41
#define MAC_SEMICOLON_KEY		0x29
#define MAC_APOSTROPHE_KEY		0x27

/**> FUNCTION PROTOTYPES <**/
Boolean	IsKeyDown( unsigned char *keyMap, unsigned short theKey );
void 	InitMouse();
void 	MoveMouse(int xcoord, int ycoord, Point *mouseloc);
void 	RefreshMouse(Point *mouseloc);
void 	DisposeMouse();
unsigned short 	CharToKey(char* which);
char* 	KeyToChar(unsigned short which);
char 	KeyToSingleChar(unsigned short which);
char 	Shift(char which);
bool 	Compare(char *thestring, char *tocompare, int start, int end);

#endif
