#ifndef _MENU_H_
#define _MENU_H_

#include "Game.h"

namespace Menu {
    void clearMenu();
    void addLabel(int id,const string& text,int x,int y,float r=1,float g=0,float b=0);
    void addButton(int id,const string& text,int x,int y,float r=1,float g=0,float b=0);
    void addImage(int id,int texture,int x,int y,int w,int h,float r=1,float g=1,float b=1);
    void addButtonImage(int id,int texture,int x,int y,int w,int h,float r=1,float g=1,float b=1);
    void addMapLine(int x, int y, int w, int h, float startsize, float endsize, float r,float g,float b);
    void addMapMarker(int id,int texture,int x,int y,int w,int h,float r,float g,float b);
    void addMapLabel(int id,const string& text,int x,int y,float r=1,float g=0,float b=0);
    void setText(int id,const string& text);
    void setText(int id,const string& text,int x,int y,int w,int h);
    int getSelected(int mousex, int mousey);
    void drawItems();
}

#endif
