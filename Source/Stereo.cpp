
#include "Game.h"
#include "Stereo.h"


extern int kContextWidth;
extern int kContextHeight;

bool CanInitStereo(StereoMode mode) {
	GLint stencilbits = 0;
	
	switch(mode) {
		case stereoNone:
			return true;
			break;
		case stereoAnaglyph:
			return true;
			break;
		case stereoHorizontalInterlaced:
		case stereoVerticalInterlaced:
			glGetIntegerv(GL_STENCIL_BITS, &stencilbits);
			if ( stencilbits < 1 ) {
				fprintf(stderr, "Failed to get a stencil buffer, interlaced stereo not available.\n");
				return false;
			} else {
				fprintf(stderr, "Stencil buffer has %i bits, good.\n", stencilbits);
			}
			return true;
			break;
		default:
			return false;
	}

}

void InitStereo(StereoMode mode) {
	switch(mode) {
		case stereoNone:
			glDisable( GL_STENCIL_TEST);
			return;
		case stereoAnaglyph:
			glDisable( GL_STENCIL_TEST);
			return;
		case stereoHorizontalInterlaced:
		case stereoVerticalInterlaced:
			
			fprintf(stderr, "Screen width is %i, height is %i\n", kContextWidth, kContextHeight);
			
			glEnable( GL_STENCIL_TEST);
			glClearStencil(0);
			glClear(  GL_STENCIL_BUFFER_BIT );
			glStencilFunc(GL_ALWAYS, 0x1, 0x1);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			
			
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 3);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
			glColorMask( 1.0, 1.0, 1.0, 1.0 );
			char stencil[] = {64,127,255};
			
			glViewport(0,0, kContextWidth, kContextHeight);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho((GLdouble)0, (GLdouble)kContextWidth, (GLdouble)kContextHeight, 0, -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			
			for(int y=0;y<kContextHeight;y+=2) {
				
				for(int x=0;x<kContextWidth;x++) {
					glRasterPos2i(x, y);
					glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_BYTE, &stencil);
				}
			}
			
			glStencilFunc(GL_NOTEQUAL, 0x01, 0x01);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

			// Something gets screwed up due to the changes above
			// revert to default.
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	}
	
}

const char* StereoModeName(StereoMode mode) {
	switch(mode) {
		case stereoNone:                 return "None"; break;
		case stereoAnaglyph:             return "Anaglyph"; break;
		case stereoHorizontalInterlaced: return "Horizontal interlacing"; break;
		case stereoVerticalInterlaced:   return "Vertical interlacing"; break;
		case stereoHorizontalSplit:      return "Horizontal split"; break;
		case stereoVerticalSplit:        return "Vertical split"; break;
		case stereoOpenGL:               return "OpenGL"; break;
		default:                         return "(error)"; break; 
	}
}