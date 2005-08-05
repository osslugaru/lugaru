#ifndef _TEXT_H_
#define _TEXT_H_


/**> HEADER FILES <**/
#include "Quaternions.h"
//#include "Files.h"
#include "Quaternions.h"
#include "gl.h"
#include "TGALoader.h"

class Text{
public:
	GLuint FontTexture;
	GLuint base;

	void LoadFontTexture(char *fileName);
	void BuildFont();
	void glPrint(float x, float y, char *string, int set, float size, float width, float height);
	void glPrintOutline(float x, float y, char *string, int set, float size, float width, float height);
	void glPrint(float x, float y, char *string, int set, float size, float width, float height,int start,int end);
	void glPrintOutline(float x, float y, char *string, int set, float size, float width, float height,int start,int end);
	void glPrintOutlined(float x, float y, char *string, int set, float size, float width, float height);
	void glPrintOutlined(float r, float g, float b, float x, float y, char *string, int set, float size, float width, float height);

	Text();
	~Text();
};

#endif