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
#include "Person.h"
#include "openal_wrapper.h"
#include "Animation.h"
#include "Sounds.h"
#include "Awards.h"
#include "Game.h"

extern float multiplier;
extern Terrain terrain;
extern float gravity;
extern int environment;
extern int detail;
extern FRUSTUM frustum;
extern XYZ viewer;
extern float realmultiplier;
extern int slomo;
extern float slomodelay;
extern bool cellophane;
extern float texdetail;
extern float realtexdetail;
extern GLubyte bloodText[512*512*3];
extern GLubyte wolfbloodText[512*512*3];
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
extern bool decals;
extern float fadestart;
extern bool freeze;
extern bool winfreeze;
extern float flashamount,flashr,flashg,flashb;
extern int flashdelay;
extern bool showpoints;
extern bool immediate;
extern int test;
extern bool tilt2weird;
extern bool tiltweird;
extern bool midweird;
extern bool proportionweird;
extern bool vertexweird[6];
extern XYZ envsound[30];
extern float envsoundvol[30];
extern float envsoundlife[30];
extern int numenvsounds;
extern int tutoriallevel;
extern float smoketex;
extern int tutorialstage;
extern bool reversaltrain;
extern bool canattack;
extern bool cananger;
extern float damagedealt;
extern int hostile;
extern float hostiletime;

extern int indialogue;

extern bool gamestarted;

Person player[maxplayers];

void Person::CheckKick()
{
  if (!(hasvictim
	&& (targetanimation == rabbitkickanim
	    && victim
	    && victim != this
	    && currentframe >= 2
	    && currentanimation == rabbitkickanim)
	&& (findDistancefast(&coords,&victim->coords) < 1.2)
	&& (!victim->skeleton.free)))
    return;

  if (animation[victim->targetanimation].height!=lowheight)
    {
      float damagemult = (creature == wolftype ? 2.5 : 1.) * power * power;
      XYZ relative = velocity;
      relative.y=0;
      Normalise(&relative);

      victim->spurt=1;
      DoBlood(.2,250);
      if(tutoriallevel!=1)
	emit_sound_at(heavyimpactsound, victim->coords);
      victim->RagDoll(0);
      for(int i=0;i<victim->skeleton.num_joints;i++){
	victim->skeleton.joints[i].velocity+=relative*120*damagemult;
      }
      victim->Puff(neck);
      victim->DoDamage(100*damagemult/victim->protectionhigh);
      if(id==0)camerashake+=.4;

      target=0;
      currentframe=3;
      targetanimation=backflipanim;
      targetframe=4;
      velocity=facing*-10;
      velocity.y=5;
      skeleton.free=0;
      if(id==0)
	resume_stream(whooshsound);

      award_bonus(id, cannon);
    }
  else if (victim->isCrouch())
    {
      targetanimation=rabbitkickreversedanim;
      currentanimation=rabbitkickreversedanim;
      victim->currentanimation=rabbitkickreversalanim;
      victim->targetanimation=rabbitkickreversalanim;
      targettilt2=0;
      currentframe=0;
      targetframe=1;
      target=0;
      velocity=0;
      victim->oldcoords=victim->coords;
      coords=victim->coords;
      victim->targetrotation=targetrotation;
      victim->victim=this;
    }
}

void Person::CatchFire(){
	XYZ flatfacing,flatvelocity;
	int howmany;
	for(int i=0;i<10;i++){
		howmany=abs(Random()%(skeleton.num_joints));
		if(!skeleton.free)flatvelocity=velocity;
		if(skeleton.free)flatvelocity=skeleton.joints[howmany].velocity;
		if(!skeleton.free)flatfacing=DoRotation(DoRotation(DoRotation(skeleton.joints[howmany].position,0,0,tilt),tilt2,0,0),0,rotation,0)*scale+coords;
		if(skeleton.free)flatfacing=skeleton.joints[howmany].position*scale+coords;
		Sprite::MakeSprite(flamesprite, flatfacing,flatvelocity, 1,1,1, 2, 1);
	}

	onfiredelay=0.5;

	emit_sound_at(firestartsound, coords);

	emit_stream_at(stream_firesound, coords);

	flamedelay=0;

	onfire=1;
}

int Person::getIdle(){
	if(indialogue!=-1&&howactive==typeactive&&creature==rabbittype)return talkidleanim;
	if(hasvictim&&victim!=this/*||(id==0&&attackkeydown)*/)if(/*(id==0&&attackkeydown)||*/(!victim->dead&&victim->aitype!=passivetype&&victim->aitype!=searchtype&&aitype!=passivetype&&aitype!=searchtype&&victim->id<numplayers)){
		if((aitype==playercontrolled&&stunned<=0&&weaponactive==-1)||pause){
			if(creature==rabbittype)return fightidleanim;
			if(creature==wolftype)return wolfidle;
		}
		if(aitype==playercontrolled&&stunned<=0&&weaponactive!=-1){
			if(weapons.type[weaponids[weaponactive]]==knife)return knifefightidleanim;
			if(weapons.type[weaponids[weaponactive]]==sword&&victim->weaponactive!=-1)return swordfightidlebothanim;
			if(weapons.type[weaponids[weaponactive]]==sword)return swordfightidleanim;
			if(weapons.type[weaponids[weaponactive]]==staff)return swordfightidleanim;
		}
		if(aitype!=playercontrolled&&stunned<=0&&creature!=wolftype&&!pause)return fightsidestep;
	}
	if((damage>permanentdamage||damage>damagetolerance*.8||deathbleeding>0)&&creature!=wolftype)return hurtidleanim;
	if(howactive==typesitting)return sitanim;
	if(howactive==typesittingwall)return sitwallanim;
	if(howactive==typesleeping)return sleepanim;
	if(howactive==typedead1)return dead1anim;
	if(howactive==typedead2)return dead2anim;
	if(howactive==typedead3)return dead3anim;
	if(howactive==typedead4)return dead4anim;
	if(creature==rabbittype)return bounceidleanim;
	if(creature==wolftype)return wolfidle;
	return 0;
}

int Person::getCrouch(){
	if(creature==rabbittype)return crouchanim;
	if(creature==wolftype)return wolfcrouchanim;
	return 0;
}

int Person::getRun(){
	if(creature==rabbittype&&(!superruntoggle||weaponactive!=-1))return runanim;
	if(creature==wolftype&&(!superruntoggle))return wolfrunanim;

	if(creature==rabbittype&&(superruntoggle&&weaponactive==-1))return rabbitrunninganim;
	if(creature==wolftype&&(superruntoggle))return wolfrunninganim;
	return 0;
}

int Person::getStop(){
	if(creature==rabbittype)return stopanim;
	if(creature==wolftype)return wolfstopanim;
	return 0;
}

int Person::getLanding(){
	if(creature==rabbittype)return landanim;
	if(creature==wolftype)return wolflandanim;
	return 0;
}

int Person::getLandhard(){
	if(creature==rabbittype)return landhardanim;
	if(creature==wolftype)return wolflandhardanim;
	return 0;
}

static void
SolidHitBonus(int playerid)
{
  if (bonustime < 1.5 && bonus >= solidhit && bonus <= megacombo)
    award_bonus(playerid, bonus == megacombo ? bonus : bonus + 1);
  else
    award_bonus(playerid, solidhit);
}

