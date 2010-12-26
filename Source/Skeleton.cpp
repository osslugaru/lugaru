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
#include "Game.h"
#include "Skeleton.h"
#include "openal_wrapper.h"
#include "Animation.h"

extern float multiplier;
extern float gravity;
extern Skeleton testskeleton;
extern Terrain terrain;
extern int channels[100];
extern Objects objects;
extern int environment;
extern float terraindetail;
extern float camerashake;
extern bool freeze;
extern int detail;
extern XYZ envsound[30];
extern float envsoundvol[30];
extern int numenvsounds;
extern float envsoundlife[30];
extern int tutoriallevel;

extern int whichjointstartarray[26];
extern int whichjointendarray[26];

extern Game * pgame;
extern bool visibleloading;
extern "C"	void PlaySoundEx(int channel, OPENAL_SAMPLE *sptr, OPENAL_DSPUNIT *dsp, signed char startpaused);

void dealloc2(void* param){
	free(param);
	param=0;
}

void Muscle::DoConstraint(bool spinny)
{
	static XYZ vel;
	static XYZ midp;
	static XYZ newpoint1,newpoint2;

	static float oldlength;
	static float relaxlength;

	oldlength=length;

	if(type!=boneconnect)relaxlength=findDistance(&parent1->position,&parent2->position);

	if(type==boneconnect)strength=1;
	if(type==constraint)strength=0;

	if(strength<0)strength=0;
	if(strength>1)strength=1;

	length-=(length-relaxlength)*(1-strength)*multiplier*10000;
	length-=(length-targetlength)*(strength)*multiplier*10000;
	if(strength==0)length=relaxlength;

	if((relaxlength-length>0&&relaxlength-oldlength<0)||(relaxlength-length<0&&relaxlength-oldlength>0))length=relaxlength;

	//if(!broken){
	if(length<minlength)length=minlength;
	if(length>maxlength)length=maxlength;
	//}
	/*
	if(broken){
	if(length<minlength*.6)length=minlength*.6;
	if(length>maxlength*1.4)length=maxlength*1.4;
	}
	*/
	if(length==relaxlength)return;

	//Find midpoint
	midp=(parent1->position*parent1->mass+parent2->position*parent2->mass)/(parent1->mass+parent2->mass);
	//Find vector from midpoint to second vector
	vel=parent2->position-midp;
	//Change to unit vector
	Normalise(&vel);
	//Apply velocity change
	newpoint1=midp-vel*length*(parent2->mass/(parent1->mass+parent2->mass));
	newpoint2=midp+vel*length*(parent1->mass/(parent1->mass+parent2->mass));
	if(!freeze&&spinny){
		parent1->velocity=parent1->velocity+(newpoint1-parent1->position)/multiplier/4;
		parent2->velocity=parent2->velocity+(newpoint2-parent2->position)/multiplier/4;
	}
	else
	{
		parent1->velocity=parent1->velocity+(newpoint1-parent1->position);
		parent2->velocity=parent2->velocity+(newpoint2-parent2->position);
	}
	//Move child point to within certain distance of parent point
	parent1->position=newpoint1;
	parent2->position=newpoint2;
}

void Skeleton::FindForwardsfirst()
{
	//Find forward vectors
	CrossProduct(joints[forwardjoints[1]].position-joints[forwardjoints[0]].position,joints[forwardjoints[2]].position-joints[forwardjoints[0]].position,&forward);
	Normalise(&forward);

	CrossProduct(joints[lowforwardjoints[1]].position-joints[lowforwardjoints[0]].position,joints[lowforwardjoints[2]].position-joints[lowforwardjoints[0]].position,&lowforward);
	Normalise(&lowforward);

	//Special forwards
	specialforward[0]=forward;
	specialforward[1]=forward;
	specialforward[2]=forward;
	specialforward[3]=forward;
	specialforward[4]=forward;

}
void Skeleton::FindForwards()
{
	//Find forward vectors
	CrossProduct(joints[forwardjoints[1]].position-joints[forwardjoints[0]].position,joints[forwardjoints[2]].position-joints[forwardjoints[0]].position,&forward);
	Normalise(&forward);

	CrossProduct(joints[lowforwardjoints[1]].position-joints[lowforwardjoints[0]].position,joints[lowforwardjoints[2]].position-joints[lowforwardjoints[0]].position,&lowforward);
	Normalise(&lowforward);

	//Special forwards
	specialforward[0]=forward;

	specialforward[1]=joints[jointlabels[rightshoulder]].position+joints[jointlabels[rightwrist]].position;
	specialforward[1]=joints[jointlabels[rightelbow]].position-specialforward[1]/2;
	specialforward[1]+=forward*.4;
	Normalise(&specialforward[1]);
	specialforward[2]=joints[jointlabels[leftshoulder]].position+joints[jointlabels[leftwrist]].position;
	specialforward[2]=joints[jointlabels[leftelbow]].position-specialforward[2]/2;
	specialforward[2]+=forward*.4;
	Normalise(&specialforward[2]);

	specialforward[3]=joints[jointlabels[righthip]].position+joints[jointlabels[rightankle]].position;
	specialforward[3]=specialforward[3]/2-joints[jointlabels[rightknee]].position;
	specialforward[3]+=lowforward*.4;
	Normalise(&specialforward[3]);
	specialforward[4]=joints[jointlabels[lefthip]].position+joints[jointlabels[leftankle]].position;
	specialforward[4]=specialforward[4]/2-joints[jointlabels[leftknee]].position;
	specialforward[4]+=lowforward*.4;
	Normalise(&specialforward[4]);
}

