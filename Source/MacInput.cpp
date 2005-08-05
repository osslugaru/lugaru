/**> HEADER FILES <**/
#include "MacInput.h"
#include "String.h"

extern bool keyboardfrozen;

/********************> IsKeyDown() <*****/
Boolean	IsKeyDown( unsigned char *keyMap, unsigned short theKey )
{
	if(keyboardfrozen)return 0;

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

unsigned short 	CharToKey(char* which)
{
	if(!strcmp(which,"a")){
		return MAC_A_KEY;
	}
	if(!strcmp(which,"b")){
		return MAC_B_KEY;
	}
	if(!strcmp(which,"c")){
		return MAC_C_KEY;
	}
	if(!strcmp(which,"d")){
		return MAC_D_KEY;
	}
	if(!strcmp(which,"e")){
		return MAC_E_KEY;
	}
	if(!strcmp(which,"f")){
		return MAC_F_KEY;
	}
	if(!strcmp(which,"g")){
		return MAC_G_KEY;
	}
	if(!strcmp(which,"h")){
		return MAC_H_KEY;
	}
	if(!strcmp(which,"i")){
		return MAC_I_KEY;
	}
	if(!strcmp(which,"j")){
		return MAC_J_KEY;
	}
	if(!strcmp(which,"k")){
		return MAC_K_KEY;
	}
	if(!strcmp(which,"l")){
		return MAC_L_KEY;
	}
	if(!strcmp(which,"m")){
		return MAC_M_KEY;
	}
	if(!strcmp(which,"n")){
		return MAC_N_KEY;
	}
	if(!strcmp(which,"o")){
		return MAC_O_KEY;
	}
	if(!strcmp(which,"p")){
		return MAC_P_KEY;
	}
	if(!strcmp(which,"q")){
		return MAC_Q_KEY;
	}
	if(!strcmp(which,"r")){
		return MAC_R_KEY;
	}
	if(!strcmp(which,"s")){
		return MAC_S_KEY;
	}
	if(!strcmp(which,"t")){
		return MAC_T_KEY;
	}
	if(!strcmp(which,"u")){
		return MAC_U_KEY;
	}
	if(!strcmp(which,"v")){
		return MAC_V_KEY;
	}
	if(!strcmp(which,"w")){
		return MAC_W_KEY;
	}
	if(!strcmp(which,"x")){
		return MAC_X_KEY;
	}
	if(!strcmp(which,"y")){
		return MAC_Y_KEY;
	}
	if(!strcmp(which,"z")){
		return MAC_Z_KEY;
	}
	if(!strcmp(which,"0")){
		return MAC_NUMPAD_0_KEY;
	}
	if(!strcmp(which,"1")){
		return MAC_NUMPAD_1_KEY;
	}
	if(!strcmp(which,"2")){
		return MAC_NUMPAD_2_KEY;
	}
	if(!strcmp(which,"3")){
		return MAC_NUMPAD_3_KEY;
	}
	if(!strcmp(which,"4")){
		return MAC_NUMPAD_4_KEY;
	}
	if(!strcmp(which,"5")){
		return MAC_NUMPAD_5_KEY;
	}
	if(!strcmp(which,"6")){
		return MAC_NUMPAD_6_KEY;
	}
	if(!strcmp(which,"7")){
		return MAC_NUMPAD_7_KEY;
	}
	if(!strcmp(which,"8")){
		return MAC_NUMPAD_8_KEY;
	}
	if(!strcmp(which,"9")){
		return MAC_NUMPAD_9_KEY;
	}
	if(!strcmp(which,"enter")){
		return MAC_ENTER_KEY;
	}
	if(!strcmp(which,"control")){
		return MAC_CONTROL_KEY;
	}
	if(!strcmp(which,"return")){
		return MAC_RETURN_KEY;
	}
	if(!strcmp(which,"space")){
		return MAC_SPACE_KEY;
	}
	if(!strcmp(which,"shift")){
		return MAC_SHIFT_KEY;
	}
	if(!strcmp(which,"uparrow")){
		return MAC_ARROW_UP_KEY;
	}
	if(!strcmp(which,"downarrow")){
		return MAC_ARROW_DOWN_KEY;
	}
	if(!strcmp(which,"leftarrow")){
		return MAC_ARROW_LEFT_KEY;
	}
	if(!strcmp(which,"rightarrow")){
		return MAC_ARROW_RIGHT_KEY;
	}
}

char* 	KeyToChar(unsigned short which)
{
	static int i;
	
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
	if(which==MAC_NUMPAD_1_KEY){
		return "1";
	}
	if(which==MAC_NUMPAD_2_KEY){
		return "2";
	}
	if(which==MAC_NUMPAD_3_KEY){
		return "3";
	}
	if(which==MAC_NUMPAD_4_KEY){
		return "4";
	}
	if(which==MAC_NUMPAD_5_KEY){
		return "5";
	}
	if(which==MAC_NUMPAD_6_KEY){
		return "6";
	}
	if(which==MAC_NUMPAD_7_KEY){
		return "7";
	}
	if(which==MAC_NUMPAD_8_KEY){
		return "8";
	}
	if(which==MAC_NUMPAD_9_KEY){
		return "9";
	}
	if(which==MAC_ENTER_KEY){
		return "enter";
	}
	if(which==MAC_NUMPAD_0_KEY){
		return "0";
	}
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
		return "caps lock";
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
		return "page up";
	}
	if(which==MAC_PAGE_DOWN_KEY){
		return "page down";
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