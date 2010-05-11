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

/**> HEADER FILES <**/
#include "WinInput.h"

#ifdef WIN32
#include "String.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

extern bool keyboardfrozen;
extern bool buttons[3];
/********************> IsKeyDown() <*****/
Boolean	IsKeyDown( unsigned char *keyMap, unsigned short theKey )
{
	if(keyboardfrozen)return 0;
	if(theKey< 0x80 /*1000*/){
		static long	keyMapIndex;
		static Boolean	isKeyDown;
		static short	bitToCheck;

		// Calculate the key map index
		keyMapIndex = keyMap[theKey/8];

		// Calculate the individual bit to check
		bitToCheck = theKey%8;

		// Check the status of the key
		isKeyDown = ( keyMapIndex >> bitToCheck ) & 0x01;

		// Return the status of the key
		return isKeyDown;
	}
	else if(theKey==MAC_MOUSEBUTTON1)
		return buttons[0];
	else if(theKey==MAC_MOUSEBUTTON2)
		return buttons[1];

	return 0;
}

unsigned short 	CharToKey(char* which)
{
	// alphabetic keys
	if(!stricmp(which,"a")){
		return MAC_A_KEY;
	}
	if(!stricmp(which,"b")){
		return MAC_B_KEY;
	}
	if(!stricmp(which,"c")){
		return MAC_C_KEY;
	}
	if(!stricmp(which,"d")){
		return MAC_D_KEY;
	}
	if(!stricmp(which,"e")){
		return MAC_E_KEY;
	}
	if(!stricmp(which,"f")){
		return MAC_F_KEY;
	}
	if(!stricmp(which,"g")){
		return MAC_G_KEY;
	}
	if(!stricmp(which,"h")){
		return MAC_H_KEY;
	}
	if(!stricmp(which,"i")){
		return MAC_I_KEY;
	}
	if(!stricmp(which,"j")){
		return MAC_J_KEY;
	}
	if(!stricmp(which,"k")){
		return MAC_K_KEY;
	}
	if(!stricmp(which,"l")){
		return MAC_L_KEY;
	}
	if(!stricmp(which,"m")){
		return MAC_M_KEY;
	}
	if(!stricmp(which,"n")){
		return MAC_N_KEY;
	}
	if(!stricmp(which,"o")){
		return MAC_O_KEY;
	}
	if(!stricmp(which,"p")){
		return MAC_P_KEY;
	}
	if(!stricmp(which,"q")){
		return MAC_Q_KEY;
	}
	if(!stricmp(which,"r")){
		return MAC_R_KEY;
	}
	if(!stricmp(which,"s")){
		return MAC_S_KEY;
	}
	if(!stricmp(which,"t")){
		return MAC_T_KEY;
	}
	if(!stricmp(which,"u")){
		return MAC_U_KEY;
	}
	if(!stricmp(which,"v")){
		return MAC_V_KEY;
	}
	if(!stricmp(which,"w")){
		return MAC_W_KEY;
	}
	if(!stricmp(which,"x")){
		return MAC_X_KEY;
	}
	if(!stricmp(which,"y")){
		return MAC_Y_KEY;
	}
	if(!stricmp(which,"z")){
		return MAC_Z_KEY;
	}

	// keypad keys
	if(!stricmp(which,"KP0")){
		return MAC_NUMPAD_0_KEY;
	}
	if(!stricmp(which,"KP1")){
		return MAC_NUMPAD_1_KEY;
	}
	if(!stricmp(which,"KP2")){
		return MAC_NUMPAD_2_KEY;
	}
	if(!stricmp(which,"KP3")){
		return MAC_NUMPAD_3_KEY;
	}
	if(!stricmp(which,"KP4")){
		return MAC_NUMPAD_4_KEY;
	}
	if(!stricmp(which,"KP5")){
		return MAC_NUMPAD_5_KEY;
	}
	if(!stricmp(which,"KP6")){
		return MAC_NUMPAD_6_KEY;
	}
	if(!stricmp(which,"KP7")){
		return MAC_NUMPAD_7_KEY;
	}
	if(!stricmp(which,"KP8")){
		return MAC_NUMPAD_8_KEY;
	}
	if(!stricmp(which,"KP9")){
		return MAC_NUMPAD_9_KEY;
	}

	// enter
	if(!stricmp(which,"enter")){
		return MAC_ENTER_KEY;
	}

	// number keys
	if(!stricmp(which,"0")){
		return MAC_0_KEY;
	}
	if(!stricmp(which,"1")){
		return MAC_1_KEY;
	}
	if(!stricmp(which,"2")){
		return MAC_2_KEY;
	}
	if(!stricmp(which,"3")){
		return MAC_3_KEY;
	}
	if(!stricmp(which,"4")){
		return MAC_4_KEY;
	}
	if(!stricmp(which,"5")){
		return MAC_5_KEY;
	}
	if(!stricmp(which,"6")){
		return MAC_6_KEY;
	}
	if(!stricmp(which,"7")){
		return MAC_7_KEY;
	}
	if(!stricmp(which,"8")){
		return MAC_8_KEY;
	}
	if(!stricmp(which,"9")){
		return MAC_9_KEY;
	}

	// function keys
	if(!stricmp(which,"F1")){
		return MAC_F1_KEY;
	}
	if(!stricmp(which,"F2")){
		return MAC_F2_KEY;
	}
	if(!stricmp(which,"F3")){
		return MAC_F3_KEY;
	}
	if(!stricmp(which,"F4")){
		return MAC_F4_KEY;
	}
	if(!stricmp(which,"F5")){
		return MAC_F5_KEY;
	}
	if(!stricmp(which,"F6")){
		return MAC_F6_KEY;
	}
	if(!stricmp(which,"F7")){
		return MAC_F7_KEY;
	}
	if(!stricmp(which,"F8")){
		return MAC_F8_KEY;
	}
	if(!stricmp(which,"F9")){
		return MAC_F9_KEY;
	}
	if(!stricmp(which,"F10")){
		return MAC_F10_KEY;
	}
	if(!stricmp(which,"F11")){
		return MAC_F11_KEY;
	}
	if(!stricmp(which,"F12")){
		return MAC_F12_KEY;
	}

	// escape
	if(!stricmp(which,"escape")){
		return MAC_ESCAPE_KEY;
	}
	if(!stricmp(which,"backspace")){
		return MAC_DELETE_KEY;
	}
	if(!stricmp(which,"tab")){
		return MAC_TAB_KEY;
	}
	if(!stricmp(which,"`")){
		return MAC_TILDE_KEY;
	}
	if(!stricmp(which,"caps_lock")){
		return MAC_CAPS_LOCK_KEY;
	}
//	if(which==){
//		return "";
//	}
	if(!stricmp(which,"command")){
		return MAC_COMMAND_KEY;
	}
	if(!stricmp(which,"option")){
		return MAC_OPTION_KEY;
	}
	if(!stricmp(which,"delete")){
		return MAC_DEL_KEY;
	}
	if(!stricmp(which,"insert")){
		return MAC_INSERT_KEY;
	}
	if(!stricmp(which,"home")){
		return MAC_HOME_KEY;
	}
	if(!stricmp(which,"end")){
		return MAC_END_KEY;
	}
	if(!stricmp(which,"page_up")){
		return MAC_PAGE_UP_KEY;
	}
	if(!stricmp(which,"page_down")){
		return MAC_PAGE_DOWN_KEY;
	}
	if(!stricmp(which,"clear")){
		return MAC_NUMPAD_CLEAR_KEY;
	}

	if(!stricmp(which,"control")){
		return MAC_CONTROL_KEY;
	}
	if(!stricmp(which,"return")){
		return MAC_RETURN_KEY;
	}
	if(!stricmp(which,"space")){
		return MAC_SPACE_KEY;
	}
	if(!stricmp(which,"shift")){
		return MAC_SHIFT_KEY;
	}
	if(!stricmp(which,"uparrow")){
		return MAC_ARROW_UP_KEY;
	}
	if(!stricmp(which,"downarrow")){
		return MAC_ARROW_DOWN_KEY;
	}
	if(!stricmp(which,"leftarrow")){
		return MAC_ARROW_LEFT_KEY;
	}
	if(!stricmp(which,"rightarrow")){
		return MAC_ARROW_RIGHT_KEY;
	}
	if(!stricmp(which,"mouse1")){
		return MAC_MOUSEBUTTON1;
	}
	if(!stricmp(which,"mouse2")){
		return MAC_MOUSEBUTTON2;
	}
	if(!stricmp(which,"+")){
		return MAC_NUMPAD_PLUS_KEY;
	}
	if(!stricmp(which,"*")){
		return MAC_NUMPAD_ASTERISK_KEY;
	}
	if(!stricmp(which,"/")){
		return MAC_SLASH_KEY;
	}
	if(!stricmp(which,"\\")){
		return MAC_BACKSLASH_KEY;
	}
	if(!stricmp(which,"[")){
		return MAC_LEFTBRACKET_KEY;
	}
	if(!stricmp(which,"]")){
		return MAC_RIGHTBRACKET_KEY;
	}
	if(!stricmp(which,".")){
		return MAC_PERIOD_KEY;
	}
	if(!stricmp(which,",")){
		return MAC_COMMA_KEY;
	}
	if(!stricmp(which,"\"")){
		return MAC_APOSTROPHE_KEY;
	}
	if(!stricmp(which,";")){
		return MAC_SEMICOLON_KEY;
	}
	return UNKNOWN_KEY;
}

