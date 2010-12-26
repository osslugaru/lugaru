/*
Copyright (C) 2010 - Lugaru authors

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

#include "Awards.h"
#include "Person.h"
#include "Game.h"

int bonus;
float bonusvalue;
float bonustotal;
float startbonustotal;
float bonustime;

// FIXME: make these per-player
float damagetaken;
int numfalls;
int numflipfail;
int numseen;
int numresponded;
int numstaffattack;
int numswordattack;
int numknifeattack;
int numunarmedattack;
int numescaped;
int numflipped;
int numwallflipped;
int numthrowkill;
int numafterkill;
int numreversals;
int numattacks;
int maxalarmed;

int award_awards(int *awards)
{
  int numawards = 0, i;
  if(damagetaken==0&&player[0].bloodloss==0){
    awards[numawards]=awardflawless;
    numawards++;
  }
  bool alldead = true;
  for(i=1;i<numplayers;i++){
    if(player[i].dead!=2)alldead=0;
  }
  if(alldead){
    awards[numawards]=awardalldead;
    numawards++;
  }
  alldead=1;
  for(i=1;i<numplayers;i++){
    if(player[i].dead!=1)alldead=0;
  }
  if(alldead){
    awards[numawards]=awardnodead;
    numawards++;
  }
  if(numresponded==0&&!numthrowkill){
    awards[numawards]=awardstealth;
    numawards++;
  }
  if(numattacks==numstaffattack&&numattacks>0){
    awards[numawards]=awardbojutsu;
    numawards++;
  }
  if(numattacks==numswordattack&&numattacks>0){
    awards[numawards]=awardswordsman;
    numawards++;
  }
  if(numattacks==numknifeattack&&numattacks>0){
    awards[numawards]=awardknifefighter;
    numawards++;
  }
  if(numattacks==numunarmedattack&&numthrowkill==0&&weapons.numweapons>0){
    awards[numawards]=awardkungfu;
    numawards++;
  }
  if(numescaped>0){
    awards[numawards]=awardevasion;
    numawards++;
  }
  if(numflipfail==0&&numflipped+numwallflipped*2>20){
    awards[numawards]=awardacrobat;
    numawards++;
  }
  if(numthrowkill==numplayers-1){
    awards[numawards]=awardlongrange;
    numawards++;
  }
  alldead=1;
  for(i=1;i<numplayers;i++){
    if(player[i].dead!=2)alldead=0;
  }
  if(numafterkill>0&&alldead){
    awards[numawards]=awardbrutal;
    numawards++;
  }
  if(numreversals>((float)numattacks)*.8&&numreversals>3){
    awards[numawards]=awardaikido;
    numawards++;
  }
  if(maxalarmed==1&&numplayers>2){
    awards[numawards]=awardstrategy;
    numawards++;
  }
  if(numflipfail>3){
    awards[numawards]=awardklutz;
    numawards++;
  }
  return numawards;
}
