
#ifndef STEREO_H_
#define STEREO_H_

enum StereoMode {
	stereoNone,
	stereoAnaglyph,             /* red/cyan */
	stereoHorizontalInterlaced, /* some 3D monitors */
	stereoVerticalInterlaced,
	stereoHorizontalSplit,      /* cross-eyed view */
	stereoVerticalSplit,
	stereoOpenGL,               /* Whatever OpenGL does, if supported */
	stereoCount                 /* must be last element */
};


enum StereoSide {
	// Code multiplies by StereoSide to calculate camera offsets
	stereoLeft   = -1,
	stereoCenter = 0,
	stereoRight  = 1
};

extern StereoMode stereomode;
extern StereoMode newstereomode;
extern float stereoseparation;
extern bool  stereoreverse;

bool CanInitStereo(StereoMode mode);
void InitStereo(StereoMode mode);
const char* StereoModeName(StereoMode mode);

#endif