#ifndef _SKELETON_H_
#define _SKELETON_H_

#include "Models.h"
#include "Quaternions.h"
#include "Constants.h"


/**> HEADER FILES <**/
#include "gamegl.h"
#include "Quaternions.h"
#include "fmod.h"
#include "Objects.h"
#include "Sprites.h"
#include "binio.h"

#define neutral 0
#define normalattack 1
#define reversed 2
#define reversal 3
#define lowheight 0
#define middleheight 1
#define highheight 2

class Joint
{
public:
	XYZ position;
	XYZ oldposition;
	XYZ realoldposition;
	XYZ velocity;
	XYZ oldvelocity;
	XYZ startpos;
	float blurred;
	float length;
	float mass;
	bool lower;
	bool hasparent;
	bool locked;
	int modelnum;
	bool visible;
	Joint* parent;
	bool sametwist;
	int label;
	int hasgun;
	float delay;
	XYZ velchange;

	Joint()
	{
		blurred = 0;
		length = 0;
		mass = 0;
		lower = 0;
		hasparent = 0;
		locked = 0;
		modelnum = 0;
		visible = 0;
		parent = 0;
		sametwist = 0;
		label = 0;
		hasgun = 0;
		delay = 0;
	}

	void DoConstraint();
};

class Muscle
{
public:
	int numvertices;
	int* vertices;
	int numverticeslow;
	int* verticeslow;
	int numverticesclothes;
	int* verticesclothes;
	float length;
	float targetlength;
	Joint* parent1;
	Joint* parent2;
	float maxlength;
	float minlength;
	int type;
	bool visible;
	void DoConstraint(bool spinny);
	float rotate1,rotate2,rotate3;
	float lastrotate1,lastrotate2,lastrotate3;
	float oldrotate1,oldrotate2,oldrotate3;
	float newrotate1,newrotate2,newrotate3;

	float strength;

	~Muscle();
	Muscle();
};

class Animation
{
public:
	int numframes;
	int height;
	int attack;
	int joints;
	int weapontargetnum;

	XYZ**  position;
	float** twist;
	float** twist2;
	float* speed;
	bool** onground;
	XYZ* forward;
	int* label;
	XYZ* weapontarget;


	XYZ offset;

	Animation();

	~Animation();

	Animation & operator = (const Animation & ani);

	void Load(char *fileName, int aheight, int aattack);
	void Move(XYZ how);

protected:
	void deallocate();
};


class Skeleton
{
public:
	int num_joints;
	//Joint joints[max_joints];
	//Joint *joints;
	Joint* joints;

	int num_muscles;
	//Muscle muscles[max_muscles];
	//Muscle *muscles;
	Muscle* muscles;

	int selected;

	int forwardjoints[3];
	XYZ forward;

	int id;

	int lowforwardjoints[3];
	XYZ lowforward;

	XYZ specialforward[5];
	int jointlabels[max_joints];

	Model model[7];
	Model modellow;
	Model modelclothes;
	int num_models;

	Model drawmodel;
	Model drawmodellow;
	Model drawmodelclothes;

	bool clothes;
	bool spinny;

	GLubyte skinText[512*512*3];
	int skinsize;

	float checkdelay;

	float longdead;
	bool broken;

	int free;
	int oldfree;
	float freetime;
	bool freefall;

	void FindForwards();
	void FindForwardsfirst();
	float DoConstraints(XYZ *coords,float *scale);
	void DoGravity(float *scale);
	void DoBalance();
	void MusclesSet();
	void Draw(int muscleview);
	void AddJoint(float x, float y, float z, int which);
	void SetJoint(float x, float y, float z, int which, int whichjoint);
	void DeleteJoint(int whichjoint);
	void AddMuscle(int attach1,int attach2,float maxlength,float minlength,int type);
	void DeleteMuscle(int whichmuscle);
	void FindRotationJoint(int which);
	void FindRotationJointSameTwist(int which);
	void FindRotationMuscle(int which, int animation);
	void Load(char *fileName,char *lowfileName,char *clothesfileName,char *modelfileName,char *model2fileName,char *model3fileName,char *model4fileName,char *model5fileNamee,char *model6fileName,char *model7fileName,char *modellowfileName,char *modelclothesfileName, bool aclothes);

	Skeleton();

	~Skeleton();
};

#endif