float Skeleton::DoConstraints(XYZ *coords,float *scale)
{
	static float friction=1.5;
	static float elasticity=.3;
	static XYZ bounceness;
	static XYZ oldpos[100];
	static int numrepeats=3;
	static float groundlevel=.15;
	static float soundvolume;
	static int i,j,k,l,m;
	static XYZ temp,start,end;
	static XYZ terrainnormal;
	static float r=.05;
	static float r2=.08;
	static int whichhit;
	//static int whichjointstart,whichjointend;
	static float distance;
	static float frictionness;
	static XYZ terrainlight;
	static int whichpatchx;
	static int whichpatchz;
	static float damage;
	static bool freely;
	static float tempmult;
	static bool breaking;
	breaking=0;

	damage=0;

	if(free){
		freetime+=multiplier;

		whichpatchx=coords->x/(terrain.size/subdivision*terrain.scale*terraindetail);
		whichpatchz=coords->z/(terrain.size/subdivision*terrain.scale*terraindetail);

		terrainlight=*coords;
		objects.SphereCheckPossible(&terrainlight, 1);
		/*
		for(i=0; i<num_joints; i++){
		oldpos[i]=joints[i].position;
		}*/

		//Add velocity
		for(i=0; i<num_joints; i++){
			//if(!isnormal(joints[i].velocity.x)||!isnormal(joints[i].velocity.y)||!isnormal(joints[i].velocity.z))joints[i].velocity=0;
			joints[i].position=joints[i].position+joints[i].velocity*multiplier;
			groundlevel=.15;
			if(joints[i].label==head)groundlevel=.8;
			if(joints[i].label==righthand||joints[i].label==rightwrist||joints[i].label==rightelbow)groundlevel=.2;
			if(joints[i].label==lefthand||joints[i].label==leftwrist||joints[i].label==leftelbow)groundlevel=.2;
			joints[i].position.y-=groundlevel;
			//if(!joints[i].locked&&!broken)joints[i].velocity+=joints[i].velchange*multiplier*10*(500-longdead)/500;
			joints[i].oldvelocity=joints[i].velocity;
		}
		tempmult=multiplier;
		//multiplier/=numrepeats;
		for(j=0; j<numrepeats; j++){
			if(!joints[jointlabels[rightknee]].locked&&!joints[jointlabels[righthip]].locked){
				temp=joints[jointlabels[rightknee]].position-(joints[jointlabels[righthip]].position+joints[jointlabels[rightankle]].position)/2;
				while(normaldotproduct(temp,lowforward)>-.1&&!sphere_line_intersection(&joints[jointlabels[righthip]].position,&joints[jointlabels[rightankle]].position,&joints[jointlabels[rightknee]].position,&r)){
					joints[jointlabels[rightknee]].position-=lowforward*.05;
					if(spinny)joints[jointlabels[rightknee]].velocity-=lowforward*.05/multiplier/4;
					else joints[jointlabels[rightknee]].velocity-=lowforward*.05;
					joints[jointlabels[rightankle]].position+=lowforward*.025;
					if(spinny)joints[jointlabels[rightankle]].velocity+=lowforward*.025/multiplier/4;
					else joints[jointlabels[rightankle]].velocity+=lowforward*.25;
					joints[jointlabels[righthip]].position+=lowforward*.025;
					if(spinny)joints[jointlabels[righthip]].velocity+=lowforward*.025/multiplier/4;
					else joints[jointlabels[righthip]].velocity+=lowforward*.025;
					temp=joints[jointlabels[rightknee]].position-(joints[jointlabels[righthip]].position+joints[jointlabels[rightankle]].position)/2;
				}
			}
			if(!joints[jointlabels[leftknee]].locked&&!joints[jointlabels[righthip]].locked){
				temp=joints[jointlabels[leftknee]].position-(joints[jointlabels[lefthip]].position+joints[jointlabels[leftankle]].position)/2;
				while(normaldotproduct(temp,lowforward)>-.1&&!sphere_line_intersection(&joints[jointlabels[lefthip]].position,&joints[jointlabels[leftankle]].position,&joints[jointlabels[leftknee]].position,&r)){
					joints[jointlabels[leftknee]].position-=lowforward*.05;
					if(spinny)joints[jointlabels[leftknee]].velocity-=lowforward*.05/multiplier/4;
					else joints[jointlabels[leftknee]].velocity-=lowforward*.05;
					joints[jointlabels[leftankle]].position+=lowforward*.025;
					if(spinny)joints[jointlabels[leftankle]].velocity+=lowforward*.025/multiplier/4;
					else joints[jointlabels[leftankle]].velocity+=lowforward*.25;
					joints[jointlabels[lefthip]].position+=lowforward*.025;
					if(spinny)joints[jointlabels[lefthip]].velocity+=lowforward*.025/multiplier/4;
					else joints[jointlabels[lefthip]].velocity+=lowforward*.025;
					temp=joints[jointlabels[leftknee]].position-(joints[jointlabels[lefthip]].position+joints[jointlabels[leftankle]].position)/2;
				}
			}
			/*
			if(terrain.patchobjectnum[whichpatchx][whichpatchz])
			for(m=0;m<terrain.patchobjectnum[whichpatchx][whichpatchz];m++){
			k=terrain.patchobjects[whichpatchx][whichpatchz][m];
			if(k<objects.numobjects&&k>=0)
			if(objects.possible[k]){
			temp=joints[jointlabels[head]].position*(*scale)+*coords;
			if(objects.model[k].SphereCheck(&temp, 0.06, &start, &objects.position[k], &objects.rotation[k])!=-1){
			//temp=(joints[jointlabels[head]].position*(*scale)+*coords)-start;
			//Normalise(&temp);
			//joints[jointlabels[head]].position=((temp*.2+start)-*coords)/(*scale);
			joints[jointlabels[head]].position=(temp-*coords)/(*scale);
			}
			}
			}	*/


			//Ears check
			/*XYZ startheadpos;
			startheadpos=joints[jointlabels[head]].position;
			XYZ headpos;
			headpos=joints[jointlabels[head]].position+(joints[jointlabels[head]].position-joints[jointlabels[neck]].position);
			if(terrain.patchobjectnum[whichpatchx][whichpatchz])
			for(m=0;m<terrain.patchobjectnum[whichpatchx][whichpatchz];m++){
			k=terrain.patchobjects[whichpatchx][whichpatchz][m];
			if(k<objects.numobjects&&k>=0)
			if(objects.possible[k]){
			friction=objects.friction[k];
			start=joints[jointlabels[head]].position*(*scale)+*coords;
			end=(headpos)*(*scale)+*coords;
			whichhit=objects.model[k].LineCheckPossible(&start,&end,&temp,&objects.position[k],&objects.rotation[k]);
			if(whichhit!=-1){
			if(joints[jointlabels[head]].label==groin&&!joints[jointlabels[head]].locked&&joints[jointlabels[head]].delay<=0){
			joints[jointlabels[head]].locked=1;
			joints[jointlabels[head]].delay=1;
			static float gLoc[3];
			static float vel[3];
			gLoc[0]=headpos.x*(*scale)+coords->x;
			gLoc[1]=headpos.y*(*scale)+coords->y;
			gLoc[2]=headpos.z*(*scale)+coords->z;
			vel[0]=joints[jointlabels[head]].velocity.x;
			vel[1]=joints[jointlabels[head]].velocity.y;
			vel[2]=joints[jointlabels[head]].velocity.z;
			PlaySoundEx( landsound1, samp[landsound1], NULL, true);
			OPENAL_3D_SetAttributes(channels[landsound1], gLoc, vel);
			OPENAL_SetVolume(channels[landsound1], 128);
			OPENAL_SetPaused(channels[landsound1], false);

			breaking=1;
			}

			if(joints[jointlabels[head]].label==head&&!joints[jointlabels[head]].locked&&joints[jointlabels[head]].delay<=0){
			joints[jointlabels[head]].locked=1;
			joints[jointlabels[head]].delay=1;
			static float gLoc[3];
			static float vel[3];
			gLoc[0]=headpos.x*(*scale)+coords->x;
			gLoc[1]=headpos.y*(*scale)+coords->y;
			gLoc[2]=headpos.z*(*scale)+coords->z;
			vel[0]=joints[jointlabels[head]].velocity.x;
			vel[1]=joints[jointlabels[head]].velocity.y;
			vel[2]=joints[jointlabels[head]].velocity.z;
			PlaySoundEx( landsound2, samp[landsound2], NULL, true);
			OPENAL_3D_SetAttributes(channels[landsound2], gLoc, vel);
			OPENAL_SetVolume(channels[landsound2], 128);
			OPENAL_SetPaused(channels[landsound2], false);
			}

			terrainnormal=DoRotation(objects.model[k].facenormals[whichhit],0,objects.rotation[k],0)*-1;
			if(terrainnormal.y>.8)freefall=0;
			bounceness=terrainnormal*findLength(&joints[jointlabels[head]].velocity)*(abs(normaldotproduct(joints[jointlabels[head]].velocity,terrainnormal)));
			if(findLengthfast(&joints[jointlabels[head]].velocity)>findLengthfast(&joints[jointlabels[head]].oldvelocity)){
			bounceness=0;
			joints[jointlabels[head]].velocity=joints[jointlabels[head]].oldvelocity;
			}
			if(findLengthfast(&bounceness)>4000&&breaking){
			objects.model[k].MakeDecal(breakdecal,DoRotation(temp-objects.position[k],0,-objects.rotation[k],0),.4,.5,Random()%360);
			Sprite::MakeSprite(cloudsprite, headpos*(*scale)+*coords,joints[jointlabels[head]].velocity*.06, 1,1,1, 4, .2);
			breaking=0;
			camerashake+=.6;

			static float gLoc[3];
			static float vel[3];
			gLoc[0]=headpos.x*(*scale)+coords->x;
			gLoc[1]=headpos.y*(*scale)+coords->y;
			gLoc[2]=headpos.z*(*scale)+coords->z;
			vel[0]=joints[jointlabels[head]].velocity.x;
			vel[1]=joints[jointlabels[head]].velocity.y;
			vel[2]=joints[jointlabels[head]].velocity.z;
			PlaySoundEx( breaksound2, samp[breaksound2], NULL, true);
			OPENAL_3D_SetAttributes(channels[breaksound2], gLoc, vel);
			OPENAL_SetVolume(channels[breaksound2], 300);
			OPENAL_SetPaused(channels[breaksound2], false);

			envsound[numenvsounds]=*coords;
			envsoundvol[numenvsounds]=64;
			envsoundlife[numenvsounds]=.4;
			numenvsounds++;
			}
			if(objects.type[k]==treetrunktype){
			objects.rotx[k]+=joints[jointlabels[head]].velocity.x*multiplier*.4;
			objects.roty[k]+=joints[jointlabels[head]].velocity.z*multiplier*.4;
			objects.rotx[k+1]+=joints[jointlabels[head]].velocity.x*multiplier*.4;
			objects.roty[k+1]+=joints[jointlabels[head]].velocity.z*multiplier*.4;
			}
			if(!joints[jointlabels[head]].locked)damage+=findLengthfast(&bounceness)/2500;
			ReflectVector(&joints[jointlabels[head]].velocity,&terrainnormal);
			frictionness=abs(normaldotproduct(joints[jointlabels[head]].velocity,terrainnormal));//findLength(&bounceness)/findLength(&joints[jointlabels[head]].velocity);
			joints[jointlabels[head]].velocity-=bounceness;
			if(1-friction*frictionness>0)joints[jointlabels[head]].velocity*=1-friction*frictionness;
			else joints[jointlabels[head]].velocity=0;
			if(findLengthfast(&bounceness)>2500){
			Normalise(&bounceness);
			bounceness=bounceness*50;
			}
			joints[jointlabels[head]].velocity+=bounceness*elasticity;


			if(!joints[jointlabels[head]].locked)
			if(findLengthfast(&joints[jointlabels[head]].velocity)<1){
			joints[jointlabels[head]].locked=1;
			//joints[jointlabels[head]].velocity*=3;
			}
			if(findLengthfast(&bounceness)>500)Sprite::MakeSprite(cloudsprite, headpos*(*scale)+*coords,joints[jointlabels[head]].velocity*.06, 1,1,1, .5, .2);
			joints[jointlabels[head]].position=(temp-*coords)/(*scale)+(startheadpos-headpos)+terrainnormal*.005;
			if(longdead>100)broken=1;
			}
			}
			}
			*/

			for(i=0; i<num_joints; i++){
				//joints[i].delay-=multiplier/1.5;
				if(joints[i].locked)
					if(!spinny)if(findLengthfast(&joints[i].velocity)>320)joints[i].locked=0;
				if(spinny)if(findLengthfast(&joints[i].velocity)>600)joints[i].locked=0;
				if(joints[i].delay>0){
					freely=1;
					for(j=0;j<num_joints;j++){
						if(joints[j].locked)freely=0;
					}
					if(freely)joints[i].delay-=multiplier*3;
				}
				//if(joints[i].delay>0)
				//if(findLengthfast(&joints[i].velocity)>700&&joints[i].label!=head)joints[i].delay-=multiplier;
			}

			if(num_muscles)
				for(i=0; i<num_muscles; i++){
					//Length constraints
					//muscles[i].DoConstraint(broken);
					muscles[i].DoConstraint(spinny);
				}

				for(i=0; i<num_joints; i++){
					//joints[i].delay-=multiplier/1.5;
					//Length constraints
					//Ground constraint
					groundlevel=0;
					if(joints[i].position.y*(*scale)+coords->y<terrain.getHeight(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z)+groundlevel){
						freefall=0;
						friction=1.5;
						if(joints[i].label==groin&&!joints[i].locked&&joints[i].delay<=0){
							joints[i].locked=1;
							joints[i].delay=1;
							static float gLoc[3];
							static float vel[3];
							gLoc[0]=joints[i].position.x*(*scale)+coords->x;
							gLoc[1]=joints[i].position.y*(*scale)+coords->y;
							gLoc[2]=joints[i].position.z*(*scale)+coords->z;
							vel[0]=joints[i].velocity.x;
							vel[1]=joints[i].velocity.y;
							vel[2]=joints[i].velocity.z;
							if(tutoriallevel!=1||id==0){
								PlaySoundEx( landsound1, samp[landsound1], NULL, true);
								OPENAL_3D_SetAttributes(channels[landsound1], gLoc, vel);
								OPENAL_SetVolume(channels[landsound1], 128);
								OPENAL_SetPaused(channels[landsound1], false);
							}
							breaking=1;
						}

						if(joints[i].label==head&&!joints[i].locked&&joints[i].delay<=0){
							joints[i].locked=1;
							joints[i].delay=1;
							static float gLoc[3];
							static float vel[3];
							gLoc[0]=joints[i].position.x*(*scale)+coords->x;
							gLoc[1]=joints[i].position.y*(*scale)+coords->y;
							gLoc[2]=joints[i].position.z*(*scale)+coords->z;
							vel[0]=joints[i].velocity.x;
							vel[1]=joints[i].velocity.y;
							vel[2]=joints[i].velocity.z;
							if(tutoriallevel!=1||id==0){
								PlaySoundEx( landsound2, samp[landsound2], NULL, true);
								OPENAL_3D_SetAttributes(channels[landsound2], gLoc, vel);
								OPENAL_SetVolume(channels[landsound2], 128);
								OPENAL_SetPaused(channels[landsound2], false);
							}
						}

						terrainnormal=terrain.getNormal(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z);
						ReflectVector(&joints[i].velocity,&terrainnormal);
						bounceness=terrainnormal*findLength(&joints[i].velocity)*(abs(normaldotproduct(joints[i].velocity,terrainnormal)));
						if(!joints[i].locked)damage+=findLengthfast(&bounceness)/4000;
						if(findLengthfast(&joints[i].velocity)<findLengthfast(&bounceness))bounceness=0;
						frictionness=abs(normaldotproduct(joints[i].velocity,terrainnormal));//findLength(&bounceness)/findLength(&joints[i].velocity);
						joints[i].velocity-=bounceness;
						if(1-friction*frictionness>0)joints[i].velocity*=1-friction*frictionness;
						else joints[i].velocity=0;

						if(tutoriallevel!=1||id==0)
							if(findLengthfast(&bounceness)>8000&&breaking){
								objects.model[k].MakeDecal(breakdecal,DoRotation(temp-objects.position[k],0,-objects.rotation[k],0),.4,.5,Random()%360);
								Sprite::MakeSprite(cloudsprite, joints[i].position*(*scale)+*coords,joints[i].velocity*.06, 1,1,1, 4, .2);
								//Sprite::MakeSprite(cloudsprite, joints[i].position*(*scale)+*coords,joints[i].velocity*.06, 1,1,1, 1, .2);
								breaking=0;
								camerashake+=.6;

								static float gLoc[3];
								static float vel[3];
								gLoc[0]=joints[i].position.x*(*scale)+coords->x;
								gLoc[1]=joints[i].position.y*(*scale)+coords->y;
								gLoc[2]=joints[i].position.z*(*scale)+coords->z;
								vel[0]=joints[i].velocity.x;
								vel[1]=joints[i].velocity.y;
								vel[2]=joints[i].velocity.z;
								PlaySoundEx( breaksound2, samp[breaksound2], NULL, true);
								OPENAL_3D_SetAttributes(channels[breaksound2], gLoc, vel);
								OPENAL_SetVolume(channels[breaksound2], 300);
								OPENAL_SetPaused(channels[breaksound2], false);

								envsound[numenvsounds]=*coords;
								envsoundvol[numenvsounds]=64;
								envsoundlife[numenvsounds]=.4;
								numenvsounds++;
							}

							if(findLengthfast(&bounceness)>2500){
								Normalise(&bounceness);
								bounceness=bounceness*50;
							}

							joints[i].velocity+=bounceness*elasticity;

							if(findLengthfast(&joints[i].velocity)>findLengthfast(&joints[i].oldvelocity)){
								bounceness=0;
								joints[i].velocity=joints[i].oldvelocity;
							}


							if(joints[i].locked==0)
								if(findLengthfast(&joints[i].velocity)<1)joints[i].locked=1;

							if(environment==snowyenvironment&&findLengthfast(&bounceness)>500&&terrain.getOpacity(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z)<.2){
								terrainlight=terrain.getLighting(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z);
								Sprite::MakeSprite(cloudsprite, joints[i].position*(*scale)+*coords,joints[i].velocity*.06, terrainlight.x,terrainlight.y,terrainlight.z, .5, .7);
								if(detail==2)terrain.MakeDecal(bodyprintdecal, joints[i].position*(*scale)+*coords,.4,.4,0);
							}
							else if(environment==desertenvironment&&findLengthfast(&bounceness)>500&&terrain.getOpacity(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z)<.2){
								terrainlight=terrain.getLighting(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z);
								Sprite::MakeSprite(cloudsprite, joints[i].position*(*scale)+*coords,joints[i].velocity*.06, terrainlight.x*190/255,terrainlight.y*170/255,terrainlight.z*108/255, .5, .7);
							}

							else if(environment==grassyenvironment&&findLengthfast(&bounceness)>500&&terrain.getOpacity(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z)<.2){
								terrainlight=terrain.getLighting(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z);
								Sprite::MakeSprite(cloudsprite, joints[i].position*(*scale)+*coords,joints[i].velocity*.06, terrainlight.x*90/255,terrainlight.y*70/255,terrainlight.z*8/255, .5, .5);
							}
							else if(findLengthfast(&bounceness)>500)Sprite::MakeSprite(cloudsprite, joints[i].position*(*scale)+*coords,joints[i].velocity*.06, terrainlight.x,terrainlight.y,terrainlight.z, .5, .2);


							joints[i].position.y=(terrain.getHeight(joints[i].position.x*(*scale)+coords->x,joints[i].position.z*(*scale)+coords->z)+groundlevel-coords->y)/(*scale);
							if(longdead>100)broken=1;
					}
					if(terrain.patchobjectnum[whichpatchx][whichpatchz])
						for(m=0;m<terrain.patchobjectnum[whichpatchx][whichpatchz];m++){
							k=terrain.patchobjects[whichpatchx][whichpatchz][m];
							if(k<objects.numobjects&&k>=0)
								if(objects.possible[k]){
									friction=objects.friction[k];
									start=joints[i].realoldposition;
									end=joints[i].position*(*scale)+*coords;
									whichhit=objects.model[k].LineCheckPossible(&start,&end,&temp,&objects.position[k],&objects.rotation[k]);
									if(whichhit!=-1){
										if(joints[i].label==groin&&!joints[i].locked&&joints[i].delay<=0){
											joints[i].locked=1;
											joints[i].delay=1;
											static float gLoc[3];
											static float vel[3];
											gLoc[0]=joints[i].position.x*(*scale)+coords->x;
											gLoc[1]=joints[i].position.y*(*scale)+coords->y;
											gLoc[2]=joints[i].position.z*(*scale)+coords->z;
											vel[0]=joints[i].velocity.x;
											vel[1]=joints[i].velocity.y;
											vel[2]=joints[i].velocity.z;
											if(tutoriallevel!=1||id==0){
												PlaySoundEx( landsound1, samp[landsound1], NULL, true);
												OPENAL_3D_SetAttributes(channels[landsound1], gLoc, vel);
												OPENAL_SetVolume(channels[landsound1], 128);
												OPENAL_SetPaused(channels[landsound1], false);
											}
											breaking=1;
										}

										if(joints[i].label==head&&!joints[i].locked&&joints[i].delay<=0){
											joints[i].locked=1;
											joints[i].delay=1;
											static float gLoc[3];
											static float vel[3];
											gLoc[0]=joints[i].position.x*(*scale)+coords->x;
											gLoc[1]=joints[i].position.y*(*scale)+coords->y;
											gLoc[2]=joints[i].position.z*(*scale)+coords->z;
											vel[0]=joints[i].velocity.x;
											vel[1]=joints[i].velocity.y;
											vel[2]=joints[i].velocity.z;
											if(tutoriallevel!=1||id==0){
												PlaySoundEx( landsound2, samp[landsound2], NULL, true);
												OPENAL_3D_SetAttributes(channels[landsound2], gLoc, vel);
												OPENAL_SetVolume(channels[landsound2], 128);
												OPENAL_SetPaused(channels[landsound2], false);
											}
										}

										terrainnormal=DoRotation(objects.model[k].facenormals[whichhit],0,objects.rotation[k],0)*-1;
										if(terrainnormal.y>.8)freefall=0;
										bounceness=terrainnormal*findLength(&joints[i].velocity)*(abs(normaldotproduct(joints[i].velocity,terrainnormal)));
										if(findLengthfast(&joints[i].velocity)>findLengthfast(&joints[i].oldvelocity)){
											bounceness=0;
											joints[i].velocity=joints[i].oldvelocity;
										}
										if(tutoriallevel!=1||id==0)
											if(findLengthfast(&bounceness)>4000&&breaking){
												objects.model[k].MakeDecal(breakdecal,DoRotation(temp-objects.position[k],0,-objects.rotation[k],0),.4,.5,Random()%360);
												Sprite::MakeSprite(cloudsprite, joints[i].position*(*scale)+*coords,joints[i].velocity*.06, 1,1,1, 4, .2);
												breaking=0;
												camerashake+=.6;

												static float gLoc[3];
												static float vel[3];
												gLoc[0]=joints[i].position.x*(*scale)+coords->x;
												gLoc[1]=joints[i].position.y*(*scale)+coords->y;
												gLoc[2]=joints[i].position.z*(*scale)+coords->z;
												vel[0]=joints[i].velocity.x;
												vel[1]=joints[i].velocity.y;
												vel[2]=joints[i].velocity.z;
												PlaySoundEx( breaksound2, samp[breaksound2], NULL, true);
												OPENAL_3D_SetAttributes(channels[breaksound2], gLoc, vel);
												OPENAL_SetVolume(channels[breaksound2], 300);
												OPENAL_SetPaused(channels[breaksound2], false);

												envsound[numenvsounds]=*coords;
												envsoundvol[numenvsounds]=64;
												envsoundlife[numenvsounds]=.4;
												numenvsounds++;
											}
											if(objects.type[k]==treetrunktype){
												//if(objects.rotx[k]==0||objects.roty[k]==0){
												/*int howmany;
												XYZ tempvel;
												XYZ pos;
												if(environment==grassyenvironment)howmany=findLength(&joints[i].velocity)*4/10;
												if(environment==snowyenvironment)howmany=findLength(&joints[i].velocity)*1/10;
												if(environment!=desertenvironment)
												for(j=0;j<howmany;j++){
												tempvel.x=float(abs(Random()%100)-50)/20;
												tempvel.y=float(abs(Random()%100)-50)/20;
												tempvel.z=float(abs(Random()%100)-50)/20;
												pos=objects.position[k];
												pos.y+=objects.scale[k]*15;
												pos.x+=float(abs(Random()%100)-50)/100*objects.scale[k]*5;
												pos.y+=float(abs(Random()%100)-50)/100*objects.scale[k]*15;
												pos.z+=float(abs(Random()%100)-50)/100*objects.scale[k]*5;
												Sprite::MakeSprite(splintersprite, pos,tempvel*.5, 165/255+float(abs(Random()%100)-50)/400,0,0, .2+float(abs(Random()%100)-50)/1300, 1);
												Sprite::special[Sprite::numsprites-1]=1;
												}*/
												objects.rotx[k]+=joints[i].velocity.x*multiplier*.4;
												objects.roty[k]+=joints[i].velocity.z*multiplier*.4;
												objects.rotx[k+1]+=joints[i].velocity.x*multiplier*.4;
												objects.roty[k+1]+=joints[i].velocity.z*multiplier*.4;
											}
											if(!joints[i].locked)damage+=findLengthfast(&bounceness)/2500;
											ReflectVector(&joints[i].velocity,&terrainnormal);
											frictionness=abs(normaldotproduct(joints[i].velocity,terrainnormal));//findLength(&bounceness)/findLength(&joints[i].velocity);
											joints[i].velocity-=bounceness;
											if(1-friction*frictionness>0)joints[i].velocity*=1-friction*frictionness;
											else joints[i].velocity=0;
											if(findLengthfast(&bounceness)>2500){
												Normalise(&bounceness);
												bounceness=bounceness*50;
											}
											joints[i].velocity+=bounceness*elasticity;


											if(!joints[i].locked)
												if(findLengthfast(&joints[i].velocity)<1){
													joints[i].locked=1;
													//joints[i].velocity*=3;
												}
												if(findLengthfast(&bounceness)>500)Sprite::MakeSprite(cloudsprite, joints[i].position*(*scale)+*coords,joints[i].velocity*.06, 1,1,1, .5, .2);
												joints[i].position=(temp-*coords)/(*scale)+terrainnormal*.005;
												if(longdead>100)broken=1;
									}
								}
						}
						joints[i].realoldposition=joints[i].position*(*scale)+*coords;
				}
		}
		multiplier=tempmult;


		if(terrain.patchobjectnum[whichpatchx][whichpatchz])
			for(m=0;m<terrain.patchobjectnum[whichpatchx][whichpatchz];m++){
				k=terrain.patchobjects[whichpatchx][whichpatchz][m];
				if(objects.possible[k]){
					for(i=0;i<26;i++){
						//Make this less stupid
						start=joints[jointlabels[whichjointstartarray[i]]].position*(*scale)+*coords;
						end=joints[jointlabels[whichjointendarray[i]]].position*(*scale)+*coords;
						whichhit=objects.model[k].LineCheckSlidePossible(&start,&end,&temp,&objects.position[k],&objects.rotation[k]);
						if(whichhit!=-1){
							joints[jointlabels[whichjointendarray[i]]].position=(end-*coords)/(*scale);
							for(j=0; j<num_muscles; j++){
								if((muscles[j].parent1->label==whichjointstartarray[i]&&muscles[j].parent2->label==whichjointendarray[i])||(muscles[j].parent2->label==whichjointstartarray[i]&&muscles[j].parent1->label==whichjointendarray[i]))
									muscles[j].DoConstraint(spinny);
							}
						}
					}
				}
			}

			for(i=0; i<num_joints; i++){
				groundlevel=.15;
				if(joints[i].label==head)groundlevel=.8;
				if(joints[i].label==righthand||joints[i].label==rightwrist||joints[i].label==rightelbow)groundlevel=.2;
				if(joints[i].label==lefthand||joints[i].label==leftwrist||joints[i].label==leftelbow)groundlevel=.2;
				joints[i].position.y+=groundlevel;
				joints[i].mass=1;
				if(joints[i].label==lefthip||joints[i].label==leftknee||joints[i].label==leftankle||joints[i].label==righthip||joints[i].label==rightknee||joints[i].label==rightankle)joints[i].mass=2;
				if(joints[i].locked){
					joints[i].mass=4;
				}
			}

			return damage;
	}
	if(!free){
		for(i=0; i<num_muscles; i++){
			if(muscles[i].type==boneconnect)
				muscles[i].DoConstraint(0);
		}
	}
	return 0;
}

