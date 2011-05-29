#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <map>
#include <string>

//keeps track of which textures are loaded
class Texture {
	private:
		static std::map<std::string,Texture> textures;
	
		bool isSkin;
		std::string fileName;
		GLuint id;
		bool mipmap;
		bool hasalpha;
		GLubyte* array;
		int arraySize;
		int skinsize;

		void load();

	public:
		Texture():
			isSkin(false), skinsize(0), arraySize(0),
			fileName(""), id(0), mipmap(false), hasalpha(false), array(NULL) { }
		~Texture();
		Texture (const std::string& _fileName, bool _mipmap, bool _hasalpha):
			isSkin(false), skinsize(0), arraySize(0), array(NULL),
			fileName(_fileName), id(0), mipmap(_mipmap), hasalpha(_hasalpha) { }
		Texture (const std::string& _fileName, bool _mipmap, bool _hasalpha, bool _isSkin):
			isSkin(_isSkin), skinsize(0), arraySize(0), array(NULL),
			fileName(_fileName), id(0), mipmap(_mipmap), hasalpha(_hasalpha) { }
		GLuint getId() const { return id; }
		
		static GLuint Load(const std::string& fileName, bool mipmap, bool hasalpha);
		static GLuint Load(const std::string& fileName, bool mipmap, bool hasalpha, GLubyte* array, int* skinsize);
};

#endif
