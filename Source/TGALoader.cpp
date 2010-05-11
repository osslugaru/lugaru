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

/**> HEADER FILES <**/
#include "TGALoader.h"
#include "Game.h"

extern float texdetail;
extern TGAImageRec texture;
extern short vRefNum;
extern long dirID;
extern bool visibleloading;
extern Game * pgame;
extern int loadscreencolor;

extern bool LoadImage(const char * fname, TGAImageRec & tex);
/********************> LoadTGA() <*****/
bool upload_image(const unsigned char* filePath, bool hasalpha)
{
	if(visibleloading){
		loadscreencolor=1;
		pgame->LoadingScreen();
	}

#if !PLATFORM_MACOSX

	// for Windows, just use TGA loader for now
	char fileName[ 256];
	CopyPascalStringToC( filePath, fileName);
/*
	// change extension to .TGA
	int len = strlen( fileName);
	if (len > 3)
	{
		fileName[ len - 3] = 't';
		fileName[ len - 2] = 'g';
		fileName[ len - 1] = 'a';
	}
*/
//	return (LoadTGA( fileName) != NULL);
	return (LoadImage(fileName, texture));

#else

	OSStatus err;
	ComponentResult cr;

	/*FSRef fsref;
	Boolean isdir;
	err = FSPathMakeRef((const UInt8*)filePath, &fsref, &isdir);
	if(err)return;

	FSSpec fsspec;
	err = FSGetCatalogInfo(&fsref, kFSCatInfoNone, NULL, NULL, &fsspec, NULL);
	if(err)return;
	*/

	//Boolean isdir;
	FSSpec fsspec;
	//err = FSMakeFSSpec (0, 0, (const unsigned char*)filePath, &fsspec);
	err = FSMakeFSSpec (0, 0, filePath, &fsspec);
	//err=FSPathMakeFSSpec((const UInt8*)filePath,&fsspec,&isdir);*/
	if(err)return;

	GraphicsImportComponent gi;
	err = GetGraphicsImporterForFile(&fsspec, &gi);
	if(err)return;

	Rect natbounds;
	cr = GraphicsImportGetNaturalBounds(gi, &natbounds);

	size_t buffersize = 4 * natbounds.bottom * natbounds.right;
	//void* buf = malloc(buffersize);
	texture.sizeX=natbounds.right;
	texture.sizeY=natbounds.bottom;
	/*if(hasalpha)*/texture.bpp = 32;
	//if(!hasalpha)texture.bpp = 24;

	GWorldPtr gw;
	err = QTNewGWorldFromPtr(&gw, k32ARGBPixelFormat, &natbounds, NULL, NULL,
		0, texture.data, 4 * natbounds.right);
	if(err)return;

	cr = GraphicsImportSetGWorld(gi, gw, NULL);

	natbounds.top = natbounds.bottom;
	natbounds.bottom = 0;

	cr = GraphicsImportSetBoundsRect(gi, &natbounds);

	cr = GraphicsImportDraw(gi);

	err = CloseComponent(gi);
	if(err)return;

	/*glTexImage2D(textureTarget, 0, GL_RGBA, natbounds.right, natbounds.top, 0,
	GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buf);
	*/

	//free(buf);
	DisposeGWorld(gw);

	// Loop Through The Image Data
	GLuint			imageSize;									// Used To Store The Image Size When Setting Aside Ram
	GLuint			temp;										// Temporary Variable
	GLuint			bytesPerPixel;										// Temporary Variable
	bytesPerPixel=texture.bpp/8;
	imageSize = texture.sizeX * texture.sizeY * bytesPerPixel;
	int alltrans=10;

	for( GLuint i = 0; i < int( imageSize ); i += 4 )
	{
		// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp = texture.data[i];					// Temporarily Store The Value At Image Data 'i'
		texture.data[i] = texture.data[i + 1];	// Set The 1st Byte To The Value Of The 3rd Byte
		texture.data[i + 1] = texture.data[i + 2];				// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
		texture.data[i + 2] = texture.data[i + 3];
		texture.data[i + 3] = temp;
	}

	int tempplace;
	tempplace=0;
	if(!hasalpha){
		for( GLuint i = 0; i < int( imageSize ); i += 4 )
		{
			texture.data[i + 3] = 255;
			/*texture.data[tempplace] = texture.data[i];	// Set The 1st Byte To The Value Of The 3rd Byte
			texture.data[tempplace + 1] = texture.data[i + 1];				// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
			texture.data[tempplace + 2] = texture.data[i + 2];
			tempplace+=3;*/
		}
	}

	if(texdetail>1){
		int which=0;
		float temp;
		float howmany;
		for( GLuint k = 0; k < int( imageSize); k += bytesPerPixel*texture.sizeX*texdetail )
		{
			for( GLuint i = 0; i < int( imageSize/texture.sizeY ); i += bytesPerPixel*texdetail )
			{
				for( GLuint b = 0; b < bytesPerPixel ; b ++ ){	
					temp=0;
					howmany=0;
					for( GLuint l = 0; l < texdetail*texture.sizeX ; l +=texture.sizeX ){
						for( GLuint j = 0; j < texdetail ; j ++ )
						{
							temp += (int)texture.data[k+i+j*bytesPerPixel+l*bytesPerPixel+b];	// Set The 1st Byte To The Value Of The 3rd Byte
							howmany++;
						}
					}
					texture.data[which+b]=GLubyte(temp/howmany);
				}
				which+=bytesPerPixel;
			}
		}
		texture.sizeX/=texdetail;
		texture.sizeY/=texdetail;
	}

	return true;

#endif
}


