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

#include "Game.h"
#include "openal_wrapper.h"

using namespace std;

extern XYZ viewer;
extern int environment;
extern float texscale;
extern Light light;
extern Terrain terrain;
extern Sprites sprites;
extern float multiplier;
extern float sps;
extern float viewdistance;
extern float fadestart;
extern float screenwidth,screenheight;
extern int kTextureSize;
extern FRUSTUM frustum;
extern Light light;
extern Objects objects;
extern int detail;
extern float usermousesensitivity;
extern bool osx;
extern float camerashake;
extern Weapons weapons;
extern Person player[maxplayers];
extern int slomo;
extern float slomodelay;
extern bool ismotionblur;
extern float woozy;
extern float blackout;
extern bool damageeffects;
extern float volume;
extern int numplayers;
extern bool texttoggle;
extern float blurness;
extern float targetblurness;
extern float playerdist;
extern bool cellophane;
extern bool freeze;
extern float flashamount,flashr,flashg,flashb;
extern int flashdelay;
extern int netstate;
extern float motionbluramount;
extern bool isclient;
extern bool alwaysblur;
extern int test;
extern bool tilt2weird;
extern bool tiltweird;
extern bool midweird;
extern bool proportionweird;
extern bool vertexweird[6];
extern bool velocityblur;
extern bool buttons[3];
extern bool debugmode;
extern int mainmenu;
extern int oldmainmenu;
extern int bloodtoggle;
extern int difficulty;
extern bool decals;
// MODIFIED GWC
//extern int texdetail;
extern float texdetail;
extern bool musictoggle;
extern int bonus;
extern float bonusvalue;
extern float bonustotal;
extern float bonustime;
extern int oldbonus;
extern float startbonustotal;
extern float bonusnum[100];
extern int tutoriallevel;
extern float smoketex;
extern float tutorialstagetime;
extern float tutorialmaxtime;
extern int tutorialstage;
extern bool againbonus;
extern float damagedealt;
extern float damagetaken;
extern bool invertmouse;

extern int numhotspots;
extern int winhotspot;
extern int killhotspot;
extern XYZ hotspot[40];
extern int hotspottype[40];
extern float hotspotsize[40];
extern char hotspottext[40][256];
extern int currenthotspot;

extern int numaccounts;
extern int accountactive;
extern int accountdifficulty[10];
extern int accountprogress[10];
extern float accountpoints[10];
extern float accounthighscore[10][50];
extern float accountfasttime[10][50];
extern bool accountunlocked[10][60];
extern char accountname[10][256];

extern int numfalls;
extern int numflipfail;
extern int numseen;
extern int numstaffattack;
extern int numswordattack;
extern int numknifeattack;
extern int numunarmedattack;
extern int numescaped;
extern int numflipped;
extern int numwallflipped;
extern int numthrowkill;
extern int numafterkill;
extern int numreversals;
extern int numattacks;
extern int maxalarmed;
extern int numresponded;

extern bool campaign;
extern bool winfreeze;

extern float menupulse;

extern bool gamestart;

extern int numdialogues;
extern int numdialogueboxes[max_dialogues];
extern int dialoguetype[max_dialogues];
extern int dialogueboxlocation[max_dialogues][max_dialoguelength];
extern float dialogueboxcolor[max_dialogues][max_dialoguelength][3];
extern int dialogueboxsound[max_dialogues][max_dialoguelength];
extern char dialoguetext[max_dialogues][max_dialoguelength][128];
extern char dialoguename[max_dialogues][max_dialoguelength][64];
extern XYZ dialoguecamera[max_dialogues][max_dialoguelength];
extern XYZ participantlocation[max_dialogues][10];
extern int participantfocus[max_dialogues][max_dialoguelength];
extern int participantaction[max_dialogues][max_dialoguelength];
extern float participantrotation[max_dialogues][10];
extern XYZ participantfacing[max_dialogues][max_dialoguelength][10];
extern float dialoguecamerarotation[max_dialogues][max_dialoguelength];
extern float dialoguecamerarotation2[max_dialogues][max_dialoguelength];
extern int indialogue;
extern int whichdialogue;
extern int directing;
extern float dialoguetime;
extern int dialoguegonethrough[20];

extern int accountcampaignchoicesmade[10];
extern int accountcampaignchoices[10][5000];

extern float accountcampaignhighscore[10];
extern float accountcampaignfasttime[10];
extern float accountcampaignscore[10];
extern float accountcampaigntime[10];

extern bool gamestarted;

extern OPENAL_SAMPLE	*samp[100];
extern int channels[100];
extern "C" 	void PlaySoundEx(int channel, OPENAL_SAMPLE *sptr, OPENAL_DSPUNIT *dsp, signed char startpaused);

/*********************> DrawGLScene() <*****/
long long Game::MD5_string (char *string){
	char temp[256]="";
	char temp2[256]="";
	long long num=90814;

	sprintf (temp, "%s",string);

	int i=0;
	while (i<256&&temp[i]!='\0'){
		if(temp[i]%3==0)num+=temp[i]*124;
		else if(temp[i]%3==1)num-=temp[i]*temp[i];
		else num*=temp[i];
		i++;
	}

	num=longlongabs(num);
	if(num==0)num+=1452;

	while(num<LONGLONGCONST(5000000000000000)){
		num*=1.85421521;
	}

	while(num>LONGLONGCONST(9900000000000000)){
		num/=1.235421521;
	}

	return num;

	//return 1111111111111111;
}

