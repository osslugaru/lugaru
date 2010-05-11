/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

GL_FUNC(void,glAlphaFunc,(GLenum f,GLclampf x),(f,x),)
GL_FUNC(void,glBegin,(GLenum e),(e),)
GL_FUNC(void,glBindTexture,(GLenum target,GLuint name),(target,name),)
GL_FUNC(void,glBlendFunc,(GLenum f,GLenum x),(f,x),)
GL_FUNC(void,glCallLists,(GLsizei a,GLenum b,const GLvoid* c),(a,b,c),)
GL_FUNC(void,glClear,(GLbitfield a),(a),)
GL_FUNC(void,glClearColor,(GLclampf r,GLclampf g,GLclampf b,GLclampf a),(r,g,b,a),)
GL_FUNC(void,glClearDepth,(GLclampd x),(x),)
GL_FUNC(void,glColor3f,(GLfloat r,GLfloat g,GLfloat b),(r,g,b),)
GL_FUNC(void,glColor4f,(GLfloat r,GLfloat g,GLfloat b,GLfloat a),(r,g,b,a),)
GL_FUNC(void,glColorMask,(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha),(red,green,blue,alpha),)
GL_FUNC(void,glColorPointer,(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer),(size, type, stride, pointer),)
GL_FUNC(void,glCopyTexImage2D,(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border),(target, level, internalFormat, x, y, width, height, border),)
GL_FUNC(void,glCopyTexSubImage2D,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height),(target, level, xoffset, yoffset, x, y, width, height),)
GL_FUNC(void,glCullFace,(GLenum mode),(mode),)
GL_FUNC(void,glDeleteLists,(GLuint list, GLsizei range),(list,range),)
GL_FUNC(void,glDeleteTextures,(GLsizei n, const GLuint *textures),(n,textures),)
GL_FUNC(void,glDepthFunc,(GLenum func),(func),)
GL_FUNC(void,glDepthMask,(GLboolean flag),(flag),)
GL_FUNC(void,glDisable,(GLenum cap),(cap),)
GL_FUNC(void,glDisableClientState,(GLenum array),(array),)
GL_FUNC(void,glDrawArrays,(GLenum mode, GLint first, GLsizei count),(mode,first,count),)
GL_FUNC(void,glDrawBuffer,(GLenum mode),(mode),)
GL_FUNC(void,glEnable,(GLenum cap),(cap),)
GL_FUNC(void,glEnableClientState,(GLenum array),(array),)
GL_FUNC(void,glEnd,(void),(),)
GL_FUNC(void,glEndList,(void),(),)
GL_FUNC(void,glFinish,(void),(),)
GL_FUNC(GLuint,glGenLists,(GLsizei range),(range),return)
GL_FUNC(void,glGenTextures,(GLsizei n, GLuint *textures),(n,textures),)
GL_FUNC(GLenum,glGetError,(void),(),return)
GL_FUNC(void,glGetFloatv,(GLenum pname, GLfloat *params),(pname,params),)
GL_FUNC(void,glHint,(GLenum target, GLenum mode),(target,mode),)
GL_FUNC(void,glInterleavedArrays,(GLenum format, GLsizei stride, const GLvoid *pointer),(format,stride,pointer),)
GL_FUNC(GLboolean,glIsTexture,(GLuint texture),(texture),return)
GL_FUNC(void,glLightfv,(GLenum light, GLenum pname, const GLfloat *params),(light,pname,params),)
GL_FUNC(void,glListBase,(GLuint base),(base),)
GL_FUNC(void,glLoadIdentity,(void),(),)
GL_FUNC(void,glLoadMatrixf,(const GLfloat *m),(m),)
GL_FUNC(void,glMatrixMode,(GLenum mode),(mode),)
GL_FUNC(void,glNewList,(GLuint list, GLenum mode),(list,mode),)
GL_FUNC(void,glNormal3f,(GLfloat nx, GLfloat ny, GLfloat nz),(nx,ny,nz),)
GL_FUNC(void,glOrtho,(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar),(left,right,bottom,top,zNear,zFar),)
GL_FUNC(void,glPixelStorei,(GLenum pname, GLint param),(pname,param),)
GL_FUNC(void,glPixelTransferi,(GLenum pname, GLint param),(pname,param),)
GL_FUNC(void,glPointSize,(GLfloat size),(size),)
GL_FUNC(void,glPopMatrix,(void),(),)
GL_FUNC(void,glPushMatrix,(void),(),)
GL_FUNC(void,glReadBuffer,(GLenum mode),(mode),)
GL_FUNC(void,glReadPixels,(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels),(x,y,width,height,format,type,pixels),)
GL_FUNC(void,glRotatef,(GLfloat angle, GLfloat x, GLfloat y, GLfloat z),(angle,x,y,z),)
GL_FUNC(void,glScalef,(GLfloat x, GLfloat y, GLfloat z),(x,y,z),)
GL_FUNC(void,glShadeModel,(GLenum mode),(mode),)
GL_FUNC(void,glTexCoord2f,(GLfloat s, GLfloat t),(s,t),)
GL_FUNC(void,glTexCoordPointer,(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer),(size,type,stride,pointer),)
GL_FUNC(void,glTexEnvf,(GLenum target, GLenum pname, GLfloat param),(target,pname,param),)
GL_FUNC(void,glTexEnvfv,(GLenum target, GLenum pname, const GLfloat *params),(target,pname,params),)
GL_FUNC(void,glTexEnvi,(GLenum target, GLenum pname, GLint param),(target,pname,param),)
GL_FUNC(void,glTexParameterf,(GLenum target, GLenum pname, GLfloat param),(target,pname,param),)
GL_FUNC(void,glTexParameteri,(GLenum target, GLenum pname, GLint param),(target,pname,param),)
GL_FUNC(void,glTexSubImage2D,(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels),(target,level,xoffset,yoffset,width,height,format,type,pixels),)
GL_FUNC(void,glTranslated,(GLdouble x, GLdouble y, GLdouble z),(x,y,z),)
GL_FUNC(void,glTranslatef,(GLfloat x, GLfloat y, GLfloat z),(x,y,z),)
GL_FUNC(void,glVertex2i,(GLint x, GLint y),(x,y),)
GL_FUNC(void,glVertex3f,(GLfloat x, GLfloat y, GLfloat z),(x,y,z),)
GL_FUNC(void,glVertexPointer,(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer),(size,type,stride,pointer),)
GL_FUNC(void,glViewport,(GLint x, GLint y, GLsizei width, GLsizei height),(x,y,width,height),)
GL_FUNC(void,glLockArraysEXT,(GLint first, GLsizei count),(first,count),)
GL_FUNC(void,glUnlockArraysEXT,(void),(),)

// stuff GLU needs...
GL_FUNC(void,glGetIntegerv,(GLenum pname, GLint *params),(pname,params),)
GL_FUNC(const GLubyte *,glGetString,(GLenum name),(name),return)
GL_FUNC(void,glGetTexLevelParameteriv,(GLenum target, GLint level, GLenum pname, GLint *params),(target,level,pname,params),)
GL_FUNC(void,glMultMatrixd,(const GLdouble *m),(m),)
GL_FUNC(void,glMultMatrixf,(const GLfloat *m),(m),)
GL_FUNC(void,glGetTexImage,(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels),(target,level,format,type,pixels),)
GL_FUNC(void,glTexImage1D,(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels),(target,level,internalFormat,width,border,format,type,pixels),)
GL_FUNC(void,glTexImage2D,(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels),(target,level,internalFormat,width,height,border,format,type,pixels),)
GL_FUNC(void,glTexImage3D,(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels),(target,level,internalformat,width,height,depth,border,format,type,pixels),)

