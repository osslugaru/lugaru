#ifndef _WEAPONS_H_
#define _WEAPONS_H_

/**> HEADER FILES <**/

#include "gl.h"
#include "Quaternions.h"
#include "fmod.h"
#include "skeleton.h"
#include "models.h"
#include "Constants.h"
#include "Terrain.h"
#include "Sprites.h"
#include "Person.h"
#include <cmath>

#define max_weapons 30
#define max_weaponinstances 20

#define knife 1
#define sword 2
#define staff 3

class Weapons
{
public:

	std::vector<float> rotation1,rotation2,rotation3;
	std::vector<float> bigrotation;
	std::vector<float> bigtilt;
	std::vector<float> bigtilt2;
	std::vector<float> smallrotation;
	std::vector<float> smallrotation2;

	int numweapons;
	std::vector<float> damage;
	std::vector<XYZ> tippoint;
	std::vector<XYZ> oldtippoint;
	std::vector<XYZ> position;
	std::vector<float> lastmult;
	std::vector<XYZ> oldposition;
	std::vector<XYZ> velocity;
	std::vector<XYZ> tipvelocity;
	std::vector<int> type;
	std::vector<int> oldowner;
	std::vector<int> owner;
	std::vector<int> bloody;
	std::vector<float> blooddrip;
	std::vector<float> blooddripdelay;
	std::vector<bool> onfire;
	std::vector<float> flamedelay;
	std::vector<bool> missed;
	std::vector<float> mass;
	std::vector<float> tipmass;
	std::vector<float> length;
	std::vector<float> freetime;
	std::vector<bool> firstfree;
	std::vector<bool> physics;
	std::vector<float> drawhowmany;
	std::vector<bool> hitsomething;

	std::vector<XYZ> lastdrawnposition;
	std::vector<XYZ> lastdrawntippoint;
	std::vector<float> lastdrawnrotation1,lastdrawnrotation2,lastdrawnrotation3;
	std::vector<float> lastdrawnbigrotation;
	std::vector<float> lastdrawnbigtilt;
	std::vector<float> lastdrawnbigtilt2;
	std::vector<float> lastdrawnsmallrotation;
	std::vector<float> lastdrawnsmallrotation2;
	std::vector<int> lastdrawnanim;

	Model throwingknifemodel;
	GLuint knifetextureptr;
	GLuint lightbloodknifetextureptr;
	GLuint bloodknifetextureptr;

	Model swordmodel;
	GLuint swordtextureptr;
	GLuint lightbloodswordtextureptr;
	GLuint bloodswordtextureptr;

	Model staffmodel;
	GLuint stafftextureptr;

	int Draw();
	void DoStuff();

	Weapons();

	~Weapons();
};

#endif