#include "Settings.h"
#include "Game.h"
#include "Input.h"

using namespace Game;

void DefaultSettings()
{
    detail = 1;
    ismotionblur = 0;
    usermousesensitivity = 1;
    newscreenwidth = kContextWidth = 640;
    newscreenheight = kContextHeight = 480;
    kBitsPerPixel = 32;
    floatjump = 0;
    autoslomo = 1;
    decals = 1;
    invertmouse = 0;
    bloodtoggle = 0;
    foliage = 1;
    musictoggle = 1;
    trilinear = 1;
    gamespeed = 1;
    difficulty = 1;
    damageeffects = 0;
    texttoggle = 1;
    alwaysblur = 0;
    showpoints = 0;
    showdamagebar = 0;
    immediate = 0;
    velocityblur = 0;
    volume = 0.8f;
    ambientsound = 1;
    vblsync = 1;
    debugmode = 0;

    crouchkey = SDLK_LSHIFT;
    jumpkey = SDLK_SPACE;
    leftkey = SDLK_a;
    forwardkey = SDLK_w;
    backkey = SDLK_s;
    rightkey = SDLK_d;
    drawkey = SDLK_e;
    throwkey = SDLK_q;
    attackkey = MOUSEBUTTON1;
    consolekey = SDLK_BACKQUOTE;
    chatkey = SDLK_t;
}

void SaveSettings()
{
    if (newdetail < 0)
        newdetail = 0;
    if (newdetail > 2)
        newdetail = 2;
    if (newscreenwidth > 3000)
        newscreenwidth = screenwidth;
    if (newscreenwidth < 0)
        newscreenwidth = screenwidth;
    if (newscreenheight > 3000)
        newscreenheight = screenheight;
    if (newscreenheight < 0)
        newscreenheight = screenheight;
    ofstream opstream(ConvertFileName(":Data:config.txt", "w"));
    opstream << "Screenwidth:\n";
    opstream << newscreenwidth;
    opstream << "\nScreenheight:\n";
    opstream << newscreenheight;
    opstream << "\nMouse sensitivity:\n";
    opstream << usermousesensitivity;
    opstream << "\nBlur(0,1):\n";
    opstream << ismotionblur;
    opstream << "\nOverall Detail(0,1,2) higher=better:\n";
    opstream << newdetail;
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
    if (oldgamespeed == 0)
        oldgamespeed = 1;
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
    opstream << Input::keyToChar(forwardkey);
    opstream << "\nBack key:\n";
    opstream << Input::keyToChar(backkey);
    opstream << "\nLeft key:\n";
    opstream << Input::keyToChar(leftkey);
    opstream << "\nRight key:\n";
    opstream << Input::keyToChar(rightkey);
    opstream << "\nJump key:\n";
    opstream << Input::keyToChar(jumpkey);
    opstream << "\nCrouch key:\n";
    opstream << Input::keyToChar(crouchkey);
    opstream << "\nDraw key:\n";
    opstream << Input::keyToChar(drawkey);
    opstream << "\nThrow key:\n";
    opstream << Input::keyToChar(throwkey);
    opstream << "\nAttack key:\n";
    opstream << Input::keyToChar(attackkey);
    opstream << "\nConsole key:\n";
    opstream << Input::keyToChar(consolekey);
    opstream << "\nChat key:\n";
    opstream << Input::keyToChar(chatkey);
    opstream << "\nDamage bar:\n";
    opstream << showdamagebar;
    opstream << "\nStereoMode:\n";
    opstream << stereomode;
    opstream << "\nStereoSeparation:\n";
    opstream << stereoseparation;
    opstream << "\nStereoReverse:\n";
    opstream << stereoreverse;
    opstream.close();
}

bool LoadSettings()
{
    ifstream ipstream(ConvertFileName(":Data:config.txt"), std::ios::in);
    if ( !ipstream || ipstream.fail() ) {
        printf("Config file not found\n");
        return false;
    }
    char setting[256];
    char string[256];

    printf("Loading config\n");
    while (!ipstream.eof()) {
        ipstream.getline( setting, sizeof(setting) );

        // skip blank lines
        // assume lines starting with spaces are all blank
        if ( strlen(setting) == 0 || setting[0] == ' ' || setting[0] == '\t')
            continue;
        //~ printf("setting : %s\n",setting);

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
            if (detail != 0)
                kBitsPerPixel = 32;
            else
                kBitsPerPixel = 16;
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
            oldgamespeed = gamespeed;
            if (oldgamespeed == 0) {
                gamespeed = 1;
                oldgamespeed = 1;
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
            ipstream.getline( string, sizeof(string) );
            forwardkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Back key", 8) ) {
            ipstream.getline( string, sizeof(string) );
            backkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Left key", 8) ) {
            ipstream.getline( string, sizeof(string) );
            leftkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Right key", 9) ) {
            ipstream.getline( string, sizeof(string) );
            rightkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Jump key", 8) ) {
            ipstream.getline( string, sizeof(string) );
            jumpkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Crouch key", 10) ) {
            ipstream.getline( string, sizeof(string) );
            crouchkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Draw key", 8) ) {
            ipstream.getline( string, sizeof(string) );
            drawkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Throw key", 9) ) {
            ipstream.getline( string, sizeof(string) );
            throwkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Attack key", 10) ) {
            ipstream.getline( string, sizeof(string) );
            attackkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Console key", 11) ) {
            ipstream.getline( string, sizeof(string) );
            consolekey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Chat key", 8) ) {
            ipstream.getline( string, sizeof(string) );
            chatkey = Input::CharToKey(string);
        } else if ( !strncmp(setting, "Damage bar", 10) ) {
            ipstream >> showdamagebar;
        } else if ( !strncmp(setting, "StereoMode", 10) ) {
            int i;
            ipstream >> i;
            stereomode = (StereoMode)i;
        } else if ( !strncmp(setting, "StereoSeparation", 16) ) {
            ipstream >> stereoseparation;
        } else if ( !strncmp(setting, "StereoReverse", 13) ) {
            ipstream >> stereoreverse;
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

    if (detail > 2)
        detail = 2;
    if (detail < 0)
        detail = 0;
    if (screenwidth < 0)
        screenwidth = 640;
    if (screenheight < 0)
        screenheight = 480;

    return true;
}
