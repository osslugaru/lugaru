#include "Settings.h"
#include "Game.h"


void DefaultSettings(Game &game) {
	detail=1;
	ismotionblur=0;
	usermousesensitivity=1;
	kContextWidth=640;
	kContextHeight=480;
	kBitsPerPixel = 32;
	floatjump=0;
	autoslomo=1;
	decals=1;
	invertmouse=0;
	bloodtoggle=0;
	foliage=1;
	musictoggle=1;
	trilinear=1;
	gamespeed=1;
	difficulty=1;
	damageeffects=0;
	texttoggle=1;
	alwaysblur=0;
	showpoints=0;
	showdamagebar=0;
	immediate=0;
	velocityblur=0;
	volume = 0.8f;
	ambientsound=1;
	vblsync=0;
	debugmode=0;
	
	game.crouchkey=MAC_SHIFT_KEY;
	game.jumpkey=MAC_SPACE_KEY;
	game.leftkey=MAC_A_KEY;
	game.forwardkey=MAC_W_KEY;
	game.backkey=MAC_S_KEY;
	game.rightkey=MAC_D_KEY;
	game.drawkey=MAC_E_KEY;
	game.throwkey=MAC_Q_KEY;
	game.attackkey=MAC_MOUSEBUTTON1;
	game.chatkey=MAC_T_KEY;
}

void SaveSettings(Game &game) {
	ofstream opstream(ConvertFileName(":Data:config.txt", "w"));
	opstream << "Screenwidth:\n";
	opstream << game.newscreenwidth;
	opstream << "\nScreenheight:\n";
	opstream << game.newscreenheight;
	opstream << "\nMouse sensitivity:\n";
	opstream << usermousesensitivity;
	opstream << "\nBlur(0,1):\n";
	opstream << ismotionblur;
	opstream << "\nOverall Detail(0,1,2) higher=better:\n";
	opstream << game.newdetail;
	opstream << "\nFloating jump:\n";
	opstream << floatjump;
	opstream << "\nMouse jump:\n";
	opstream << mousejump;
	opstream << "\nAmbient sound:\n";
	opstream << ambientsound;
	opstream << "\nBlood (0,1,2):\n";
	opstream << bloodtoggle;
	opstream << "\nAuto slomo:\n";
	opstream << autoslomo;
	opstream << "\nFoliage:\n";
	opstream << foliage;
	opstream << "\nMusic:\n";
	opstream << musictoggle;
	opstream << "\nTrilinear:\n";
	opstream << trilinear;
	opstream << "\nDecals(shadows,blood puddles,etc):\n";
	opstream << decals;
	opstream << "\nInvert mouse:\n";
	opstream << invertmouse;
	opstream << "\nGamespeed:\n";
	if(oldgamespeed==0)oldgamespeed=1;
	opstream << oldgamespeed;
	opstream << "\nDifficulty(0,1,2) higher=harder:\n";
	opstream << difficulty;
	opstream << "\nDamage effects(blackout, doublevision):\n";
	opstream << damageeffects;
	opstream << "\nText:\n";
	opstream << texttoggle;
	opstream << "\nDebug:\n";
	opstream << debugmode;
	opstream << "\nVBL Sync:\n";
	opstream << vblsync;
	opstream << "\nShow Points:\n";
	opstream << showpoints;
	opstream << "\nAlways Blur:\n";
	opstream << alwaysblur;
	opstream << "\nImmediate mode (turn on on G5):\n";
	opstream << immediate;
	opstream << "\nVelocity blur:\n";
	opstream << velocityblur;
	opstream << "\nVolume:\n";
	opstream << volume;
	opstream << "\nForward key:\n";
	opstream << KeyToChar(game.forwardkey);
	opstream << "\nBack key:\n";
	opstream << KeyToChar(game.backkey);
	opstream << "\nLeft key:\n";
	opstream << KeyToChar(game.leftkey);
	opstream << "\nRight key:\n";
	opstream << KeyToChar(game.rightkey);
	opstream << "\nJump key:\n";
	opstream << KeyToChar(game.jumpkey);
	opstream << "\nCrouch key:\n";
	opstream << KeyToChar(game.crouchkey);
	opstream << "\nDraw key:\n";
	opstream << KeyToChar(game.drawkey);
	opstream << "\nThrow key:\n";
	opstream << KeyToChar(game.throwkey);
	opstream << "\nAttack key:\n";
	opstream << KeyToChar(game.attackkey);
	opstream << "\nChat key:\n";
	opstream << KeyToChar(game.chatkey);
	opstream << "\nDamage bar:\n";
	opstream << showdamagebar;
	opstream.close();
}