void Skeleton::DoGravity(float *scale)
{
	static int i;
	for(i=0; i<num_joints; i++){
		if(((joints[i].label!=leftknee&&joints[i].label!=rightknee)||lowforward.y>-.1||joints[i].mass<5)&&((joints[i].label!=rightelbow&&joints[i].label!=rightelbow)||forward.y<.3))joints[i].velocity.y+=gravity*multiplier/(*scale);
	}
}

void Skeleton::Draw(int  muscleview)
{
	static float jointcolor[4];

	if(muscleview!=2){
		jointcolor[0]=0;
		jointcolor[1]=0;
		jointcolor[2]=.5;
		jointcolor[3]=1;
	}

	if(muscleview==2){
		jointcolor[0]=0;
		jointcolor[1]=0;
		jointcolor[2]=0;
		jointcolor[3]=.5;
	}
	//Calc motionblur-ness
	for(int i=0; i<num_joints; i++){
		joints[i].oldposition=joints[i].position;
		joints[i].blurred=findDistance(&joints[i].position,&joints[i].oldposition)*100;
		if(joints[i].blurred<1)joints[i].blurred=1;
	}

	//Do Motionblur
	glDepthMask(0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	for(int i=0; i<num_joints; i++){
		if(joints[i].hasparent){
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/joints[i].blurred);
			glVertex3f(joints[i].position.x,joints[i].position.y,joints[i].position.z);
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/joints[i].parent->blurred);
			glVertex3f(joints[i].parent->position.x,joints[i].parent->position.y,joints[i].parent->position.z);
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/joints[i].parent->blurred);
			glVertex3f(joints[i].parent->oldposition.x,joints[i].parent->oldposition.y,joints[i].parent->oldposition.z);
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/joints[i].blurred);
			glVertex3f(joints[i].oldposition.x,joints[i].oldposition.y,joints[i].oldposition.z);
		}
	}
	for(int i=0; i<num_muscles; i++){
		if(muscles[i].type==boneconnect){
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/muscles[i].parent2->blurred);
			glVertex3f(muscles[i].parent1->position.x,muscles[i].parent1->position.y,muscles[i].parent1->position.z);
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/muscles[i].parent2->blurred);
			glVertex3f(muscles[i].parent2->position.x,muscles[i].parent2->position.y,muscles[i].parent2->position.z);
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/muscles[i].parent2->blurred);
			glVertex3f(muscles[i].parent2->oldposition.x,muscles[i].parent2->oldposition.y,muscles[i].parent2->oldposition.z);
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/muscles[i].parent1->blurred);
			glVertex3f(muscles[i].parent1->oldposition.x,muscles[i].parent1->oldposition.y,muscles[i].parent1->oldposition.z);
		}
	}
	glEnd();

	glBegin(GL_LINES);
	for(int i=0; i<num_joints; i++){
		if(joints[i].hasparent){
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/joints[i].blurred);
			glVertex3f(joints[i].position.x,joints[i].position.y,joints[i].position.z);
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/joints[i].parent->blurred);
			glVertex3f(joints[i].parent->position.x,joints[i].parent->position.y,joints[i].parent->position.z);
		}
	}
	/*for(int i=0; i<num_joints; i++){
	if(joints[i].hasparent){
	glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],1);
	glVertex3f(joints[i].position.x,joints[i].position.y,joints[i].position.z);
	glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],1);
	glVertex3f(joints[i].position.x+forward.x,joints[i].position.y+forward.y,joints[i].position.z+forward.z);
	}
	}*/
	for(int i=0; i<num_muscles; i++){
		if(muscles[i].type==boneconnect){
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/muscles[i].parent1->blurred);
			glVertex3f(muscles[i].parent1->position.x,muscles[i].parent1->position.y,muscles[i].parent1->position.z);
			glColor4f(jointcolor[0],jointcolor[1],jointcolor[2],jointcolor[3]/muscles[i].parent2->blurred);
			glVertex3f(muscles[i].parent2->position.x,muscles[i].parent2->position.y,muscles[i].parent2->position.z);
		}
	}
	glColor3f(.6,.6,0);
	if(muscleview==1)
		for(int i=0; i<num_muscles; i++){
			if(muscles[i].type!=boneconnect){
				glVertex3f(muscles[i].parent1->position.x,muscles[i].parent1->position.y,muscles[i].parent1->position.z);
				glVertex3f(muscles[i].parent2->position.x,muscles[i].parent2->position.y,muscles[i].parent2->position.z);
			}
		}
		glEnd();

		if(muscleview!=2){
			glPointSize(3);
			glBegin(GL_POINTS);
			for(int i=0; i<num_joints; i++){
				if(i!=selected)glColor4f(0,0,.5,1);
				if(i==selected)glColor4f(1,1,0,1);
				if(joints[i].locked&&i!=selected)glColor4f(1,0,0,1);
				glVertex3f(joints[i].position.x,joints[i].position.y,joints[i].position.z);
			}
			glEnd();
		}

		//Set old position to current position
		if(muscleview==2)
			for(int i=0; i<num_joints; i++){
				joints[i].oldposition=joints[i].position;
			}
			glDepthMask(1);
}