void Person::DoBlood(float howmuch,int which){
	static int bleedxint,bleedyint;
	static XYZ bloodvel;
	//if(howmuch&&id==0)blooddimamount=1;
	if(bloodtoggle&&tutoriallevel!=1){
		if(bleeding<=0&&spurt){
			spurt=0;
			for(int i=0;i<3;i++){
				bloodvel=0;
				if(!skeleton.free){
					bloodvel.z=10;
					bloodvel=DoRotation(bloodvel,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
				}
				if(skeleton.free){
					bloodvel-=DoRotation(skeleton.forward*10*scale,((float)(Random()%100))/4,((float)(Random()%100))/4,0);
				}
				if(skeleton.free)bloodvel+=DoRotation(skeleton.joints[skeleton.jointlabels[head]].velocity,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
				if(!skeleton.free)bloodvel+=DoRotation(velocity,((float)(Random()%100))/4,((float)(Random()%100))/4,0)*scale;
				if(skeleton.free){
					Sprite::MakeSprite(bloodsprite, skeleton.joints[skeleton.jointlabels[head]].position*scale+coords,bloodvel, 1,1,1, .05, 1);
					Sprite::MakeSprite(bloodflamesprite, skeleton.joints[skeleton.jointlabels[head]].position*scale+coords,bloodvel, 1,1,1, .3, 1);
				}
				if(!skeleton.free){
					Sprite::MakeSprite(bloodsprite, DoRotation((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .05, 1);
					Sprite::MakeSprite(bloodflamesprite, DoRotation((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .3, 1);
				}
			}
			if(Random()%2==0)
				for(int i=0;i<3;i++){
					if(Random()%2!=0){
						bloodvel=0;
						if(skeleton.free) {
							bloodvel-=DoRotation(skeleton.forward*10*scale,((float)(Random()%100))/4,((float)(Random()%100))/4,0);
							bloodvel+=DoRotation(skeleton.joints[skeleton.jointlabels[head]].velocity,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
						} else {
							bloodvel.z=10;
							bloodvel=DoRotation(bloodvel,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
							bloodvel+=DoRotation(velocity,((float)(Random()%100))/4,((float)(Random()%100))/4,0)*scale;
						}
						bloodvel*=.2;
						if(skeleton.free){
							Sprite::MakeSprite(splintersprite, skeleton.joints[skeleton.jointlabels[head]].position*scale+coords,bloodvel, 1,1,1, .05, 1);
						} else {
							Sprite::MakeSprite(splintersprite, DoRotation((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .05, 1);
						}
						Sprite::setLastSpriteSpecial(3);
					}
				}
		}
		if(decals){
			bleeding=howmuch+(float)abs(Random()%100)/200-.25;
			bleedxint=0;
			bleedyint=0;
			int texdetailint=realtexdetail;
			if(creature==rabbittype)
				while(bloodText[bleedxint*512*3+bleedyint*3+0]>which+4||bloodText[bleedxint*512*3+bleedyint*3+0]<which-4||bleedxint<10||bleedyint<10||bleedxint>500||bleedyint>500){
					bleedxint=abs(Random()%512);
					bleedyint=abs(Random()%512);
				}
			if(creature==wolftype)
				while(wolfbloodText[bleedxint*512*3+bleedyint*3+0]>which+4||wolfbloodText[bleedxint*512*3+bleedyint*3+0]<which-4||bleedxint<10||bleedyint<10||bleedxint>500||bleedyint>500){
					bleedxint=abs(Random()%512);
					bleedyint=abs(Random()%512);
				}
			bleedy=bleedxint;
			bleedx=bleedyint;
			bleedy/=realtexdetail;
			bleedx/=realtexdetail;
			direction=abs(Random()%2)*2-1;
		}

	}
	if(bleeding>2)bleeding=2;
}

void Person::DoBloodBig(float howmuch,int which){
	static int bleedxint,bleedyint,i,j;
	static XYZ bloodvel;
	if(howmuch&&id==0)blooddimamount=1;

	if(tutoriallevel!=1||id==0)
		if(aitype!=playercontrolled&&howmuch>0){
			int whichsound=-1;

			if(creature==wolftype){
				int i=abs(Random()%2);
				if(i==0)whichsound=snarlsound;
				if(i==1)whichsound=snarl2sound;
				envsound[numenvsounds]=coords;
				envsoundvol[numenvsounds]=16;
				envsoundlife[numenvsounds]=.4;
				numenvsounds++;
			}
			if(creature==rabbittype){
				int i=abs(Random()%2);
				if(i==0)whichsound=rabbitpainsound;
				if(i==1&&howmuch>=2)whichsound=rabbitpain1sound;
				envsound[numenvsounds]=coords;
				envsoundvol[numenvsounds]=16;
				envsoundlife[numenvsounds]=.4;
				numenvsounds++;
				//if(i==2)whichsound=rabbitpain2sound;
			}

			if(whichsound!=-1)
			  emit_sound_at(whichsound, coords);
		}

		if(id==0&&howmuch>0){
			flashamount=.5;
			flashr=1;
			flashg=0;
			flashb=0;
			flashdelay=0;
		}

		if(bloodtoggle&&decals&&tutoriallevel!=1){
			if(bleeding<=0&&spurt){
				spurt=0;
				for(int i=0;i<3;i++){
					bloodvel=0;
					if(!skeleton.free){
						bloodvel.z=10;
						bloodvel=DoRotation(bloodvel,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
					}
					if(skeleton.free){
						bloodvel-=DoRotation(skeleton.forward*10*scale,((float)(Random()%100))/4,((float)(Random()%100))/4,0);
					}
					if(skeleton.free)bloodvel+=DoRotation(skeleton.joints[skeleton.jointlabels[head]].velocity,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
					if(!skeleton.free)bloodvel+=DoRotation(velocity,((float)(Random()%100))/4,((float)(Random()%100))/4,0)*scale;
					if(skeleton.free){
						Sprite::MakeSprite(bloodsprite, skeleton.joints[skeleton.jointlabels[head]].position*scale+coords,bloodvel, 1,1,1, .05, 1);
						Sprite::MakeSprite(bloodflamesprite, skeleton.joints[skeleton.jointlabels[head]].position*scale+coords,bloodvel, 1,1,1, .3, 1);
					}
					if(!skeleton.free){
						Sprite::MakeSprite(bloodsprite, DoRotation((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .05, 1);
						Sprite::MakeSprite(bloodflamesprite, DoRotation((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .3, 1);
					}
				}
			}
			int offsetx=0,offsety=0;
			if(which==225){
				offsety=Random()%40;
				offsetx=abs(Random()%60);
			}
			if(which==190||which==185){
				offsety=Random()%40;
				offsetx=abs(Random()%100)-20;
			}
			if(which==175){
				offsety=Random()%10;
				offsetx=Random()%10;
			}
			if(which==170){
				offsety=Random()%20;
				offsetx=Random()%20;
			}
			if(which==220||which==215){
				//offsety=Random()%20;
				offsetx=20;
				//offsetx=abs(Random()%80);
			}


			int startx=512;
			int starty=512;
			int endx=0;
			int endy=0;
			GLubyte color;
			if(creature==rabbittype)
				for(i=0;i<512;i++){
					for(j=0;j<512;j++){
						if(bloodText[i*512*3+j*3+0]<=which+4&&bloodText[i*512*3+j*3+0]>=which-4){
							if(i<startx)startx=i;
							if(j<starty)starty=j;
							if(i>endx)endx=i;
							if(j>endy)endy=j;
						}
					}
				}
				if(creature==wolftype)
					for(i=0;i<512;i++){
						for(j=0;j<512;j++){
							if(wolfbloodText[i*512*3+j*3+0]<=which+4&&wolfbloodText[i*512*3+j*3+0]>=which-4){
								if(i<startx)startx=i;
								if(j<starty)starty=j;
								if(i>endx)endx=i;
								if(j>endy)endy=j;
							}
						}
					}

					startx+=offsetx;
					endx+=offsetx;
					starty+=offsety;
					endy+=offsety;

					if(startx<0)startx=0;
					if(starty<0)starty=0;
					if(endx>512-1)endx=512-1;
					if(endy>512-1)endy=512-1;
					if(endx<startx)endx=startx;
					if(endy<starty)endy=starty;

					startx/=realtexdetail;
					starty/=realtexdetail;
					endx/=realtexdetail;
					endy/=realtexdetail;

					int texdetailint=realtexdetail;
					int where;
					if(creature==rabbittype)
						for(i=startx;i<endx;i++){
							for(j=starty;j<endy;j++){
								if(bloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]<=which+4&&bloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]>=which-4){
									color=Random()%85+170;
									where=i*skeleton.skinsize*3+j*3;
									if(skeleton.skinText[where+0]>color/2)skeleton.skinText[where+0]=color/2;
									skeleton.skinText[where+1]=0;
									skeleton.skinText[where+2]=0;
								}
							}
						}
						if(creature==wolftype)
							for(i=startx;i<endx;i++){
								for(j=starty;j<endy;j++){
									if(wolfbloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]<=which+4&&wolfbloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]>=which-4){
										color=Random()%85+170;
										where=i*skeleton.skinsize*3+j*3;
										if(skeleton.skinText[where+0]>color/2)skeleton.skinText[where+0]=color/2;
										skeleton.skinText[where+1]=0;
										skeleton.skinText[where+2]=0;
									}
								}
							}
							glBindTexture(GL_TEXTURE_2D,skeleton.drawmodel.textureptr);
							DoMipmaps();

							bleedxint=0;
							bleedyint=0;
							if(creature==rabbittype)
								while(bloodText[bleedxint*512*3+bleedyint*3+0]>which+4||bloodText[bleedxint*512*3+bleedyint*3+0]<which-4||bleedxint<10||bleedyint<10||bleedxint>500||bleedyint>500){
									bleedxint=abs(Random()%512);
									bleedyint=abs(Random()%512);
								}
								if(creature==wolftype)
									while(wolfbloodText[bleedxint*512*3+bleedyint*3+0]>which+4||wolfbloodText[bleedxint*512*3+bleedyint*3+0]<which-4||bleedxint<10||bleedyint<10||bleedxint>500||bleedyint>500){
										bleedxint=abs(Random()%512);
										bleedyint=abs(Random()%512);
									}
									bleedy=bleedxint+offsetx;
									bleedx=bleedyint+offsety;
									bleedy/=realtexdetail;
									bleedx/=realtexdetail;
									if(bleedx<0)bleedx=0;
									if(bleedy<0)bleedy=0;
									if(bleedx>skeleton.skinsize-1)bleedx=skeleton.skinsize-1;
									if(bleedy>skeleton.skinsize-1)bleedy=skeleton.skinsize-1;
									direction=abs(Random()%2)*2-1;

		}
		bleeding=howmuch+(float)abs(Random()%100)/200-.25;
		deathbleeding+=bleeding;
		bloodloss+=bleeding*3;

		if(tutoriallevel!=1&&aitype!=playercontrolled&&bloodloss>damagetolerance*2/3&&bloodloss<damagetolerance&&creature==rabbittype){
			if(abs(Random()%2)==0){aitype=gethelptype;
			lastseentime=12;
			}
			else aitype=attacktypecutoff;
			ally=0;
		}
		if(bleeding>2)bleeding=2;
}

bool Person::DoBloodBigWhere(float howmuch,int which, XYZ where){
	static int bleedxint,bleedyint,i,j;
	static XYZ bloodvel;
	static XYZ startpoint,endpoint,colpoint,movepoint;
	static float rotationpoint;
	static int whichtri;
	static XYZ p1,p2,p3,p0;
	static XYZ N,temp;
	XYZ bary;
	XYZ gxx,gyy;
	float coordsx,coordsy;
	float total;

	if(bloodtoggle&&decals&&tutoriallevel!=1){
		where-=coords;
		if(!skeleton.free)where=DoRotation(where,0,-rotation,0);
		//where=scale;
		startpoint=where;
		startpoint.y+=100;
		endpoint=where;
		endpoint.y-=100;
		movepoint=0;
		rotationpoint=0;
		whichtri=skeleton.drawmodel.LineCheck(&startpoint,&endpoint, &colpoint, &movepoint, &rotationpoint);
		if(whichtri!=-1){
			p0=colpoint;
			p1=skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[whichtri].vertex[0]];
			p2=skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[whichtri].vertex[1]];
			p3=skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[whichtri].vertex[2]];
			/*
			CrossProduct(p2-p1,p3-p1,&N);
			CrossProduct(p0-p1,p3-p1,&temp);
			s =  dotproduct(&temp,&N)/findLength(&N);
			CrossProduct(p2-p1,p1-p0,&temp);
			t = dotproduct(&temp,&N)/findLength(&N);
			r = 1 - (s + t);*/

			bary.x=findDistancefast(&p0,&p1);
			bary.y=findDistancefast(&p0,&p2);
			bary.z=findDistancefast(&p0,&p3);

			total=bary.x+bary.y+bary.z;
			bary.x/=total;
			bary.y/=total;
			bary.z/=total;

			bary.x=1-bary.x;
			bary.y=1-bary.y;
			bary.z=1-bary.z;

			total=bary.x+bary.y+bary.z;
			bary.x/=total;
			bary.y/=total;
			bary.z/=total;


			gxx.x=skeleton.drawmodel.Triangles[whichtri].gx[0];
			gxx.y=skeleton.drawmodel.Triangles[whichtri].gx[1];
			gxx.z=skeleton.drawmodel.Triangles[whichtri].gx[2];
			gyy.x=skeleton.drawmodel.Triangles[whichtri].gy[0];
			gyy.y=skeleton.drawmodel.Triangles[whichtri].gy[1];
			gyy.z=skeleton.drawmodel.Triangles[whichtri].gy[2];
			coordsx=skeleton.drawmodel.Triangles[whichtri].gx[0]*bary.x+skeleton.drawmodel.Triangles[whichtri].gx[1]*bary.y+skeleton.drawmodel.Triangles[whichtri].gx[2]*bary.z;
			coordsy=skeleton.drawmodel.Triangles[whichtri].gy[0]*bary.x+skeleton.drawmodel.Triangles[whichtri].gy[1]*bary.y+skeleton.drawmodel.Triangles[whichtri].gy[2]*bary.z;

			//coordsx=skeleton.drawmodel.Triangles[whichtri].gx[1];
			//coordsy=skeleton.drawmodel.Triangles[whichtri].gy[1];

			if(bleeding<=0&&spurt){
				spurt=0;
				for(int i=0;i<3;i++){
					bloodvel=0;
					if(!skeleton.free){
						bloodvel.z=10;
						bloodvel=DoRotation(bloodvel,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
					}
					if(skeleton.free){
						bloodvel-=DoRotation(skeleton.forward*10*scale,((float)(Random()%100))/4,((float)(Random()%100))/4,0);
					}
					if(skeleton.free)bloodvel+=DoRotation(skeleton.joints[skeleton.jointlabels[head]].velocity,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
					if(!skeleton.free)bloodvel+=DoRotation(velocity,((float)(Random()%100))/4,((float)(Random()%100))/4,0)*scale;
					if(skeleton.free){
						Sprite::MakeSprite(bloodsprite, skeleton.joints[skeleton.jointlabels[head]].position*scale+coords,bloodvel, 1,1,1, .05, 1);
						Sprite::MakeSprite(bloodflamesprite, skeleton.joints[skeleton.jointlabels[head]].position*scale+coords,bloodvel, 1,1,1, .3, 1);
					}
					if(!skeleton.free){
						Sprite::MakeSprite(bloodsprite, DoRotation((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .05, 1);
						Sprite::MakeSprite(bloodflamesprite, DoRotation((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .3, 1);
					}
				}
			}
			int offsetx=0,offsety=0;
			/*if(which==225){
			offsety=Random()%40;
			offsetx=abs(Random()%120);
			}
			if(which==220||which==215){
			offsety=Random()%20;
			offsetx=abs(Random()%80);
			}*/
			//which=220;
			offsetx=(1+coordsy)*512-291;
			offsety=coordsx*512-437;

			int startx=512;
			int starty=512;
			int endx=0;
			int endy=0;
			GLubyte color;
			if(creature==rabbittype)
				for(i=0;i<512;i++){
					for(j=0;j<512;j++){
						if(bloodText[i*512*3+j*3+0]<=which+4&&bloodText[i*512*3+j*3+0]>=which-4){
							if(i<startx)startx=i;
							if(j<starty)starty=j;
							if(i>endx)endx=i;
							if(j>endy)endy=j;
						}
					}
				}
				if(creature==wolftype)
					for(i=0;i<512;i++){
						for(j=0;j<512;j++){
							if(wolfbloodText[i*512*3+j*3+0]<=which+4&&wolfbloodText[i*512*3+j*3+0]>=which-4){
								if(i<startx)startx=i;
								if(j<starty)starty=j;
								if(i>endx)endx=i;
								if(j>endy)endy=j;
							}
						}
					}
					startx+=offsetx;
					endx+=offsetx;
					starty+=offsety;
					endy+=offsety;

					if(startx<0)startx=0;
					if(starty<0)starty=0;
					if(endx>512-1)endx=512-1;
					if(endy>512-1)endy=512-1;
					if(endx<startx)endx=startx;
					if(endy<starty)endy=starty;

					startx/=realtexdetail;
					starty/=realtexdetail;
					endx/=realtexdetail;
					endy/=realtexdetail;

					int texdetailint=realtexdetail;
					int where;
					if(creature==rabbittype)
						for(i=startx;i<endx;i++){
							for(j=starty;j<endy;j++){
								if(bloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]<=which+4&&bloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]>=which-4){
									color=Random()%85+170;
									where=i*skeleton.skinsize*3+j*3;
									if(skeleton.skinText[where+0]>color/2)skeleton.skinText[where+0]=color/2;
									skeleton.skinText[where+1]=0;
									skeleton.skinText[where+2]=0;
								}
								else if(bloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]<=160+4&&bloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]>=160-4){
									color=Random()%85+170;
									where=i*skeleton.skinsize*3+j*3;
									if(skeleton.skinText[where+0]>color/2)skeleton.skinText[where+0]=color/2;
									skeleton.skinText[where+1]=0;
									skeleton.skinText[where+2]=0;
								}
							}
						}
						if(creature==wolftype)
							for(i=startx;i<endx;i++){
								for(j=starty;j<endy;j++){
									if(wolfbloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]<=which+4&&wolfbloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]>=which-4){
										color=Random()%85+170;
										where=i*skeleton.skinsize*3+j*3;
										if(skeleton.skinText[where+0]>color/2)skeleton.skinText[where+0]=color/2;
										skeleton.skinText[where+1]=0;
										skeleton.skinText[where+2]=0;
									}
									else if(wolfbloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]<=160+4&&wolfbloodText[(i*texdetailint-offsetx)*512*3+(j*texdetailint-offsety)*3+0]>=160-4){
										color=Random()%85+170;
										where=i*skeleton.skinsize*3+j*3;
										if(skeleton.skinText[where+0]>color/2)skeleton.skinText[where+0]=color/2;
										skeleton.skinText[where+1]=0;
										skeleton.skinText[where+2]=0;
									}
								}
							}
							glBindTexture(GL_TEXTURE_2D,skeleton.drawmodel.textureptr);
							DoMipmaps();

							bleedy=(1+coordsy)*512;
							bleedx=coordsx*512;
							bleedy/=realtexdetail;
							bleedx/=realtexdetail;
							if(bleedx<0)bleedx=0;
							if(bleedy<0)bleedy=0;
							if(bleedx>skeleton.skinsize-1)bleedx=skeleton.skinsize-1;
							if(bleedy>skeleton.skinsize-1)bleedy=skeleton.skinsize-1;
							direction=abs(Random()%2)*2-1;
		}
		if(whichtri==-1)return 0;
	}
	bleeding=howmuch+(float)abs(Random()%100)/200-.25;
	deathbleeding+=bleeding;
	bloodloss+=bleeding*3;

	if(tutoriallevel!=1&&aitype!=playercontrolled&&bloodloss>damagetolerance*2/3&&bloodloss<damagetolerance&&creature==rabbittype){
		if(abs(Random()%2)==0){aitype=gethelptype;
		lastseentime=12;
		}
		else aitype=attacktypecutoff;
		ally=0;
	}
	if(bleeding>2)bleeding=2;
	return 1;
}



void Person::Reverse()
{
  if (!((victim->aitype == playercontrolled
	 || hostiletime > 1
	 || staggerdelay <= 0)
	&& victim->targetanimation != jumpupanim
	&& victim->targetanimation != jumpdownanim
	&& (tutoriallevel != 1 || cananger)
	&& hostile))
    return;

  if (normaldotproduct (victim->facing, victim->coords-coords) > 0
      && (victim->id != 0 || difficulty >= 2)
      && (creature != wolftype || victim->creature == wolftype))
    return;

  if(targetanimation==sweepanim){
    targetanimation=sweepreversedanim;
    currentanimation=sweepreversedanim;
    victim->currentanimation=sweepreversalanim;
    victim->targetanimation=sweepreversalanim;
  }
  if(targetanimation==spinkickanim){
    targetanimation=spinkickreversedanim;
    currentanimation=spinkickreversedanim;
    victim->currentanimation=spinkickreversalanim;
    victim->targetanimation=spinkickreversalanim;
  }
  if(targetanimation==upunchanim||targetanimation==rabbittacklinganim){
    if(targetanimation==rabbittacklinganim){
      currentframe=6;
      targetframe=7;
      victim->currentframe=6;
      victim->targetframe=7;
    }
    targetanimation=upunchreversedanim;
    currentanimation=upunchreversedanim;
    victim->currentanimation=upunchreversalanim;
    victim->targetanimation=upunchreversalanim;
  }
  if(targetanimation==staffhitanim&&findDistancefast(&victim->coords,&coords)<2&&((victim->id==0&&victim->crouchkeydown)||Random()%4==0)){
    if(victim->weaponactive!=-1){
      victim->throwtogglekeydown=1;
      weapons.owner[victim->weaponids[0]]=-1;
      weapons.velocity[victim->weaponids[0]]=victim->velocity*.2;
      if(weapons.velocity[victim->weaponids[0]].x==0)weapons.velocity[victim->weaponids[0]].x=.1;
      weapons.tipvelocity[victim->weaponids[0]]=weapons.velocity[victim->weaponids[0]];
      weapons.missed[victim->weaponids[0]]=1;
      weapons.freetime[victim->weaponids[0]]=0;
      weapons.firstfree[victim->weaponids[0]]=1;
      weapons.physics[victim->weaponids[0]]=1;
      victim->num_weapons--;
      if(victim->num_weapons){
	victim->weaponids[0]=victim->weaponids[victim->num_weapons];
	if(victim->weaponstuck==victim->num_weapons)victim->weaponstuck=0;
      }

      victim->weaponactive=-1;
      for(int j=0;j<numplayers;j++){
	player[j].wentforweapon=0;
      }
    }

    targetanimation=staffhitreversedanim;
    currentanimation=staffhitreversedanim;
    victim->currentanimation=staffhitreversalanim;
    victim->targetanimation=staffhitreversalanim;
  }
  if(targetanimation==staffspinhitanim&&findDistancefast(&victim->coords,&coords)<2&&((victim->id==0&&victim->crouchkeydown)||Random()%2==0)){
    if(victim->weaponactive!=-1){
      victim->throwtogglekeydown=1;
      weapons.owner[victim->weaponids[0]]=-1;
      weapons.velocity[victim->weaponids[0]]=victim->velocity*.2;
      if(weapons.velocity[victim->weaponids[0]].x==0)weapons.velocity[victim->weaponids[0]].x=.1;
      weapons.tipvelocity[victim->weaponids[0]]=weapons.velocity[victim->weaponids[0]];
      weapons.missed[victim->weaponids[0]]=1;
      weapons.freetime[victim->weaponids[0]]=0;
      weapons.firstfree[victim->weaponids[0]]=1;
      weapons.physics[victim->weaponids[0]]=1;
      victim->num_weapons--;
      if(victim->num_weapons){
	victim->weaponids[0]=victim->weaponids[victim->num_weapons];
	if(victim->weaponstuck==victim->num_weapons)victim->weaponstuck=0;
      }

      victim->weaponactive=-1;
      for(int j=0;j<numplayers;j++){
	player[j].wentforweapon=0;
      }
    }
    targetanimation=staffspinhitreversedanim;
    currentanimation=staffspinhitreversedanim;
    victim->currentanimation=staffspinhitreversalanim;
    victim->targetanimation=staffspinhitreversalanim;
  }
  if(targetanimation==swordslashanim&&findDistancefast(&victim->coords,&coords)<2&&((victim->id==0&&victim->crouchkeydown)||Random()%4==0)){
    if(victim->weaponactive!=-1){
      victim->throwtogglekeydown=1;
      weapons.owner[victim->weaponids[0]]=-1;
      weapons.velocity[victim->weaponids[0]]=victim->velocity*.2;
      if(weapons.velocity[victim->weaponids[0]].x==0)weapons.velocity[victim->weaponids[0]].x=.1;
      weapons.tipvelocity[victim->weaponids[0]]=weapons.velocity[victim->weaponids[0]];
      weapons.missed[victim->weaponids[0]]=1;
      weapons.freetime[victim->weaponids[0]]=0;
      weapons.firstfree[victim->weaponids[0]]=1;
      weapons.physics[victim->weaponids[0]]=1;
      victim->num_weapons--;
      if(victim->num_weapons){
	victim->weaponids[0]=victim->weaponids[victim->num_weapons];
	if(victim->weaponstuck==victim->num_weapons)victim->weaponstuck=0;
      }

      victim->weaponactive=-1;
      for(int j=0;j<numplayers;j++){
	player[j].wentforweapon=0;
      }
    }
    targetanimation=swordslashreversedanim;
    currentanimation=swordslashreversedanim;
    victim->currentanimation=swordslashreversalanim;
    victim->targetanimation=swordslashreversalanim;
  }
  if(targetanimation==knifeslashstartanim&&findDistancefast(&victim->coords,&coords)<2&&(victim->id==0||Random()%4==0)){
    if(victim->weaponactive!=-1){
      victim->throwtogglekeydown=1;
      weapons.owner[victim->weaponids[0]]=-1;
      weapons.velocity[victim->weaponids[0]]=victim->velocity*.2;
      if(weapons.velocity[victim->weaponids[0]].x==0)weapons.velocity[victim->weaponids[0]].x=.1;
      weapons.tipvelocity[victim->weaponids[0]]=weapons.velocity[victim->weaponids[0]];
      weapons.missed[victim->weaponids[0]]=1;
      weapons.freetime[victim->weaponids[0]]=0;
      weapons.firstfree[victim->weaponids[0]]=1;
      weapons.physics[victim->weaponids[0]]=1;
      victim->num_weapons--;
      if(victim->num_weapons){
	victim->weaponids[0]=victim->weaponids[victim->num_weapons];
	if(victim->weaponstuck==victim->num_weapons)victim->weaponstuck=0;
      }

      victim->weaponactive=-1;
      for(int j=0;j<numplayers;j++){
	player[j].wentforweapon=0;
      }
    }
    targetanimation=knifeslashreversedanim;
    currentanimation=knifeslashreversedanim;
    victim->currentanimation=knifeslashreversalanim;
    victim->targetanimation=knifeslashreversalanim;
  }
  if(targetanimation!=knifeslashstartanim&&targetanimation!=staffhitanim&&targetanimation!=staffspinhitanim&&targetanimation!=winduppunchanim&&targetanimation!=wolfslapanim&&targetanimation!=swordslashanim&&targetanimation!=swordslashanim){
    victim->targettilt2=targettilt2;
    victim->currentframe=currentframe;
    victim->targetframe=targetframe;
    victim->target=target;
    victim->velocity=0;
    victim->oldcoords=victim->coords;
    victim->coords=coords;
    victim->targetrotation=targetrotation;
    victim->rotation=targetrotation;
    victim->victim=this;
  }
  if(targetanimation==winduppunchanim){
    targetanimation=winduppunchblockedanim;
    victim->targetanimation=blockhighleftanim;
    victim->targetframe=1;
    victim->target=.5;
    victim->victim=this;
    victim->targetrotation=targetrotation+180;
  }
  if(targetanimation==wolfslapanim){
    targetanimation=winduppunchblockedanim;
    victim->targetanimation=blockhighleftanim;
    victim->targetframe=1;
    victim->target=.5;
    victim->victim=this;
    victim->targetrotation=targetrotation+180;
  }
  if((targetanimation==swordslashanim||targetanimation==staffhitanim||targetanimation==staffspinhitanim)&&victim->weaponactive!=-1){
    targetanimation=swordslashparriedanim;
    parriedrecently=.4;
    victim->parriedrecently=0;
    victim->targetanimation=swordslashparryanim;
    victim->targetframe=1;
    victim->target=.5;
    victim->victim=this;
    victim->targetrotation=targetrotation+180;

    if(abs(Random()%20)==0||weapons.type[victim->weaponids[victim->weaponactive]]==knife){
      if(victim->weaponactive!=-1){
	if(weapons.type[victim->weaponids[0]]==staff||weapons.type[weaponids[0]]==staff){
	  if(weapons.type[victim->weaponids[0]]==staff)weapons.damage[victim->weaponids[0]]+=.2+float(abs(Random()%100)-50)/250;
	  if(weapons.type[weaponids[0]]==staff)weapons.damage[weaponids[0]]+=.2+float(abs(Random()%100)-50)/250;
	  emit_sound_at(swordstaffsound, victim->coords);
	}
	else{
	  emit_sound_at(metalhitsound, victim->coords);
	}
      }
      XYZ aim;
      victim->Puff(righthand);
      victim->target=0;
      victim->targetframe=0;
      victim->targetanimation=staggerbackhighanim;
      victim->targetrotation=targetrotation+180;
      victim->target=0;
      weapons.owner[victim->weaponids[0]]=-1;
      aim=DoRotation(facing,0,90,0)*21;
      aim.y+=7;
      weapons.velocity[victim->weaponids[0]]=aim*-.2;
      weapons.tipvelocity[victim->weaponids[0]]=aim;
      weapons.missed[victim->weaponids[0]]=1;
      weapons.hitsomething[victim->weaponids[0]]=0;
      weapons.freetime[victim->weaponids[0]]=0;
      weapons.firstfree[victim->weaponids[0]]=1;
      weapons.physics[victim->weaponids[0]]=1;
      victim->num_weapons--;
      if(victim->num_weapons){
	victim->weaponids[0]=victim->weaponids[num_weapons];
	if(victim->weaponstuck==victim->num_weapons)victim->weaponstuck=0;
      }
      victim->weaponactive=-1;
      for(int i=0;i<numplayers;i++){
	player[i].wentforweapon=0;
      }
    }

    if(abs(Random()%20)==0){
      if(weaponactive!=-1){
	if(weapons.type[victim->weaponids[0]]==staff||weapons.type[weaponids[0]]==staff){
	  if(weapons.type[victim->weaponids[0]]==staff)weapons.damage[victim->weaponids[0]]+=.2+float(abs(Random()%100)-50)/250;
	  if(weapons.type[weaponids[0]]==staff)weapons.damage[weaponids[0]]+=.2+float(abs(Random()%100)-50)/250;

	  emit_sound_at(swordstaffsound, coords);
	}
	else{
	  emit_sound_at(metalhitsound, coords);
	}
      }

      XYZ aim;
      Puff(righthand);
      target=0;
      targetframe=0;
      targetanimation=staggerbackhighanim;
      targetrotation=targetrotation+180;
      target=0;
      weapons.owner[weaponids[0]]=-1;
      aim=DoRotation(facing,0,90,0)*21;
      aim.y+=7;
      weapons.velocity[weaponids[0]]=aim*-.2;
      weapons.tipvelocity[weaponids[0]]=aim;
      weapons.hitsomething[weaponids[0]]=0;
      weapons.missed[weaponids[0]]=1;
      weapons.freetime[weaponids[0]]=0;
      weapons.firstfree[weaponids[0]]=1;
      weapons.physics[weaponids[0]]=1;
      num_weapons--;
      if(num_weapons){
	weaponids[0]=weaponids[num_weapons];
	if(weaponstuck==num_weapons)weaponstuck=0;
      }
      weaponactive=-1;
      for(int i=0;i<numplayers;i++){
	player[i].wentforweapon=0;
      }


    }
  }
  if(hasvictim)
    if(targetanimation==knifeslashstartanim||targetanimation==swordslashanim||targetanimation==staffhitanim||targetanimation==staffspinhitanim){
      if((targetanimation!=staffhitanim&&targetanimation!=staffspinhitanim)||findDistancefast(&coords,&victim->coords)>.2){
	victim->targetanimation=dodgebackanim;
	victim->targetframe=0;
	victim->target=0;

	XYZ rotatetarget;
	rotatetarget=coords-victim->coords;
	Normalise(&rotatetarget);
	victim->targetrotation=-asin(0-rotatetarget.x);
	victim->targetrotation*=360/6.28;
	if(rotatetarget.z<0)victim->targetrotation=180-victim->targetrotation;

	victim->targettilt2=-asin(rotatetarget.y)*360/6.28;//*-70;

	victim->lastattack3=victim->lastattack2;
	victim->lastattack2=victim->lastattack;
	victim->lastattack=victim->targetanimation;
      }
      else
	{
	  victim->targetanimation=sweepanim;
	  victim->targetframe=0;
	  victim->target=0;

	  XYZ rotatetarget;
	  rotatetarget=coords-victim->coords;
	  Normalise(&rotatetarget);
	  victim->targetrotation=-asin(0-rotatetarget.x);
	  victim->targetrotation*=360/6.28;
	  if(rotatetarget.z<0)victim->targetrotation=180-victim->targetrotation;

	  victim->targettilt2=-asin(rotatetarget.y)*360/6.28;//*-70;

	  victim->lastattack3=victim->lastattack2;
	  victim->lastattack2=victim->lastattack;
	  victim->lastattack=victim->targetanimation;
	}
    }

  velocity=0;
  victim->velocity=0;

  if(aitype!=playercontrolled)feint=0;
  if(aitype!=playercontrolled&&Random()%3==0&&escapednum<2&&difficulty==2)feint=1;
  if(aitype!=playercontrolled&&Random()%5==0&&escapednum<2&&difficulty==1)feint=1;
  if(aitype!=playercontrolled&&Random()%10==0&&escapednum<2&&difficulty==0)feint=1;

  if(victim->id==0&&animation[victim->targetanimation].attack==reversal)numreversals++;
}

void Person::DoDamage(float howmuch){
	if(tutoriallevel!=1)damage+=howmuch/power;
	if(id!=0)damagedealt+=howmuch/power;
	if(id==0)damagetaken+=howmuch/power;

	if(id==0&&(bonus==solidhit||bonus==twoxcombo||bonus==threexcombo||bonus==fourxcombo||bonus==megacombo))bonus=0;
	if(tutoriallevel!=1)permanentdamage+=howmuch/2/power;
	if(tutoriallevel!=1)superpermanentdamage+=howmuch/4/power;
	if(permanentdamage>damagetolerance/2&&permanentdamage-howmuch<damagetolerance/2&&Random()%2)DoBlood(1,255);
	if((permanentdamage>damagetolerance*.8&&Random()%2&&!deathbleeding)||spurt)DoBlood(1,255);
	spurt=0;
	if(id==0)camerashake+=howmuch/100;
	if(id==0&&((howmuch>50&&damage>damagetolerance/2)))blackout=damage/damagetolerance;
	if(blackout>1)blackout=1;

	if(aitype==passivetype&&damage<damagetolerance&&((tutoriallevel!=1||cananger)&&hostile))aitype=attacktypecutoff;
	if(tutoriallevel!=1&&aitype!=playercontrolled&&damage<damagetolerance&&damage>damagetolerance*2/3&&creature==rabbittype){
		if(abs(Random()%2)==0){aitype=gethelptype;
		lastseentime=12;
		}
		else aitype=attacktypecutoff;
		ally=0;
	}

	if(howmuch>damagetolerance*50&&skeleton.free!=2){
		XYZ flatvelocity2;
		XYZ flatfacing2;
		for(int i=0;i<skeleton.num_joints; i++){
			if(!skeleton.free)flatvelocity2=velocity;
			if(skeleton.free)flatvelocity2=skeleton.joints[i].velocity;
			if(!skeleton.free)flatfacing2=DoRotation(DoRotation(DoRotation(skeleton.joints[i].position,0,0,tilt),tilt2,0,0),0,rotation,0)*scale+coords;
			if(skeleton.free)flatfacing2=skeleton.joints[i].position*scale+coords;
			flatvelocity2.x+=(float)(abs(Random()%100)-50)/10;
			flatvelocity2.y+=(float)(abs(Random()%100)-50)/10;
			flatvelocity2.z+=(float)(abs(Random()%100)-50)/10;
			Sprite::MakeSprite(bloodflamesprite, flatfacing2,flatvelocity2, 1,1,1, 3, 1);
			Sprite::MakeSprite(bloodsprite, flatfacing2,flatvelocity2, 1,1,1, .4, 1);
			Sprite::MakeSprite(cloudsprite, flatfacing2,flatvelocity2*0, .6,0,0, 1, .5);
		}

		emit_sound_at(splattersound, coords);

		skeleton.free=2;
		DoDamage(10000);
		RagDoll(0);
		/*if(autoslomo){
		slomo=1;
		slomodelay=.2;
		}*/
		if(!dead&&creature==wolftype){
		  award_bonus(0, Wolfbonus);
		}
		dead=2;
		coords=20;
	}

	if(tutoriallevel!=1||id==0)
		if(speechdelay<=0&&!dead&&aitype!=playercontrolled){
			int whichsound=-1;

			if(creature==wolftype){
				int i=abs(Random()%2);
				if(i==0)whichsound=snarlsound;
				if(i==1)whichsound=snarl2sound;
				envsound[numenvsounds]=coords;
				envsoundvol[numenvsounds]=16;
				envsoundlife[numenvsounds]=.4;
				numenvsounds++;
			}
			if(creature==rabbittype){
				int i=abs(Random()%2);
				if(i==0)whichsound=rabbitpainsound;
				if(i==1&&damage>damagetolerance)whichsound=rabbitpain1sound;
				envsound[numenvsounds]=coords;
				envsoundvol[numenvsounds]=16;
				envsoundlife[numenvsounds]=.4;
				numenvsounds++;
				//if(i==2)whichsound=rabbitpain2sound;
			}

			if(whichsound!=-1){
				emit_sound_at(whichsound, coords);
			}
		}
		speechdelay=.3;

		//if(permanentdamage>=damagetolerance&&howmuch<50)permanentdamage=damagetolerance-1;
		//if(damage>=damagetolerance&&howmuch<30&&!dead)damage=damagetolerance-1;
}

void Person::DoHead(){
	static XYZ rotatearound;
	static XYZ facing;
	static float lookspeed=500;

	if(!freeze&&!winfreeze){

		//head facing
		targetheadrotation=(float)((int)((0-rotation-targetheadrotation+180)*100)%36000)/100;
		targetheadrotation2=(float)((int)(targetheadrotation2*100)%36000)/100;

		while(targetheadrotation>180)targetheadrotation-=360;
		while(targetheadrotation<-180)targetheadrotation+=360;

		if(targetheadrotation>160)targetheadrotation2=targetheadrotation2*-1;
		if(targetheadrotation<-160)targetheadrotation2=targetheadrotation2*-1;
		if(targetheadrotation>160)targetheadrotation=targetheadrotation-180;
		if(targetheadrotation<-160)targetheadrotation=targetheadrotation+180;

		if(targetheadrotation2>120)targetheadrotation2=120;
		if(targetheadrotation2<-120)targetheadrotation2=-120;
		if(targetheadrotation>120)targetheadrotation=120;
		if(targetheadrotation<-120)targetheadrotation=-120;

		if(!isIdle())targetheadrotation2=0;
		if(isIdle()){
			if(targetheadrotation>80)targetheadrotation=80;
			if(targetheadrotation<-80)targetheadrotation=-80;
			if(targetheadrotation2>50)targetheadrotation2=50;
			if(targetheadrotation2<-50)targetheadrotation2=-50;
		}

		if(abs(headrotation-targetheadrotation)<multiplier*lookspeed)headrotation=targetheadrotation;
		else if(headrotation>targetheadrotation){
			headrotation-=multiplier*lookspeed;
		}
		else if(headrotation<targetheadrotation){
			headrotation+=multiplier*lookspeed;
		}

		if(abs(headrotation2-targetheadrotation2)<multiplier*lookspeed/2)headrotation2=targetheadrotation2;
		else if(headrotation2>targetheadrotation2){
			headrotation2-=multiplier*lookspeed/2;
		}
		else if(headrotation2<targetheadrotation2){
			headrotation2+=multiplier*lookspeed/2;
		}

		rotatearound=skeleton.joints[skeleton.jointlabels[neck]].position;
		skeleton.joints[skeleton.jointlabels[head]].position=rotatearound+DoRotation(skeleton.joints[skeleton.jointlabels[head]].position-rotatearound,headrotation2,0,0);

		facing=0;
		facing.z=-1;
		if(targetanimation!=bounceidleanim&&targetanimation!=fightidleanim&&targetanimation!=wolfidle&&targetanimation!=knifefightidleanim&&targetanimation!=drawrightanim&&targetanimation!=drawleftanim&&targetanimation!=walkanim){
			facing=DoRotation(facing,headrotation2*.4,0,0);
			facing=DoRotation(facing,0,headrotation*.4,0);
		}

		if(targetanimation==bounceidleanim||targetanimation==fightidleanim||targetanimation==wolfidle||targetanimation==knifefightidleanim||targetanimation==drawrightanim||targetanimation==drawleftanim){
			facing=DoRotation(facing,headrotation2*.8,0,0);
			facing=DoRotation(facing,0,headrotation*.8,0);
		}

		if(targetanimation==walkanim){
			facing=DoRotation(facing,headrotation2*.6,0,0);
			facing=DoRotation(facing,0,headrotation*.6,0);
		}

		skeleton.specialforward[0]=facing;
		//skeleton.specialforward[0]=DoRotation(facing,0,rotation,0);
		static int i;
		for(i=0;i<skeleton.num_muscles;i++){
			if(skeleton.muscles[i].visible&&(skeleton.muscles[i].parent1->label==head||skeleton.muscles[i].parent2->label==head))
			{
				skeleton.FindRotationMuscle(i,targetanimation);
			}
		}
	}
}

void Person::RagDoll(bool checkcollision){
	static XYZ change;
	static int l,i,j;
	static float speed;
	if(!skeleton.free){
		if(id==0)numfalls++;
		if(id==0&&isFlip())numflipfail++;

		escapednum=0;

		facing=0;
		facing.z=1;
		facing=DoRotation(facing,0,rotation,0);

		skeleton.freetime=0;

		skeleton.longdead=0;

		skeleton.free=1;
		skeleton.broken=0;
		skeleton.spinny=1;
		freefall=1;
		skeleton.freefall=1;

		if(!isnormal(velocity.x))velocity.x=0;
		if(!isnormal(velocity.y))velocity.y=0;
		if(!isnormal(velocity.z))velocity.z=0;
		if(!isnormal(rotation))rotation=0;
		if(!isnormal(coords.x))coords=0;
		if(!isnormal(tilt))tilt=0;
		if(!isnormal(tilt2))tilt2=0;

		for(i=0;i<skeleton.num_joints;i++){
			skeleton.joints[i].delay=0;
			skeleton.joints[i].locked=0;
			skeleton.joints[i].position=DoRotation(DoRotation(DoRotation(skeleton.joints[i].position,0,0,tilt),tilt2,0,0),0,rotation,0);
			if(!isnormal(skeleton.joints[i].position.x))skeleton.joints[i].position=DoRotation(skeleton.joints[i].position,0,rotation,0);
			if(!isnormal(skeleton.joints[i].position.x))skeleton.joints[i].position=skeleton.joints[i].position;
			if(!isnormal(skeleton.joints[i].position.x))skeleton.joints[i].position=coords;
			skeleton.joints[i].position.y+=.1;
			skeleton.joints[i].oldposition=skeleton.joints[i].position;
			skeleton.joints[i].realoldposition=skeleton.joints[i].position*scale+coords;
		}

		for(i=0;i<skeleton.num_joints;i++){
			skeleton.joints[i].velocity=0;
			skeleton.joints[i].velchange=0;
		}
		skeleton.DoConstraints(&coords,&scale);
		if(animation[currentanimation].height==lowheight||animation[targetanimation].height==lowheight)
		{
			skeleton.DoConstraints(&coords,&scale);
			skeleton.DoConstraints(&coords,&scale);
			skeleton.DoConstraints(&coords,&scale);
			skeleton.DoConstraints(&coords,&scale);
		}

		speed=animation[targetanimation].speed[targetframe]*2;
		if(animation[currentanimation].speed[currentframe]>animation[targetanimation].speed[targetframe]){
			speed=animation[currentanimation].speed[currentframe]*2;
		}
		if(transspeed)speed=transspeed*2;

		speed*=speedmult;

		for(i=0;i<skeleton.num_joints;i++){
			if((animation[currentanimation].attack!=reversed||currentanimation==swordslashreversedanim)&&currentanimation!=rabbitkickanim&&!isLanding()&&!wasLanding()&&animation[currentanimation].height==animation[targetanimation].height)skeleton.joints[i].velocity=velocity/scale+facing*5+DoRotation(DoRotation(DoRotation((animation[targetanimation].position[i][targetframe]-animation[currentanimation].position[i][currentframe])*speed,0,0,tilt),tilt2,0,0),0,rotation,0);
			else skeleton.joints[i].velocity=velocity/scale+facing*5;
			change.x=(float)(Random()%100)/100;
			change.y=(float)(Random()%100)/100;
			change.z=(float)(Random()%100)/100;
			skeleton.joints[i].velocity+=change;
			skeleton.joints[abs(Random()%skeleton.num_joints)].velocity-=change;

			change.x=(float)(Random()%100)/100;
			change.y=(float)(Random()%100)/100;
			change.z=(float)(Random()%100)/100;
			skeleton.joints[i].velchange+=change;
			skeleton.joints[abs(Random()%skeleton.num_joints)].velchange-=change;
		}

		if(checkcollision){
			XYZ average;
			XYZ lowpoint;
			XYZ colpoint;
			int howmany;
			average=0;
			howmany=0;
			for(j=0;j<skeleton.num_joints;j++){
				average+=skeleton.joints[j].position;
				howmany++;
			}
			average/=howmany;
			coords+=average*scale;
			for(j=0;j<skeleton.num_joints;j++){
				skeleton.joints[j].position-=average;
			}

			whichpatchx=coords.x/(terrain.size/subdivision*terrain.scale*terraindetail);
			whichpatchz=coords.z/(terrain.size/subdivision*terrain.scale*terraindetail);
			if(terrain.patchobjectnum[whichpatchx][whichpatchz])
				for(l=0;l<terrain.patchobjectnum[whichpatchx][whichpatchz];l++){
					i=terrain.patchobjects[whichpatchx][whichpatchz][l];
					lowpoint=coords;
					lowpoint.y+=1;
					if(SphereCheck(&lowpoint, 3, &colpoint, &objects.position[i], &objects.rotation[i], &objects.model[i])!=-1){
						coords.x=lowpoint.x;
						coords.z=lowpoint.z;
					}
				}
		}

		rotation=0;
		updatedelay=0;

		velocity=0;
		for(i=0;i<skeleton.num_joints;i++){
			velocity+=skeleton.joints[i].velocity*scale;
		}
		velocity/=skeleton.num_joints;

		if(Random()%2==0){
			if(weaponactive!=-1&&targetanimation!=rabbitkickanim&&num_weapons>0){
				weapons.owner[weaponids[0]]=-1;
				weapons.hitsomething[weaponids[0]]=0;
				weapons.velocity[weaponids[0]]=skeleton.joints[skeleton.jointlabels[righthand]].velocity*scale*-.3;
				weapons.velocity[weaponids[0]].x+=.01;
				weapons.tipvelocity[weaponids[0]]=skeleton.joints[skeleton.jointlabels[righthand]].velocity*scale;
				weapons.missed[weaponids[0]]=1;
				weapons.freetime[weaponids[0]]=0;
				weapons.firstfree[weaponids[0]]=1;
				weapons.physics[weaponids[0]]=1;
				num_weapons--;
				if(num_weapons){
					weaponids[0]=weaponids[num_weapons];
					if(weaponstuck==num_weapons)weaponstuck=0;
				}
				weaponactive=-1;
				for(i=0;i<numplayers;i++){
					player[i].wentforweapon=0;
				}
			}
		}

		targetanimation=bounceidleanim;
		currentanimation=bounceidleanim;
		targetframe=0;
		currentframe=0;
	}
}



void Person::FootLand(int which, float opacity){
	static XYZ terrainlight;
	static XYZ footvel,footpoint;
	if(opacity>=1||skiddelay<=0)
		if(opacity>1)
		{
			footvel=0;
			if(which==0)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
			if(which==1)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
			//footpoint.y=coords.y;
			if(findDistancefast(&footpoint,&viewer))Sprite::MakeSprite(cloudsprite, footpoint,footvel, 1,1,1, .5, .2*opacity);
		}
		else if(environment==snowyenvironment&&onterrain&&terrain.getOpacity(coords.x,coords.z)<.2){
			footvel=velocity/5;
			if(footvel.y<.8)footvel.y=.8;
			if(which==0)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
			if(which==1)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
			footpoint.y=terrain.getHeight(footpoint.x,footpoint.z);
			terrainlight=terrain.getLighting(footpoint.x,footpoint.z);
			if(findDistancefast(&footpoint,&viewer)<viewdistance*viewdistance/4)Sprite::MakeSprite(cloudsprite, footpoint,footvel*.6, terrainlight.x,terrainlight.y,terrainlight.z, .5, .7*opacity);
			if(opacity>=1||detail==2)if(detail==2)if(findDistancefast(&footpoint,&viewer)<viewdistance*viewdistance/4)terrain.MakeDecal(footprintdecal,footpoint,.2,1*opacity,rotation);
		}
		else if(environment==grassyenvironment&&onterrain&&terrain.getOpacity(coords.x,coords.z)<.2){
			footvel=velocity/5;
			if(footvel.y<.8)footvel.y=.8;
			if(which==0)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
			if(which==1)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
			footpoint.y=terrain.getHeight(footpoint.x,footpoint.z);
			terrainlight=terrain.getLighting(footpoint.x,footpoint.z);
			if(findDistancefast(&footpoint,&viewer)<viewdistance*viewdistance/4)Sprite::MakeSprite(cloudsprite, footpoint,footvel*.6, terrainlight.x*90/255,terrainlight.y*70/255,terrainlight.z*8/255, .5, .5*opacity);
		}
		else if(environment==desertenvironment&&onterrain&&terrain.getOpacity(coords.x,coords.z)<.2){
			footvel=velocity/5;
			if(footvel.y<.8)footvel.y=.8;
			if(which==0)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
			if(which==1)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
			footpoint.y=terrain.getHeight(footpoint.x,footpoint.z);
			terrainlight=terrain.getLighting(footpoint.x,footpoint.z);
			if(findDistancefast(&footpoint,&viewer)<viewdistance*viewdistance/4)Sprite::MakeSprite(cloudsprite, footpoint,footvel*.6, terrainlight.x*190/255,terrainlight.y*170/255,terrainlight.z*108/255, .5, .7*opacity);
			if(opacity>=1||detail==2)if(detail==2)if(findDistancefast(&footpoint,&viewer)<viewdistance*viewdistance/4)terrain.MakeDecal(footprintdecal,footpoint,.2,.25*opacity,rotation);
		}
		else if(isLanding()||targetanimation==jumpupanim||isLandhard())
		{
			footvel=velocity/5;
			if(footvel.y<.8)footvel.y=.8;
			if(which==0)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
			if(which==1)footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
			//footpoint.y=coords.y;
			if(findDistancefast(&footpoint,&viewer)<viewdistance*viewdistance/4)Sprite::MakeSprite(cloudsprite, footpoint,footvel*.6, 1,1,1, .5, .2*opacity);
		}
}

void Person::Puff(int whichlabel){
	static XYZ footvel,footpoint;

	footvel=0;
	footpoint=DoRotation(skeleton.joints[skeleton.jointlabels[whichlabel]].position,0,rotation,0)*scale+coords;
	Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,1,1, .9, .3);
}


void	Person::DoAnimations(){
	if(!skeleton.free){
		int i = 0;
		static float oldtarget;

		if(isIdle()&&currentanimation!=getIdle())normalsupdatedelay=0;

		if(targetanimation==tempanim||currentanimation==tempanim){
			animation[tempanim]=tempanimation;
		}
		if(targetanimation==jumpupanim||targetanimation==jumpdownanim||isFlip()){
			float gLoc[3];
			float vel[3];
			gLoc[0]=coords.x;
			gLoc[1]=coords.y;
			gLoc[2]=coords.z;
			vel[0]=velocity.x;
			vel[1]=velocity.y;
			vel[2]=velocity.z;

			if(id==0){
				OPENAL_3D_SetAttributes(channels[whooshsound], gLoc, vel);
				OPENAL_SetVolume(channels[whooshsound], 64*findLength(&velocity)/5);
			}
			if(((velocity.y<-15)||(crouchkeydown&&velocity.y<-8))&&abs(velocity.y)*4>fast_sqrt(velocity.x*velocity.x*velocity.z*velocity.z))landhard=1;
			if(!crouchkeydown&&velocity.y>=-15)landhard=0;
		}
		if((currentanimation==jumpupanim||targetanimation==jumpdownanim)/*&&velocity.y<40*/&&!isFlip()&&(!isLanding()&&!isLandhard())&&((crouchkeydown&&!crouchtogglekeydown))){
			XYZ targfacing;
			targfacing=0;
			targfacing.z=1;

			targfacing=DoRotation(targfacing,0,targetrotation,0);

			if(normaldotproduct(targfacing,velocity)>=-.3)targetanimation=flipanim;
			else targetanimation=backflipanim;
			crouchtogglekeydown=1;
			targetframe=0;
			target=0;

			if(id==0)numflipped++;
		}

		if(animation[targetanimation].attack!=reversed)feint=0;
		if(!crouchkeydown||(isLanding()||isLandhard())||(wasLanding()||wasLandhard())){
			crouchtogglekeydown=0;
			if(aitype==playercontrolled)feint=0;
		}
		else
		{
			if(!crouchtogglekeydown&&animation[targetanimation].attack==reversed&&aitype==playercontrolled&&(escapednum<2||reversaltrain))feint=1;
			if(!isFlip())crouchtogglekeydown=1;
		}


		if(animation[targetanimation].attack||currentanimation==getupfrombackanim||currentanimation==getupfromfrontanim){
			if(detail)normalsupdatedelay=0;
		}

		if(target>=1){
			if(targetanimation==rollanim&&targetframe==3&&onfire){
				onfire=0;
				emit_sound_at(fireendsound, coords);
				pause_sound(stream_firesound);
				deathbleeding=0;
			}

			if(targetanimation==rabbittacklinganim&&targetframe==1){
				//if(victim->aitype==attacktypecutoff&&Random()%2==0&&victim->stunned<=0&&animation[victim->targetanimation].attack==neutral&&victim->id!=0)Reverse();
				if(victim->aitype==attacktypecutoff&&victim->stunned<=0&&victim->surprised<=0&&victim->id!=0)Reverse();
				if(targetanimation==rabbittacklinganim&&targetframe==1&&!victim->isCrouch()&&victim->targetanimation!=backhandspringanim){
					if(normaldotproduct(victim->facing,facing)>0)victim->targetanimation=rabbittackledbackanim;
					else victim->targetanimation=rabbittackledfrontanim;
					victim->targetframe=2;
					victim->target=0;
					victim->rotation=rotation;
					victim->targetrotation=rotation;
					if(victim->aitype==gethelptype)victim->DoDamage(victim->damagetolerance-victim->damage);
					//victim->DoDamage(30);
					if(creature==wolftype){
						DoBloodBig(0,255);
						emit_sound_at(clawslicesound, victim->coords);
						victim->spurt=1;
						victim->DoBloodBig(1/victim->armorhead,210);
					}
					award_bonus(id, TackleBonus,
						    victim->aitype == gethelptype ? 50 : 0);
				}
			}

			if(!drawtogglekeydown&&drawkeydown&&(weaponactive==-1||num_weapons==1)&&(animation[targetanimation].label[targetframe]||(targetanimation!=currentanimation&&currentanimation==rollanim))&&num_weapons>0&&creature!=wolftype){
				if(weapons.type[weaponids[0]]==knife){
					if(weaponactive==-1)weaponactive=0;
					else if(weaponactive==0)weaponactive=-1;

					if(weaponactive==-1){
						emit_sound_at(knifesheathesound, coords);
					}
					if(weaponactive!=-1){
						emit_sound_at(knifedrawsound, coords, 128);
					}
				}
				drawtogglekeydown=1;
			}
			//Footstep sounds
			if(tutoriallevel!=1||id==0)
				if((animation[targetanimation].label[targetframe]&&(animation[targetanimation].label[targetframe]<5||animation[targetanimation].label[targetframe]==8))/*||(targetanimation==rollanim&&targetframe==animation[rollanim].numframes-1)*/){
					int whichsound;
					if(onterrain){
						if(terrain.getOpacity(coords.x,coords.z)<.2){
							if(animation[targetanimation].label[targetframe]==1)whichsound=footstepsound;
							else whichsound=footstepsound2;
							if(animation[targetanimation].label[targetframe]==1)FootLand(0,1);
							if(animation[targetanimation].label[targetframe]==2)FootLand(1,1);
							if(animation[targetanimation].label[targetframe]==3&&isRun()){
								FootLand(1,1);
								FootLand(0,1);
							}

						}
						if(terrain.getOpacity(coords.x,coords.z)>=.2){
							if(animation[targetanimation].label[targetframe]==1)whichsound=footstepsound3;
							else whichsound=footstepsound4;
						}
					}
					if(!onterrain){
						if(animation[targetanimation].label[targetframe]==1)whichsound=footstepsound3;
						else whichsound=footstepsound4;
					}
					if(animation[targetanimation].label[targetframe]==4&&(weaponactive==-1||(targetanimation!=knifeslashstartanim&&targetanimation!=knifethrowanim&&targetanimation!=crouchstabanim&&targetanimation!=swordgroundstabanim&&targetanimation!=knifefollowanim))){
						if(animation[targetanimation].attack!=neutral){
							i=abs(Random()%3);
							if(i==0)whichsound=lowwhooshsound;
							if(i==1)whichsound=midwhooshsound;
							if(i==2)whichsound=highwhooshsound;
						}
						if(animation[targetanimation].attack==neutral)whichsound=movewhooshsound;
					}
					else if(animation[targetanimation].label[targetframe]==4)whichsound=knifeswishsound;
					if(animation[targetanimation].label[targetframe]==8&&tutoriallevel!=1)whichsound=landsound2;

					emit_sound_at(whichsound, coords, 256.);

					if(id==0)
						if(whichsound==footstepsound||whichsound==footstepsound2||whichsound==footstepsound3||whichsound==footstepsound4){
							envsound[numenvsounds]=coords;
							if(targetanimation==wolfrunninganim||targetanimation==rabbitrunninganim)envsoundvol[numenvsounds]=15;
							else envsoundvol[numenvsounds]=6;
							envsoundlife[numenvsounds]=.4;
							numenvsounds++;
						}

						if(animation[targetanimation].label[targetframe]==3){
							whichsound--;
							emit_sound_at(whichsound, coords, 128.);
						}
				}

				//Combat sounds
				if(tutoriallevel!=1||id==0)
					if(speechdelay<=0)
						if(targetanimation!=crouchstabanim&&targetanimation!=swordgroundstabanim&&targetanimation!=staffgroundsmashanim)
							if((animation[targetanimation].label[targetframe]&&(animation[targetanimation].label[targetframe]<5||animation[targetanimation].label[targetframe]==8))/*||(targetanimation==rollanim&&targetframe==animation[rollanim].numframes-1)*/){
								int whichsound=-1;
								if(animation[targetanimation].label[targetframe]==4&&aitype!=playercontrolled){
									if(animation[targetanimation].attack!=neutral){
										i=abs(Random()%4);
										if(creature==rabbittype){
											if(i==0)whichsound=rabbitattacksound;
											if(i==1)whichsound=rabbitattack2sound;
											if(i==2)whichsound=rabbitattack3sound;
											if(i==3)whichsound=rabbitattack4sound;
										}
										if(creature==wolftype){
											if(i==0)whichsound=barksound;
											if(i==1)whichsound=bark2sound;
											if(i==2)whichsound=bark3sound;
											if(i==3)whichsound=barkgrowlsound;
										}
										speechdelay=.3;
									}
									//if(animation[targetanimation].attack==neutral)whichsound=movewhooshsound;
								}
								//else if(animation[targetanimation].label[targetframe]==4)whichsound=knifeswishsound;
								//if(animation[targetanimation].label[targetframe]==8)whichsound=landsound2;

								if(whichsound!=-1){
									emit_sound_at(whichsound, coords);
								}
							}



							if((!wasLanding()&&!wasLandhard())&&currentanimation!=getIdle()&&(isLanding()||isLandhard())){
								FootLand(0,1);
								FootLand(1,1);
							}

							transspeed=0;
							currentoffset=targetoffset;
							targetframe=currentframe;
							currentanimation=targetanimation;
							targetframe++;

							if(targetanimation==removeknifeanim&&animation[targetanimation].label[currentframe]==5){
								for(i=0;i<weapons.numweapons;i++){
									if(/*weapons.velocity[i].x==0&&weapons.velocity[i].y==0&&weapons.velocity[i].z==0&&*/weapons.owner[i]==-1)
										if(findDistancefastflat(&coords,&weapons.position[i])<4&&weaponactive==-1){
											if(findDistancefast(&coords,&weapons.position[i])>=1){
												if(weapons.type[i]!=staff){
													emit_sound_at(knifedrawsound, coords, 128.);
												}

												weaponactive=0;
												weapons.owner[i]=id;
												if(num_weapons>0){
													weaponids[num_weapons]=weaponids[0];
												}
												num_weapons++;
												weaponids[0]=i;
											}
										}
								}
							}

							static bool willwork;
							if(targetanimation==crouchremoveknifeanim&&animation[targetanimation].label[currentframe]==5){
								for(i=0;i<weapons.numweapons;i++){
									bool willwork=1;
									if(weapons.owner[i]!=-1)
										if(player[weapons.owner[i]].weaponstuck!=-1)
											if(player[weapons.owner[i]].weaponids[player[weapons.owner[i]].weaponstuck]==i)
												if(player[weapons.owner[i]].num_weapons>1)willwork=0;
									if((/*weapons.velocity[i].x==0&&weapons.velocity[i].y==0&&weapons.velocity[i].z==0&&*/weapons.owner[i]==-1)||(hasvictim&&weapons.owner[i]==victim->id&&victim->skeleton.free))
										if(willwork&&findDistancefastflat(&coords,&weapons.position[i])<3&&weaponactive==-1){
											if(findDistancefast(&coords,&weapons.position[i])<1||hasvictim){
												bool fleshstuck=0;
												if(weapons.owner[i]!=-1)
													if(victim->weaponstuck!=-1){
														if(victim->weaponids[victim->weaponstuck]==i){
															fleshstuck=1;
														}
													}
													if(!fleshstuck){
														if(weapons.type[i]!=staff){
															emit_sound_at(knifedrawsound, coords, 128.);
														}
													}
													if(fleshstuck){
														emit_sound_at(fleshstabremovesound, coords, 128.);
													}
													weaponactive=0;
													if(weapons.owner[i]!=-1){

														victim=&player[weapons.owner[i]];
														if(victim->num_weapons==1)victim->num_weapons=0;
														else victim->num_weapons=1;

														//victim->weaponactive=-1;
														victim->skeleton.longdead=0;
														victim->skeleton.free=1;
														victim->skeleton.broken=0;

														for(int j=0;j<victim->skeleton.num_joints;j++){
															victim->skeleton.joints[j].velchange=0;
															victim->skeleton.joints[j].locked=0;
														}

														XYZ relative;
														relative=0;
														relative.y=10;
														Normalise(&relative);
														XYZ footvel,footpoint;
														footvel=0;
														footpoint=weapons.position[i];
														if(victim->weaponstuck!=-1){
															if(victim->weaponids[victim->weaponstuck]==i){
																if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .8, .3);
																weapons.bloody[i]=2;
																weapons.blooddrip[i]=5;
																victim->weaponstuck=-1;
															}
														}
														if(victim->num_weapons>0){
															if(victim->weaponstuck!=0&&victim->weaponstuck!=-1)victim->weaponstuck=0;
															if(victim->weaponids[0]==i)
																victim->weaponids[0]=victim->weaponids[victim->num_weapons];
														}

														victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*6;
														victim->skeleton.joints[victim->skeleton.jointlabels[neck]].velocity+=relative*6;
														victim->skeleton.joints[victim->skeleton.jointlabels[rightshoulder]].velocity+=relative*6;
														victim->skeleton.joints[victim->skeleton.jointlabels[leftshoulder]].velocity+=relative*6;
													}
													weapons.owner[i]=id;
													if(num_weapons>0){
														weaponids[num_weapons]=weaponids[0];
													}
													num_weapons++;
													weaponids[0]=i;
											}
										}
								}
							}

							if(currentanimation==drawleftanim&&animation[targetanimation].label[currentframe]==5){
								if(weaponactive==-1)weaponactive=0;
								else if(weaponactive==0){
									weaponactive=-1;
									if(num_weapons==2){
										int buffer;
										buffer=weaponids[0];
										weaponids[0]=weaponids[1];
										weaponids[1]=buffer;
									}
								}
								if(weaponactive==-1){
									emit_sound_at(knifesheathesound, coords, 128.);
								}
								if(weaponactive!=-1){
									emit_sound_at(knifedrawsound, coords, 128.);
								}
							}


							if((currentanimation==walljumprightkickanim&&targetanimation==walljumprightkickanim)||(currentanimation==walljumpleftkickanim&&targetanimation==walljumpleftkickanim)){
								XYZ rotatetarget=DoRotation(skeleton.forward,0,rotation,0);
								Normalise(&rotatetarget);
								targetrotation=-asin(0-rotatetarget.x);
								targetrotation*=360/6.28;
								if(rotatetarget.z<0)targetrotation=180-targetrotation;

								if(targetanimation==walljumprightkickanim)targetrotation+=40;
								if(targetanimation==walljumpleftkickanim)targetrotation-=40;
							}

							bool dojumpattack;
							dojumpattack=0;
							if((targetanimation==rabbitrunninganim||targetanimation==wolfrunninganim)&&targetframe==3&&(jumpkeydown||attackkeydown||id!=0))dojumpattack=1;
							if(hasvictim)
			if(findDistancefast(&victim->coords,&/*player[i].*/coords)<5&&victim->aitype==gethelptype&&(attackkeydown)&&!victim->skeleton.free&&victim->isRun()&&victim->runninghowlong>=1)dojumpattack=1;							if(!hostile)dojumpattack=0;
							if(dojumpattack){
								if((targetanimation==rabbitrunninganim||targetanimation==wolfrunninganim)&&id==0){
									targetanimation=rabbittackleanim;
									targetframe=0;
									emit_sound_at(jumpsound, coords);
								}

								float closestdist;
								closestdist=0;
								int closestid;
								closestid=-1;
								XYZ targetloc;
								targetloc=velocity;
								Normalise(&targetloc);
								targetloc+=coords;
								for(i=0;i<numplayers;i++){
									if(i!=id)
										if(findDistancefast(&targetloc,&player[i].coords)<closestdist||closestdist==0){
											closestdist=findDistancefast(&targetloc,&player[i].coords);
											closestid=i;
										}
								}
								if(closestid!=-1)
									if(closestdist<5&&!player[closestid].dead&&animation[player[closestid].targetanimation].height!=lowheight&&player[closestid].targetanimation!=backhandspringanim){
										hasvictim=1;
										victim=&player[closestid];
										coords=victim->coords;
										currentanimation=rabbittacklinganim;
										targetanimation=rabbittacklinganim;
										currentframe=0;
										targetframe=1;
										XYZ rotatetarget;
										if(coords.z!=victim->coords.z||coords.x!=victim->coords.x){
											rotatetarget=coords-victim->coords;
											Normalise(&rotatetarget);
											targetrotation=-asin(0-rotatetarget.x);
											targetrotation*=360/6.28;
											if(rotatetarget.z<0)targetrotation=180-targetrotation;
										}
										if(targetanimation!=rabbitrunninganim){
											emit_sound_at(jumpsound, coords, 128.);
										}
									}
							}

							//Move impacts
							float damagemult=1*power;
							if(creature==wolftype)damagemult=2.5*power;
							if(hasvictim){damagemult/=victim->damagetolerance/200;}
							//if(onfire)damagemult=3;
							if((animation[targetanimation].attack==normalattack||targetanimation==walljumprightkickanim||targetanimation==walljumpleftkickanim)&&(!feint)&&(victim->skeleton.free!=2||targetanimation==killanim||targetanimation==dropkickanim||targetanimation==crouchstabanim||targetanimation==swordgroundstabanim||targetanimation==staffgroundsmashanim)){
								if(targetanimation==spinkickanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3&&3&&animation[victim->targetanimation].height!=lowheight){
										escapednum=0;
										if(id==0)camerashake+=.4;
										if(Random()%2||creature==wolftype){
											victim->spurt=1;
											DoBlood(.2,250);
											if(creature==wolftype)DoBloodBig(0,250);
										}
										if(tutoriallevel!=1){
											emit_sound_at(heavyimpactsound, victim->coords, 128.);
										}
										if(creature==wolftype){
											emit_sound_at(clawslicesound, victim->coords, 128.);
											victim->spurt=1;
											victim->DoBloodBig(2/victim->armorhead,175);
										}
										victim->RagDoll(0);
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										relative=DoRotation(relative,0,-90,0);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*40;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*200;
										//FootLand(1,2);
										victim->Puff(head);
										victim->DoDamage(damagemult*100/victim->protectionhead);

										SolidHitBonus(id);
									}
								}

								if(targetanimation==wolfslapanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3&&3&&animation[victim->targetanimation].height!=lowheight){
										escapednum=0;
										if(id==0)camerashake+=.4;
										if(Random()%2||creature==wolftype){
											victim->spurt=1;
											if(creature==wolftype)DoBloodBig(0,235);
										}
										emit_sound_at(whooshhitsound, victim->coords);
										if(creature==wolftype){
											emit_sound_at(clawslicesound, victim->coords, 128.);
											victim->spurt=1;
											victim->DoBloodBig(2,175);
										}
										victim->RagDoll(0);
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										relative.y-=1;
										Normalise(&relative);
										relative=DoRotation(relative,0,90,0);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*20;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*100;
										//FootLand(1,2);
										victim->Puff(head);
										victim->DoDamage(damagemult*50/victim->protectionhead);
									}
								}

								if(targetanimation==walljumprightkickanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3&&animation[victim->targetanimation].height!=lowheight){
										escapednum=0;
										if(id==0)camerashake+=.4;
										victim->spurt=1;
										DoBlood(.2,250);
										if(tutoriallevel!=1){
											emit_sound_at(heavyimpactsound, victim->coords, 160.);
										}
										if(creature==wolftype){
											emit_sound_at(clawslicesound, victim->coords, 128.);
											victim->spurt=1;
											victim->DoBloodBig(2/victim->armorhead,175);
										}
										victim->RagDoll(0);
										XYZ relative;
										relative=facing;
										relative.y=0;
										Normalise(&relative);
										relative=DoRotation(relative,0,-90,0);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*40;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*200;
										//FootLand(1,2);
										victim->Puff(head);
										victim->DoDamage(damagemult*150/victim->protectionhead);

										if(victim->damage>victim->damagetolerance)
										  award_bonus(id, style);
										else
										  SolidHitBonus(id);
									}
								}

								if(targetanimation==walljumpleftkickanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3&&animation[victim->targetanimation].height!=lowheight){
										escapednum=0;
										if(id==0)camerashake+=.4;
										victim->spurt=1;
										DoBlood(.2,250);
										if(tutoriallevel!=1){
											emit_sound_at(heavyimpactsound, victim->coords, 160.);
										}
										if(creature==wolftype){
											emit_sound_at(clawslicesound, victim->coords, 128.);
											victim->spurt=1;
											victim->DoBloodBig(2/victim->armorhead,175);
										}
										victim->RagDoll(0);
										XYZ relative;
										relative=facing;
										relative.y=0;
										Normalise(&relative);
										relative=DoRotation(relative,0,90,0);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*40;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*200;
										//FootLand(1,2);
										victim->Puff(head);
										victim->DoDamage(damagemult*150/victim->protectionhead);

										if(victim->damage>victim->damagetolerance)
										  award_bonus(id, style);
										else
										  SolidHitBonus(id);
									}
								}

								if(targetanimation==blockhighleftstrikeanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3&&animation[victim->targetanimation].height!=lowheight){
										escapednum=0;
										if(id==0)camerashake+=.4;
										if(Random()%2){
											victim->spurt=1;
											DoBlood(.2,235);
										}
										emit_sound_at(whooshhitsound, victim->coords);
										victim->RagDoll(0);
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*30;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*100;
										//FootLand(1,2);
										victim->Puff(head);
										victim->DoDamage(damagemult*50/victim->protectionhead);
									}
								}

								if(targetanimation==killanim&&animation[targetanimation].label[currentframe]==8){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3&&victim->dead){
										escapednum=0;
										if(id==0)camerashake+=.2;
										emit_sound_at(whooshhitsound, victim->coords, 128.);

										victim->skeleton.longdead=0;
										victim->skeleton.free=1;
										victim->skeleton.broken=0;
										victim->skeleton.spinny=1;

										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velchange=0;
											victim->skeleton.joints[i].delay=0;
											victim->skeleton.joints[i].locked=0;
											//victim->skeleton.joints[i].velocity=0;
										}

										XYZ relative;
										relative=0;
										relative.y=1;
										Normalise(&relative);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity.y=relative.y*10;
											victim->skeleton.joints[i].position.y+=relative.y*.3;
											victim->skeleton.joints[i].oldposition.y+=relative.y*.3;
											victim->skeleton.joints[i].realoldposition.y+=relative.y*.3;
										}
										victim->Puff(abdomen);
										victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity.y=relative.y*400;
									}
								}

								if(targetanimation==killanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*9&&victim->dead){
										escapednum=0;
										if(id==0)camerashake+=.4;
										if(tutoriallevel!=1){
											emit_sound_at(heavyimpactsound, coords, 128.);
										}
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*90;
										}
										victim->Puff(abdomen);
										if(victim->dead!=2&&victim->permanentdamage>victim->damagetolerance-250&&autoslomo){
											slomo=1;
											slomodelay=.2;
										}
										victim->DoDamage(damagemult*500/victim->protectionhigh);
										victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*300;
									}
								}

								if(targetanimation==dropkickanim&&animation[targetanimation].label[currentframe]==7){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*9&&victim->skeleton.free){
										escapednum=0;
										if(id==0)camerashake+=.4;
										if(tutoriallevel!=1){
											emit_sound_at(thudsound, coords);
										}

										victim->skeleton.longdead=0;
										victim->skeleton.free=1;
										victim->skeleton.broken=0;
										victim->skeleton.spinny=1;

										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velchange=0;
											//victim->skeleton.joints[i].delay=0;
											victim->skeleton.joints[i].locked=0;
										}
										XYZ relative;
										relative=victim->coords-coords;
										Normalise(&relative);
										relative.y+=.3;
										Normalise(&relative);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*20;
										}
										if(!victim->dead)
										  SolidHitBonus(id);

										victim->Puff(abdomen);
										victim->DoDamage(damagemult*20/victim->protectionhigh);
										victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
										staggerdelay=.5;
										if(!victim->dead)staggerdelay=1.2;


									}
								}

								if((targetanimation==crouchstabanim||targetanimation==swordgroundstabanim)&&animation[targetanimation].label[currentframe]==5){
									//if(id==0)camerashake+=.4;

									if(hasvictim)
										if(!victim->skeleton.free)hasvictim=0;

									if(!hasvictim){
										terrain.MakeDecal(blooddecalfast,(weapons.tippoint[weaponids[weaponactive]]*.8+weapons.position[weaponids[weaponactive]]*.2),.08,.6,Random()%360);
										emit_sound_at(knifesheathesound, coords, 128.);
									}

									if(victim&&hasvictim){
										if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3){

											XYZ where,startpoint,endpoint,movepoint,colpoint;
											float rotationpoint;
											int whichtri;
											if(weapons.type[weaponids[weaponactive]]==knife){
												where=(weapons.tippoint[weaponids[weaponactive]]*.6+weapons.position[weaponids[weaponactive]]*.4);
												where-=victim->coords;
												if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
												//where=scale;
												startpoint=where;
												startpoint.y+=100;
												endpoint=where;
												endpoint.y-=100;
											}
											if(weapons.type[weaponids[weaponactive]]==sword){
												where=weapons.position[weaponids[weaponactive]];
												where-=victim->coords;
												if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
												startpoint=where;
												where=weapons.tippoint[weaponids[weaponactive]];
												where-=victim->coords;
												if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
												endpoint=where;
											}
											if(weapons.type[weaponids[weaponactive]]==staff){
												where=weapons.position[weaponids[weaponactive]];
												where-=victim->coords;
												if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
												startpoint=where;
												where=weapons.tippoint[weaponids[weaponactive]];
												where-=victim->coords;
												if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
												endpoint=where;
											}
											movepoint=0;
											rotationpoint=0;
											whichtri=victim->skeleton.drawmodel.LineCheck(&startpoint,&endpoint, &colpoint, &movepoint, &rotationpoint);

											if(whichtri!=-1){
												if(victim->dead!=2){
													victim->DoDamage(abs((victim->damagetolerance-victim->permanentdamage)*2));
													if (!victim->dead)
													  award_bonus(id, FinishedBonus);
												}
												if(bloodtoggle)weapons.bloody[weaponids[weaponactive]]=2;

												victim->skeleton.longdead=0;
												victim->skeleton.free=1;
												victim->skeleton.broken=0;

												for(i=0;i<victim->skeleton.num_joints;i++){
													victim->skeleton.joints[i].velchange=0;
													victim->skeleton.joints[i].locked=0;
													//victim->skeleton.joints[i].velocity=0;
												}
												emit_sound_at(fleshstabsound, coords, 128);

											}
											if(whichtri!=-1||weapons.bloody[weaponids[weaponactive]]){
												weapons.blooddrip[weaponids[weaponactive]]+=5;
												weapons.blooddripdelay[weaponids[weaponactive]]=0;
											}
											if(whichtri==-1){
												hasvictim=0;
												emit_sound_at(knifesheathesound, coords, 128.);
											}
										}
									}
								}

								if((targetanimation==crouchstabanim||targetanimation==swordgroundstabanim)&&animation[targetanimation].label[currentframe]==6){
									if(!hasvictim){
										emit_sound_at(knifedrawsound, coords, 128);
									}

									if(victim&&hasvictim){
										XYZ footvel,footpoint;

										emit_sound_at(fleshstabremovesound, coords, 128.);

										footvel=0;
										footpoint=(weapons.tippoint[weaponids[weaponactive]]*.8+weapons.position[weaponids[weaponactive]]*.2);

										if(weapons.type[weaponids[weaponactive]]==sword){
											XYZ where,startpoint,endpoint,movepoint;
											float rotationpoint;
											int whichtri;

											where=weapons.position[weaponids[weaponactive]];
											where-=victim->coords;
											if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
											startpoint=where;
											where=weapons.tippoint[weaponids[weaponactive]];
											where-=victim->coords;
											if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
											endpoint=where;

											movepoint=0;
											rotationpoint=0;
											whichtri=victim->skeleton.drawmodel.LineCheck(&startpoint,&endpoint, &footpoint, &movepoint, &rotationpoint);
											footpoint+=victim->coords;

											if(whichtri==-1){
												footpoint=(weapons.tippoint[weaponids[weaponactive]]*.8+weapons.position[weaponids[weaponactive]]*.2);
											}
										}
										if(weapons.type[weaponids[weaponactive]]==staff){
											XYZ where,startpoint,endpoint,movepoint;
											float rotationpoint;
											int whichtri;

											where=weapons.position[weaponids[weaponactive]];
											where-=victim->coords;
											if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
											startpoint=where;
											where=weapons.tippoint[weaponids[weaponactive]];
											where-=victim->coords;
											if(!victim->skeleton.free)where=DoRotation(where,0,-victim->rotation,0);
											endpoint=where;

											movepoint=0;
											rotationpoint=0;
											whichtri=victim->skeleton.drawmodel.LineCheck(&startpoint,&endpoint, &footpoint, &movepoint, &rotationpoint);
											footpoint+=victim->coords;

											if(whichtri==-1){
												footpoint=(weapons.tippoint[weaponids[weaponactive]]*.8+weapons.position[weaponids[weaponactive]]*.2);
											}
										}
										hasvictim=victim->DoBloodBigWhere(2,220,footpoint);
										if(hasvictim){
											if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3){
												victim->skeleton.longdead=0;
												victim->skeleton.free=1;
												victim->skeleton.broken=0;

												for(i=0;i<victim->skeleton.num_joints;i++){
													victim->skeleton.joints[i].velchange=0;
													victim->skeleton.joints[i].locked=0;
													//victim->skeleton.joints[i].velocity=0;
												}

												XYZ relative;
												relative=0;
												relative.y=10;
												Normalise(&relative);
												//victim->Puff(abdomen);
												if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .8, .3);

												if(victim->bloodloss<victim->damagetolerance){
													victim->bloodloss+=1000;
													victim->bled=0;
												}

												victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*20;
											}
										}
									}
									if(!hasvictim&&onterrain){
										weapons.bloody[weaponids[weaponactive]]=0;
										weapons.blooddrip[weaponids[weaponactive]]=0;
									}
								}

								if(targetanimation==upunchanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3){
										escapednum=0;
										if(id==0)camerashake+=.4;
										if(Random()%2){
											victim->spurt=1;
											DoBlood(.2,235);
										}
										if(tutoriallevel!=1){
										  emit_sound_at(heavyimpactsound, victim->coords, 128);
										}

										victim->RagDoll(0);
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity=relative*30;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*150;

										victim->targetframe=0;
										victim->targetanimation=staggerbackhardanim;
										victim->targetrotation=targetrotation+180;
										victim->target=0;
										victim->stunned=1;

										victim->Puff(head);
										victim->Puff(abdomen);
										victim->DoDamage(damagemult*60/victim->protectionhigh);

										SolidHitBonus(id);
									}
								}


								if(targetanimation==winduppunchanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*2){
										escapednum=0;
										if(id==0)camerashake+=.4;
										if(victim->damage<=victim->damagetolerance-60&&normaldotproduct(victim->facing,victim->coords-coords)<(scale*5)*(scale*5)*0&&animation[victim->targetanimation].height!=lowheight){
											if(tutoriallevel!=1){
												emit_sound_at(thudsound, victim->coords);
											}
										}
										else if(victim->damage<=victim->damagetolerance-60&&normaldotproduct(victim->facing,victim->coords-coords)<(scale*5)*(scale*5)*0&&animation[victim->targetanimation].height==lowheight){
											if(tutoriallevel!=1){
												emit_sound_at(whooshhitsound, victim->coords);
											}
										}
										else {
											if(tutoriallevel!=1){
												emit_sound_at(heavyimpactsound, victim->coords);
											}
										}

										if(victim->damage>victim->damagetolerance-60||normaldotproduct(victim->facing,victim->coords-coords)>0||animation[victim->targetanimation].height==lowheight)
											victim->RagDoll(0);
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										relative.y=.3;
										Normalise(&relative);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity=relative*5;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*400;

										victim->targetframe=0;
										victim->targetanimation=staggerbackhardanim;
										victim->targetrotation=targetrotation+180;
										victim->target=0;
										victim->stunned=1;

										victim->Puff(abdomen);
										victim->DoDamage(damagemult*60/victim->protectionhigh);

										SolidHitBonus(id);
									}
								}

								if(targetanimation==blockhighleftanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*4){
										if(victim->id==0)camerashake+=.4;
										emit_sound_at(landsound2, victim->coords);

										Puff(righthand);
									}
								}

								if(targetanimation==swordslashparryanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*4){
										if(victim->id==0)camerashake+=.4;

										if(weaponactive!=-1){
											if(weapons.type[victim->weaponids[0]]==staff||weapons.type[weaponids[0]]==staff){
												if(weapons.type[victim->weaponids[0]]==staff)weapons.damage[victim->weaponids[0]]+=.2+float(abs(Random()%100)-50)/250;
												if(weapons.type[weaponids[0]]==staff)weapons.damage[weaponids[0]]+=.2+float(abs(Random()%100)-50)/250;

												emit_sound_at(swordstaffsound, victim->coords);
											}
											else{
												emit_sound_at(metalhitsound, victim->coords);
											}
										}

										//Puff(righthand);
									}
								}

								if(targetanimation==knifethrowanim&&animation[targetanimation].label[currentframe]==5){
									if(weaponactive!=-1){
										escapednum=0;
										XYZ aim;
										weapons.owner[weaponids[0]]=-1;
										aim=victim->coords+DoRotation(victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].position,0,victim->rotation,0)*victim->scale+victim->velocity*findDistance(&victim->coords,&coords)/50-(coords+DoRotation(skeleton.joints[skeleton.jointlabels[righthand]].position,0,rotation,0)*scale);
										Normalise(&aim);
										/*if(victim->targetanimation==jumpupanim||victim->targetanimation==jumpdownanim){
										aim=DoRotation(aim,(float)abs(Random()%15)-7,(float)abs(Random()%15)-7,0);
										}*/
										weapons.velocity[weaponids[0]]=aim*50;
										weapons.tipvelocity[weaponids[0]]=aim*50;
										weapons.missed[weaponids[0]]=0;
										weapons.hitsomething[weaponids[0]]=0;
										weapons.freetime[weaponids[0]]=0;
										weapons.firstfree[weaponids[0]]=1;
										weapons.physics[weaponids[0]]=0;
										num_weapons--;
										if(num_weapons){
											weaponids[0]=weaponids[num_weapons];
										}
										weaponactive=-1;
									}
								}

								if(targetanimation==knifeslashstartanim&&animation[targetanimation].label[currentframe]==5){
									if(hasvictim)
										if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*4.5&&/*animation[victim->targetanimation].height!=lowheight&&*/victim->targetanimation!=dodgebackanim&&victim->targetanimation!=rollanim){
											escapednum=0;
											if(tutoriallevel!=1)victim->DoBloodBig(1.5/victim->armorhigh,225);

											award_bonus(id, Slicebonus);
											if(tutoriallevel!=1){
												emit_sound_at(knifeslicesound, victim->coords);
											}
											//victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
											if(animation[victim->targetanimation].attack&&(victim->aitype!=playercontrolled||victim->targetanimation==knifeslashstartanim)&&(victim->creature==rabbittype||victim->deathbleeding<=0)){
												if(victim->id != 0 || difficulty==2){
													victim->targetframe=0;
													victim->targetanimation=staggerbackhardanim;
													victim->targetrotation=targetrotation+180;
													victim->target=0;
												}
											}
											victim->lowreversaldelay=0;
											victim->highreversaldelay=0;
											if(aitype!=playercontrolled)weaponmissdelay=.6;

											if(tutoriallevel!=1)if(bloodtoggle&&!weapons.bloody[weaponids[weaponactive]])weapons.bloody[weaponids[weaponactive]]=1;
											if(tutoriallevel!=1)weapons.blooddrip[weaponids[weaponactive]]+=3;

											XYZ footvel,footpoint;
											footvel=0;
											if(skeleton.free){
												footpoint=(victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].position+victim->skeleton.joints[victim->skeleton.jointlabels[neck]].position)/2*victim->scale+victim->coords;
											}
											if(!skeleton.free){
												footpoint=DoRotation((victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].position+victim->skeleton.joints[victim->skeleton.jointlabels[neck]].position)/2,0,victim->rotation,0)*victim->scale+victim->coords;
											}
											if(tutoriallevel!=1){
												if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .6, .3);
												footvel=DoRotation(facing,0,90,0)*.8;
												//footvel.y-=.3;
												Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*7,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
												Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*3,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
												Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*5, 1,1,1, .2, 1);
												Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*2, 1,1,1, .2, 1);
											}
											if(tutoriallevel==1){
												Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,1,1, .6, .3);
											}
											victim->DoDamage(damagemult*0);
										}
								}
								if(targetanimation==swordslashanim&&animation[targetanimation].label[currentframe]==5&&victim->targetanimation!=rollanim){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*6.5&&victim->targetanimation!=dodgebackanim){
										if(victim->weaponactive==-1||normaldotproduct(victim->facing,victim->coords-coords)>0||(Random()%2==0)){
											award_bonus(id, Slashbonus);
											escapednum=0;
											if(tutoriallevel!=1){
												if(normaldotproduct(victim->facing,victim->coords-coords)<0)victim->DoBloodBig(2/victim->armorhigh,190);
												else victim->DoBloodBig(2/victim->armorhigh,185);
												victim->deathbleeding=1;
												emit_sound_at(swordslicesound, victim->coords);
											}
											//victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
											if(tutoriallevel!=1){
												victim->targetframe=0;
												victim->targetanimation=staggerbackhardanim;
												victim->targetrotation=targetrotation+180;
												victim->target=0;
											}

											if(tutoriallevel!=1){
												if(bloodtoggle&&!weapons.bloody[weaponids[weaponactive]])weapons.bloody[weaponids[weaponactive]]=1;
												weapons.blooddrip[weaponids[weaponactive]]+=3;

												float bloodlossamount;
												bloodlossamount=200+abs((float)(Random()%40))-20;
												victim->bloodloss+=bloodlossamount/victim->armorhigh;
												//victim->bloodloss+=100*(6.5-findDistancefast(&coords,&victim->coords));
												victim->DoDamage(damagemult*0);

												XYZ footvel,footpoint;
												footvel=0;
												if(skeleton.free){
													footpoint=(victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].position+victim->skeleton.joints[victim->skeleton.jointlabels[neck]].position)/2*victim->scale+victim->coords;
												}
												if(!skeleton.free){
													footpoint=DoRotation((victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].position+victim->skeleton.joints[victim->skeleton.jointlabels[neck]].position)/2,0,victim->rotation,0)*victim->scale+victim->coords;
												}
												if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .9, .3);
												footvel=DoRotation(facing,0,90,0)*.8;
												footvel.y-=.3;
												Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*7,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
												Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*3,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
												Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*5, 1,1,1, .3, 1);
												Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*2, 1,1,1, .3, 1);
											}
										}
										else {
											if(victim->weaponactive!=-1){
												if(weapons.type[victim->weaponids[0]]==staff||weapons.type[weaponids[0]]==staff){
													if(weapons.type[victim->weaponids[0]]==staff)weapons.damage[victim->weaponids[0]]+=.2+float(abs(Random()%100)-50)/250;
													if(weapons.type[weaponids[0]]==staff)weapons.damage[weaponids[0]]+=.2+float(abs(Random()%100)-50)/250;

													emit_sound_at(swordstaffsound, victim->coords);
												}
												else{
													emit_sound_at(metalhitsound, victim->coords);
												}
											}


											XYZ aim;
											victim->Puff(righthand);
											victim->target=0;
											victim->targetframe=0;
											victim->targetanimation=staggerbackhighanim;
											victim->targetrotation=targetrotation+180;
											victim->target=0;
											weapons.owner[victim->weaponids[0]]=-1;
											aim=DoRotation(facing,0,90,0)*21;
											aim.y+=7;
											weapons.velocity[victim->weaponids[0]]=aim*-.2;
											weapons.tipvelocity[victim->weaponids[0]]=aim;
											weapons.missed[victim->weaponids[0]]=1;
											weapons.hitsomething[weaponids[0]]=0;
											weapons.freetime[victim->weaponids[0]]=0;
											weapons.firstfree[victim->weaponids[0]]=1;
											weapons.physics[victim->weaponids[0]]=1;
											victim->num_weapons--;
											if(victim->num_weapons){
												victim->weaponids[0]=victim->weaponids[num_weapons];
												if(victim->weaponstuck==victim->num_weapons)victim->weaponstuck=0;
											}
											victim->weaponactive=-1;
											for(i=0;i<numplayers;i++){
												player[i].wentforweapon=0;
											}

										}
									}
								}

								if(targetanimation==staffhitanim&&animation[targetanimation].label[currentframe]==5&&victim->targetanimation!=rollanim){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*6.5&&victim->targetanimation!=dodgebackanim&&victim->targetanimation!=sweepanim){
										if(tutoriallevel!=1){
											weapons.damage[weaponids[0]]+=.4+float(abs(Random()%100)-50)/250;
											escapednum=0;
											if(id==0)camerashake+=.4;
											if(Random()%2||creature==wolftype){
												victim->spurt=1;
											}
											emit_sound_at(staffheadsound, victim->coords);
										}
										victim->RagDoll(0);
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										relative=DoRotation(relative,0,90,0);
										relative.y-=1;
										Normalise(&relative);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*60;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*230;
										victim->skeleton.joints[victim->skeleton.jointlabels[neck]].velocity+=relative*damagemult*230;
										//FootLand(1,2);
										victim->Puff(head);
										if(tutoriallevel!=1){
											victim->DoDamage(damagemult*120/victim->protectionhigh);

											award_bonus(id, solidhit, 30);
										}
									}
								}

								if(targetanimation==staffspinhitanim&&animation[targetanimation].label[currentframe]==5&&victim->targetanimation!=rollanim){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*6.5&&victim->targetanimation!=dodgebackanim&&victim->targetanimation!=sweepanim){
										if(tutoriallevel!=1){
											weapons.damage[weaponids[0]]+=.6+float(abs(Random()%100)-50)/250;
											escapednum=0;
											if(id==0)camerashake+=.4;
											if(Random()%2||creature==wolftype){
												victim->spurt=1;
											}
											emit_sound_at(staffheadsound, victim->coords);
										}
										victim->RagDoll(0);
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										relative=DoRotation(relative,0,-90,0);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity+=relative*damagemult*40;
										}
										victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*220;
										victim->skeleton.joints[victim->skeleton.jointlabels[neck]].velocity+=relative*damagemult*220;
										//FootLand(1,2);
										victim->Puff(head);
										if(tutoriallevel!=1){victim->DoDamage(damagemult*350/victim->protectionhead);

										award_bonus(id, solidhit, 60);
										}
									}
								}

								if(targetanimation==staffgroundsmashanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*6.5){
										escapednum=0;
										if(tutoriallevel!=1){
											if(!victim->dead)weapons.damage[weaponids[0]]+=.4+float(abs(Random()%100)-50)/500;
											if(id==0)camerashake+=.4;
											if(Random()%2||creature==wolftype){
												victim->spurt=1;
											}
											emit_sound_at(staffbodysound, victim->coords);
										}
										victim->skeleton.longdead=0;
										victim->skeleton.free=1;
										victim->skeleton.broken=0;

										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velchange=0;
											victim->skeleton.joints[i].locked=0;
											//victim->skeleton.joints[i].velocity=0;
										}

										victim->RagDoll(0);
										XYZ relative;
										relative=0;
										/*relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);
										relative=DoRotation(relative,0,90,0);*/
										relative.y=-1;
										Normalise(&relative);
										if(!victim->dead){
											for(i=0;i<victim->skeleton.num_joints;i++){
												victim->skeleton.joints[i].velocity=relative*damagemult*40;
											}
											//FootLand(1,2);
											victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*40;
										}
										if(victim->dead){
											for(i=0;i<victim->skeleton.num_joints;i++){
												victim->skeleton.joints[i].velocity=relative*damagemult*abs(Random()%20);
											}
											//FootLand(1,2);
											//victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*20;
										}
										victim->Puff(abdomen);
										if(tutoriallevel!=1){victim->DoDamage(damagemult*100/victim->protectionhigh);

										if(!victim->dead){
										  award_bonus(id, solidhit, 40);
										}
										}
									}
								}

								if(targetanimation==lowkickanim&&animation[targetanimation].label[currentframe]==5){
									if(findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3&&animation[victim->targetanimation].height!=highheight){
										escapednum=0;
										if(id==0)camerashake+=.4;
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);

										SolidHitBonus(id);

										if(animation[victim->targetanimation].height==lowheight){
											if(Random()%2){
												victim->spurt=1;
												DoBlood(.2,250);
											}
											victim->RagDoll(0);
											for(i=0;i<victim->skeleton.num_joints;i++){
												victim->skeleton.joints[i].velocity+=relative*damagemult*40;
											}
											victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*200;
											if(tutoriallevel!=1){
												emit_sound_at(heavyimpactsound, victim->coords, 128.);
											}
											victim->Puff(head);
											victim->DoDamage(damagemult*100/victim->protectionhead);
											if(victim->howactive==typesleeping)victim->DoDamage(damagemult*150/victim->protectionhead);
											if(creature==wolftype){
												emit_sound_at(clawslicesound, victim->coords, 128.);
												victim->spurt=1;
												victim->DoBloodBig(2/victim->armorhead,175);
											}
										}
										else{
											if(victim->damage>=victim->damagetolerance)victim->RagDoll(0);
											for(i=0;i<victim->skeleton.num_joints;i++){
												victim->skeleton.joints[i].velocity+=relative*damagemult*10;
											}
											victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
											victim->targetframe=0;
											victim->targetanimation=staggerbackhighanim;
											victim->targetrotation=targetrotation+180;
											victim->target=0;
											if(tutoriallevel!=1){
												emit_sound_at(landsound2, victim->coords, 128.);
											}
											victim->Puff(abdomen);
											victim->DoDamage(damagemult*30/victim->protectionhigh);
											if(creature==wolftype){
												emit_sound_at(clawslicesound, victim->coords, 128.);
												victim->spurt=1;
												victim->DoBloodBig(2/victim->armorhigh,170);
											}
										}

									}
								}

								if(targetanimation==sweepanim&&animation[targetanimation].label[currentframe]==5){
									if(victim->targetanimation!=jumpupanim&&findDistancefast(&coords,&victim->coords)<(scale*5)*(scale*5)*3&&victim!=this){
										escapednum=0;
										if(id==0)camerashake+=.2;
										if(tutoriallevel!=1){
											emit_sound_at(landsound2, victim->coords, 128.);
										}
										XYZ relative;
										relative=victim->coords-coords;
										relative.y=0;
										Normalise(&relative);

										if(animation[victim->targetanimation].height==middleheight||animation[victim->currentanimation].height==middleheight||victim->damage>=victim->damagetolerance-40){
											victim->RagDoll(0);

											for(i=0;i<victim->skeleton.num_joints;i++){
												victim->skeleton.joints[i].velocity+=relative*damagemult*15;
											}
											relative=DoRotation(relative,0,-90,0);
											relative.y+=.1;
											for(i=0;i<victim->skeleton.num_joints;i++){
												if(victim->skeleton.joints[i].label==leftfoot||victim->skeleton.joints[i].label==rightfoot||victim->skeleton.joints[i].label==leftankle||victim->skeleton.joints[i].label==rightankle)
													victim->skeleton.joints[i].velocity=relative*80;
											}
											victim->Puff(rightankle);
											victim->Puff(leftankle);
											victim->DoDamage(damagemult*40/victim->protectionlow);
										}
										else{
											if(victim->damage>=victim->damagetolerance)victim->RagDoll(0);
											for(i=0;i<victim->skeleton.num_joints;i++){
												victim->skeleton.joints[i].velocity+=relative*damagemult*10;
											}
											relative=DoRotation(relative,0,-90,0);
											for(i=0;i<victim->skeleton.num_joints;i++){
												if(victim->skeleton.joints[i].label==leftfoot||victim->skeleton.joints[i].label==rightfoot||victim->skeleton.joints[i].label==leftankle||victim->skeleton.joints[i].label==rightankle)
													victim->skeleton.joints[i].velocity+=relative*damagemult*80;
											}
											victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
											victim->targetframe=0;
											victim->targetanimation=staggerbackhighanim;
											victim->targetrotation=targetrotation+180;
											victim->target=0;
											if(tutoriallevel!=1){
												emit_sound_at(landsound2, victim->coords, 128.);
											}
											victim->Puff(abdomen);
											victim->DoDamage(damagemult*30/victim->protectionlow);
										}

										SolidHitBonus(id);

									}
								}
							}
							if(animation[targetanimation].attack==reversal&&(!victim->feint||(victim->lastattack==victim->lastattack2&&victim->lastattack2==victim->lastattack3&&Random()%2)||targetanimation==knifefollowanim)){
								if(targetanimation==spinkickreversalanim&&animation[targetanimation].label[currentframe]==7){
									escapednum=0;
									if(id==0)camerashake+=.4;
									if(Random()%2){
										victim->spurt=1;
										DoBlood(.2,230);
									}
									if(tutoriallevel!=1){
										emit_sound_at(heavyimpactsound, victim->coords, 128.);
									}
									if(creature==wolftype){
										emit_sound_at(clawslicesound, victim->coords, 128);
										victim->spurt=1;
										victim->DoBloodBig(2/victim->armorhigh,170);
									}
									victim->RagDoll(0);
									XYZ relative;
									relative=victim->coords-oldcoords;
									relative.y=0;
									Normalise(&relative);
									//relative=DoRotation(relative,0,-90,0);
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*40;
									}
									victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
									//FootLand(1,2);
									victim->Puff(abdomen);
									victim->DoDamage(damagemult*150/victim->protectionhigh);

									award_bonus(id, Reversal);
								}

								if((targetanimation==swordslashreversalanim||targetanimation==knifeslashreversalanim||targetanimation==staffhitreversalanim||targetanimation==staffspinhitreversalanim)&&animation[targetanimation].label[currentframe]==5){
									if(victim->weaponactive!=-1&&victim->num_weapons>0){
										if(weapons.owner[victim->weaponids[victim->weaponactive]]==victim->id){
											weapons.owner[victim->weaponids[victim->weaponactive]]=id;
											weaponactive=0;
											if(num_weapons>0){
												weaponids[num_weapons]=weaponids[victim->weaponactive];
											}
											num_weapons++;
											weaponids[0]=victim->weaponids[victim->weaponactive];
											victim->num_weapons--;
											if(victim->num_weapons>0){
												victim->weaponids[victim->weaponactive]=victim->weaponids[victim->num_weapons];
												//if(victim->weaponstuck==victim->num_weapons)victim->weaponstuck=0;
											}
											victim->weaponactive=-1;
										}
									}
								}

								if(targetanimation==staffhitreversalanim&&animation[targetanimation].label[currentframe]==5){
									escapednum=0;
									if(id==0)camerashake+=.4;
									if(Random()%2){
										victim->spurt=1;
										DoBlood(.2,230);
									}
									emit_sound_at(whooshhitsound, victim->coords, 128.);
									victim->RagDoll(0);
									XYZ relative;
									relative=victim->coords-oldcoords;
									relative.y=0;
									Normalise(&relative);
									//relative=DoRotation(relative,0,-90,0);
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*30;
									}
									victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
									//FootLand(1,2);
									victim->Puff(head);
									victim->DoDamage(damagemult*70/victim->protectionhigh);
								}

								if(targetanimation==staffspinhitreversalanim&&animation[targetanimation].label[currentframe]==7){
									escapednum=0;
									if(id==0)camerashake+=.4;
									if(Random()%2){
										victim->spurt=1;
										DoBlood(.2,230);
									}

									award_bonus(id, staffreversebonus);

									if(tutoriallevel!=1){
										emit_sound_at(heavyimpactsound, victim->coords, 128.);
									}
									victim->RagDoll(0);
									award_bonus(id, staffreversebonus); // Huh, again?

									XYZ relative;
									relative=victim->coords-oldcoords;
									relative.y=0;
									Normalise(&relative);
									//relative=DoRotation(relative,0,-90,0);
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*30;
									}
									victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
									//FootLand(1,2);
									victim->Puff(head);
									victim->DoDamage(damagemult*70/victim->protectionhigh);
								}

								if(targetanimation==upunchreversalanim&&animation[targetanimation].label[currentframe]==7){
									escapednum=0;
									victim->RagDoll(1);
									XYZ relative;
									relative=facing;
									relative.y=0;
									Normalise(&relative);
									//relative*=-1;
									relative.y-=.1;
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*70;
									}
									victim->skeleton.joints[victim->skeleton.jointlabels[lefthand]].velocity*=.1;
									victim->skeleton.joints[victim->skeleton.jointlabels[leftwrist]].velocity*=.2;
									victim->skeleton.joints[victim->skeleton.jointlabels[leftelbow]].velocity*=.5;
									victim->skeleton.joints[victim->skeleton.jointlabels[leftshoulder]].velocity*=.7;
									victim->skeleton.joints[victim->skeleton.jointlabels[righthand]].velocity*=.1;
									victim->skeleton.joints[victim->skeleton.jointlabels[rightwrist]].velocity*=.2;
									victim->skeleton.joints[victim->skeleton.jointlabels[rightelbow]].velocity*=.5;
									victim->skeleton.joints[victim->skeleton.jointlabels[rightshoulder]].velocity*=.7;

									victim->Puff(abdomen);
									victim->DoDamage(damagemult*90/victim->protectionhigh);

									award_bonus(id, Reversal);

									bool doslice;
									doslice=0;
									if(weaponactive!=-1||creature==wolftype)doslice=1;
									if(creature==rabbittype&&weaponactive!=-1)if(weapons.type[weaponids[0]]==staff)doslice=0;
									if(doslice){
										if(weaponactive!=-1){
											victim->DoBloodBig(2/victim->armorhigh,225);
											emit_sound_at(knifeslicesound, victim->coords);
											if(bloodtoggle&&!weapons.bloody[weaponids[weaponactive]])weapons.bloody[weaponids[weaponactive]]=1;
											weapons.blooddrip[weaponids[weaponactive]]+=3;
										}
										if(weaponactive==-1&&creature==wolftype){;
											emit_sound_at(clawslicesound, victim->coords, 128.);
											victim->spurt=1;
											victim->DoBloodBig(2/victim->armorhigh,175);
										}
									}
								}



								if(targetanimation==swordslashreversalanim&&animation[targetanimation].label[currentframe]==7){
									escapednum=0;
									victim->RagDoll(1);
									XYZ relative;
									relative=facing;
									relative.y=0;
									Normalise(&relative);
									//relative*=-1;
									relative.y-=.1;
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*70;
									}
									victim->skeleton.joints[victim->skeleton.jointlabels[lefthand]].velocity*=.1-1;
									victim->skeleton.joints[victim->skeleton.jointlabels[leftwrist]].velocity*=.2-1;
									victim->skeleton.joints[victim->skeleton.jointlabels[leftelbow]].velocity*=.5-1;
									victim->skeleton.joints[victim->skeleton.jointlabels[leftshoulder]].velocity*=.7-1;
									victim->skeleton.joints[victim->skeleton.jointlabels[righthand]].velocity*=.1-1;
									victim->skeleton.joints[victim->skeleton.jointlabels[rightwrist]].velocity*=.2-1;
									victim->skeleton.joints[victim->skeleton.jointlabels[rightelbow]].velocity*=.5-1;
									victim->skeleton.joints[victim->skeleton.jointlabels[rightshoulder]].velocity*=.7-1;

									award_bonus(id, swordreversebonus);
								}

								if(hasvictim&&targetanimation==knifeslashreversalanim&&animation[targetanimation].label[currentframe]==7){
									escapednum=0;
									if(id==0)camerashake+=.4;
									if(Random()%2){
										victim->spurt=1;
										DoBlood(.2,230);
									}
									if(tutoriallevel!=1){
										emit_sound_at(heavyimpactsound, victim->coords, 128.);
									}
									victim->RagDoll(0);
									XYZ relative;
									relative=victim->coords-oldcoords;
									relative.y=0;
									Normalise(&relative);
									relative=DoRotation(relative,0,-90,0);
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*40;
									}
									victim->skeleton.joints[victim->skeleton.jointlabels[abdomen]].velocity+=relative*damagemult*200;
									//FootLand(1,2);
									victim->Puff(abdomen);
									victim->DoDamage(damagemult*30/victim->protectionhigh);

									award_bonus(id, Reversal);
								}

								if(hasvictim&&targetanimation==sneakattackanim&&animation[targetanimation].label[currentframe]==7){
									escapednum=0;
									victim->RagDoll(0);
									victim->skeleton.spinny=0;
									XYZ relative;
									relative=facing*-1;
									relative.y=-3;
									Normalise(&relative);
									if(victim->id==0)relative/=30;
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*40;
									}
									//victim->DoDamage(1000);
									victim->damage=victim->damagetolerance;
									victim->permanentdamage=victim->damagetolerance-1;
									bool doslice;
									doslice=0;
									if(weaponactive!=-1||creature==wolftype)doslice=1;
									if(creature==rabbittype&&weaponactive!=-1)if(weapons.type[weaponids[0]]==staff)doslice=0;
									if(doslice){
										if(weaponactive!=-1){
											victim->DoBloodBig(200,225);
											emit_sound_at(knifeslicesound, victim->coords);
											if(bloodtoggle)weapons.bloody[weaponids[weaponactive]]=2;
											weapons.blooddrip[weaponids[weaponactive]]+=5;
										}

										if(creature==wolftype&&weaponactive==-1){
											emit_sound_at(clawslicesound, victim->coords, 128.);
											victim->spurt=1;
											victim->DoBloodBig(2,175);
										}
									}
									award_bonus(id, spinecrusher);
								}

								if(hasvictim&&(targetanimation==knifefollowanim||targetanimation==knifesneakattackanim)&&animation[targetanimation].label[currentframe]==5){
									if(weaponactive!=-1&&victim->bloodloss<victim->damagetolerance){
										escapednum=0;
										if(targetanimation==knifefollowanim)victim->DoBloodBig(200,210);
										if(targetanimation==knifesneakattackanim){
											/*victim->DoBloodBig(200,195);
											XYZ bloodvel;
											bloodvel=0;
											bloodvel.z=20;
											bloodvel.y=5;
											bloodvel=DoRotation(bloodvel,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
											Sprite::MakeSprite(bloodsprite, DoRotation(skeleton.joints[skeleton.jointlabels[neck]].position,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .05, 1);
											*/
											XYZ footvel,footpoint;
											footvel=0;
											footpoint=weapons.tippoint[weaponids[0]];
											if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .9, .3);
											footvel=(weapons.tippoint[weaponids[0]]-weapons.position[weaponids[0]]);
											Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*7,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
											Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*3,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
											Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*5, 1,1,1, .3, 1);
											Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*2, 1,1,1, .3, 1);
											victim->DoBloodBig(200,195);
											award_bonus(id, tracheotomy);
										}
										if(targetanimation==knifefollowanim){
											award_bonus(id, Stabbonus);
											XYZ footvel,footpoint;
											footvel=0;
											footpoint=weapons.tippoint[weaponids[0]];
											if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .9, .3);
											footvel=(weapons.tippoint[weaponids[0]]-weapons.position[weaponids[0]])*-1;
											Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*7,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
											Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*3,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
											Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*5, 1,1,1, .2, 1);
											Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*2, 1,1,1, .2, 1);

										}
										victim->bloodloss+=10000;
										victim->velocity=0;
										emit_sound_at(fleshstabsound, victim->coords);
										if(bloodtoggle)weapons.bloody[weaponids[weaponactive]]=2;
										weapons.blooddrip[weaponids[weaponactive]]+=5;
									}
								}

								if(hasvictim&&(targetanimation==knifefollowanim||targetanimation==knifesneakattackanim)&&animation[targetanimation].label[currentframe]==6){
									escapednum=0;
									victim->velocity=0;
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity=0;
									}
									if(targetanimation==knifefollowanim){
										victim->RagDoll(0);
										for(i=0;i<victim->skeleton.num_joints;i++){
											victim->skeleton.joints[i].velocity=0;
										}
									}
									if(weaponactive!=-1&&animation[victim->targetanimation].attack!=reversal){
										emit_sound_at(fleshstabremovesound, victim->coords);
										if(bloodtoggle)weapons.bloody[weaponids[weaponactive]]=2;
										weapons.blooddrip[weaponids[weaponactive]]+=5;

										XYZ footvel,footpoint;
										footvel=0;
										footpoint=weapons.tippoint[weaponids[0]];
										if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .9, .3);
										footvel=(weapons.tippoint[weaponids[0]]-weapons.position[weaponids[0]])*-1;
										Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*7,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
										Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*3,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
										Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*5, 1,1,1, .3, 1);
										Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*2, 1,1,1, .3, 1);
									}
								}

								if(hasvictim&&(targetanimation==swordsneakattackanim)&&animation[targetanimation].label[currentframe]==5){
									if(weaponactive!=-1&&victim->bloodloss<victim->damagetolerance){
										award_bonus(id, backstab);

										escapednum=0;

										XYZ footvel,footpoint;
										footvel=0;
										footpoint=(weapons.tippoint[weaponids[0]]+weapons.position[weaponids[0]])/2;
										if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .9, .3);
										footvel=(weapons.tippoint[weaponids[0]]-weapons.position[weaponids[0]]);
										Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*7,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
										Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*3,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
										Sprite::MakeSprite(bloodflamesprite, footpoint,DoRotation(footvel*5,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .3, 1);
										Sprite::MakeSprite(bloodflamesprite, footpoint,DoRotation(footvel*3,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .3, 1);
										victim->DoBloodBig(200,180);
										victim->DoBloodBig(200,215);
										victim->bloodloss+=10000;
										victim->velocity=0;
										emit_sound_at(fleshstabsound, victim->coords);
										if(bloodtoggle)weapons.bloody[weaponids[weaponactive]]=2;
										weapons.blooddrip[weaponids[weaponactive]]+=5;
									}
								}

								if(hasvictim&&targetanimation==swordsneakattackanim&&animation[targetanimation].label[currentframe]==6){
									escapednum=0;
									victim->velocity=0;
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity=0;
									}
									if(weaponactive!=-1){
										emit_sound_at(fleshstabremovesound, victim->coords);
										if(bloodtoggle)weapons.bloody[weaponids[weaponactive]]=2;
										weapons.blooddrip[weaponids[weaponactive]]+=5;

										XYZ footvel,footpoint;
										footvel=0;
										footpoint=weapons.tippoint[weaponids[0]];
										if(bloodtoggle)Sprite::MakeSprite(cloudimpactsprite, footpoint,footvel, 1,0,0, .9, .3);
										footvel=(weapons.tippoint[weaponids[0]]-weapons.position[weaponids[0]])*-1;
										Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*7,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
										Sprite::MakeSprite(bloodsprite,footpoint,DoRotation(footvel*3,(float)(Random()%20),(float)(Random()%20),0), 1,1,1, .05, .9);
										Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*5, 1,1,1, .3, 1);
										Sprite::MakeSprite(bloodflamesprite, footpoint,footvel*2, 1,1,1, .3, 1);
									}
								}

								if(targetanimation==sweepreversalanim&&animation[targetanimation].label[currentframe]==7){
									escapednum=0;
									if(id==0)camerashake+=.4;
									if(Random()%2){
										victim->spurt=1;
										DoBlood(.2,240);
									}
									if(weaponactive==-1){
										if(tutoriallevel!=1){
											emit_sound_at(heavyimpactsound, victim->coords, 128.);
										}
									}
									bool doslice;
									doslice=0;
									if(weaponactive!=-1||creature==wolftype)doslice=1;
									if(creature==rabbittype&&weaponactive!=-1)if(weapons.type[weaponids[0]]==staff)doslice=0;
									if(doslice){
										if(weaponactive!=-1){
											victim->DoBloodBig(2/victim->armorhead,225);
											emit_sound_at(knifeslicesound, victim->coords);
											if(bloodtoggle&&!weapons.bloody[weaponids[weaponactive]])weapons.bloody[weaponids[weaponactive]]=1;
											weapons.blooddrip[weaponids[weaponactive]]+=3;
										}
										if(weaponactive==-1&&creature==wolftype){
											emit_sound_at(clawslicesound, victim->coords, 128.);
											victim->spurt=1;
											victim->DoBloodBig(2/victim->armorhead,175);
										}
									}

									award_bonus(id, Reversal);

									victim->Puff(neck);

									XYZ relative;
									//relative=victim->coords-oldcoords;
									relative=facing*-1;
									relative.y=0;
									Normalise(&relative);
									relative=DoRotation(relative,0,90,0);
									relative.y=.5;
									Normalise(&relative);
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*20;
									}
									victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*200;
									if(victim->damage<victim->damagetolerance-100)victim->velocity=relative*200;
									victim->DoDamage(damagemult*100/victim->protectionhead);
									victim->velocity=0;
								}

								if(targetanimation==sweepreversalanim&&((animation[targetanimation].label[currentframe]==9&&victim->damage<victim->damagetolerance)||(animation[targetanimation].label[currentframe]==7&&victim->damage>victim->damagetolerance))){
									escapednum=0;
									victim->RagDoll(0);
									XYZ relative;
									//relative=victim->coords-oldcoords;
									relative=facing*-1;
									relative.y=0;
									Normalise(&relative);
									relative=DoRotation(relative,0,90,0);
									relative.y=.5;
									Normalise(&relative);
									for(i=0;i<victim->skeleton.num_joints;i++){
										victim->skeleton.joints[i].velocity+=relative*damagemult*20;
									}
									victim->skeleton.joints[victim->skeleton.jointlabels[head]].velocity+=relative*damagemult*200;
								}

								if(hasvictim&&(targetanimation==spinkickreversalanim||targetanimation==sweepreversalanim||targetanimation==rabbitkickreversalanim||targetanimation==upunchreversalanim||targetanimation==jumpreversalanim||targetanimation==swordslashreversalanim||targetanimation==knifeslashreversalanim||targetanimation==rabbittacklereversal||targetanimation==wolftacklereversal||targetanimation==staffhitreversalanim||targetanimation==staffspinhitreversalanim))
									if(victim->damage>victim->damagetolerance&&bonus!=reverseko){
									  award_bonus(id, reverseko);
									}
							}


							//Animation end
							if(targetframe>animation[currentanimation].numframes-1){
								targetframe=0;
								if(wasStop()){
									targetanimation=getIdle();
									FootLand(0,1);
									FootLand(1,1);
								}
								if(currentanimation==rabbittackleanim||currentanimation==rabbittacklinganim){
									targetanimation=rollanim;
									targetframe=3;
									emit_sound_at(movewhooshsound, coords, 128.);
								}
								if(currentanimation==staggerbackhighanim){
									targetanimation=getIdle();
								}
								if(currentanimation==staggerbackhardanim){
									targetanimation=getIdle();
								}
								if(currentanimation==removeknifeanim){
									targetanimation=getIdle();
								}
								if(currentanimation==crouchremoveknifeanim){
									targetanimation=getCrouch();
								}
								if(currentanimation==backhandspringanim){
									targetanimation=getIdle();
								}
								if(currentanimation==dodgebackanim){
									targetanimation=getIdle();
								}
								if(currentanimation==drawleftanim){
									targetanimation=getIdle();
								}
								if(currentanimation==drawrightanim||currentanimation==crouchdrawrightanim){
									targetanimation=getIdle();
									if(currentanimation==crouchdrawrightanim){
										targetanimation=getCrouch();
									}
									if(weaponactive==-1)weaponactive=0;
									else if(weaponactive==0){
										weaponactive=-1;
										if(num_weapons==2){
											int buffer;
											buffer=weaponids[0];
											weaponids[0]=weaponids[1];
											weaponids[1]=buffer;
										}
									}

									if(weaponactive==-1){
										emit_sound_at(knifesheathesound, coords, 128.);
									}
									if(weaponactive!=-1){
										emit_sound_at(knifedrawsound, coords, 128.);
									}
								}
								if(currentanimation==rollanim){
									targetanimation=getCrouch();
									FootLand(0,1);
									FootLand(1,1);
								}
								if(isFlip()){
									if(targetanimation==walljumprightkickanim){
										targetrot=-190;
									}
									if(targetanimation==walljumpleftkickanim){
										targetrot=190;
									}
									targetanimation=jumpdownanim;
								}
								if(currentanimation==climbanim){
									targetanimation=getCrouch();
									targetframe=1;
									coords+=facing*.1;
									if(!isnormal(coords.x))
										coords=oldcoords;
									oldcoords=coords;
									collided=0;
									targetoffset=0;
									currentoffset=0;
									grabdelay=1;
									velocity=0;
									collided=0;
									avoidcollided=0;
								}
								if(targetanimation==rabbitkickreversalanim){
									targetanimation=getCrouch();
									lastfeint=0;
								}
								if(targetanimation==jumpreversalanim){
									targetanimation=getCrouch();
									lastfeint=0;
								}
								if(targetanimation==walljumprightanim||targetanimation==walljumpbackanim||targetanimation==walljumpfrontanim){
									if(attackkeydown&&targetanimation!=walljumpfrontanim){
										int closest=-1;
										float closestdist=-1;
										float distance;
										if(numplayers>1)
											for(i=0;i<numplayers;i++){
												if(id!=i&&player[i].coords.y<coords.y&&!player[i].skeleton.free){
													distance=findDistancefast(&player[i].coords,&coords);
													if(closestdist==-1||distance<closestdist){
														closestdist=distance;
														closest=i;
													}
												}
											}
											if(closestdist>0&&closest>=0&&closestdist<16){
												victim=&player[closest];
												targetanimation=walljumprightkickanim;
												targetframe=0;
												XYZ rotatetarget=victim->coords-coords;
												Normalise(&rotatetarget);
												rotation=-asin(0-rotatetarget.x);
												rotation*=360/6.28;
												if(rotatetarget.z<0)rotation=180-rotation;
												targettilt2=-asin(rotatetarget.y)*360/6.28;
												velocity=(victim->coords-coords)*4;
												velocity.y+=2;
												transspeed=40;
											}
									}
									if(targetanimation==walljumpbackanim){
										targetanimation=backflipanim;
										targetframe=3;
										velocity=facing*-8;
										velocity.y=4;
										if(id==0)
										  resume_stream(whooshsound);
									}
									if(targetanimation==walljumprightanim){
										targetanimation=rightflipanim;
										targetframe=4;
										targetrotation-=90;
										rotation-=90;
										velocity=DoRotation(facing,0,30,0)*-8;
										velocity.y=4;
									}
									if(targetanimation==walljumpfrontanim){
										targetanimation=frontflipanim;
										targetframe=2;
										//targetrotation-=180;
										////rotation-=180;
										velocity=facing*8;
										velocity.y=4;
									}
									if(id==0)
									  resume_stream(whooshsound);
								}
								if(targetanimation==walljumpleftanim){
									if(attackkeydown){
										int closest=-1;
										float closestdist=-1;
										float distance;
										if(numplayers>1)
											for(i=0;i<numplayers;i++){
												if(id!=i&&player[i].coords.y<coords.y&&!player[i].skeleton.free){
													distance=findDistancefast(&player[i].coords,&coords);
													if(closestdist==-1||distance<closestdist){
														closestdist=distance;
														closest=i;
													}
												}
											}
											if(closestdist>0&&closest>=0&&closestdist<16){
												victim=&player[closest];
												targetanimation=walljumpleftkickanim;
												targetframe=0;
												XYZ rotatetarget=victim->coords-coords;
												Normalise(&rotatetarget);
												rotation=-asin(0-rotatetarget.x);
												rotation*=360/6.28;
												if(rotatetarget.z<0)rotation=180-rotation;
												targettilt2=-asin(rotatetarget.y)*360/6.28;
												velocity=(victim->coords-coords)*4;
												velocity.y+=2;
												transspeed=40;
											}
									}
									if(targetanimation!=walljumpleftkickanim){
										targetanimation=leftflipanim;
										targetframe=4;
										targetrotation+=90;
										rotation+=90;
										velocity=DoRotation(facing,0,-30,0)*-8;
										velocity.y=4;
									}
									if(id==0)
									  resume_stream(whooshsound);
								}
								if(targetanimation==sneakattackanim){
									float ycoords=oldcoords.y;
									currentanimation=getCrouch();
									targetanimation=getCrouch();
									targetframe=1;
									currentframe=0;
									targetrotation+=180;
									rotation+=180;
									targettilt2*=-1;
									tilt2*=-1;
									transspeed=1000000;
									targetheadrotation+=180;
									coords-=facing*.7;
									if(onterrain)coords.y=terrain.getHeight(coords.x,coords.z);

									lastfeint=0;
								}
								if(targetanimation==knifesneakattackanim||targetanimation==swordsneakattackanim){
									float ycoords=oldcoords.y;
									targetanimation=getIdle();
									targetframe=0;
									if(onterrain)coords.y=terrain.getHeight(coords.x,coords.z);

									lastfeint=0;
								}
								if(currentanimation==knifefollowanim){
									targetanimation=getIdle();
									lastfeint=0;
								}
								if(animation[targetanimation].attack==reversal&&currentanimation!=sneakattackanim&&currentanimation!=knifesneakattackanim&&currentanimation!=swordsneakattackanim&&currentanimation!=knifefollowanim){
									float ycoords=oldcoords.y;
									targetanimation=getStop();
									targetrotation+=180;
									rotation+=180;
									targettilt2*=-1;
									tilt2*=-1;
									transspeed=1000000;
									targetheadrotation+=180;
									if(!isnormal(coords.x))
										coords=oldcoords;
									if(currentanimation==spinkickreversalanim||currentanimation==swordslashreversalanim)
										oldcoords=coords+facing*.5;
									else if(currentanimation==sweepreversalanim)
										oldcoords=coords+facing*1.1;
									else if(currentanimation==upunchreversalanim){
										oldcoords=coords+facing*1.5;
										targetrotation+=180;
										rotation+=180;
										targetheadrotation+=180;
										targettilt2*=-1;
										tilt2*=-1;
									}
									else if(currentanimation==knifeslashreversalanim){
										oldcoords=coords+facing*.5;
										targetrotation+=90;
										rotation+=90;
										targetheadrotation+=90;
										targettilt2=0;
										tilt2=0;
									}
									else if(currentanimation==staffspinhitreversalanim){
										targetrotation+=180;
										rotation+=180;
										targetheadrotation+=180;
										targettilt2=0;
										tilt2=0;
									}
									if(onterrain)oldcoords.y=terrain.getHeight(oldcoords.x,oldcoords.z);
									else oldcoords.y=ycoords;
									currentoffset=coords-oldcoords;
									targetoffset=0;
									coords=oldcoords;

									lastfeint=0;
								}
								if(currentanimation==knifesneakattackedanim||currentanimation==swordsneakattackedanim){
									velocity=0;
									velocity.y=-5;
									RagDoll(0);
								}
								if(animation[targetanimation].attack==reversed){
									escapednum++;
									if(targetanimation==sweepreversedanim)targetrotation+=90;
									targetanimation=backhandspringanim;
									targetframe=2;
									emit_sound_at(landsound, coords, 128);

									if(currentanimation==upunchreversedanim||currentanimation==swordslashreversedanim){
										targetanimation=rollanim;
										targetframe=5;
										oldcoords=coords;
										coords+=(DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)+DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0))/2*scale;
										coords.y=oldcoords.y;
									}
									if(currentanimation==knifeslashreversedanim){
										targetanimation=rollanim;
										targetframe=0;
										targetrotation+=90;
										rotation+=90;
										oldcoords=coords;
										coords+=(DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)+DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0))/2*scale;
										coords.y=oldcoords.y;
									}
								}
								if(wasFlip()){
									targetanimation=jumpdownanim;
								}
								if(wasLanding())targetanimation=getIdle();
								if(wasLandhard())targetanimation=getIdle();
								if(currentanimation==spinkickanim||currentanimation==getupfrombackanim||currentanimation==getupfromfrontanim||currentanimation==lowkickanim){
									targetanimation=getIdle();
									oldcoords=coords;
									coords+=(DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)+DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0))/2*scale;
									coords.y=oldcoords.y;
									//coords+=DoRotation(animation[currentanimation].offset,0,rotation,0)*scale;
									targetoffset.y=coords.y;
									if(onterrain)targetoffset.y=terrain.getHeight(coords.x,coords.z);
									currentoffset=DoRotation(animation[currentanimation].offset*-1,0,rotation,0)*scale;
									currentoffset.y-=(coords.y-targetoffset.y);
									coords.y=targetoffset.y;
									targetoffset=0;
									normalsupdatedelay=0;
								}
								if(currentanimation==upunchanim){
									targetanimation=getStop();
									normalsupdatedelay=0;
									lastfeint=0;
								}
								if(currentanimation==rabbitkickanim&&targetanimation!=backflipanim){
									targetrotation=rotation;
									bool hasstaff;
									hasstaff=0;
									if(num_weapons>0)if(weapons.type[0]==staff)hasstaff=1;
									if(!hasstaff)DoDamage(35);
									RagDoll(0);
									lastfeint=0;
									rabbitkickragdoll=1;
								}
								if(currentanimation==rabbitkickreversedanim){
									if(!feint){
										velocity=0;
										velocity.y=-10;
										//DoDamage(100);
										RagDoll(0);
										skeleton.spinny=0;
										SolidHitBonus(!id); // FIXME: tricky id
									}
									if(feint){
										escapednum++;
										targetanimation=rollanim;
										coords+=facing;
										if(id==0)pause_sound(whooshsound);
									}
									lastfeint=0;
								}
								if(currentanimation==rabbittackledbackanim||currentanimation==rabbittackledfrontanim){
									velocity=0;
									velocity.y=-10;
									RagDoll(0);
									skeleton.spinny=0;
								}
								if(currentanimation==jumpreversedanim){
									if(!feint){
										velocity=0;
										velocity.y=-10;
										//DoDamage(100);
										RagDoll(0);
										skeleton.spinny=0;
										SolidHitBonus(!id); // FIXME: tricky id
									}
									if(feint){
										escapednum++;
										targetanimation=rollanim;
										coords+=facing*2;
										if(id==0)pause_sound(whooshsound);
									}
									lastfeint=0;
								}

								if(animation[currentanimation].attack==normalattack&&!victim->skeleton.free&&victim->targetanimation!=staggerbackhighanim&&victim->targetanimation!=staggerbackhardanim&&targetanimation!=winduppunchblockedanim&&targetanimation!=blockhighleftanim&&targetanimation!=swordslashparryanim&&targetanimation!=swordslashparriedanim&&targetanimation!=crouchstabanim&&targetanimation!=swordgroundstabanim){
									targetanimation=getupfromfrontanim;
									lastfeint=0;
								}
								else if(animation[currentanimation].attack==normalattack){
									targetanimation=getIdle();
									lastfeint=0;
								}
								if(currentanimation==blockhighleftanim&&aitype!=playercontrolled){
									targetanimation=blockhighleftstrikeanim;
								}
								if(currentanimation==knifeslashstartanim||currentanimation==knifethrowanim||currentanimation==swordslashanim||currentanimation==staffhitanim||currentanimation==staffgroundsmashanim||currentanimation==staffspinhitanim){
									targetanimation=getIdle();
									lastfeint=0;
								}
								if(currentanimation==spinkickanim&&victim->skeleton.free){
									if(creature==rabbittype)targetanimation=fightidleanim;
								}
							}
							target=0;

							if(isIdle()&&!wasIdle())normalsupdatedelay=0;

							if(currentanimation==jumpupanim&&velocity.y<0&&!isFlip()){
								targetanimation=jumpdownanim;
							}
		}
		if(!skeleton.free){
			oldtarget=target;
			if(!transspeed&&animation[targetanimation].attack!=2&&animation[targetanimation].attack!=3){
				if(!isRun()||!wasRun()){
					if(animation[targetanimation].speed[targetframe]>animation[currentanimation].speed[currentframe])
						target+=multiplier*animation[targetanimation].speed[targetframe]*speed*2;
					if(animation[targetanimation].speed[targetframe]<=animation[currentanimation].speed[currentframe])
						target+=multiplier*animation[currentanimation].speed[currentframe]*speed*2;
				}
				if(isRun()&&wasRun()){
					float tempspeed;
					tempspeed=velspeed;
					if(tempspeed<10*speedmult)tempspeed=10*speedmult;
					target+=multiplier*animation[targetanimation].speed[currentframe]*speed*1.7*tempspeed/(speed*45*scale);
				}
			}
			else if(transspeed)target+=multiplier*transspeed*speed*2;
			else{
				if(!isRun()||!wasRun()){
					if(animation[targetanimation].speed[targetframe]>animation[currentanimation].speed[currentframe])
						target+=multiplier*animation[targetanimation].speed[targetframe]*2;
					if(animation[targetanimation].speed[targetframe]<=animation[currentanimation].speed[currentframe])
						target+=multiplier*animation[currentanimation].speed[currentframe]*2;
				}
			}

			if(currentanimation!=targetanimation)target=(target+oldtarget)/2;

			if(target>1){currentframe=targetframe; target=1;}
			oldrot=rot;
			rot=targetrot*target;
			rotation+=rot-oldrot;
			if(target==1){
				rot=0;
				oldrot=0;
				targetrot=0;
			}
			if(currentanimation!=oldcurrentanimation||targetanimation!=oldtargetanimation||((currentframe!=oldcurrentframe||targetframe!=oldtargetframe)&&!calcrot)){
				//Old rotates
				for(i=0;i<skeleton.num_joints;i++){
					skeleton.joints[i].position=animation[currentanimation].position[i][currentframe];
				}

				skeleton.FindForwards();

				for(i=0;i<skeleton.num_muscles;i++){
					if(skeleton.muscles[i].visible)
					{
						skeleton.FindRotationMuscle(i,targetanimation);
					}
				}
				for(i=0;i<skeleton.num_muscles;i++){
					if(skeleton.muscles[i].visible)
					{
						if(isnormal((float)((int)(skeleton.muscles[i].rotate1*100)%36000)/100))skeleton.muscles[i].oldrotate1=(float)((int)(skeleton.muscles[i].rotate1*100)%36000)/100;
						if(isnormal((float)((int)(skeleton.muscles[i].rotate2*100)%36000)/100))skeleton.muscles[i].oldrotate2=(float)((int)(skeleton.muscles[i].rotate2*100)%36000)/100;
						if(isnormal((float)((int)(skeleton.muscles[i].rotate3*100)%36000)/100))skeleton.muscles[i].oldrotate3=(float)((int)(skeleton.muscles[i].rotate3*100)%36000)/100;
					}
				}

				//New rotates
				for(i=0;i<skeleton.num_joints;i++){
					skeleton.joints[i].position=animation[targetanimation].position[i][targetframe];
				}

				skeleton.FindForwards();

				for(i=0;i<skeleton.num_muscles;i++){
					if(skeleton.muscles[i].visible)
					{
						skeleton.FindRotationMuscle(i,targetanimation);
					}
				}
				for(i=0;i<skeleton.num_muscles;i++){
					if(skeleton.muscles[i].visible)
					{
						if(isnormal((float)((int)(skeleton.muscles[i].rotate1*100)%36000)/100))skeleton.muscles[i].newrotate1=(float)((int)(skeleton.muscles[i].rotate1*100)%36000)/100;
						if(isnormal((float)((int)(skeleton.muscles[i].rotate2*100)%36000)/100))skeleton.muscles[i].newrotate2=(float)((int)(skeleton.muscles[i].rotate2*100)%36000)/100;
						if(isnormal((float)((int)(skeleton.muscles[i].rotate3*100)%36000)/100))skeleton.muscles[i].newrotate3=(float)((int)(skeleton.muscles[i].rotate3*100)%36000)/100;
						if(skeleton.muscles[i].newrotate3>skeleton.muscles[i].oldrotate3+180)skeleton.muscles[i].newrotate3-=360;
						if(skeleton.muscles[i].newrotate3<skeleton.muscles[i].oldrotate3-180)skeleton.muscles[i].newrotate3+=360;
						if(skeleton.muscles[i].newrotate2>skeleton.muscles[i].oldrotate2+180)skeleton.muscles[i].newrotate2-=360;
						if(skeleton.muscles[i].newrotate2<skeleton.muscles[i].oldrotate2-180)skeleton.muscles[i].newrotate2+=360;
						if(skeleton.muscles[i].newrotate1>skeleton.muscles[i].oldrotate1+180)skeleton.muscles[i].newrotate1-=360;
						if(skeleton.muscles[i].newrotate1<skeleton.muscles[i].oldrotate1-180)skeleton.muscles[i].newrotate1+=360;
					}
				}
			}
			if(currentframe>=animation[currentanimation].numframes)currentframe=animation[currentanimation].numframes-1;

			oldcurrentanimation=currentanimation;
			oldtargetanimation=targetanimation;
			oldtargetframe=targetframe;
			oldcurrentframe=currentframe;

			for(i=0;i<skeleton.num_joints;i++){
				skeleton.joints[i].velocity=(animation[currentanimation].position[i][currentframe]*(1-target)+animation[targetanimation].position[i][targetframe]*(target)-skeleton.joints[i].position)/multiplier;
				skeleton.joints[i].position=animation[currentanimation].position[i][currentframe]*(1-target)+animation[targetanimation].position[i][targetframe]*(target);
			}
			offset=currentoffset*(1-target)+targetoffset*target;
			for(i=0;i<skeleton.num_muscles;i++){
				if(skeleton.muscles[i].visible)
				{
					skeleton.muscles[i].rotate1=skeleton.muscles[i].oldrotate1*(1-target)+skeleton.muscles[i].newrotate1*(target);
					skeleton.muscles[i].rotate2=skeleton.muscles[i].oldrotate2*(1-target)+skeleton.muscles[i].newrotate2*(target);
					skeleton.muscles[i].rotate3=skeleton.muscles[i].oldrotate3*(1-target)+skeleton.muscles[i].newrotate3*(target);
				}
			}
		}

		if(isLanding()&&landhard){
			if(id==0)camerashake+=.4;
			targetanimation=getLandhard();
			targetframe=0;
			target=0;
			landhard=0;
			transspeed=15;
		}
	}
	//skeleton.DoConstraints();
}

