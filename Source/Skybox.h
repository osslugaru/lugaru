#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "Quaternions.h"
#include "TGALoader.h"
#include "Quaternions.h"
#include "gamegl.h"

class SkyBox{
public:
	GLuint 				front,left,back,right,up,down,cloud,reflect;
	float cloudmove;

	bool load(char *ffront,char *fleft,char *fback,char *fright,char *fup,char *fdown,char *fcloud,char *freflect);
	void draw();

	SkyBox();
	~SkyBox();
};

#endif