void Skeleton::AddJoint(float x, float y, float z, int which)
{
	if(num_joints<max_joints-1){
		joints[num_joints].velocity=0;
		joints[num_joints].position.x=x;
		joints[num_joints].position.y=y;
		joints[num_joints].position.z=z;
		joints[num_joints].mass=1;
		joints[num_joints].locked=0;

		/*if(which>=num_joints||which<0)*/joints[num_joints].hasparent=0;
		/*if(which<num_joints&&which>=0){
		joints[num_joints].parent=&joints[which];
		joints[num_joints].hasparent=1;
		joints[num_joints].length=findDistance(joints[num_joints].position,joints[num_joints].parent->position);
		}*/
		num_joints++;
		if(which<num_joints&&which>=0)AddMuscle(num_joints-1,which,0,10,boneconnect);
	}
}

void Skeleton::DeleteJoint(int whichjoint)
{
	if(whichjoint<num_joints&&whichjoint>=0){
		joints[whichjoint].velocity=joints[num_joints-1].velocity;
		joints[whichjoint].position=joints[num_joints-1].position;
		joints[whichjoint].oldposition=joints[num_joints-1].oldposition;
		joints[whichjoint].hasparent=joints[num_joints-1].hasparent;
		joints[whichjoint].parent=joints[num_joints-1].parent;
		joints[whichjoint].length=joints[num_joints-1].length;
		joints[whichjoint].locked=joints[num_joints-1].locked;
		joints[whichjoint].modelnum=joints[num_joints-1].modelnum;
		joints[whichjoint].visible=joints[num_joints-1].visible;

		for(int i=0;i<num_muscles;i++){
			while(muscles[i].parent1==&joints[whichjoint]&&i<num_muscles)DeleteMuscle(i);
			while(muscles[i].parent2==&joints[whichjoint]&&i<num_muscles)DeleteMuscle(i);
		}
		for(int i=0;i<num_muscles;i++){
			while(muscles[i].parent1==&joints[num_joints-1]&&i<num_muscles)muscles[i].parent1=&joints[whichjoint];
			while(muscles[i].parent2==&joints[num_joints-1]&&i<num_muscles)muscles[i].parent2=&joints[whichjoint];
		}
		for(int i=0;i<num_joints;i++){
			if(joints[i].parent==&joints[whichjoint])joints[i].hasparent=0;
		}
		for(int i=0;i<num_joints;i++){
			if(joints[i].parent==&joints[num_joints-1])joints[i].parent=&joints[whichjoint];
		}

		num_joints--;
	}
}