void	Person::DoStuff(){
	static XYZ terrainnormal;
	static XYZ flatfacing;
	static XYZ flatvelocity;
	static float flatvelspeed;
	static int i,j,l;
	static XYZ average;
	static int howmany;
	static int bloodsize;
	static int startx,starty,endx,endy;
	static int texdetailint;
	static GLubyte color;
	static XYZ bloodvel;

	onfiredelay-=multiplier;
	if(onfiredelay<0&&onfire)
	{
		if(Random()%2==0){
			crouchkeydown=1;
		}
		onfiredelay=0.3;
	}

	crouchkeydowntime+=multiplier;
	if(!crouchkeydown)crouchkeydowntime=0;
	jumpkeydowntime+=multiplier;
	if(!jumpkeydown&&skeleton.free)jumpkeydowntime=0;

	if(hostile||damage>0||bloodloss>0)immobile=0;

	if(isIdle()||isRun())targetoffset=0;

	if(num_weapons==1&&weaponactive!=-1)weaponstuck=-1;

	if(id==0)blooddimamount-=multiplier*.3;
	speechdelay-=multiplier;
	texupdatedelay-=multiplier;
	interestdelay-=multiplier;
	flamedelay-=multiplier;
	parriedrecently-=multiplier;
	if(!victim){
		victim=this;
		hasvictim=0;
	}

	if(id==0)speed=1.1*speedmult;
	else speed=1.0*speedmult;
	if(!skeleton.free)rabbitkickragdoll=0;

	speed*=speedmult;

	if(id!=0&&(creature==rabbittype||difficulty!=2))superruntoggle=0;
	if(id!=0&&creature==wolftype&&difficulty==2){
		superruntoggle=0;
		if(aitype!=passivetype){
			superruntoggle=1;
			if(aitype==attacktypecutoff&&(player[0].isIdle()||player[0].isCrouch()||player[0].skeleton.free||player[0].targetanimation==getupfrombackanim||player[0].targetanimation==getupfromfrontanim||player[0].targetanimation==sneakanim)&&findDistancefast(&coords,&player[0].coords)<16){
				superruntoggle=0;
			}
		}
		if(scale<0.2)superruntoggle=0;
		if(targetanimation==wolfrunninganim&&!superruntoggle){
			targetanimation=getRun();
			targetframe=0;
		}
	}
	if(weaponactive==-1&&num_weapons>0){
		if(weapons.type[weaponids[0]]==staff){
			weaponactive=0;
		}
	}

	if(onfire){
		burnt+=multiplier;
		/*if(aitype!=playercontrolled)*///deathbleeding=5;
		/*if(aitype!=playercontrolled)*/
		deathbleeding=1;
		if(burnt>.6)burnt=.6;
		OPENAL_SetVolume(channels[stream_firesound], 256+256*findLength(&velocity)/3);

		if(targetanimation==jumpupanim||targetanimation==jumpdownanim||isFlip()){
			float gLoc[3];
			float vel[3];
			gLoc[0]=coords.x;
			gLoc[1]=coords.y;
			gLoc[2]=coords.z;
			vel[0]=velocity.x;
			vel[1]=velocity.y;
			vel[2]=velocity.z;

			if(id==0){
				OPENAL_3D_SetAttributes(channels[whooshsound], gLoc, vel);
				OPENAL_SetVolume(channels[whooshsound], 64*findLength(&velocity)/5);
			}
		}
	}
	while(flamedelay<0&&onfire){
		flamedelay+=.006;
		howmany=abs(Random()%(skeleton.num_joints));
		if(!skeleton.free)flatvelocity=(coords-oldcoords)/multiplier/2;//velocity/2;
		if(skeleton.free)flatvelocity=skeleton.joints[howmany].velocity*scale/2;
		if(!skeleton.free)flatfacing=DoRotation(DoRotation(DoRotation(skeleton.joints[howmany].position,0,0,tilt),tilt2,0,0),0,rotation,0)*scale+coords;
		if(skeleton.free)flatfacing=skeleton.joints[howmany].position*scale+coords;
		Sprite::MakeSprite(flamesprite, flatfacing,flatvelocity, 1,1,1, .6+(float)abs(Random()%100)/200-.25, 1);
	}

	while(flamedelay<0&&!onfire&&tutoriallevel==1&&id!=0){
		flamedelay+=.05;
		howmany=abs(Random()%(skeleton.num_joints));
		if(!skeleton.free)flatvelocity=(coords-oldcoords)/multiplier/2;//velocity/2;
		if(skeleton.free)flatvelocity=skeleton.joints[howmany].velocity*scale/2;
		if(!skeleton.free)flatfacing=DoRotation(DoRotation(DoRotation(skeleton.joints[howmany].position,0,0,tilt),tilt2,0,0),0,rotation,0)*scale+coords;
		if(skeleton.free)flatfacing=skeleton.joints[howmany].position*scale+coords;
		Sprite::MakeSprite(breathsprite, flatfacing,flatvelocity, 1,1,1, .6+(float)abs(Random()%100)/200-.25, .3);
	}

	if(bleeding>0){
		bleeding-=multiplier*.3;
		if(bloodtoggle==2){
			glBindTexture(GL_TEXTURE_2D,skeleton.drawmodel.textureptr);
			if(bleeding<=0&&(detail!=2||osx))DoMipmaps();
		}
	}

	if(neckspurtamount>0){
		neckspurtamount-=multiplier;
		neckspurtdelay-=multiplier*3;
		neckspurtparticledelay-=multiplier*3;
		if(neckspurtparticledelay<0&&neckspurtdelay>2){
			spurt=0;
			bloodvel=0;
			if(!skeleton.free){
				bloodvel.z=5*neckspurtamount;
				bloodvel=DoRotation(bloodvel,((float)(Random()%100))/40,rotation+((float)(Random()%100))/40,0)*scale;
			}
			if(skeleton.free){
				bloodvel-=DoRotation(skeleton.forward*10*scale,((float)(Random()%100))/40,((float)(Random()%100))/40,0);
			}
			if(skeleton.free)bloodvel+=DoRotation(skeleton.joints[skeleton.jointlabels[head]].velocity,((float)(Random()%100))/40,rotation+((float)(Random()%100))/40,0)*scale;
			if(!skeleton.free)bloodvel+=DoRotation(velocity,((float)(Random()%100))/40,((float)(Random()%100))/40,0)*scale;
			if(skeleton.free)Sprite::MakeSprite(bloodsprite, (skeleton.joints[skeleton.jointlabels[neck]].position+(skeleton.joints[skeleton.jointlabels[neck]].position-skeleton.joints[skeleton.jointlabels[head]].position)/5)*scale+coords,bloodvel, 1,1,1, .05, .9);
			if(!skeleton.free)Sprite::MakeSprite(bloodsprite, DoRotation(skeleton.joints[skeleton.jointlabels[neck]].position+(skeleton.joints[skeleton.jointlabels[neck]].position-skeleton.joints[skeleton.jointlabels[head]].position)/5,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .05, .9);
			neckspurtparticledelay=.05;
		}
		if(neckspurtdelay<0){
			neckspurtdelay=3;
		}
	}

	if(deathbleeding>0&&dead!=2){
		if(deathbleeding<5)bleeddelay-=deathbleeding*multiplier/4;
		else bleeddelay-=5*multiplier/4;
		if(bleeddelay<0&&bloodtoggle){
			bleeddelay=1;
			XYZ bloodvel;
			if(bloodtoggle){
				bloodvel=0;
				if(skeleton.free)bloodvel+=DoRotation(skeleton.joints[skeleton.jointlabels[abdomen]].velocity,((float)(Random()%100))/4,rotation+((float)(Random()%100))/4,0)*scale;
				if(!skeleton.free)bloodvel+=DoRotation(velocity,((float)(Random()%100))/4,((float)(Random()%100))/4,0)*scale;
				if(skeleton.free)Sprite::MakeSprite(bloodsprite, skeleton.joints[skeleton.jointlabels[abdomen]].position*scale+coords,bloodvel, 1,1,1, .05, 1);
				if(!skeleton.free)Sprite::MakeSprite(bloodsprite, DoRotation((skeleton.joints[skeleton.jointlabels[abdomen]].position+skeleton.joints[skeleton.jointlabels[abdomen]].position)/2,0,rotation,0)*scale+coords,bloodvel, 1,1,1, .05, 1);
			}
		}
		bloodloss+=deathbleeding*multiplier*80;
		deathbleeding-=multiplier*1.6;
		//if(id==0)deathbleeding-=multiplier*.2;
		if(deathbleeding<0)deathbleeding=0;
		if(bloodloss>damagetolerance&&animation[targetanimation].attack==neutral){
			if(weaponactive!=-1){
				weapons.owner[weaponids[0]]=-1;
				weapons.velocity[weaponids[0]]=velocity*scale*-.3;
				weapons.velocity[weaponids[0]].x+=.01;
				weapons.tipvelocity[weaponids[0]]=velocity*scale;
				weapons.missed[weaponids[0]]=1;
				weapons.hitsomething[weaponids[0]]=0;
				weapons.freetime[weaponids[0]]=0;
				weapons.firstfree[weaponids[0]]=1;
				weapons.physics[weaponids[0]]=1;
				num_weapons--;
				if(num_weapons){
					weaponids[0]=weaponids[num_weapons];
					if(weaponstuck==num_weapons)weaponstuck=0;
				}
				weaponactive=-1;
				for(i=0;i<numplayers;i++){
					player[i].wentforweapon=0;
				}

				if(id==0){
					flashamount=.5;
					flashr=1;
					flashg=0;
					flashb=0;
					flashdelay=0;
				}
			}

			if(!dead&&creature==wolftype){
			  award_bonus(0, Wolfbonus);
			}
			dead=2;
			if(targetanimation==knifefollowedanim&&!skeleton.free){
				for(i=0;i<skeleton.num_joints;i++){
					skeleton.joints[i].velocity=0;
					skeleton.joints[i].velocity.y=-2;
				}
			}
			if(id!=0&&unconscioustime>.1){
				numafterkill++;
			}

			RagDoll(0);
		}
	}

	if(texupdatedelay<0&&bleeding>0&&bloodtoggle==2&&findDistancefast(&viewer,&coords)<9){
		texupdatedelay=.12;

		bloodsize=5-realtexdetail;

		startx=0;
		starty=0;
		texdetailint=realtexdetail;
		startx=bleedy;//abs(Random()%(skeleton.skinsize-bloodsize-1));
		starty=bleedx;//abs(Random()%(skeleton.skinsize-bloodsize-1));
		endx=startx+bloodsize;
		endy=starty+bloodsize;

		if(startx<0){startx=0;bleeding=0;}
		if(starty<0){starty=0;bleeding=0;}
		if(endx>skeleton.skinsize-1){endx=skeleton.skinsize-1;bleeding=0;}
		if(endy>skeleton.skinsize-1){endy=skeleton.skinsize-1;bleeding=0;}
		if(endx<startx)endx=startx;
		if(endy<starty)endy=starty;

		for(i=startx;i<endx;i++){
			for(j=starty;j<endy;j++){
				if(Random()%2==0){
					color=Random()%85+170;
					if(skeleton.skinText[i*skeleton.skinsize*3+j*3+0]>color/2)skeleton.skinText[i*skeleton.skinsize*3+j*3+0]=color/2;
					skeleton.skinText[i*skeleton.skinsize*3+j*3+1]=0;
					skeleton.skinText[i*skeleton.skinsize*3+j*3+2]=0;
				}
			}
		}
		if(!osx&&detail>1){
			glBindTexture(GL_TEXTURE_2D,skeleton.drawmodel.textureptr);
			DoMipmaps();
		}

		if(!skeleton.free){
			bleedy-=4/realtexdetail;
			if(detail==2)bleedx+=(abs(Random()%3)-1)*2/realtexdetail;
			else bleedx+=(abs(Random()%3)-1)*4/realtexdetail;
		}
		if(skeleton.free){
			bleedx+=4*direction/realtexdetail;
			if(detail==2)bleedy+=(abs(Random()%3)-1)*2/realtexdetail;
			else bleedy+=(abs(Random()%3)-1)*4/realtexdetail;
		}
	}

	if(abs(righthandmorphness-targetrighthandmorphness)<multiplier*4){
		righthandmorphness=targetrighthandmorphness;
		righthandmorphstart=righthandmorphend;
	}
	else if(righthandmorphness>targetrighthandmorphness){
		righthandmorphness-=multiplier*4;
	}
	else if(righthandmorphness<targetrighthandmorphness){
		righthandmorphness+=multiplier*4;
	}

	if(abs(lefthandmorphness-targetlefthandmorphness)<multiplier*4){
		lefthandmorphness=targetlefthandmorphness;
		lefthandmorphstart=lefthandmorphend;
	}
	else if(lefthandmorphness>targetlefthandmorphness){
		lefthandmorphness-=multiplier*4;
	}
	else if(lefthandmorphness<targetlefthandmorphness){
		lefthandmorphness+=multiplier*4;
	}

	if(creature==rabbittype||targettailmorphness==5||targettailmorphness==0){
		if(abs(tailmorphness-targettailmorphness)<multiplier*10){
			tailmorphness=targettailmorphness;
			tailmorphstart=tailmorphend;
		}
		else if(tailmorphness>targettailmorphness){
			tailmorphness-=multiplier*10;
		}
		else if(tailmorphness<targettailmorphness){
			tailmorphness+=multiplier*10;
		}
	}

	if(creature==wolftype){
		if(abs(tailmorphness-targettailmorphness)<multiplier*4){
			tailmorphness=targettailmorphness;
			tailmorphstart=tailmorphend;
		}
		else if(tailmorphness>targettailmorphness){
			tailmorphness-=multiplier*2;
		}
		else if(tailmorphness<targettailmorphness){
			tailmorphness+=multiplier*2;
		}
	}

	if(headmorphend==3||headmorphstart==3){
		if(abs(headmorphness-targetheadmorphness)<multiplier*7){
			headmorphness=targetheadmorphness;
			headmorphstart=headmorphend;
		}
		else if(headmorphness>targetheadmorphness){
			headmorphness-=multiplier*7;
		}
		else if(headmorphness<targetheadmorphness){
			headmorphness+=multiplier*7;
		}
	}
	else if(headmorphend==5||headmorphstart==5){
		if(abs(headmorphness-targetheadmorphness)<multiplier*10){
			headmorphness=targetheadmorphness;
			headmorphstart=headmorphend;
		}
		else if(headmorphness>targetheadmorphness){
			headmorphness-=multiplier*10;
		}
		else if(headmorphness<targetheadmorphness){
			headmorphness+=multiplier*10;
		}
	}
	else{
		if(abs(headmorphness-targetheadmorphness)<multiplier*4){
			headmorphness=targetheadmorphness;
			headmorphstart=headmorphend;
		}
		else if(headmorphness>targetheadmorphness){
			headmorphness-=multiplier*4;
		}
		else if(headmorphness<targetheadmorphness){
			headmorphness+=multiplier*4;
		}
	}

	if(abs(chestmorphness-targetchestmorphness)<multiplier){
		chestmorphness=targetchestmorphness;
		chestmorphstart=chestmorphend;
	}
	else if(chestmorphness>targetchestmorphness){
		chestmorphness-=multiplier;
	}
	else if(chestmorphness<targetchestmorphness){
		chestmorphness+=multiplier;
	}

	if(dead!=2&&howactive<=typesleeping){
		if(chestmorphstart==0&&chestmorphend==0){
			chestmorphness=0;
			targetchestmorphness=1;
			chestmorphend=3;
		}
		if(chestmorphstart!=0&&chestmorphend!=0){
			chestmorphness=0;
			targetchestmorphness=1;
			chestmorphend=0;
			if(environment==snowyenvironment){
				XYZ footpoint;
				XYZ footvel;
				if(!skeleton.free)footvel=DoRotation(skeleton.specialforward[0],0,rotation,0)*-1;
				if(skeleton.free)footvel=skeleton.specialforward[0]*-1;
				if(!skeleton.free)footpoint=DoRotation((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2,0,rotation,0)*scale+coords;
				if(skeleton.free)footpoint=((skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2)*scale+coords;
				if(targetanimation==sleepanim)footvel=DoRotation(footvel,0,90,0);
				Sprite::MakeSprite(breathsprite, footpoint+footvel*.2,footvel*.4, 1,1,1, .4, .3);
			}
		}

		if(!dead&&howactive<typesleeping){
			blinkdelay-=multiplier*2;
			if(headmorphstart==0&&headmorphend==0&&blinkdelay<=0){
				headmorphness=0;
				targetheadmorphness=1;
				headmorphend=3;
				blinkdelay=(float)(abs(Random()%40))/5;
			}
			if(headmorphstart==3&&headmorphend==3){
				headmorphness=0;
				targetheadmorphness=1;
				headmorphend=0;
			}
		}
		if(!dead){
			twitchdelay-=multiplier*1.5;
			if(targetanimation!=hurtidleanim){
				if(headmorphstart==0&&headmorphend==0&&twitchdelay<=0){
					headmorphness=0;
					targetheadmorphness=1;
					headmorphend=5;
					twitchdelay=(float)(abs(Random()%40))/5;
				}
				if(headmorphstart==5&&headmorphend==5){
					headmorphness=0;
					targetheadmorphness=1;
					headmorphend=0;
				}
			}
			if((isIdle()||isCrouch())&&targetanimation!=hurtidleanim){
				twitchdelay3-=multiplier*1;
				if(Random()%2==0){
					if(righthandmorphstart==0&&righthandmorphend==0&&twitchdelay3<=0){
						righthandmorphness=0;
						targetrighthandmorphness=1;
						righthandmorphend=1;
						if(Random()%2==0)twitchdelay3=(float)(abs(Random()%40))/5;
					}
					if(righthandmorphstart==1&&righthandmorphend==1){
						righthandmorphness=0;
						targetrighthandmorphness=1;
						righthandmorphend=0;
					}
				}
				if(Random()%2==0){
					if(lefthandmorphstart==0&&lefthandmorphend==0&&twitchdelay3<=0){
						lefthandmorphness=0;
						targetlefthandmorphness=1;
						lefthandmorphend=1;
						twitchdelay3=(float)(abs(Random()%40))/5;
					}
					if(lefthandmorphstart==1&&lefthandmorphend==1){
						lefthandmorphness=0;
						targetlefthandmorphness=1;
						lefthandmorphend=0;
					}
				}
			}
		}
		if(!dead){
			if(creature==rabbittype){
				if(howactive<typesleeping)twitchdelay2-=multiplier*1.5;
				else twitchdelay2-=multiplier*0.5;
				if(howactive<=typesleeping){
					if(tailmorphstart==0&&tailmorphend==0&&twitchdelay2<=0){
						tailmorphness=0;
						targettailmorphness=1;
						tailmorphend=1;
						twitchdelay2=(float)(abs(Random()%40))/5;
					}
					if(tailmorphstart==1&&tailmorphend==1){
						tailmorphness=0;
						targettailmorphness=1;
						tailmorphend=2;
					}
					if(tailmorphstart==2&&tailmorphend==2){
						tailmorphness=0;
						targettailmorphness=1;
						tailmorphend=0;
					}
				}
			}
		}
	}
	if(creature==wolftype){
		twitchdelay2-=multiplier*1.5;
		if(tailmorphend!=0)
			if((isRun()||targetanimation==jumpupanim||targetanimation==jumpdownanim||targetanimation==backflipanim)&&!skeleton.free){
				tailmorphness=0;
				targettailmorphness=1;
				tailmorphend=0;
				twitchdelay2=.1;
			}
			if(tailmorphend!=5)
				if(targetanimation==flipanim||targetanimation==frontflipanim||targetanimation==rollanim||skeleton.free){
					tailmorphness=0;
					targettailmorphness=1;
					tailmorphend=5;
					twitchdelay2=.1;
				}
				if(twitchdelay2<=0){
					if(((tailmorphstart==0&&tailmorphend==0)||(tailmorphstart==5&&tailmorphend==5))){
						tailmorphness=0;
						targettailmorphness=1;
						tailmorphend=1;
					}
					if(tailmorphstart==1&&tailmorphend==1){
						tailmorphness=0;
						targettailmorphness=1;
						tailmorphend=2;
					}
					if(tailmorphstart==2&&tailmorphend==2){
						tailmorphness=0;
						targettailmorphness=1;
						tailmorphend=3;
					}
					if(tailmorphstart==3&&tailmorphend==3){
						tailmorphness=0;
						targettailmorphness=1;
						tailmorphend=4;
					}
					if(tailmorphstart==4&&tailmorphend==4){
						tailmorphness=0;
						targettailmorphness=1;
						tailmorphend=1;
					}
				}
	}

	if(dead!=1)unconscioustime=0;

	if(dead==1||howactive==typesleeping){
		unconscioustime+=multiplier;
		//If unconscious, close eyes and mouth
		if(righthandmorphend!=0)righthandmorphness=0;
		righthandmorphend=0;
		targetrighthandmorphness=1;

		if(lefthandmorphend!=0)lefthandmorphness=0;
		lefthandmorphend=0;
		targetlefthandmorphness=1;

		if(headmorphend!=3&&headmorphend!=5)headmorphness=0;
		headmorphend=3;
		targetheadmorphness=1;
	}


	if(howactive>typesleeping){
		XYZ headpoint;
		headpoint=coords;
		if(bloodtoggle&&!bled){
			terrain.MakeDecal(blooddecalslow,headpoint,.8,.5,0);
		}
		if(bloodtoggle&&!bled)
			for(l=0;l<terrain.patchobjectnum[whichpatchx][whichpatchz];l++){
				j=terrain.patchobjects[whichpatchx][whichpatchz][l];
				XYZ point=DoRotation(headpoint-objects.position[j],0,-objects.rotation[j],0);
				float size=.8;
				float opacity=.6;
				float rotation=0;
				objects.model[j].MakeDecal(blooddecalslow,&point,&size,&opacity,&rotation);
			}
			bled=1;
	}

	if(dead==2||howactive>typesleeping){
		//If dead, open mouth and hands
		if(righthandmorphend!=0)righthandmorphness=0;
		righthandmorphend=0;
		targetrighthandmorphness=1;

		if(lefthandmorphend!=0)lefthandmorphness=0;
		lefthandmorphend=0;
		targetlefthandmorphness=1;

		if(headmorphend!=2)headmorphness=0;
		headmorphend=2;
		targetheadmorphness=1;
	}

	if(stunned>0&&!dead&&headmorphend!=2){
		if(headmorphend!=4)headmorphness=0;
		headmorphend=4;
		targetheadmorphness=1;
	}

	if(damage>damagetolerance&&!dead){

		dead=1;
		unconscioustime=0;

		if(creature==wolftype){
		  award_bonus(0, Wolfbonus);
		}

		RagDoll(0);

		if(weaponactive!=-1){
			weapons.owner[weaponids[0]]=-1;
			weapons.velocity[weaponids[0]]=velocity*scale*-.3;
			weapons.velocity[weaponids[0]].x+=.01;
			weapons.tipvelocity[weaponids[0]]=velocity*scale;
			weapons.missed[weaponids[0]]=1;
			weapons.hitsomething[weaponids[0]]=0;
			weapons.freetime[weaponids[0]]=0;
			weapons.firstfree[weaponids[0]]=1;
			weapons.physics[weaponids[0]]=1;
			num_weapons--;
			if(num_weapons){
				weaponids[0]=weaponids[num_weapons];
				if(weaponstuck==num_weapons)weaponstuck=0;
			}
			weaponactive=-1;
			for(i=0;i<numplayers;i++){
				player[i].wentforweapon=0;
			}
		}



		if((id==0||findDistancefast(&coords,&viewer)<50)&&autoslomo){
			slomo=1;
			slomodelay=.2;
		}

		damage+=20;
	}

	//if(dead)damage-=multiplier/4;
	if(!dead)damage-=multiplier*13;
	//if(!dead&&deathbleeding<=0&&id==0)bloodloss-=multiplier*4;
	if(!dead)permanentdamage-=multiplier*4;
	if(isIdle()||isCrouch()){
		if(!dead)permanentdamage-=multiplier*4;
		//if(!dead&&deathbleeding<=0&&id==0)bloodloss-=multiplier*4;
	}
	if(damage<0)damage=0;
	if(permanentdamage<0)permanentdamage=0;
	if(superpermanentdamage<0)superpermanentdamage=0;
	if(permanentdamage<superpermanentdamage){
		permanentdamage=superpermanentdamage;
	}
	if(damage<permanentdamage){
		damage=permanentdamage;
	}
	if(dead==1&&damage<damagetolerance){
		dead=0;
		skeleton.free=1;
		damage-=20;
		for(i=0;i<skeleton.num_joints;i++){
			skeleton.joints[i].velocity=0;
		}
	}
	if(permanentdamage>damagetolerance&&dead!=2){
		DoBlood(1,255);

		if(weaponactive!=-1){
			weapons.owner[weaponids[0]]=-1;
			weapons.velocity[weaponids[0]]=velocity*scale*-.3;
			weapons.velocity[weaponids[0]].x+=.01;
			weapons.tipvelocity[weaponids[0]]=velocity*scale;
			weapons.missed[weaponids[0]]=1;
			weapons.hitsomething[weaponids[0]]=0;
			weapons.freetime[weaponids[0]]=0;
			weapons.firstfree[weaponids[0]]=1;
			weapons.physics[weaponids[0]]=1;
			num_weapons--;
			if(num_weapons){
				weaponids[0]=weaponids[num_weapons];
				if(weaponstuck==num_weapons)weaponstuck=0;
			}
			weaponactive=-1;
			for(i=0;i<numplayers;i++){
				player[i].wentforweapon=0;
			}
		}

		bled=0;

		if(!dead&&creature==wolftype){
		  award_bonus(0, Wolfbonus);
		}

		if(unconscioustime<.1&&(bonus!=spinecrusher||bonustime>1)&&(bonus!=FinishedBonus||bonustime>1)&&bloodloss<damagetolerance)
		  award_bonus(id, touchofdeath);
		if(id!=0&&unconscioustime>.1){
			numafterkill++;
		}

		dead=2;

		skeleton.free=1;

		emit_sound_at(breaksound, coords);
	}

	if(skeleton.free==1){
		if(id==0)pause_sound(whooshsound);

		if(!dead){
			//If knocked over, open hands and close mouth
			if(righthandmorphend!=0)righthandmorphness=0;
			righthandmorphend=0;
			targetrighthandmorphness=1;

			if(lefthandmorphend!=0)lefthandmorphness=0;
			lefthandmorphend=0;
			targetlefthandmorphness=1;

			if(headmorphend!=3&&headmorphend!=5&&headmorphstart!=3&&headmorphstart!=5){
				if(headmorphend!=0)headmorphness=0;
				headmorphend=0;
				targetheadmorphness=1;
			}
		}

		skeleton.DoGravity(&scale);
		float damageamount;
		damageamount=skeleton.DoConstraints(&coords,&scale)*5;
		if(damage>damagetolerance-damageamount&&!dead&&(bonus!=spinecrusher||bonustime>1)&&(bonus!=style||bonustime>1)&&(bonus!=cannon||bonustime>1))
		  award_bonus(id, deepimpact);
		DoDamage(damageamount/((protectionhigh+protectionhead+protectionlow)/3));

		average=0;
		howmany=0;
		for(j=0;j<skeleton.num_joints;j++){
			average+=skeleton.joints[j].position;
			howmany++;
		}
		average/=howmany;
		coords+=average*scale;
		for(j=0;j<skeleton.num_joints;j++){
			skeleton.joints[j].position-=average;
		}
		average/=multiplier;

		//velocity=skeleton.joints[skeleton.jointlabels[groin]].velocity*scale;
		velocity=0;
		for(i=0;i<skeleton.num_joints;i++){
			velocity+=skeleton.joints[i].velocity*scale;
		}
		velocity/=skeleton.num_joints;

		if(!isnormal(velocity.x)&&velocity.x){
			velocity=0;
		}

		float gLoc[3];
		float vel[3];
		gLoc[0]=coords.x;
		gLoc[1]=coords.y;
		gLoc[2]=coords.z;
		vel[0]=velocity.x;
		vel[1]=velocity.y;
		vel[2]=velocity.z;

		if(findLength(&average)<10&&dead&&skeleton.free){
			skeleton.longdead+=(2000-findLength(&average))*multiplier+multiplier;
			if(skeleton.longdead>2000){
				if(skeleton.longdead>6000){
					if(id==0)pause_sound(whooshsound);
					skeleton.free=3;
					DrawSkeleton();
					skeleton.free=2;
				}
				if(dead==2&&bloodloss<damagetolerance){
					XYZ headpoint;
					headpoint=(skeleton.joints[skeleton.jointlabels[head]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2*scale+coords;
					DoBlood(1,255);
					if(bloodtoggle&&!bled){
						terrain.MakeDecal(blooddecal,headpoint,.2*1.2,.5,0);
					}
					if(bloodtoggle&&!bled)
						for(l=0;l<terrain.patchobjectnum[whichpatchx][whichpatchz];l++){
							j=terrain.patchobjects[whichpatchx][whichpatchz][l];
							XYZ point=DoRotation(headpoint-objects.position[j],0,-objects.rotation[j],0);
							float size=.2*1.2;
							float opacity=.6;
							float rotation=0;
							objects.model[j].MakeDecal(blooddecal,&point,&size,&opacity,&rotation);
						}
						bled=1;
				}
				if(dead==2&&bloodloss>=damagetolerance){
					XYZ headpoint;
					headpoint=(skeleton.joints[skeleton.jointlabels[abdomen]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2*scale+coords;
					if(bleeding<=0)DoBlood(1,255);
					if(bloodtoggle&&!bled){
						terrain.MakeDecal(blooddecalslow,headpoint,.8,.5,0);
					}
					if(bloodtoggle&&!bled)
						for(l=0;l<terrain.patchobjectnum[whichpatchx][whichpatchz];l++){
							j=terrain.patchobjects[whichpatchx][whichpatchz][l];
							XYZ point=DoRotation(headpoint-objects.position[j],0,-objects.rotation[j],0);
							float size=.8;
							float opacity=.6;
							float rotation=0;
							objects.model[j].MakeDecal(blooddecalslow,&point,&size,&opacity,&rotation);
						}
						bled=1;
				}
			}
		}

		if(!dead&&crouchkeydown&&skeleton.freetime>.5&&id==0&&skeleton.free){
			bool canrecover=1;
			XYZ startpoint,endpoint,colpoint,colviewer,coltarget;
			startpoint=coords;
			endpoint=coords;
			endpoint.y-=.7;
			if(terrain.lineTerrain(startpoint,endpoint,&colpoint)!=-1)canrecover=0;
			if(velocity.y<-30)canrecover=0;
			for(i=0;i<objects.numobjects;i++){
				if(objects.type[i]!=treeleavestype&&objects.type[i]!=bushtype&&objects.type[i]!=firetype){
					colviewer=startpoint;
					coltarget=endpoint;
					if(objects.model[i].LineCheck(&colviewer,&coltarget,&colpoint,&objects.position[i],&objects.rotation[i])!=-1)canrecover=0;
				}
			}
			if(canrecover){
				skeleton.free=0;
				XYZ middle;
				middle=0;

				terrainnormal=skeleton.joints[skeleton.jointlabels[groin]].position-skeleton.joints[skeleton.jointlabels[abdomen]].position;
				if(skeleton.joints[skeleton.jointlabels[groin]].locked&&skeleton.joints[skeleton.jointlabels[abdomen]].locked){
					terrainnormal=skeleton.joints[skeleton.jointlabels[groin]].position-skeleton.joints[skeleton.jointlabels[abdomen]].position;
					middle=(skeleton.joints[skeleton.jointlabels[groin]].position+skeleton.joints[skeleton.jointlabels[abdomen]].position)/2;
				}
				if(skeleton.joints[skeleton.jointlabels[abdomen]].locked&&skeleton.joints[skeleton.jointlabels[neck]].locked){
					terrainnormal=skeleton.joints[skeleton.jointlabels[abdomen]].position-skeleton.joints[skeleton.jointlabels[neck]].position;
					middle=(skeleton.joints[skeleton.jointlabels[neck]].position+skeleton.joints[skeleton.jointlabels[abdomen]].position)/2;
				}
				if(skeleton.joints[skeleton.jointlabels[groin]].locked&&skeleton.joints[skeleton.jointlabels[neck]].locked){
					terrainnormal=skeleton.joints[skeleton.jointlabels[groin]].position-skeleton.joints[skeleton.jointlabels[neck]].position;
					middle=(skeleton.joints[skeleton.jointlabels[groin]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2;
				}
				Normalise(&terrainnormal);

				targetrotation=-asin(0-terrainnormal.x);
				targetrotation*=360/6.28;
				if(terrainnormal.z<0)targetrotation=180-targetrotation;
				rotation=targetrotation;

				targetframe=0;
				//	targetframe=2;
				targetanimation=flipanim;
				crouchtogglekeydown=1;
				target=0;
				tilt2=0;
				targettilt2=0;

				currentanimation=tempanim;
				currentframe=0;
				target=0;
				//tilt2=targettilt2;

				//if(middle.y>0)targetoffset.y=middle.y+1;

				for(i=0;i<skeleton.num_joints;i++){
					tempanimation.position[i][0]=skeleton.joints[i].position;
					tempanimation.position[i][0]=DoRotation(tempanimation.position[i][0],0,-rotation,0);
				}
			}
		}

		if(findLength(&average)<10&&!dead&&skeleton.free){
			skeleton.longdead+=(2000-findLength(&average))*multiplier+multiplier;
			if(skeleton.longdead>(damage+500)*1.5){
				if(id==0)pause_sound(whooshsound);
				skeleton.free=0;
				velocity=0;
				XYZ middle;
				middle=0;

				terrainnormal=skeleton.joints[skeleton.jointlabels[groin]].position-skeleton.joints[skeleton.jointlabels[abdomen]].position;
				if(skeleton.joints[skeleton.jointlabels[groin]].locked&&skeleton.joints[skeleton.jointlabels[abdomen]].locked){
					terrainnormal=skeleton.joints[skeleton.jointlabels[groin]].position-skeleton.joints[skeleton.jointlabels[abdomen]].position;
					middle=(skeleton.joints[skeleton.jointlabels[groin]].position+skeleton.joints[skeleton.jointlabels[abdomen]].position)/2;
				}
				if(skeleton.joints[skeleton.jointlabels[abdomen]].locked&&skeleton.joints[skeleton.jointlabels[neck]].locked){
					terrainnormal=skeleton.joints[skeleton.jointlabels[abdomen]].position-skeleton.joints[skeleton.jointlabels[neck]].position;
					middle=(skeleton.joints[skeleton.jointlabels[neck]].position+skeleton.joints[skeleton.jointlabels[abdomen]].position)/2;
				}
				if(skeleton.joints[skeleton.jointlabels[groin]].locked&&skeleton.joints[skeleton.jointlabels[neck]].locked){
					terrainnormal=skeleton.joints[skeleton.jointlabels[groin]].position-skeleton.joints[skeleton.jointlabels[neck]].position;
					middle=(skeleton.joints[skeleton.jointlabels[groin]].position+skeleton.joints[skeleton.jointlabels[neck]].position)/2;
				}
				Normalise(&terrainnormal);

				targetrotation=-asin(0-terrainnormal.x);
				targetrotation*=360/6.28;
				if(terrainnormal.z<0)targetrotation=180-targetrotation;
				rotation=targetrotation;

				/*if(onterrain){
				terrainnormal=terrain.getNormal(coords.x,coords.z);
				targettilt2=asin(terrainnormal.y)*180/3.14*-1;
				}
				else*/

				/*XYZ otherterrainnormal;
				otherterrainnormal=terrain.getNormal(coords.x,coords.y);
				otherterrainnormal.y=fast_sqrt(otherterrainnormal.x*otherterrainnormal.x+otherterrainnormal.z*otherterrainnormal.z)*-1;
				if(abs(terrainnormal.y)<abs(otherterrainnormal.y))terrainnormal.y=fast_sqrt(otherterrainnormal.x*otherterrainnormal.x+otherterrainnormal.z*otherterrainnormal.z)*-1;
				targettilt2=asin(otherterrainnormal.y)*180/3.14;
				*/

				targettilt2=asin(terrainnormal.y)*180/3.14*-1;



				if(skeleton.forward.y<0){
					targetanimation=getupfrombackanim;
					targetframe=0;
					targettilt2=0;
				}
				if(skeleton.forward.y>-.3){
					targetanimation=getupfromfrontanim;
					rotation+=180;
					targetrotation+=180;
					targettilt2*=-1;
					targetframe=0;
					targettilt2=0;
				}

				if((Random()%8==0&&id!=0&&creature==rabbittype)||(Random()%2==0&&id!=0&&creature==wolftype)||(id==0&&crouchkeydown&&(forwardkeydown||backkeydown||leftkeydown||rightkeydown))){
					targetanimation=rollanim;
					targetrotation=lookrotation;
					if(id==0){
						if(rightkeydown){
							targetrotation-=90;
							if(forwardkeydown)targetrotation+=45;
							if(backkeydown)targetrotation-=45;
						}
						if(leftkeydown){
							targetrotation+=90;
							if(forwardkeydown)targetrotation-=45;
							if(backkeydown)targetrotation+=45;
						}
						if(backkeydown){
							if ( !leftkeydown&&!rightkeydown)
								targetrotation+=180;
						}
						targetrotation+=180;
					}
				}

				if(abs(targettilt2)>50)targettilt2=0;
				currentanimation=tempanim;
				currentframe=0;
				target=0;
				tilt2=targettilt2;

				if(middle.y>0&&targetanimation!=rollanim)targetoffset.y=middle.y+1;

				for(i=0;i<skeleton.num_joints;i++){
					tempanimation.position[i][0]=skeleton.joints[i].position;
					tempanimation.position[i][0]=DoRotation(tempanimation.position[i][0],0,-rotation,0);
				}
			}
		}

		bool hasstaff;
		hasstaff=0;
		if(num_weapons>0)if(weapons.type[0]==staff)hasstaff=1;
		if(!skeleton.freefall&&freefall&&((jumpkeydown&&jumpkeydowntime<.2)||(hasstaff&&rabbitkickragdoll))&&!dead){
			if(velocity.y>-30){
				XYZ tempvelocity;
				tempvelocity=velocity;
				Normalise(&tempvelocity);
				targetrotation=-asin(0-tempvelocity.x);
				targetrotation*=360/6.28;
				if(velocity.z<0)targetrotation=180-targetrotation;
				//targetrotation+=180;

				skeleton.free=0;
				if(dotproduct(&skeleton.forward,&tempvelocity)<0){
					targetanimation=rollanim;
					targetframe=2;
				}
				else{
					targetanimation=backhandspringanim;
					targetrotation+=180;
					targetframe=6;
				}
				target=0;

				emit_sound_at(movewhooshsound, coords, 128.);

				currentanimation=targetanimation;
				currentframe=targetframe-1;
				target=0;

				velocity=0;

				rotation=targetrotation;
				tilt=0;
				targettilt=0;
				tilt2=0;
				targettilt2=0;
			}
		}
		if(skeleton.freefall==0)freefall=0;

		if(!isnormal(velocity.x)&&velocity.x){
			int xy=1;
		}
	}

	if(aitype!=passivetype||skeleton.free==1)
		if(findLengthfast(&velocity)>.1)
			for(i=0;i<objects.numobjects;i++){
				if(objects.type[i]==firetype)
					if(findDistancefastflat(&coords,&objects.position[i])<objects.scale[i]*objects.scale[i]*12&&findDistancefast(&coords,&objects.position[i])<objects.scale[i]*objects.scale[i]*49){
						if(onfire){
							if(!objects.onfire[i]){
								emit_sound_at(firestartsound, objects.position[i]);
							}
							objects.onfire[i]=1;
						}
						if(!onfire){
							if(objects.onfire[i]){
								CatchFire();
							}
						}
					}
					if(objects.type[i]==bushtype)
						if(findDistancefastflat(&coords,&objects.position[i])<objects.scale[i]*objects.scale[i]*12&&findDistancefast(&coords,&objects.position[i])<objects.scale[i]*objects.scale[i]*49){
							if(onfire){
								if(!objects.onfire[i]){
									emit_sound_at(firestartsound, objects.position[i]);
								}
								objects.onfire[i]=1;
							}

							if(!onfire){
								if(objects.onfire[i]){
									CatchFire();
								}
							}
							if(objects.messedwith[i]<=0){
								XYZ tempvel;
								XYZ pos;

								emit_sound_at(bushrustle, coords, 40*findLength(&velocity));

								if(id==0){
									envsound[numenvsounds]=coords;
									envsoundvol[numenvsounds]=4*findLength(&velocity);
									envsoundlife[numenvsounds]=.4;
									numenvsounds++;
								}

								int howmany;
								if(environment==grassyenvironment)howmany=findLength(&velocity)*4;
								if(environment==snowyenvironment)howmany=findLength(&velocity)*2;
								if(detail==2)
									if(environment!=desertenvironment)
										for(j=0;j<howmany;j++){
											tempvel.x=float(abs(Random()%100)-50)/20;
											tempvel.y=float(abs(Random()%100)-50)/20;
											tempvel.z=float(abs(Random()%100)-50)/20;
											pos=coords;
											pos.y+=1;
											pos.x+=float(abs(Random()%100)-50)/200;
											pos.y+=float(abs(Random()%100)-50)/200;
											pos.z+=float(abs(Random()%100)-50)/200;
											Sprite::MakeSprite(splintersprite, pos,tempvel*.5+velocity*float(abs(Random()%100))/100, 165/255+float(abs(Random()%100)-50)/400,0,0, .2+float(abs(Random()%100)-50)/1300, 1);
											Sprite::setLastSpriteSpecial(1);
										}
										howmany=findLength(&velocity)*4;
										if(detail==2)
											if(environment==snowyenvironment)
												for(j=0;j<howmany;j++){
													tempvel.x=float(abs(Random()%100)-50)/20;
													tempvel.y=float(abs(Random()%100)-50)/20;
													tempvel.z=float(abs(Random()%100)-50)/20;
													pos=coords;
													pos.y+=1;
													pos.x+=float(abs(Random()%100)-50)/200;
													pos.y+=float(abs(Random()%100)-50)/200;
													pos.z+=float(abs(Random()%100)-50)/200;
													Sprite::MakeSprite(splintersprite, pos,tempvel*.3+velocity*float(abs(Random()%100))/100/2, 1,1,1, .1, 1);
													Sprite::setLastSpriteSpecial(2);
												}
							}
							objects.rotx[i]+=velocity.x*multiplier*6;
							objects.roty[i]+=velocity.z*multiplier*6;
							objects.messedwith[i]=.5;
						}
						XYZ tempcoord;
						if(objects.type[i]==treeleavestype&&environment!=desertenvironment){
							if(objects.rotation2[i]==0)tempcoord=coords;
							else{
								tempcoord=coords-objects.position[i];
								tempcoord=DoRotation(tempcoord,0,-objects.rotation[i],0);
								tempcoord=DoRotation(tempcoord,-objects.rotation2[i],0,0);
								tempcoord+=objects.position[i];
							}
							if(findDistancefastflat(&tempcoord,&objects.position[i])<objects.scale[i]*objects.scale[i]*8&&findDistancefast(&tempcoord,&objects.position[i])<objects.scale[i]*objects.scale[i]*300&&tempcoord.y>objects.position[i].y+3*objects.scale[i]){
								if(objects.messedwith[i]<=0){
									XYZ tempvel;
									XYZ pos;

									emit_sound_at(bushrustle, coords, 40*findLength(&velocity));

									if(id==0){
										envsound[numenvsounds]=coords;
										envsoundvol[numenvsounds]=4*findLength(&velocity);
										envsoundlife[numenvsounds]=.4;
										numenvsounds++;
									}

									int howmany;
									if(environment==grassyenvironment)howmany=findLength(&velocity)*4;
									if(environment==snowyenvironment)howmany=findLength(&velocity)*2;
									if(detail==2)
										if(environment!=desertenvironment)
											for(j=0;j<howmany;j++){
												tempvel.x=float(abs(Random()%100)-50)/20;
												tempvel.y=float(abs(Random()%100)-50)/20;
												tempvel.z=float(abs(Random()%100)-50)/20;
												pos=coords;
												pos+=velocity*.1;
												pos.y+=1;
												pos.x+=float(abs(Random()%100)-50)/150;
												pos.y+=float(abs(Random()%100)-50)/150;
												pos.z+=float(abs(Random()%100)-50)/150;
												Sprite::MakeSprite(splintersprite, pos,tempvel*.5+velocity*float(abs(Random()%100))/100, 165/255+float(abs(Random()%100)-50)/400,0,0, .2+float(abs(Random()%100)-50)/1300, 1);
												Sprite::setLastSpriteSpecial(1);
											}
											howmany=findLength(&velocity)*4;
											if(detail==2)
												if(environment==snowyenvironment)
													for(j=0;j<howmany;j++){
														tempvel.x=float(abs(Random()%100)-50)/20;
														tempvel.y=float(abs(Random()%100)-50)/20;
														tempvel.z=float(abs(Random()%100)-50)/20;
														pos=coords;
														pos+=velocity*.1;
														pos.y+=1;
														pos.x+=float(abs(Random()%100)-50)/150;
														pos.y+=float(abs(Random()%100)-50)/150;
														pos.z+=float(abs(Random()%100)-50)/150;
														Sprite::MakeSprite(splintersprite, pos,tempvel*.3+velocity*float(abs(Random()%100))/100/2, 1,1,1, .1, 1);
														Sprite::setLastSpriteSpecial(2);
													}
								}
								objects.messedwith[i]=.5;
							}
						}
			}

			if(!skeleton.free){
				bool play;
				play=0;
				if((stunned>0||surprised>0)&&numplayers>2&&aitype!=passivetype)play=1;
				if(hasvictim)
					if(aitype!=passivetype&&victim->skeleton.free&&!victim->dead)play=1;
				if(tutoriallevel==1&&id!=0)play=0;
				if(play&&aitype!=playercontrolled){
					int whichsound=-1;
					i=abs(Random()%4);
					if(speechdelay<=0){
						if(creature==rabbittype){
							if(i==0)whichsound=rabbitchitter;
							if(i==1)whichsound=rabbitchitter2;
						}
						if(creature==wolftype){
							if(i==0)whichsound=growlsound;
							if(i==1)whichsound=growl2sound;
						}
					}
					speechdelay=.3;

					if(whichsound!=-1){
						emit_sound_at(whichsound, coords);
					}
				}

				if(targetanimation==staggerbackhighanim)staggerdelay=1;
				if(targetanimation==staggerbackhardanim)staggerdelay=1;
				staggerdelay-=multiplier;
				if(targetanimation!=crouchstabanim&&targetanimation!=swordgroundstabanim&&targetanimation!=staffgroundsmashanim)hasvictim=1;
				if(velocity.y<-30&&targetanimation==jumpdownanim)RagDoll(0);
				if(currentanimation!=getIdle()&&wasIdle()&&targetanimation!=getIdle()&&isIdle()){
					targetanimation=getIdle();
					targetframe=0;
					target=0;
				}
				weaponmissdelay-=multiplier;
				highreversaldelay-=multiplier;
				lowreversaldelay-=multiplier;
				lastcollide-=multiplier;
				skiddelay-=multiplier;
				if(!isnormal(velocity.x)&&velocity.x){
					velocity=0;
				}
				if(!isnormal(targettilt)&&targettilt){
					targettilt=0;
				}
				if(!isnormal(targettilt2)&&targettilt2){
					targettilt2=0;
				}
				if(!isnormal(targetrotation)&&targetrotation){
					targetrotation=0;
				}

				if(targetanimation==bounceidleanim||targetanimation==wolfidle||targetanimation==walkanim||targetanimation==drawrightanim||targetanimation==crouchdrawrightanim||targetanimation==drawleftanim||targetanimation==fightidleanim||targetanimation==fightsidestep||targetanimation==hanganim||isCrouch()||targetanimation==backhandspringanim){
					//open hands and close mouth
					if(righthandmorphend!=0&&righthandmorphness==targetrighthandmorphness){
						righthandmorphness=0;
						righthandmorphend=0;
						targetrighthandmorphness=1;
					}

					if(lefthandmorphend!=0&&lefthandmorphness==targetlefthandmorphness){
						lefthandmorphness=0;
						lefthandmorphend=0;
						targetlefthandmorphness=1;
					}

					if(headmorphend!=3&&headmorphend!=5&&headmorphstart!=3&&headmorphstart!=5&&headmorphend!=0&&headmorphness==targetheadmorphness){
						headmorphness=0;
						headmorphend=0;
						targetheadmorphness=1;
					}
				}

				if(targetanimation==rollanim||targetanimation==dodgebackanim||targetanimation==removeknifeanim||targetanimation==knifefightidleanim||targetanimation==swordfightidleanim||targetanimation==blockhighleftstrikeanim||targetanimation==crouchremoveknifeanim||targetanimation==sneakanim||targetanimation==sweepanim||targetanimation==spinkickreversedanim||targetanimation==jumpdownanim||isWallJump()||isFlip()||targetanimation==climbanim||isRun()||targetanimation==getupfrombackanim||targetanimation==getupfromfrontanim){
					//open hands and mouth
					if(righthandmorphend!=0&&righthandmorphness==targetrighthandmorphness){
						righthandmorphness=0;
						righthandmorphend=0;
						targetrighthandmorphness=1;
					}

					if(lefthandmorphend!=0&&lefthandmorphness==targetlefthandmorphness){
						lefthandmorphness=0;
						lefthandmorphend=0;
						targetlefthandmorphness=1;
					}

					if(headmorphend!=1&&headmorphness==targetheadmorphness){
						headmorphness=0;
						headmorphend=1;
						targetheadmorphness=1;
					}
				}

				if(targetanimation==jumpupanim||targetanimation==crouchstabanim||targetanimation==swordgroundstabanim||targetanimation==swordfightidlebothanim||targetanimation==blockhighleftanim||targetanimation==blockhighleftanim){
					//close hands and mouth
					if(righthandmorphend!=1&&righthandmorphness==targetrighthandmorphness){
						righthandmorphness=0;
						righthandmorphend=1;
						targetrighthandmorphness=1;
					}

					if(lefthandmorphend!=1&&lefthandmorphness==targetlefthandmorphness){
						lefthandmorphness=0;
						lefthandmorphend=1;
						targetlefthandmorphness=1;
					}

					if(headmorphend!=0&&headmorphness==targetheadmorphness){
						headmorphness=0;
						headmorphend=0;
						targetheadmorphness=1;
					}
				}

				if(targetanimation==spinkickanim||targetanimation==staffspinhitreversalanim||targetanimation==staffspinhitreversedanim||targetanimation==staffhitreversalanim||targetanimation==staffhitreversedanim||targetanimation==hurtidleanim||targetanimation==winduppunchanim||targetanimation==swordslashreversalanim||targetanimation==swordslashreversedanim||targetanimation==knifeslashreversalanim||targetanimation==knifeslashreversedanim||targetanimation==knifethrowanim||targetanimation==knifefollowanim||targetanimation==knifefollowedanim||targetanimation==killanim||targetanimation==dropkickanim||targetanimation==upunchanim||targetanimation==knifeslashstartanim||targetanimation==swordslashanim||targetanimation==staffhitanim||targetanimation==staffspinhitanim||targetanimation==staffgroundsmashanim||targetanimation==spinkickreversalanim||targetanimation==sweepreversalanim||targetanimation==lowkickanim||targetanimation==sweepreversedanim||targetanimation==rabbitkickreversalanim||targetanimation==rabbitkickreversedanim||targetanimation==jumpreversalanim||targetanimation==jumpreversedanim){
					//close hands and yell
					if(righthandmorphend!=1&&righthandmorphness==targetrighthandmorphness){
						righthandmorphness=0;
						righthandmorphend=1;
						targetrighthandmorphness=1;
					}

					if(lefthandmorphend!=1&&lefthandmorphness==targetlefthandmorphness){
						lefthandmorphness=0;
						lefthandmorphend=1;
						targetlefthandmorphness=1;
					}

					if(headmorphend!=2&&headmorphness==targetheadmorphness){
						headmorphness=1;
						headmorphend=2;
						targetheadmorphness=1;
					}
				}
				/*
				if(speechdelay>.25){
				if(headmorphend!=2)headmorphness=0;
				headmorphend=2;
				targetheadmorphness=1;
				}
				*/
				bool behind;
				behind=0;
				if(hasvictim){
					if(victim!=this&&!victim->dead&&victim->aitype!=passivetype&&victim->aitype!=searchtype&&aitype!=passivetype&&aitype!=searchtype&&victim->id<numplayers&&aitype!=passivetype){
						behind=(normaldotproduct(facing,coords-victim->coords)>0);
					}
				}

				if(!dead&&targetanimation!=hurtidleanim)
					if(behind||targetanimation==killanim||targetanimation==knifethrowanim||targetanimation==knifefollowanim||targetanimation==spinkickreversalanim||targetanimation==rabbitkickreversedanim||targetanimation==jumpreversedanim){
						if(headmorphend!=4||headmorphness==targetheadmorphness){
							headmorphend=4;
							//headmorphness=1;
							targetheadmorphness=1;
						}
					}

					if(weaponactive!=-1){
						if(weapons.type[weaponids[weaponactive]]!=staff){
							righthandmorphstart=1;
							righthandmorphend=1;
						}
						if(weapons.type[weaponids[weaponactive]]==staff){
							righthandmorphstart=2;
							righthandmorphend=2;
						}
						targetrighthandmorphness=1;
					}

					terrainnormal=terrain.getNormal(coords.x,coords.z);

					if(animation[targetanimation].attack!=reversal){
						if(!isnormal(coords.x))
							coords=oldcoords;
						oldcoords=coords;
					}

					flatfacing=0;
					flatfacing.z=1;

					flatfacing=DoRotation(flatfacing,0,rotation,0);
					facing=flatfacing;
					ReflectVector(&facing,terrainnormal);
					Normalise(&facing);

					if(isRun()||targetanimation==sneakanim||targetanimation==rollanim||targetanimation==walkanim){
						if(onterrain)targettilt2=-facing.y*20;
						else targettilt2=0;
					}
					onterrain=0;
					if(!isRun()&&!animation[targetanimation].attack&&targetanimation!=getupfromfrontanim&&targetanimation!=getupfrombackanim&&targetanimation!=sneakanim)targettilt2=0;
					if(targetanimation==jumpupanim||targetanimation==jumpdownanim||isFlip()){
						flatvelocity=velocity;
						flatvelocity.y=0;
						flatvelspeed=findLength(&flatvelocity);
						targettilt=flatvelspeed*fast_sqrt(abs(velocity.y)*.7)*normaldotproduct(DoRotation(flatfacing,0,-90,0),flatvelocity);
						targettilt2=flatvelspeed*fast_sqrt(abs(velocity.y)*.7)*normaldotproduct(flatfacing,flatvelocity);
						if(velocity.y<0)targettilt2*=-1;
						if(velocity.y<0)targettilt*=-1;
						if(targettilt>25)targettilt=25;
						if(targettilt<-25)targettilt=-25;
					}

					if(targettilt2>45)targettilt2=45;
					if(targettilt2<-45)targettilt2=-45;
					if(abs(tilt2-targettilt2)<multiplier*400)tilt2=targettilt2;
					else if(tilt2>targettilt2){
						tilt2-=multiplier*400;
					}
					else if(tilt2<targettilt2){
						tilt2+=multiplier*400;
					}
					if(!animation[targetanimation].attack&&targetanimation!=getupfrombackanim&&targetanimation!=getupfromfrontanim){
						if(tilt2>25)tilt2=25;
						if(tilt2<-25)tilt2=-25;
					}

					if(!isnormal(targettilt)&&targettilt){
						targettilt=0;
					}
					if(!isnormal(targettilt2)&&targettilt2){
						targettilt2=0;
					}

					//Running velocity
					//if(!creature==wolftype||targetanimation==rabbitkickanim)
					if(targetanimation==rabbittackleanim){
						velocity+=facing*multiplier*speed*700*scale;
						velspeed=findLength(&velocity);
						if(velspeed>speed*65*scale){
							velocity/=velspeed;
							velspeed=speed*65*scale;
							velocity*=velspeed;
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=flatfacing*velspeed;
					}
					if(targetanimation!=rabbitrunninganim&&targetanimation!=wolfrunninganim){
						if(isRun()||targetanimation==rabbitkickanim){
							velocity+=facing*multiplier*speed*700*scale;
							velspeed=findLength(&velocity);
							if(velspeed>speed*45*scale){
								velocity/=velspeed;
								velspeed=speed*45*scale;
								velocity*=velspeed;
							}
							velocity.y+=gravity*multiplier*20;
							ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
							velspeed=findLength(&velocity);
							if(velspeed<speed*30*scale)velspeed=speed*30*scale;
							velocity=flatfacing*velspeed;
						}
					}
					else if(isRun()){
						velocity+=facing*multiplier*speed*700*scale;
						velspeed=findLength(&velocity);
						if(creature==rabbittype){
							if(velspeed>speed*55*scale){
								velocity/=velspeed;
								velspeed=speed*55*scale;
								velocity*=velspeed;
							}
						}
						if(creature==wolftype){
							if(velspeed>speed*75*scale){
								velocity/=velspeed;
								velspeed=speed*75*scale;
								velocity*=velspeed;
							}
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=flatfacing*velspeed;
					}

					if(targetanimation==rollanim&&animation[targetanimation].label[targetframe]!=6){
						velocity+=facing*multiplier*speed*700*scale;
						velspeed=findLength(&velocity);
						if(velspeed>speed*45*scale){
							velocity/=velspeed;
							velspeed=speed*45*scale;
							velocity*=velspeed;
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=flatfacing*velspeed;
					}


					/*if(currentanimation==rollanim&&(isCrouch()||isIdle())){
					velocity+=facing*multiplier*speed*700*scale;
					velspeed=findLength(&velocity);
					if(velspeed>speed*25*scale){
					velocity/=velspeed;
					velspeed=speed*25*scale;
					velocity*=velspeed;
					}
					velocity.y+=gravity*multiplier*20;
					ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
					velspeed=findLength(&velocity);
					velocity=flatfacing*velspeed;
					}*/

					if(targetanimation==sneakanim||targetanimation==walkanim){
						velocity+=facing*multiplier*speed*700*scale;
						velspeed=findLength(&velocity);
						if(velspeed>speed*12*scale){
							velocity/=velspeed;
							velspeed=speed*12*scale;
							velocity*=velspeed;
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=flatfacing*velspeed;
					}

					if((targetanimation==fightidleanim||targetanimation==knifefightidleanim)&&(currentanimation==bounceidleanim||currentanimation==hurtidleanim)){
						velocity+=facing*multiplier*speed*700*scale;
						velspeed=findLength(&velocity);
						if(velspeed>speed*2*scale){
							velocity/=velspeed;
							velspeed=speed*2*scale;
							velocity*=velspeed;
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=flatfacing*velspeed;
					}


					if((targetanimation==bounceidleanim||currentanimation==hurtidleanim)&&(currentanimation==fightidleanim||currentanimation==knifefightidleanim)){
						velocity-=facing*multiplier*speed*700*scale;
						velspeed=findLength(&velocity);
						if(velspeed>speed*2*scale){
							velocity/=velspeed;
							velspeed=speed*2*scale;
							velocity*=velspeed;
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=flatfacing*velspeed*-1;
					}

					if(targetanimation==fightsidestep){
						velocity+=DoRotation(facing*multiplier*speed*700*scale,0,-90,0);
						velspeed=findLength(&velocity);
						if(velspeed>speed*12*scale){
							velocity/=velspeed;
							velspeed=speed*12*scale;
							velocity*=velspeed;
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=DoRotation(flatfacing*velspeed,0,-90,0);
					}

					if(targetanimation==staggerbackhighanim){
						coords-=facing*multiplier*speed*16*scale;
						velocity=0;
					}
					if(targetanimation==staggerbackhardanim&&animation[staggerbackhardanim].label[targetframe]!=6){
						coords-=facing*multiplier*speed*20*scale;
						velocity=0;
					}

					if(targetanimation==backhandspringanim){
						//coords-=facing*multiplier*50*scale;
						velocity+=facing*multiplier*speed*700*scale*-1;
						velspeed=findLength(&velocity);
						if(velspeed>speed*50*scale){
							velocity/=velspeed;
							velspeed=speed*50*scale;
							velocity*=velspeed;
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=flatfacing*velspeed*-1;
					}
					if(targetanimation==dodgebackanim){
						//coords-=facing*multiplier*50*scale;
						velocity+=facing*multiplier*speed*700*scale*-1;
						velspeed=findLength(&velocity);
						if(velspeed>speed*60*scale){
							velocity/=velspeed;
							velspeed=speed*60*scale;
							velocity*=velspeed;
						}
						velocity.y+=gravity*multiplier*20;
						ReflectVector(&velocity,terrain.getNormal(coords.x,coords.z));
						velspeed=findLength(&velocity);
						velocity=flatfacing*velspeed*-1;
					}

					if(targetanimation==jumpupanim||targetanimation==jumpdownanim||isFlip()){
						velspeed=findLength(&velocity);
					}


					if(targetanimation==jumpupanim||targetanimation==jumpdownanim||isFlip()){
						velocity.y+=gravity*multiplier;
					}

					if(targetanimation!=climbanim&&targetanimation!=hanganim&&!isWallJump())coords+=velocity*multiplier;

					if(coords.y<terrain.getHeight(coords.x,coords.z)&&(targetanimation==jumpdownanim||targetanimation==jumpupanim||isFlip())){
						if(isFlip()&&animation[targetanimation].label[targetframe]==7)RagDoll(0);

						if(targetanimation==jumpupanim){jumppower=-4;targetanimation=getIdle();}
						target=0;
						targetframe=0;
						onterrain=1;

						if(id==0){
							pause_sound(whooshsound);
							OPENAL_SetVolume(channels[whooshsound], 0);
						}

						if(targetanimation==jumpdownanim||isFlip()){
							if(isFlip())jumppower=-4;
							targetanimation=getLanding();
							emit_sound_at(landsound, coords, 128.);

							if(id==0){
								envsound[numenvsounds]=coords;
								envsoundvol[numenvsounds]=16;
								envsoundlife[numenvsounds]=.4;
								numenvsounds++;
							}
						}
					}

					if(targetanimation!=jumpupanim&&targetanimation!=jumpdownanim&&!isFlip()&&targetanimation!=climbanim&&targetanimation!=hanganim&&!isWallJump())coords.y+=gravity*multiplier*2;
					if(targetanimation!=jumpupanim&&targetanimation!=jumpdownanim&&!isFlip()&&coords.y<terrain.getHeight(coords.x,coords.z)){
						coords.y=terrain.getHeight(coords.x,coords.z);
						onterrain=1;
					}


					if(isIdle()||targetanimation==drawrightanim||targetanimation==drawleftanim||targetanimation==crouchdrawrightanim||targetanimation==crouchstabanim||targetanimation==swordgroundstabanim||isStop()||targetanimation==removeknifeanim||targetanimation==crouchremoveknifeanim||isLanding()||isCrouch()||animation[targetanimation].attack||(targetanimation==rollanim&&animation[targetanimation].label[targetframe]==6)){
						velspeed=findLength(&velocity);
						velocity.y=0;
						if(velspeed<multiplier*300*scale){
							velocity=0;
						} else velocity-=velocity/velspeed*multiplier*300*scale;
						if(velspeed>5&&(isLanding()||isLandhard())){
							skiddingdelay+=multiplier;
							if(skiddelay<=0){
								FootLand(0,.5);
								FootLand(1,.5);
								skiddelay=.02;
							}
						}
						else skiddingdelay=0;
					}

					if(isLandhard()){
						velspeed=findLength(&velocity);
						velocity.y=0;
						if(velspeed<multiplier*600*scale){
							velocity=0;
						} else velocity-=velocity/velspeed*multiplier*600*scale;
						velocity=0;
						if(velspeed>5&&(isLanding()||isLandhard())){
							skiddingdelay+=multiplier;
							if(skiddelay<=0){
								FootLand(0,.5);
								FootLand(1,.5);
								skiddelay=.02;
							}
						}
						else skiddingdelay=0;
					}

					if(skiddingdelay<0)skiddingdelay+=multiplier;
					if(skiddingdelay>.02&&!forwardkeydown&&!backkeydown&&!leftkeydown&&!rightkeydown&&!jumpkeydown&&isLanding()&&!landhard){
						skiddingdelay=-1;
						if(!onterrain||environment==grassyenvironment){
							emit_sound_at(skidsound, coords, 128*velspeed/10);
						}
						else {
							emit_sound_at(snowskidsound, coords, 128*velspeed/10);
						}
					}

					if(animation[targetanimation].attack==normalattack&&targetanimation!=rabbitkickanim&&!victim->skeleton.free){
						terrainnormal=victim->coords-coords;
						Normalise(&terrainnormal);
						targetrotation=-asin(0-terrainnormal.x);
						targetrotation*=360/6.28;
						if(terrainnormal.z<0)targetrotation=180-targetrotation;
						targettilt2=-asin(terrainnormal.y)*360/6.28;//*-70;
					}

					if(animation[targetanimation].attack==reversal&&targetanimation!=rabbittacklinganim){
						targetrotation=victim->targetrotation;
					}
					if(targetanimation==rabbittacklinganim){
						coords=victim->coords;
					}
			}
			skeleton.oldfree=skeleton.free;

			XYZ midterrain;
			midterrain=0;
			midterrain.x=terrain.size*terrain.scale/2;
			midterrain.z=terrain.size*terrain.scale/2;
			if(findDistancefastflat(&coords,&midterrain)>(terrain.size*terrain.scale/2-viewdistance)*(terrain.size*terrain.scale/2-viewdistance)){
				XYZ tempposit;
				tempposit=coords-midterrain;
				tempposit.y=0;
				Normalise(&tempposit);
				tempposit*=(terrain.size*terrain.scale/2-viewdistance);
				coords.x=tempposit.x+midterrain.x;
				coords.z=tempposit.z+midterrain.z;
			}
}

int Person::DrawSkeleton(){
	int oldplayerdetail;
	if((frustum.SphereInFrustum(coords.x,coords.y+scale*3,coords.z,scale*8)&&findDistancefast(&viewer,&coords)<viewdistance*viewdistance)||skeleton.free==3){
		if(onterrain&&(isIdle()||isCrouch()||wasIdle()||wasCrouch())&&!skeleton.free){
			calcrot=1;
		}

		if(headless){
			headmorphness=0;
			headmorphstart=6;
			headmorphend=6;
		}

		glAlphaFunc(GL_GREATER, 0.0001);
		XYZ terrainlight;
		float terrainheight;
		float distance;
		if(!isnormal(rotation))rotation=0;
		if(!isnormal(tilt))tilt=0;
		if(!isnormal(tilt2))tilt2=0;
		oldplayerdetail=playerdetail;
		playerdetail=0;
		if(findDistancefast(&viewer,&coords)<viewdistance*viewdistance/32&&detail==2){
			playerdetail=1;
		}
		if(findDistancefast(&viewer,&coords)<viewdistance*viewdistance/128&&detail==1){
			playerdetail=1;
		}
		if(findDistancefast(&viewer,&coords)<viewdistance*viewdistance/256&&(detail!=1&&detail!=2)){
			playerdetail=1;
		}
		if(id==0)
			playerdetail=1;
		if(playerdetail!=oldplayerdetail) {
			updatedelay=0;
			normalsupdatedelay=0;
		}
		static float updatedelaychange;
		static float morphness;
		static float framemult;
		if(calcrot){
			skeleton.FindForwards();
			if(howactive==typesittingwall){
				skeleton.specialforward[1]=0;
				skeleton.specialforward[1].z=1;
			}
		}
		static XYZ mid;
		static float M[16];
		static int i,j,k;
		static int weaponattachmuscle;
		static int weaponrotatemuscle,weaponrotatemuscle2;
		static XYZ weaponpoint;
		static int start,endthing;
		if((dead!=2||skeleton.free!=2)&&updatedelay<=0){
			if(!isSleeping()&&!isSitting()){
				if(onterrain&&((isIdle()||isCrouch()||isLanding()||isLandhard()||targetanimation==drawrightanim||targetanimation==drawleftanim||targetanimation==crouchdrawrightanim)&&(wasIdle()||wasCrouch()||wasLanding()||wasLandhard()||currentanimation==drawrightanim||currentanimation==drawleftanim||currentanimation==crouchdrawrightanim))&&!skeleton.free){
					XYZ point,newpoint,change,change2;
					point=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
					heightleft=terrain.getHeight(point.x,point.z)+.04;
					point.y=heightleft;
					change=skeleton.joints[skeleton.jointlabels[leftankle]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
					change2=skeleton.joints[skeleton.jointlabels[leftknee]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
					skeleton.joints[skeleton.jointlabels[leftfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0);
					skeleton.joints[skeleton.jointlabels[leftankle]].position=skeleton.joints[skeleton.jointlabels[leftfoot]].position+change;
					skeleton.joints[skeleton.jointlabels[leftknee]].position=(skeleton.joints[skeleton.jointlabels[leftfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[leftknee]].position)/2;

					point=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
					heightright=terrain.getHeight(point.x,point.z)+.04;
					point.y=heightright;
					change=skeleton.joints[skeleton.jointlabels[rightankle]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
					change2=skeleton.joints[skeleton.jointlabels[rightknee]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
					skeleton.joints[skeleton.jointlabels[rightfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0);
					skeleton.joints[skeleton.jointlabels[rightankle]].position=skeleton.joints[skeleton.jointlabels[rightfoot]].position+change;
					skeleton.joints[skeleton.jointlabels[rightknee]].position=(skeleton.joints[skeleton.jointlabels[rightfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[rightknee]].position)/2;
					skeleton.DoConstraints(&coords,&scale);

					if(creature==wolftype){
						point=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
						heightleft=terrain.getHeight(point.x,point.z)+.04;
						point.y=heightleft;
						change=skeleton.joints[skeleton.jointlabels[leftankle]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
						change2=skeleton.joints[skeleton.jointlabels[leftknee]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
						skeleton.joints[skeleton.jointlabels[leftfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0);
						skeleton.joints[skeleton.jointlabels[leftankle]].position=skeleton.joints[skeleton.jointlabels[leftfoot]].position+change;
						skeleton.joints[skeleton.jointlabels[leftknee]].position=(skeleton.joints[skeleton.jointlabels[leftfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[leftknee]].position)/2;

						point=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
						heightright=terrain.getHeight(point.x,point.z)+.04;
						point.y=heightright;
						change=skeleton.joints[skeleton.jointlabels[rightankle]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
						change2=skeleton.joints[skeleton.jointlabels[rightknee]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
						skeleton.joints[skeleton.jointlabels[rightfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0);
						skeleton.joints[skeleton.jointlabels[rightankle]].position=skeleton.joints[skeleton.jointlabels[rightfoot]].position+change;
						skeleton.joints[skeleton.jointlabels[rightknee]].position=(skeleton.joints[skeleton.jointlabels[rightfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[rightknee]].position)/2;
						skeleton.DoConstraints(&coords,&scale);
					}
				}
				if(onterrain&&((isIdle()||isCrouch()||isLanding()||isLandhard()||targetanimation==drawrightanim||targetanimation==drawleftanim||targetanimation==crouchdrawrightanim)&&!(wasIdle()||wasCrouch()||wasLanding()||wasLandhard()||currentanimation==drawrightanim||currentanimation==drawleftanim||currentanimation==crouchdrawrightanim))&&!skeleton.free){
					XYZ point,newpoint,change,change2;
					point=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
					heightleft=terrain.getHeight(point.x,point.z)+.04;
					point.y=heightleft;
					change=skeleton.joints[skeleton.jointlabels[leftankle]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
					change2=skeleton.joints[skeleton.jointlabels[leftknee]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
					skeleton.joints[skeleton.jointlabels[leftfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0)*target+skeleton.joints[skeleton.jointlabels[leftfoot]].position*(1-target);
					skeleton.joints[skeleton.jointlabels[leftankle]].position=skeleton.joints[skeleton.jointlabels[leftfoot]].position+change;
					skeleton.joints[skeleton.jointlabels[leftknee]].position=(skeleton.joints[skeleton.jointlabels[leftfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[leftknee]].position)/2;

					point=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
					heightright=terrain.getHeight(point.x,point.z)+.04;
					point.y=heightright;
					change=skeleton.joints[skeleton.jointlabels[rightankle]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
					change2=skeleton.joints[skeleton.jointlabels[rightknee]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
					skeleton.joints[skeleton.jointlabels[rightfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0)*target+skeleton.joints[skeleton.jointlabels[rightfoot]].position*(1-target);
					skeleton.joints[skeleton.jointlabels[rightankle]].position=skeleton.joints[skeleton.jointlabels[rightfoot]].position+change;
					skeleton.joints[skeleton.jointlabels[rightknee]].position=(skeleton.joints[skeleton.jointlabels[rightfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[rightknee]].position)/2;
					skeleton.DoConstraints(&coords,&scale);

					if(creature==wolftype){
						point=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
						heightleft=terrain.getHeight(point.x,point.z)+.04;
						point.y=heightleft;
						change=skeleton.joints[skeleton.jointlabels[leftankle]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
						change2=skeleton.joints[skeleton.jointlabels[leftknee]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
						skeleton.joints[skeleton.jointlabels[leftfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0)*target+skeleton.joints[skeleton.jointlabels[leftfoot]].position*(1-target);
						skeleton.joints[skeleton.jointlabels[leftankle]].position=skeleton.joints[skeleton.jointlabels[leftfoot]].position+change;
						skeleton.joints[skeleton.jointlabels[leftknee]].position=(skeleton.joints[skeleton.jointlabels[leftfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[leftknee]].position)/2;

						point=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
						heightright=terrain.getHeight(point.x,point.z)+.04;
						point.y=heightright;
						change=skeleton.joints[skeleton.jointlabels[rightankle]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
						change2=skeleton.joints[skeleton.jointlabels[rightknee]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
						skeleton.joints[skeleton.jointlabels[rightfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0)*target+skeleton.joints[skeleton.jointlabels[rightfoot]].position*(1-target);
						skeleton.joints[skeleton.jointlabels[rightankle]].position=skeleton.joints[skeleton.jointlabels[rightfoot]].position+change;
						skeleton.joints[skeleton.jointlabels[rightknee]].position=(skeleton.joints[skeleton.jointlabels[rightfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[rightknee]].position)/2;
						skeleton.DoConstraints(&coords,&scale);
					}
				}

				if(onterrain&&(!(isIdle()||isCrouch()||isLanding()||isLandhard()||targetanimation==drawrightanim||targetanimation==drawleftanim||targetanimation==crouchdrawrightanim)&&(wasIdle()||wasCrouch()||wasLanding()||wasLandhard()||currentanimation==drawrightanim||currentanimation==drawleftanim||currentanimation==crouchdrawrightanim))&&!skeleton.free){
					XYZ point,newpoint,change,change2;
					point=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
					heightleft=terrain.getHeight(point.x,point.z)+.04;
					point.y=heightleft;
					change=skeleton.joints[skeleton.jointlabels[leftankle]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
					change2=skeleton.joints[skeleton.jointlabels[leftknee]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
					skeleton.joints[skeleton.jointlabels[leftfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0)*(1-target)+skeleton.joints[skeleton.jointlabels[leftfoot]].position*target;
					skeleton.joints[skeleton.jointlabels[leftankle]].position=skeleton.joints[skeleton.jointlabels[leftfoot]].position+change;
					skeleton.joints[skeleton.jointlabels[leftknee]].position=(skeleton.joints[skeleton.jointlabels[leftfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[leftknee]].position)/2;

					point=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
					heightright=terrain.getHeight(point.x,point.z)+.04;
					point.y=heightright;
					change=skeleton.joints[skeleton.jointlabels[rightankle]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
					change2=skeleton.joints[skeleton.jointlabels[rightknee]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
					skeleton.joints[skeleton.jointlabels[rightfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0)*(1-target)+skeleton.joints[skeleton.jointlabels[rightfoot]].position*target;
					skeleton.joints[skeleton.jointlabels[rightankle]].position=skeleton.joints[skeleton.jointlabels[rightfoot]].position+change;
					skeleton.joints[skeleton.jointlabels[rightknee]].position=(skeleton.joints[skeleton.jointlabels[rightfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[rightknee]].position)/2;
					skeleton.DoConstraints(&coords,&scale);

					if(creature==wolftype){
						point=DoRotation(skeleton.joints[skeleton.jointlabels[leftfoot]].position,0,rotation,0)*scale+coords;
						heightleft=terrain.getHeight(point.x,point.z)+.04;
						point.y=heightleft;
						change=skeleton.joints[skeleton.jointlabels[leftankle]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
						change2=skeleton.joints[skeleton.jointlabels[leftknee]].position-skeleton.joints[skeleton.jointlabels[leftfoot]].position;
						skeleton.joints[skeleton.jointlabels[leftfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0)*(1-target)+skeleton.joints[skeleton.jointlabels[leftfoot]].position*target;
						skeleton.joints[skeleton.jointlabels[leftankle]].position=skeleton.joints[skeleton.jointlabels[leftfoot]].position+change;
						skeleton.joints[skeleton.jointlabels[leftknee]].position=(skeleton.joints[skeleton.jointlabels[leftfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[leftknee]].position)/2;

						point=DoRotation(skeleton.joints[skeleton.jointlabels[rightfoot]].position,0,rotation,0)*scale+coords;
						heightright=terrain.getHeight(point.x,point.z)+.04;
						point.y=heightright;
						change=skeleton.joints[skeleton.jointlabels[rightankle]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
						change2=skeleton.joints[skeleton.jointlabels[rightknee]].position-skeleton.joints[skeleton.jointlabels[rightfoot]].position;
						skeleton.joints[skeleton.jointlabels[rightfoot]].position=DoRotation((point-coords)/scale,0,-rotation,0)*(1-target)+skeleton.joints[skeleton.jointlabels[rightfoot]].position*target;
						skeleton.joints[skeleton.jointlabels[rightankle]].position=skeleton.joints[skeleton.jointlabels[rightfoot]].position+change;
						skeleton.joints[skeleton.jointlabels[rightknee]].position=(skeleton.joints[skeleton.jointlabels[rightfoot]].position+change2)/2+(skeleton.joints[skeleton.jointlabels[rightknee]].position)/2;
						skeleton.DoConstraints(&coords,&scale);
					}
				}
			}
			if(!skeleton.free&&(!animation[targetanimation].attack&&targetanimation!=getupfrombackanim&&((targetanimation!=rollanim&&!isFlip())||animation[targetanimation].label[targetframe]==6)&&targetanimation!=getupfromfrontanim&&targetanimation!=wolfrunninganim&&targetanimation!=rabbitrunninganim&&targetanimation!=backhandspringanim&&targetanimation!=walljumpfrontanim&&targetanimation!=hurtidleanim&&!isLandhard()&&!isSleeping()))
				DoHead();
			else {
				targetheadrotation=-targetrotation;
				targetheadrotation2=0;
				if(animation[targetanimation].attack==3)targetheadrotation+=180;
			}
			for(i=0;i<skeleton.drawmodel.vertexNum;i++){
				skeleton.drawmodel.vertex[i]=0;
				skeleton.drawmodel.vertex[i].y=999;
			}
			for(i=0;i<skeleton.drawmodellow.vertexNum;i++){
				skeleton.drawmodellow.vertex[i]=0;
				skeleton.drawmodellow.vertex[i].y=999;
			}
			for(i=0;i<skeleton.drawmodelclothes.vertexNum;i++){
				skeleton.drawmodelclothes.vertex[i]=0;
				skeleton.drawmodelclothes.vertex[i].y=999;
			}
			for(i=0;i<skeleton.num_muscles;i++){
				if((skeleton.muscles[i].numvertices>0&&playerdetail)||(skeleton.muscles[i].numverticeslow>0&&!playerdetail)){
					morphness=0;
					start=0;
					endthing=0;
					if(skeleton.muscles[i].parent1->label==righthand||skeleton.muscles[i].parent2->label==righthand){
						morphness=righthandmorphness;
						start=righthandmorphstart;
						endthing=righthandmorphend;
					}
					if(skeleton.muscles[i].parent1->label==lefthand||skeleton.muscles[i].parent2->label==lefthand){
						morphness=lefthandmorphness;
						start=lefthandmorphstart;
						endthing=lefthandmorphend;
					}
					if(skeleton.muscles[i].parent1->label==head||skeleton.muscles[i].parent2->label==head){
						morphness=headmorphness;
						start=headmorphstart;
						endthing=headmorphend;
					}
					if((skeleton.muscles[i].parent1->label==neck&&skeleton.muscles[i].parent2->label==abdomen)||(skeleton.muscles[i].parent2->label==neck&&skeleton.muscles[i].parent1->label==abdomen)){
						morphness=chestmorphness;
						start=chestmorphstart;
						endthing=chestmorphend;
					}
					if((skeleton.muscles[i].parent1->label==groin&&skeleton.muscles[i].parent2->label==abdomen)||(skeleton.muscles[i].parent2->label==groin&&skeleton.muscles[i].parent1->label==abdomen)){
						morphness=tailmorphness;
						start=tailmorphstart;
						endthing=tailmorphend;
					}
					if(calcrot)skeleton.FindRotationMuscle(i,targetanimation);
					mid=(skeleton.muscles[i].parent1->position+skeleton.muscles[i].parent2->position)/2;
					glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
					glPushMatrix();
						glLoadIdentity();
						if(!skeleton.free)glRotatef(tilt2,1,0,0);
						if(!skeleton.free)glRotatef(tilt,0,0,1);


						glTranslatef(mid.x,mid.y,mid.z);

						skeleton.muscles[i].lastrotate1=skeleton.muscles[i].rotate1;
						glRotatef(-skeleton.muscles[i].lastrotate1+90,0,1,0);

						skeleton.muscles[i].lastrotate2=skeleton.muscles[i].rotate2;
						glRotatef(-skeleton.muscles[i].lastrotate2+90,0,0,1);

						skeleton.muscles[i].lastrotate3=skeleton.muscles[i].rotate3;
						glRotatef(-skeleton.muscles[i].lastrotate3,0,1,0);

						if(playerdetail||skeleton.free==3)
						{
							for(j=0;j<skeleton.muscles[i].numvertices;j++)
							{
								glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
								glPushMatrix();
									if(skeleton.muscles[i].parent1->label==abdomen||skeleton.muscles[i].parent2->label==abdomen)
										glTranslatef((skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].x*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].x*morphness)*proportionbody.x,
										(skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].y*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].y*morphness)*proportionbody.y,
										(skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].z*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].z*morphness)*proportionbody.z);
									if(skeleton.muscles[i].parent1->label==lefthand||skeleton.muscles[i].parent1->label==righthand||skeleton.muscles[i].parent1->label==leftwrist||skeleton.muscles[i].parent1->label==rightwrist||skeleton.muscles[i].parent1->label==leftelbow||skeleton.muscles[i].parent1->label==rightelbow||skeleton.muscles[i].parent2->label==leftelbow||skeleton.muscles[i].parent2->label==rightelbow)
										glTranslatef((skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].x*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].x*morphness)*proportionarms.x,
										(skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].y*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].y*morphness)*proportionarms.y,
										(skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].z*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].z*morphness)*proportionarms.z);
									if(skeleton.muscles[i].parent1->label==leftfoot||skeleton.muscles[i].parent1->label==rightfoot||skeleton.muscles[i].parent1->label==leftankle||skeleton.muscles[i].parent1->label==rightankle||skeleton.muscles[i].parent1->label==leftknee||skeleton.muscles[i].parent1->label==rightknee||skeleton.muscles[i].parent2->label==leftknee||skeleton.muscles[i].parent2->label==rightknee)
										glTranslatef((skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].x*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].x*morphness)*proportionlegs.x,
										(skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].y*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].y*morphness)*proportionlegs.y,
										(skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].z*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].z*morphness)*proportionlegs.z);
									if(skeleton.muscles[i].parent1->label==head||skeleton.muscles[i].parent2->label==head)
										glTranslatef((skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].x*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].x*morphness)*proportionhead.x,
										(skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].y*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].y*morphness)*proportionhead.y,
										(skeleton.model[start].vertex[skeleton.muscles[i].vertices[j]].z*(1-morphness)+skeleton.model[endthing].vertex[skeleton.muscles[i].vertices[j]].z*morphness)*proportionhead.z);
									glGetFloatv(GL_MODELVIEW_MATRIX,M);
									//if(!isnormal(M[12])||!isnormal(M[13])||!isnormal(M[14]))test=0;
									//if(!isnormal(scale))test=1;
									skeleton.drawmodel.vertex[skeleton.muscles[i].vertices[j]].x=M[12]*scale;
									skeleton.drawmodel.vertex[skeleton.muscles[i].vertices[j]].y=M[13]*scale;
									skeleton.drawmodel.vertex[skeleton.muscles[i].vertices[j]].z=M[14]*scale;
									//test=2;
								glPopMatrix();
							}
						}
						if(!playerdetail||skeleton.free==3)
						{
							for(j=0;j<skeleton.muscles[i].numverticeslow;j++)
							{
								glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
								glPushMatrix();
									if(skeleton.muscles[i].parent1->label==abdomen||skeleton.muscles[i].parent2->label==abdomen)
										glTranslatef((skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].x)*proportionbody.x,
										(skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].y)*proportionbody.y,
										(skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].z)*proportionbody.z);
									if(skeleton.muscles[i].parent1->label==lefthand||skeleton.muscles[i].parent1->label==righthand||skeleton.muscles[i].parent1->label==leftwrist||skeleton.muscles[i].parent1->label==rightwrist||skeleton.muscles[i].parent1->label==leftelbow||skeleton.muscles[i].parent1->label==rightelbow||skeleton.muscles[i].parent2->label==leftelbow||skeleton.muscles[i].parent2->label==rightelbow)
										glTranslatef((skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].x)*proportionarms.x,
										(skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].y)*proportionarms.y,
										(skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].z)*proportionarms.z);
									if(skeleton.muscles[i].parent1->label==leftfoot||skeleton.muscles[i].parent1->label==rightfoot||skeleton.muscles[i].parent1->label==leftankle||skeleton.muscles[i].parent1->label==rightankle||skeleton.muscles[i].parent1->label==leftknee||skeleton.muscles[i].parent1->label==rightknee||skeleton.muscles[i].parent2->label==leftknee||skeleton.muscles[i].parent2->label==rightknee)
										glTranslatef((skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].x)*proportionlegs.x,
										(skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].y)*proportionlegs.y,
										(skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].z)*proportionlegs.z);
									if(skeleton.muscles[i].parent1->label==head||skeleton.muscles[i].parent2->label==head)
										glTranslatef((skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].x)*proportionhead.x,
										(skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].y)*proportionhead.y,
										(skeleton.modellow.vertex[skeleton.muscles[i].verticeslow[j]].z)*proportionhead.z);

									glGetFloatv(GL_MODELVIEW_MATRIX,M);
									skeleton.drawmodellow.vertex[skeleton.muscles[i].verticeslow[j]].x=M[12]*scale;
									skeleton.drawmodellow.vertex[skeleton.muscles[i].verticeslow[j]].y=M[13]*scale;
									skeleton.drawmodellow.vertex[skeleton.muscles[i].verticeslow[j]].z=M[14]*scale;
								glPopMatrix();
							}
						}
					glPopMatrix();
				}
				if(skeleton.clothes&&skeleton.muscles[i].numverticesclothes>0){
					mid=(skeleton.muscles[i].parent1->position+skeleton.muscles[i].parent2->position)/2;

					glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
					glPushMatrix();
						glLoadIdentity();
						if(!skeleton.free)glRotatef(tilt2,1,0,0);
						if(!skeleton.free)glRotatef(tilt,0,0,1);
						glTranslatef(mid.x,mid.y,mid.z);
						skeleton.muscles[i].lastrotate1=skeleton.muscles[i].rotate1;
						glRotatef(-skeleton.muscles[i].lastrotate1+90,0,1,0);

						skeleton.muscles[i].lastrotate2=skeleton.muscles[i].rotate2;
						glRotatef(-skeleton.muscles[i].lastrotate2+90,0,0,1);

						skeleton.muscles[i].lastrotate3=skeleton.muscles[i].rotate3;
						glRotatef(-skeleton.muscles[i].lastrotate3,0,1,0);

						for(j=0;j<skeleton.muscles[i].numverticesclothes;j++){
							glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
							glPushMatrix();
								if(skeleton.muscles[i].parent1->label==abdomen||skeleton.muscles[i].parent2->label==abdomen)
									glTranslatef((skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].x)*proportionbody.x,
									(skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].y)*proportionbody.y,
									(skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].z)*proportionbody.z);
								if(skeleton.muscles[i].parent1->label==lefthand||skeleton.muscles[i].parent1->label==righthand||skeleton.muscles[i].parent1->label==leftwrist||skeleton.muscles[i].parent1->label==rightwrist||skeleton.muscles[i].parent1->label==leftelbow||skeleton.muscles[i].parent1->label==rightelbow||skeleton.muscles[i].parent2->label==leftelbow||skeleton.muscles[i].parent2->label==rightelbow)
									glTranslatef((skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].x)*proportionarms.x,
									(skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].y)*proportionarms.y,
									(skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].z)*proportionarms.z);
								if(skeleton.muscles[i].parent1->label==leftfoot||skeleton.muscles[i].parent1->label==rightfoot||skeleton.muscles[i].parent1->label==leftankle||skeleton.muscles[i].parent1->label==rightankle||skeleton.muscles[i].parent1->label==leftknee||skeleton.muscles[i].parent1->label==rightknee||skeleton.muscles[i].parent2->label==leftknee||skeleton.muscles[i].parent2->label==rightknee)
									glTranslatef((skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].x)*proportionlegs.x,
									(skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].y)*proportionlegs.y,
									(skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].z)*proportionlegs.z);
								if(skeleton.muscles[i].parent1->label==head||skeleton.muscles[i].parent2->label==head)
									glTranslatef((skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].x)*proportionhead.x,
									(skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].y)*proportionhead.y,
									(skeleton.modelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].z)*proportionhead.z);
								glGetFloatv(GL_MODELVIEW_MATRIX,M);
								skeleton.drawmodelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].x=M[12]*scale;
								skeleton.drawmodelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].y=M[13]*scale;
								skeleton.drawmodelclothes.vertex[skeleton.muscles[i].verticesclothes[j]].z=M[14]*scale;
							glPopMatrix();
						}
					glPopMatrix();
				}
				updatedelay=1+(float)(Random()%100)/1000;
			}
			if(skeleton.free!=2&&(skeleton.free==1||skeleton.free==3||id==0||(normalsupdatedelay<=0)||targetanimation==getupfromfrontanim||targetanimation==getupfrombackanim||currentanimation==getupfromfrontanim||currentanimation==getupfrombackanim)){
				normalsupdatedelay=1;
				if(playerdetail||skeleton.free==3)skeleton.drawmodel.CalculateNormals(0);
				if(!playerdetail||skeleton.free==3)skeleton.drawmodellow.CalculateNormals(0);
				if(skeleton.clothes)skeleton.drawmodelclothes.CalculateNormals(0);
			}
			else
			{
				if(playerdetail||skeleton.free==3)skeleton.drawmodel.UpdateVertexArrayNoTexNoNorm();
				if(!playerdetail||skeleton.free==3)skeleton.drawmodellow.UpdateVertexArrayNoTexNoNorm();
				if(skeleton.clothes){
					skeleton.drawmodelclothes.UpdateVertexArrayNoTexNoNorm();
				}
			}
		}
		framemult=.01;
		updatedelaychange=-framemult*4*(45-findDistance(&viewer,&coords)*1);
		if(updatedelaychange>-realmultiplier*30)updatedelaychange=-realmultiplier*30;
		if(updatedelaychange>-framemult*4)updatedelaychange=-framemult*4;
		if(skeleton.free==1)updatedelaychange*=6;
		if(id==0)updatedelaychange*=8;
		updatedelay+=updatedelaychange;

		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glPushMatrix();
		if(!skeleton.free)glTranslatef(coords.x,coords.y-.02,coords.z);
		if(skeleton.free)glTranslatef(coords.x,coords.y-.02,coords.z);
		if(!skeleton.free)glTranslatef(offset.x*scale,offset.y*scale,offset.z*scale);
		if(!skeleton.free)glRotatef(rotation,0,1,0);
		if(showpoints){
			glPointSize(5);
			glColor4f(.4,1,.4,1);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_POINTS);
			if(playerdetail)
				for(i=0;i<skeleton.drawmodel.vertexNum;i++){
					glVertex3f(skeleton.drawmodel.vertex[i].x,skeleton.drawmodel.vertex[i].y,skeleton.drawmodel.vertex[i].z);
				}
				glEnd();
				glBegin(GL_LINES);

				if(playerdetail)
					for(i=0;i<skeleton.drawmodel.TriangleNum;i++){
						glVertex3f(skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[0]].x,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[0]].y,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[0]].z);
						glVertex3f(skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[1]].x,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[1]].y,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[1]].z);
						glVertex3f(skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[1]].x,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[1]].y,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[1]].z);
						glVertex3f(skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[2]].x,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[2]].y,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[2]].z);
						glVertex3f(skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[2]].x,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[2]].y,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[2]].z);
						glVertex3f(skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[0]].x,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[0]].y,skeleton.drawmodel.vertex[skeleton.drawmodel.Triangles[i].vertex[0]].z);
					}

				glEnd();
		}

		terrainlight=terrain.getLighting(coords.x,coords.z);
		distance=findDistancefast(&viewer,&coords);
		distance=(viewdistance*viewdistance-(distance-(viewdistance*viewdistance*fadestart))*(1/(1-fadestart)))/viewdistance/viewdistance;
		if(distance>1)distance=1;
		if(distance>0){
			terrainheight=(coords.y-terrain.getHeight(coords.x,coords.z))/3+1;
			if(terrainheight<1)terrainheight=1;
			if(terrainheight>1.7)terrainheight=1.7;

			//burnt=0;
			glColor4f((1-(1-terrainlight.x)/terrainheight)-burnt,(1-(1-terrainlight.y)/terrainheight)-burnt,(1-(1-terrainlight.z)/terrainheight)-burnt,distance);
			glDisable(GL_BLEND);
			glAlphaFunc(GL_GREATER, 0.0001);
			glEnable(GL_TEXTURE_2D);
			if(cellophane){
				glDisable(GL_TEXTURE_2D);
				glColor4f(.7,.35,0,.5);
				glDepthMask(0);
				glEnable(GL_LIGHTING);
				glEnable(GL_BLEND);
			}
			if(tutoriallevel&&id!=0){
				//glDisable(GL_TEXTURE_2D);
				glColor4f(.7,.7,.7,0.6);
				glDepthMask(0);
				glEnable(GL_LIGHTING);
				glEnable(GL_BLEND);
				if(canattack&&cananger)
					if(animation[targetanimation].attack==normalattack||animation[targetanimation].attack==reversed){
						glDisable(GL_TEXTURE_2D);
						glColor4f(1,0,0,0.8);
					}
					glMatrixMode(GL_TEXTURE);
					glPushMatrix();
					glTranslatef(0,-smoketex,0);
					glTranslatef(-smoketex,0,0);
			}
			if(playerdetail){
				if(!showpoints){
					if((tutoriallevel&&id!=0))skeleton.drawmodel.drawdifftex(Sprite::cloudimpacttexture);
					else skeleton.drawmodel.draw();
				}
			}
			if(!playerdetail){
				if((tutoriallevel&&id!=0))skeleton.drawmodellow.drawdifftex(Sprite::cloudimpacttexture);
				else skeleton.drawmodellow.drawdifftex(skeleton.drawmodel.textureptr);
			}

			if(!(animation[targetanimation].attack==normalattack||animation[targetanimation].attack==reversed))
				if(tutoriallevel&&id!=0){
					glPopMatrix();
					glMatrixMode(GL_MODELVIEW);
					glEnable(GL_TEXTURE_2D);
					glColor4f(.7,.7,.7,0.6);
					glDepthMask(0);
					glEnable(GL_LIGHTING);
					glEnable(GL_BLEND);
					if(canattack&&cananger)
						if(animation[targetanimation].attack==normalattack||animation[targetanimation].attack==reversed){
							glDisable(GL_TEXTURE_2D);
							glColor4f(1,0,0,0.8);
						}
						glMatrixMode(GL_TEXTURE);
						glPushMatrix();
						glTranslatef(0,-smoketex*.6,0);
						glTranslatef(smoketex*.6,0,0);
						if(playerdetail){
							if(!showpoints){
								if((tutoriallevel&&id!=0))skeleton.drawmodel.drawdifftex(Sprite::cloudimpacttexture);
								else skeleton.drawmodel.draw();
							}
						}
						if(!playerdetail){
							if((tutoriallevel&&id!=0))skeleton.drawmodellow.drawdifftex(Sprite::cloudimpacttexture);
							else skeleton.drawmodellow.drawdifftex(skeleton.drawmodel.textureptr);
						}
				}


				if(tutoriallevel&&id!=0){
					glPopMatrix();
					glMatrixMode(GL_MODELVIEW);
					glEnable(GL_TEXTURE_2D);
				}
				if(skeleton.clothes){
					glDepthMask(0);
					glEnable(GL_BLEND);
					if(!immediate)skeleton.drawmodelclothes.draw();
					if(immediate)skeleton.drawmodelclothes.drawimmediate();
					glDepthMask(1);
				}
		}
		glPopMatrix();

		if(num_weapons>0){
			for(k=0;k<num_weapons;k++){
				i=weaponids[k];
				if(weaponactive==k){
					if(weapons.type[i]!=staff){
						for(j=0;j<skeleton.num_muscles;j++){
							if((skeleton.muscles[j].parent1->label==righthand||skeleton.muscles[j].parent2->label==righthand)&&skeleton.muscles[j].numvertices>0){
								weaponattachmuscle=j;
							}
						}
						for(j=0;j<skeleton.num_muscles;j++){
							if((skeleton.muscles[j].parent1->label==rightwrist||skeleton.muscles[j].parent2->label==rightwrist)&&(skeleton.muscles[j].parent1->label!=righthand&&skeleton.muscles[j].parent2->label!=righthand)&&skeleton.muscles[j].numvertices>0){
								weaponrotatemuscle=j;
							}
						}
						weaponpoint=(skeleton.muscles[weaponattachmuscle].parent1->position+skeleton.muscles[weaponattachmuscle].parent2->position)/2;
						if(creature==wolftype)weaponpoint=(skeleton.joints[skeleton.jointlabels[rightwrist]].position*.7+skeleton.joints[skeleton.jointlabels[righthand]].position*.3);
					}
					if(weapons.type[i]==staff){
						for(j=0;j<skeleton.num_muscles;j++){
							if((skeleton.muscles[j].parent1->label==righthand||skeleton.muscles[j].parent2->label==righthand)&&skeleton.muscles[j].numvertices>0){
								weaponattachmuscle=j;
							}
						}
						for(j=0;j<skeleton.num_muscles;j++){
							if((skeleton.muscles[j].parent1->label==rightelbow||skeleton.muscles[j].parent2->label==rightelbow)&&(skeleton.muscles[j].parent1->label!=rightshoulder&&skeleton.muscles[j].parent2->label!=rightshoulder)&&skeleton.muscles[j].numvertices>0){
								weaponrotatemuscle=j;
							}
						}
						//weaponpoint=skeleton.joints[skeleton.jointlabels[rightwrist]].position;
						weaponpoint=(skeleton.muscles[weaponattachmuscle].parent1->position+skeleton.muscles[weaponattachmuscle].parent2->position)/2;
						//weaponpoint+=skeleton.specialforward[1]*.1+(skeleton.joints[skeleton.jointlabels[rightwrist]].position-skeleton.joints[skeleton.jointlabels[rightelbow]].position);
						XYZ tempnormthing,vec1,vec2;
						vec1=(skeleton.joints[skeleton.jointlabels[rightwrist]].position-skeleton.joints[skeleton.jointlabels[rightelbow]].position);
						vec2=(skeleton.joints[skeleton.jointlabels[rightwrist]].position-skeleton.joints[skeleton.jointlabels[rightshoulder]].position);
						CrossProduct(&vec1,&vec2,&tempnormthing);
						Normalise(&tempnormthing);
						if(targetanimation!=staffhitanim&&currentanimation!=staffhitanim&&targetanimation!=staffgroundsmashanim&&currentanimation!=staffgroundsmashanim&&targetanimation!=staffspinhitanim&&currentanimation!=staffspinhitanim)weaponpoint+=tempnormthing*.1-skeleton.specialforward[1]*.3+(skeleton.joints[skeleton.jointlabels[rightwrist]].position-skeleton.joints[skeleton.jointlabels[rightelbow]].position);
					}
				}
				if(weaponactive!=k&&weaponstuck!=k){
					if(weapons.type[i]==knife)weaponpoint=skeleton.joints[skeleton.jointlabels[abdomen]].position+(skeleton.joints[skeleton.jointlabels[righthip]].position-skeleton.joints[skeleton.jointlabels[lefthip]].position)*.1+(skeleton.joints[skeleton.jointlabels[rightshoulder]].position-skeleton.joints[skeleton.jointlabels[leftshoulder]].position)*.35;
					if(weapons.type[i]==sword)weaponpoint=skeleton.joints[skeleton.jointlabels[abdomen]].position+(skeleton.joints[skeleton.jointlabels[lefthip]].position-skeleton.joints[skeleton.jointlabels[righthip]].position)*.09+(skeleton.joints[skeleton.jointlabels[leftshoulder]].position-skeleton.joints[skeleton.jointlabels[rightshoulder]].position)*.33;
					if(weapons.type[i]==staff)weaponpoint=skeleton.joints[skeleton.jointlabels[abdomen]].position+(skeleton.joints[skeleton.jointlabels[lefthip]].position-skeleton.joints[skeleton.jointlabels[righthip]].position)*.09+(skeleton.joints[skeleton.jointlabels[leftshoulder]].position-skeleton.joints[skeleton.jointlabels[rightshoulder]].position)*.33;
					for(j=0;j<skeleton.num_muscles;j++){
						if((skeleton.muscles[j].parent1->label==abdomen||skeleton.muscles[j].parent2->label==abdomen)&&(skeleton.muscles[j].parent1->label==neck||skeleton.muscles[j].parent2->label==neck)&&skeleton.muscles[j].numvertices>0){
							weaponrotatemuscle=j;
						}
					}
				}
				if(weaponstuck==k){
					if(weaponstuckwhere==0)weaponpoint=skeleton.joints[skeleton.jointlabels[abdomen]].position*.5+skeleton.joints[skeleton.jointlabels[neck]].position*.5-skeleton.forward*.8;
					else weaponpoint=skeleton.joints[skeleton.jointlabels[abdomen]].position*.5+skeleton.joints[skeleton.jointlabels[neck]].position*.5+skeleton.forward*.8;
					for(j=0;j<skeleton.num_muscles;j++){
						if((skeleton.muscles[j].parent1->label==abdomen||skeleton.muscles[j].parent2->label==abdomen)&&(skeleton.muscles[j].parent1->label==neck||skeleton.muscles[j].parent2->label==neck)&&skeleton.muscles[j].numvertices>0){
							weaponrotatemuscle=j;
						}
					}
				}
				if(!skeleton.free){
					weapons.position[i]=DoRotation(DoRotation(DoRotation(weaponpoint,0,0,tilt),tilt2,0,0),0,rotation,0)*scale+coords+currentoffset*(1-target)*scale+targetoffset*target*scale;
					weapons.bigrotation[i]=rotation;
					weapons.bigtilt[i]=tilt;
					weapons.bigtilt2[i]=tilt2;
				}
				if(skeleton.free){
					weapons.position[i]=weaponpoint*scale+coords;
					weapons.bigrotation[i]=0;
					weapons.bigtilt[i]=0;
					weapons.bigtilt2[i]=0;
				}
				weapons.rotation1[i]=skeleton.muscles[weaponrotatemuscle].lastrotate1;
				weapons.rotation2[i]=skeleton.muscles[weaponrotatemuscle].lastrotate2;
				weapons.rotation3[i]=skeleton.muscles[weaponrotatemuscle].lastrotate3;
				if(weaponactive==k){
					if(weapons.type[i]==knife){
						weapons.smallrotation[i]=180;
						weapons.smallrotation2[i]=0;
						if(isCrouch()||wasCrouch()){
							weapons.smallrotation2[i]=20;
						}
						if(targetanimation==hurtidleanim){
							weapons.smallrotation2[i]=50;
						}
						if((currentanimation==crouchstabanim&&targetanimation==crouchstabanim)||(currentanimation==backhandspringanim&&targetanimation==backhandspringanim)){
							XYZ temppoint1,temppoint2,tempforward;
							float distance;

							temppoint1=skeleton.joints[skeleton.jointlabels[righthand]].position;
							temppoint2=animation[currentanimation].weapontarget[currentframe]*(1-target)+animation[targetanimation].weapontarget[targetframe]*(target);
							distance=findDistance(&temppoint1,&temppoint2);
							weapons.rotation2[i]=asin((temppoint1.y-temppoint2.y)/distance);
							weapons.rotation2[i]*=360/6.28;
							temppoint1.y=0;
							temppoint2.y=0;
							weapons.rotation1[i]=acos((temppoint1.z-temppoint2.z)/findDistance(&temppoint1,&temppoint2));
							weapons.rotation1[i]*=360/6.28;
							weapons.rotation3[i]=0;
							weapons.smallrotation[i]=-90;
							weapons.smallrotation2[i]=0;
							if(temppoint1.x>temppoint2.x)weapons.rotation1[i]=360-weapons.rotation1[i];
						}
						if((currentanimation==knifeslashreversalanim&&targetanimation==knifeslashreversalanim)||(currentanimation==knifeslashreversedanim&&targetanimation==knifeslashreversedanim)){
							XYZ temppoint1,temppoint2,tempforward;
							float distance;

							temppoint1=skeleton.joints[skeleton.jointlabels[righthand]].position;
							temppoint2=animation[currentanimation].weapontarget[currentframe]*(1-target)+animation[targetanimation].weapontarget[targetframe]*(target);
							distance=findDistance(&temppoint1,&temppoint2);
							weapons.rotation2[i]=asin((temppoint1.y-temppoint2.y)/distance);
							weapons.rotation2[i]*=360/6.28;
							temppoint1.y=0;
							temppoint2.y=0;
							weapons.rotation1[i]=acos((temppoint1.z-temppoint2.z)/findDistance(&temppoint1,&temppoint2));
							weapons.rotation1[i]*=360/6.28;
							weapons.rotation3[i]=0;
							weapons.smallrotation[i]=90;
							weapons.smallrotation2[i]=0;
							if(temppoint1.x>temppoint2.x)weapons.rotation1[i]=360-weapons.rotation1[i];
						}
						if(targetanimation==knifethrowanim){
							weapons.smallrotation[i]=90;
							//weapons.smallrotation2[i]=-90;
							weapons.smallrotation2[i]=0;
							weapons.rotation1[i]=0;
							weapons.rotation2[i]=0;
							weapons.rotation3[i]=0;
						}
						if(targetanimation==knifesneakattackanim&&targetframe<5){
							weapons.smallrotation[i]=-90;
							weapons.rotation1[i]=0;
							weapons.rotation2[i]=0;
							weapons.rotation3[i]=0;
						}
					}
					if(weapons.type[i]==sword){
						weapons.smallrotation[i]=0;
						weapons.smallrotation2[i]=0;
						if(targetanimation==knifethrowanim){
							weapons.smallrotation[i]=-90;
							weapons.smallrotation2[i]=0;
							weapons.rotation1[i]=0;
							weapons.rotation2[i]=0;
							weapons.rotation3[i]=0;
						}
						if((targetanimation==swordgroundstabanim&&currentanimation==swordgroundstabanim)||(targetanimation==swordsneakattackanim&&currentanimation==swordsneakattackanim)||(targetanimation==swordslashparryanim&&currentanimation==swordslashparryanim)||(targetanimation==swordslashparriedanim&&currentanimation==swordslashparriedanim)||(targetanimation==swordslashreversalanim&&currentanimation==swordslashreversalanim)||(targetanimation==swordslashreversedanim&&currentanimation==swordslashreversedanim)||(targetanimation==knifeslashreversalanim&&currentanimation==knifeslashreversalanim)||(targetanimation==knifeslashreversedanim&&currentanimation==knifeslashreversedanim)||(targetanimation==swordslashanim&&currentanimation==swordslashanim)||(targetanimation==drawleftanim&&currentanimation==drawleftanim)||(currentanimation==backhandspringanim&&targetanimation==backhandspringanim)){
							XYZ temppoint1,temppoint2,tempforward;
							float distance;

							temppoint1=animation[currentanimation].position[skeleton.jointlabels[righthand]][currentframe]*(1-target)+animation[targetanimation].position[skeleton.jointlabels[righthand]][targetframe]*(target); //skeleton.joints[skeleton.jointlabels[righthand]].position;
							temppoint2=animation[currentanimation].weapontarget[currentframe]*(1-target)+animation[targetanimation].weapontarget[targetframe]*(target);
							distance=findDistance(&temppoint1,&temppoint2);
							weapons.rotation2[i]=asin((temppoint1.y-temppoint2.y)/distance);
							weapons.rotation2[i]*=360/6.28;
							temppoint1.y=0;
							temppoint2.y=0;
							weapons.rotation1[i]=acos((temppoint1.z-temppoint2.z)/findDistance(&temppoint1,&temppoint2));
							weapons.rotation1[i]*=360/6.28;
							weapons.rotation3[i]=0;
							weapons.smallrotation[i]=90;
							weapons.smallrotation2[i]=0;
							if(temppoint1.x>temppoint2.x)weapons.rotation1[i]=360-weapons.rotation1[i];
						}
					}
					if(weapons.type[i]==staff){
						weapons.smallrotation[i]=100;
						weapons.smallrotation2[i]=0;
						if((targetanimation==staffhitanim&&currentanimation==staffhitanim)||(targetanimation==staffhitreversedanim&&currentanimation==staffhitreversedanim)||(targetanimation==staffspinhitreversedanim&&currentanimation==staffspinhitreversedanim)||(targetanimation==staffgroundsmashanim&&currentanimation==staffgroundsmashanim)||(targetanimation==staffspinhitanim&&currentanimation==staffspinhitanim)){
							XYZ temppoint1,temppoint2,tempforward;
							float distance;

							temppoint1=animation[currentanimation].position[skeleton.jointlabels[righthand]][currentframe]*(1-target)+animation[targetanimation].position[skeleton.jointlabels[righthand]][targetframe]*(target); //skeleton.joints[skeleton.jointlabels[righthand]].position;
							temppoint2=animation[currentanimation].weapontarget[currentframe]*(1-target)+animation[targetanimation].weapontarget[targetframe]*(target);
							distance=findDistance(&temppoint1,&temppoint2);
							weapons.rotation2[i]=asin((temppoint1.y-temppoint2.y)/distance);
							weapons.rotation2[i]*=360/6.28;
							temppoint1.y=0;
							temppoint2.y=0;
							weapons.rotation1[i]=acos((temppoint1.z-temppoint2.z)/findDistance(&temppoint1,&temppoint2));
							weapons.rotation1[i]*=360/6.28;
							weapons.rotation3[i]=0;
							weapons.smallrotation[i]=90;
							weapons.smallrotation2[i]=0;
							if(temppoint1.x>temppoint2.x)weapons.rotation1[i]=360-weapons.rotation1[i];
						}
					}
				}
				if(weaponactive!=k&&weaponstuck!=k){
					if(weapons.type[i]==knife){
						weapons.smallrotation[i]=-70;
						weapons.smallrotation2[i]=10;
					}
					if(weapons.type[i]==sword){
						weapons.smallrotation[i]=-100;
						weapons.smallrotation2[i]=-8;
					}
					if(weapons.type[i]==staff){
						weapons.smallrotation[i]=-100;
						weapons.smallrotation2[i]=-8;
					}
				}
				if(weaponstuck==k){
					if(weaponstuckwhere==0)weapons.smallrotation[i]=180;
					else weapons.smallrotation[i]=0;
					weapons.smallrotation2[i]=10;
				}
			}
		}
	}

	calcrot=0;
	if(skeleton.free)calcrot=1;
	if(animation[targetanimation].attack||isRun()||targetanimation==staggerbackhardanim||isFlip()||targetanimation==climbanim||targetanimation==sneakanim||targetanimation==rollanim||targetanimation==walkanim||targetanimation==backhandspringanim||isFlip()||isWallJump())calcrot=1;
	if(currentanimation!=targetanimation)calcrot=1;
	//if(id==0)calcrot=1;
	if(skeleton.free==2)calcrot=0;

	return 0;
}


