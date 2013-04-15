#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <map>
#include <vector>
#include <string>
using namespace std;

class TextureRes;

class Texture
{
private:
    TextureRes* tex;
public:
    inline Texture(): tex(NULL) {}
    void load(const string& filename, bool hasMipmap, bool hasAlpha);
    void load(const string& filename, bool hasMipmap, GLubyte* array, int* skinsizep);
    void destroy();
    void bind();

    static void reloadAll();
};

#endif