void Skeleton::DeleteMuscle(int whichmuscle)
{
	if(whichmuscle<num_muscles){
		muscles[whichmuscle].minlength=muscles[num_muscles-1].minlength;
		muscles[whichmuscle].maxlength=muscles[num_muscles-1].maxlength;
		muscles[whichmuscle].strength=muscles[num_muscles-1].strength;
		muscles[whichmuscle].parent1=muscles[num_muscles-1].parent1;
		muscles[whichmuscle].parent2=muscles[num_muscles-1].parent2;
		muscles[whichmuscle].length=muscles[num_muscles-1].length;
		muscles[whichmuscle].visible=muscles[num_muscles-1].visible;
		muscles[whichmuscle].type=muscles[num_muscles-1].type;
		muscles[whichmuscle].targetlength=muscles[num_muscles-1].targetlength;

		num_muscles--;
	}
}

void Skeleton::SetJoint(float x, float y, float z, int which, int whichjoint)
{
	if(whichjoint<num_joints){
		joints[whichjoint].velocity=0;
		joints[whichjoint].position.x=x;
		joints[whichjoint].position.y=y;
		joints[whichjoint].position.z=z;

		if(which>=num_joints||which<0)joints[whichjoint].hasparent=0;
		if(which<num_joints&&which>=0){
			joints[whichjoint].parent=&joints[which];
			joints[whichjoint].hasparent=1;
			joints[whichjoint].length=findDistance(&joints[whichjoint].position,&joints[whichjoint].parent->position);
		}
	}
}

void Skeleton::AddMuscle(int attach1,int attach2,float minlength,float maxlength,int type)
{
	if(num_muscles<max_muscles-1&&attach1<num_joints&&attach1>=0&&attach2<num_joints&&attach2>=0&&attach1!=attach2){
		muscles[num_muscles].parent1=&joints[attach1];
		muscles[num_muscles].parent2=&joints[attach2];
		muscles[num_muscles].length=findDistance(&muscles[num_muscles].parent1->position,&muscles[num_muscles].parent2->position);
		muscles[num_muscles].targetlength=findDistance(&muscles[num_muscles].parent1->position,&muscles[num_muscles].parent2->position);
		muscles[num_muscles].strength=.7;
		muscles[num_muscles].type=type;
		muscles[num_muscles].minlength=minlength;
		muscles[num_muscles].maxlength=maxlength;

		num_muscles++;
	}
}

void Skeleton::MusclesSet()
{
	for(int i=0;i<num_muscles;i++){
		muscles[i].length=findDistance(&muscles[i].parent1->position,&muscles[i].parent2->position);
	}
}

void Skeleton::DoBalance()
{
	/*XYZ newpoint;
	newpoint=joints[0].position;
	newpoint.x=(joints[2].position.x+joints[4].position.x)/2;
	newpoint.z=(joints[2].position.z+joints[4].position.z)/2;
	joints[0].velocity=joints[0].velocity+(newpoint-joints[0].position);
	//Move child point to within certain distance of parent point
	joints[0].position=newpoint;

	MusclesSet();*/
}

