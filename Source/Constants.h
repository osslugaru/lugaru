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

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define awardklutz 0
#define awardflawless 1
#define awardalldead 2
#define awardnodead 3
#define awardstealth 4
#define awardswordsman 5
#define awardkungfu 6
#define awardknifefighter 7
#define awardcoward 8
#define awardevasion 9
#define awardacrobat 10
#define awardlongrange 11
#define awardbrutal 12
#define awardhyper 13
#define awardaikido 14
#define awardrambo 15
#define awardfast 16
#define awardrealfast 17
#define awarddamnfast 18
#define awardstrategy 19
#define awardbojutsu 20

#define mapkilleveryone 0
#define mapgosomewhere 1
#define mapkillsomeone 2
#define mapkillmost 3

#define wpkeepwalking 0
#define wppause 1

#define typeactive 0
#define typesitting 1
#define typesittingwall 2
#define typesleeping 3
#define typedead1 4
#define typedead2 5
#define typedead3 6
#define typedead4 7

#define tracheotomy 1
#define backstab 2
#define spinecrusher 3
#define ninja 4
#define style 5
#define cannon 6
#define aimbonus 7
#define deepimpact 8
#define touchofdeath 9
#define swordreversebonus 10
#define staffreversebonus 11
#define reverseko 12
#define solidhit 13
#define twoxcombo 14
#define threexcombo 15
#define fourxcombo 16
#define megacombo 17
#define Reversal 18
#define Stabbonus 19
#define Slicebonus 20
#define Bullseyebonus 21
#define Slashbonus 22
#define Wolfbonus 23
#define FinishedBonus 24
#define TackleBonus 25
#define AboveBonus 26

#define boneconnect 0
#define constraint 1
#define muscle 2

const int head = 0; // Do not conflict with 'head' var in zlib 

#define neck 1
#define leftshoulder 2
#define leftelbow 3
#define leftwrist 4
#define lefthand 5
#define rightshoulder 6
#define rightelbow 7
#define rightwrist 8
#define righthand 9
#define abdomen 10
#define lefthip 11
#define righthip 12
#define groin 13
#define leftknee 14
#define leftankle 15
#define leftfoot 16
#define rightknee 17
#define rightankle 18
#define rightfoot 19

#define max_joints 50
#define max_frames 50
#define max_muscles 100

#define animation_count 140

#define runanim 0
#define bounceidleanim 1
#define stopanim 2
#define jumpupanim 3
#define jumpdownanim 4
#define landanim 5
#define climbanim 6
#define hanganim 7
#define spinkickanim 8
#define tempanim 9
#define getupfromfrontanim 10
#define getupfrombackanim 11
#define crouchanim 12
#define sneakanim 13
#define rollanim 14
#define flipanim 15
#define spinkickreversedanim 16
#define spinkickreversalanim 17
#define lowkickanim 18
#define sweepanim 19
#define sweepreversedanim 20
#define sweepreversalanim 21
#define rabbitkickanim 22
#define rabbitkickreversedanim 23
#define rabbitkickreversalanim 24
#define upunchanim 25
#define staggerbackhighanim 26
#define upunchreversedanim 27
#define upunchreversalanim 28
#define hurtidleanim 29
#define backhandspringanim 30
#define fightidleanim 31
#define walkanim 32
#define fightsidestep 33
#define killanim 34
#define sneakattackanim 35
#define sneakattackedanim 36
#define drawrightanim 37
#define knifeslashstartanim 38
#define crouchstabanim 39
#define crouchdrawrightanim 40
#define knifefollowanim 41
#define knifefollowedanim 42
#define knifethrowanim 43
#define removeknifeanim 44
#define crouchremoveknifeanim 45
#define jumpreversedanim 46
#define jumpreversalanim 47
#define landhardanim 48
#define staggerbackhardanim 49
#define dropkickanim 50
#define winduppunchanim 51
#define winduppunchblockedanim 52
#define blockhighleftanim 53
#define blockhighleftstrikeanim 54
#define walljumpfrontanim 55
#define walljumpbackanim 56
#define walljumpleftanim 57
#define walljumprightanim 58
#define backflipanim 59
#define leftflipanim 60
#define rightflipanim 61
#define walljumprightkickanim 62
#define walljumpleftkickanim 63
#define knifefightidleanim 64
#define knifesneakattackanim 65
#define knifesneakattackedanim 66
#define swordstabanim 67
#define swordslashleftanim 68
#define swordslashrightanim 69
#define swordfightidleanim 70
#define swordsneakattackanim 71
#define swordsneakattackedanim 72
#define drawleftanim 73
#define swordslashanim 74
#define swordgroundstabanim 75
#define dodgebackanim 76
#define swordslashreversedanim 77
#define swordslashreversalanim 78
#define knifeslashreversedanim 79
#define knifeslashreversalanim 80
#define swordfightidlebothanim 81
#define swordslashparryanim 82
#define sworddisarmanim 83
#define swordslashparriedanim 84
#define wolfidle 85
#define wolffightidle 86    
#define wolfswordidle 87
#define wolfhurtidle 88
#define wolfcrouchanim 89
#define wolfsneakanim 90
#define wolfrunanim 91
#define wolfstopanim 92
#define wolfclawanim 93
#define wolflandanim 94
#define wolflandhardanim 95
#define wolfrunninganim 96
#define rabbitrunninganim 97
#define frontflipanim 98
#define rabbittackleanim 99
#define rabbittacklinganim 100
#define rabbittackledfrontanim 101
#define rabbittackledbackanim 102
#define rabbittacklereversal 103
#define rabbittacklereversed 104
#define wolftackleanim 105
#define wolftacklinganim 106
#define wolftackledfrontanim 107
#define wolftackledbacanim 108
#define wolftacklereversal 109
#define wolftacklereversed 110
#define wolfslapanim 111
#define wolfbashanim 112
#define staffhitanim 113
#define staffgroundsmashanim 114
#define staffspinhitanim 115
#define staffhitreversedanim 116
#define staffhitreversalanim 117
#define staffspinhitreversedanim 118
#define staffspinhitreversalanim 119
#define sleepanim 120
#define sitanim 121
#define talkidleanim 122
#define sitwallanim 123
#define dead1anim 124
#define dead2anim 125
#define dead3anim 126
#define dead4anim 127