char* 	KeyToChar(unsigned short which)
{
	static int i;

	// alphabetic keys
	if(which==MAC_A_KEY){
		return "a";
	}
	if(which==MAC_B_KEY){
		return "b";
	}
	if(which==MAC_C_KEY){
		return "c";
	}
	if(which==MAC_D_KEY){
		return "d";
	}
	if(which==MAC_E_KEY){
		return "e";
	}
	if(which==MAC_F_KEY){
		return "f";
	}
	if(which==MAC_G_KEY){
		return "g";
	}
	if(which==MAC_H_KEY){
		return "h";
	}
	if(which==MAC_I_KEY){
		return "i";
	}
	if(which==MAC_J_KEY){
		return "j";
	}
	if(which==MAC_K_KEY){
		return "k";
	}
	if(which==MAC_L_KEY){
		return "l";
	}
	if(which==MAC_M_KEY){
		return "m";
	}
	if(which==MAC_N_KEY){
		return "n";
	}
	if(which==MAC_O_KEY){
		return "o";
	}
	if(which==MAC_P_KEY){
		return "p";
	}
	if(which==MAC_Q_KEY){
		return "q";
	}
	if(which==MAC_R_KEY){
		return "r";
	}
	if(which==MAC_S_KEY){
		return "s";
	}
	if(which==MAC_T_KEY){
		return "t";
	}
	if(which==MAC_U_KEY){
		return "u";
	}
	if(which==MAC_V_KEY){
		return "v";
	}
	if(which==MAC_W_KEY){
		return "w";
	}
	if(which==MAC_X_KEY){
		return "x";
	}
	if(which==MAC_Y_KEY){
		return "y";
	}
	if(which==MAC_Z_KEY){
		return "z";
	}

	// keypad keys
	if(which==MAC_NUMPAD_1_KEY){
		return "KP1";
	}
	if(which==MAC_NUMPAD_2_KEY){
		return "KP2";
	}
	if(which==MAC_NUMPAD_3_KEY){
		return "KP3";
	}
	if(which==MAC_NUMPAD_4_KEY){
		return "KP4";
	}
	if(which==MAC_NUMPAD_5_KEY){
		return "KP5";
	}
	if(which==MAC_NUMPAD_6_KEY){
		return "KP6";
	}
	if(which==MAC_NUMPAD_7_KEY){
		return "KP7";
	}
	if(which==MAC_NUMPAD_8_KEY){
		return "KP8";
	}
	if(which==MAC_NUMPAD_9_KEY){
		return "KP9";
	}
	if(which==MAC_NUMPAD_0_KEY){
		return "KP0";
	}

	// enter
	if(which==MAC_ENTER_KEY){
		return "enter";
	}

	// number keys
	if(which==MAC_1_KEY){
		return "1";
	}
	if(which==MAC_2_KEY){
		return "2";
	}
	if(which==MAC_3_KEY){
		return "3";
	}
	if(which==MAC_4_KEY){
		return "4";
	}
	if(which==MAC_5_KEY){
		return "5";
	}
	if(which==MAC_6_KEY){
		return "6";
	}
	if(which==MAC_7_KEY){
		return "7";
	}
	if(which==MAC_8_KEY){
		return "8";
	}
	if(which==MAC_9_KEY){
		return "9";
	}
	if(which==MAC_0_KEY){
		return "0";
	}

	// function keys
	if(which==MAC_F1_KEY){
		return "F1";
	}
	if(which==MAC_F2_KEY){
		return "F2";
	}
	if(which==MAC_F3_KEY){
		return "F3";
	}
	if(which==MAC_F4_KEY){
		return "F4";
	}
	if(which==MAC_F5_KEY){
		return "F5";
	}
	if(which==MAC_F6_KEY){
		return "F6";
	}
	if(which==MAC_F7_KEY){
		return "F7";
	}
	if(which==MAC_F8_KEY){
		return "F8";
	}
	if(which==MAC_F9_KEY){
		return "F9";
	}
	if(which==MAC_F10_KEY){
		return "F10";
	}
	if(which==MAC_F11_KEY){
		return "F11";
	}
	if(which==MAC_F12_KEY){
		return "F12";
	}

	// escape
	if(which==MAC_ESCAPE_KEY){
		return "escape";
	}
	if(which==MAC_DELETE_KEY){
		return "backspace";
	}
	if(which==MAC_TAB_KEY){
		return "tab";
	}
	if(which==MAC_TILDE_KEY){
		return "`";
	}
	if(which==MAC_CAPS_LOCK_KEY){
		return "caps_lock";
	}
	if(which==MAC_COMMAND_KEY){
		return "command";
	}
	if(which==MAC_OPTION_KEY){
		return "option";
	}
	if(which==MAC_DEL_KEY){
		return "delete";
	}
	if(which==MAC_INSERT_KEY){
		return "insert";
	}
	if(which==MAC_HOME_KEY){
		return "home";
	}
	if(which==MAC_END_KEY){
		return "end";
	}
	if(which==MAC_PAGE_UP_KEY){
		return "page_up";
	}
	if(which==MAC_PAGE_DOWN_KEY){
		return "page_down";
	}
	if(which==MAC_NUMPAD_CLEAR_KEY){
		return "clear";
	}
	if(which==MAC_CONTROL_KEY){
		return "control";
	}
	if(which==MAC_SPACE_KEY){
		return "space";
	}
	if(which==MAC_RETURN_KEY){
		return "return";
	}
	if(which==MAC_SHIFT_KEY){
		return "shift";
	}
	if(which==MAC_ARROW_UP_KEY){
		return "uparrow";
	}
	if(which==MAC_ARROW_DOWN_KEY){
		return "downarrow";
	}
	if(which==MAC_ARROW_LEFT_KEY){
		return "leftarrow";
	}
	if(which==MAC_ARROW_RIGHT_KEY){
		return "rightarrow";
	}
	if(which==MAC_MOUSEBUTTON1){
		return "mouse1";
	}
	if(which==MAC_MOUSEBUTTON2){
		return "mouse2";
	}
	if(which==MAC_ARROW_RIGHT_KEY){
		return "rightarrow";
	}
	if(which==MAC_MINUS_KEY||which==MAC_NUMPAD_MINUS_KEY){
		return "-";
	}
	if(which==MAC_PLUS_KEY||which==MAC_NUMPAD_EQUALS_KEY){
		return "=";
	}
	if(which==MAC_NUMPAD_PLUS_KEY){
		return "+";
	}
	if(which==MAC_NUMPAD_ASTERISK_KEY){
		return "*";
	}
	if(which==MAC_SLASH_KEY||which==MAC_NUMPAD_SLASH_KEY){
		return "/";
	}
	if(which==MAC_BACKSLASH_KEY){
		return "\\";
	}
	if(which==MAC_LEFTBRACKET_KEY){
		return "[";
	}
	if(which==MAC_RIGHTBRACKET_KEY){
		return "]";
	}
	if(which==MAC_PERIOD_KEY||which==MAC_NUMPAD_PERIOD_KEY){
		return ".";
	}
	if(which==MAC_COMMA_KEY){
		return ",";
	}
	if(which==MAC_APOSTROPHE_KEY){
		return "\"";
	}
	if(which==MAC_SEMICOLON_KEY){
		return ";";
	}
	return "unknown";
}