void Skeleton::FindRotationMuscle(int which, int animation)
{
	static XYZ temppoint1,temppoint2,tempforward;
	static float distance;

	temppoint1=muscles[which].parent1->position;
	temppoint2=muscles[which].parent2->position;
	distance=sqrt((temppoint1.x-temppoint2.x)*(temppoint1.x-temppoint2.x)+(temppoint1.y-temppoint2.y)*(temppoint1.y-temppoint2.y)+(temppoint1.z-temppoint2.z)*(temppoint1.z-temppoint2.z));
	if((temppoint1.y-temppoint2.y)<=distance)muscles[which].rotate2=asin((temppoint1.y-temppoint2.y)/distance);
	if((temppoint1.y-temppoint2.y)>distance)muscles[which].rotate2=asin(1.f);
	muscles[which].rotate2*=360/6.28;
	temppoint1.y=0;
	temppoint2.y=0;
	distance=sqrt((temppoint1.x-temppoint2.x)*(temppoint1.x-temppoint2.x)+(temppoint1.y-temppoint2.y)*(temppoint1.y-temppoint2.y)+(temppoint1.z-temppoint2.z)*(temppoint1.z-temppoint2.z));
	if((temppoint1.z-temppoint2.z)<=distance)muscles[which].rotate1=acos((temppoint1.z-temppoint2.z)/distance);
	if((temppoint1.z-temppoint2.z)>distance)muscles[which].rotate1=acos(1.f);
	muscles[which].rotate1*=360/6.28;
	if(temppoint1.x>temppoint2.x)muscles[which].rotate1=360-muscles[which].rotate1;
	if(!isnormal(muscles[which].rotate1))muscles[which].rotate1=0;
	if(!isnormal(muscles[which].rotate2))muscles[which].rotate2=0;

	if(muscles[which].parent1->label==head)tempforward=specialforward[0];
	else if(muscles[which].parent1->label==rightshoulder||muscles[which].parent1->label==rightelbow||muscles[which].parent1->label==rightwrist||muscles[which].parent1->label==righthand)tempforward=specialforward[1];
	else if(muscles[which].parent1->label==leftshoulder||muscles[which].parent1->label==leftelbow||muscles[which].parent1->label==leftwrist||muscles[which].parent1->label==lefthand)tempforward=specialforward[2];
	else if(muscles[which].parent1->label==righthip||muscles[which].parent1->label==rightknee||muscles[which].parent1->label==rightankle||muscles[which].parent1->label==rightfoot)tempforward=specialforward[3];
	else if(muscles[which].parent1->label==lefthip||muscles[which].parent1->label==leftknee||muscles[which].parent1->label==leftankle||muscles[which].parent1->label==leftfoot)tempforward=specialforward[4];
	else if(!muscles[which].parent1->lower)tempforward=forward;
	else if(muscles[which].parent1->lower)tempforward=lowforward;

	if(animation==hanganim){
		if(muscles[which].parent1->label==righthand||muscles[which].parent2->label==righthand){
			tempforward=0;
			tempforward.x=-1;
		}
		if(muscles[which].parent1->label==lefthand||muscles[which].parent2->label==lefthand){
			tempforward=0;
			tempforward.x=1;
		}
	}

	if(free==0){
		if(muscles[which].parent1->label==rightfoot||muscles[which].parent2->label==rightfoot){
			tempforward.y-=.3;
		}
		if(muscles[which].parent1->label==leftfoot||muscles[which].parent2->label==leftfoot){
			tempforward.y-=.3;
		}
	}


	tempforward=DoRotation(tempforward,0,muscles[which].rotate1-90,0);
	tempforward=DoRotation(tempforward,0,0,muscles[which].rotate2-90);
	tempforward.y=0;
	tempforward/=sqrt(tempforward.x*tempforward.x+tempforward.y*tempforward.y+tempforward.z*tempforward.z);
	if(tempforward.z<=1&&tempforward.z>=-1)muscles[which].rotate3=acos(0-tempforward.z);
	else muscles[which].rotate3=acos(-1.f);
	muscles[which].rotate3*=360/6.28;
	if(0>tempforward.x)muscles[which].rotate3=360-muscles[which].rotate3;
	if(!isnormal(muscles[which].rotate3))muscles[which].rotate3=0;
}

void Animation::Load(const char *filename, int aheight, int aattack)
{
	static FILE *tfile;
	static int i,j;
	static XYZ startoffset,endoffset;
	static int howmany;

	static const char *anim_prefix = ":Data:Animations:";


	LOGFUNC;

	int len = strlen(anim_prefix) + strlen(filename);
	char *buf = new char[len + 1];
	snprintf(buf, len + 1, "%s%s", anim_prefix, filename);
	// Changing the filename into something the OS can understand
	char *fixedFN = ConvertFileName(buf);
	delete[] buf;

	LOG(std::string("Loading animation...") + fixedFN);

	deallocate();

	height=aheight;
	attack=aattack;

	if(visibleloading)pgame->LoadingScreen();

	tfile=fopen( fixedFN, "rb" );
	if(tfile){
		funpackf(tfile, "Bi Bi", &numframes, &joints);
		/*
		for(i = 0; i < joints; i++){
		if(position[i])dealloc2(position[i]);
		if(twist[i])dealloc2(twist[i]);
		if(twist2[i])dealloc2(twist2[i]);
		if(onground[i])dealloc2(onground[i]);
		}*/
		/*
		if(position)dealloc2(position);
		if(twist)dealloc2(twist);
		if(twist2)dealloc2(twist2);
		if(speed)dealloc2(speed);
		if(onground)dealloc2(onground);
		if(forward)dealloc2(forward);
		if(weapontarget)dealloc2(weapontarget);
		if(label)dealloc2(label);*/

		position=(XYZ**)malloc(sizeof(XYZ*)*joints);
		for(i = 0; i < joints; i++)
			position[i] = (XYZ*)malloc(sizeof(XYZ)*numframes);

		twist=(float**)malloc(sizeof(float*)*joints);
		for(i = 0; i < joints; i++)
			twist[i] = (float*)malloc(sizeof(float)*numframes);

		twist2=(float**)malloc(sizeof(float*)*joints);
		for(i = 0; i < joints; i++)
			twist2[i] = (float*)malloc(sizeof(float)*numframes);

		speed = (float*)malloc(sizeof(float)*numframes);

		onground=(bool**)malloc(sizeof(bool*)*joints);
		for(i = 0; i < joints; i++)
			onground[i] =(bool*)malloc(sizeof(bool)*numframes);

		forward = (XYZ*)malloc(sizeof(XYZ)*numframes);
		weapontarget = (XYZ*)malloc(sizeof(XYZ)*numframes);
		label = (int*)malloc(sizeof(int)*numframes);

		/*position = new XYZ[joints][numframes];
		twist = new float[joints][numframes];
		twist2 = new float[joints][numframes];
		speed = new float[numframes];
		onground = new bool[joints][numframes];
		forward = new XYZ[numframes];
		label = new int[numframes];*/

		for(i=0;i<numframes;i++){
			for(j=0;j<joints;j++){
				funpackf(tfile, "Bf Bf Bf", &position[j][i].x,&position[j][i].y,&position[j][i].z);
			}
			for(j=0;j<joints;j++){
				funpackf(tfile, "Bf", &twist[j][i]);
			}
			for(j=0;j<joints;j++){
				unsigned char uch;
				funpackf(tfile, "Bb", &uch);
				onground[j][i] = (uch != 0);
			}
			funpackf(tfile, "Bf", &speed[i]);
		}
		for(i=0;i<numframes;i++){
			for(j=0;j<joints;j++){
				funpackf(tfile, "Bf", &twist2[j][i]);
			}
		}
		for(i=0;i<numframes;i++){
			funpackf(tfile, "Bf", &label[i]);
		}
		funpackf(tfile, "Bi", &weapontargetnum);
		for(i=0;i<numframes;i++){
			funpackf(tfile, "Bf Bf Bf", &weapontarget[i].x,&weapontarget[i].y,&weapontarget[i].z);
		}

		fclose(tfile);
	}

	startoffset=0;
	endoffset=0;
	howmany=0;
	for(j=0;j<joints;j++){
		if(position[j][0].y<1)
			startoffset+=position[j][0];
		if(position[j][numframes-1].y<1)
			endoffset+=position[j][numframes-1];
		howmany++;
	}
	startoffset/=howmany;
	endoffset/=howmany;
	offset=endoffset;
	offset.y=0;
}


void Animation::Move(XYZ how)
{
	static int i,j,joints;
	for(i=0;i<numframes;i++){
		for(j=0;j<joints;j++){
			position[j][i]=0;
		}
	}
}