bool LoadSettings(Game &game) {
	ifstream ipstream(ConvertFileName(":Data:config.txt"), std::ios::in);
	if ( !ipstream || ipstream.fail() ) {
		printf("Config file not found\n");
		return false;
	}
	char setting[256];
	char string[256];
	
	printf("Loading config\n");
	while(!ipstream.eof()) {
		ipstream.getline( setting, sizeof(setting) );
		
		// skip blank lines
		// assume lines starting with spaces are all blank
		if ( strlen(setting) == 0 || setting[0] == ' ' || setting[0] == '\t') continue;

		if ( ipstream.eof() || ipstream.fail() ) {
			fprintf(stderr, "Error reading config file: Got setting name '%s', but value can't be read\n", setting);
			ipstream.close();			
			return false;
		}
		
	
		if ( !strncmp(setting, "Screenwidth", 11) ) {
			ipstream >> kContextWidth;
		} else if ( !strncmp(setting, "Screenheight", 12) ) {
			ipstream >> kContextHeight;
		} else if ( !strncmp(setting, "Mouse sensitivity", 17) ) {
			ipstream >> usermousesensitivity;
		} else if ( !strncmp(setting, "Blur", 4) ) {
			ipstream >> ismotionblur;
		} else if ( !strncmp(setting, "Overall Detail", 14) ) {
			ipstream >> detail;
			if(detail!=0)kBitsPerPixel=32;
			else kBitsPerPixel=16;
		} else if ( !strncmp(setting, "Floating jump", 13) ) {
			ipstream >> floatjump;
		} else if ( !strncmp(setting, "Mouse jump", 10) ) {
			ipstream >> mousejump;
		} else if ( !strncmp(setting, "Ambient sound", 13) ) {
			ipstream >> ambientsound;
		} else if ( !strncmp(setting, "Blood ", 6) ) {
			ipstream >> bloodtoggle;
		} else if ( !strncmp(setting, "Auto slomo", 10) ) {
			ipstream >> autoslomo;
		} else if ( !strncmp(setting, "Foliage", 7) ) {
			ipstream >> foliage;
		} else if ( !strncmp(setting, "Music", 5) ) {
			ipstream >> musictoggle;
		} else if ( !strncmp(setting, "Trilinear", 9) ) {
			ipstream >> trilinear;
		} else if ( !strncmp(setting, "Decals", 6) ) {
			ipstream >> decals;
		} else if ( !strncmp(setting, "Invert mouse", 12) ) {
			ipstream >> invertmouse;
		} else if ( !strncmp(setting, "Gamespeed", 9) ) {
			ipstream >> gamespeed;
			oldgamespeed=gamespeed;
			if(oldgamespeed==0){
				gamespeed=1;
				oldgamespeed=1;
			}
		} else if ( !strncmp(setting, "Difficulty", 10) ) {
			ipstream >> difficulty;
		} else if ( !strncmp(setting, "Damage effects", 14) ) {
			ipstream >> damageeffects;
		} else if ( !strncmp(setting, "Text", 4) ) {
			ipstream >> texttoggle;
		} else if ( !strncmp(setting, "Debug", 5) ) {
			ipstream >> debugmode;
		} else if ( !strncmp(setting, "VBL Sync", 8) ) {
			ipstream >> vblsync;
		} else if ( !strncmp(setting, "Show Points", 11) ) {
			ipstream >> showpoints;
		} else if ( !strncmp(setting, "Always Blur", 11) ) {
			ipstream >> alwaysblur;
		} else if ( !strncmp(setting, "Immediate mode ", 15) ) {
			ipstream >> immediate;
		} else if ( !strncmp(setting, "Velocity blur", 13) ) {
			ipstream >> velocityblur;
		} else if ( !strncmp(setting, "Volume", 6) ) {
			ipstream >> volume;
		} else if ( !strncmp(setting, "Forward key", 11) ) {
			ipstream >> string;
			game.forwardkey = CharToKey(string);
		} else if ( !strncmp(setting, "Back key", 8) ) {
			ipstream >> string;
			game.backkey = CharToKey(string);
		} else if ( !strncmp(setting, "Left key", 8) ) {
			ipstream >> string;
			game.leftkey = CharToKey(string);
		} else if ( !strncmp(setting, "Right key", 9) ) {
			ipstream >> string;
			game.rightkey = CharToKey(string);
		} else if ( !strncmp(setting, "Jump key", 8) ) {
			ipstream >> string;
			game.jumpkey = CharToKey(string);
		} else if ( !strncmp(setting, "Crouch key", 10) ) {
			ipstream >> string;
			game.crouchkey = CharToKey(string);
		} else if ( !strncmp(setting, "Draw key", 8) ) {
			ipstream >> string;
			game.drawkey = CharToKey(string);
		} else if ( !strncmp(setting, "Throw key", 9) ) {
			ipstream >> string;
			game.throwkey = CharToKey(string);
		} else if ( !strncmp(setting, "Attack key", 10) ) {
			ipstream >> string;
			game.attackkey = CharToKey(string);
		} else if ( !strncmp(setting, "Chat key", 8) ) {
			ipstream >> string;
			game.chatkey = CharToKey(string);
		} else if ( !strncmp(setting, "Damage bar", 10) ) {
			ipstream >> showdamagebar;
		} else {
			ipstream >> string;
			fprintf(stderr, "Unknown config option '%s' with value '%s'. Ignoring.\n", setting, string);
		}
		
		if ( ipstream.fail() ) {
			fprintf(stderr, "Error reading config file: EOF reached when trying to read value for setting '%s'.\n", setting);
			ipstream.close();			
			return false;
		}
		
		if ( ipstream.bad() ) {
			fprintf(stderr, "Error reading config file: Failed to read value for setting '%s'.\n", setting);
			ipstream.close();
			return false;
		}
	}
	
	ipstream.close();
	
	if(detail>2)detail=2;
	if(detail<0)detail=0;
	if(screenwidth<0)screenwidth=640;
	if(screenheight<0)screenheight=480;
	
	return true;
}