int Person::SphereCheck(XYZ *p1,float radius, XYZ *p, XYZ *move, float *rotate, Model *model)
{
	static int i,j;
	static float distance;
	static float olddistance;
	static int intersecting;
	static int firstintersecting;
	static XYZ point;
	static XYZ oldp1;
	static XYZ start,end;
	static float slopethreshold=-.4;

	firstintersecting=-1;

	oldp1=*p1;
	*p1=*p1-*move;
	if(findDistancefast(p1,&model->boundingspherecenter)>radius*radius+model->boundingsphereradius*model->boundingsphereradius)return -1;
	if(*rotate)*p1=DoRotation(*p1,0,-*rotate,0);
	for(i=0;i<4;i++){
		for (j=0;j<model->TriangleNum;j++){
			if(model->facenormals[j].y<=slopethreshold){
				intersecting=0;
				distance=abs((model->facenormals[j].x*p1->x)+(model->facenormals[j].y*p1->y)+(model->facenormals[j].z*p1->z)-((model->facenormals[j].x*model->vertex[model->Triangles[j].vertex[0]].x)+(model->facenormals[j].y*model->vertex[model->Triangles[j].vertex[0]].y)+(model->facenormals[j].z*model->vertex[model->Triangles[j].vertex[0]].z)));
				if(distance<radius){
					point=*p1-model->facenormals[j]*distance;
					if(PointInTriangle( &point, model->facenormals[j], &model->vertex[model->Triangles[j].vertex[0]], &model->vertex[model->Triangles[j].vertex[1]], &model->vertex[model->Triangles[j].vertex[2]]))intersecting=1;
					if(!intersecting)intersecting=sphere_line_intersection(&model->vertex[model->Triangles[j].vertex[0]],
						&model->vertex[model->Triangles[j].vertex[1]],
						p1, &radius);
					if(!intersecting)intersecting=sphere_line_intersection(&model->vertex[model->Triangles[j].vertex[1]],
						&model->vertex[model->Triangles[j].vertex[2]],
						p1, &radius);
					if(!intersecting)intersecting=sphere_line_intersection(&model->vertex[model->Triangles[j].vertex[0]],
						&model->vertex[model->Triangles[j].vertex[2]],
						p1, &radius);
					end=*p1-point;
					if(dotproduct(&model->facenormals[j],&end)>0&&intersecting){
						start=*p1;
						end=*p1;
						end.y-=radius;
						if(LineFacetd(&start,&end,&model->vertex[model->Triangles[j].vertex[0]],&model->vertex[model->Triangles[j].vertex[1]],&model->vertex[model->Triangles[j].vertex[2]],&model->facenormals[j],&point)){
							p1->y=point.y+radius;
							if((targetanimation==jumpdownanim||isFlip())){
								if(isFlip()&&(targetframe<5||animation[targetanimation].label[targetframe]==7||animation[targetanimation].label[targetframe]==4))RagDoll(0);

								if(targetanimation==jumpupanim){jumppower=-4;targetanimation=getIdle();}
								target=0;
								targetframe=0;
								onterrain=1;

								if(id==0){
									pause_sound(whooshsound);
									OPENAL_SetVolume(channels[whooshsound], 0);
								}

								if((targetanimation==jumpdownanim||isFlip())&&!wasLanding()&&!wasLandhard()){
									if(isFlip())jumppower=-4;
									targetanimation=getLanding();
									emit_sound_at(landsound, coords, 128.);

									if(id==0){
										envsound[numenvsounds]=coords;
										envsoundvol[numenvsounds]=16;
										envsoundlife[numenvsounds]=.4;
										numenvsounds++;
									}
								}
							}
						}
					}
				}
				if((distance<olddistance||firstintersecting==-1)&&intersecting){olddistance=distance; firstintersecting=j; *p=point;}
			}
		}
		for (j=0;j<model->TriangleNum;j++){
			if(model->facenormals[j].y>slopethreshold){
				intersecting=0;
				start=*p1;
				start.y-=radius/4;
				distance=abs((model->facenormals[j].x*start.x)+(model->facenormals[j].y*start.y)+(model->facenormals[j].z*start.z)-((model->facenormals[j].x*model->vertex[model->Triangles[j].vertex[0]].x)+(model->facenormals[j].y*model->vertex[model->Triangles[j].vertex[0]].y)+(model->facenormals[j].z*model->vertex[model->Triangles[j].vertex[0]].z)));
				if(distance<radius*.5){
					point=start-model->facenormals[j]*distance;
					if(PointInTriangle( &point, model->facenormals[j], &model->vertex[model->Triangles[j].vertex[0]], &model->vertex[model->Triangles[j].vertex[1]], &model->vertex[model->Triangles[j].vertex[2]]))intersecting=1;
					if(!intersecting)intersecting=sphere_line_intersection(model->vertex[model->Triangles[j].vertex[0]].x,model->vertex[model->Triangles[j].vertex[0]].y,model->vertex[model->Triangles[j].vertex[0]].z,
						model->vertex[model->Triangles[j].vertex[1]].x,model->vertex[model->Triangles[j].vertex[1]].y,model->vertex[model->Triangles[j].vertex[1]].z,
						p1->x, p1->y, p1->z, radius/2);
					if(!intersecting)intersecting=sphere_line_intersection(model->vertex[model->Triangles[j].vertex[1]].x,model->vertex[model->Triangles[j].vertex[1]].y,model->vertex[model->Triangles[j].vertex[1]].z,
						model->vertex[model->Triangles[j].vertex[2]].x,model->vertex[model->Triangles[j].vertex[2]].y,model->vertex[model->Triangles[j].vertex[2]].z,
						p1->x, p1->y, p1->z, radius/2);
					if(!intersecting)intersecting=sphere_line_intersection(model->vertex[model->Triangles[j].vertex[0]].x,model->vertex[model->Triangles[j].vertex[0]].y,model->vertex[model->Triangles[j].vertex[0]].z,
						model->vertex[model->Triangles[j].vertex[2]].x,model->vertex[model->Triangles[j].vertex[2]].y,model->vertex[model->Triangles[j].vertex[2]].z,
						p1->x, p1->y, p1->z, radius/2);
					end=*p1-point;
					if(dotproduct(&model->facenormals[j],&end)>0&&intersecting){
						if((targetanimation==jumpdownanim||targetanimation==jumpupanim||isFlip())){
							start=velocity;
							velocity-=DoRotation(model->facenormals[j],0,*rotate,0)*findLength(&velocity)*abs(normaldotproduct(velocity,DoRotation(model->facenormals[j],0,*rotate,0)));//(distance-radius*.5)/multiplier;
							if(findLengthfast(&start)<findLengthfast(&velocity))velocity=start;
						}
						*p1+=model->facenormals[j]*(distance-radius*.5);
					}
				}
				if((distance<olddistance||firstintersecting==-1)&&intersecting){olddistance=distance; firstintersecting=j; *p=point;}
			}
		}
	}
	if(*rotate)*p=DoRotation(*p,0,*rotate,0);
	*p=*p+*move;
	if(*rotate)*p1=DoRotation(*p1,0,*rotate,0);
	*p1+=*move;
	return firstintersecting;
}
