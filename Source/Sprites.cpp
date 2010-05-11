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

#include "Sprites.h"
#include "Person.h"
extern XYZ viewer;
extern float viewdistance;
extern float fadestart;
extern int environment;
extern float texscale;
extern Light light;
extern float multiplier;
extern float gravity;
extern Terrain terrain;
extern Objects objects;
extern int detail;
extern XYZ viewerfacing;
extern float terraindetail;
extern int bloodtoggle;
extern XYZ windvector;
extern int numplayers;
extern Person player[maxplayers];
//Functions

void Sprites::Draw()
{
	static int i,j,k;
	static float M[16];
	static XYZ point;
	static float distancemult;
	static int lasttype;
	static int lastspecial;
	static int whichpatchx,whichpatchz;
	static XYZ start,end,colpoint;
	static bool check;
	static bool blend;
	static float tempmult;
	static XYZ difference;
	static float lightcolor[3];
	static float viewdistsquared=viewdistance*viewdistance;
	static XYZ tempviewer;

	tempviewer=viewer+viewerfacing*6;
	check=0;

	lightcolor[0]=light.color[0]*.5+light.ambient[0];
	lightcolor[1]=light.color[1]*.5+light.ambient[1];
	lightcolor[2]=light.color[2]*.5+light.ambient[2];

	checkdelay-=multiplier*10;

	if(checkdelay<=0){
		check=1;
		checkdelay=1;
	}

	lasttype=-1;
	lastspecial=-1;
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	blend = 1;
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(0);
	glAlphaFunc(GL_GREATER, 0.0001);
	for(i=0;i<numsprites;i++){
		if(type[i]==cloudsprite&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, cloudtexture);
			if(!blend){
				blend=1;
				glAlphaFunc(GL_GREATER, 0.0001);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		if(type[i]==cloudimpactsprite&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, cloudimpacttexture);
			if(!blend){
				blend=1;
				glAlphaFunc(GL_GREATER, 0.0001);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		if(type[i]==breathsprite&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, cloudimpacttexture);
			if(!blend){
				blend=1;
				glAlphaFunc(GL_GREATER, 0.0001);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		if(type[i]==smoketype&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, smoketexture);
			if(!blend){
				blend=1;
				glAlphaFunc(GL_GREATER, 0.0001);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		if(type[i]==bloodsprite&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, bloodtexture);
			if(!blend){
				blend=1;
				glAlphaFunc(GL_GREATER, 0.0001);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		if(type[i]==splintersprite&&(lasttype!=type[i]||lastspecial!=special[i])){
			if(special[i]==0)glBindTexture( GL_TEXTURE_2D, splintertexture);
			if(special[i]==1)glBindTexture( GL_TEXTURE_2D, leaftexture);
			if(special[i]==2)glBindTexture( GL_TEXTURE_2D, snowflaketexture);
			if(special[i]==3)glBindTexture( GL_TEXTURE_2D, toothtexture);
			if(!blend){
				blend=1;
				glAlphaFunc(GL_GREATER, 0.0001);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		if(type[i]==snowsprite&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, snowflaketexture);
			if(!blend){
				blend=1;
				glAlphaFunc(GL_GREATER, 0.0001);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		if(type[i]==weaponshinesprite&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, shinetexture);
			if(blend){
				blend=0;
				glAlphaFunc(GL_GREATER, 0.001);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			}
		}
		if((type[i]==flamesprite||type[i]==weaponflamesprite)&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, flametexture);
			if(blend||lasttype==bloodflamesprite){
				blend=0;
				glAlphaFunc(GL_GREATER, 0.3);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			}
		}
		if((type[i]==bloodflamesprite)&&lasttype!=type[i]){
			glBindTexture( GL_TEXTURE_2D, bloodflametexture);
			if(blend){
				blend=0;
				glAlphaFunc(GL_GREATER, 0.3);
				glBlendFunc(GL_ONE,GL_ZERO);
				//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}
		}
		if(type[i]!=snowsprite)distancemult=(viewdistsquared-(findDistancefast(&viewer,&position[i])-(viewdistsquared*fadestart))*(1/(1-fadestart)))/viewdistsquared;
		if(type[i]==snowsprite)distancemult=(144-(findDistancefast(&tempviewer,&position[i])-(144*fadestart))*(1/(1-fadestart)))/144;
		if(type[i]!=flamesprite){
			if(distancemult>=1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],opacity[i]);
			if(distancemult<1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],opacity[i]*distancemult);
		}
		if(type[i]==flamesprite){
			if(distancemult>=1)glColor4f(color[i][0],color[i][1],color[i][2],opacity[i]);
			if(distancemult<1)glColor4f(color[i][0],color[i][1],color[i][2],opacity[i]*distancemult);
		}
		lasttype=type[i];
		lastspecial=special[i];
		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glPushMatrix();
			glTranslatef(position[i].x,position[i].y,position[i].z);
			if((type[i]==flamesprite||type[i]==weaponflamesprite||type[i]==weaponshinesprite)){
				difference=viewer-position[i];
				Normalise(&difference);
				glTranslatef(difference.x*size[i]/4, difference.y*size[i]/4, difference.z*size[i]/4);
			}
			if(type[i]==snowsprite){
				glRotatef(rotation[i]*.2,0,.3,1);
				glTranslatef(1,0,0);
			}
			glGetFloatv(GL_MODELVIEW_MATRIX,M);
			point.x=M[12];
			point.y=M[13];
			point.z=M[14];
			glLoadIdentity();
			glTranslatef(point.x, point.y, point.z);

			glRotatef(rotation[i],0,0,1);

			if((type[i]==flamesprite||type[i]==weaponflamesprite||type[i]==weaponshinesprite||type[i]==bloodflamesprite)){
				if(alivetime[i]<.14)glScalef(alivetime[i]/.14,alivetime[i]/.14,alivetime[i]/.14);
			}
			if(type[i]==smoketype||type[i]==snowsprite||type[i]==weaponshinesprite||type[i]==breathsprite){
				if(alivetime[i]<.3){
					if(distancemult>=1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],opacity[i]*alivetime[i]/.3);
					if(distancemult<1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],opacity[i]*distancemult*alivetime[i]/.3);
				}
			}
			if(type[i]==splintersprite&&special[i]>0&&special[i]!=3){
				if(alivetime[i]<.2){
					if(distancemult>=1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],alivetime[i]/.2);
					if(distancemult<1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],distancemult*alivetime[i]/.2);
				}
				else{
					if(distancemult>=1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],1);
					if(distancemult<1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],1);
				}
			}
			if(type[i]==splintersprite&&(special[i]==0||special[i]==3)){
				if(distancemult>=1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],1);
				if(distancemult<1)glColor4f(color[i][0]*lightcolor[0],color[i][1]*lightcolor[1],color[i][2]*lightcolor[2],1);
			}
			/*
			if(type[i]==snowsprite){
			glRotatef(rotation[i],0,0,1);
			glTranslatef(1,0,0);
			}*/

			glBegin(GL_TRIANGLES);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( .5*size[i], .5*size[i], 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-.5*size[i], .5*size[i], 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( .5*size[i],-.5*size[i], 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-.5*size[i],-.5*size[i], 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( .5*size[i], -.5*size[i], 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-.5*size[i], .5*size[i], 0.0f);
			glEnd();
		glPopMatrix();
	}
	tempmult=multiplier;
	for(i=numsprites-1;i>=0;i--){
		multiplier=tempmult;
		if(type[i]!=snowsprite)position[i]+=velocity[i]*multiplier;
		if(type[i]!=snowsprite)velocity[i]+=windvector*multiplier;
		if(type[i]==flamesprite||type[i]==smoketype)position[i]+=windvector*multiplier/2;
		if((type[i]==flamesprite||type[i]==weaponflamesprite||type[i]==weaponshinesprite||type[i]==bloodflamesprite))multiplier*=speed[i]*.7;
		alivetime[i]+=multiplier;

		if(type[i]==cloudsprite||type[i]==cloudimpactsprite){
			opacity[i]-=multiplier/2;
			size[i]+=multiplier/2;
			velocity[i].y+=gravity*multiplier*.25;
		}
		if(type[i]==breathsprite){
			opacity[i]-=multiplier/2;
			size[i]+=multiplier/2;
			if(findLength(&velocity[i])<=multiplier)velocity[i]=0;
			else{
				XYZ slowdown;
				slowdown=velocity[i]*-1;
				Normalise(&slowdown);
				slowdown*=multiplier;
				velocity[i]+=slowdown;
			}
		}
		if(type[i]==snowsprite){
			size[i]-=multiplier/120;
			rotation[i]+=multiplier*360;
			position[i].y-=multiplier;
			position[i]+=windvector*multiplier;
			if(position[i].y<tempviewer.y-6)position[i].y+=12;
			if(position[i].y>tempviewer.y+6)position[i].y-=12;
			if(position[i].z<tempviewer.z-6)position[i].z+=12;
			if(position[i].z>tempviewer.z+6)position[i].z-=12;
			if(position[i].x<tempviewer.x-6)position[i].x+=12;
			if(position[i].x>tempviewer.x+6)position[i].x-=12;
		}
		if(type[i]==bloodsprite){
			bool spritehit=0;
			rotation[i]+=multiplier*100;
			velocity[i].y+=gravity*multiplier;
			if(check){
				XYZ where,startpoint,endpoint,movepoint,footpoint;
				float rotationpoint;
				int whichtri;

				for(j=0;j<numplayers;j++){
					if(!spritehit&&player[j].dead&&alivetime[i]>.1){
						where=oldposition[i];
						where-=player[j].coords;
						if(!player[j].skeleton.free)where=DoRotation(where,0,-player[j].rotation,0);
						startpoint=where;
						where=position[i];
						where-=player[j].coords;
						if(!player[j].skeleton.free)where=DoRotation(where,0,-player[j].rotation,0);
						endpoint=where;

						movepoint=0;
						rotationpoint=0;
						whichtri=player[j].skeleton.drawmodel.LineCheck(&startpoint,&endpoint, &footpoint, &movepoint, &rotationpoint);
						if(whichtri!=-1){
							spritehit=1;
							player[j].DoBloodBigWhere(0,160,oldposition[i]);
							DeleteSprite(i);
						}
					}
				}

				whichpatchx=position[i].x/(terrain.size/subdivision*terrain.scale*terraindetail);
				whichpatchz=position[i].z/(terrain.size/subdivision*terrain.scale*terraindetail);
				if(whichpatchx>0&&whichpatchz>0&&whichpatchx<subdivision&&whichpatchz<subdivision)
					if(terrain.patchobjectnum[whichpatchx][whichpatchz]){
						if(!spritehit)
							for(j=0;j<terrain.patchobjectnum[whichpatchx][whichpatchz];j++){
								k=terrain.patchobjects[whichpatchx][whichpatchz][j];
								start=oldposition[i];
								end=position[i];
								if(!spritehit)
									if(objects.model[k].LineCheck(&start,&end,&colpoint,&objects.position[k],&objects.rotation[k])!=-1){
										if(detail==2||(detail==1&&abs(Random()%4)==0)||(detail==0&&abs(Random()%8)==0))objects.model[k].MakeDecal(blooddecalfast,DoRotation(colpoint-objects.position[k],0,-objects.rotation[k],0),size[i]*1.6/*+abs((float)(Random()%100))/2400*/,.5,Random()%360);
										DeleteSprite(i);
										spritehit=1;
									}	
							}
					}
					if(!spritehit)
						if(position[i].y<terrain.getHeight(position[i].x,position[i].z)){
							terrain.MakeDecal(blooddecalfast,position[i],size[i]*1.6/*+abs((float)(Random()%100))/2400*/,.6,Random()%360);
							DeleteSprite(i);
						}
			}
		}
		if(type[i]==splintersprite){
			rotation[i]+=rotatespeed[i]*multiplier;
			opacity[i]-=multiplier/2;
			if(special[i]==0||special[i]==2||special[i]==3)velocity[i].y+=gravity*multiplier;
			if(special[i]==1)velocity[i].y+=gravity*multiplier*.5;
		}
		if(type[i]==flamesprite||type[i]==weaponflamesprite||type[i]==weaponshinesprite||type[i]==bloodflamesprite){
			rotation[i]+=multiplier*rotatespeed[i];
			opacity[i]-=multiplier*5/4;
			if(type[i]!=weaponshinesprite&&type[i]!=bloodflamesprite)
				if(opacity[i]<.5&&opacity[i]+multiplier*5/4>=.5&&(abs(Random()%4)==0||(initialsize[i]>2&&Random()%2==0)))MakeSprite(smoketype, position[i],velocity[i], .9,.9,.6, size[i]*1.2, .4);
			if(alivetime[i]>.14&&(type[i]==flamesprite)){
				velocity[i]=0;
				velocity[i].y=1.5;
			}
		}
		/*if(type[i]==smoketype){
		opacity[i]-=multiplier/3/initialsize[i];
		size[i]+=multiplier;
		velocity[i]=0;
		velocity[i].y=1.5;
		rotation[i]+=multiplier*rotatespeed[i]/5;
		}*/
		if(type[i]==smoketype){
			opacity[i]-=multiplier/3/initialsize[i];
			color[i][0]-=multiplier;
			color[i][1]-=multiplier;
			color[i][2]-=multiplier;
			if(color[i][0]<.6)color[i][0]=.6;
			if(color[i][1]<.6)color[i][1]=.6;
			if(color[i][2]<.6)color[i][2]=.6;
			size[i]+=multiplier;
			velocity[i]=0;
			velocity[i].y=1.5;
			rotation[i]+=multiplier*rotatespeed[i]/5;
		}
		if(opacity[i]<=0||size[i]<=0)DeleteSprite(i);
	}
	if(check)
		for(i=numsprites-1;i>=0;i--){
			oldposition[i]=position[i];
		}
		glAlphaFunc(GL_GREATER, 0.0001);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void Sprites::DeleteSprite(int which)
{
	type[which]=type[numsprites-1];
	rotation[which]=rotation[numsprites-1];
	alivetime[which]=alivetime[numsprites-1];
	opacity[which]=opacity[numsprites-1];
	position[which]=position[numsprites-1];
	velocity[which]=velocity[numsprites-1];
	size[which]=size[numsprites-1];
	speed[which]=speed[numsprites-1];
	special[which]=special[numsprites-1];
	color[which][0]=color[numsprites-1][0];
	color[which][1]=color[numsprites-1][1];
	color[which][2]=color[numsprites-1][2];
	numsprites--;
}

void Sprites::MakeSprite(int atype, XYZ where, XYZ avelocity, float red, float green, float blue, float asize, float aopacity){
	if(numsprites<max_sprites-1){
		if((atype!=bloodsprite&&atype!=bloodflamesprite)||bloodtoggle){
			special[numsprites]=0;
			type[numsprites]=atype;
			position[numsprites]=where;
			oldposition[numsprites]=where;
			velocity[numsprites]=avelocity;
			alivetime[numsprites]=0;
			opacity[numsprites]=aopacity;
			size[numsprites]=asize;
			initialsize[numsprites]=asize;
			color[numsprites][0]=red;
			color[numsprites][1]=green;
			color[numsprites][2]=blue;
			rotatespeed[numsprites]=abs(Random()%720)-360;
			speed[numsprites]=float(abs(Random()%100))/200+1.5;
		}		
		numsprites++;
	}
}

Sprites::Sprites()
{
	cloudtexture = 0;
	cloudimpacttexture = 0;
	bloodtexture = 0;
	flametexture = 0;
	bloodflametexture = 0;
	smoketexture = 0;
	snowflaketexture = 0;
	shinetexture = 0;
	splintertexture = 0;
	leaftexture = 0;
	toothtexture = 0;

	memset(oldposition, 0, sizeof(oldposition));
	memset(position, 0, sizeof(position));
	memset(velocity, 0, sizeof(velocity));
	memset(size, 0, sizeof(size));
	memset(initialsize, 0, sizeof(initialsize));
	memset(type, 0, sizeof(type));
	memset(special, 0, sizeof(special));
	memset(color, 0, sizeof(color));
	memset(opacity, 0, sizeof(opacity));
	memset(rotation, 0, sizeof(rotation));
	memset(alivetime, 0, sizeof(alivetime));
	memset(speed, 0, sizeof(speed));
	memset(rotatespeed, 0, sizeof(rotatespeed));

	checkdelay = 0;
	numsprites = 0;
}
Sprites::~Sprites()
{
	if (toothtexture) glDeleteTextures( 1, &toothtexture );
	if (cloudtexture) glDeleteTextures( 1, &cloudtexture );
	if (cloudimpacttexture) glDeleteTextures( 1, &cloudimpacttexture );
	if (bloodtexture) glDeleteTextures( 1, &bloodtexture );
	if (flametexture) glDeleteTextures( 1, &flametexture );
	if (bloodflametexture) glDeleteTextures( 1, &bloodflametexture );
	if (smoketexture) glDeleteTextures( 1, &smoketexture );
	if (snowflaketexture) glDeleteTextures( 1, &snowflaketexture );
	if (shinetexture) glDeleteTextures( 1, &shinetexture );
	if (splintertexture) glDeleteTextures( 1, &splintertexture );
	if (leaftexture) glDeleteTextures( 1, &leaftexture );
}

