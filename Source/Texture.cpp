#include "gamegl.h"
#include "Texture.h"
#include "TGALoader.h"

using namespace std;

map<string,Texture> Texture::textures;

extern TGAImageRec texture;
extern bool trilinear;

void Texture::load()  {
	GLuint type;

	LOGFUNC;

	LOG(std::string("Loading texture...") + fileName);

	unsigned char fileNamep[256];
	CopyCStringToPascal(ConvertFileName(fileName.c_str()), fileNamep);
	//Load Image
	upload_image( fileNamep ,hasalpha);
	
	//Alpha channel?
	if ( texture.bpp == 24 )
		type = GL_RGB;
	else
		type = GL_RGBA;

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	if(!id)
		glGenTextures( 1, &id );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, id);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	if(mipmap)
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (trilinear?GL_LINEAR_MIPMAP_LINEAR:GL_LINEAR_MIPMAP_NEAREST) );
	else
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	
	skinsize=texture.sizeX;
	
	if(isSkin) {
		int tempnum=0;
		int nb = (texture.sizeY*texture.sizeX*(texture.bpp/8));
		array = (GLubyte*)malloc(nb*sizeof(GLubyte));
		for(int i=0;i<nb;i++) {
			if((i+1)%4||type==GL_RGB) {
				array[tempnum]=texture.data[i];
				tempnum++;
			}
		}
		arraySize=tempnum;

		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, GL_RGB, GL_UNSIGNED_BYTE, array );
	} else {
		gluBuild2DMipmaps( GL_TEXTURE_2D, type, texture.sizeX, texture.sizeY, type, GL_UNSIGNED_BYTE, texture.data );
	}
}

GLuint Texture::Load(const string& fileName, bool mipmap, bool hasalpha) {
	map<string,Texture>::iterator it = textures.find(fileName);
	if(it==textures.end()) {
		textures.insert(make_pair(fileName,Texture(fileName,mipmap,hasalpha)));
		textures[fileName].load();
		return textures[fileName].getId();
	} else {
		return it->second.getId();
	}
}

GLuint Texture::Load(const string& fileName, bool mipmap, bool hasalpha, GLubyte* array, int* skinsize) {
	map<string,Texture>::iterator it = textures.find(fileName);
	if(it==textures.end()) {
		textures.insert(make_pair(fileName,Texture(fileName,mipmap,hasalpha,true)));
		textures[fileName].load();
		*skinsize = textures[fileName].skinsize;
		for(int i=0;i<textures[fileName].arraySize;i++) {
			array[i] = textures[fileName].array[i];
		}
		return textures[fileName].getId();
	} else {
		*skinsize = it->second.skinsize;
		for(int i=0;i<it->second.arraySize;i++) {
			array[i] = it->second.array[i];
		}
		return it->second.getId();
	}
}