char 	KeyToSingleChar(unsigned short which)
{
	static int i;

	if(which==MAC_A_KEY){
		return 'a';
	}
	if(which==MAC_B_KEY){
		return 'b';
	}
	if(which==MAC_C_KEY){
		return 'c';
	}
	if(which==MAC_D_KEY){
		return 'd';
	}
	if(which==MAC_E_KEY){
		return 'e';
	}
	if(which==MAC_F_KEY){
		return 'f';
	}
	if(which==MAC_G_KEY){
		return 'g';
	}
	if(which==MAC_H_KEY){
		return 'h';
	}
	if(which==MAC_I_KEY){
		return 'i';
	}
	if(which==MAC_J_KEY){
		return 'j';
	}
	if(which==MAC_K_KEY){
		return 'k';
	}
	if(which==MAC_L_KEY){
		return 'l';
	}
	if(which==MAC_M_KEY){
		return 'm';
	}
	if(which==MAC_N_KEY){
		return 'n';
	}
	if(which==MAC_O_KEY){
		return 'o';
	}
	if(which==MAC_P_KEY){
		return 'p';
	}
	if(which==MAC_Q_KEY){
		return 'q';
	}
	if(which==MAC_R_KEY){
		return 'r';
	}
	if(which==MAC_S_KEY){
		return 's';
	}
	if(which==MAC_T_KEY){
		return 't';
	}
	if(which==MAC_U_KEY){
		return 'u';
	}
	if(which==MAC_V_KEY){
		return 'v';
	}
	if(which==MAC_W_KEY){
		return 'w';
	}
	if(which==MAC_X_KEY){
		return 'x';
	}
	if(which==MAC_Y_KEY){
		return 'y';
	}
	if(which==MAC_Z_KEY){
		return 'z';
	}
	if(which==MAC_NUMPAD_1_KEY){
		return '1';
	}
	if(which==MAC_NUMPAD_2_KEY){
		return '2';
	}
	if(which==MAC_NUMPAD_3_KEY){
		return '3';
	}
	if(which==MAC_NUMPAD_4_KEY){
		return '4';
	}
	if(which==MAC_NUMPAD_5_KEY){
		return '5';
	}
	if(which==MAC_NUMPAD_6_KEY){
		return '6';
	}
	if(which==MAC_NUMPAD_7_KEY){
		return '7';
	}
	if(which==MAC_NUMPAD_8_KEY){
		return '8';
	}
	if(which==MAC_NUMPAD_9_KEY){
		return '9';
	}
	if(which==MAC_NUMPAD_0_KEY){
		return '0';
	}
	if(which==MAC_1_KEY){
		return '1';
	}
	if(which==MAC_2_KEY){
		return '2';
	}
	if(which==MAC_3_KEY){
		return '3';
	}
	if(which==MAC_4_KEY){
		return '4';
	}
	if(which==MAC_5_KEY){
		return '5';
	}
	if(which==MAC_6_KEY){
		return '6';
	}
	if(which==MAC_7_KEY){
		return '7';
	}
	if(which==MAC_8_KEY){
		return '8';
	}
	if(which==MAC_9_KEY){
		return '9';
	}
	if(which==MAC_0_KEY){
		return '0';
	}
	if(which==MAC_SPACE_KEY){
		return ' ';
	}
	if(which==MAC_MINUS_KEY||which==MAC_NUMPAD_MINUS_KEY){
		return '-';
	}
	if(which==MAC_PLUS_KEY||which==MAC_NUMPAD_EQUALS_KEY){
		return '=';
	}
	if(which==MAC_NUMPAD_PLUS_KEY){
		return '+';
	}
	if(which==MAC_NUMPAD_ASTERISK_KEY){
		return '*';
	}
	if(which==MAC_SLASH_KEY||which==MAC_NUMPAD_SLASH_KEY){
		return '/';
	}
	if(which==MAC_BACKSLASH_KEY){
		return '\\';
	}
	if(which==MAC_LEFTBRACKET_KEY){
		return '[';
	}
	if(which==MAC_RIGHTBRACKET_KEY){
		return ']';
	}
	if(which==MAC_PERIOD_KEY||which==MAC_NUMPAD_PERIOD_KEY){
		return '.';
	}
	if(which==MAC_COMMA_KEY){
		return ',';
	}
	if(which==MAC_APOSTROPHE_KEY){
		return '\'';
	}
	if(which==MAC_SEMICOLON_KEY){
		return ';';
	}
	return '\0';
}