void Skeleton::Load(const char *filename,       const char *lowfilename, const char *clothesfilename, 
                    const char *modelfilename,  const char *model2filename, 
                    const char *model3filename, const char *model4filename, 
                    const char *model5filename, const char *model6filename, 
                    const char *model7filename, const char *modellowfilename, 
                    const char *modelclothesfilename, bool aclothes)
{
	static GLfloat M[16];
	static int parentID;
	static FILE *tfile;
	static float lSize;
	static int i,j,tempmuscle;
	int newload;
	int edit;

	LOGFUNC;


	newload=0;

	num_models=7;

	clothes=aclothes;

	for(i=0;i<num_models;i++){
		if(i==0)model[i].loadnotex(modelfilename);
		if(i==1)model[i].loadnotex(model2filename);
		if(i==2)model[i].loadnotex(model3filename);
		if(i==3)model[i].loadnotex(model4filename);
		if(i==4)model[i].loadnotex(model5filename);
		if(i==5)model[i].loadnotex(model6filename);
		if(i==6)model[i].loadnotex(model7filename);
		model[i].Rotate(180,0,0);
		model[i].Scale(.04,.04,.04);
		model[i].CalculateNormals(0);
	}

	drawmodel.load(modelfilename,0);
	drawmodel.Rotate(180,0,0);
	drawmodel.Scale(.04,.04,.04);
	drawmodel.FlipTexCoords();
	if(tutoriallevel==1&&id!=0)drawmodel.UniformTexCoords();
	if(tutoriallevel==1&&id!=0)drawmodel.ScaleTexCoords(0.1);
	drawmodel.CalculateNormals(0);

	modellow.loadnotex(modellowfilename);
	modellow.Rotate(180,0,0);
	modellow.Scale(.04,.04,.04);
	modellow.CalculateNormals(0);

	drawmodellow.load(modellowfilename,0);
	drawmodellow.Rotate(180,0,0);
	drawmodellow.Scale(.04,.04,.04);
	drawmodellow.FlipTexCoords();
	if(tutoriallevel==1&&id!=0)drawmodellow.UniformTexCoords();
	if(tutoriallevel==1&&id!=0)drawmodellow.ScaleTexCoords(0.1);
	drawmodellow.CalculateNormals(0);

	if(clothes){
		modelclothes.loadnotex(modelclothesfilename);
		modelclothes.Rotate(180,0,0);
		modelclothes.Scale(.041,.04,.041);
		modelclothes.CalculateNormals(0);

		drawmodelclothes.load(modelclothesfilename,0);
		drawmodelclothes.Rotate(180,0,0);
		drawmodelclothes.Scale(.04,.04,.04);
		drawmodelclothes.FlipTexCoords();
		drawmodelclothes.CalculateNormals(0);
	}

	tfile=fopen( ConvertFileName(filename), "rb" );
	if(1){
		funpackf(tfile, "Bi", &num_joints);
		//joints.resize(num_joints);
		if(joints) delete [] joints; //dealloc2(joints);
		joints=(Joint*)new Joint[num_joints]; //malloc(sizeof(Joint)*num_joints);

		for(i=0;i<num_joints;i++){
			funpackf(tfile, "Bf Bf Bf Bf Bf", &joints[i].position.x, &joints[i].position.y, &joints[i].position.z, &joints[i].length,&joints[i].mass);
			funpackf(tfile, "Bb Bb", &joints[i].hasparent,&joints[i].locked);
			funpackf(tfile, "Bi", &joints[i].modelnum);
			funpackf(tfile, "Bb Bb", &joints[i].visible,&joints[i].sametwist);
			funpackf(tfile, "Bi Bi", &joints[i].label,&joints[i].hasgun);
			funpackf(tfile, "Bb", &joints[i].lower);
			funpackf(tfile, "Bi", &parentID);
			if(joints[i].hasparent)joints[i].parent=&joints[parentID];
			joints[i].velocity=0;
			joints[i].oldposition=joints[i].position;
		}
		tempmuscle=num_muscles;
		funpackf(tfile, "Bi", &num_muscles);
		//muscles.clear();
		if(muscles) delete [] muscles; //dealloc2(muscles);
		muscles=(Muscle*)new Muscle[num_muscles]; //malloc(sizeof(Muscle)*num_muscles);
		newload=1;
		for(i=0;i<num_muscles;i++){
			tempmuscle=muscles[i].numvertices;
			funpackf(tfile, "Bf Bf Bf Bf Bf Bi Bi", &muscles[i].length, &muscles[i].targetlength,&muscles[i].minlength, &muscles[i].maxlength,&muscles[i].strength,&muscles[i].type,&muscles[i].numvertices);
			//muscles[i].vertices.clear();
			//muscles[i].vertices.resize(muscles[i].numvertices);
			//if(muscles[i].vertices)dealloc2(muscles[i].vertices);
			muscles[i].vertices=(int*)malloc(sizeof(int)*muscles[i].numvertices);

			edit=0;
			for(j=0;j<muscles[i].numvertices-edit;j++){
				funpackf(tfile, "Bi", &muscles[i].vertices[j+edit]);
				if(muscles[i].vertices[j+edit]>=model[0].vertexNum){
					muscles[i].numvertices--;
					edit--;
				}
			}
			funpackf(tfile, "Bb Bi", &muscles[i].visible, &parentID);
			muscles[i].parent1=&joints[parentID];
			funpackf(tfile, "Bi", &parentID);
			muscles[i].parent2=&joints[parentID];
		}
		for(j=0;j<3;j++){
			funpackf(tfile, "Bi", &forwardjoints[j]);
		}
		for(j=0;j<3;j++){
			funpackf(tfile, "Bi", &lowforwardjoints[j]);
		}
		for(j=0;j<num_muscles;j++){
			for(i=0;i<muscles[j].numvertices;i++){
				for(int k=0;k<num_models;k++){
					if(muscles[j].numvertices&&muscles[j].vertices[i]<model[k].vertexNum)model[k].owner[muscles[j].vertices[i]]=j;
				}
			}
		}
		FindForwards();
		for(i=0;i<num_joints;i++){
			joints[i].startpos=joints[i].position;
		}
		for(i=0;i<num_muscles;i++){
			FindRotationMuscle(i,-1);
		}
		for(int k=0;k<num_models;k++){
			for(i=0;i<model[k].vertexNum;i++){
				model[k].vertex[i]=model[k].vertex[i]-(muscles[model[k].owner[i]].parent1->position+muscles[model[k].owner[i]].parent2->position)/2;
				glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
				glPushMatrix();
					glLoadIdentity();
					glRotatef(muscles[model[k].owner[i]].rotate3,0,1,0);
					glRotatef(muscles[model[k].owner[i]].rotate2-90,0,0,1);
					glRotatef(muscles[model[k].owner[i]].rotate1-90,0,1,0);
					glTranslatef(model[k].vertex[i].x,model[k].vertex[i].y,model[k].vertex[i].z);
					glGetFloatv(GL_MODELVIEW_MATRIX,M);
					model[k].vertex[i].x=M[12]*1;
					model[k].vertex[i].y=M[13]*1;
					model[k].vertex[i].z=M[14]*1;
				glPopMatrix();
			}
			model[k].CalculateNormals(0);
		}
	}
	fclose(tfile);

	tfile=fopen( ConvertFileName(lowfilename), "rb" );
	if(1){
		lSize=sizeof(num_joints);
		fseek ( tfile, lSize, SEEK_CUR);
		//joints = new Joint[num_joints];
		//jointlabels = new int[num_joints];
		for(i=0;i<num_joints;i++){
			lSize=sizeof(XYZ);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			if(joints[i].hasparent)joints[i].parent=&joints[parentID];
			joints[i].velocity=0;
			joints[i].oldposition=joints[i].position;
		}
		funpackf(tfile, "Bi", &num_muscles);
		//muscles = new Muscle[num_muscles];
		for(i=0;i<num_muscles;i++){
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			tempmuscle=muscles[i].numverticeslow;
			funpackf(tfile, "Bi", &muscles[i].numverticeslow);
			if(muscles[i].numverticeslow){
				//muscles[i].verticeslow.clear();
				//muscles[i].verticeslow.resize(muscles[i].numverticeslow);
				//if(muscles[i].verticeslow)dealloc2(muscles[i].verticeslow);
				muscles[i].verticeslow=(int*)malloc(sizeof(int)*muscles[i].numverticeslow);
				edit=0;
				for(j=0;j<muscles[i].numverticeslow-edit;j++){
					funpackf(tfile, "Bi", &muscles[i].verticeslow[j+edit]);
					if(muscles[i].verticeslow[j+edit]>=modellow.vertexNum){
						muscles[i].numverticeslow--;
						edit--;
					}
				}


			}
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			fseek ( tfile, lSize, SEEK_CUR);
		}
		lSize=sizeof(int);
		for(j=0;j<num_muscles;j++){
			for(i=0;i<muscles[j].numverticeslow;i++){
				if(muscles[j].numverticeslow&&muscles[j].verticeslow[i]<modellow.vertexNum)modellow.owner[muscles[j].verticeslow[i]]=j;
			}
		}
		/*FindForwards();
		for(i=0;i<num_joints;i++){
		joints[i].startpos=joints[i].position;
		}
		for(i=0;i<num_muscles;i++){
		FindRotationMuscle(i,-1);
		}*/
		for(i=0;i<modellow.vertexNum;i++){
			modellow.vertex[i]=modellow.vertex[i]-(muscles[modellow.owner[i]].parent1->position+muscles[modellow.owner[i]].parent2->position)/2;
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();
				glLoadIdentity();
				glRotatef(muscles[modellow.owner[i]].rotate3,0,1,0);
				glRotatef(muscles[modellow.owner[i]].rotate2-90,0,0,1);
				glRotatef(muscles[modellow.owner[i]].rotate1-90,0,1,0);
				glTranslatef(modellow.vertex[i].x,modellow.vertex[i].y,modellow.vertex[i].z);
				glGetFloatv(GL_MODELVIEW_MATRIX,M);
				modellow.vertex[i].x=M[12];
				modellow.vertex[i].y=M[13];
				modellow.vertex[i].z=M[14];
			glPopMatrix();
		}
		modellow.CalculateNormals(0);
	}

	if(clothes){
		tfile=fopen( ConvertFileName(clothesfilename), "rb" );
		lSize=sizeof(num_joints);
		fseek ( tfile, lSize, SEEK_CUR);
		//joints = new Joint[num_joints];
		//jointlabels = new int[num_joints];
		for(i=0;i<num_joints;i++){
			lSize=sizeof(XYZ);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			if(joints[i].hasparent)joints[i].parent=&joints[parentID];
			joints[i].velocity=0;
			joints[i].oldposition=joints[i].position;
		}
		funpackf(tfile, "Bi", &num_muscles);
		//muscles = new Muscle[num_muscles];
		for(i=0;i<num_muscles;i++){
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(float);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			tempmuscle=muscles[i].numverticesclothes;
			funpackf(tfile, "Bi", &muscles[i].numverticesclothes);
			if(muscles[i].numverticesclothes){
				//muscles[i].verticesclothes.clear();
				//muscles[i].verticesclothes.resize(muscles[i].numverticesclothes);
				//if(muscles[i].verticesclothes)dealloc2(muscles[i].verticesclothes);
				muscles[i].verticesclothes=(int*)malloc(sizeof(int)*muscles[i].numverticesclothes);
				edit=0;
				for(j=0;j<muscles[i].numverticesclothes-edit;j++){
					funpackf(tfile, "Bi", &muscles[i].verticesclothes[j+edit]);
					if(muscles[i].verticesclothes[j+edit]>=modelclothes.vertexNum){
						muscles[i].numverticesclothes--;
						edit--;
					}
				}
			}
			lSize=1;//sizeof(bool);
			fseek ( tfile, lSize, SEEK_CUR);
			lSize=sizeof(int);
			fseek ( tfile, lSize, SEEK_CUR);
			fseek ( tfile, lSize, SEEK_CUR);
		}
		lSize=sizeof(int);
		for(j=0;j<num_muscles;j++){
			for(i=0;i<muscles[j].numverticesclothes;i++){
				if(muscles[j].numverticesclothes&&muscles[j].verticesclothes[i]<modelclothes.vertexNum)modelclothes.owner[muscles[j].verticesclothes[i]]=j;
			}
		}
		/*FindForwards();
		for(i=0;i<num_joints;i++){
		joints[i].startpos=joints[i].position;
		}
		for(i=0;i<num_muscles;i++){
		FindRotationMuscle(i,-1);
		}*/
		for(i=0;i<modelclothes.vertexNum;i++){
			modelclothes.vertex[i]=modelclothes.vertex[i]-(muscles[modelclothes.owner[i]].parent1->position+muscles[modelclothes.owner[i]].parent2->position)/2;
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();
				glLoadIdentity();
				glRotatef(muscles[modelclothes.owner[i]].rotate3,0,1,0);
				glRotatef(muscles[modelclothes.owner[i]].rotate2-90,0,0,1);
				glRotatef(muscles[modelclothes.owner[i]].rotate1-90,0,1,0);
				glTranslatef(modelclothes.vertex[i].x,modelclothes.vertex[i].y,modelclothes.vertex[i].z);
				glGetFloatv(GL_MODELVIEW_MATRIX,M);
				modelclothes.vertex[i].x=M[12];
				modelclothes.vertex[i].y=M[13];
				modelclothes.vertex[i].z=M[14];
			glPopMatrix();
		}
		modelclothes.CalculateNormals(0);
	}
	fclose(tfile);

	for(i=0;i<num_joints;i++){
		for(j=0;j<num_joints;j++){
			if(joints[i].label==j)jointlabels[j]=i;
		}
	}

	free=0;
}

