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
#include "SDL.h"

#ifdef WIN32
#include "String.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

extern bool keyboardfrozen;
/********************> IsKeyDown() <*****/

unsigned short 	CharToKey(const char* which)
{
	for(unsigned short i=0;i<SDLK_LAST;i++) {
		if(!strcasecmp(which,SDL_GetKeyName(SDLKey(i))))
			return i;
	}
	if(!strcasecmp(which,"mouse1")){
		return MOUSEBUTTON1;
	}
	if(!strcasecmp(which,"mouse2")){
		return MOUSEBUTTON2;
	}
	return SDLK_LAST;
}

char 	KeyToSingleChar(unsigned short which)
{
	static int i;

	if(which==SDLK_a){
		return 'a';
	}
	if(which==SDLK_b){
		return 'b';
	}
	if(which==SDLK_c){
		return 'c';
	}
	if(which==SDLK_d){
		return 'd';
	}
	if(which==SDLK_e){
		return 'e';
	}
	if(which==SDLK_f){
		return 'f';
	}
	if(which==SDLK_g){
		return 'g';
	}
	if(which==SDLK_h){
		return 'h';
	}
	if(which==SDLK_i){
		return 'i';
	}
	if(which==SDLK_j){
		return 'j';
	}
	if(which==SDLK_k){
		return 'k';
	}
	if(which==SDLK_m){
		return 'm';
	}
	if(which==SDLK_n){
		return 'n';
	}
	if(which==SDLK_o){
		return 'o';
	}
	if(which==SDLK_p){
		return 'p';
	}
	if(which==SDLK_q){
		return 'q';
	}
	if(which==SDLK_s){
		return 's';
	}
	if(which==SDLK_u){
		return 'u';
	}
	if(which==SDLK_v){
		return 'v';
	}
	if(which==SDLK_x){
		return 'x';
	}
	if(which==SDLK_z){
		return 'z';
	}
	if(which==SDLK_KP1){
		return '1';
	}
	if(which==SDLK_KP2){
		return '2';
	}
	if(which==SDLK_KP3){
		return '3';
	}
	if(which==SDLK_KP4){
		return '4';
	}
	if(which==SDLK_KP5){
		return '5';
	}
	if(which==SDLK_KP6){
		return '6';
	}
	if(which==SDLK_KP7){
		return '7';
	}
	if(which==SDLK_KP8){
		return '8';
	}
	if(which==SDLK_KP9){
		return '9';
	}
	if(which==SDLK_KP0){
		return '0';
	}
	if(which==SDLK_1){
		return '1';
	}
	if(which==SDLK_2){
		return '2';
	}
	if(which==SDLK_3){
		return '3';
	}
	if(which==SDLK_4){
		return '4';
	}
	if(which==SDLK_5){
		return '5';
	}
	if(which==SDLK_6){
		return '6';
	}
	if(which==SDLK_7){
		return '7';
	}
	if(which==SDLK_8){
		return '8';
	}
	if(which==SDLK_9){
		return '9';
	}
	if(which==SDLK_0){
		return '0';
	}
	if(which==SDLK_SPACE){
		return ' ';
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