char 	Shift(char which)
{
	static int i;

	if(which=='a'){
		return 'A';
	}
	if(which=='b'){
		return 'B';
	}
	if(which=='c'){
		return 'C';
	}
	if(which=='d'){
		return 'D';
	}
	if(which=='e'){
		return 'E';
	}
	if(which=='f'){
		return 'F';
	}
	if(which=='g'){
		return 'G';
	}
	if(which=='h'){
		return 'H';
	}
	if(which=='e'){
		return 'E';
	}
	if(which=='f'){
		return 'F';
	}
	if(which=='g'){
		return 'G';
	}
	if(which=='h'){
		return 'H';
	}
	if(which=='i'){
		return 'I';
	}
	if(which=='j'){
		return 'J';
	}
	if(which=='k'){
		return 'K';
	}
	if(which=='l'){
		return 'L';
	}
	if(which=='m'){
		return 'M';
	}
	if(which=='n'){
		return 'N';
	}
	if(which=='o'){
		return 'O';
	}
	if(which=='p'){
		return 'P';
	}
	if(which=='q'){
		return 'Q';
	}
	if(which=='r'){
		return 'R';
	}
	if(which=='s'){
		return 'S';
	}
	if(which=='t'){
		return 'T';
	}
	if(which=='u'){
		return 'U';
	}
	if(which=='v'){
		return 'V';
	}
	if(which=='w'){
		return 'W';
	}
	if(which=='x'){
		return 'X';
	}
	if(which=='y'){
		return 'Y';
	}
	if(which=='z'){
		return 'Z';
	}
	if(which=='1'){
		return '!';
	}
	if(which=='2'){
		return '@';
	}
	if(which=='3'){
		return '#';
	}
	if(which=='4'){
		return '$';
	}
	if(which=='5'){
		return '%';
	}
	if(which=='6'){
		return '^';
	}
	if(which=='7'){
		return '&';
	}
	if(which=='8'){
		return '*';
	}
	if(which=='9'){
		return '(';
	}
	if(which=='0'){
		return ')';
	}
	if(which=='-'){
		return '_';
	}
	if(which=='='){
		return '+';
	}
	if(which=='['){
		return '{';
	}
	if(which==']'){
		return '}';
	}
	if(which=='\\'){
		return '|';
	}
	if(which=='.'){
		return '>';
	}
	if(which==','){
		return '<';
	}
	if(which=='/'){
		return '?';
	}
	if(which==';'){
		return ':';
	}
	if(which=='\''){
		return '\"';
	}
	return which;
}

bool 	Compare(char *thestring, char *tocompare, int start, int end)
{
	static int i;
	for(i=start;i<=end;i++){
		if(thestring[i]!=tocompare[i-start]&&thestring[i]!=tocompare[i-start]+'A'-'a')return 0;
	}
	return 1;
}
