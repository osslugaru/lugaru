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

#include "Objects.h"
extern XYZ viewer;
extern float viewdistance;
extern float fadestart;
extern int environment;
extern float texscale;
extern Light light;
extern float multiplier;
extern float gravity;
extern FRUSTUM frustum;
extern Terrain terrain;
extern bool foliage;
extern int detail;
extern float blurness;
extern float windvar;
extern float playerdist;
extern bool skyboxtexture;

//Functions

bool 	Objects::checkcollide(XYZ startpoint,XYZ endpoint,int which){
	static XYZ colpoint,colviewer,coltarget;
	static int i;

	startpoint.y+=.1;
	endpoint.y+=.1;
	startpoint.y-=.1;
	endpoint.y-=.1;

	for(i=0;i<numobjects;i++){
		if(type[i]!=treeleavestype&&type[i]!=treetrunktype&&type[i]!=bushtype&&type[i]!=firetype&&i!=which){
			colviewer=startpoint;
			coltarget=endpoint;
			if(model[i].LineCheck(&colviewer,&coltarget,&colpoint,&position[i],&yaw[i])!=-1)return 1;	
		}
	}

	return 0;
}

void Objects::SphereCheckPossible(XYZ *p1,float radius)
{
	static int i,j;
	static int whichpatchx;
	static int whichpatchz;

	whichpatchx=p1->x/(terrain.size/subdivision*terrain.scale);
	whichpatchz=p1->z/(terrain.size/subdivision*terrain.scale);

	if(whichpatchx>=0&&whichpatchz>=0&&whichpatchx<subdivision&&whichpatchz<subdivision)
		if(terrain.patchobjectnum[whichpatchx][whichpatchz]>0&&terrain.patchobjectnum[whichpatchx][whichpatchz]<500)
			for(j=0;j<terrain.patchobjectnum[whichpatchx][whichpatchz];j++){
				i=terrain.patchobjects[whichpatchx][whichpatchz][j];
				possible[i]=0;
				if(model[i].SphereCheckPossible(p1, radius, &position[i], &yaw[i])!=-1){
					possible[i]=1;
				}
			}
}

