
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
			
			// Setup stencil buffer
			glEnable( GL_STENCIL_TEST);
			glClearStencil(0);
			glClear(  GL_STENCIL_BUFFER_BIT );
			glStencilFunc(GL_ALWAYS, 0x1, 0x1);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
			
			// Setup viewport
			glViewport(0,0, kContextWidth, kContextHeight);
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
				glLoadIdentity();
				glOrtho((GLdouble)0, (GLdouble)kContextWidth, (GLdouble)kContextHeight, 0, -1, 1);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
					glLoadIdentity();
					
					glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
					
					if ( mode == stereoHorizontalInterlaced ) {
						for(int y=0;y<kContextHeight;y+=2) {
							glBegin(GL_LINES);
								glVertex3f(0, y, 0);
								glVertex3f(kContextWidth, y, 0);
							glEnd();
						}
					} else {
						for(int x=0;x<kContextWidth;x+=2) {
							glBegin(GL_LINES);
								glVertex3f(x, 0, 0);
								glVertex3f(x, kContextHeight, 0);
							glEnd();
						}
					}
					
					glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
					
				glPopMatrix();
				glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			
			glStencilFunc(GL_NOTEQUAL, 0x01, 0x01);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
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