int Game::DrawGLScene(void)
{	
	static float texcoordwidth,texcoordheight;
	static float texviewwidth, texviewheight;
	static int i,j,k,l;
	static GLubyte color;
	static float newbrightness;
	static float changespeed;
	static XYZ checkpoint;
	static float tempmult;
	float tutorialopac;
	static char string[256]="";
	static char string2[256]="";
	static char string3[256]="";

	static float lastcheck;

	lastcheck+=multiplier;

	glColorMask( 1.0, 1.0, 1.0, 1.0 );
	if(!registered)debugmode=0;


	if(freeze||winfreeze||(mainmenu&&gameon)||(!gameon&&gamestarted)){
		tempmult=multiplier;
		multiplier=0;
	}

	if(!mainmenu){
		if(editorenabled){
			numboundaries=mapradius*2;
			if(numboundaries>360)numboundaries=360;
			for(i=0;i<numboundaries;i++){
				boundary[i]=0;
				boundary[i].z=1;
				boundary[i]=mapcenter+DoRotation(boundary[i]*mapradius,0,i*(360/((float)(numboundaries))),0);
			}
		}

		SetUpLighting();

		static int changed;
		changed=0;

		olddrawmode=drawmode;
		if(ismotionblur&&!loading){
			if((findLengthfast(&player[0].velocity)>200)&&velocityblur&&!cameramode){
				drawmode=motionblurmode;
				motionbluramount=200/(findLengthfast(&player[0].velocity));
				changed=1;
			}
			if(player[0].damage-player[0].superpermanentdamage>(player[0].damagetolerance-player[0].superpermanentdamage)*1/2&&damageeffects&&!cameramode){
				drawmode=doublevisionmode;
				changed=1;
			}
		}

		if(slomo&&!loading){
			if(ismotionblur)
				drawmode=motionblurmode;
			motionbluramount=.2;
			slomodelay-=multiplier;
			if(slomodelay<0)slomo=0;
			camerashake=0;
			changed=1;
		}
		if((!changed&&!slomo)||loading){
			drawmode=normalmode;
			if(ismotionblur&&(/*fps>100||*/alwaysblur)){
				if(olddrawmode!=realmotionblurmode)change=1;
				else change=0;
				drawmode=realmotionblurmode;
			}
			else if(olddrawmode==realmotionblurmode)change=2;
			else change=0;
		}

		if(freeze||winfreeze||(mainmenu&&gameon)||(!gameon&&gamestarted))drawmode=normalmode;
		if((freeze||winfreeze)&&ismotionblur&&!mainmenu)drawmode=radialzoommode;

		if(winfreeze||mainmenu)drawmode=normalmode;

		//drawmode=glowmode;
		if(drawmode==glowmode){
			RGBColor color2;
			color2.red=0;
			color2.green=0;
			color2.blue=0;
#if PLATFORM_MACOSX
			DSpContext_FadeGamma(NULL,200,&color2);
#endif
		}

		if(drawtoggle!=2)drawtoggle=1-drawtoggle;

		if(!texcoordwidth){

			texviewwidth=kTextureSize;
			if(texviewwidth>screenwidth)texviewwidth=screenwidth;
			texviewheight=kTextureSize;
			if(texviewheight>screenheight)texviewheight=screenheight;

			texcoordwidth=screenwidth/kTextureSize;
			texcoordheight=screenheight/kTextureSize;
			if(texcoordwidth>1)texcoordwidth=1;
			if(texcoordheight>1)texcoordheight=1;
		}

		glDrawBuffer(GL_BACK);
		glReadBuffer(GL_BACK);

		/*if(environment==desertenvironment)glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, (float)(abs(Random()%100))/50 );
		else glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
		*/
		if(abs(blurness-targetblurness)<multiplier*10||abs(blurness-targetblurness)>2){
			blurness=targetblurness;
			targetblurness=(float)(abs(Random()%100))/40;
		}
		if(blurness<targetblurness)blurness+=multiplier*5;
		if(blurness>targetblurness)blurness-=multiplier*5;

		//glFinish();
		static XYZ terrainlight;
		static float distance;
		//if(drawmode==normalmode)ReSizeGLScene(90,.1);
		if(drawmode==normalmode)ReSizeGLScene(90,.1f);
		if(drawmode!=normalmode)glViewport(0,0,texviewwidth,texviewheight);	
		glDepthFunc(GL_LEQUAL);
		glDepthMask(1);
		glAlphaFunc(GL_GREATER, 0.0001f);
		glEnable(GL_ALPHA_TEST);
		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		glMatrixMode (GL_MODELVIEW);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glLoadIdentity ();
		if(!cameramode&&!freeze&&!winfreeze){
			glRotatef(float(Random()%100)/10*camerashake/*+(woozy*woozy)/10*/,0,0,1);
			glRotatef(rotation2+sin(woozy/2)*(player[0].damage/player[0].damagetolerance)*5,1,0,0);
			glRotatef(rotation+sin(woozy)*(player[0].damage/player[0].damagetolerance)*5,0,1,0);
		}
		if(cameramode||freeze||winfreeze){
			//glRotatef(float(Random()%100)/10*camerashake/*+(woozy*woozy)/10*/,0,0,1);
			glRotatef(rotation2,1,0,0);
			glRotatef(rotation,0,1,0);
		}

		if(environment==desertenvironment){
			glRotatef((float)(abs(Random()%100))/3000-1,1,0,0);
			glRotatef((float)(abs(Random()%100))/3000-1,0,1,0);
			//glRotatef(blurness/40-1,1,0,0);
			//glRotatef(blurness/40-1,0,1,0);
		}
		SetUpLight(&light,0);
		glPushMatrix();
		if(environment==desertenvironment&&detail==2)glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, blurness+.4 );
		if(environment==desertenvironment){
			glRotatef((float)(abs(Random()%100))/1000,1,0,0);
			glRotatef((float)(abs(Random()%100))/1000,0,1,0);
		}	
		skybox.draw();
		glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0);
		glPopMatrix();
		glTranslatef(-viewer.x,-viewer.y,-viewer.z);
		frustum.GetFrustum();

		//Make Shadow
		static XYZ point;
		static float size,opacity,rotation;
		rotation=0;
		for(k=0;k<numplayers;k++){
			if(!player[k].skeleton.free&&player[k].playerdetail&&player[k].howactive<typesleeping)
				if(frustum.SphereInFrustum(player[k].coords.x,player[k].coords.y+player[k].scale*3,player[k].coords.z,player[k].scale*7)&&player[k].occluded<25)
					for(i=0;i<player[k].skeleton.num_joints;i++){
						if(player[k].skeleton.joints[i].label==leftknee||player[k].skeleton.joints[i].label==rightknee||player[k].skeleton.joints[i].label==groin){
							point=DoRotation(player[k].skeleton.joints[i].position,0,player[k].rotation,0)*player[k].scale+player[k].coords;
							size=.4f;
							opacity=.4-player[k].skeleton.joints[i].position.y*player[k].scale/5-(player[k].coords.y-terrain.getHeight(player[k].coords.x,player[k].coords.z))/10;
							if(k!=0&&tutoriallevel==1){
								opacity=.2+.2*sin(smoketex*6+i)-player[k].skeleton.joints[i].position.y*player[k].scale/5-(player[k].coords.y-terrain.getHeight(player[k].coords.x,player[k].coords.z))/10;
							}
							terrain.MakeDecal(shadowdecal,point,size,opacity,rotation);
							if(terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz])
								for(l=0;l<terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz];l++){
									j=terrain.patchobjects[player[k].whichpatchx][player[k].whichpatchz][l];
									if(objects.position[j].y<player[k].coords.y||objects.type[j]==tunneltype||objects.type[j]==weirdtype){
										point=DoRotation(DoRotation(player[k].skeleton.joints[i].position,0,player[k].rotation,0)*player[k].scale+player[k].coords-objects.position[j],0,-objects.rotation[j],0);
										size=.4f;
										opacity=.4f;
										if(k!=0&&tutoriallevel==1){
											opacity=.2+.2*sin(smoketex*6+i);
										}
										objects.model[j].MakeDecal(shadowdecal,&point,&size,&opacity,&rotation);
									}
								}
						}
					}
					if((player[k].skeleton.free||player[k].howactive>=typesleeping)&&player[k].playerdetail)
						if(frustum.SphereInFrustum(player[k].coords.x,player[k].coords.y,player[k].coords.z,player[k].scale*5)&&player[k].occluded<25)
							for(i=0;i<player[k].skeleton.num_joints;i++){
								if(player[k].skeleton.joints[i].label==leftknee||player[k].skeleton.joints[i].label==rightknee||player[k].skeleton.joints[i].label==groin||player[k].skeleton.joints[i].label==leftelbow||player[k].skeleton.joints[i].label==rightelbow||player[k].skeleton.joints[i].label==neck){
									if(player[k].skeleton.free)point=player[k].skeleton.joints[i].position*player[k].scale+player[k].coords;
									else point=DoRotation(player[k].skeleton.joints[i].position,0,player[k].rotation,0)*player[k].scale+player[k].coords;
									size=.4f;
									opacity=.4-player[k].skeleton.joints[i].position.y*player[k].scale/5-(player[k].coords.y-terrain.getHeight(player[k].coords.x,player[k].coords.z))/5;
									if(k!=0&&tutoriallevel==1){
										opacity=.2+.2*sin(smoketex*6+i)-player[k].skeleton.joints[i].position.y*player[k].scale/5-(player[k].coords.y-terrain.getHeight(player[k].coords.x,player[k].coords.z))/10;
									}
									terrain.MakeDecal(shadowdecal,point,size,opacity*.7,rotation);
									if(terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz])
										for(l=0;l<terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz];l++){
											j=terrain.patchobjects[player[k].whichpatchx][player[k].whichpatchz][l];
											if(objects.position[j].y<player[k].coords.y||objects.type[j]==tunneltype||objects.type[j]==weirdtype){
												if(player[k].skeleton.free)point=DoRotation(player[k].skeleton.joints[i].position*player[k].scale+player[k].coords-objects.position[j],0,-objects.rotation[j],0);
												else point=DoRotation(DoRotation(player[k].skeleton.joints[i].position,0,player[k].rotation,0)*player[k].scale+player[k].coords-objects.position[j],0,-objects.rotation[j],0);
												size=.4f;
												opacity=.4f;
												if(k!=0&&tutoriallevel==1){
													opacity=.2+.2*sin(smoketex*6+i);
												}
												objects.model[j].MakeDecal(shadowdecal,&point,&size,&opacity,&rotation);
											}
										}
								}
							}

							if(!player[k].playerdetail)
								if(frustum.SphereInFrustum(player[k].coords.x,player[k].coords.y,player[k].coords.z,player[k].scale*5))
								{
									point=player[k].coords;
									size=.7;
									opacity=.4-(player[k].coords.y-terrain.getHeight(player[k].coords.x,player[k].coords.z))/5;
									terrain.MakeDecal(shadowdecal,point,size,opacity*.7,rotation);
									if(terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz])
										for(l=0;l<terrain.patchobjectnum[player[k].whichpatchx][player[k].whichpatchz];l++){
											j=terrain.patchobjects[player[k].whichpatchx][player[k].whichpatchz][l];
											point=DoRotation(player[k].coords-objects.position[j],0,-objects.rotation[j],0);
											size=.7;
											opacity=.4f;
											objects.model[j].MakeDecal(shadowdecal,&point,&size,&opacity,&rotation);
										}
								}
		}

		//Terrain
		glEnable(GL_TEXTURE_2D);
		glDepthMask(1);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
		glBindTexture( GL_TEXTURE_2D, terraintexture);
		terrain.draw(0);
		glBindTexture( GL_TEXTURE_2D, terraintexture2);
		terrain.draw(1);
		//glBindTexture( GL_TEXTURE_2D, terraintexture3);
		//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		//terrain.draw(2);

		terrain.drawdecals();

		//Model
		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glDepthMask(1);

		glEnable(GL_COLOR_MATERIAL);

		test=2;
		tilt2weird=0;
		tiltweird=0;
		midweird=0;
		proportionweird=0;
		vertexweird[0]=0;
		vertexweird[1]=0;
		vertexweird[2]=0;
		vertexweird[3]=0;
		vertexweird[4]=0;
		vertexweird[5]=0;

		if(!cellophane){
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glDepthMask(1);
			for(k=0;k<numplayers;k++){
				if(k==0||tutoriallevel!=1){
					glEnable(GL_BLEND);
					glEnable(GL_LIGHTING);
					terrainlight=terrain.getLighting(player[k].coords.x,player[k].coords.z);
					distance=findDistancefast(&viewer,&player[k].coords);
					distance=(viewdistance*viewdistance-(distance-(viewdistance*viewdistance*fadestart))*(1/(1-fadestart)))/viewdistance/viewdistance;
					glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,distance);
					if(distance>=1)glDisable(GL_BLEND);
					if(distance>=.5){
						checkpoint=DoRotation(player[k].skeleton.joints[abs(Random()%player[k].skeleton.num_joints)].position,0,player[k].rotation,0)*player[k].scale+player[k].coords;
						checkpoint.y+=1;
						if(!player[k].occluded==0)i=checkcollide(viewer,checkpoint,player[k].lastoccluded);
						if(i==-1||player[k].occluded==0)i=checkcollide(viewer,checkpoint);
						if(i!=-1){
							player[k].occluded+=1;
							player[k].lastoccluded=i;
						}
						else player[k].occluded=0;
						if(player[k].occluded<25)player[k].DrawSkeleton();
					}
				}
			}
		}

		if(!cameramode&&musictype==stream_music2)playerdist=findDistancefastflat(&player[0].coords,&viewer);
		else playerdist=-100;
		glPushMatrix();
		glCullFace(GL_BACK);
		glEnable(GL_TEXTURE_2D);
		objects.Draw();
		glPopMatrix();

		glPushMatrix();
		if(frustum.SphereInFrustum(realhawkcoords.x+hawk.boundingspherecenter.x,realhawkcoords.y+hawk.boundingspherecenter.y,realhawkcoords.z+hawk.boundingspherecenter.z,2)){   
			glAlphaFunc(GL_GREATER, 0.0001f);
			glDepthMask(1);
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glEnable(GL_BLEND);
			glTranslatef(hawkcoords.x,hawkcoords.y,hawkcoords.z);
			glRotatef(hawkrotation,0,1,0);
			glTranslatef(25,0,0);
			distance=findDistancefast(&viewer,&realhawkcoords)*1.2;
			glColor4f(light.color[0],light.color[1],light.color[2],(viewdistance*viewdistance-(distance-(viewdistance*viewdistance*fadestart))*(1/(1-fadestart)))/viewdistance/viewdistance);
			if((viewdistance*viewdistance-(distance-(viewdistance*viewdistance*fadestart))*(1/(1-fadestart)))/viewdistance/viewdistance>1)glColor4f(light.color[0],light.color[1],light.color[2],1);
			if((viewdistance*viewdistance-(distance-(viewdistance*viewdistance*fadestart))*(1/(1-fadestart)))/viewdistance/viewdistance>0)
				hawk.drawdifftex(hawktexture);
		}
		glPopMatrix();

		//if(cellophane){
		/*glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glDepthMask(1);
		for(k=0;k<numplayers;k++){
		glEnable(GL_BLEND);
		glEnable(GL_LIGHTING);
		terrainlight=terrain.getLighting(player[k].coords.x,player[k].coords.z);
		distance=findDistancefast(&viewer,&player[k].coords);
		distance=(viewdistance*viewdistance-(distance-(viewdistance*viewdistance*fadestart))*(1/(1-fadestart)))/viewdistance/viewdistance;
		glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,distance);
		if(distance>=1)glDisable(GL_BLEND);
		if(distance>0){
		checkpoint=DoRotation(player[k].skeleton.joints[abs(Random()%player[k].skeleton.num_joints)].position,0,player[k].rotation,0)*player[k].scale+player[k].coords;
		checkpoint.y+=1;
		if(checkcollide(viewer,checkpoint)){
		player[k].occluded+=1;
		}
		else player[k].occluded=0;
		if(player[k].occluded<25)player[k].DrawSkeleton();
		}
		}*/

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glDepthMask(1);
		for(k=0;k<numplayers;k++){
			if(!(k==0||tutoriallevel!=1)){
				glEnable(GL_BLEND);
				glEnable(GL_LIGHTING);
				terrainlight=terrain.getLighting(player[k].coords.x,player[k].coords.z);
				distance=findDistancefast(&viewer,&player[k].coords);
				distance=(viewdistance*viewdistance-(distance-(viewdistance*viewdistance*fadestart))*(1/(1-fadestart)))/viewdistance/viewdistance;
				glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,distance);
				if(distance>=1)glDisable(GL_BLEND);
				if(distance>=.5){
					checkpoint=DoRotation(player[k].skeleton.joints[abs(Random()%player[k].skeleton.num_joints)].position,0,player[k].rotation,0)*player[k].scale+player[k].coords;
					checkpoint.y+=1;
					if(!player[k].occluded==0)i=checkcollide(viewer,checkpoint,player[k].lastoccluded);
					if(i==-1||player[k].occluded==0)i=checkcollide(viewer,checkpoint);
					if(i!=-1){
						player[k].occluded+=1;
						player[k].lastoccluded=i;
					}
					else player[k].occluded=0;
					if(player[k].occluded<25)player[k].DrawSkeleton();
				}
			}
		}
		//}

		glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		weapons.Draw();
		glPopMatrix();
		glCullFace(GL_BACK);

		glDisable(GL_COLOR_MATERIAL);

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glDepthMask(0);

		sprites.Draw();

		if(editorenabled){
			glEnable(GL_BLEND);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_COLOR_MATERIAL);
			glColor4f(1,1,0,1);

			for(k=0;k<numplayers;k++){
				if(player[k].numwaypoints>1){
					glBegin(GL_LINE_LOOP);
					for(i=0;i<player[k].numwaypoints;i++){
						glVertex3f(player[k].waypoints[i].x,player[k].waypoints[i].y+.5,player[k].waypoints[i].z);
					}
					glEnd();
				}
			}


			if(numpathpoints>1){
				glColor4f(0,1,0,1);
				for(k=0;k<numpathpoints;k++){
					if(numpathpointconnect[k]){
						for(i=0;i<numpathpointconnect[k];i++){
							glBegin(GL_LINE_LOOP);
							glVertex3f(pathpoint[k].x,pathpoint[k].y+.5,pathpoint[k].z);
							glVertex3f(pathpoint[pathpointconnect[k][i]].x,pathpoint[pathpointconnect[k][i]].y+.5,pathpoint[pathpointconnect[k][i]].z);					
							glEnd();
						}
					}
				}
				glColor4f(1,1,1,1);
				glPointSize(4);
				glBegin(GL_POINTS);
				glVertex3f(pathpoint[pathpointselected].x,pathpoint[pathpointselected].y+.5,pathpoint[pathpointselected].z);
				glEnd();
			}
		}

		//Text

		glEnable(GL_TEXTURE_2D);
		glColor4f(.5,.5,.5,1);
		if(!console){
			sprintf (string, " ",(int)(fps));
			text.glPrint(10,30,string,0,.8,screenwidth,screenheight);

			if(!tutoriallevel)
				if(bonus>0&&bonustime<1&&!winfreeze&&indialogue==-1/*bonustime<4*/){
					if(bonus==tracheotomy)sprintf (string, "Tracheotomy!");
					else if(bonus==backstab)sprintf (string, "Backstabber!");
					else if(bonus==spinecrusher)sprintf (string, "Spinecrusher!");
					else if(bonus==ninja)sprintf (string, "Ninja Bonus!");
					else if(bonus==style)sprintf (string, "Style Bonus!");
					else if(bonus==cannon)sprintf (string, "Leg Cannon!");
					else if(bonus==aimbonus)sprintf (string, "Nice Aim!");
					else if(bonus==deepimpact)sprintf (string, "Heavy Impact!");
					else if(bonus==touchofdeath)sprintf (string, "Touch of Death!");
					else if(bonus==swordreversebonus)sprintf (string, "Sword Disarm!");
					else if(bonus==staffreversebonus)sprintf (string, "Staff Disarm!");
					else if(bonus==reverseko)sprintf (string, "Reversal KO!");
					else if(bonus==solidhit)sprintf (string, "Solid Hit!");
					else if(bonus==twoxcombo)sprintf (string, "2X Combo!");
					else if(bonus==threexcombo)sprintf (string, "3X Combo!");
					else if(bonus==fourxcombo)sprintf (string, "4X COMBO!");
					else if(bonus==megacombo)sprintf (string, "MEGA COMBO!");
					else if(bonus==Reversal)sprintf (string, "Reversal!");
					else if(bonus==Stabbonus)sprintf (string, "Punctured!");
					else if(bonus==Slicebonus)sprintf (string, "Sliced!");
					else if(bonus==Bullseyebonus)sprintf (string, "Bullseye!");
					else if(bonus==Slashbonus)sprintf (string, "Slashed!");
					else if(bonus==Wolfbonus)sprintf (string, "WOLF SLAYER!");
					else if(bonus==FinishedBonus)sprintf (string, "SLAIN!");
					else if(bonus==TackleBonus)sprintf (string, "Tackle!");
					else if(bonus==AboveBonus)sprintf (string, "Death from Above!");
					else sprintf (string, "Excellent!");

					glColor4f(0,0,0,1-bonustime);
					text.glPrintOutline(1024/2-10*strlen(string)-4,768/16-4+768*4/5,string,1,2.5,1024,768);
					glColor4f(1,0,0,1-bonustime);
					text.glPrint(1024/2-10*strlen(string),768/16+768*4/5,string,1,2,1024,768);

					sprintf (string, "%d",(int)bonusvalue);
					glColor4f(0,0,0,1-bonustime);
					text.glPrintOutline(1024/2-10*strlen(string)-4,768/16-4-20+768*4/5,string,1,2.5*.8,1024,768);
					glColor4f(1,0,0,1-bonustime);
					text.glPrint(1024/2-10*strlen(string),768/16-20+768*4/5,string,1,2*.8,1024,768);
					glColor4f(.5,.5,.5,1);
				}

				if(tutoriallevel==1){
					tutorialopac=tutorialmaxtime-tutorialstagetime;
					if(tutorialopac>1)tutorialopac=1;
					if(tutorialopac<0)tutorialopac=0;

					sprintf (string, " ");
					sprintf (string2, " ");
					sprintf (string3, " ");
					if(tutorialstage==0){
						sprintf (string, " ");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==1){
						sprintf (string, "Welcome to the Lugaru training level!");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==2){
						sprintf (string, "BASIC MOVEMENT:");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==3){
						sprintf (string, "You can move the mouse to rotate the camera.");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==4){
						sprintf (string, "Try using the %s, %s, %s and %s keys to move around.",KeyToChar(forwardkey),KeyToChar(leftkey),KeyToChar(backkey),KeyToChar(rightkey));
						sprintf (string2, "All movement is relative to the camera.");
						sprintf (string3, " ");
					}
					if(tutorialstage==5){
						sprintf (string, "Please press %s to jump.",KeyToChar(jumpkey));
						sprintf (string2, "You can hold it longer to jump higher.");
						sprintf (string3, " ");
					}
					if(tutorialstage==6){
						sprintf (string, "You can press %s to crouch.",KeyToChar(crouchkey));
						sprintf (string2, "You can jump higher from a crouching position.");
						sprintf (string3, " ");
					}
					if(tutorialstage==7){
						sprintf (string, "While running, you can press %s to roll.",KeyToChar(crouchkey));
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==8){
						sprintf (string, "While crouching, you can sneak around silently");
						sprintf (string2, "using the movement keys.");
						sprintf (string3, " ");
					}
					if(tutorialstage==9){
						sprintf (string, "Release the crouch key while sneaking and hold the movement keys");
						sprintf (string2, "to run animal-style.");
						sprintf (string3, " ");
					}
					if(tutorialstage==10){
						sprintf (string, "ADVANCED MOVEMENT:");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==11){
						sprintf (string, "When you jump at a wall, you can hold %s again",KeyToChar(jumpkey));
						sprintf (string2, "during impact to perform a walljump.");
						sprintf (string3, "Be sure to use the movement keys to press against the wall");
					}
					if(tutorialstage==12){
						sprintf (string, "While in the air, you can press crouch to flip.",KeyToChar(jumpkey));
						sprintf (string2, "Walljumps and flips confuse enemies and give you more control.");
						sprintf (string3, " ");
					}
					if(tutorialstage==13){
						sprintf (string, "BASIC COMBAT:");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==14){
						sprintf (string, "There is now an imaginary enemy");
						sprintf (string2, "in the middle of the training area.");
						sprintf (string3, " ");
					}
					if(tutorialstage==15){
						if(attackkey==MAC_MOUSEBUTTON1)sprintf (string, "Click to attack when you are near an enemy.");
						else sprintf (string, "Press %s to attack when you are near an enemy.",KeyToChar(attackkey));
						sprintf (string2, "You can punch by standing still near an enemy and attacking.");
						sprintf (string3, " ");
					}
					if(tutorialstage==16){
						sprintf (string, "If you are close, you will perform a weak punch.");
						sprintf (string2, "The weak punch is excellent for starting attack combinations.");
						sprintf (string3, " ");
					}
					if(tutorialstage==17){
						sprintf (string, "Attacking while running results in a spin kick.");
						sprintf (string2, "This is one of your most powerful ground attacks.");
						sprintf (string3, " ");
					}
					if(tutorialstage==18){
						sprintf (string, "Sweep the enemy's legs out by attacking while crouched.");
						sprintf (string2, "This is a very fast attack, and easy to follow up.");
						sprintf (string3, " ");
					}
					if(tutorialstage==19){
						sprintf (string, "When an enemy is on the ground, you can deal some extra");
						sprintf (string2, "damage by running up and drop-kicking him.");
						sprintf (string3, "(Try knocking them down with a sweep first)");
					}
					if(tutorialstage==20){
						sprintf (string, "Your most powerful individual attack is the rabbit kick.");
						if(attackkey==MAC_MOUSEBUTTON1)sprintf (string2, "Run at the enemy while holding the mouse button, and press");
						else sprintf (string2, "Run at the enemy while holding %s, and press", KeyToChar(attackkey));
						sprintf (string3, "the jump key (%s) to attack.",KeyToChar(jumpkey));
					}
					if(tutorialstage==21){
						sprintf (string, "This attack is devastating if timed correctly.");
						sprintf (string2, "Even if timed incorrectly, it will knock the enemy over.");
						if(againbonus)sprintf (string3, "Try rabbit-kicking the imaginary enemy again.");
						else sprintf (string3, "Try rabbit-kicking the imaginary enemy.");
					}
					if(tutorialstage==22){
						sprintf (string, "If you sneak behind an enemy unnoticed, you can kill");
						sprintf (string2, "him instantly. Move close behind this enemy");
						sprintf (string3, "and attack.");
					}
					if(tutorialstage==23){
						sprintf (string, "Another important attack is the wall kick. When an enemy");
						sprintf (string2, "is near a wall, perform a walljump nearby and hold");
						sprintf (string3, "the attack key during impact with the wall.");
					}
					if(tutorialstage==24){
						sprintf (string, "You can tackle enemies by running at them animal-style");
						if(attackkey==MAC_MOUSEBUTTON1)sprintf (string2, "and pressing jump (%s) or attack(mouse button).",KeyToChar(jumpkey));
						else sprintf (string2, "and pressing jump (%s) or attack(%s).",KeyToChar(jumpkey),KeyToChar(attackkey));
						sprintf (string3, "This is especially useful when they are running away.");
					}
					if(tutorialstage==25){
						sprintf (string, "Dodge by pressing back and attack. Dodging is essential");
						sprintf (string2, "against enemies with swords or other long weapons.");
						sprintf (string3, " ");
					}
					if(tutorialstage==26){
						sprintf (string, "REVERSALS AND COUNTER-REVERSALS");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==27){
						sprintf (string, "The enemy can now reverse your attacks.");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==28){
						sprintf (string, "If you attack, you will notice that the enemy now sometimes");
						sprintf (string2, "catches your attack and uses it against you. Hold");
						sprintf (string3, "crouch (%s) after attacking to escape from reversals.",KeyToChar(crouchkey));
					}
					if(tutorialstage==29){
						sprintf (string, "Try escaping from two more reversals in a row.");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==30){
						sprintf (string, "Good!");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==31){
						sprintf (string, "To reverse an attack, you must tap crouch (%s) during the",KeyToChar(crouchkey));
						sprintf (string2, "enemy's attack. You must also be close to the enemy;");
						sprintf (string3, "this is especially important against armed opponents.");
					}
					if(tutorialstage==32){
						sprintf (string, "The enemy can attack in %d seconds.", (int)(tutorialmaxtime-tutorialstagetime));
						sprintf (string2, "This imaginary opponents attacks will be highlighted");
						sprintf (string3, "to make this easier.");
					}
					if(tutorialstage==33){
						sprintf (string, "Reverse three enemy attacks!");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==34){
						sprintf (string, "Reverse two more enemy attacks!");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==35){
						sprintf (string, "Reverse one more enemy attack!");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==36){
						sprintf (string, "Excellent!");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==37){
						sprintf (string, "Now spar with the enemy for %d more seconds.", (int)(tutorialmaxtime-tutorialstagetime));
						sprintf (string2, "Damage dealt: %d",(int)damagedealt);
						sprintf (string3, "Damage taken: %d.",(int)damagetaken);
					}
					if(tutorialstage==38){
						sprintf (string, "WEAPONS:");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==39){
						sprintf (string, "There is now an imaginary knife");
						sprintf (string2, "in the center of the training area.");
						sprintf (string3, " ");
					}
					if(tutorialstage==40){
						sprintf (string, "Stand, roll or handspring over the knife");
						sprintf (string2, "while pressing %s to pick it up.",KeyToChar(throwkey));
						sprintf (string3, "You can crouch and press the same key to drop it again.");
					}
					if(tutorialstage==41){
						sprintf (string, "You can equip and unequip weapons using the %s key.",KeyToChar(drawkey));
						sprintf (string2, "Sometimes it is best to keep them unequipped to");
						sprintf (string3, "prevent enemies from taking them. ");
					}
					if(tutorialstage==42){
						sprintf (string, "The knife is the smallest weapon and the least encumbering.");
						sprintf (string2, "You can equip or unequip it while standing, crouching,");
						sprintf (string3, "running or flipping.");
					}
					if(tutorialstage==43){
						sprintf (string, "You perform weapon attacks the same way as unarmed attacks,");
						sprintf (string2, "but sharp weapons cause permanent damage, instead of the");
						sprintf (string3, "temporary trauma from blunt weapons, fists and feet.");
					}
					if(tutorialstage==44){
						sprintf (string, "The enemy now has your knife!");
						sprintf (string2, "Please reverse two of his knife attacks.");
						sprintf (string3, " ");
					}
					if(tutorialstage==45){
						sprintf (string, "Please reverse one more of his knife attacks.");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}
					if(tutorialstage==46){
						sprintf (string, "Now he has a sword!");
						sprintf (string2, "The sword has longer reach than your arms, so you");
						sprintf (string3, "must move close to reverse the sword slash.");
					}
					if(tutorialstage==47){
						sprintf (string, "Long weapons like the sword and staff are also useful for defense;");
						sprintf (string2, "you can parry enemy weapon attacks by pressing the attack key");
						sprintf (string3, "at the right time. Please try parrying the enemy's attacks!");
					}
					if(tutorialstage==48){
						sprintf (string, "The staff is like the sword, but has two main attacks.");
						sprintf (string2, "The standing smash is fast and effective, and the running");
						sprintf (string3, "spin smash is slower and more powerful.");
					}
					if(tutorialstage==49){
						sprintf (string, "When facing an enemy, you can throw the knife with %s.",KeyToChar(throwkey));
						sprintf (string2, "It is possible to throw the knife while flipping,");
						sprintf (string3, "but it is very inaccurate.");
					}
					if(tutorialstage==50){
						sprintf (string, "You now know everything you can learn from training.");
						sprintf (string2, "Everything else you must learn from experience!");
						sprintf (string3, " ");
					}
					if(tutorialstage==51){
						sprintf (string, "Walk out of the training area to return to the main menu.");
						sprintf (string2, " ");
						sprintf (string3, " ");
					}

					glColor4f(0,0,0,tutorialopac);
					text.glPrintOutline(screenwidth/2-7.6*strlen(string)*screenwidth/1024-4,screenheight/16-4+screenheight*4/5,string,1,1.5*1.25*screenwidth/1024,screenwidth,screenheight);
					text.glPrintOutline(screenwidth/2-7.6*strlen(string2)*screenwidth/1024-4,screenheight/16-4+screenheight*4/5-20*screenwidth/1024,string2,1,1.5*1.25*screenwidth/1024,screenwidth,screenheight);
					text.glPrintOutline(screenwidth/2-7.6*strlen(string3)*screenwidth/1024-4,screenheight/16-4+screenheight*4/5-40*screenwidth/1024,string3,1,1.5*1.25*screenwidth/1024,screenwidth,screenheight);
					glColor4f(1,1,1,tutorialopac);
					text.glPrint(screenwidth/2-7.6*strlen(string)*screenwidth/1024,screenheight/16+screenheight*4/5,string,1,1.5*screenwidth/1024,screenwidth,screenheight);
					text.glPrint(screenwidth/2-7.6*strlen(string2)*screenwidth/1024,screenheight/16+screenheight*4/5-20*screenwidth/1024,string2,1,1.5*screenwidth/1024,screenwidth,screenheight);
					text.glPrint(screenwidth/2-7.6*strlen(string3)*screenwidth/1024,screenheight/16+screenheight*4/5-40*screenwidth/1024,string3,1,1.5*screenwidth/1024,screenwidth,screenheight);

					sprintf (string, "Press 'tab' to skip to the next item.",KeyToChar(jumpkey));
					sprintf (string2, "Press escape at any time to");
					sprintf (string3, "pause or exit the tutorial.");

					glColor4f(0,0,0,1);
					text.glPrintOutline(screenwidth/2-7.6*strlen(string)*screenwidth/1024*.8-4,0-4+screenheight*1/10,string,1,1.5*1.25*screenwidth/1024*.8,screenwidth,screenheight);
					text.glPrintOutline(screenwidth/2-7.6*strlen(string2)*screenwidth/1024*.8-4,0-4+screenheight*1/10-20*.8*screenwidth/1024,string2,1,1.5*1.25*screenwidth/1024*.8,screenwidth,screenheight);
					text.glPrintOutline(screenwidth/2-7.6*strlen(string3)*screenwidth/1024*.8-4,0-4+screenheight*1/10-40*.8*screenwidth/1024,string3,1,1.5*1.25*screenwidth/1024*.8,screenwidth,screenheight);
					glColor4f(0.5,0.5,0.5,1);
					text.glPrint(screenwidth/2-7.6*strlen(string)*screenwidth/1024*.8,0+screenheight*1/10,string,1,1.5*screenwidth/1024*.8,screenwidth,screenheight);
					text.glPrint(screenwidth/2-7.6*strlen(string2)*screenwidth/1024*.8,0+screenheight*1/10-20*.8*screenwidth/1024,string2,1,1.5*screenwidth/1024*.8,screenwidth,screenheight);
					text.glPrint(screenwidth/2-7.6*strlen(string3)*screenwidth/1024*.8,0+screenheight*1/10-40*.8*screenwidth/1024,string3,1,1.5*screenwidth/1024*.8,screenwidth,screenheight);
				}
				//Hot spots	

				if(numhotspots&&(bonustime>=1||bonus<=0||bonustime<0)&&!tutoriallevel){
					int closest=-1;
					float closestdist=-1;
					float distance=0;
					closest=currenthotspot;
					for(i=0;i<numhotspots;i++){
						distance=findDistancefast(&player[0].coords,&hotspot[i]);
						if(closestdist==-1||distance<closestdist){
							if(findDistancefast(&player[0].coords,&hotspot[i])<hotspotsize[i]&&((hotspottype[i]<=10&&hotspottype[i]>=0)||(hotspottype[i]<=40&&hotspottype[i]>=20))){
								closestdist=distance;
								closest=i;
							}
						}
					}
					if(closest!=-1)
						currenthotspot=closest;
					if(currenthotspot!=-1){
						if(hotspottype[closest]<=10){
							if(findDistancefast(&player[0].coords,&hotspot[closest])<hotspotsize[closest])
								tutorialstagetime=0;
							tutorialmaxtime=1;
							tutorialopac=tutorialmaxtime-tutorialstagetime;
							if(tutorialopac>1)tutorialopac=1;
							if(tutorialopac<0)tutorialopac=0;

							sprintf (string, "%s", hotspottext[closest]);

							int lastline;
							int line=0;
							bool done=0;
							lastline=0;
							i=0;
							while(!done){
								if(string[i]=='\n'||string[i]>'z'||string[i]<' '||string[i]=='\0'){
									glColor4f(0,0,0,tutorialopac);
									text.glPrintOutline(screenwidth/2-7.6*(i-lastline)*screenwidth/1024-4,screenheight/16-4+screenheight*4/5-20*screenwidth/1024*line,string,1,1.5*1.25*screenwidth/1024,screenwidth,screenheight,lastline,i);
									glColor4f(1,1,1,tutorialopac);
									text.glPrint(screenwidth/2-7.6*(i-lastline)*screenwidth/1024,screenheight/16+screenheight*4/5-20*screenwidth/1024*line,string,1,1.5*screenwidth/1024,screenwidth,screenheight,lastline,i);
									lastline=i+1;
									line++;
									if(string[i]=='\0')done=1;
								}
								if(i>=255)done=1;
								i++;
							}
						}
						else if (hotspottype[closest]>=20&&dialoguegonethrough[hotspottype[closest]-20]==0){
							whichdialogue=hotspottype[closest]-20;
							for(j=0;j<numdialogueboxes[whichdialogue];j++){
								player[participantfocus[whichdialogue][j]].coords=participantlocation[whichdialogue][participantfocus[whichdialogue][j]];
								player[participantfocus[whichdialogue][j]].rotation=participantrotation[whichdialogue][participantfocus[whichdialogue][j]];
								player[participantfocus[whichdialogue][j]].targetrotation=participantrotation[whichdialogue][participantfocus[whichdialogue][j]];
								player[participantfocus[whichdialogue][j]].velocity=0;
								player[participantfocus[whichdialogue][j]].targetanimation=player[participantfocus[whichdialogue][j]].getIdle();
								player[participantfocus[whichdialogue][j]].targetframe=0;
							}
							directing=0;
							indialogue=0;
							dialoguegonethrough[whichdialogue]++;
							if(dialogueboxsound[whichdialogue][indialogue]!=0){
								static float gLoc[3];
								static float vel[3];
								gLoc[0]=player[participantfocus[whichdialogue][indialogue]].coords.x;
								gLoc[1]=player[participantfocus[whichdialogue][indialogue]].coords.y;
								gLoc[2]=player[participantfocus[whichdialogue][indialogue]].coords.z;
								vel[0]=0;
								vel[1]=0;
								vel[2]=0;
								int whichsoundplay;
								if(dialogueboxsound[whichdialogue][indialogue]==1)whichsoundplay=rabbitchitter;
								if(dialogueboxsound[whichdialogue][indialogue]==2)whichsoundplay=rabbitchitter2;
								if(dialogueboxsound[whichdialogue][indialogue]==3)whichsoundplay=rabbitpainsound;
								if(dialogueboxsound[whichdialogue][indialogue]==4)whichsoundplay=rabbitpain1sound;
								if(dialogueboxsound[whichdialogue][indialogue]==5)whichsoundplay=rabbitattacksound;
								if(dialogueboxsound[whichdialogue][indialogue]==6)whichsoundplay=rabbitattack2sound;
								if(dialogueboxsound[whichdialogue][indialogue]==7)whichsoundplay=rabbitattack3sound;
								if(dialogueboxsound[whichdialogue][indialogue]==8)whichsoundplay=rabbitattack4sound;
								if(dialogueboxsound[whichdialogue][indialogue]==9)whichsoundplay=growlsound;
								if(dialogueboxsound[whichdialogue][indialogue]==10)whichsoundplay=growl2sound;
								if(dialogueboxsound[whichdialogue][indialogue]==11)whichsoundplay=snarlsound;
								if(dialogueboxsound[whichdialogue][indialogue]==12)whichsoundplay=snarl2sound;
								if(dialogueboxsound[whichdialogue][indialogue]==13)whichsoundplay=barksound;
								if(dialogueboxsound[whichdialogue][indialogue]==14)whichsoundplay=bark2sound;
								if(dialogueboxsound[whichdialogue][indialogue]==15)whichsoundplay=bark3sound;
								if(dialogueboxsound[whichdialogue][indialogue]==16)whichsoundplay=barkgrowlsound;
								if(dialogueboxsound[whichdialogue][indialogue]==-1)whichsoundplay=fireendsound;
								if(dialogueboxsound[whichdialogue][indialogue]==-2)whichsoundplay=firestartsound;
								if(dialogueboxsound[whichdialogue][indialogue]==-3)whichsoundplay=consolesuccesssound;
								if(dialogueboxsound[whichdialogue][indialogue]==-4)whichsoundplay=consolefailsound;
								PlaySoundEx( whichsoundplay, samp[whichsoundplay], NULL, true);
								OPENAL_3D_SetAttributes(channels[whichsoundplay], gLoc, vel);
								OPENAL_SetVolume(channels[whichsoundplay], 256);
								OPENAL_SetPaused(channels[whichsoundplay], false);
							}
						}
					}
				}

				if(indialogue!=-1&&!mainmenu){
					glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
					glDisable(GL_CULL_FACE);
					glDisable(GL_LIGHTING);
					glDisable(GL_TEXTURE_2D);
					glDepthMask(0);
					glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
					glPushMatrix();										// Store The Projection Matrix
					glLoadIdentity();									// Reset The Projection Matrix
					glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
					glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
					glPushMatrix();										// Store The Modelview Matrix
					glLoadIdentity();								// Reset The Modelview Matrix
					if(dialogueboxlocation[whichdialogue][indialogue]==1)glTranslatef(0,screenheight*3/4,0);
					glScalef(screenwidth,screenheight/4,1);
					glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					glEnable(GL_BLEND);

					glColor4f(dialogueboxcolor[whichdialogue][indialogue][0],dialogueboxcolor[whichdialogue][indialogue][1],dialogueboxcolor[whichdialogue][indialogue][2],0.7);
					glBegin(GL_QUADS);
					glVertex3f(0,		0, 	 0.0f);
					glVertex3f(1,	0, 	 0.0f);
					glVertex3f(1,	1, 0.0f);
					glVertex3f(0, 	1, 0.0f);
					glEnd();
					glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
					glPopMatrix();										// Restore The Old Projection Matrix
					glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
					glPopMatrix();										// Restore The Old Projection Matrix
					glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
					glEnable(GL_CULL_FACE);
					glDisable(GL_BLEND);
					glDepthMask(1);
					glEnable(GL_TEXTURE_2D);

					tutorialopac=1;

					float startx;
					float starty;

					startx=screenwidth*1/5;
					if(dialogueboxlocation[whichdialogue][indialogue]==1)starty=screenheight/16+screenheight*4/5;
					if(dialogueboxlocation[whichdialogue][indialogue]==2)starty=screenheight*1/5-screenheight/16;

//					char tempname[64];
					char tempname[264];
					bool goodchar;
					int tempnum=0;
//					for(i=0;i<64;i++){
					for(i=0;i<264;i++){
						tempname[i]='\0';
					}

					for(i=0;i<(int)strlen(dialoguename[whichdialogue][indialogue]);i++){
						tempname[tempnum]=dialoguename[whichdialogue][indialogue][i];
						goodchar=1;
						if(dialoguename[whichdialogue][indialogue][i]=='#'||dialoguename[whichdialogue][indialogue][i]=='\0')goodchar=0;
						//if(tempnum>2)if(tempname[tempnum-2]=='e'&&tempname[tempnum-1]=='r')goodchar=0;
						//if(tempnum>2)if(tempname[tempnum]=='r'&&tempname[0]=='a')goodchar=0;
						if(goodchar)tempnum++;
						else tempname[tempnum]='\0';
					}

					sprintf (string, "%s: ", tempname);

					if(dialogueboxcolor[whichdialogue][indialogue][0]+dialogueboxcolor[whichdialogue][indialogue][1]+dialogueboxcolor[whichdialogue][indialogue][2]<1.5){
						glColor4f(0,0,0,tutorialopac);
						text.glPrintOutline(startx-2*7.6*strlen(string)*screenwidth/1024-4,starty-4,string,1,1.5*1.25*screenwidth/1024,screenwidth,screenheight);
						glColor4f(0.7,0.7,0.7,tutorialopac);
						text.glPrint(startx-2*7.6*strlen(string)*screenwidth/1024,starty,string,1,1.5*screenwidth/1024,screenwidth,screenheight);
					}
					else
					{
						glColor4f(0,0,0,tutorialopac);
						text.glPrintOutline(startx-2*7.6*strlen(string)*screenwidth/1024-4,starty-4,string,1,1.5*1.25*screenwidth/1024,screenwidth,screenheight);

					}

					tempnum=0;
					for(i=0;i<(int)strlen(dialoguetext[whichdialogue][indialogue])+1;i++){
						tempname[tempnum]=dialoguetext[whichdialogue][indialogue][i];
						if(dialoguetext[whichdialogue][indialogue][i]!='#')tempnum++;
					}


					sprintf (string, "%s", tempname);


					int lastline;
					int line=0;
					bool done=0;
					lastline=0;
					i=0;
					while(!done){
						if(string[i]=='\n'||string[i]>'z'||string[i]<' '||string[i]=='\0'){
							if(dialogueboxcolor[whichdialogue][indialogue][0]+dialogueboxcolor[whichdialogue][indialogue][1]+dialogueboxcolor[whichdialogue][indialogue][2]<1.5){
								glColor4f(0,0,0,tutorialopac);
								text.glPrintOutline(startx/*-7.6*(i-lastline)*screenwidth/1024*/-4,starty-4-20*screenwidth/1024*line,string,1,1.5*1.25*screenwidth/1024,screenwidth,screenheight,lastline,i);
								glColor4f(1,1,1,tutorialopac);
								text.glPrint(startx/*-7.6*(i-lastline)*screenwidth/1024*/,starty-20*screenwidth/1024*line,string,1,1.5*screenwidth/1024,screenwidth,screenheight,lastline,i);
							}
							else
							{
								glColor4f(0,0,0,tutorialopac);
								text.glPrint(startx/*-7.6*(i-lastline)*screenwidth/1024*/,starty-20*screenwidth/1024*line,string,1,1.5*screenwidth/1024,screenwidth,screenheight,lastline,i);
							}
							lastline=i+1;
							line++;
							if(string[i]=='\0')done=1;
						}
						if(i>=255)done=1;
						i++;
					}
				}	

				if(!tutoriallevel&&!winfreeze&&indialogue==-1&&!mainmenu){
					if(campaign){
						if(!scoreadded)sprintf (string, "Score: %d", (int)accountcampaignscore[accountactive]+(int)bonustotal);//(int)bonustotal);
						if(scoreadded)sprintf (string, "Score: %d", (int)accountcampaignscore[accountactive]);//(int)bonustotal);
					}
					if(!campaign)sprintf (string, "Score: %d", (int)bonustotal);
					glColor4f(0,0,0,1);
					text.glPrintOutline(1024/40-4,768/16-4+768*14/16,string,1,1.5*1.25,1024,768);
					glColor4f(1,0,0,1);
					text.glPrint(1024/40,768/16+768*14/16,string,1,1.5,1024,768);
				}

				glColor4f(.5,.5,.5,1);


				if((texttoggle||editorenabled)&&debugmode&&!mainmenu){
					sprintf (string, "The framespersecond is %d.",(int)(fps));
					text.glPrint(10,30,string,0,.8,1024,768);

					sprintf (string, "Name: %s", registrationname);
					text.glPrint(10,260,string,0,.8,1024,768);


					if(editorenabled)sprintf (string, "Map editor enabled.");
					if(!editorenabled)sprintf (string, "Map editor Disabled.");
					text.glPrint(10,60,string,0,.8,1024,768);
					if(editorenabled){
						sprintf (string, "Object size: %f",editorsize);
						text.glPrint(10,75,string,0,.8,1024,768);
						if(editorrotation>=0)sprintf (string, "Object rotation: %f",editorrotation);
						else sprintf (string, "Object rotation: Random");
						text.glPrint(10,90,string,0,.8,1024,768);
						if(editorrotation2>=0)sprintf (string, "Object rotation2: %f",editorrotation2);
						else sprintf (string, "Object rotation2: Random");
						text.glPrint(10,105,string,0,.8,1024,768);
						sprintf (string, "Object type: %d",editortype);
						text.glPrint(10,120,string,0,.8,1024,768);
						if(editortype==boxtype)sprintf (string, "(box)");
						if(editortype==treetrunktype)sprintf (string, "(tree)");
						if(editortype==walltype)sprintf (string, "(wall)");
						if(editortype==weirdtype)sprintf (string, "(weird)");
						if(editortype==spiketype)sprintf (string, "(spike)");
						if(editortype==rocktype)sprintf (string, "(rock)");
						if(editortype==bushtype)sprintf (string, "(bush)");
						if(editortype==tunneltype)sprintf (string, "(tunnel)");
						if(editortype==chimneytype)sprintf (string, "(chimney)");
						if(editortype==platformtype)sprintf (string, "(platform)");
						if(editortype==cooltype)sprintf (string, "(cool)");
						if(editortype==firetype)sprintf (string, "(fire)");
						text.glPrint(130,120,string,0,.8,1024,768);

						sprintf (string, "Numplayers: %d",numplayers);
						text.glPrint(10,155,string,0,.8,1024,768);
						sprintf (string, "Player %d: numwaypoints: %d",numplayers,player[numplayers-1].numwaypoints);
						text.glPrint(10,140,string,0,.8,1024,768);
					}
					/*sprintf (string, "Coords are: %f %f %f",player[0].coords.x,player[0].coords.y,player[0].coords.z);
					text.glPrint(10,200,string,0,.8,1024,768);*/
					sprintf (string, "Difficulty: %d",difficulty);
					text.glPrint(10,240,string,0,.8,1024,768);
					/*
					sprintf (string, "lasthotspot: %d",hotspottype[numhotspots-1]);
					text.glPrint(10,240,string,0,.8,1024,768);
					sprintf (string, "killhotspot: %d",killhotspot);
					text.glPrint(10,220,string,0,.8,1024,768);
					sprintf (string, "winhotspot: %d",winhotspot);
					text.glPrint(10,200,string,0,.8,1024,768);*/

				}
		}

		if(drawmode==glowmode){
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef(screenwidth,screenheight,1);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glColor4f(0,0,0,.5);
			glBegin(GL_QUADS);
			glVertex3f(0,		0, 	 0.0f);
			glVertex3f(256,	0, 	 0.0f);
			glVertex3f(256,	256, 0.0f);
			glVertex3f(0, 	256, 0.0f);
			glEnd();
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);
		}

		if((((blackout&&damageeffects)||(player[0].bloodloss>0&&damageeffects&&player[0].blooddimamount>0)||player[0].dead)&&!cameramode)||console){
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef(screenwidth,screenheight,1);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			if(player[0].dead)blackout+=multiplier*3;
			if(player[0].dead==1)blackout=.4f;
			if(player[0].dead==2&&blackout>.6)blackout=.6;
			glColor4f(0,0,0,blackout);
			if(!player[0].dead){
				if((player[0].bloodloss/player[0].damagetolerance*(sin(woozy)/4+.5))*.3<.3){
					glColor4f(0,0,0,player[0].blooddimamount*player[0].bloodloss/player[0].damagetolerance*(sin(woozy)/4+.5)*.3);
					blackout=player[0].blooddimamount*player[0].bloodloss/player[0].damagetolerance*(sin(woozy)/4+.5)*.3;
				}
				else {
					glColor4f(0,0,0,player[0].blooddimamount*.3);
					blackout=player[0].blooddimamount*.3;
				}
			}
			if(console)glColor4f(.7,0,0,.2);
			glBegin(GL_QUADS);
			glVertex3f(0,		0, 	 0.0f);
			glVertex3f(256,	0, 	 0.0f);
			glVertex3f(256,	256, 0.0f);
			glVertex3f(0, 	256, 0.0f);
			glEnd();
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);
		}

		if(flashamount>0&&damageeffects){
			if(flashamount>1)flashamount=1;
			if(flashdelay<=0)flashamount-=multiplier;
			flashdelay--;
			if(flashamount<0)flashamount=0;
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef(screenwidth,screenheight,1);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glColor4f(flashr,flashg,flashb,flashamount);
			glBegin(GL_QUADS);
			glVertex3f(0,		0, 	 0.0f);
			glVertex3f(256,	0, 	 0.0f);
			glVertex3f(256,	256, 0.0f);
			glVertex3f(0, 	256, 0.0f);
			glEnd();
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);
		}	

		if(!console){
			displaytime[0]=0;
			glEnable(GL_TEXTURE_2D);
			glColor4f(1,1,1,1);
			if(chatting){
				sprintf (string, " ]");
				text.glPrint(10,30+screenheight-330,string,0,1,screenwidth,screenheight);
				if(displayblink){
					sprintf (string, "_");
					text.glPrint(30+(float)(displayselected)*10,30+(screenheight-330),string,0,1,screenwidth,screenheight);
				}
			}
			for(i=0;i<15;i++){
				if((i!=0||chatting)&&displaytime[i]<4)
					for(j=0;j<displaychars[i];j++){
						glColor4f(1,1,1,4-displaytime[i]);
						if(j<displaychars[i]){
							sprintf (string, "%c",displaytext[i][j]);
							text.glPrint(30+j*10,30+i*20+(screenheight-330),string,0,1,screenwidth,screenheight);
						}
					}
			}
		}

		if(minimap&&indialogue==-1){
			float mapviewdist;
			mapviewdist=20000;

			glDisable(GL_DEPTH_TEST);
			glColor3f (1.0, 1.0, 1.0); // no coloring

			glEnable(GL_TEXTURE_2D);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef((float)screenwidth/2,(float)screenwidth/2,1);
			glTranslatef(1.75,.25,0);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glColor4f(1,1,1,1);
			glPushMatrix();
			float opac;
			opac=.7;
			XYZ center;
			float radius;
			float distcheck;
			center=0;
			int numliveplayers=0;
			for(i=0;i<numplayers;i++){
				if(!player[i].dead)numliveplayers++;
			}

			int numadd;
			numadd=0;

			for(i=0;i<objects.numobjects;i++){
				if(objects.type[i]==treetrunktype||objects.type[i]==boxtype){
					center+=objects.position[i];	
					numadd++;
				}
			}
			for(i=0;i<numplayers;i++){
				if(!player[i].dead)center+=player[i].coords;	
			}
			center/=numadd+numliveplayers;

			center=player[0].coords;

			float maxdistance=0;
			float tempdist;
			int whichclosest;
			for(i=0;i<objects.numobjects;i++){
				tempdist=findDistancefast(&center,&objects.position[i]);
				if(tempdist>maxdistance){
					whichclosest=i;
					maxdistance=tempdist;
				}
			}
			for(i=0;i<numplayers;i++){
				if(!player[i].dead){
					tempdist=findDistancefast(&center,&player[i].coords);
					if(tempdist>maxdistance){
						whichclosest=i;
						maxdistance=tempdist;
					}
				}
			}
			radius=fast_sqrt(maxdistance);

			radius=110;

			glScalef(.25/radius*256*terrain.scale*.4,.25/radius*256*terrain.scale*.4,1);
			glPushMatrix();
			glScalef(1/(1/radius*256*terrain.scale*.4),1/(1/radius*256*terrain.scale*.4),1);
			/*float startx,starty,endx,endy;
			glBegin(GL_QUADS);
			glTexCoord2f(1-(center.x-radius)/terrain.scale/256,(center.z-radius)/terrain.scale/256);
			glVertex3f(-1,		-1, 	 0.0f);
			glTexCoord2f(1-(center.x+radius)/terrain.scale/256,(center.z-radius)/terrain.scale/256);
			glVertex3f(1,	-1, 	 0.0f);
			glTexCoord2f(1-(center.x+radius)/terrain.scale/256,(center.z+radius)/terrain.scale/256);
			glVertex3f(1,	1, 0.0f);
			glTexCoord2f(1-(center.x-radius)/terrain.scale/256,(center.z+radius)/terrain.scale/256);
			glVertex3f(-1, 	1, 0.0f);
			glEnd();*/
			glPopMatrix();
			glRotatef(player[0].lookrotation*-1+180,0,0,1);
			glTranslatef(-(center.x/terrain.scale/256*-2+1),(center.z/terrain.scale/256*-2+1),0);
			for(i=0;i<objects.numobjects;i++){
				if(objects.type[i]==treetrunktype){
					distcheck=findDistancefast(&player[0].coords,&objects.position[i]);
					if(distcheck<mapviewdist){
						glBindTexture( GL_TEXTURE_2D, Mapcircletexture);
						glColor4f(0,.3,0,opac*(1-distcheck/mapviewdist));
						glPushMatrix();
						glTranslatef(objects.position[i].x/terrain.scale/256*-2+1,objects.position[i].z/terrain.scale/256*2-1,0);
						glRotatef(objects.rotation[i],0,0,1);
						glScalef(.003,.003,.003);
						glBegin(GL_QUADS);
						glTexCoord2f(0,0);
						glVertex3f(-1,		-1, 	 0.0f);
						glTexCoord2f(1,0);
						glVertex3f(1,	-1, 	 0.0f);
						glTexCoord2f(1,1);
						glVertex3f(1,	1, 0.0f);
						glTexCoord2f(0,1);
						glVertex3f(-1, 	1, 0.0f);
						glEnd();
						glPopMatrix();
					}
				}
				if(objects.type[i]==boxtype){
					distcheck=findDistancefast(&player[0].coords,&objects.position[i]);
					if(distcheck<mapviewdist){
						glBindTexture( GL_TEXTURE_2D, Mapboxtexture);
						glColor4f(.4,.4,.4,opac*(1-distcheck/mapviewdist));
						glPushMatrix();
						glTranslatef(objects.position[i].x/terrain.scale/256*-2+1,objects.position[i].z/terrain.scale/256*2-1,0);
						glRotatef(objects.rotation[i],0,0,1);
						glScalef(.01*objects.scale[i],.01*objects.scale[i],.01*objects.scale[i]);
						glBegin(GL_QUADS);
						glTexCoord2f(0,0);
						glVertex3f(-1,		-1, 	 0.0f);
						glTexCoord2f(1,0);
						glVertex3f(1,	-1, 	 0.0f);
						glTexCoord2f(1,1);
						glVertex3f(1,	1, 0.0f);
						glTexCoord2f(0,1);
						glVertex3f(-1, 	1, 0.0f);
						glEnd();
						glPopMatrix();
					}
				}
			}
			if(editorenabled){
				glBindTexture( GL_TEXTURE_2D, Mapcircletexture);
				for(i=0;i<numboundaries;i++){
					glColor4f(0,0,0,opac/3);
					glPushMatrix();
					glTranslatef(boundary[i].x/terrain.scale/256*-2+1,boundary[i].z/terrain.scale/256*2-1,0);
					glScalef(.002,.002,.002);
					glBegin(GL_QUADS);
					glTexCoord2f(0,0);
					glVertex3f(-1,		-1, 	 0.0f);
					glTexCoord2f(1,0);
					glVertex3f(1,	-1, 	 0.0f);
					glTexCoord2f(1,1);
					glVertex3f(1,	1, 0.0f);
					glTexCoord2f(0,1);
					glVertex3f(-1, 	1, 0.0f);
					glEnd();
					glPopMatrix();
				}
			}
			for(i=0;i<numplayers;i++){
				distcheck=findDistancefast(&player[0].coords,&player[i].coords);
				if(distcheck<mapviewdist){
					glPushMatrix();
					glBindTexture( GL_TEXTURE_2D, Maparrowtexture);
					if(i==0)glColor4f(1,1,1,opac);
					else if(player[i].dead==2||player[i].howactive>typesleeping)glColor4f(0,0,0,opac*(1-distcheck/mapviewdist));
					else if(player[i].dead)glColor4f(.3,.3,.3,opac*(1-distcheck/mapviewdist));
					else if(player[i].aitype==attacktypecutoff)glColor4f(1,0,0,opac*(1-distcheck/mapviewdist));
					else if(player[i].aitype==passivetype)glColor4f(0,1,0,opac*(1-distcheck/mapviewdist));
					else glColor4f(1,1,0,1);
					glTranslatef(player[i].coords.x/terrain.scale/256*-2+1,player[i].coords.z/terrain.scale/256*2-1,0);
					glRotatef(player[i].rotation+180,0,0,1);
					glScalef(.005,.005,.005);
					glBegin(GL_QUADS);
					glTexCoord2f(0,0);
					glVertex3f(-1,		-1, 	 0.0f);
					glTexCoord2f(1,0);
					glVertex3f(1,	-1, 	 0.0f);
					glTexCoord2f(1,1);
					glVertex3f(1,	1, 0.0f);
					glTexCoord2f(0,1);
					glVertex3f(-1, 	1, 0.0f);
					glEnd();
					/*glBegin(GL_TRIANGLES);
					glTexCoord2f(0,0);
					glVertex3f(-1,		-1, 	 0.0f);
					glTexCoord2f(1,0);
					glVertex3f(1,	-1, 	 0.0f);
					glTexCoord2f(1,1);
					glVertex3f(0,	1, 0.0f);
					glEnd();*/
					glPopMatrix();
				}
			}
			/*glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex3f(-1,		-1, 	 0.0f);
			glTexCoord2f(1,0);
			glVertex3f(1,	-1, 	 0.0f);
			glTexCoord2f(1,1);
			glVertex3f(1,	1, 0.0f);
			glTexCoord2f(0,1);
			glVertex3f(-1, 	1, 0.0f);
			glEnd();*/
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);
		}

		/*if(loading){
		loading=2;
		drawmode=normalmode;
		}*/


		if(loading&&!stealthloading&&(!campaign||player[0].dead)){
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef(screenwidth,screenheight,1);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glColor4f(0,0,0,.7);
			glBegin(GL_QUADS);
			glVertex3f(0,		0, 	 0.0f);
			glVertex3f(256,	0, 	 0.0f);
			glVertex3f(256,	256, 0.0f);
			glVertex3f(0, 	256, 0.0f);
			glEnd();
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);

			//logo
			glDisable(GL_DEPTH_TEST);
			glColor3f (1.0, 1.0, 1.0); // no coloring

			glEnable(GL_TEXTURE_2D);
			/*glBindTexture( GL_TEXTURE_2D, logotexture);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef((float)screenwidth/2,(float)screenwidth/2,1);
			glTranslatef(1.8,1.25,0);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glColor4f(1,1,1,1);
			glPushMatrix();
			glScalef(.25,.25,.25);
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex3f(-1,		-1, 	 0.0f);
			glTexCoord2f(1,0);
			glVertex3f(1,	-1, 	 0.0f);
			glTexCoord2f(1,1);
			glVertex3f(1,	1, 0.0f);
			glTexCoord2f(0,1);
			glVertex3f(-1, 	1, 0.0f);
			glEnd();
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);*/

			//Minimap

			if(loading!=4){				
				glEnable(GL_TEXTURE_2D);
				glColor4f(1,1,1,1);
				sprintf (string, "Loading...");
				text.glPrint(1024/2-90,768/2,string,1,2,1024,768);
			}
			loading=2;
			//if(ismotionblur)drawmode=motionblurmode;
			drawmode=normalmode;
		}

		if(winfreeze&&!campaign){
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef(screenwidth,screenheight,1);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glColor4f(0,0,0,.4);
			glBegin(GL_QUADS);
			glVertex3f(0,		0, 	 0.0f);
			glVertex3f(256,	0, 	 0.0f);
			glVertex3f(256,	256, 0.0f);
			glVertex3f(0, 	256, 0.0f);
			glEnd();
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);

			//logo
			glDisable(GL_DEPTH_TEST);
			glColor3f (1.0, 1.0, 1.0); // no coloring

			glEnable(GL_TEXTURE_2D);
			/*glBindTexture( GL_TEXTURE_2D, logotexture);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef((float)screenwidth/2,(float)screenwidth/2,1);
			glTranslatef(1.8,1.25,0);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glColor4f(1,1,1,1);
			glPushMatrix();
			glScalef(.25,.25,.25);
			glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex3f(-1,		-1, 	 0.0f);
			glTexCoord2f(1,0);
			glVertex3f(1,	-1, 	 0.0f);
			glTexCoord2f(1,1);
			glVertex3f(1,	1, 0.0f);
			glTexCoord2f(0,1);
			glVertex3f(-1, 	1, 0.0f);
			glEnd();
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);*/

			//Awards
			int numawards;
			int awards[30];
			numawards=0;

			if(damagetaken==0&&player[0].bloodloss==0){
				awards[numawards]=awardflawless;
				numawards++;
			}
			bool alldead;
			alldead=1;
			if(numplayers>1)
				for(i=1;i<numplayers;i++){		
					if(player[i].dead!=2)alldead=0;
				}
				if(alldead){
					awards[numawards]=awardalldead;
					numawards++;
				}
				alldead=1;
				if(numplayers>1)
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
					if(numplayers>1)
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


						//Win Screen Won Victory

						glEnable(GL_TEXTURE_2D);
						glColor4f(1,1,1,1);
						sprintf (string, "Level Cleared!");
						text.glPrintOutlined(1024/2-strlen(string)*10,768*7/8,string,1,2,1024,768);

						sprintf (string, "Score:     %d",(int)(bonustotal-startbonustotal));
						text.glPrintOutlined(1024/30,768*6/8,string,1,2,1024,768);

						if(!campaign)sprintf (string, "Press Escape to return to menu or Space to continue");
						if(campaign)sprintf (string, "Press Escape or Space to continue");
						text.glPrintOutlined(640/2-strlen(string)*5,480*1/16,string,1,1,640,480);

						char temp[255];

						for(i=0;i<255;i++)string[i]='\0';
						sprintf (temp, "Time:      %d:",(int)(((int)leveltime-(int)(leveltime)%60)/60));
						strcat(string,temp);
						if((int)(leveltime)%60<10)strcat(string,"0");
						sprintf (temp, "%d",(int)(leveltime)%60);
						strcat(string,temp);
						text.glPrintOutlined(1024/30,768*6/8-40,string,1,2,1024,768);

						for(i=0;i<numawards;i++){
							if(i<6){
								if(awards[i]==awardklutz)sprintf (string, "Suicidal");
								if(awards[i]==awardflawless)sprintf (string, "Flawless!");
								if(awards[i]==awardalldead)sprintf (string, "Take no prisoners");
								if(awards[i]==awardnodead)sprintf (string, "Merciful");
								if(awards[i]==awardstealth)sprintf (string, "One with the shadows!");
								if(awards[i]==awardswordsman)sprintf (string, "Swordsman");
								if(awards[i]==awardkungfu)sprintf (string, "Unarmed!");
								if(awards[i]==awardknifefighter)sprintf (string, "Knife fighter");
								if(awards[i]==awardcoward)sprintf (string, "Coward");
								if(awards[i]==awardevasion)sprintf (string, "Escape artist");
								if(awards[i]==awardacrobat)sprintf (string, "Gymnast");
								if(awards[i]==awardlongrange)sprintf (string, "Blade slinger");
								if(awards[i]==awardbrutal)sprintf (string, "Brutal");
								if(awards[i]==awardhyper)sprintf (string, "Hyper");
								if(awards[i]==awardaikido)sprintf (string, "Aikido master!");
								if(awards[i]==awardrambo)sprintf (string, "Rambo");
								if(awards[i]==awardfast)sprintf (string, "Fast");
								if(awards[i]==awardrealfast)sprintf (string, "Real fast");
								if(awards[i]==awarddamnfast)sprintf (string, "Damn fast");
								if(awards[i]==awardstrategy)sprintf (string, "Divide and conquer");
								if(awards[i]==awardbojutsu)sprintf (string, "Bojutsu");
								text.glPrintOutlined(1024/30,768*6/8-90-40*i,string,1,2,1024,768);
							}
						}

						//drawmode=normalmode;
		}

		if(drawmode!=normalmode){
			glEnable(GL_TEXTURE_2D);
			glFinish();
			if(!drawtoggle||drawmode!=realmotionblurmode||(drawtoggle==2||change==1)){
				if(screentexture){

					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
					GLfloat subtractColor[4] = { 0.5, 0.5, 0.5, 0.0 };
					glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, subtractColor);
					//glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_SUBTRACT);
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE);
					glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT);
					glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);

					glBindTexture( GL_TEXTURE_2D, screentexture);
					glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texviewwidth, texviewheight);		
				}
			}
			if((drawtoggle||change==1)&&drawmode==realmotionblurmode){
				if(screentexture2){
					glBindTexture( GL_TEXTURE_2D, screentexture2);
					glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texviewwidth, texviewheight);		
				}
				if(!screentexture2){
					glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

					glGenTextures( 1, &screentexture2 );
					glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

					glEnable(GL_TEXTURE_2D);
					glBindTexture( GL_TEXTURE_2D, screentexture2);
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

					glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, kTextureSize, kTextureSize, 0);		
				}
			}
			//glFlush();
		}

		glClear(GL_DEPTH_BUFFER_BIT);
		ReSizeGLScene(90,.1f);
		glViewport(0,0,screenwidth,screenheight);	

		if(drawmode!=normalmode){
			glDisable(GL_DEPTH_TEST);
			if(drawmode==motionblurmode){
				glDrawBuffer(GL_FRONT);
				glReadBuffer(GL_BACK);
				//myassert(glGetError() == GL_NO_ERROR);
				//glFlush();
			}
			glColor3f (1.0, 1.0, 1.0); // no coloring

			glEnable(GL_TEXTURE_2D);
			glBindTexture( GL_TEXTURE_2D, screentexture);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
			glLoadIdentity();									// Reset The Projection Matrix
			glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPushMatrix();										// Store The Modelview Matrix
			glLoadIdentity();								// Reset The Modelview Matrix
			glScalef((float)screenwidth/2,(float)screenheight/2,1);
			glTranslatef(1,1,0);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			if(drawmode==motionblurmode){
				if(motionbluramount<.2)motionbluramount=.2;
				//glColor4f(1,1,1,fast_sqrt(multiplier)*2.9*motionbluramount);
				glColor4f(1,1,1,motionbluramount);
				glPushMatrix();
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(-1,		-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,0);
				glVertex3f(1,	-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,texcoordheight);
				glVertex3f(1,	1, 0.0f);
				glTexCoord2f(0,texcoordheight);
				glVertex3f(-1, 	1, 0.0f);
				glEnd();
				glPopMatrix();
			}
			if(drawmode==realmotionblurmode){
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE);
				glBindTexture( GL_TEXTURE_2D, screentexture);
				glColor4f(1,1,1,.5);
				glPushMatrix();
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(-1,		-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,0);
				glVertex3f(1,	-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,texcoordheight);
				glVertex3f(1,	1, 0.0f);
				glTexCoord2f(0,texcoordheight);
				glVertex3f(-1, 	1, 0.0f);
				glEnd();
				glPopMatrix();
				glBindTexture( GL_TEXTURE_2D, screentexture2);
				glColor4f(1,1,1,.5);
				glPushMatrix();
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(-1,		-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,0);
				glVertex3f(1,	-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,texcoordheight);
				glVertex3f(1,	1, 0.0f);
				glTexCoord2f(0,texcoordheight);
				glVertex3f(-1, 	1, 0.0f);
				glEnd();
				glPopMatrix();
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
			if(drawmode==doublevisionmode){
				static float crosseyedness;
				crosseyedness=abs(player[0].damage-player[0].superpermanentdamage-(player[0].damagetolerance-player[0].superpermanentdamage)*1/2)/30;
				if(crosseyedness>1)crosseyedness=1;
				if(crosseyedness<0)crosseyedness=0;
				glColor4f(1,1,1,1);
				glDisable(GL_BLEND);
				glPushMatrix();
				glScalef(1,1,1);
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(-1,		-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,0);
				glVertex3f(1,	-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,texcoordheight);
				glVertex3f(1,	1, 0.0f);
				glTexCoord2f(0,texcoordheight);
				glVertex3f(-1, 	1, 0.0f);
				glEnd();
				glPopMatrix();
				if(crosseyedness){
					glColor4f(1,1,1,.5);
					glEnable(GL_BLEND);
					glPushMatrix();
					glTranslatef(.015*crosseyedness,0,0);
					glScalef(1,1,1);
					glBegin(GL_QUADS);
					glTexCoord2f(0,0);
					glVertex3f(-1,		-1, 	 0.0f);
					glTexCoord2f(texcoordwidth,0);
					glVertex3f(1,	-1, 	 0.0f);
					glTexCoord2f(texcoordwidth,texcoordheight);
					glVertex3f(1,	1, 0.0f);
					glTexCoord2f(0,texcoordheight);
					glVertex3f(-1, 	1, 0.0f);
					glEnd();
					glPopMatrix();
				}
			}
			if(drawmode==glowmode){
				glColor4f(.5,.5,.5,.5);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE);
				glPushMatrix();
				glTranslatef(.01,0,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(-1,		-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,0);
				glVertex3f(1,	-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,texcoordheight);
				glVertex3f(1,	1, 0.0f);
				glTexCoord2f(0,texcoordheight);
				glVertex3f(-1, 	1, 0.0f);
				glEnd();
				glPopMatrix();
				glPushMatrix();
				glTranslatef(-.01,0,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(-1,		-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,0);
				glVertex3f(1,	-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,texcoordheight);
				glVertex3f(1,	1, 0.0f);
				glTexCoord2f(0,texcoordheight);
				glVertex3f(-1, 	1, 0.0f);
				glEnd();
				glPopMatrix();
				glPushMatrix();
				glTranslatef(.0,.01,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(-1,		-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,0);
				glVertex3f(1,	-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,texcoordheight);
				glVertex3f(1,	1, 0.0f);
				glTexCoord2f(0,texcoordheight);
				glVertex3f(-1, 	1, 0.0f);
				glEnd();
				glPopMatrix();
				glPushMatrix();
				glTranslatef(0,-.01,0);
				glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(-1,		-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,0);
				glVertex3f(1,	-1, 	 0.0f);
				glTexCoord2f(texcoordwidth,texcoordheight);
				glVertex3f(1,	1, 0.0f);
				glTexCoord2f(0,texcoordheight);
				glVertex3f(-1, 	1, 0.0f);
				glEnd();
				glPopMatrix();
			}
			if(drawmode==radialzoommode){
				for(i=0;i<3;i++){
					//glRotatef((float)i*.1,0,0,1);
					glColor4f(1,1,1,1/((float)i+1));
					glPushMatrix();
					glScalef(1+(float)i*.01,1+(float)i*.01,1);
					glBegin(GL_QUADS);
					glTexCoord2f(0,0);
					glVertex3f(-1,		-1, 	 0.0f);
					glTexCoord2f(texcoordwidth,0);
					glVertex3f(1,	-1, 	 0.0f);
					glTexCoord2f(texcoordwidth,texcoordheight);
					glVertex3f(1,	1, 0.0f);
					glTexCoord2f(0,texcoordheight);
					glVertex3f(-1, 	1, 0.0f);
					glEnd();
					glPopMatrix();
				}
			}
			glDisable(GL_TEXTURE_2D);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
			glPopMatrix();										// Restore The Old Projection Matrix
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(1);
		}

		if(console){
			glEnable(GL_TEXTURE_2D);
			glColor4f(1,1,1,1);
			if(console){
				int offset;
				offset=0;
				if(consoleselected>=60)offset=consoleselected-60;
				sprintf (string, " ]");
				text.glPrint(10,30,string,0,1,1024,768);
				if(consoleblink){
					sprintf (string, "_");
					text.glPrint(30+(float)(consoleselected)*10-offset*10,30,string,0,1,1024,768);
				}
				for(i=0;i<15;i++){
					for(j=0;j<consolechars[i];j++){
						glColor4f(1,1,1,1-(float)(i)/16);
						if(j<consolechars[i]){
							sprintf (string, "%c",consoletext[i][j]);
							text.glPrint(30+j*10-offset*10,30+i*20,string,0,1,1024,768);
						}
					}
				}
			}
		}
	}

	if(freeze||winfreeze||(mainmenu&&gameon)||(!gameon&&gamestarted)||(mainmenu&&gameon)||(!gameon&&gamestarted)||(!gameon&&gamestarted)){
		multiplier=tempmult;
	}

	if(mainmenu){
#if USE_SDL
        // !!! FIXME: hack: clamp framerate in menu so text input works correctly on fast systems.
        SDL_Delay(15);
#endif
		glDrawBuffer(GL_BACK);
		glReadBuffer(GL_BACK);
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		ReSizeGLScene(90,.1f);

		int temptexdetail;
		temptexdetail=texdetail;
		if(texdetail>2)texdetail=2;
		if(mainmenu!=oldmainmenu&&oldmainmenu!=0){
			if(mainmenu==1){
				LoadTexture(":Data:Textures:Newgame.png",&Mainmenuitems[1],0,0);
				LoadTexture(":Data:Textures:Quit.png",&Mainmenuitems[3],0,0);
				/*if(oldmainmenu==1||oldmainmenu==0){
				LoadTexture(":Data:Textures:World.png",&Mainmenuitems[7],0,0);
				LoadTexture(":Data:Textures:Options.png",&Mainmenuitems[2],0,0);
				LoadTexture(":Data:Textures:Lugaru.png",&Mainmenuitems[0],0,0);
				loaddistrib=0;
				}*/
			}
			if(mainmenu==2){
				LoadTexture(":Data:Textures:Resume.png",&Mainmenuitems[1],0,0);
				LoadTexture(":Data:Textures:Endgame.png",&Mainmenuitems[3],0,0);
				/*if(oldmainmenu==2||oldmainmenu==0){
				LoadTexture(":Data:Textures:World.png",&Mainmenuitems[7],0,0);
				LoadTexture(":Data:Textures:Options.png",&Mainmenuitems[2],0,0);
				LoadTexture(":Data:Textures:Lugaru.png",&Mainmenuitems[0],0,0);
				loaddistrib=0;
				}*/
			}
		}
		if(lastcheck>.5||oldmainmenu!=mainmenu){
			if(mainmenu==5){
				ifstream ipstream(ConvertFileName(":Data:Campaigns:main.txt"));
				//campaignnumlevels=0;
				//accountcampaignchoicesmade[accountactive]=0;
				ipstream.ignore(256,':');
				ipstream >> campaignnumlevels;
				for(i=0;i<campaignnumlevels;i++){
					ipstream.ignore(256,':');
					ipstream.ignore(256,':');
					ipstream.ignore(256,' ');
					ipstream >> campaignmapname[i];
					ipstream.ignore(256,':');
					ipstream >> campaigndescription[i];
					for(j=0;j<256;j++){
						if(campaigndescription[i][j]=='_')campaigndescription[i][j]=' ';
					}
					ipstream.ignore(256,':');
					ipstream >> campaignchoosenext[i];
					ipstream.ignore(256,':');
					ipstream >> campaignnumnext[i];
					if(campaignnumnext[i])
						for(j=0;j<campaignnumnext[i];j++){
							ipstream.ignore(256,':');
							ipstream >> campaignnextlevel[i][j];
							campaignnextlevel[i][j]-=1;
						}
						ipstream.ignore(256,':');
						ipstream >> campaignlocationx[i];
						//campaignlocationx[i]-=30;
						ipstream.ignore(256,':');
						ipstream >> campaignlocationy[i];
						//campaignlocationy[i]+=30;
				}
				ipstream.close();

				for(i=0;i<campaignnumlevels;i++){
					levelvisible[i]=0;
					levelhighlight[i]=0;
				}

				levelorder[0]=0;
				levelvisible[0]=1;
				if(accountcampaignchoicesmade[accountactive])
					for(i=0;i<accountcampaignchoicesmade[accountactive];i++){
						levelorder[i+1]=campaignnextlevel[levelorder[i]][accountcampaignchoices[accountactive][i]];
						levelvisible[levelorder[i+1]]=1;
					}
					int whichlevelstart;
					whichlevelstart=accountcampaignchoicesmade[accountactive]-1;
					if(whichlevelstart<0){
						accountcampaignscore[accountactive]=0;
						accountcampaignfasttime[accountactive]=0;
						campaignchoicenum=1;
						campaignchoicewhich[0]=0;
					}
					else
					{
						campaignchoicenum=campaignnumnext[levelorder[whichlevelstart]];
						if(campaignchoicenum==0){
							if(accountcampaignscore[accountactive]>accountcampaignhighscore[accountactive])accountcampaignhighscore[accountactive]=accountcampaignscore[accountactive];
							if(accountcampaignfasttime[accountactive]==0||accountcampaigntime[accountactive]<accountcampaignfasttime[accountactive])accountcampaignfasttime[accountactive]=accountcampaigntime[accountactive];		
						}
						if(campaignchoicenum)
							for(i=0;i<campaignchoicenum;i++){
								campaignchoicewhich[i]=campaignnextlevel[levelorder[whichlevelstart]][i];
								levelvisible[campaignnextlevel[levelorder[whichlevelstart]][i]]=1;
								levelhighlight[campaignnextlevel[levelorder[whichlevelstart]][i]]=1;
							}
					}
					/*levelorder[0]=0;
					levelorder[1]=1;
					levelorder[2]=2;
					levelorder[3]=3;*/
			}
		}
		if(mainmenu==5){
			lastcheck=0;
		}

		texdetail=temptexdetail;

		/*if(mainmenu!=0)*/oldmainmenu=mainmenu;

		if(mainmenu==3||mainmenu==4||mainmenu==5||mainmenu==6||mainmenu==7||mainmenu==8||mainmenu==9||mainmenu==10||mainmenu==119||mainmenu==12||mainmenu==13||mainmenu==14||mainmenu==15||mainmenu==16||mainmenu==17){
			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.001f);
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDepthMask(0);
			glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPushMatrix();										// Store The Projection Matrix
				glLoadIdentity();									// Reset The Projection Matrix
				glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
				glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
					glPushMatrix();										// Store The Modelview Matrix
					glLoadIdentity();								// Reset The Modelview Matrix
					glTranslatef(screenwidth/2,screenheight/2,0);
					glPushMatrix();
						glScalef((float)screenwidth/2,(float)screenheight/2,1);
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						glDisable(GL_BLEND);
						glColor4f(0,0,0,1.0);
						glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
						glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
						glDisable(GL_TEXTURE_2D);
						glPushMatrix();
							//glScalef(.25,.25,.25);
							glBegin(GL_QUADS);
							glTexCoord2f(0,0);
							glVertex3f(-1,		-1, 	 0.0f);
							glTexCoord2f(1,0);
							glVertex3f(1,	-1, 	 0.0f);
							glTexCoord2f(1,1);
							glVertex3f(1,	1, 0.0f);
							glTexCoord2f(0,1);
							glVertex3f(-1, 	1, 0.0f);
							glEnd();
						glPopMatrix();
						glEnable(GL_BLEND);
						glColor4f(0.4,0.4,0.4,1.0);
						glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
						glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
						glEnable(GL_TEXTURE_2D);
						glBindTexture( GL_TEXTURE_2D, Mainmenuitems[4]);
						glPushMatrix();
							//glScalef(.25,.25,.25);
							glBegin(GL_QUADS);
							glTexCoord2f(0,0);
							glVertex3f(-1,		-1, 	 0.0f);
							glTexCoord2f(1,0);
							glVertex3f(1,	-1, 	 0.0f);
							glTexCoord2f(1,1);
							glVertex3f(1,	1, 0.0f);
							glTexCoord2f(0,1);
							glVertex3f(-1, 	1, 0.0f);
							glEnd();
						glPopMatrix();
					glPopMatrix();
				glPopMatrix();
				glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

			if(mainmenu==3){			
				nummenuitems=12;
				if((float)newscreenwidth>(float)newscreenheight*1.61||(float)newscreenwidth<(float)newscreenheight*1.59)sprintf (menustring[0], "Resolution: %d*%d",(int)newscreenwidth,(int)newscreenheight);
				else sprintf (menustring[0], "Resolution: %d*%d (widescreen)",(int)newscreenwidth,(int)newscreenheight);
				startx[0]=10+20;
				starty[0]=440;
				endx[0]=startx[0]+strlen(menustring[0])*10;
				endy[0]=starty[0]+20;
				movex[0]=0;
				movey[0]=0;

				if(newdetail==2)sprintf (menustring[1], "Detail: High");
				else if(newdetail==1)sprintf (menustring[1], "Detail: Medium");
				else sprintf (menustring[1], "Detail: Low");
				startx[1]=10+60;
				starty[1]=405;
				endx[1]=startx[1]+strlen(menustring[1])*10;
				endy[1]=starty[1]+20;
				movex[1]=0;
				movey[1]=0;

				if(bloodtoggle==2)sprintf (menustring[2], "Blood: On, high detail (slower)");
				if(bloodtoggle==1)sprintf (menustring[2], "Blood: On, low detail");
				if(bloodtoggle==0)sprintf (menustring[2], "Blood: Off");
				startx[2]=10+70;
				starty[2]=370;
				endx[2]=startx[2]+strlen(menustring[2])*10;
				endy[2]=starty[2]+20;
				movex[2]=0;
				movey[2]=0;

				if(difficulty==2)sprintf (menustring[3], "Difficulty: Insane");
				if(difficulty==1)sprintf (menustring[3], "Difficulty: Difficult");
				if(difficulty==0)sprintf (menustring[3], "Difficulty: Easier");
				startx[3]=10+20-1000;
				starty[3]=335-1000;
				endx[3]=startx[3]+strlen(menustring[3])*10;
				endy[3]=starty[3]+20;
				movex[3]=0;
				movey[3]=0;

				if(ismotionblur==1)sprintf (menustring[4], "Blur Effects: Enabled (less compatible)");
				if(ismotionblur==0)sprintf (menustring[4], "Blur Effects: Disabled (more compatible)");
				startx[4]=10;
				starty[4]=335;
				endx[4]=startx[4]+strlen(menustring[4])*10;
				endy[4]=starty[4]+20;
				movex[4]=0;
				movey[4]=0;

				if(decals==1)sprintf (menustring[5], "Decals: Enabled (slower)");
				if(decals==0)sprintf (menustring[5], "Decals: Disabled");
				startx[5]=10+60;
				starty[5]=300;
				endx[5]=startx[5]+strlen(menustring[5])*10;
				endy[5]=starty[5]+20;
				movex[5]=0;
				movey[5]=0;

				if(musictoggle==1)sprintf (menustring[6], "Music: Enabled");
				if(musictoggle==0)sprintf (menustring[6], "Music: Disabled");
				startx[6]=10+70;
				starty[6]=265;
				endx[6]=startx[6]+strlen(menustring[6])*10;
				endy[6]=starty[6]+20;
				movex[6]=0;
				movey[6]=0;

				if(invertmouse==1)sprintf (menustring[9], "Invert mouse: Yes");
				if(invertmouse==0)sprintf (menustring[9], "Invert mouse: No");
				startx[9]=10;
				starty[9]=230;
				endx[9]=startx[9]+strlen(menustring[9])*10;
				endy[9]=starty[9]+20;
				movex[9]=0;
				movey[9]=0;

				sprintf (menustring[10], "Mouse Speed: %d", (int)(usermousesensitivity*5));
				startx[10]=20;
				starty[10]=195;
				endx[10]=startx[10]+strlen(menustring[10])*10;
				endy[10]=starty[10]+20;
				movex[10]=0;
				movey[10]=0;
				
				sprintf (menustring[11], "Volume: %d%%", (int)(volume*100));
				startx[11]=10+60;
				starty[11]=155;
				endx[11]=startx[11]+strlen(menustring[11])*10;
				endy[11]=starty[11]+20;
				movex[11]=0;
				movey[11]=0;
				
				sprintf (menustring[7], "-Configure Controls-");
				startx[7]=10+15;
				starty[7]=100;
				endx[7]=startx[7]+strlen(menustring[7])*10;
				endy[7]=starty[7]+20;
				movex[7]=0;
				movey[7]=0;

				if(newdetail==detail&&newscreenheight==(int)screenheight&&newscreenwidth==(int)screenwidth)sprintf (menustring[8], "Back");
				else sprintf (menustring[8], "Back (some changes take effect next time Lugaru is opened)");
				startx[8]=10;
				endx[8]=startx[8]+strlen(menustring[8])*10;
				starty[8]=10;
				endy[8]=starty[8]+20;
				movex[8]=0;
				movey[8]=0;
			}

			if(mainmenu==4){			
				nummenuitems=10;
				if(keyselect!=0)sprintf (menustring[0], "Forwards: %s",KeyToChar(forwardkey));
				else sprintf (menustring[0], "Forwards: _");
				startx[0]=10;
				starty[0]=400;
				endx[0]=startx[0]+strlen(menustring[0])*10;
				endy[0]=starty[0]+20;
				movex[0]=0;
				movey[0]=0;

				if(keyselect!=1)sprintf (menustring[1], "Back: %s",KeyToChar(backkey));
				else sprintf (menustring[1], "Back: _");
				startx[1]=10+40;
				starty[1]=360;
				endx[1]=startx[1]+strlen(menustring[1])*10;
				endy[1]=starty[1]+20;
				movex[1]=0;
				movey[1]=0;

				if(keyselect!=2)sprintf (menustring[2], "Left: %s",KeyToChar(leftkey));
				else sprintf (menustring[2], "Left: _");
				startx[2]=10+40;
				starty[2]=320;
				endx[2]=startx[2]+strlen(menustring[2])*10;
				endy[2]=starty[2]+20;
				movex[2]=0;
				movey[2]=0;

				if(keyselect!=3)sprintf (menustring[3], "Right: %s",KeyToChar(rightkey));
				else sprintf (menustring[3], "Right: _");
				startx[3]=10+30;
				starty[3]=280;
				endx[3]=startx[3]+strlen(menustring[3])*10;
				endy[3]=starty[3]+20;
				movex[3]=0;
				movey[3]=0;

				if(keyselect!=4)sprintf (menustring[4], "Crouch: %s",KeyToChar(crouchkey));
				else sprintf (menustring[4], "Crouch: _");
				startx[4]=10+20;
				starty[4]=240;
				endx[4]=startx[4]+strlen(menustring[4])*10;
				endy[4]=starty[4]+20;
				movex[4]=0;
				movey[4]=0;

				if(keyselect!=5)sprintf (menustring[5], "Jump: %s",KeyToChar(jumpkey));
				else sprintf (menustring[5], "Jump: _");
				startx[5]=10+40;
				starty[5]=200;
				endx[5]=startx[5]+strlen(menustring[5])*10;
				endy[5]=starty[5]+20;
				movex[5]=0;
				movey[5]=0;

				if(keyselect!=6)sprintf (menustring[6], "Draw: %s",KeyToChar(drawkey));
				else sprintf (menustring[6], "Draw: _");
				startx[6]=10+40;
				starty[6]=160;
				endx[6]=startx[6]+strlen(menustring[6])*10;
				endy[6]=starty[6]+20;
				movex[6]=0;
				movey[6]=0;

				if(keyselect!=7)sprintf (menustring[7], "Throw: %s",KeyToChar(throwkey));
				else sprintf (menustring[7], "Throw: _");
				startx[7]=10+30;
				starty[7]=120;
				endx[7]=startx[7]+strlen(menustring[7])*10;
				endy[7]=starty[7]+20;
				movex[7]=0;
				movey[7]=0;

				if(keyselect!=8)sprintf (menustring[8], "Attack: %s",KeyToChar(attackkey));
				else sprintf (menustring[8], "Attack: _");
				startx[8]=10+20;
				starty[8]=80;
				endx[8]=startx[8]+strlen(menustring[8])*10;
				endy[8]=starty[8]+20;
				movex[8]=0;
				movey[8]=0;



				sprintf (menustring[9], "Back");
				startx[9]=10;
				endx[9]=startx[9]+strlen(menustring[9])*10;
				starty[9]=10;
				endy[9]=starty[9]+20;
				movex[9]=0;
				movey[9]=0;
			}
			if(mainmenu==5){			
				nummenuitems=7+accountcampaignchoicesmade[accountactive]+campaignchoicenum;

				sprintf (menustring[0], "%s",accountname[accountactive]);
				startx[0]=5;
				starty[0]=400;
				endx[0]=startx[0]+strlen(menustring[0])*10;
				endy[0]=starty[0]+20;
				movex[0]=0;
				movey[0]=0;

				sprintf (menustring[1], "Tutorial");
				startx[1]=5;
				starty[1]=300;
				endx[1]=startx[1]+strlen(menustring[1])*10;
				endy[1]=starty[1]+20;
				movex[1]=0;
				movey[1]=0;

				sprintf (menustring[2], "Challenge");
				startx[2]=5;
				starty[2]=240;
				endx[2]=startx[2]+strlen(menustring[2])*10;
				endy[2]=starty[2]+20;
				movex[2]=0;
				movey[2]=0;

				sprintf (menustring[3], "Delete User");
				startx[3]=400;
				starty[3]=10;
				endx[3]=startx[3]+strlen(menustring[3])*10;
				endy[3]=starty[3]+20;
				movex[3]=0;
				movey[3]=0;

				sprintf (menustring[4], "Main Menu");
				startx[4]=5;
				starty[4]=10;
				endx[4]=startx[4]+strlen(menustring[4])*10;
				endy[4]=starty[4]+20;
				movex[4]=0;
				movey[4]=0;

				sprintf (menustring[5], "Change User");
				startx[5]=5;
				endx[5]=startx[5]+strlen(menustring[5])*10;
				starty[5]=180;
				endy[5]=starty[5]+20;
				movex[5]=0;
				movey[5]=0;

				//World

				sprintf (menustring[6], "World");
				startx[6]=30+120;
				starty[6]=30+480-400-50;
				endx[6]=startx[6]+400;
				endy[6]=30+480-50;
				movex[6]=0;
				movey[6]=0;

				if(accountcampaignchoicesmade[accountactive])
					for(i=0;i<accountcampaignchoicesmade[accountactive];i++){
						sprintf (menustring[7+i], campaigndescription[levelorder[i]]);
						startx[7+i]=30+120+campaignlocationx[levelorder[i]]*400/512;
						starty[7+i]=30+30+(512-campaignlocationy[levelorder[i]])*400/512;
						endx[7+i]=startx[7+i]+10;
						endy[7+i]=starty[7+i]+10;
						movex[7+i]=0;
						movey[7+i]=0;
					}

					if(campaignchoicenum>0)
						for(i=accountcampaignchoicesmade[accountactive];i<accountcampaignchoicesmade[accountactive]+campaignchoicenum;i++){
							sprintf (menustring[7+i], campaigndescription[levelorder[i]]);
							startx[7+i]=30+120+campaignlocationx[campaignchoicewhich[i-(accountcampaignchoicesmade[accountactive])]]*400/512;
							starty[7+i]=30+30+(512-campaignlocationy[campaignchoicewhich[i-(accountcampaignchoicesmade[accountactive])]])*400/512;
							endx[7+i]=startx[7+i]+10;
							endy[7+i]=starty[7+i]+10;
							movex[7+i]=0;
							movey[7+i]=0;
						}

						/*sprintf (menustring[7], "Dot");
						startx[7]=120+260*400/512;
						starty[7]=30+(512-184)*400/512;
						endx[7]=startx[7]+10;
						endy[7]=starty[7]+10;
						movex[7]=0;
						movey[7]=0;

						sprintf (menustring[8], "Dot");
						startx[8]=120+129*400/512;
						starty[8]=30+(512-284)*400/512;
						endx[8]=startx[8]+10;
						endy[8]=starty[8]+10;
						movex[8]=0;
						movey[8]=0;

						sprintf (menustring[9], "Dot");
						startx[9]=120+358*400/512;
						starty[9]=30+(512-235)*400/512;
						endx[9]=startx[9]+10;
						endy[9]=starty[9]+10;
						movex[9]=0;
						movey[9]=0;

						sprintf (menustring[10], "Dot");
						startx[10]=120+359*400/512;
						starty[10]=30+(512-308)*400/512;
						endx[10]=startx[10]+10;
						endy[10]=starty[10]+10;
						movex[10]=0;
						movey[10]=0;

						sprintf (menustring[11], "Dot");
						startx[11]=120+288*400/512;
						starty[11]=30+(512-277)*400/512;
						endx[11]=startx[11]+10;
						endy[11]=starty[11]+10;
						movex[11]=0;
						movey[11]=0;*/
			}

			if(mainmenu==6){			
				nummenuitems=3;

				sprintf (menustring[0], "Are you sure you want to delete this user?");
				startx[0]=10;
				starty[0]=400;
				endx[0]=startx[0]+strlen(menustring[0])*10;
				endy[0]=starty[0]+20;
				movex[0]=0;
				movey[0]=0;

				sprintf (menustring[1], "Yes");
				startx[1]=10;
				starty[1]=360;
				endx[1]=startx[1]+strlen(menustring[1])*10;
				endy[1]=starty[1]+20;
				movex[1]=0;
				movey[1]=0;

				sprintf (menustring[2], "No");
				startx[2]=10;
				starty[2]=320;
				endx[2]=startx[2]+strlen(menustring[2])*10;
				endy[2]=starty[2]+20;
				movex[2]=0;
				movey[2]=0;

				sprintf (menustring[3], "Extra 4");
				startx[3]=10;
				starty[3]=280;
				endx[3]=startx[3]+strlen(menustring[3])*10;
				endy[3]=starty[3]+20;
				movex[3]=0;
				movey[3]=0;

				sprintf (menustring[4], "Extra 5");
				startx[4]=10;
				starty[4]=240;
				endx[4]=startx[4]+strlen(menustring[4])*10;
				endy[4]=starty[4]+20;
				movex[4]=0;
				movey[4]=0;

				sprintf (menustring[5], "Back");
				startx[5]=10;
				endx[5]=startx[5]+strlen(menustring[5])*10;
				starty[5]=10;
				endy[5]=starty[5]+20;
				movex[5]=0;
				movey[5]=0;
			}

			if(mainmenu==7){			
				nummenuitems=numaccounts+2;

				int num;

				if(numaccounts<8)
					sprintf (menustring[0], "New User");
				else
					sprintf (menustring[0], "No More Users");
				startx[0]=10;
				starty[0]=400;
				endx[0]=startx[0]+strlen(menustring[0])*10;
				endy[0]=starty[0]+20;
				movex[0]=0;
				movey[0]=0;

				if(entername)startx[0]+=10;


				num=1;
				if(numaccounts)
					for(i=0;i<numaccounts;i++){
						sprintf (menustring[num], "%s",accountname[i]);
						startx[num]=10;
						starty[num]=360-20-20*num;
						endx[num]=startx[num]+strlen(menustring[num])*10;
						endy[num]=starty[num]+20;
						movex[num]=0;
						movey[num]=0;

						num++;
					}

					sprintf (menustring[num], "Back");
					startx[num]=10;
					endx[num]=startx[num]+strlen(menustring[num])*10;
					starty[num]=10;
					endy[num]=starty[num]+20;
					movex[num]=0;
					movey[num]=0;
			}
			if(mainmenu==8){			
				nummenuitems=3;

				sprintf (menustring[0], "Easier");
				startx[0]=10;
				starty[0]=400;
				endx[0]=startx[0]+strlen(menustring[0])*10;
				endy[0]=starty[0]+20;
				movex[0]=0;
				movey[0]=0;

				sprintf (menustring[1], "Difficult");
				startx[1]=10;
				starty[1]=360;
				endx[1]=startx[1]+strlen(menustring[1])*10;
				endy[1]=starty[1]+20;
				movex[1]=0;
				movey[1]=0;

				sprintf (menustring[2], "Insane");
				startx[2]=10;
				starty[2]=320;
				endx[2]=startx[2]+strlen(menustring[2])*10;
				endy[2]=starty[2]+20;
				movex[2]=0;
				movey[2]=0;
			}
			if(mainmenu==9){			
				int tempncl;
				//tempncl=numchallengelevels;
				//numchallengelevels=9;
				nummenuitems=2+numchallengelevels;
				char temp[255];

				for(j=0;j<numchallengelevels;j++){
					for(i=0;i<255;i++)menustring[j][i]='\0';
					sprintf (temp, "Level %d",j+1);
					strcpy(menustring[j],temp);
					for(i=0;i<17;i++)if(menustring[j][i]=='\0')menustring[j][i]=' ';
					menustring[j][17]='\0';
					sprintf (temp, "%d",(int)accounthighscore[accountactive][j]);
					strcat(menustring[j],temp);
					for(i=18;i<32;i++)if(menustring[j][i]=='\0')menustring[j][i]=' ';
					menustring[j][32]='\0';
					sprintf (temp, "%d:",(int)(((int)accountfasttime[accountactive][j]-(int)(accountfasttime[accountactive][j])%60)/60));
					strcat(menustring[j],temp);
					if((int)(accountfasttime[accountactive][j])%60<10)strcat(menustring[j],"0");
					sprintf (temp, "%d",(int)(accountfasttime[accountactive][j])%60);
					strcat(menustring[j],temp);

					startx[j]=10;
					starty[j]=400-j*25;
					endx[j]=startx[j]+strlen(menustring[j])*10;
					endy[j]=starty[j]+20;
					movex[j]=0;
					movey[j]=0;
				}

				sprintf (menustring[numchallengelevels], "Back");
				startx[numchallengelevels]=10;
				endx[numchallengelevels]=startx[numchallengelevels]+strlen(menustring[numchallengelevels])*10;
				starty[numchallengelevels]=10;
				endy[numchallengelevels]=starty[numchallengelevels]+20;
				movex[numchallengelevels]=0;
				movey[numchallengelevels]=0;

				sprintf (menustring[numchallengelevels+1], "             High Score      Best Time");
				startx[numchallengelevels+1]=10;
				starty[numchallengelevels+1]=440;
				endx[numchallengelevels+1]=startx[numchallengelevels+1]+strlen(menustring[numchallengelevels+1])*10;
				endy[numchallengelevels+1]=starty[numchallengelevels+1]+20;
				movex[numchallengelevels+1]=0;
				movey[numchallengelevels+1]=0;

				//numchallengelevels=tempncl;

			}
			if(mainmenu==11){			
				nummenuitems=2+numchallengelevels;
				char temp[255];

				for(j=0;j<numchallengelevels;j++){
					for(i=0;i<255;i++)menustring[j][i]='\0';
					sprintf (temp, "Level %d",j+1);
					strcpy(menustring[j],temp);
					for(i=0;i<17;i++)if(menustring[j][i]=='\0')menustring[j][i]=' ';
					menustring[j][17]='\0';
					sprintf (temp, "%d",(int)accounthighscore[accountactive][j]);
					strcat(menustring[j],temp);
					for(i=18;i<32;i++)if(menustring[j][i]=='\0')menustring[j][i]=' ';
					menustring[j][32]='\0';
					sprintf (temp, "%d:",(int)(((int)accountfasttime[accountactive][j]-(int)(accountfasttime[accountactive][j])%60)/60));
					strcat(menustring[j],temp);
					if((int)(accountfasttime[accountactive][j])%60<10)strcat(menustring[j],"0");
					sprintf (temp, "%d",(int)(accountfasttime[accountactive][j])%60);
					strcat(menustring[j],temp);

					startx[j]=10;
					starty[j]=360-j*40;
					endx[j]=startx[j]+strlen(menustring[j])*10;
					endy[j]=starty[j]+20;
					movex[j]=0;
					movey[j]=0;
				}

				sprintf (menustring[numchallengelevels], "Back");
				startx[numchallengelevels]=10;
				endx[numchallengelevels]=startx[numchallengelevels]+strlen(menustring[numchallengelevels])*10;
				starty[numchallengelevels]=10;
				endy[numchallengelevels]=starty[numchallengelevels]+20;
				movex[numchallengelevels]=0;
				movey[numchallengelevels]=0;

				sprintf (menustring[numchallengelevels+1], "             High Score      Best Time");
				startx[numchallengelevels+1]=10;
				starty[numchallengelevels+1]=400;
				endx[numchallengelevels+1]=startx[numchallengelevels+1]+strlen(menustring[numchallengelevels+1])*10;
				endy[numchallengelevels+1]=starty[numchallengelevels+1]+20;
				movex[numchallengelevels+1]=0;
				movey[numchallengelevels+1]=0;

			}
			if(mainmenu==10){			
				nummenuitems=6;
				char temp[255];

				sprintf (menustring[0], "Congratulations!");
				startx[0]=220;
				starty[0]=330;
				endx[0]=startx[0]+strlen(menustring[0])*10;
				endy[0]=starty[0]+20;
				movex[0]=0;
				movey[0]=0;

				sprintf (menustring[1], "You have avenged your family and");
				startx[1]=140;
				starty[1]=300;
				endx[1]=startx[1]+strlen(menustring[1])*10;
				endy[1]=starty[1]+20;
				movex[1]=0;
				movey[1]=0;

				sprintf (menustring[2], "restored peace to the island of Lugaru.");
				startx[2]=110;
				starty[2]=270;
				endx[2]=startx[2]+strlen(menustring[2])*10;
				endy[2]=starty[2]+20;
				movex[2]=0;
				movey[2]=0;

				sprintf (menustring[3], "Back");
				startx[3]=10;
				endx[3]=startx[3]+strlen(menustring[3])*10;
				starty[3]=10;
				endy[3]=starty[3]+20;
				movex[3]=0;
				movey[3]=0;

				for(i=0;i<255;i++)menustring[4][i]='\0';
				sprintf (temp, "Your score:");
				strcpy(menustring[4],temp);
				for(i=0;i<20;i++)if(menustring[4][i]=='\0')menustring[4][i]=' ';
				menustring[4][20]='\0';
				sprintf (temp, "%d",(int)accountcampaignscore[accountactive]);
				strcat(menustring[4],temp);
				startx[4]=190;
				endx[4]=startx[4]+strlen(menustring[4])*10;
				starty[4]=200;
				endy[4]=starty[4]+20;
				movex[4]=0;
				movey[4]=0;
				/*
				for(i=0;i<255;i++)menustring[5][i]='\0';
				sprintf (temp, "Your time:");
				strcpy(menustring[5],temp);
				for(i=0;i<20;i++)if(menustring[5][i]=='\0')menustring[5][i]=' ';
				menustring[5][20]='\0';
				sprintf (temp, "%d",(int)accountcampaigntime[accountactive]);
				strcat(menustring[5],temp);
				startx[5]=200;
				endx[5]=startx[5]+strlen(menustring[5])*10;
				starty[5]=180;
				endy[5]=starty[5]+20;
				movex[5]=0;
				movey[5]=0;
				*/
				for(i=0;i<255;i++)menustring[5][i]='\0';
				sprintf (temp, "Highest score:");
				strcpy(menustring[5],temp);
				for(i=0;i<20;i++)if(menustring[5][i]=='\0')menustring[5][i]=' ';
				menustring[5][20]='\0';
				sprintf (temp, "%d",(int)accountcampaignhighscore[accountactive]);
				strcat(menustring[5],temp);
				startx[5]=190;
				endx[5]=startx[5]+strlen(menustring[5])*10;
				starty[5]=180;
				endy[5]=starty[5]+20;
				movex[5]=0;
				movey[5]=0;
				/*
				for(i=0;i<255;i++)menustring[7][i]='\0';
				sprintf (temp, "Lowest time:");
				strcpy(menustring[7],temp);
				for(i=0;i<20;i++)if(menustring[7][i]=='\0')menustring[7][i]=' ';
				menustring[7][20]='\0';
				sprintf (temp, "%d",(int)accountcampaignfasttime[accountactive]);
				strcat(menustring[7],temp);
				startx[7]=200;
				endx[7]=startx[7]+strlen(menustring[7])*10;
				starty[7]=130;
				endy[7]=starty[7]+20;
				movex[7]=0;
				movey[7]=0;*/
			}
		}

		if(mainmenu==12){	
			menupulse+=multiplier*2;

			nummenuitems=6;
			char temp[255];

			sprintf (menustring[0], "Register now for only $19.95!");
			startx[0]=160;
			starty[0]=270;
			endx[0]=startx[0]+strlen(menustring[0])*10;
			endy[0]=starty[0]+20;
			movex[0]=0;
			movey[0]=0;

			sprintf (menustring[1], "Confront raiders, wolves, and more!");
			startx[1]=130;
			starty[1]=240;
			endx[1]=startx[1]+strlen(menustring[1])*10;
			endy[1]=starty[1]+20;
			movex[1]=0;
			movey[1]=0;

			sprintf (menustring[2], "Fight using swords, staves and armor!");
			startx[2]=125;
			starty[2]=210;
			endx[2]=startx[2]+strlen(menustring[2])*10;
			endy[2]=starty[2]+20;
			movex[2]=0;
			movey[2]=0;

			if(!tryquit)sprintf (menustring[3], "Back");
			else sprintf (menustring[3], "Quit");
			startx[3]=10;
			endx[3]=startx[3]+strlen(menustring[3])*10;
			starty[3]=10;
			endy[3]=starty[3]+20;
			movex[3]=0;
			movey[3]=0;

			sprintf (menustring[4], "Register now!");
			startx[4]=250;
			endx[4]=startx[4]+strlen(menustring[4])*10;
			starty[4]=140;
			endy[4]=starty[4]+20;
			movex[4]=0;
			movey[4]=0;

			sprintf (menustring[5], "Enter registration code!");
			startx[5]=190;
			endx[5]=startx[5]+strlen(menustring[5])*10;
			starty[5]=120;
			endy[5]=starty[5]+20;
			movex[5]=0;
			movey[5]=0;
		}

		if(mainmenu==15){	
			nummenuitems=2;
			char temp[255];

			sprintf (menustring[0], "Thank you for supporting Wolfire Software!");
			startx[0]=100;
			starty[0]=270;
			endx[0]=startx[0]+strlen(menustring[0])*10;
			endy[0]=starty[0]+20;
			movex[0]=0;
			movey[0]=0;

			sprintf (menustring[1], "Back");
			startx[1]=10;
			endx[1]=startx[1]+strlen(menustring[1])*10;
			starty[1]=10;
			endy[1]=starty[1]+20;
			movex[1]=0;
			movey[1]=0;
		}

		if(mainmenu==16){	
			nummenuitems=5;
			char temp[255];

			sprintf (menustring[0], "Sorry, that name/serial number combination is incorrect.");
			startx[0]=40;
			starty[0]=270;
			endx[0]=startx[0]+strlen(menustring[0])*10;
			endy[0]=starty[0]+20;
			movex[0]=0;
			movey[0]=0;

			sprintf (menustring[1], "Back");
			startx[1]=10;
			endx[1]=startx[1]+strlen(menustring[1])*10;
			starty[1]=10;
			endy[1]=starty[1]+20;
			movex[1]=0;
			movey[1]=0;

			sprintf (menustring[2], "Please make sure you are copying your name and serial");
			startx[2]=50;
			starty[2]=240;
			endx[2]=startx[2]+strlen(menustring[2])*10;
			endy[2]=starty[2]+20;
			movex[2]=0;
			movey[2]=0;

			sprintf (menustring[3], "number exactly as they appear in your email.");
			startx[3]=90;
			starty[3]=210;
			endx[3]=startx[3]+strlen(menustring[3])*10;
			endy[3]=starty[3]+20;
			movex[3]=0;
			movey[3]=0;

			sprintf (menustring[4], "Capitalization and punctuation matter!");
			startx[4]=120;
			starty[4]=180;
			endx[4]=startx[4]+strlen(menustring[4])*10;
			endy[4]=starty[4]+20;
			movex[4]=0;
			movey[4]=0;
		}

		if(mainmenu==13){	
			nummenuitems=2;
			char temp[255];

			sprintf (menustring[0], "Please enter your name:");
			startx[0]=50;
			starty[0]=250;
			endx[0]=startx[0]+strlen(menustring[0])*10;
			endy[0]=starty[0]+20;
			movex[0]=0;
			movey[0]=0;

			sprintf (menustring[1], "Please enter your name:");
			startx[1]=290;
			starty[1]=250;
			endx[1]=startx[1]+strlen(menustring[1])*10;
			endy[1]=starty[1]+20;
			movex[1]=0;
			movey[1]=0;
		}

		if(mainmenu==14){	
			nummenuitems=2;
			char temp[255];

			sprintf (menustring[0], "Please enter your number:");
			startx[0]=30;
			starty[0]=250;
			endx[0]=startx[0]+strlen(menustring[0])*10;
			endy[0]=starty[0]+20;
			movex[0]=0;
			movey[0]=0;

			sprintf (menustring[1], "Please enter your name:");
			startx[1]=290;
			starty[1]=250;
			endx[1]=startx[1]+strlen(menustring[1])*10;
			endy[1]=starty[1]+20;
			movex[1]=0;
			movey[1]=0;
			/*
			char tempstring[256];
			sprintf (tempstring, "%s", registrationname);
			long num1;
			long num2;
			long num3;
			long num4;
			long long longnum;
			longnum = MD5_string ( tempstring);
			//longnum = 1111111111111111;
			num1 = longnum/100000000;
			num2 = longnum%100000000;
			sprintf (tempstring, "%d-%d-%d-%d", num1/10000, num1%10000, num2/10000, num2%10000);

			sprintf (menustring[2], "%s", tempstring);
			startx[2]=290;
			starty[2]=230;
			endx[2]=startx[2]+strlen(menustring[2])*10;
			endy[2]=starty[2]+20;
			movex[2]=0;
			movey[2]=0;				*/	
		}

		if(mainmenu==1||mainmenu==2){
			nummenuitems=7;
			startx[0]=150;
			starty[0]=480-128;
			endx[0]=150+256;
			endy[0]=480;
			movex[0]=0;
			movey[0]=0;

			startx[1]=18;
			starty[1]=480-152-32;
			endx[1]=18+128;
			endy[1]=480-152;
			movex[1]=0;
			movey[1]=0;

			startx[2]=18;
			starty[2]=480-228-32;
			endx[2]=2+128;
			endy[2]=480-228;
			movex[2]=0;
			movey[2]=0;

			if(mainmenu==1){
				startx[3]=18;
				starty[3]=480-306-32;
				endx[3]=22+64;
				endy[3]=480-306;
				movex[3]=0;
				movey[3]=0;
			}

			if(mainmenu==2){
				startx[3]=18;
				starty[3]=480-306-32;
				endx[3]=22+128;
				endy[3]=480-306;
				movex[3]=0;
				movey[3]=0;
			}

			/*startx[4]=150;
			starty[4]=480-256;
			endx[4]=150+256;
			endy[4]=480;
			*/
			if(anim==0){
				startx[4]=380;
				starty[4]=480-140-256;
				endx[4]=380+256;
				endy[4]=480-140;
				movex[4]=80;
				movey[4]=0;

				startx[5]=145;
				starty[5]=480-138-256;
				endx[5]=145+256;
				endy[5]=480-138;
				movex[5]=40;
				movey[5]=0;

				startx[6]=254;
				starty[6]=480-144-256;
				endx[6]=254+256;
				endy[6]=480-144;
				movex[6]=20;
				movey[6]=0;
			}
			if(anim==1){
				startx[4]=180;
				starty[4]=480-140-256;
				endx[4]=180+256;
				endy[4]=480-140;
				movex[4]=80;
				movey[4]=0;

				startx[5]=500;
				starty[5]=480-138-256;
				endx[5]=500+256;
				endy[5]=480-138;
				movex[5]=40;
				movey[5]=0;

				startx[6]=340;
				starty[6]=480-144-256;
				endx[6]=340+256;
				endy[6]=480-144;
				movex[6]=20;
				movey[6]=0;
			}
			if(anim==2){
				startx[4]=460;
				starty[4]=480-140-256;
				endx[4]=460+256;
				endy[4]=480-140;
				movex[4]=50;
				movey[4]=0;

				startx[5]=295;
				starty[5]=480-150-256;
				endx[5]=295+256;
				endy[5]=480-138;
				movex[5]=-10;
				movey[5]=0;

				startx[6]=204;
				starty[6]=480-144-256;
				endx[6]=204+256;
				endy[6]=480-144;
				movex[6]=-30;
				movey[6]=0;
			}
			if(anim==3){
				startx[4]=150;
				starty[4]=480-140-256;
				endx[4]=200+256;
				endy[4]=480-140;
				movex[4]=80;
				movey[4]=0;

				startx[5]=350;
				starty[5]=480-150-256;
				endx[5]=350+256;
				endy[5]=480-138;
				movex[5]=5;
				movey[5]=0;

				startx[6]=500;
				starty[6]=480-144-256;
				endx[6]=500+256;
				endy[6]=480-144;
				movex[6]=-10;
				movey[6]=0;
			}
			if(anim==4){
				startx[4]=190;
				starty[4]=480-100-256;
				endx[4]=190+256;
				endy[4]=480-100;
				movex[4]=-30;
				movey[4]=0;

				startx[5]=185;
				starty[5]=480-120-256;
				endx[5]=185+256;
				endy[5]=480-120;
				movex[5]=-5;
				movey[5]=0;

				startx[6]=400;
				starty[6]=480-144-256;
				endx[6]=400+256;
				endy[6]=480-144;
				movex[6]=20;
				movey[6]=0;
			}
		}

		selected=-1;

		if(mainmenu==1||mainmenu==2)
			for(i=1;i<4;i++){
				if((mousecoordh/screenwidth*640)>startx[i]&&(mousecoordh/screenwidth*640)<endx[i]&&480-(mousecoordv/screenheight*480)>starty[i]&&480-(mousecoordv/screenheight*480)<endy[i]){
					selected=i;
				}
			}

			if(mainmenu==3||mainmenu==4||mainmenu==5||mainmenu==6||mainmenu==7||mainmenu==8||mainmenu==9||mainmenu==10||mainmenu==11||mainmenu==12||mainmenu==13||mainmenu==14||mainmenu==15||mainmenu==16||mainmenu==17)
				for(i=0;i<nummenuitems;i++){
					if((mousecoordh/screenwidth*640)>startx[i]&&(mousecoordh/screenwidth*640)<endx[i]&&480-(mousecoordv/screenheight*480)>starty[i]&&480-(mousecoordv/screenheight*480)<endy[i]){
						if(mainmenu!=5)selected=i;
						if(mainmenu==5&&(i!=0&&i!=6))selected=i;
						if(mainmenu==9&&(i!=numchallengelevels+1))selected=i;
						if(mainmenu==11&&(i!=numchallengelevels+1))selected=i;
					}
				}

				if(nummenuitems>0)
					for(i=0;i<nummenuitems;i++){
						if(selected==i)selectedlong[i]+=multiplier*5;
						if(selectedlong[i]>1)selectedlong[i]=1;
						if(selected!=i)selectedlong[i]-=multiplier*5;
						if(selectedlong[i]<0)selectedlong[i]=0;	
						//if(i>=4)selectedlong[i]=.3;		
						if(i>=4&&(mainmenu==1||mainmenu==2))selectedlong[i]=0;	
					}

					if(nummenuitems>0)
						for(i=0;i<nummenuitems;i++){
							offsetx[i]=(startx[i]+endx[i])/2-(mousecoordh/screenwidth*640);
							offsety[i]=(starty[i]+endy[i])/2-(480-(mousecoordv/screenheight*480));
							offsetx[i]*=.06f;
							offsety[i]*=.06f;
							offsetx[i]=0;
							offsety[i]=0;
							if(i>=4&&(mainmenu==1||mainmenu==2)){
								offsetx[i]=(startx[i]+endx[i]+movex[i]*transition)/2-(640+190)/2;
								offsety[i]=(starty[i]+endy[i]+movey[i]*transition)/2-(336+150)/2;
								offsetx[i]*=.06f;
								offsety[i]*=.06f;
							}
						}

						if(mainmenu==1||mainmenu==2){
							glClear(GL_DEPTH_BUFFER_BIT);
							glEnable(GL_ALPHA_TEST);
							glAlphaFunc(GL_GREATER, 0.001f);
							glEnable(GL_TEXTURE_2D);
							glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
							glDisable(GL_CULL_FACE);
							glDisable(GL_LIGHTING);
							glDepthMask(0);
							glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
							glPushMatrix();										// Store The Projection Matrix
								glLoadIdentity();									// Reset The Projection Matrix
								glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
								glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
								glPushMatrix();										// Store The Modelview Matrix
									glLoadIdentity();								// Reset The Modelview Matrix
									glTranslatef(screenwidth/2,screenheight/2,0);
									glPushMatrix();
										glScalef((float)screenwidth/2,(float)screenheight/2,1);
										glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
										glDisable(GL_BLEND);
										glColor4f(0,0,0,1.0);
										glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
										glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
										glDisable(GL_TEXTURE_2D);
										glPushMatrix();
											//glScalef(.25,.25,.25);
											glBegin(GL_QUADS);
											glTexCoord2f(0,0);
											glVertex3f(-1,		-1, 	 0.0f);
											glTexCoord2f(1,0);
											glVertex3f(1,	-1, 	 0.0f);
											glTexCoord2f(1,1);
											glVertex3f(1,	1, 0.0f);
											glTexCoord2f(0,1);
											glVertex3f(-1, 	1, 0.0f);
											glEnd();
										glPopMatrix();
										glEnable(GL_BLEND);
										glColor4f(0.4,0.4,0.4,1.0);
										glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
										glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
										glEnable(GL_TEXTURE_2D);
										glBindTexture( GL_TEXTURE_2D, Mainmenuitems[4]);
										glPushMatrix();
											//glScalef(.25,.25,.25);
											glBegin(GL_QUADS);
											glTexCoord2f(0,0);
											glVertex3f(-1,		-1, 	 0.0f);
											glTexCoord2f(1,0);
											glVertex3f(1,	-1, 	 0.0f);
											glTexCoord2f(1,1);
											glVertex3f(1,	1, 0.0f);
											glTexCoord2f(0,1);
											glVertex3f(-1, 	1, 0.0f);
											glEnd();
										glPopMatrix();
									glPopMatrix();
								glPopMatrix();
								glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
							glPopMatrix();

							glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
							glPushMatrix();										// Store The Projection Matrix
								glLoadIdentity();									// Reset The Projection Matrix
								glOrtho(0,640,0,480,-100,100);						// Set Up An Ortho Screen
								glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
								glPushMatrix();										// Store The Modelview Matrix
									glLoadIdentity();								// Reset The Modelview Matrix
									glPushMatrix();
										glDisable(GL_TEXTURE_2D);
										glColor4f(1,0,0,1);
										/*glPushMatrix();
										glBegin(GL_QUADS);
										glTexCoord2f(0,0);
										if(anim!=1)glVertex3f(190,	150, 	 0.0f);
										if(anim==1)glVertex3f(190+movex[4]*transition,	150, 	 0.0f);
										glTexCoord2f(1,0);
										glVertex3f(640,	150, 	 0.0f);
										glTexCoord2f(1,1);
										glVertex3f(640,	336, 0.0f);
										glTexCoord2f(0,1);
										if(anim!=1)glVertex3f(190, 336, 0.0f);
										if(anim==1)glVertex3f(190+movex[4]*transition, 336, 	 0.0f);
										glEnd();
										glPopMatrix();*/
									glPopMatrix();
								glPopMatrix();
								glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
							glPopMatrix();
							glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

						}

						glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
						glPushMatrix();										// Store The Projection Matrix
						glLoadIdentity();									// Reset The Projection Matrix
						glOrtho(0,640,0,480,-100,100);						// Set Up An Ortho Screen
						glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
						glPushMatrix();										// Store The Modelview Matrix
						glLoadIdentity();								// Reset The Modelview Matrix
						glEnable(GL_TEXTURE_2D);
						if(nummenuitems>0)
						{
							for(j=0;j<nummenuitems;j++)
							{
								if(j<=3||(mainmenu!=1&&mainmenu!=2))
								{
									//glDisable(GL_BLEND);
									glEnable(GL_ALPHA_TEST);
									glEnable(GL_BLEND);
									//glDisable(GL_ALPHA_TEST);
									glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
									if(mainmenu==1||mainmenu==2)
									{
										glColor4f(1,1,1,1);
										glBlendFunc(GL_SRC_ALPHA,GL_ONE);
										glBindTexture( GL_TEXTURE_2D, Mainmenuitems[j]);
										glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
										glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
										glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
										glPushMatrix();
											glBegin(GL_QUADS);
											glTexCoord2f(0,0);
											glVertex3f(startx[j]+movex[j]*transition,	starty[j]+movey[j]*transition, 	 0.0f);
											glTexCoord2f(1,0);
											glVertex3f(endx[j]+movex[j]*transition,		starty[j]+movey[j]*transition, 	 0.0f);
											glTexCoord2f(1,1);
											glVertex3f(endx[j]+movex[j]*transition,		endy[j]+movey[j]*transition, 0.0f);
											glTexCoord2f(0,1);
											glVertex3f(startx[j]+movex[j]*transition, 	endy[j]+movey[j]*transition, 0.0f);
											glEnd();
										glPopMatrix();
										glEnable(GL_BLEND);
										//glDisable(GL_ALPHA_TEST);
										if(j<4)glBlendFunc(GL_SRC_ALPHA,GL_ONE);
										for(i=0;i<10;i++)
										{
											if(1-((float)i)/10-(1-selectedlong[j])>0)
											{
												glColor4f(1,1,1,(1-((float)i)/10-(1-selectedlong[j]))*.25);
												glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
												glPushMatrix();
													glBegin(GL_QUADS);
													glTexCoord2f(0,0);
													glVertex3f(startx[j]-((float)i)*1/2+offsetx[j]*((float)i)/2+movex[j]*transition,	starty[j]-((float)i)*1/2+offsety[j]*((float)i)/2+movey[j]*transition, 	 0.0f);
													glTexCoord2f(1,0);
													glVertex3f(endx[j]+((float)i)*1/2+offsetx[j]*((float)i)/2+movex[j]*transition,	starty[j]-((float)i)*1/2+offsety[j]*((float)i)/2+movey[j]*transition, 	 0.0f);
													glTexCoord2f(1,1);
													glVertex3f(endx[j]+((float)i)*1/2+offsetx[j]*((float)i)/2+movex[j]*transition,	endy[j]+((float)i)*1/2+offsety[j]*((float)i)/2+movey[j]*transition, 0.0f);
													glTexCoord2f(0,1);
													glVertex3f(startx[j]-((float)i)*1/2+offsetx[j]*((float)i)/2+movex[j]*transition, endy[j]+((float)i)*1/2+offsety[j]*((float)i)/2+movey[j]*transition, 0.0f);
													glEnd();
												glPopMatrix();
											}
										}
									}
									if(mainmenu==3||mainmenu==4||mainmenu==5||mainmenu==6||mainmenu==7||mainmenu==8||mainmenu==9||mainmenu==10||mainmenu==11||mainmenu==12||mainmenu==13||mainmenu==14||mainmenu==15||mainmenu==16||mainmenu==17)
									{
										if(mainmenu!=5||j<6)
										{
											glColor4f(1,0,0,1);
											if(mainmenu==12&&j==4)glColor4f(1,(sin(menupulse)+1)/2,(sin(menupulse)+1)/2,1);
											if(mainmenu==9&&j>accountprogress[accountactive]&&j<numchallengelevels)glColor4f(0.5,0,0,1);
											if(mainmenu==11&&j>accountprogress[accountactive]&&j<numchallengelevels)glColor4f(0.5,0,0,1);
											//if(1-((float)i)/10-(1-selectedlong[j])>0){
											glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
											glPushMatrix();
												if((mainmenu!=7||j!=0||!entername)&&(mainmenu!=13||j!=1)&&(mainmenu!=14||j!=1))text.glPrint(startx[j],starty[j],menustring[j],0,1,640,480);
												else
												{
													if(displayblink){
														sprintf (string, "_");
														text.glPrint(startx[j]+(float)(displayselected)*10,starty[j],string,0,1,640,480);
													}
													k=0;
													for(l=0;l<displaychars[k];l++){
														if(l<displaychars[k]){
															sprintf (string, "%c",displaytext[k][l]);
															text.glPrint(startx[j]+l*10,starty[j],string,0,1,640,480);
														}
													}
												}
											glPopMatrix();
											/*}
											else{
											glPushMatrix();
											sprintf (string, "Hooo!");
											text.glPrint(startx[0],starty[0],string,0,1,640,480);
											glPopMatrix();
											}*/
											glEnable(GL_BLEND);
											glBlendFunc(GL_SRC_ALPHA,GL_ONE);
											for(i=0;i<15;i++)
											{
												if(1-((float)i)/15-(1-selectedlong[j])>0)
												{
													glColor4f(1,0,0,(1-((float)i)/10-(1-selectedlong[j]))*.25);
													if(mainmenu==12&&j==4)glColor4f(1,(sin(menupulse)+1)/2,(sin(menupulse)+1)/2,(1-((float)i)/10-(1-selectedlong[j]))*.25);
													if(mainmenu==9&&j>accountprogress[accountactive]&&j<numchallengelevels)glColor4f(0.5,0,0,(1-((float)i)/10-(1-selectedlong[j]))*.25);
													if(mainmenu==11&&j>accountprogress[accountactive]&&j<numchallengelevels)glColor4f(0.5,0,0,(1-((float)i)/10-(1-selectedlong[j]))*.25);
													if(mainmenu==3)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4-((/*1*/+((float)i)/70)*strlen(menustring[j]))*3,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==4)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==5)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==6)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==7&&(j!=0||!entername))text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==8)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==9)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==11)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==10)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==12)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==15)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==16)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==17)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==13&&j!=1)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													if(mainmenu==14&&j!=1)text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4/*-((((float)i)/70)*strlen(menustring[j]))*3*/,starty[j]/*-i*1/2*/+offsety[j]*((float)i)/4,menustring[j],0,1+((float)i)/70,640,480);
													/*else{
													if(displayblink){
													sprintf (string, "_");
													text.glPrint(startx[j]-((float)i)+offsetx[j]*((float)i)/4+(float)(displayselected)*10*(1+((float)i)/70),starty[j]+offsety[j]*((float)i)/4,string,0,1+((float)i)/70,640,480);
													}
													k=0;
													for(l=0;l<displaychars[k];l++){
													if(l<displaychars[k]){
													sprintf (string, "%c",displaytext[k][l]);
													text.glPrint(startx[j]-((float)k)+offsetx[j]*((float)k)/4+l*10*(1+((float)i)/70),starty[j]+offsety[j]*((float)i)/4,string,0,1+((float)i)/70,640,480);
													}
													}
													}*/
												}
											}
										}
										else
										{
											glClear(GL_DEPTH_BUFFER_BIT);
											glEnable(GL_ALPHA_TEST);
											glAlphaFunc(GL_GREATER, 0.001f);
											glEnable(GL_TEXTURE_2D);
											glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
											glDisable(GL_CULL_FACE);
											glDisable(GL_LIGHTING);
											if(j==6)glColor4f(1,1,1,1);
											else glColor4f(1,0,0,1);

											glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
											glPushMatrix();										// Store The Projection Matrix
												glLoadIdentity();									// Reset The Projection Matrix
												glOrtho(0,640,0,480,-100,100);						// Set Up An Ortho Screen
												glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
												glPushMatrix();										// Store The Modelview Matrix
													glLoadIdentity();								// Reset The Modelview Matrix
													glPushMatrix();

														//Draw world, draw map
														glTranslatef(2,-5,0);

														if(j>6&&j<nummenuitems-1)
														{
															XYZ linestart,lineend,offset;
															XYZ fac;
															float startsize;
															float endsize;
															linestart=0;
															lineend=0;
															offset=0;
															//float linestartx,lineendx,linestarty,lineendy,offsetx,offsety;
															linestart.x=(startx[j]+endx[j])/2;
															linestart.y=(starty[j]+endy[j])/2;
															if(j>=6+accountcampaignchoicesmade[accountactive]){
																linestart.x=(startx[6+accountcampaignchoicesmade[accountactive]]+endx[6+accountcampaignchoicesmade[accountactive]])/2;
																linestart.y=(starty[6+accountcampaignchoicesmade[accountactive]]+endy[6+accountcampaignchoicesmade[accountactive]])/2;
															}
															lineend.x=(startx[j+1]+endx[j+1])/2;
															lineend.y=(starty[j+1]+endy[j+1])/2;
															offset=lineend-linestart;
															fac=offset;
															Normalise(&fac);
															offset=DoRotation(offset,0,0,90);
															Normalise(&offset);
															glDisable(GL_TEXTURE_2D);							

															if(j<6+accountcampaignchoicesmade[accountactive]){
																glColor4f(0.5,0,0,1);
																startsize=.5;
																endsize=.5;
															}
															if(j>=6+accountcampaignchoicesmade[accountactive]){
																glColor4f(1,0,0,1);
																endsize=1;
																startsize=.5;
															}

															linestart+=fac*4*startsize;
															lineend-=fac*4*endsize;

															if(!(j>7+accountcampaignchoicesmade[accountactive]+campaignchoicenum)){
																glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
																glPushMatrix();
																	glBegin(GL_QUADS);
																	glTexCoord2f(0,0);
																	glVertex3f(linestart.x-offset.x*startsize,	linestart.y-offset.y*startsize, 	 0.0f);
																	glTexCoord2f(1,0);
																	glVertex3f(linestart.x+offset.x*startsize,	linestart.y+offset.y*startsize, 	 0.0f);
																	glTexCoord2f(1,1);
																	glVertex3f(lineend.x+offset.x*endsize,		lineend.y+offset.y*endsize, 0.0f);
																	glTexCoord2f(0,1);
																	glVertex3f(lineend.x-offset.x*endsize, 		lineend.y-offset.y*endsize, 0.0f);
																	glEnd();
																glPopMatrix();
															}
															glEnable(GL_TEXTURE_2D);
														}


														if(j==6)glBindTexture( GL_TEXTURE_2D, Mainmenuitems[7]);
														else glBindTexture( GL_TEXTURE_2D, Mapcircletexture);
														glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
														glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
														if(j-7<accountcampaignchoicesmade[accountactive])glColor4f(0.5,0,0,1);
														if(j-7>=accountcampaignchoicesmade[accountactive])glColor4f(1,0,0,1);
														if(j==6)glColor4f(1,1,1,1);
														XYZ midpoint;
														float itemsize;
														itemsize=abs(startx[j]-endx[j])/2;
														midpoint=0;
														midpoint.x=(startx[j]+endx[j])/2;
														midpoint.y=(starty[j]+endy[j])/2;
														if(j>6&&(j-7<accountcampaignchoicesmade[accountactive]))itemsize*=.5;
														if(!(j-7>accountcampaignchoicesmade[accountactive]+campaignchoicenum))
														{
															glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
															glPushMatrix();
																glBegin(GL_QUADS);
																glTexCoord2f(0,0);
																glVertex3f(midpoint.x-itemsize+movex[j]*transition,	midpoint.y-itemsize+movey[j]*transition, 	 0.0f);
																glTexCoord2f(1,0);
																glVertex3f(midpoint.x+itemsize+movex[j]*transition,		midpoint.y-itemsize+movey[j]*transition, 	 0.0f);
																glTexCoord2f(1,1);
																glVertex3f(midpoint.x+itemsize+movex[j]*transition,		midpoint.y+itemsize+movey[j]*transition, 0.0f);
																glTexCoord2f(0,1);
																glVertex3f(midpoint.x-itemsize+movex[j]*transition, 	midpoint.y+itemsize+movey[j]*transition, 0.0f);
																glEnd();
															glPopMatrix();
															glEnable(GL_BLEND);
															//glDisable(GL_ALPHA_TEST);
															if(j<4)glBlendFunc(GL_SRC_ALPHA,GL_ONE);
															for(i=0;i<10;i++)
															{
																if(1-((float)i)/10-(1-selectedlong[j])>0)
																{
																	glColor4f(1,0,0,(1-((float)i)/10-(1-selectedlong[j]))*.25);
																	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
																	glPushMatrix();
																		glBegin(GL_QUADS);
																		glTexCoord2f(0,0);
																		glVertex3f(midpoint.x-itemsize-((float)i)*1/2+offsetx[j]*((float)i)/2+movex[j]*transition,	midpoint.y-itemsize-((float)i)*1/2+offsety[j]*((float)i)/2+movey[j]*transition, 	 0.0f);
																		glTexCoord2f(1,0);
																		glVertex3f(midpoint.x+itemsize+((float)i)*1/2+offsetx[j]*((float)i)/2+movex[j]*transition,	midpoint.y-itemsize-((float)i)*1/2+offsety[j]*((float)i)/2+movey[j]*transition, 	 0.0f);
																		glTexCoord2f(1,1);
																		glVertex3f(midpoint.x+itemsize+((float)i)*1/2+offsetx[j]*((float)i)/2+movex[j]*transition,	midpoint.y+itemsize+((float)i)*1/2+offsety[j]*((float)i)/2+movey[j]*transition, 0.0f);
																		glTexCoord2f(0,1);
																		glVertex3f(midpoint.x-itemsize-((float)i)*1/2+offsetx[j]*((float)i)/2+movex[j]*transition, midpoint.y+itemsize+((float)i)*1/2+offsety[j]*((float)i)/2+movey[j]*transition, 0.0f);
																		glEnd();
																	glPopMatrix();
																}
															}
														}
													glPopMatrix();
												glPopMatrix();
												glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
											glPopMatrix();
											glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix

											if(j-7>=accountcampaignchoicesmade[accountactive]){
												//glColor4f(0,0,0,1);
												//text.glPrintOutline(startx[j]+10-1.5,starty[j]-4-1.5,menustring[j],0,0.6*1.25,640,480);
												//glColor4f(1,0,0,1);
												//text.glPrint(startx[j]+10,starty[j]-4,menustring[j],0,0.6,640,480);
												text.glPrintOutlined(0.9,0,0,startx[j]+10,starty[j]-4,menustring[j],0,0.6,640,480);
												glDisable(GL_DEPTH_TEST);
											}
										}
									}
								}
							}
						}
						glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
						glPopMatrix();
						glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
						glPopMatrix();

							if(mainmenu==1||mainmenu==2)
								if(transition<.1||transition>.9){
									glClear(GL_DEPTH_BUFFER_BIT);
									glEnable(GL_ALPHA_TEST);
									glAlphaFunc(GL_GREATER, 0.001f);
									glEnable(GL_TEXTURE_2D);
									glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
									glDisable(GL_CULL_FACE);
									glDisable(GL_LIGHTING);
									glDepthMask(0);
									glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
									glPushMatrix();										// Store The Projection Matrix
										glLoadIdentity();									// Reset The Projection Matrix
										glOrtho(0,640,0,480,-100,100);						// Set Up An Ortho Screen
										glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
										glPushMatrix();										// Store The Modelview Matrix
											glLoadIdentity();								// Reset The Modelview Matrix
											glPushMatrix();
												glDisable(GL_TEXTURE_2D);
												if(transition<.1)glColor4f(1,0,0,1-(transition*10));
												if(transition>.9)glColor4f(1,0,0,1-((1-transition)*10));
												/*glPushMatrix();
												glBegin(GL_QUADS);
												glTexCoord2f(0,0);
												glVertex3f(190,	150, 	 0.0f);
												glTexCoord2f(1,0);
												glVertex3f(640,	150, 	 0.0f);
												glTexCoord2f(1,1);
												glVertex3f(640,	336, 0.0f);
												glTexCoord2f(0,1);
												glVertex3f(190, 336, 0.0f);
												glEnd();
												glPopMatrix();*/
											glPopMatrix();
										glPopMatrix();
										glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
									glPopMatrix();
								}

								glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
								glPushMatrix();										// Store The Projection Matrix
									glLoadIdentity();									// Reset The Projection Matrix
									glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
									glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
									glPushMatrix();										// Store The Modelview Matrix
										glLoadIdentity();								// Reset The Modelview Matrix
										glTranslatef(screenwidth/2,screenheight/2,0);
										glPushMatrix();
											glScalef((float)screenwidth/2,(float)screenheight/2,1);
											glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
											glEnable(GL_BLEND);
											glEnable(GL_TEXTURE_2D);
											glColor4f(1,1,1,1);
											glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
											glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
										glPopMatrix();
										glPushMatrix();
											glTranslatef(mousecoordh-screenwidth/2,mousecoordv*-1+screenheight/2,0);
											glScalef((float)screenwidth/64,(float)screenwidth/64,1);
											glTranslatef(1,-1,0);
											glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
											glColor4f(1,1,1,1);
											glBindTexture( GL_TEXTURE_2D, cursortexture);
											glPushMatrix();
												//glScalef(.25,.25,.25);
												glBegin(GL_QUADS);
												glTexCoord2f(0,0);
												glVertex3f(-1,		-1, 	 0.0f);
												glTexCoord2f(1,0);
												glVertex3f(1,	-1, 	 0.0f);
												glTexCoord2f(1,1);
												glVertex3f(1,	1, 0.0f);
												glTexCoord2f(0,1);
												glVertex3f(-1, 	1, 0.0f);
												glEnd();
											glPopMatrix();
										glPopMatrix();
									glPopMatrix();
									glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
								glPopMatrix();


								if(flashamount>0)
								{
									if(flashamount>1)flashamount=1;
									if(flashdelay<=0)flashamount-=multiplier;
									flashdelay--;
									if(flashamount<0)flashamount=0;
									glDisable(GL_DEPTH_TEST);							// Disables Depth Testing
									glDisable(GL_CULL_FACE);
									glDisable(GL_LIGHTING);
									glDisable(GL_TEXTURE_2D);
									glDepthMask(0);
									glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
									glPushMatrix();										// Store The Projection Matrix
										glLoadIdentity();									// Reset The Projection Matrix
										glOrtho(0,screenwidth,0,screenheight,-100,100);						// Set Up An Ortho Screen
										glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
										glPushMatrix();										// Store The Modelview Matrix
											glLoadIdentity();								// Reset The Modelview Matrix
											glScalef(screenwidth,screenheight,1);
											glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
											glEnable(GL_BLEND);
											glColor4f(flashr,flashg,flashb,flashamount);
											glBegin(GL_QUADS);
											glVertex3f(0,		0, 	 0.0f);
											glVertex3f(256,	0, 	 0.0f);
											glVertex3f(256,	256, 0.0f);
											glVertex3f(0, 	256, 0.0f);
											glEnd();
											glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
										glPopMatrix();										// Restore The Old Projection Matrix
									glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
									glPopMatrix();										// Restore The Old Projection Matrix
									glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
									glEnable(GL_CULL_FACE);
									glDisable(GL_BLEND);
									glDepthMask(1);
								}	
	}

	if(freeze||winfreeze||(mainmenu&&gameon)||(!gameon&&gamestarted)||(!gameon&&gamestarted)){
		tempmult=multiplier;
		multiplier=0;
	}


	//glFlush();
	if(drawmode!=motionblurmode||mainmenu){
        #if !USE_SDL
        // this prevents menus from rendering if you hit ESC during
        //  motion blur sequences...maybe SDL is buffering differently?
		if(drawmode!=motionblurmode)
			swap_gl_buffers();
        #else
		swap_gl_buffers();
        #endif
	}

	//myassert(glGetError() == GL_NO_ERROR);
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	//glFlush();

	weapons.DoStuff();

	if(drawtoggle==2)drawtoggle=0;

	if(freeze||winfreeze||(mainmenu&&gameon)||(!gameon&&gamestarted)){
		multiplier=tempmult;
	}
	//Jordan fixed your warning!
	return 0;
}