#define max_dialogues 20
#define max_dialoguelength 20

#define max_model_vertex			3000		// maximum number of vertexs
#define max_textured_triangle		3000		// maximum number of texture-filled triangles in a model

#define stream_music1desert 0
#define stream_music1grass 1
#define stream_music1snow 2
#define stream_music2 3
#define stream_music3 4
#define stream_music4 5
#define stream_menumusic 6
#define stream_desertambient 7
#define stream_firesound 8
#define stream_wind 9

//#define music1desert 0
//#define music1grass 1
//#define music1snow 2
//#define music2 3
//#define music3 4
//#define music4 5
//#define menumusic 6
//#define desertambient 7
//#define firesound 8
//#define wind 9
#define footstepsound 10
#define footstepsound2 11
#define footstepsound3 12
#define footstepsound4 13
#define jumpsound 14
#define landsound 15
#define whooshsound 16
#define hawksound 17
#define landsound1 18
#define landsound2 19
#define breaksound 20
#define lowwhooshsound 21
#define heavyimpactsound 22
#define firestartsound 23
#define fireendsound 24
#define breaksound2 25
#define knifedrawsound 26
#define knifesheathesound 27
#define knifeswishsound 28
#define knifeslicesound 29
#define skidsound 30
#define snowskidsound 31
#define bushrustle 32
#define midwhooshsound 33
#define highwhooshsound 34
#define movewhooshsound 35
#define thudsound 36
#define whooshhitsound 37
#define clank1sound 38
#define clank2sound 39
#define clank3sound 40
#define clank4sound 41
#define consolefailsound 42
#define consolesuccesssound 43
#define swordslicesound 44
#define metalhitsound 45
#define clawslicesound 46
#define splattersound 47
#define growlsound 48
#define growl2sound 49
#define barksound 50
#define snarlsound 51
#define snarl2sound 52
#define barkgrowlsound 53
#define bark2sound 54
#define bark3sound 55
#define rabbitattacksound 56
#define rabbitattack2sound 57
#define rabbitattack3sound 58
#define rabbitattack4sound 59
#define rabbitpainsound 60
#define rabbitpain1sound 61
#define rabbitpain2sound 62
#define rabbitchitter 63
#define rabbitchitter2 64
#define fleshstabsound 65
#define fleshstabremovesound 66
#define swordstaffsound 67
#define staffbodysound 68
#define staffheadsound 69
#define alarmsound 70
#define staffbreaksound 71

#define normalmode 0
#define motionblurmode 1
#define radialzoommode 2
#define realmotionblurmode 3
#define doublevisionmode 4
#define glowmode 5

#define maxplayers 10
#endif
