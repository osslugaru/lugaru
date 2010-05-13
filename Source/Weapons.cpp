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
#include "Weapons.h"
#include "openal_wrapper.h"

extern float multiplier;
extern Animation animation[animation_count];
extern OPENAL_SAMPLE	*samp[100];
extern int channels[100];
extern Terrain terrain;
extern float gravity;
extern int environment;
extern Sprites sprites;
extern int detail;
extern FRUSTUM frustum;
extern XYZ viewer;
extern float realmultiplier;
extern int slomo;
extern float slomodelay;
extern bool cellophane;
extern float texdetail;
extern GLubyte bloodText[512*512*3];
extern int bloodtoggle;
extern Objects objects;
extern bool osx;
extern bool autoslomo;
extern float camerashake;
extern float woozy;
extern float terraindetail;
extern float viewdistance;
extern float blackout;
extern int difficulty;
extern Person player[maxplayers];
extern int numplayers;
extern bool freeze;
extern int bonus;
extern float bonusvalue;
extern float bonustotal;
extern float bonustime;
extern int tutoriallevel;
extern int numthrowkill;
extern "C"	void PlaySoundEx(int channel, OPENAL_SAMPLE *sptr, OPENAL_DSPUNIT *dsp, signed char startpaused);

void	Weapons::DoStuff(){
	static int i,whichpatchx,whichpatchz,j,k,whichhit,m;
	static XYZ start,end,colpoint,normalrot,footvel,footpoint;
	static XYZ terrainnormal;
	static XYZ vel;
	static XYZ midp;
	static XYZ newpoint1,newpoint2;
	static float friction=3.5;
	static float elasticity=.4;
	static XYZ bounceness;
	static float frictionness;
	static float moveamount;
	int closestline;
	static float closestdistance;
	static float distance;
	static XYZ point[3];
	static XYZ closestpoint;
	static XYZ closestswordpoint;
	static XYZ extramove;
	static float proportion;
	static float tempmult;

	//Move

	for(i=0;i<numweapons;i++){
		if(owner[i]!=-1){
			oldowner[i]=owner[i];
		}
		if(damage[i]>=2&&type[i]==staff&&owner[i]!=-1){
			float gLoc[3];
			float vel[3];
			gLoc[0]=tippoint[i].x;
			gLoc[1]=tippoint[i].y;
			gLoc[2]=tippoint[i].z;
			vel[0]=0;
			vel[1]=0;
			vel[2]=0;
			PlaySoundEx( staffbreaksound, samp[staffbreaksound], NULL, true);
			OPENAL_3D_SetAttributes(channels[staffbreaksound], gLoc, vel);
			OPENAL_SetVolume(channels[staffbreaksound], 256);
			OPENAL_SetPaused(channels[staffbreaksound], false);
			XYZ tempvel;
			XYZ speed;
			//speed=(tippoint[i]-oldtippoint[i])/multiplier/6;
			speed=0;
			/*for(j=0;j<10;j++){
			tempvel.x=float(abs(Random()%100)-50)/20;
			tempvel.y=float(abs(Random()%100)-50)/20;
			tempvel.z=float(abs(Random()%100)-50)/20;
			tempvel+=speed;
			sprites.MakeSprite(cloudimpactsprite, position[i]+(tippoint[i]-position[i])*((float)j-2)/8,tempvel*.5, 115/255,73/255,12/255, .15+float(abs(Random()%100)-50)/1000, .7);
			}*/
			for(j=0;j<40;j++){
				tempvel.x=float(abs(Random()%100)-50)/20;
				tempvel.y=float(abs(Random()%100)-50)/20;
				tempvel.z=float(abs(Random()%100)-50)/20;
				tempvel+=speed;
				sprites.MakeSprite(splintersprite, position[i]+(tippoint[i]-position[i])*((float)j-8)/32,tempvel*.5, 115/255,73/255,12/255, .1, 1);
			}
			int tempowner;
			tempowner=owner[i];
			owner[i]=-1;
			hitsomething[i]=0;
			missed[i]=1;
			freetime[i]=0;
			firstfree[i]=1;
			position[i]=0;
			physics[i]=0;
			if(tempowner!=-1){
				player[tempowner].num_weapons--;
				if(player[tempowner].num_weapons){
					player[tempowner].weaponids[0]=player[tempowner].weaponids[player[tempowner].num_weapons];
					if(player[tempowner].weaponstuck==player[tempowner].num_weapons)player[tempowner].weaponstuck=0;
				}
				player[tempowner].weaponactive=-1;
			}
		}
		oldposition[i]=position[i];
		oldtippoint[i]=tippoint[i];
		if(owner[i]==-1&&(velocity[i].x||velocity[i].y||velocity[i].z)&&!physics[i]){
			position[i]+=velocity[i]*multiplier;
			tippoint[i]+=velocity[i]*multiplier;
			whichpatchx=position[i].x/(terrain.size/subdivision*terrain.scale*terraindetail);
			whichpatchz=position[i].z/(terrain.size/subdivision*terrain.scale*terraindetail);
			if(whichpatchx>0&&whichpatchz>0&&whichpatchx<subdivision&&whichpatchz<subdivision)
				if(terrain.patchobjectnum[whichpatchx][whichpatchz]){
					for(j=0;j<terrain.patchobjectnum[whichpatchx][whichpatchz];j++){
						k=terrain.patchobjects[whichpatchx][whichpatchz][j];
						start=oldtippoint[i];
						end=tippoint[i];
						whichhit=objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k]);
						if(whichhit!=-1){
							if(objects.type[k]==treetrunktype){
								objects.model[k].MakeDecal(breakdecal,DoRotation(colpoint-objects.position[k],0,-objects.rotation[k],0),.1,1,Random()%360);
								normalrot=DoRotation(objects.model[k].facenormals[whichhit],0,objects.rotation[k],0);
								velocity[i]=0;
								if(type[i]==knife)position[i]=colpoint-normalrot*.1;
								if(type[i]==sword)position[i]=colpoint-normalrot*.2;
								if(type[i]==staff)position[i]=colpoint-normalrot*.2;
								XYZ temppoint1,temppoint2,tempforward;
								float distance;

								temppoint1=0;
								temppoint2=normalrot;
								distance=findDistance(&temppoint1,&temppoint2);
								rotation2[i]=asin((temppoint1.y-temppoint2.y)/distance);
								rotation2[i]*=360/6.28;
								temppoint1.y=0;
								temppoint2.y=0;
								rotation1[i]=acos((temppoint1.z-temppoint2.z)/findDistance(&temppoint1,&temppoint2));
								rotation1[i]*=360/6.28;
								if(temppoint1.x>temppoint2.x)rotation1[i]=360-rotation1[i];

								rotation3[i]=0;
								smallrotation[i]=90;
								smallrotation2[i]=0;
								bigtilt[i]=0;
								bigtilt2[i]=0;
								bigrotation[i]=0;

								float gLoc[3];
								float vel[3];
								gLoc[0]=position[i].x;
								gLoc[1]=position[i].y;
								gLoc[2]=position[i].z;
								vel[0]=0;
								vel[1]=0;
								vel[2]=0;
								PlaySoundEx( knifesheathesound, samp[knifesheathesound], NULL, true);
								OPENAL_3D_SetAttributes(channels[knifesheathesound], gLoc, vel);
								OPENAL_SetVolume(channels[knifesheathesound], 128);
								OPENAL_SetPaused(channels[knifesheathesound], false);

								bloody[i]=0;

								sprites.MakeSprite(cloudimpactsprite, position[i],velocity[i], 1,1,1, .8, .3);
							}
							else {
								physics[i]=1;
								firstfree[i]=1;
								position[i]-=velocity[i]*multiplier;
								tippoint[i]-=velocity[i]*multiplier;
								tipvelocity[i]=velocity[i];
							}
						}
					}
				}
				if(velocity[i].x||velocity[i].y||velocity[i].z)
					for(j=0;j<numplayers;j++){
						footvel=0;
						footpoint=DoRotation((player[j].skeleton.joints[player[j].skeleton.jointlabels[abdomen]].position+player[j].skeleton.joints[player[j].skeleton.jointlabels[neck]].position)/2,0,player[j].rotation,0)*player[j].scale+player[j].coords;
						if(owner[i]==-1&&findDistancefastflat(&position[i],&player[j].coords)<1.5&&findDistancefast(&position[i],&player[j].coords)<4&&player[j].weaponstuck==-1&&!player[j].skeleton.free&&j!=oldowner[i]){
							if((player[j].aitype!=attacktypecutoff||abs(Random()%6)==0||(player[j].targetanimation!=backhandspringanim&&player[j].targetanimation!=rollanim&&player[j].targetanimation!=flipanim&&Random()%2==0))&&!missed[i]){
								bool caught=0;
								if((player[j].creature==wolftype&&Random()%3!=0&&player[j].weaponactive==-1&&(player[j].isIdle()||player[j].isRun()||player[j].targetanimation==walkanim))||(player[j].creature==rabbittype&&Random()%2==0&&player[j].aitype==attacktypecutoff&&player[j].weaponactive==-1)){
									float gLoc[3];
									float vel[3];
									gLoc[0]=player[j].coords.x;
									gLoc[1]=player[j].coords.y;
									gLoc[2]=player[j].coords.z;
									vel[0]=player[j].velocity.x;
									vel[1]=player[j].velocity.y;
									vel[2]=player[j].velocity.z;
									PlaySoundEx( knifedrawsound, samp[knifedrawsound], NULL, true);
									OPENAL_3D_SetAttributes(channels[knifedrawsound], gLoc, vel);
									OPENAL_SetVolume(channels[knifedrawsound], 128);
									OPENAL_SetPaused(channels[knifedrawsound], false);

									player[j].weaponactive=0;
									player[j].targetanimation=removeknifeanim;
									player[j].targetframe=1;
									player[j].target=1;
									owner[i]=player[j].id;
									if(player[j].num_weapons>0){
										player[j].weaponids[player[j].num_weapons]=player[j].weaponids[0];
									}
									player[j].num_weapons++;
									player[j].weaponids[0]=i;

									player[j].aitype=attacktypecutoff;
								}
								else {
									if(j!=0)numthrowkill++;
									player[j].num_weapons++;
									player[j].weaponstuck=player[j].num_weapons-1;
									if(normaldotproduct(player[j].facing,velocity[i])>0)player[j].weaponstuckwhere=1;
									else player[j].weaponstuckwhere=0;

									player[j].weaponids[player[j].num_weapons-1]=i;

									player[j].RagDoll(0);
									player[j].skeleton.joints[player[j].skeleton.jointlabels[abdomen]].velocity+=velocity[i]*2;
									player[j].skeleton.joints[player[j].skeleton.jointlabels[neck]].velocity+=velocity[i]*2;
									player[j].skeleton.joints[player[j].skeleton.jointlabels[rightshoulder]].velocity+=velocity[i]*2;
									player[j].skeleton.joints[player[j].skeleton.jointlabels[leftshoulder]].velocity+=velocity[i]*2;
									//player[j].Puff(abdomen);
									if(bloodtoggle&&tutoriallevel!=1)sprites.MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .8, .3);
									if(tutoriallevel==1)sprites.MakeSprite(cloudimpactsprite, footpoint,footvel, 1,1,1, .8, .3);
									footvel=tippoint[i]-position[i];
									Normalise(&footvel);
									if(bloodtoggle&&tutoriallevel!=1)sprites.MakeSprite(bloodflamesprite, footpoint,footvel*-1, 1,0,0, .6, 1);

									if(tutoriallevel!=1){
										if(player[j].weaponstuckwhere==0)player[j].DoBloodBig(2,205);
										if(player[j].weaponstuckwhere==1)player[j].DoBloodBig(2,200);
										player[j].damage+=200/player[j].armorhigh;
										player[j].deathbleeding=1;
										player[j].bloodloss+=(200+abs((float)(Random()%40))-20)/player[j].armorhigh;
										owner[i]=j;
										bloody[i]=2;
										blooddrip[i]=5;
									}

									float gLoc[3];
									float vel[3];
									gLoc[0]=position[i].x;
									gLoc[1]=position[i].y;
									gLoc[2]=position[i].z;
									vel[0]=0;
									vel[1]=0;
									vel[2]=0;
									PlaySoundEx( fleshstabsound, samp[fleshstabsound], NULL, true);
									OPENAL_3D_SetAttributes(channels[fleshstabsound], gLoc, vel);
									OPENAL_SetVolume(channels[fleshstabsound], 128);
									OPENAL_SetPaused(channels[fleshstabsound], false);

									if(animation[player[0].targetanimation].height==highheight){
										bonus=ninja;
										bonustime=0;
										bonusvalue=60;
									}
									else{
										bonus=Bullseyebonus;
										bonustime=0;
										bonusvalue=30;
									}
								}
							}
							else missed[i]=1;
						}
					}
					if(position[i].y<terrain.getHeight(position[i].x,position[i].z)){
						if(terrain.getOpacity(position[i].x,position[i].z)<.2){
							velocity[i]=0;
							if(terrain.lineTerrain(oldposition[i],position[i],&colpoint)!=-1){
								position[i]=colpoint*terrain.scale;
							}
							else position[i].y=terrain.getHeight(position[i].x,position[i].z);

							terrain.MakeDecal(shadowdecalpermanent,position[i],.06,.5,0);
							normalrot=terrain.getNormal(position[i].x,position[i].z)*-1;
							velocity[i]=0;
							//position[i]-=normalrot*.1;
							glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
							glPushMatrix();
								GLfloat M[16];
								glLoadIdentity();
								glRotatef(bigrotation[i],0,1,0);
								glRotatef(bigtilt2[i],1,0,0);
								glRotatef(bigtilt[i],0,0,1);
								glRotatef(-rotation1[i]+90,0,1,0);
								glRotatef(-rotation2[i]+90,0,0,1);
								glRotatef(-rotation3[i],0,1,0);
								glRotatef(smallrotation[i],1,0,0);
								glRotatef(smallrotation2[i],0,1,0);
								glTranslatef(0,0,1);
								glGetFloatv(GL_MODELVIEW_MATRIX,M);
								tippoint[i].x=M[12];
								tippoint[i].y=M[13];
								tippoint[i].z=M[14];
							glPopMatrix();
							position[i]-=tippoint[i]*.15;
							XYZ temppoint1,temppoint2,tempforward;
							float distance;

							rotation3[i]=0;
							smallrotation[i]=90;
							smallrotation2[i]=0;
							bigtilt[i]=0;
							bigtilt2[i]=0;
							bigrotation[i]=0;

							float gLoc[3];
							float vel[3];
							gLoc[0]=position[i].x;
							gLoc[1]=position[i].y;
							gLoc[2]=position[i].z;
							vel[0]=0;
							vel[1]=0;
							vel[2]=0;
							PlaySoundEx( knifesheathesound, samp[knifesheathesound], NULL, true);
							OPENAL_3D_SetAttributes(channels[knifesheathesound], gLoc, vel);
							OPENAL_SetVolume(channels[knifesheathesound], 128);
							OPENAL_SetPaused(channels[knifesheathesound], false);

							XYZ terrainlight;
							terrainlight=terrain.getLighting(position[i].x,position[i].z);
							if(environment==snowyenvironment){
								if(findDistancefast(&position[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, position[i],velocity[i], terrainlight.x,terrainlight.y,terrainlight.z, .5, .7);
							}
							else if(environment==grassyenvironment){
								if(findDistancefast(&position[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, position[i],velocity[i], terrainlight.x*90/255,terrainlight.y*70/255,terrainlight.z*8/255, .5, .5);
							}
							else if(environment==desertenvironment){
								if(findDistancefast(&position[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, position[i],velocity[i], terrainlight.x*190/255,terrainlight.y*170/255,terrainlight.z*108/255, .5, .7);
							}

							bloody[i]=0;
						}
						else {
							physics[i]=1;
							firstfree[i]=1;
							position[i]-=velocity[i]*multiplier;
							tippoint[i]-=velocity[i]*multiplier;
							tipvelocity[i]=velocity[i];
						}
					}
					if(velocity[i].x!=0||velocity[i].z!=0||velocity[i].y!=0){
						velocity[i].y+=gravity*multiplier;

						XYZ temppoint1,temppoint2,tempforward;
						float distance;

						temppoint1=0;
						temppoint2=velocity[i];
						distance=findDistance(&temppoint1,&temppoint2);
						rotation2[i]=asin((temppoint1.y-temppoint2.y)/distance);
						rotation2[i]*=360/6.28;
						temppoint1.y=0;
						temppoint2.y=0;
						rotation1[i]=acos((temppoint1.z-temppoint2.z)/findDistance(&temppoint1,&temppoint2));
						rotation1[i]*=360/6.28;
						rotation3[i]=0;
						smallrotation[i]=90;
						smallrotation2[i]=0;
						bigtilt[i]=0;
						bigtilt2[i]=0;
						bigrotation[i]=0;
						if(temppoint1.x>temppoint2.x)rotation1[i]=360-rotation1[i];
					}
		}
		//Sword physics
		XYZ mid;
		XYZ oldmid;
		XYZ oldmid2;

		tempmult=multiplier;
		multiplier/=10;
		for(int l=0;l<10;l++){
			if(owner[i]==-1&&(velocity[i].x||velocity[i].y||velocity[i].z)&&physics[i]){
				//move
				position[i]+=velocity[i]*multiplier;
				tippoint[i]+=tipvelocity[i]*multiplier;

				//Length constrain
				midp=(position[i]*mass[i]+tippoint[i]*tipmass[i])/(mass[i]+tipmass[i]);
				vel=tippoint[i]-midp;
				Normalise(&vel);
				newpoint1=midp-vel*length[i]*(tipmass[i]/(mass[i]+tipmass[i]));
				newpoint2=midp+vel*length[i]*(mass[i]/(mass[i]+tipmass[i]));
				if(!freeze){
					if(freetime[i]>.04)velocity[i]=velocity[i]+(newpoint1-position[i])/multiplier;
					if(freetime[i]>.04)tipvelocity[i]=tipvelocity[i]+(newpoint2-tippoint[i])/multiplier;
				}
				position[i]=newpoint1;
				tippoint[i]=newpoint2;


				//Object collisions
				whichpatchx=(position[i].x)/(terrain.size/subdivision*terrain.scale*terraindetail);
				whichpatchz=(position[i].z)/(terrain.size/subdivision*terrain.scale*terraindetail);
				if(whichpatchx>0&&whichpatchz>0&&whichpatchx<subdivision&&whichpatchz<subdivision)
					if(terrain.patchobjectnum[whichpatchx][whichpatchz]){
						for(j=0;j<terrain.patchobjectnum[whichpatchx][whichpatchz];j++){
							k=terrain.patchobjects[whichpatchx][whichpatchz][j];

							if(firstfree[i]){
								if(type[i]!=staff){
									start=position[i]-(tippoint[i]-position[i])/5;
									end=tippoint[i]+(tippoint[i]-position[i])/30;
									whichhit=objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k]);
									if(whichhit!=-1){
										XYZ diff;
										diff=(colpoint-tippoint[i]);
										Normalise(&diff);
										hitsomething[i]=1;

										position[i]+=(colpoint-tippoint[i])+diff*.05;
										tippoint[i]=colpoint+diff*.05;
										oldposition[i]=position[i];
										oldtippoint[i]=tippoint[i];
									}
								}
								if(type[i]==staff){
									start=tippoint[i]-(position[i]-tippoint[i])/5;
									end=position[i]+(position[i]-tippoint[i])/30;
									whichhit=objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k]);
									if(whichhit!=-1){
										XYZ diff;
										diff=(colpoint-position[i]);
										Normalise(&diff);
										hitsomething[i]=1;

										tippoint[i]+=(colpoint-position[i])+diff*.05;
										position[i]=colpoint+diff*.05;
										oldtippoint[i]=tippoint[i];
										oldposition[i]=tippoint[i];
									}
								}
							}

							start=oldposition[i];
							end=position[i];
							whichhit=objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k]);
							if(whichhit!=-1){
								hitsomething[i]=1;
								position[i]=colpoint;
								terrainnormal=DoRotation(objects.model[k].facenormals[whichhit],0,objects.rotation[k],0)*-1;
								ReflectVector(&velocity[i],&terrainnormal);
								position[i]+=terrainnormal*.002;

								bounceness=terrainnormal*findLength(&velocity[i])*(abs(normaldotproduct(velocity[i],terrainnormal)));
								if(findLengthfast(&velocity[i])<findLengthfast(&bounceness))bounceness=0;
								frictionness=abs(normaldotproduct(velocity[i],terrainnormal));
								velocity[i]-=bounceness;
								if(1-friction*frictionness>0)velocity[i]*=1-friction*frictionness;
								else velocity[i]=0;
								velocity[i]+=bounceness*elasticity;

								if(findLengthfast(&bounceness)>1){
									float gLoc[3];
									float vel[3];
									//int whichsound=clank1sound+abs(Random()%4);
									int whichsound;
									if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
									if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);gLoc[0]=position[i].x;
									gLoc[1]=position[i].y;
									gLoc[2]=position[i].z;
									vel[0]=0;
									vel[1]=0;
									vel[2]=0;
									PlaySoundEx( whichsound, samp[whichsound], NULL, true);
									OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
									OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
									OPENAL_SetPaused(channels[whichsound], false);
								}
							}
							start=oldtippoint[i];
							end=tippoint[i];
							whichhit=objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k]);
							if(whichhit!=-1){
								hitsomething[i]=1;
								tippoint[i]=colpoint;
								terrainnormal=DoRotation(objects.model[k].facenormals[whichhit],0,objects.rotation[k],0)*-1;
								ReflectVector(&tipvelocity[i],&terrainnormal);
								tippoint[i]+=terrainnormal*.002;

								bounceness=terrainnormal*findLength(&tipvelocity[i])*(abs(normaldotproduct(tipvelocity[i],terrainnormal)));
								if(findLengthfast(&tipvelocity[i])<findLengthfast(&bounceness))bounceness=0;
								frictionness=abs(normaldotproduct(tipvelocity[i],terrainnormal));
								tipvelocity[i]-=bounceness;
								if(1-friction*frictionness>0)tipvelocity[i]*=1-friction*frictionness;
								else tipvelocity[i]=0;
								tipvelocity[i]+=bounceness*elasticity;

								if(findLengthfast(&bounceness)>1){
									float gLoc[3];
									float vel[3];
									//int whichsound=clank1sound+abs(Random()%4);
									int whichsound;
									if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
									if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);gLoc[0]=position[i].x;
									gLoc[0]=position[i].x;
									gLoc[1]=position[i].y;
									gLoc[2]=position[i].z;
									vel[0]=0;
									vel[1]=0;
									vel[2]=0;
									PlaySoundEx( whichsound, samp[whichsound], NULL, true);
									OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
									OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
									OPENAL_SetPaused(channels[whichsound], false);
								}
							}

							if((objects.type[k]!=boxtype&&objects.type[k]!=platformtype&&objects.type[k]!=walltype&&objects.type[k]!=weirdtype)||objects.rotation2[k]!=0)
								for(m=0;m<2;m++){
									mid=(position[i]*(21+(float)m*10)+tippoint[i]*(19-(float)m*10))/40;
									oldmid2=mid;
									oldmid=(oldposition[i]*(21+(float)m*10)+oldtippoint[i]*(19-(float)m*10))/40;

									start=oldmid;
									end=mid;
									whichhit=objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k]);
									if(whichhit!=-1){
										hitsomething[i]=1;
										mid=colpoint;
										terrainnormal=DoRotation(objects.model[k].facenormals[whichhit],0,objects.rotation[k],0)*-1;
										ReflectVector(&velocity[i],&terrainnormal);

										bounceness=terrainnormal*findLength(&velocity[i])*(abs(normaldotproduct(velocity[i],terrainnormal)));
										if(findLengthfast(&velocity[i])<findLengthfast(&bounceness))bounceness=0;
										frictionness=abs(normaldotproduct(velocity[i],terrainnormal));
										velocity[i]-=bounceness;
										if(1-friction*frictionness>0)velocity[i]*=1-friction*frictionness;
										else velocity[i]=0;
										velocity[i]+=bounceness*elasticity;

										if(findLengthfast(&bounceness)>1){
											float gLoc[3];
											float vel[3];
											//int whichsound=clank1sound+abs(Random()%4);
											int whichsound;
											if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
											if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);gLoc[0]=mid.x;
											gLoc[1]=mid.y;
											gLoc[2]=mid.z;
											vel[0]=0;
											vel[1]=0;
											vel[2]=0;
											PlaySoundEx( whichsound, samp[whichsound], NULL, true);
											OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
											OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
											OPENAL_SetPaused(channels[whichsound], false);
										}
										position[i]+=(mid-oldmid2)*(20/(1+(float)m*10));
									}

									mid=(position[i]*(19-(float)m*10)+tippoint[i]*(21+(float)m*10))/40;
									oldmid2=mid;
									oldmid=(oldposition[i]*(19-(float)m*10)+oldtippoint[i]*(21+(float)m*10))/40;

									start=oldmid;
									end=mid;
									whichhit=objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k]);
									if(whichhit!=-1){
										hitsomething[i]=1;
										mid=colpoint;
										terrainnormal=DoRotation(objects.model[k].facenormals[whichhit],0,objects.rotation[k],0)*-1;
										ReflectVector(&tipvelocity[i],&terrainnormal);

										bounceness=terrainnormal*findLength(&tipvelocity[i])*(abs(normaldotproduct(tipvelocity[i],terrainnormal)));
										if(findLengthfast(&tipvelocity[i])<findLengthfast(&bounceness))bounceness=0;
										frictionness=abs(normaldotproduct(tipvelocity[i],terrainnormal));
										tipvelocity[i]-=bounceness;
										if(1-friction*frictionness>0)tipvelocity[i]*=1-friction*frictionness;
										else tipvelocity[i]=0;
										tipvelocity[i]+=bounceness*elasticity;

										if(findLengthfast(&bounceness)>1){
											float gLoc[3];
											float vel[3];
											//int whichsound=clank1sound+abs(Random()%4);
											int whichsound;
											if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
											if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);gLoc[0]=mid.x;
											gLoc[1]=mid.y;
											gLoc[2]=mid.z;
											vel[0]=0;
											vel[1]=0;
											vel[2]=0;
											PlaySoundEx( whichsound, samp[whichsound], NULL, true);
											OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
											OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
											OPENAL_SetPaused(channels[whichsound], false);
										}
										tippoint[i]+=(mid-oldmid2)*(20/(1+(float)m*10));
									}
								}
							else
							{
								start=position[i];
								end=tippoint[i];
								whichhit=objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k]);
								if(whichhit!=-1){
									hitsomething[i]=1;
									closestdistance=-1;
									closestswordpoint=colpoint;//(position[i]+tippoint[i])/2;
									point[0]=DoRotation(objects.model[k].vertex[objects.model[k].Triangles[whichhit].vertex[0]],0,objects.rotation[k],0)+objects.position[k];
									point[1]=DoRotation(objects.model[k].vertex[objects.model[k].Triangles[whichhit].vertex[1]],0,objects.rotation[k],0)+objects.position[k];
									point[2]=DoRotation(objects.model[k].vertex[objects.model[k].Triangles[whichhit].vertex[2]],0,objects.rotation[k],0)+objects.position[k];
									if(DistancePointLine(&closestswordpoint, &point[0], &point[1], &distance,&colpoint ))
										if(distance<closestdistance||closestdistance==-1){
											closestpoint=colpoint;
											closestdistance=distance;
											closestline=0;
										}
										if(DistancePointLine(&closestswordpoint, &point[1], &point[2], &distance,&colpoint ))
											if(distance<closestdistance||closestdistance==-1){
												closestpoint=colpoint;
												closestdistance=distance;
												closestline=1;
											}
											if(DistancePointLine(&closestswordpoint, &point[2], &point[0], &distance,&colpoint ))
												if(distance<closestdistance||closestdistance==-1){
													closestpoint=colpoint;
													closestdistance=distance;
													closestline=2;
												}
												if(closestdistance!=-1&&isnormal(closestdistance)){
													if(DistancePointLine(&closestpoint, &position[i], &tippoint[i], &distance,&colpoint )){
														closestswordpoint=colpoint;
														velocity[i]+=(closestpoint-closestswordpoint);
														tipvelocity[i]+=(closestpoint-closestswordpoint);
														position[i]+=(closestpoint-closestswordpoint);
														tippoint[i]+=(closestpoint-closestswordpoint);
													}
												}
								}
							}

						}
					}
					//Terrain collisions
					whichhit=terrain.lineTerrain(oldposition[i],position[i],&colpoint);
					if(whichhit!=-1||position[i].y<terrain.getHeight(position[i].x,position[i].z)){
						hitsomething[i]=1;
						if(whichhit!=-1)position[i]=colpoint*terrain.scale;
						else position[i].y=terrain.getHeight(position[i].x,position[i].z);

						terrainnormal=terrain.getNormal(position[i].x,position[i].z);
						ReflectVector(&velocity[i],&terrainnormal);
						position[i]+=terrainnormal*.002;
						bounceness=terrainnormal*findLength(&velocity[i])*(abs(normaldotproduct(velocity[i],terrainnormal)));
						if(findLengthfast(&velocity[i])<findLengthfast(&bounceness))bounceness=0;
						frictionness=abs(normaldotproduct(velocity[i],terrainnormal));
						velocity[i]-=bounceness;
						if(1-friction*frictionness>0)velocity[i]*=1-friction*frictionness;
						else velocity[i]=0;
						if(terrain.getOpacity(position[i].x,position[i].z)<.2)velocity[i]+=bounceness*elasticity*.3;
						else velocity[i]+=bounceness*elasticity;
//if (type[i]==knife) printf("velocity of knife %d now %f,%f,%f.\n", i, velocity[i].x, velocity[i].y, velocity[i].z);
						if(findLengthfast(&bounceness)>1){
							float gLoc[3];
							float vel[3];
							int whichsound;
							if(terrain.getOpacity(position[i].x,position[i].z)>.2){
								if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
								if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);
							}
							else whichsound=footstepsound+abs(Random()%2);
							gLoc[0]=position[i].x;
							gLoc[1]=position[i].y;
							gLoc[2]=position[i].z;
							vel[0]=0;
							vel[1]=0;
							vel[2]=0;
							PlaySoundEx( whichsound, samp[whichsound], NULL, true);
							OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
							if(terrain.getOpacity(position[i].x,position[i].z)>.2)OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
							else OPENAL_SetVolume(channels[whichsound], 32*findLengthfast(&bounceness));
							OPENAL_SetPaused(channels[whichsound], false);

							if(terrain.getOpacity(position[i].x,position[i].z)<.2){
								XYZ terrainlight;
								terrainlight=terrain.getLighting(position[i].x,position[i].z);
								if(environment==snowyenvironment){
									if(findDistancefast(&position[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, position[i],velocity[i], terrainlight.x,terrainlight.y,terrainlight.z, .5, .7);
								}
								else if(environment==grassyenvironment){
									if(findDistancefast(&position[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, position[i],velocity[i], terrainlight.x*90/255,terrainlight.y*70/255,terrainlight.z*8/255, .5, .5);
								}
								else if(environment==desertenvironment){
									if(findDistancefast(&position[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, position[i],velocity[i], terrainlight.x*190/255,terrainlight.y*170/255,terrainlight.z*108/255, .5, .7);
								}
							}
						}
					}
					whichhit=terrain.lineTerrain(oldtippoint[i],tippoint[i],&colpoint);
					if(whichhit!=-1||tippoint[i].y<terrain.getHeight(tippoint[i].x,tippoint[i].z)){
						if(whichhit!=-1)tippoint[i]=colpoint*terrain.scale;
						else tippoint[i].y=terrain.getHeight(tippoint[i].x,tippoint[i].z);

						terrainnormal=terrain.getNormal(tippoint[i].x,tippoint[i].z);
						ReflectVector(&tipvelocity[i],&terrainnormal);
						tippoint[i]+=terrainnormal*.002;
						bounceness=terrainnormal*findLength(&tipvelocity[i])*(abs(normaldotproduct(tipvelocity[i],terrainnormal)));
						if(findLengthfast(&tipvelocity[i])<findLengthfast(&bounceness))bounceness=0;
						frictionness=abs(normaldotproduct(tipvelocity[i],terrainnormal));
						tipvelocity[i]-=bounceness;
						if(1-friction*frictionness>0)tipvelocity[i]*=1-friction*frictionness;
						else tipvelocity[i]=0;
						if(terrain.getOpacity(tippoint[i].x,tippoint[i].z)<.2)tipvelocity[i]+=bounceness*elasticity*.3;
						else tipvelocity[i]+=bounceness*elasticity;
//if (type[i]==knife) printf("tipvelocity of knife %d now %f,%f,%f.\n", i, tipvelocity[i].x, tipvelocity[i].y, tipvelocity[i].z);

						if(findLengthfast(&bounceness)>1){
							float gLoc[3];
							float vel[3];
							int whichsound;
							if(terrain.getOpacity(tippoint[i].x,tippoint[i].z)>.2){
								if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
								if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);
							}
							else whichsound=footstepsound+abs(Random()%2);
							gLoc[0]=tippoint[i].x;
							gLoc[1]=tippoint[i].y;
							gLoc[2]=tippoint[i].z;
							vel[0]=0;
							vel[1]=0;
							vel[2]=0;
							PlaySoundEx( whichsound, samp[whichsound], NULL, true);
							OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
							if(terrain.getOpacity(tippoint[i].x,tippoint[i].z)>.2)OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
							else OPENAL_SetVolume(channels[whichsound], 32*findLengthfast(&bounceness));
							OPENAL_SetPaused(channels[whichsound], false);

							if(terrain.getOpacity(tippoint[i].x,tippoint[i].z)<.2){
								XYZ terrainlight;
								terrainlight=terrain.getLighting(tippoint[i].x,tippoint[i].z);
								if(environment==snowyenvironment){
									if(findDistancefast(&tippoint[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, tippoint[i],tipvelocity[i], terrainlight.x,terrainlight.y,terrainlight.z, .5, .7);
								}
								else if(environment==grassyenvironment){
									if(findDistancefast(&tippoint[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, tippoint[i],tipvelocity[i], terrainlight.x*90/255,terrainlight.y*70/255,terrainlight.z*8/255, .5, .5);
								}
								else if(environment==desertenvironment){
									if(findDistancefast(&tippoint[i],&viewer)<viewdistance*viewdistance/4)sprites.MakeSprite(cloudsprite, tippoint[i],tipvelocity[i], terrainlight.x*190/255,terrainlight.y*170/255,terrainlight.z*108/255, .5, .7);
								}
							}
						}
					}

					//Edges
					mid=position[i]+tippoint[i];
					mid/=2;
					mid+=(position[i]-mid)/20;
					oldmid=mid;
					if(mid.y<terrain.getHeight(mid.x,mid.z)){
						hitsomething[i]=1;
						mid.y=terrain.getHeight(mid.x,mid.z);

						terrainnormal=terrain.getNormal(mid.x,mid.z);
						ReflectVector(&velocity[i],&terrainnormal);
						//mid+=terrainnormal*.002;
						bounceness=terrainnormal*findLength(&velocity[i])*(abs(normaldotproduct(velocity[i],terrainnormal)));
						if(findLengthfast(&velocity[i])<findLengthfast(&bounceness))bounceness=0;
						frictionness=abs(normaldotproduct(velocity[i],terrainnormal));
						velocity[i]-=bounceness;
						if(1-friction*frictionness>0)velocity[i]*=1-friction*frictionness;
						else velocity[i]=0;
						if(terrain.getOpacity(mid.x,mid.z)<.2)velocity[i]+=bounceness*elasticity*.3;
						else velocity[i]+=bounceness*elasticity;

						if(findLengthfast(&bounceness)>1){
							float gLoc[3];
							float vel[3];
							int whichsound;
							if(terrain.getOpacity(mid.x,mid.z)>.2){
								if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
								if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);
							}
							else whichsound=footstepsound+abs(Random()%2);
							gLoc[0]=mid.x;
							gLoc[1]=mid.y;
							gLoc[2]=mid.z;
							vel[0]=0;
							vel[1]=0;
							vel[2]=0;
							PlaySoundEx( whichsound, samp[whichsound], NULL, true);
							OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
							if(terrain.getOpacity(position[i].x,position[i].z)>.2)OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
							else OPENAL_SetVolume(channels[whichsound], 32*findLengthfast(&bounceness));
							OPENAL_SetPaused(channels[whichsound], false);
						}
						position[i]+=(mid-oldmid)*20;
					}

					mid=position[i]+tippoint[i];
					mid/=2;
					mid+=(tippoint[i]-mid)/20;
					oldmid=mid;
					if(mid.y<terrain.getHeight(mid.x,mid.z)){
						hitsomething[i]=1;
						mid.y=terrain.getHeight(mid.x,mid.z);

						terrainnormal=terrain.getNormal(mid.x,mid.z);
						ReflectVector(&tipvelocity[i],&terrainnormal);
						//mid+=terrainnormal*.002;
						bounceness=terrainnormal*findLength(&tipvelocity[i])*(abs(normaldotproduct(tipvelocity[i],terrainnormal)));
						if(findLengthfast(&tipvelocity[i])<findLengthfast(&bounceness))bounceness=0;
						frictionness=abs(normaldotproduct(tipvelocity[i],terrainnormal));
						tipvelocity[i]-=bounceness;
						if(1-friction*frictionness>0)tipvelocity[i]*=1-friction*frictionness;
						else tipvelocity[i]=0;
						if(terrain.getOpacity(mid.x,mid.z)<.2)tipvelocity[i]+=bounceness*elasticity*.3;
						else tipvelocity[i]+=bounceness*elasticity;

						if(findLengthfast(&bounceness)>1){
							float gLoc[3];
							float vel[3];
							int whichsound;
							if(terrain.getOpacity(mid.x,mid.z)>.2){
								if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
								if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);
							}
							else whichsound=footstepsound+abs(Random()%2);
							gLoc[0]=mid.x;
							gLoc[1]=mid.y;
							gLoc[2]=mid.z;
							vel[0]=0;
							vel[1]=0;
							vel[2]=0;
							PlaySoundEx( whichsound, samp[whichsound], NULL, true);
							OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
							if(terrain.getOpacity(position[i].x,position[i].z)>.2)OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
							else OPENAL_SetVolume(channels[whichsound], 32*findLengthfast(&bounceness));
							OPENAL_SetPaused(channels[whichsound], false);
						}
						tippoint[i]+=(mid-oldmid)*20;
					}
					/*XYZ mid;
					mid=position[i]+tippoint[i];
					mid/=2;
					if(position[i].y<terrain.getHeightExtrude(mid.x,mid.z,position[i].x,position[i].z)){
					hitsomething[i]=1;
					position[i].y=terrain.getHeightExtrude(mid.x,mid.z,position[i].x,position[i].z);

					terrainnormal=terrain.getNormal(mid.x,mid.z);
					ReflectVector(&velocity[i],&terrainnormal);
					position[i]+=terrainnormal*.002;
					bounceness=terrainnormal*findLength(&velocity[i])*(abs(normaldotproduct(velocity[i],terrainnormal)));
					if(findLengthfast(&velocity[i])<findLengthfast(&bounceness))bounceness=0;
					frictionness=abs(normaldotproduct(velocity[i],terrainnormal));
					velocity[i]-=bounceness;
					if(1-friction*frictionness>0)velocity[i]*=1-friction*frictionness;
					else velocity[i]=0;
					if(terrain.getOpacity(mid.x,mid.z)<.2)velocity[i]+=bounceness*elasticity*.3;
					else velocity[i]+=bounceness*elasticity;

					if(findLengthfast(&bounceness)>1){
					float gLoc[3];
					float vel[3];
					int whichsound;
					if(terrain.getOpacity(mid.x,mid.z)>.2){
					if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
					if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);
					}
					else whichsound=footstepsound+abs(Random()%2);
					gLoc[0]=position[i].x;
					gLoc[1]=position[i].y;
					gLoc[2]=position[i].z;
					vel[0]=0;
					vel[1]=0;
					vel[2]=0;
					PlaySoundEx( whichsound, samp[whichsound], NULL, true);
					OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
					if(terrain.getOpacity(position[i].x,position[i].z)>.2)OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
					else OPENAL_SetVolume(channels[whichsound], 32*findLengthfast(&bounceness));
					OPENAL_SetPaused(channels[whichsound], false);
					}
					}

					if(tippoint[i].y<terrain.getHeightExtrude(mid.x,mid.z,tippoint[i].x,tippoint[i].z)){
					hitsomething[i]=1;
					tippoint[i].y=terrain.getHeightExtrude(mid.x,mid.z,tippoint[i].x,tippoint[i].z);

					terrainnormal=terrain.getNormal(mid.x,mid.z);
					ReflectVector(&tipvelocity[i],&terrainnormal);
					tippoint[i]+=terrainnormal*.002;
					bounceness=terrainnormal*findLength(&tipvelocity[i])*(abs(normaldotproduct(tipvelocity[i],terrainnormal)));
					if(findLengthfast(&tipvelocity[i])<findLengthfast(&bounceness))bounceness=0;
					frictionness=abs(normaldotproduct(tipvelocity[i],terrainnormal));
					tipvelocity[i]-=bounceness;
					if(1-friction*frictionness>0)tipvelocity[i]*=1-friction*frictionness;
					else tipvelocity[i]=0;
					if(terrain.getOpacity(mid.x,mid.z)<.2)tipvelocity[i]+=bounceness*elasticity*.3;
					else tipvelocity[i]+=bounceness*elasticity;

					if(findLengthfast(&bounceness)>1){
					float gLoc[3];
					float vel[3];
					int whichsound;
					if(terrain.getOpacity(mid.x,mid.z)>.2){
					if(type[i]==staff)whichsound=footstepsound3+abs(Random()%2);
					if(type[i]!=staff)whichsound=clank1sound+abs(Random()%4);
					}
					else whichsound=footstepsound+abs(Random()%2);
					gLoc[0]=tippoint[i].x;
					gLoc[1]=tippoint[i].y;
					gLoc[2]=tippoint[i].z;
					vel[0]=0;
					vel[1]=0;
					vel[2]=0;
					PlaySoundEx( whichsound, samp[whichsound], NULL, true);
					OPENAL_3D_SetAttributes(channels[whichsound], gLoc, vel);
					if(terrain.getOpacity(tippoint[i].x,tippoint[i].z)>.2)OPENAL_SetVolume(channels[whichsound], 128*findLengthfast(&bounceness));
					else OPENAL_SetVolume(channels[whichsound], 32*findLengthfast(&bounceness));
					OPENAL_SetPaused(channels[whichsound], false);
					}
					}*/

					//Fix terrain edge collision
					/*start=position[i];
					end=tippoint[i];
					whichhit=terrain.lineTerrain(start,end,&colpoint);
					if(whichhit!=-1){
					XYZ tippoi,posit;
					tippoi=tippoint[i];
					posit=position[i];


					while(whichhit!=-1){
					position[i].y+=.1;
					tippoint[i].y+=.1;
					velocity[i].y+=.1;
					tipvelocity[i].y+=.1;
					start=position[i];
					end=tippoint[i];
					whichhit=terrain.lineTerrain(start,end,&colpoint);
					if(whichhit!=-1)
					closestpoint=colpoint*terrain.scale;
					}
					position[i].y-=.1;
					tippoint[i].y-=.1;
					velocity[i].y-=.1;
					tipvelocity[i].y-=.1;
					start=position[i];
					end=tippoint[i];
					whichhit=terrain.lineTerrain(start,end,&colpoint);
					while(whichhit!=-1){
					position[i].y+=.01;
					tippoint[i].y+=.01;
					velocity[i].y+=.01;
					tipvelocity[i].y+=.01;
					start=position[i];
					end=tippoint[i];
					whichhit=terrain.lineTerrain(start,end,&colpoint);
					if(whichhit!=-1)
					closestpoint=colpoint*terrain.scale;
					}
					}*/
					/*if(whichhit!=-1){
					whichhit=terrain.lineTerrain(end,start,&closestswordpoint);
					if(whichhit!=-1){
					colpoint=(closestswordpoint*terrain.scale+colpoint*terrain.scale)/2;
					proportion=findDistance(&tippoint[i],&colpoint)/findDistance(&position[i],&tippoint[i]);
					if(proportion<=1){
					while(whichhit!=-1){
					position[i].y+=.1*proportion;
					tippoint[i].y+=.1*(1-proportion);
					velocity[i].y+=.1*proportion;
					tipvelocity[i].y+=.1*(1-proportion);
					start=position[i];
					end=tippoint[i];
					whichhit=terrain.lineTerrain(start,end,&colpoint);
					}
					position[i].y-=.1*proportion;
					tippoint[i].y-=.1*(1-proportion);
					velocity[i].y-=.1*proportion;
					tipvelocity[i].y-=.1*(1-proportion);
					start=position[i];
					end=tippoint[i];
					whichhit=terrain.lineTerrain(start,end,&colpoint);
					while(whichhit!=-1){
					position[i].y+=.01*proportion;
					tippoint[i].y+=.01*(1-proportion);
					velocity[i].y+=.01*proportion;
					tipvelocity[i].y+=.01*(1-proportion);
					start=position[i];
					end=tippoint[i];
					whichhit=terrain.lineTerrain(start,end,&colpoint);
					}
					}
					}
					}
					*/
					//Gravity
					velocity[i].y+=gravity*multiplier;
					tipvelocity[i].y+=gravity*multiplier;
					//position[i].y+=gravity*multiplier*multiplier;
					//tippoint[i].y+=gravity*multiplier*multiplier;

					//Rotation
					XYZ temppoint1,temppoint2,tempforward;
					float distance;

					temppoint1=position[i];
					temppoint2=tippoint[i];
					distance=findDistance(&temppoint1,&temppoint2);
					rotation2[i]=asin((temppoint1.y-temppoint2.y)/distance);
					rotation2[i]*=360/6.28;
					temppoint1.y=0;
					temppoint2.y=0;
					rotation1[i]=acos((temppoint1.z-temppoint2.z)/findDistance(&temppoint1,&temppoint2));
					rotation1[i]*=360/6.28;
					rotation3[i]=0;
					smallrotation[i]=90;
					smallrotation2[i]=0;
					bigtilt[i]=0;
					bigtilt2[i]=0;
					bigrotation[i]=0;
					if(temppoint1.x>temppoint2.x)rotation1[i]=360-rotation1[i];

					//Stop moving
					if(findLengthfast(&velocity[i])<.3&&findLengthfast(&tipvelocity[i])<.3&&hitsomething[i]){
						freetime[i]+=multiplier;
					}

					//velocity[i]=(position[i]-oldposition[i])/multiplier;
					//tipvelocity[i]==(tippoint[i-+oldtippoint[i])/multiplier;
					if(freetime[i]>.4){
						velocity[i]=0;
						tipvelocity[i]=0;
					}
					firstfree[i]=0;
			}
		}
		multiplier=tempmult;
		if(blooddrip[i]&&bloody[i]){
			blooddripdelay[i]-=blooddrip[i]*multiplier/2;
			blooddrip[i]-=multiplier;
			if(blooddrip[i]<0)blooddrip[i]=0;
			if(blooddrip[i]>5)blooddrip[i]=5;
			if(blooddripdelay[i]<0&&bloodtoggle){
				blooddripdelay[i]=1;
				XYZ bloodvel;
				XYZ bloodloc;
				bloodloc=position[i]+(tippoint[i]-position[i])*.7;
				bloodloc.y-=.05;
				if(bloodtoggle){
					bloodvel=0;
					sprites.MakeSprite(bloodsprite, bloodloc,bloodvel, 1,1,1, .03, 1);
				}
			}
		}
		if(onfire[i]){
			flamedelay[i]-=multiplier;
			if(onfire[i]&&flamedelay[i]<=0){
				flamedelay[i]=.020;
				flamedelay[i]-=multiplier;
				normalrot=0;
				if(owner[i]!=-1){
					normalrot=player[owner[i]].velocity;
				}
				normalrot.y+=1;
				if(owner[i]!=-1){
					if(player[owner[i]].onterrain){
						normalrot.y=1;
					}
				}
				sprites.MakeSprite(weaponflamesprite, position[i]+tippoint[i]*(((float)abs(Random()%100))/600+.05),normalrot, 1,1,1, (.6+(float)abs(Random()%100)/200-.25)*1/3, 1);
				sprites.speed[sprites.numsprites-1]=4;
				sprites.alivetime[sprites.numsprites-1]=.3;
			}
		}

		if(!onfire[i]&&owner[i]==-1&&type[i]!=staff){
			flamedelay[i]-=multiplier;
			if(flamedelay[i]<=0){
				flamedelay[i]=.020;
				flamedelay[i]-=multiplier;
				normalrot=0;
				if(Random()%50==0&&findDistancefast(&position[i],&viewer)>80){
					XYZ shinepoint;
					shinepoint=position[i]+(tippoint[i]-position[i])*(((float)abs(Random()%100))/100);
					sprites.MakeSprite(weaponshinesprite, shinepoint,normalrot, 1,1,1, (.1+(float)abs(Random()%100)/200-.25)*1/3*fast_sqrt(findDistance(&shinepoint,&viewer)), 1);
					sprites.speed[sprites.numsprites-1]=4;
					sprites.alivetime[sprites.numsprites-1]=.3;
				}
			}
		}
	}
}

int Weapons::Draw()
{
	static int i,j;
	static XYZ terrainlight;
	static GLfloat M[16];
	static bool draw;
	glAlphaFunc(GL_GREATER, 0.9);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDepthMask(1);
	for(i=0;i<numweapons;i++)
	{
		if((frustum.SphereInFrustum(position[i].x,position[i].y,position[i].z,1)&&findDistancefast(&viewer,&position[i])<viewdistance*viewdistance))
		{
			draw=0;
			if(owner[i]==-1)
			{
				draw=1;
				if(velocity[i].x&&!physics[i])drawhowmany[i]=10;
				else drawhowmany[i]=1;
			}
			if(owner[i]!=-1)
			{
				if(player[owner[i]].occluded<25)
					if((frustum.SphereInFrustum(player[owner[i]].coords.x,player[owner[i]].coords.y+player[owner[i]].scale*3,player[owner[i]].coords.z,player[owner[i]].scale*8)&&findDistancefast(&viewer,&player[owner[i]].coords)<viewdistance*viewdistance)||player[owner[i]].skeleton.free==3)
						draw=1;
				if((player[owner[i]].targetanimation==knifeslashstartanim||player[owner[i]].targetanimation==swordsneakattackanim||(player[owner[i]].currentanimation==staffhitanim&&player[owner[i]].currentframe>1)||(player[owner[i]].currentanimation==staffhitreversedanim&&player[owner[i]].currentframe>1)||(player[owner[i]].currentanimation==staffspinhitanim&&player[owner[i]].currentframe>1)||(player[owner[i]].currentanimation==staffspinhitreversedanim&&player[owner[i]].currentframe>1)||(player[owner[i]].currentanimation==staffgroundsmashanim&&player[owner[i]].currentframe>1)||(player[owner[i]].targetanimation==swordslashanim&&player[owner[i]].targetframe<7)||player[owner[i]].targetanimation==crouchstabanim||player[owner[i]].targetanimation==swordslashreversalanim||player[owner[i]].targetanimation==swordslashreversedanim||player[owner[i]].targetanimation==knifefollowanim||player[owner[i]].targetanimation==swordgroundstabanim||player[owner[i]].targetanimation==knifethrowanim)&&player[owner[i]].targetanimation==lastdrawnanim[i]&&!player[owner[i]].skeleton.free)
				{
					drawhowmany[i]=10;
				}
				else drawhowmany[i]=1;
				if(player[owner[i]].targetanimation==swordgroundstabanim)
				{
					lastdrawnrotation1[i]=rotation1[i];
					lastdrawnrotation2[i]=rotation2[i];
					lastdrawnrotation3[i]=rotation3[i];
					lastdrawnbigrotation[i]=bigrotation[i];
					lastdrawnbigtilt[i]=bigtilt[i];
					lastdrawnbigtilt2[i]=bigtilt2[i];
					lastdrawnsmallrotation[i]=smallrotation[i];
					lastdrawnsmallrotation2[i]=smallrotation2[i];
				}
			}
			if(draw)
			{
				terrainlight=terrain.getLighting(position[i].x,position[i].z);
				if(drawhowmany[i]>0)
				{
					glAlphaFunc(GL_GREATER, 0.01);
				}
				for(j=drawhowmany[i];j>0;j--)
				{
					glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
					glPushMatrix();
						glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,j/drawhowmany[i]);
						if(owner[i]!=-1)glTranslatef(position[i].x*(((float)(j))/drawhowmany[i])+lastdrawnposition[i].x*(1-((float)(j))/drawhowmany[i]),position[i].y*(((float)(j))/drawhowmany[i])-.02+lastdrawnposition[i].y*(1-((float)(j))/drawhowmany[i]),position[i].z*(((float)(j))/drawhowmany[i])+lastdrawnposition[i].z*(1-((float)(j))/drawhowmany[i]));
						if(owner[i]==-1)glTranslatef(position[i].x*(((float)(j))/drawhowmany[i])+lastdrawnposition[i].x*(1-((float)(j))/drawhowmany[i]),position[i].y*(((float)(j))/drawhowmany[i])+lastdrawnposition[i].y*(1-((float)(j))/drawhowmany[i]),position[i].z*(((float)(j))/drawhowmany[i])+lastdrawnposition[i].z*(1-((float)(j))/drawhowmany[i]));
						//glTranslatef(position[i].x,position[i].y-.02,position[i].z);
						glRotatef(bigrotation[i]*(((float)(j))/drawhowmany[i])+lastdrawnbigrotation[i]*(1-((float)(j))/drawhowmany[i]),0,1,0);
						glRotatef(bigtilt2[i]*(((float)(j))/drawhowmany[i])+lastdrawnbigtilt2[i]*(1-((float)(j))/drawhowmany[i]),1,0,0);
						glRotatef(bigtilt[i]*(((float)(j))/drawhowmany[i])+lastdrawnbigtilt[i]*(1-((float)(j))/drawhowmany[i]),0,0,1);
						glRotatef(-rotation1[i]*(((float)(j))/drawhowmany[i])-lastdrawnrotation1[i]*(1-((float)(j))/drawhowmany[i])+90,0,1,0);
						glRotatef(-rotation2[i]*(((float)(j))/drawhowmany[i])-lastdrawnrotation2[i]*(1-((float)(j))/drawhowmany[i])+90,0,0,1);
						glRotatef(-rotation3[i]*(((float)(j))/drawhowmany[i])-lastdrawnrotation3[i]*(1-((float)(j))/drawhowmany[i]),0,1,0);
						glRotatef(smallrotation[i]*(((float)(j))/drawhowmany[i])+lastdrawnsmallrotation[i]*(1-((float)(j))/drawhowmany[i]),1,0,0);
						glRotatef(smallrotation2[i]*(((float)(j))/drawhowmany[i])+lastdrawnsmallrotation2[i]*(1-((float)(j))/drawhowmany[i]),0,1,0);

						if(owner[i]!=-1)
						{
							if(player[owner[i]].targetanimation==staffhitanim||player[owner[i]].currentanimation==staffhitanim||player[owner[i]].targetanimation==staffhitreversedanim||player[owner[i]].currentanimation==staffhitreversedanim)
							{
								glTranslatef(0,0,-.3);
							}
							if(player[owner[i]].targetanimation==staffgroundsmashanim||player[owner[i]].currentanimation==staffgroundsmashanim||player[owner[i]].targetanimation==staffspinhitreversedanim||player[owner[i]].currentanimation==staffspinhitreversedanim||player[owner[i]].targetanimation==staffspinhitanim||player[owner[i]].currentanimation==staffspinhitanim)
							{
								glTranslatef(0,0,-.1);
							}
						}
						/*if(type[i]==knife){
						if(owner[i]==-1){
						if(!physics[i]&&findDistance(&position[i],&oldposition[i])*5>1)glScalef(1,1,findDistance(&position[i],&oldposition[i])*5);
						}
						}*/

						if(type[i]==knife)
						{
							glEnable(GL_LIGHTING);
							if(!bloody[i]||!bloodtoggle)throwingknifemodel.drawdifftex(knifetextureptr);
							if(bloodtoggle)
							{
								if(bloody[i]==1)throwingknifemodel.drawdifftex(lightbloodknifetextureptr);
								if(bloody[i]==2)throwingknifemodel.drawdifftex(bloodknifetextureptr);
							}
						}
						if(type[i]==sword)
						{
							glEnable(GL_LIGHTING);
							if(!bloody[i]||!bloodtoggle)swordmodel.drawdifftex(swordtextureptr);
							if(bloodtoggle)
							{
								if(bloody[i]==1)swordmodel.drawdifftex(lightbloodswordtextureptr);
								if(bloody[i]==2)swordmodel.drawdifftex(bloodswordtextureptr);
							}
						}
						if(type[i]==staff)
						{
							glEnable(GL_LIGHTING);
							staffmodel.drawdifftex(stafftextureptr);
						}

					glPopMatrix();
				}

				lastdrawnposition[i]=position[i];
				lastdrawntippoint[i]=tippoint[i];
				lastdrawnrotation1[i]=rotation1[i];
				lastdrawnrotation2[i]=rotation2[i];
				lastdrawnrotation3[i]=rotation3[i];
				lastdrawnbigrotation[i]=bigrotation[i];
				lastdrawnbigtilt[i]=bigtilt[i];
				lastdrawnbigtilt2[i]=bigtilt2[i];
				lastdrawnsmallrotation[i]=smallrotation[i];
				lastdrawnsmallrotation2[i]=smallrotation2[i];
				if(owner[i]!=-1)lastdrawnanim[i]=player[owner[i]].currentanimation;
			}
			if(owner[i]!=-1)
			{
				glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
				glPushMatrix();
					glLoadIdentity();
					glTranslatef(position[i].x,position[i].y-.02,position[i].z);
					glRotatef(bigrotation[i],0,1,0);
					glRotatef(bigtilt2[i],1,0,0);
					glRotatef(bigtilt[i],0,0,1);
					glRotatef(-rotation1[i]+90,0,1,0);
					glRotatef(-rotation2[i]+90,0,0,1);
					glRotatef(-rotation3[i],0,1,0);
					glRotatef(smallrotation[i],1,0,0);
					glRotatef(smallrotation2[i],0,1,0);
					glTranslatef(0,0,length[i]);
					glGetFloatv(GL_MODELVIEW_MATRIX,M);
					tippoint[i].x=M[12];
					tippoint[i].y=M[13];
					tippoint[i].z=M[14];
				glPopMatrix();
			}
			/*XYZ shinepoint;
			XYZ nothingpoint;
			nothingpoint=0;
			shinepoint=position[i];
			sprites.MakeSprite(weaponshinesprite, shinepoint,nothingpoint, 1,1,1,multiplier*2, 1);
			sprites.speed[sprites.numsprites-1]=4;
			sprites.alivetime[sprites.numsprites-1]=.3;
			shinepoint=tippoint[i];
			sprites.MakeSprite(weaponshinesprite, shinepoint,nothingpoint, 1,1,1,multiplier*2, 1);
			sprites.speed[sprites.numsprites-1]=4;
			sprites.alivetime[sprites.numsprites-1]=.3;*/
		}
	}
	return 0;
}

Weapons::Weapons()
{
	numweapons = 0;

	//		Model throwingknifemodel;
	knifetextureptr = 0;
	lightbloodknifetextureptr = 0;
	bloodknifetextureptr = 0;

	//		Model swordmodel;
	swordtextureptr = 0;
	lightbloodswordtextureptr = 0;
	bloodswordtextureptr = 0;

	//		Model staffmodel;
	stafftextureptr = 0;
}

Weapons::~Weapons()
{
	if (stafftextureptr) glDeleteTextures( 1, &stafftextureptr );
	if (knifetextureptr) glDeleteTextures( 1, &knifetextureptr );
	if (lightbloodknifetextureptr) glDeleteTextures( 1, &lightbloodknifetextureptr );
	if (bloodknifetextureptr) glDeleteTextures( 1, &bloodknifetextureptr );
	if (swordtextureptr) glDeleteTextures( 1, &swordtextureptr );
	if (lightbloodswordtextureptr) glDeleteTextures( 1, &lightbloodswordtextureptr );
	if (bloodswordtextureptr) glDeleteTextures( 1, &bloodswordtextureptr );
}

