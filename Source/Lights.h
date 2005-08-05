#ifndef _LIGHTS_H_
#define _LIGHTS_H_


/**> HEADER FILES <**/
#include "gamegl.h"
#include "Quaternions.h"

class Light{
public:
	GLint type;
	GLfloat color[3];
	GLfloat ambient[3];
	int attach;
	XYZ location;
};

void SetUpMainLight(Light* whichsource, int whichlight, float ambientr, float ambientg, float ambientb);
void SetUpLight(Light* whichsource, int whichlight);

#endif