Animation::Animation()
{
	numframes = 0;
	height = 0;
	attack = 0;
	joints = 0;
	weapontargetnum = 0;

	position=0;
	twist=0;
	twist2=0;
	speed=0;
	onground=0;
	forward=0;
	label=0;
	weapontarget=0;
}

Animation::~Animation()
{
	deallocate();
}

void Animation::deallocate()
{
	int i = 0;

	if(position)
	{
		for(i = 0; i < joints; i++)
			dealloc2(position[i]);

		dealloc2(position);
	}
	position = 0;

	if(twist)
	{
		for(i = 0; i < joints; i++)
			dealloc2(twist[i]);

		dealloc2(twist);
	}
	twist = 0;

	if(twist2)
	{
		for(i = 0; i < joints; i++)
			dealloc2(twist2[i]);

		dealloc2(twist2);
	}
	twist2 = 0;

	if(onground)
	{
		for(i = 0; i < joints; i++)
			dealloc2(onground[i]);

		dealloc2(onground);
	}
	onground = 0;

	if(speed)dealloc2(speed);
	speed = 0;

	if(forward)dealloc2(forward);
	forward = 0;

	if(weapontarget)dealloc2(weapontarget);
	weapontarget = 0;

	if(label)dealloc2(label);
	label = 0;

	joints = 0;
}

Skeleton::Skeleton()
{
	num_joints = 0;

	num_muscles = 0;

	selected = 0;

	memset(forwardjoints, 0, sizeof(forwardjoints));
	//		XYZ forward;

	id = 0;

	memset(lowforwardjoints, 0, sizeof(lowforwardjoints));
	//		XYZ lowforward;

	//		XYZ specialforward[5];
	memset(jointlabels, 0, sizeof(jointlabels));

	//		Model model[7];
	//		Model modellow;
	//		Model modelclothes;
	num_models = 0;

	//		Model drawmodel;
	//		Model drawmodellow;
	//		Model drawmodelclothes;

	clothes = 0;
	spinny = 0;

	memset(skinText, 0, sizeof(skinText));
	skinsize = 0;

	checkdelay = 0;

	longdead = 0;
	broken = 0;

	free = 0;
	oldfree = 0;
	freetime = 0;
	freefall = 0;

	joints=0;
	muscles=0;
}

Skeleton::~Skeleton()
{
	if (muscles)
	{
		delete [] muscles;
	}
	muscles = 0;

	if (joints)
	{
		delete [] joints;
	}
	joints = 0;
}

Muscle::Muscle()
{
	vertices=0;
	verticeslow=0;
	verticesclothes=0;

	numvertices = 0;
	numverticeslow = 0;
	numverticesclothes = 0;
	length = 0;
	targetlength = 0;
	parent1 = 0;
	parent2 = 0;
	maxlength = 0;
	minlength = 0;
	type = 0;
	visible = 0;
	rotate1 = 0,rotate2 = 0,rotate3 = 0;
	lastrotate1 = 0,lastrotate2 = 0,lastrotate3 = 0;
	oldrotate1 = 0,oldrotate2 = 0,oldrotate3 = 0;
	newrotate1 = 0,newrotate2 = 0,newrotate3 = 0;

	strength = 0;
}

Muscle::~Muscle()
{
	dealloc2(vertices);
	dealloc2(verticeslow);
	dealloc2(verticesclothes);
}

Animation & Animation::operator = (const Animation & ani)
{
	int i = 0;

	bool allocate = true;

	allocate = ((ani.numframes != numframes) || (ani.joints != joints));

	if (allocate) deallocate();

	numframes = ani.numframes;
	height = ani.height;
	attack = ani.attack;
	joints = ani.joints;
	weapontargetnum = ani.weapontargetnum;

	if (allocate) position=(XYZ**)malloc(sizeof(XYZ*)*ani.joints);
	for(i = 0; i < ani.joints; i++)
	{
		if (allocate) position[i] = (XYZ*)malloc(sizeof(XYZ)*ani.numframes);
		memcpy(position[i], ani.position[i], sizeof(XYZ)*ani.numframes);
	}

	if (allocate) twist=(float**)malloc(sizeof(float*)*ani.joints);
	for(i = 0; i < ani.joints; i++)
	{
		if (allocate) twist[i] = (float*)malloc(sizeof(float)*ani.numframes);
		memcpy(twist[i], ani.twist[i], sizeof(float)*ani.numframes);
	}

	if (allocate) twist2=(float**)malloc(sizeof(float*)*ani.joints);
	for(i = 0; i < ani.joints; i++)
	{
		if (allocate) twist2[i] = (float*)malloc(sizeof(float)*ani.numframes);
		memcpy(twist2[i], ani.twist2[i], sizeof(float)*ani.numframes);
	}

	if (allocate) speed = (float*)malloc(sizeof(float)*ani.numframes);
	memcpy(speed, ani.speed, sizeof(float)*ani.numframes);

	if (allocate) onground=(bool**)malloc(sizeof(bool*)*ani.joints);
	for(i = 0; i < ani.joints; i++)
	{
		if (allocate) onground[i] =(bool*)malloc(sizeof(bool)*ani.numframes);
		memcpy(onground[i], ani.onground[i], sizeof(bool)*ani.numframes);
	}

	if (allocate) forward = (XYZ*)malloc(sizeof(XYZ)*ani.numframes);
	memcpy(forward, ani.forward, sizeof(XYZ)*ani.numframes);

	if (allocate) weapontarget = (XYZ*)malloc(sizeof(XYZ)*ani.numframes);
	memcpy(weapontarget, ani.weapontarget, sizeof(XYZ)*ani.numframes);

	if (allocate) label = (int*)malloc(sizeof(int)*ani.numframes);
	memcpy(label, ani.label, sizeof(int)*ani.numframes);

	return (*this);
}