void Objects::Draw()
{
	static float distance;
	static int i,j;
	static XYZ moved,terrainlight;
	bool hidden;

	for(i=0;i<numobjects;i++){
		if(type[i]!=firetype){
			moved=DoRotation(model[i].boundingspherecenter,0,yaw[i],0);
			if(type[i]==tunneltype||frustum.SphereInFrustum(position[i].x+moved.x,position[i].y+moved.y,position[i].z+moved.z,model[i].boundingsphereradius)){   
				distance=findDistancefast(&viewer,&position[i]);
				distance*=1.2;
				hidden=!(findDistancefastflat(&viewer,&position[i])>playerdist+3||(type[i]!=bushtype&&type[i]!=treeleavestype));
				if(!hidden){

					if(detail==2&&distance>viewdistance*viewdistance/4&&environment==desertenvironment)glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, blurness );
					else glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0 );
					distance=(viewdistance*viewdistance-(distance-(viewdistance*viewdistance*fadestart))*(1/(1-fadestart)))/viewdistance/viewdistance;
					if(distance>1)distance=1;
					if(distance>0){

						/*if(checkcollide(viewer,DoRotation(model[i].vertex[model[i].vertexNum],0,yaw[i],0)*scale[i]+position[i],i)){
						occluded[i]+=1;
						}
						else occluded[i]=0;*/
						if(occluded[i]<6){
							glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
							glPushMatrix();
								if(!model[i].color)glEnable(GL_LIGHTING);
								else glDisable(GL_LIGHTING);
								glDepthMask(1);
								glTranslatef(position[i].x,position[i].y,position[i].z);
								if(type[i]==bushtype){
									messedwith[i]-=multiplier;
									if(rotxvel[i]||rotx[i]){
										if(rotx[i]>0)rotxvel[i]-=multiplier*8*fabs(rotx[i]);
										if(rotx[i]<0)rotxvel[i]+=multiplier*8*fabs(rotx[i]);
										if(rotx[i]>0)rotxvel[i]-=multiplier*4;
										if(rotx[i]<0)rotxvel[i]+=multiplier*4;
										if(rotxvel[i]>0)rotxvel[i]-=multiplier*4;
										if(rotxvel[i]<0)rotxvel[i]+=multiplier*4;
										if(fabs(rotx[i])<multiplier*4)rotx[i]=0;
										if(fabs(rotxvel[i])<multiplier*4)rotxvel[i]=0;

										rotx[i]+=rotxvel[i]*multiplier*4;
									}
									if(rotyvel[i]||roty[i]){
										if(roty[i]>0)rotyvel[i]-=multiplier*8*fabs(roty[i]);
										if(roty[i]<0)rotyvel[i]+=multiplier*8*fabs(roty[i]);
										if(roty[i]>0)rotyvel[i]-=multiplier*4;
										if(roty[i]<0)rotyvel[i]+=multiplier*4;
										if(rotyvel[i]>0)rotyvel[i]-=multiplier*4;
										if(rotyvel[i]<0)rotyvel[i]+=multiplier*4;
										if(fabs(roty[i])<multiplier*4)roty[i]=0;
										if(fabs(rotyvel[i])<multiplier*4)rotyvel[i]=0;

										roty[i]+=rotyvel[i]*multiplier*4;
									}
									if(roty[i]){
										glRotatef(roty[i],1,0,0);
									}
									if(rotx[i]){
										glRotatef(-rotx[i],0,0,1);
									}
									if(rotx[i]>10)rotx[i]=10;
									if(rotx[i]<-10)rotx[i]=-10;
									if(roty[i]>10)roty[i]=10;
									if(roty[i]<-10)roty[i]=-10;
								}
								if(type[i]==treetrunktype||type[i]==treeleavestype){
									if(type[i]==treetrunktype||environment==2){
										messedwith[i]-=multiplier;
										if(rotxvel[i]||rotx[i]){
											if(rotx[i]>0)rotxvel[i]-=multiplier*8*fabs(rotx[i]);
											if(rotx[i]<0)rotxvel[i]+=multiplier*8*fabs(rotx[i]);
											if(rotx[i]>0)rotxvel[i]-=multiplier*4;
											if(rotx[i]<0)rotxvel[i]+=multiplier*4;
											if(rotxvel[i]>0)rotxvel[i]-=multiplier*4;
											if(rotxvel[i]<0)rotxvel[i]+=multiplier*4;
											if(fabs(rotx[i])<multiplier*4)rotx[i]=0;
											if(fabs(rotxvel[i])<multiplier*4)rotxvel[i]=0;

											rotx[i]+=rotxvel[i]*multiplier*4;
										}
										if(rotyvel[i]||roty[i]){
											if(roty[i]>0)rotyvel[i]-=multiplier*8*fabs(roty[i]);
											if(roty[i]<0)rotyvel[i]+=multiplier*8*fabs(roty[i]);
											if(roty[i]>0)rotyvel[i]-=multiplier*4;
											if(roty[i]<0)rotyvel[i]+=multiplier*4;
											if(rotyvel[i]>0)rotyvel[i]-=multiplier*4;
											if(rotyvel[i]<0)rotyvel[i]+=multiplier*4;
											if(fabs(roty[i])<multiplier*4)roty[i]=0;
											if(fabs(rotyvel[i])<multiplier*4)rotyvel[i]=0;

											roty[i]+=rotyvel[i]*multiplier*4;
										}
										if(roty[i]){
											glRotatef(roty[i]/6,1,0,0);
										}
										if(rotx[i]){
											glRotatef(-rotx[i]/6,0,0,1);
										}
										if(rotx[i]>10)rotx[i]=10;
										if(rotx[i]<-10)rotx[i]=-10;
										if(roty[i]>10)roty[i]=10;
										if(roty[i]<-10)roty[i]=-10;
									}
									else
									{
										messedwith[i]-=multiplier;
										if(rotxvel[i]||rotx[i]){
											if(rotx[i]>0)rotxvel[i]-=multiplier*8*fabs(rotx[i]);
											if(rotx[i]<0)rotxvel[i]+=multiplier*8*fabs(rotx[i]);
											if(rotx[i]>0)rotxvel[i]-=multiplier*4;
											if(rotx[i]<0)rotxvel[i]+=multiplier*4;
											if(rotxvel[i]>0)rotxvel[i]-=multiplier*4;
											if(rotxvel[i]<0)rotxvel[i]+=multiplier*4;
											if(fabs(rotx[i])<multiplier*4)rotx[i]=0;
											if(fabs(rotxvel[i])<multiplier*4)rotxvel[i]=0;

											rotx[i]+=rotxvel[i]*multiplier*4;
										}
										if(rotyvel[i]||roty[i]){
											if(roty[i]>0)rotyvel[i]-=multiplier*8*fabs(roty[i]);
											if(roty[i]<0)rotyvel[i]+=multiplier*8*fabs(roty[i]);
											if(roty[i]>0)rotyvel[i]-=multiplier*4;
											if(roty[i]<0)rotyvel[i]+=multiplier*4;
											if(rotyvel[i]>0)rotyvel[i]-=multiplier*4;
											if(rotyvel[i]<0)rotyvel[i]+=multiplier*4;
											if(fabs(roty[i])<multiplier*4)roty[i]=0;
											if(fabs(rotyvel[i])<multiplier*4)rotyvel[i]=0;

											roty[i]+=rotyvel[i]*multiplier*4;
										}
										if(roty[i]){
											glRotatef(roty[i]/4,1,0,0);
										}
										if(rotx[i]){
											glRotatef(-rotx[i]/4,0,0,1);
										}
										if(rotx[i]>10)rotx[i]=10;
										if(rotx[i]<-10)rotx[i]=-10;
										if(roty[i]>10)roty[i]=10;
										if(roty[i]<-10)roty[i]=-10;
									}

								}
								if(/*detail==2&&*/environment==snowyenvironment){
									if(type[i]==treeleavestype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*1.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
									if(type[i]==treetrunktype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
									if(type[i]==bushtype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*4*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
								}
								if(/*detail==2&&*/environment==grassyenvironment){
									if(type[i]==treeleavestype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*1.5*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
									if(type[i]==treetrunktype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*.5*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
									if(type[i]==bushtype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*4*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
								}
								if(/*detail==2&&*/environment==desertenvironment){
									if(type[i]==bushtype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*4*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
								}
								glRotatef(yaw[i],0,1,0);
								if(distance>1)distance=1;
								glColor4f((1-shadowed[i])/2+.5,(1-shadowed[i])/2+.5,(1-shadowed[i])/2+.5,distance);
								if(distance>=1){
									glDisable(GL_BLEND);
									glAlphaFunc(GL_GREATER, 0.5);
								}
								if(distance<1){
									glEnable(GL_BLEND);
									glAlphaFunc(GL_GREATER, 0.1);
								}
								if(type[i]!=treetrunktype&&type[i]!=treeleavestype&&type[i]!=bushtype&&type[i]!=rocktype){
									glEnable(GL_CULL_FACE);
									glAlphaFunc(GL_GREATER, 0.0001);
									model[i].drawdifftex(boxtextureptr);
									model[i].drawdecals(terrain.shadowtexture,terrain.bloodtexture,terrain.bloodtexture2,terrain.breaktexture);
								}
								if(type[i]==rocktype){
									glEnable(GL_CULL_FACE);
									glAlphaFunc(GL_GREATER, 0.0001);
									glColor4f((1-shadowed[i])/2+light.ambient[0],(1-shadowed[i])/2+light.ambient[1],(1-shadowed[i])/2+light.ambient[2],distance);
									model[i].drawdifftex(rocktextureptr);
									model[i].drawdecals(terrain.shadowtexture,terrain.bloodtexture,terrain.bloodtexture2,terrain.breaktexture);
								}
								if(type[i]==treeleavestype){
									glDisable(GL_CULL_FACE);
									glDisable(GL_LIGHTING);
									terrainlight=terrain.getLighting(position[i].x,position[i].z);
									if(!hidden){
										glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,distance);
										if(distance<1)glAlphaFunc(GL_GREATER, 0.2);
									}
									if(hidden){
										glDepthMask(0);
										glEnable(GL_BLEND);
										glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,distance/3);
										glAlphaFunc(GL_GREATER, 0);
									}
									model[i].drawdifftex(treetextureptr);
								}
								if(type[i]==bushtype){
									glDisable(GL_CULL_FACE);
									glDisable(GL_LIGHTING);
									terrainlight=terrain.getLighting(position[i].x,position[i].z);
									if(!hidden){
										glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,distance);
										if(distance<1)glAlphaFunc(GL_GREATER, 0.2);
									}
									if(hidden){
										glDepthMask(0);
										glEnable(GL_BLEND);
										glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,distance/3);
										glAlphaFunc(GL_GREATER, 0);
									}
									model[i].drawdifftex(bushtextureptr);
								}
								if(type[i]==treetrunktype){
									glEnable(GL_CULL_FACE);
									terrainlight=terrain.getLighting(position[i].x,position[i].z);
									glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,distance);
									model[i].drawdifftex(treetextureptr);
								}
							glPopMatrix();
						}
					}
				}
			}
		}
	}

	glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0 );
	for(i=0;i<numobjects;i++){
		if(type[i]==treeleavestype||type[i]==bushtype){
			moved=DoRotation(model[i].boundingspherecenter,0,yaw[i],0);
			if(frustum.SphereInFrustum(position[i].x+moved.x,position[i].y+moved.y,position[i].z+moved.z,model[i].boundingsphereradius)){   
				hidden=findDistancefastflat(&viewer,&position[i])<=playerdist+3;
				if(hidden){
					distance=1;
					if(distance>0){
						if(1==1||occluded[i]<6){
							glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
							glPushMatrix();
								glEnable(GL_LIGHTING);
								glDepthMask(1);
								glTranslatef(position[i].x,position[i].y,position[i].z);
								if(type[i]==bushtype){
									messedwith[i]-=multiplier;
									if(rotxvel[i]||rotx[i]){
										if(rotx[i]>0)rotxvel[i]-=multiplier*8*fabs(rotx[i]);
										if(rotx[i]<0)rotxvel[i]+=multiplier*8*fabs(rotx[i]);
										if(rotx[i]>0)rotxvel[i]-=multiplier*4;
										if(rotx[i]<0)rotxvel[i]+=multiplier*4;
										if(rotxvel[i]>0)rotxvel[i]-=multiplier*4;
										if(rotxvel[i]<0)rotxvel[i]+=multiplier*4;
										if(fabs(rotx[i])<multiplier*4)rotx[i]=0;
										if(fabs(rotxvel[i])<multiplier*4)rotxvel[i]=0;

										rotx[i]+=rotxvel[i]*multiplier*4;
									}
									if(rotyvel[i]||roty[i]){
										if(roty[i]>0)rotyvel[i]-=multiplier*8*fabs(roty[i]);
										if(roty[i]<0)rotyvel[i]+=multiplier*8*fabs(roty[i]);
										if(roty[i]>0)rotyvel[i]-=multiplier*4;
										if(roty[i]<0)rotyvel[i]+=multiplier*4;
										if(rotyvel[i]>0)rotyvel[i]-=multiplier*4;
										if(rotyvel[i]<0)rotyvel[i]+=multiplier*4;
										if(fabs(roty[i])<multiplier*4)roty[i]=0;
										if(fabs(rotyvel[i])<multiplier*4)rotyvel[i]=0;

										roty[i]+=rotyvel[i]*multiplier*4;
									}
									if(roty[i]){
										glRotatef(roty[i],1,0,0);
									}
									if(rotx[i]){
										glRotatef(-rotx[i],0,0,1);
									}
									if(rotx[i]>10)rotx[i]=10;
									if(rotx[i]<-10)rotx[i]=-10;
									if(roty[i]>10)roty[i]=10;
									if(roty[i]<-10)roty[i]=-10;
								}
								if(type[i]==treetrunktype||type[i]==treeleavestype){
									messedwith[i]-=multiplier;
									if(rotxvel[i]||rotx[i]){
										if(rotx[i]>0)rotxvel[i]-=multiplier*8*fabs(rotx[i]);
										if(rotx[i]<0)rotxvel[i]+=multiplier*8*fabs(rotx[i]);
										if(rotx[i]>0)rotxvel[i]-=multiplier*4;
										if(rotx[i]<0)rotxvel[i]+=multiplier*4;
										if(rotxvel[i]>0)rotxvel[i]-=multiplier*4;
										if(rotxvel[i]<0)rotxvel[i]+=multiplier*4;
										if(fabs(rotx[i])<multiplier*4)rotx[i]=0;
										if(fabs(rotxvel[i])<multiplier*4)rotxvel[i]=0;

										rotx[i]+=rotxvel[i]*multiplier*4;
									}
									if(rotyvel[i]||roty[i]){
										if(roty[i]>0)rotyvel[i]-=multiplier*8*fabs(roty[i]);
										if(roty[i]<0)rotyvel[i]+=multiplier*8*fabs(roty[i]);
										if(roty[i]>0)rotyvel[i]-=multiplier*4;
										if(roty[i]<0)rotyvel[i]+=multiplier*4;
										if(rotyvel[i]>0)rotyvel[i]-=multiplier*4;
										if(rotyvel[i]<0)rotyvel[i]+=multiplier*4;
										if(fabs(roty[i])<multiplier*4)roty[i]=0;
										if(fabs(rotyvel[i])<multiplier*4)rotyvel[i]=0;

										roty[i]+=rotyvel[i]*multiplier*4;
									}
									if(roty[i]){
										glRotatef(roty[i]/2,1,0,0);
									}
									if(rotx[i]){
										glRotatef(-rotx[i]/2,0,0,1);
									}
									if(rotx[i]>10)rotx[i]=10;
									if(rotx[i]<-10)rotx[i]=-10;
									if(roty[i]>10)roty[i]=10;
									if(roty[i]<-10)roty[i]=-10;
								}
								if(environment==snowyenvironment){
									if(type[i]==treeleavestype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*1.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
									if(type[i]==treetrunktype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
									if(type[i]==bushtype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*4*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
								}
								if(environment==grassyenvironment){
									if(type[i]==treeleavestype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*1.5*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
									if(type[i]==treetrunktype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*.5*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
									if(type[i]==bushtype){
										glRotatef((sin(windvar+position[i].x*.3)+.5)*4*.5*(sin(windvar*2+position[i].x*.3)+1)/2,1,0,0);
									}
								}
								glRotatef(yaw[i],0,1,0);
								glColor4f(1,1,1,distance);
								if(type[i]==treeleavestype){
									glDisable(GL_CULL_FACE);
									glDisable(GL_LIGHTING);
									terrainlight=terrain.getLighting(position[i].x,position[i].z);
									glDepthMask(0);
									glEnable(GL_BLEND);
									glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,.3);
									glAlphaFunc(GL_GREATER, 0);
									glDisable(GL_ALPHA_TEST);
									model[i].drawdifftex(treetextureptr);
								}
								if(type[i]==bushtype){
									glDisable(GL_CULL_FACE);
									glDisable(GL_LIGHTING);
									terrainlight=terrain.getLighting(position[i].x,position[i].z);
									glDepthMask(0);
									glEnable(GL_BLEND);
									glColor4f(terrainlight.x,terrainlight.y,terrainlight.z,.3);
									glAlphaFunc(GL_GREATER, 0);
									glDisable(GL_ALPHA_TEST);
									model[i].drawdifftex(bushtextureptr);
								}
							glPopMatrix();
						}
					}
				}
			}
		}
	}
	if(environment==desertenvironment)glTexEnvf( GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, 0 );
	glEnable(GL_ALPHA_TEST);						
	SetUpLight(&light,0);
}

void Objects::DeleteObject(int which)
{
	type[numobjects-1]=0;
	yaw[numobjects-1]=0;
	position[numobjects-1]=0;
	scale[numobjects-1]=0;
	friction[numobjects-1]=0;

	numobjects--;
}

void Objects::MakeObject(int atype, XYZ where, float ayaw, float ascale){
	if((atype!=treeleavestype&&atype!=bushtype)||foliage==1){
		scale[numobjects]=ascale;
		if(atype==treeleavestype)scale[numobjects]+=fabs((float)(Random()%100)/900)*ascale;

		onfire[numobjects]=0;
		flamedelay[numobjects]=0;
		type[numobjects]=atype;

		if(atype==firetype)onfire[numobjects]=1;

		position[numobjects]=where;
		if(atype==bushtype)position[numobjects].y=terrain.getHeight(position[numobjects].x,position[numobjects].z)-.3;
		yaw[numobjects]=ayaw;

		rotxvel[numobjects]=0;
		rotyvel[numobjects]=0;
		rotx[numobjects]=0;
		roty[numobjects]=0;

		if(atype==boxtype)model[numobjects].loaddecal((char *)":Data:Models:Box.solid",0);
		if(atype==cooltype)model[numobjects].loaddecal((char *)":Data:Models:Cool.solid",0);
		if(atype==walltype)model[numobjects].loaddecal((char *)":Data:Models:Wall.solid",0);
		if(atype==tunneltype)model[numobjects].loaddecal((char *)":Data:Models:Tunnel.solid",0);
		if(atype==chimneytype)model[numobjects].loaddecal((char *)":Data:Models:Chimney.solid",0);
		if(atype==spiketype)model[numobjects].load((char *)":Data:Models:Spike.solid",0);
		if(atype==weirdtype)model[numobjects].loaddecal((char *)":Data:Models:Weird.solid",0);
		if(atype==rocktype)model[numobjects].loaddecal((char *)":Data:Models:Rock.solid",0);
		if(atype==treetrunktype)model[numobjects].load((char *)":Data:Models:Treetrunk.solid",0);
		if(atype==treeleavestype)model[numobjects].load((char *)":Data:Models:Leaves.solid",0);
		if(atype==bushtype)model[numobjects].load((char *)":Data:Models:Bush.solid",0);

		if(atype==boxtype)friction[numobjects]=1.5;
		if(atype==cooltype)friction[numobjects]=1.5;
		if(atype==walltype)friction[numobjects]=1.5;
		if(atype==platformtype)friction[numobjects]=1.5;
		if(atype==tunneltype)friction[numobjects]=1.5;
		if(atype==chimneytype)friction[numobjects]=1.5;
		if(atype==rocktype)friction[numobjects]=.5;
		if(atype==rocktype&&ascale>.5)friction[numobjects]=1.5;
		if(atype==weirdtype)friction[numobjects]=1.5;
		if(atype==spiketype)friction[numobjects]=.4;
		if(atype==treetrunktype)friction[numobjects]=.4;
		if(atype==treeleavestype)friction[numobjects]=0;

		if(atype==platformtype){
			model[numobjects].loaddecal((char *)":Data:Models:Platform.solid",0);
			model[numobjects].Rotate(90,0,0);
		}

		if(type[numobjects]==boxtype||type[numobjects]==cooltype||type[numobjects]==spiketype||type[numobjects]==weirdtype||type[numobjects]==walltype||type[numobjects]==chimneytype||type[numobjects]==tunneltype||type[numobjects]==platformtype){
			model[numobjects].ScaleTexCoords(scale[numobjects]*1.5);
		}
		if(type[numobjects]==rocktype){
			model[numobjects].ScaleTexCoords(scale[numobjects]*3);
		}
		model[numobjects].flat=1;
		if(atype==treetrunktype||atype==treeleavestype||atype==rocktype){
			model[numobjects].flat=0;
		}
		model[numobjects].Scale(.3*scale[numobjects],.3*scale[numobjects],.3*scale[numobjects]);
		model[numobjects].Rotate(90,1,1);
		if(type[numobjects]==rocktype){
			model[numobjects].Rotate(ayaw*5,1,1);
		}
		model[numobjects].CalculateNormals(1);
		model[numobjects].ScaleNormals(-1,-1,-1);

		if(atype==treetrunktype&&position[numobjects].y<terrain.getHeight(position[numobjects].x,position[numobjects].z)+1){
			if(detail==2)terrain.MakeDecal(shadowdecalpermanent,position[numobjects],2,.4,0);
		}	

		if(atype==bushtype&&position[numobjects].y<terrain.getHeight(position[numobjects].x,position[numobjects].z)+1){
			if(detail==2)terrain.MakeDecal(shadowdecalpermanent,position[numobjects],1,.4,0);
		}

		if(atype!=treeleavestype&&atype!=bushtype&&atype!=firetype)
			terrain.AddObject(where+DoRotation(model[numobjects].boundingspherecenter,0,ayaw,0),model[numobjects].boundingsphereradius,numobjects);

		numobjects++;
	}	
}

void Objects::MakeObject(int atype, XYZ where, float ayaw, float apitch, float ascale){
	if((atype!=treeleavestype&&atype!=bushtype)||foliage==1){
		scale[numobjects]=ascale;
		if(atype==treeleavestype)scale[numobjects]+=fabs((float)(Random()%100)/900)*ascale;

		onfire[numobjects]=0;
		flamedelay[numobjects]=0;
		type[numobjects]=atype;

		if(atype==firetype)onfire[numobjects]=1;

		position[numobjects]=where;
		if(atype==bushtype)position[numobjects].y=terrain.getHeight(position[numobjects].x,position[numobjects].z)-.3;
		/*if(atype==firetype){
		if(position[numobjects].y<terrain.getHeight(position[numobjects].x,position[numobjects].z)-.3)
		position[numobjects].y=terrain.getHeight(position[numobjects].x,position[numobjects].z)-.3;
		}*/
		yaw[numobjects]=ayaw;
		pitch[numobjects]=apitch;

		rotxvel[numobjects]=0;
		rotyvel[numobjects]=0;
		rotx[numobjects]=0;
		roty[numobjects]=0;

		if(atype==boxtype)model[numobjects].loaddecal((char *)":Data:Models:Box.solid",0);
		if(atype==cooltype)model[numobjects].loaddecal((char *)":Data:Models:Cool.solid",0);
		if(atype==walltype)model[numobjects].loaddecal((char *)":Data:Models:Wall.solid",0);
		if(atype==tunneltype)model[numobjects].loaddecal((char *)":Data:Models:Tunnel.solid",0);
		if(atype==chimneytype)model[numobjects].loaddecal((char *)":Data:Models:Chimney.solid",0);
		if(atype==spiketype)model[numobjects].load((char *)":Data:Models:Spike.solid",0);
		if(atype==weirdtype)model[numobjects].loaddecal((char *)":Data:Models:Weird.solid",0);
		if(atype==rocktype)model[numobjects].loaddecal((char *)":Data:Models:Rock.solid",0);
		if(atype==treetrunktype)model[numobjects].load((char *)":Data:Models:Treetrunk.solid",0);
		if(atype==treeleavestype)model[numobjects].load((char *)":Data:Models:Leaves.solid",0);
		if(atype==bushtype)model[numobjects].load((char *)":Data:Models:Bush.solid",0);

		if(atype==boxtype)friction[numobjects]=1.5;
		if(atype==cooltype)friction[numobjects]=1.5;
		if(atype==walltype)friction[numobjects]=1.5;
		if(atype==platformtype)friction[numobjects]=1.5;
		if(atype==tunneltype)friction[numobjects]=1.5;
		if(atype==chimneytype)friction[numobjects]=1.5;
		if(atype==rocktype)friction[numobjects]=.5;
		if(atype==rocktype&&ascale>.5)friction[numobjects]=1.5;
		if(atype==weirdtype)friction[numobjects]=1.5;
		if(atype==spiketype)friction[numobjects]=.4;
		if(atype==treetrunktype)friction[numobjects]=.4;
		if(atype==treeleavestype)friction[numobjects]=0;

		if(friction[numobjects]==1.5&&fabs(apitch)>5)friction[numobjects]=.5;

		if(atype==platformtype){
			model[numobjects].loaddecal((char *)":Data:Models:Platform.solid",0);
			model[numobjects].Rotate(90,0,0);
		}

		if(type[numobjects]==boxtype||type[numobjects]==cooltype||type[numobjects]==spiketype||type[numobjects]==weirdtype||type[numobjects]==walltype||type[numobjects]==chimneytype||type[numobjects]==tunneltype||type[numobjects]==platformtype){
			model[numobjects].ScaleTexCoords(scale[numobjects]*1.5);
		}
		if(type[numobjects]==rocktype){
			model[numobjects].ScaleTexCoords(scale[numobjects]*3);
		}
		model[numobjects].flat=1;
		if(atype==treetrunktype||atype==treeleavestype||atype==rocktype){
			model[numobjects].flat=0;
		}
		model[numobjects].Scale(.3*scale[numobjects],.3*scale[numobjects],.3*scale[numobjects]);
		model[numobjects].Rotate(90,1,1);
		model[numobjects].Rotate(apitch,0,0);
		if(type[numobjects]==rocktype){
			model[numobjects].Rotate(ayaw*5,0,0);
		}
		model[numobjects].CalculateNormals(1);
		model[numobjects].ScaleNormals(-1,-1,-1);

		if(atype==treetrunktype&&position[numobjects].y<terrain.getHeight(position[numobjects].x,position[numobjects].z)+1){
			if(detail==2)terrain.MakeDecal(shadowdecalpermanent,position[numobjects],2,.4,0);
		}	

		if(atype==bushtype&&position[numobjects].y<terrain.getHeight(position[numobjects].x,position[numobjects].z)+1){
			if(detail==2)terrain.MakeDecal(shadowdecalpermanent,position[numobjects],1,.4,0);
		}

		if(atype!=treeleavestype&&atype!=bushtype&&atype!=firetype)
			terrain.AddObject(where+DoRotation(model[numobjects].boundingspherecenter,0,ayaw,0),model[numobjects].boundingsphereradius,numobjects);

		numobjects++;
	}	
}

void Objects::DoStuff()
{
	XYZ spawnpoint;
	for(int i=0;i<numobjects;i++){
		/*if(type[i]==firetype){
		Sprite::MakeSprite(weaponshinesprite, position[i],position[i]*0, 1,1,1, 5, 1);
		}*/

		if(type[i]==firetype)onfire[i]=1;
		if(onfire[i]){
			if(type[i]==bushtype)flamedelay[i]-=multiplier*3;
			if(type[i]==firetype)flamedelay[i]-=multiplier*3;
			if(type[i]==treeleavestype)flamedelay[i]-=multiplier*4;
			while(flamedelay[i]<0&&onfire[i]){
				flamedelay[i]+=.006;
				if(type[i]==bushtype||type[i]==firetype){
					spawnpoint.x=((float)(Random()%100))/30*scale[i];
					spawnpoint.y=((float)(Random()%100)+60)/30*scale[i];
					spawnpoint.z=0;
					spawnpoint=DoRotation(spawnpoint,0,Random()%360,0);
					spawnpoint+=position[i];
					Sprite::MakeSprite(flamesprite, spawnpoint,spawnpoint*0, 1,1,1, (.6+(float)abs(Random()%100)/200-.25)*5*scale[i], 1);
				}
				if(type[i]==treeleavestype){
					spawnpoint.x=((float)(Random()%100))/80*scale[i];
					spawnpoint.y=((float)(Random()%100)+80)/12*scale[i];
					spawnpoint.z=0;
					spawnpoint=DoRotation(spawnpoint,0,Random()%360,0);
					spawnpoint+=position[i];
					Sprite::MakeSprite(flamesprite, spawnpoint,spawnpoint*0, 1,1,1, (.6+(float)abs(Random()%100)/200-.25)*6, 1);
				}
			}

		}
	}
}

void Objects::DoShadows()
{
	int i,j,k,l;
	static XYZ testpoint,testpoint2, terrainpoint,lightloc,col;
	lightloc=light.location;
	if(!skyboxtexture)lightloc=0;
	lightloc.y+=10;
	Normalise(&lightloc);
	int patchx,patchz;

	if(numobjects>0)
		for(i=0;i<numobjects;i++){
			if(type[i]!=treeleavestype&&type[i]!=treetrunktype&&type[i]!=bushtype&&type[i]!=firetype){
				for(j=0;j<model[i].vertexNum;j++){
					terrainpoint=position[i]+DoRotation(model[i].vertex[j]+model[i].normals[j]*.1,0,yaw[i],0);
					//terrainpoint.y+=model[i].boundingsphereradius;
					shadowed[i]=0;
					patchx=terrainpoint.x/(terrain.size/subdivision*terrain.scale);
					patchz=terrainpoint.z/(terrain.size/subdivision*terrain.scale);
					if(patchx>=0&&patchz>=0&&patchx<subdivision&&patchz<subdivision)
						if(terrain.patchobjectnum[patchx][patchz])
							for(k=0;k<terrain.patchobjectnum[patchx][patchz];k++){
								l=terrain.patchobjects[patchx][patchz][k];
								if(type[l]!=treetrunktype/*&&l!=i*/){
									testpoint=terrainpoint;
									testpoint2=terrainpoint+lightloc*50*(1-shadowed[i]);
									if(model[l].LineCheck(&testpoint,&testpoint2,&col,&position[l],&yaw[l])!=-1){
										shadowed[i]=1-(findDistance(&terrainpoint,&col)/50);	
									}
								}
							}
							if(shadowed[i]>0){
								col=model[i].normals[j]-DoRotation(lightloc*shadowed[i],0,-yaw[i],0);
								Normalise(&col);
								for(k=0;k<model[i].TriangleNum;k++){
									if(model[i].Triangles[k].vertex[0]==j){
										l=k*24;
										model[i].vArray[l+2]=col.x;
										model[i].vArray[l+3]=col.y;
										model[i].vArray[l+4]=col.z;
									}
									if(model[i].Triangles[k].vertex[1]==j){
										l=k*24;
										model[i].vArray[l+10]=col.x;
										model[i].vArray[l+11]=col.y;
										model[i].vArray[l+12]=col.z;
									}
									if(model[i].Triangles[k].vertex[2]==j){
										l=k*24;
										model[i].vArray[l+18]=col.x;
										model[i].vArray[l+19]=col.y;
										model[i].vArray[l+20]=col.z;
									}
								}
							}
				}
			}
			shadowed[i]=0;
		}
}

Objects::Objects()
{
	center = 0;
	radius = 0;
	numobjects = 0;
	boxtextureptr = 0;
	treetextureptr = 0;
	bushtextureptr = 0;
	rocktextureptr = 0;

	memset(position, 0, sizeof(position));
	memset(type, 0, sizeof(type));
	memset(yaw, 0, sizeof(yaw));
	memset(pitch, 0, sizeof(pitch));
	memset(rotx, 0, sizeof(rotx));
	memset(rotxvel, 0, sizeof(rotxvel));
	memset(roty, 0, sizeof(roty));
	memset(rotyvel, 0, sizeof(rotyvel));
	memset(possible, 0, sizeof(possible));
	memset(model, 0, sizeof(model));
	memset(displaymodel, 0, sizeof(displaymodel));
	memset(friction, 0, sizeof(friction));
	memset(scale, 0, sizeof(scale));
	memset(messedwith, 0, sizeof(messedwith));
	memset(checked, 0, sizeof(checked));
	memset(shadowed, 0, sizeof(shadowed));
	memset(occluded, 0, sizeof(occluded));
	memset(onfire, 0, sizeof(onfire));
	memset(flamedelay, 0, sizeof(flamedelay));
}	

Objects::~Objects()
{
	if (boxtextureptr) glDeleteTextures( 1, &boxtextureptr );
	if (treetextureptr) glDeleteTextures( 1, &treetextureptr );
	if (bushtextureptr) glDeleteTextures( 1, &bushtextureptr );
	if (rocktextureptr) glDeleteTextures( 1, &rocktextureptr );
};

