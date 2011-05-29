#ifndef _MENU_H_
#define _MENU_H_

#include "Game.h"

namespace Menu {
    typedef void(*MBCallback)(int id);

    void GUITick();
    void clearMenu();
    void addLabel(int id,const string& label,int x,int y);
    void addButton(int id,const string& label,MBCallback cb,int x,int y,int w,int h,float r=1,float g=0,float b=0);
    void addImage(int id,int texture,int x,int y,int w,int h);
    void addImageButton(int id,int texture,MBCallback cb,int x,int y,int w,int h,float r=1,float g=1,float b=1);
    void addMapLine(int x, int y, int w, int h, float startsize, float endsize, float r,float g,float b);
    void addMapMarker(int id,int texture,MBCallback cb,int x,int y,int w,int h,float r,float g,float b);
    void setMapItem(int id);
    void setButtonText(int id,const string& label);
    void setButtonText(int id,const string& label,int x,int y,int w,int h);
    int getSelected(int mousex, int mousey);
    void drawItems();
}

#endif
