#ifndef _SPRITES_H_
#define _SPRITES_H_

#include "Quaternions.h"
#include "gamegl.h"
#include "TGALoader.h"
#include "Quaternions.h"
#include "Frustum.h"
#include "Lights.h"
#include "Terrain.h"
#include "Objects.h"
//
// Model Structures
//

#define max_sprites 20000

#define cloudsprite 0
#define bloodsprite 1
#define flamesprite 2
#define smoketype 3
#define weaponflamesprite 4
#define cloudimpactsprite 5
#define snowsprite 6
#define weaponshinesprite 7
#define bloodflamesprite 8
#define breathsprite 9
#define splintersprite 10

class Sprites{
public:
	GLuint cloudtexture;
	GLuint cloudimpacttexture;
	GLuint bloodtexture;
	GLuint flametexture;
	GLuint bloodflametexture;
	GLuint smoketexture;
	GLuint snowflaketexture;
	GLuint shinetexture;
	GLuint splintertexture;
	GLuint leaftexture;
	GLuint toothtexture;

	XYZ oldposition[max_sprites];
	XYZ position[max_sprites];
	XYZ velocity[max_sprites];
	float size[max_sprites];
	float initialsize[max_sprites];
	int type[max_sprites];
	int special[max_sprites];
	float color[max_sprites][3];
	float opacity[max_sprites];
	float rotation[max_sprites];
	float alivetime[max_sprites];
	float speed[max_sprites];
	float rotatespeed[max_sprites];
	float checkdelay;
	int numsprites;

	void DeleteSprite(int which);
	void MakeSprite(int atype, XYZ where, XYZ avelocity, float red, float green, float blue, float asize, float aopacity);
	void Draw();

	Sprites();
	~Sprites();
};

#endif