TGAImageRec*	LoadTGA( char *filename )
{    
	GLubyte			TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	GLubyte			TGAcompare[12];								// Used To Compare TGA Header
	GLubyte			header[6];									// First 6 Useful Bytes From The Header
	GLuint			bytesPerPixel;								// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint			imageSize;									// Used To Store The Image Size When Setting Aside Ram
	GLuint			temp;										// Temporary Variable
	GLuint			type = GL_RGBA;								// Set The Default GL Mode To RBGA (32 BPP)
	//TGAImageRec		*texture;
	FILE			*file;

	// Open The TGA File
	file = fopen( filename, "rb" );

	if( ( file == NULL ) ||	// Does File Even Exist?
		( fread( TGAcompare, 1, sizeof( TGAcompare ), file ) != sizeof( TGAcompare ) ) ||	// Are There 12 Bytes To Read?
		( memcmp( TGAheader, TGAcompare, sizeof( TGAheader ) ) != 0 ) ||	// Does The Header Match What We Want?
		( fread( header, 1, sizeof( header ), file ) != sizeof( header ) ) )// If So Read Next 6 Header Bytes
	{
		// If anything failed then close the file and return false
		if (file) fclose( file );
		return NULL;
	}

	// Create a new RGBAImageRec
	//texture = ( TGAImageRec* )malloc( sizeof( TGAImageRec ) );

	// Determine the TGA width (highbyte*256+lowbyte) and height (highbyte*256+lowbyte)
	texture.sizeX  = (header[1] * 256 + header[0]);
	texture.sizeY = (header[3] * 256 + header[2]);

	// Make sure the height, width, and bit depth are valid
	if(	( texture.sizeX <= 0 ) || ( texture.sizeY <= 0 ) || ( ( header[4] != 24 ) && ( header[4] != 32 ) ) )
	{
		// If anything failed then close the file, free up memory for the image, and return NULL
		fclose( file );
		//free( texture );
		return NULL;
	}

	// Grab The TGA's Bits Per Pixel (24 or 32)
	texture.bpp = header[4];							
	bytesPerPixel = texture.bpp/8;	// Divide By 8 To Get The Bytes Per Pixel

	// Calculate The Memory Required For The TGA Data
	imageSize = texture.sizeX * texture.sizeY * bytesPerPixel;

	// Reserve Memory To Hold The TGA Data
	//texture.data = ( GLubyte* )malloc( imageSize );		

	// Make sure the right amount of memory was allocated
	if(	( texture.data == NULL ) || ( fread( texture.data, 1, imageSize, file ) != imageSize ) )
	{
		// Free up the image data if there was any
		//		if( texture.data != NULL )
		//			free( texture.data );

		// If anything failed then close the file, free up memory for the image, and return NULL
		fclose( file );
		//		free( texture );
		return NULL;
	}

	// Loop Through The Image Data
	for( GLuint i = 0; i < int( imageSize ); i += bytesPerPixel )
	{
		// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp = texture.data[i];					// Temporarily Store The Value At Image Data 'i'
		texture.data[i] = texture.data[i + 2];	// Set The 1st Byte To The Value Of The 3rd Byte
		texture.data[i + 2] = temp;				// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	if(texdetail>1){
		int which=0;
		float temp;
		float howmany;
		for( GLuint k = 0; k < int( imageSize); k += bytesPerPixel*texture.sizeX*texdetail )
		{
			for( GLuint i = 0; i < int( imageSize/texture.sizeY ); i += bytesPerPixel*texdetail )
			{
				for( GLuint b = 0; b < bytesPerPixel ; b ++ ){	
					temp=0;
					howmany=0;
					for( GLuint l = 0; l < texdetail*texture.sizeX ; l +=texture.sizeX ){
						for( GLuint j = 0; j < texdetail ; j ++ )
						{
							temp += (int)texture.data[k+i+j*bytesPerPixel+l*bytesPerPixel+b];	// Set The 1st Byte To The Value Of The 3rd Byte
							howmany++;
						}
					}
					texture.data[which+b]=GLubyte(temp/howmany);
				}
				which+=bytesPerPixel;
			}
		}
		texture.sizeX/=texdetail;
		texture.sizeY/=texdetail;
	}

	// Close The File
	fclose( file );
	return &texture;
}
