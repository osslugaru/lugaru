#include "Game.h"
#include "nommgr.h"

extern bool buttons[3];
extern float multiplier;
extern float screenwidth,screenheight;
extern float sps;
extern float realmultiplier;
extern int slomo;
extern bool ismotionblur;
extern float usermousesensitivity;
extern int detail;
extern bool floatjump;
extern bool cellophane;
extern int terraindetail;
extern int texdetail;
extern int bloodtoggle;
extern bool osx;
extern bool autoslomo;
extern bool foliage;
extern bool musictoggle;
extern bool trilinear;
extern float gamespeed;
extern int difficulty;
extern bool damageeffects;
extern int numplayers;
extern bool decals;
extern bool invertmouse;
extern bool texttoggle;
extern bool ambientsound;
extern bool mousejump;
extern bool freeze;
extern Person player[maxplayers];
extern bool vblsync;
extern bool stillloading;
extern bool showpoints;
extern bool alwaysblur;
extern bool immediate;
extern bool velocityblur;
extern bool debugmode;
extern int mainmenu;
extern int kBitsPerPixel;
extern float slomospeed;
extern float slomofreq;
extern float oldgamespeed;

 #define kUseAGLFullScreenX 1
 #define kUseFades 1
 
// system includes ----------------------------------------------------------

#ifdef __APPLE_CC__
    #include "Carbon Include.h"
    #include <Carbon/Carbon.h>
    
    #include <DrawSprocket/DrawSprocket.h>
	#include <AGL/agl.h>
	#include <AGL/aglRenderers.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
    #include <DrawSprocket.h>
    
    #include <Devices.h>
    #include <Dialogs.h>
    #include <DriverServices.h>
    #include <Events.h>
    #include <Gestalt.h>
    #include <LowMem.h>
    #include <Sound.h>
    #include <TextEdit.h>
    #include <ToolUtils.h>
    #include <Windows.h>

    // profile
    #include <Profiler.h>

	#include "agl.h"
    #include "gamegl.h"
	#include "glu.h"
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <Folders.h>
#include <fstream.h>
 
// project includes ---------------------------------------------------------

#include "aglString.h"
#include "MacInput.h"

// functions (internal/private) ---------------------------------------------

static UInt32 CheckMacOSX (void);
void CToPStr (StringPtr outString, const char *inString);
void ReportError (char * strError);
OSStatus DSpDebugStr (OSStatus error);
GLenum aglDebugStr (void);
GLenum glDebugStr (void);

CGrafPtr SetupDSpFullScreen (GDHandle hGD);
void ShutdownDSp (CGrafPtr pDSpPort);

AGLContext SetupAGL (GDHandle hGD, AGLDrawable win);
AGLContext SetupAGLFullScreen (GDHandle display, short * pDepth, short * pWidth, short * pHeight);
void CleanupAGL (AGLContext ctx);
void DrawGL (Rect * pRectPort);

void InitToolbox(void);
void CreateGLWindow (void);
Boolean SetUp (void);
void DoMenu (SInt32 menuResult);
void DoKey (SInt8 theKey, SInt8 theCode);
void DoUpdate (void);

pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData);
EventLoopTimerUPP GetTimerUPP (void);
static Boolean RunningOnCarbonX(void);

void DoEvent (void);
void CleanUp (void);

// profile wrappers
Boolean WaitNextEventWrapper (EventMask eventMask, EventRecord *theEvent, unsigned long sleep,RgnHandle mouseRgn);
OSStatus DSpProcessEventWrapper (EventRecord *inEvent, Boolean *outEventWasProcessed);
void UpdateWrapper (EventRecord *theEvent);

// statics/globals (internal only) ------------------------------------------

Point delta;

// Menu defs
enum 
{
	kMenuApple = 128,
	kMenuFile = 129,
	
	kAppleAbout = 1,
	kFileQuit = 1
};

enum 
{
	kForegroundSleep = 10,
	kBackgroundSleep = 10000
};
EventLoopTimerRef gTimer = NULL;

int kContextWidth;
int kContextHeight;

const RGBColor rgbBlack = { 0x0000, 0x0000, 0x0000 };

NumVersion gVersionDSp;
DSpContextAttributes gContextAttributes;
DSpContextReference gContext = 0;
extern AGLContext gaglContext;
GLuint gFontList;
char gcstrMode [256] = "";

AGLDrawable gpDSpPort = NULL; // will be NULL for full screen under X
Rect gRectPort = {0, 0, 0, 0};

UInt32 gSleepTime = kForegroundSleep;
Boolean gDone = false, gfFrontProcess = true;

Game game;

// profile
#if __profile__
OSErr gProfErr = noErr;
#endif

#pragma mark -

//-----------------------------------------------------------------------------------------------------------------------

// are we running on Mac OS X
// returns 0 if < Mac OS X or version number of Mac OS X (10.0 for GM)

static UInt32 CheckMacOSX (void)
{
	UInt32 response;
    
	if ((Gestalt(gestaltSystemVersion, (SInt32 *) &response) == noErr) && (response >= 0x01000))
		return response;
	else
		return 0;
}

//-----------------------------------------------------------------------------------------------------------------------

// Copy C string to Pascal string

void CToPStr (StringPtr outString, const char *inString)
{	
	unsigned char x = 0;
	do
		*(((char*)outString) + x + 1) = *(inString + x++);
	while ((*(inString + x) != 0)  && (x < 256));
	*((char*)outString) = (char) x;									
}

// --------------------------------------------------------------------------

void ReportError (char * strError)
{
	char errMsgCStr [256];
	Str255 strErr;

	sprintf (errMsgCStr, "%s", strError); 

	// out as debug string
	CToPStr (strErr, errMsgCStr);
	DebugStr (strErr);
}

//-----------------------------------------------------------------------------------------------------------------------

OSStatus DSpDebugStr (OSStatus error)
{
	switch (error)
	{
		case noErr:
			break;
		case kDSpNotInitializedErr:
			ReportError ("DSp Error: Not initialized");
			break;
		case kDSpSystemSWTooOldErr:
			ReportError ("DSp Error: system Software too old");
			break;
		case kDSpInvalidContextErr:
			ReportError ("DSp Error: Invalid context");
			break;
		case kDSpInvalidAttributesErr:
			ReportError ("DSp Error: Invalid attributes");
			break;
		case kDSpContextAlreadyReservedErr:
			ReportError ("DSp Error: Context already reserved");
			break;
		case kDSpContextNotReservedErr:
			ReportError ("DSp Error: Context not reserved");
			break;
		case kDSpContextNotFoundErr:
			ReportError ("DSp Error: Context not found");
			break;
		case kDSpFrameRateNotReadyErr:
			ReportError ("DSp Error: Frame rate not ready");
			break;
		case kDSpConfirmSwitchWarning:
//			ReportError ("DSp Warning: Must confirm switch"); // removed since it is just a warning, add back for debugging
			return 0; // don't want to fail on this warning
			break;
		case kDSpInternalErr:
			ReportError ("DSp Error: Internal error");
			break;
		case kDSpStereoContextErr:
			ReportError ("DSp Error: Stereo context");
			break;
	}
	return error;
}

//-----------------------------------------------------------------------------------------------------------------------

// if error dump agl errors to debugger string, return error

GLenum aglDebugStr (void)
{
	GLenum err = aglGetError();
	if (AGL_NO_ERROR != err)
		ReportError ((char *)aglErrorString(err));
	return err;
}

//-----------------------------------------------------------------------------------------------------------------------

// if error dump agl errors to debugger string, return error

GLenum glDebugStr (void)
{
	GLenum err = glGetError();
	if (GL_NO_ERROR != err)
		ReportError ((char *)gluErrorString(err));
	return err;
}

#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

// Set up DSp screen on graphics device requested
// side effect: sets both gpDSpWindow and gpPort

CGrafPtr SetupDSpFullScreen (GDHandle hGD)
{
	DSpContextAttributes foundAttributes;
	DisplayIDType displayID = NULL;
	
	if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpGetVersion) 
	{
		ReportError ("DrawSprocket not installed.");
		return NULL;
	}
	else
		gVersionDSp = DSpGetVersion ();

	if ((gVersionDSp.majorRev == 0x01) && (gVersionDSp.minorAndBugRev < 0x99))
	{
		// this version of DrawSprocket is not completely functional on Mac OS X
		if (CheckMacOSX ())
		{
			ReportError ("DrawSprocket 1.99 or greate required on Mac OS X, please update to at least Mac OS X 10.1.");
			return NULL;
		}
	}
			
	// Note: DSp < 1.7.3 REQUIRES the back buffer attributes even if only one buffer is required
	memset(&gContextAttributes, 0, sizeof (DSpContextAttributes));
	gContextAttributes.displayWidth			= kContextWidth;
	gContextAttributes.displayHeight		= kContextHeight;
	gContextAttributes.colorNeeds			= kDSpColorNeeds_Require;
	gContextAttributes.displayBestDepth		= kBitsPerPixel;
	gContextAttributes.backBufferBestDepth	= kBitsPerPixel;
	gContextAttributes.displayDepthMask		= kDSpDepthMask_All;
	gContextAttributes.backBufferDepthMask	= kDSpDepthMask_All;
	gContextAttributes.pageCount			= 1;								// only the front buffer is needed
	
	screenwidth=kContextWidth;
	screenheight=kContextHeight;
	
	DMGetDisplayIDByGDevice (hGD, &displayID, true);
	
	if (noErr != DSpDebugStr (DSpFindBestContextOnDisplayID (&gContextAttributes, &gContext, displayID)))
	{
		ReportError ("DSpFindBestContext() had an error.");
		return NULL;
	}

	if (noErr != DSpDebugStr (DSpContext_GetAttributes (gContext, &foundAttributes))) // see what we actually found
	{
		ReportError ("DSpContext_GetAttributes() had an error.");
		return NULL;
	}

	// reset width and height to full screen and handle our own centering
	// HWA will not correctly center less than full screen size contexts
	gContextAttributes.displayWidth 	= foundAttributes.displayWidth;
	gContextAttributes.displayHeight 	= foundAttributes.displayHeight;
	gContextAttributes.pageCount		= 1;									// only the front buffer is needed
	gContextAttributes.contextOptions	= 0 | kDSpContextOption_DontSyncVBL;	// no page flipping and no VBL sync needed

	DSpSetBlankingColor(&rgbBlack);

	if (noErr !=  DSpDebugStr (DSpContext_Reserve ( gContext, &gContextAttributes))) // reserve our context
	{
		ReportError ("DSpContext_Reserve() had an error.");
		return NULL;
	}
 
	HideCursor ();

	if (noErr != DSpDebugStr (DSpContext_SetState (gContext, kDSpContextState_Active))) // activate our context
	{
		ReportError ("DSpContext_SetState() had an error.");
		return NULL;
	}


	if ((CheckMacOSX ()) && !((gVersionDSp.majorRev > 0x01) || ((gVersionDSp.majorRev == 0x01) && (gVersionDSp.minorAndBugRev >= 0x99))))// DSp should be supported in version after 1.98
	{
		ReportError ("Mac OS X with DSp < 1.99 does not support DrawSprocket for OpenGL full screen");
		return NULL;
	}
	else if (CheckMacOSX ()) // DSp should be supported in versions 1.99 and later
	{
		CGrafPtr pPort;
		// use DSp's front buffer on Mac OS X
		if (noErr != DSpDebugStr (DSpContext_GetFrontBuffer (gContext, &pPort)))
		{
			ReportError ("DSpContext_GetFrontBuffer() had an error.");
			return NULL;
		}
		// there is a problem in Mac OS X 10.0 GM CoreGraphics that may not size the port pixmap correctly
		// this will check the vertical sizes and offset if required to fix the problem
		// this will not center ports that are smaller then a particular resolution
		{
			long deltaV, deltaH;
			Rect portBounds;
			PixMapHandle hPix = GetPortPixMap (pPort);
			Rect pixBounds = (**hPix).bounds;
			GetPortBounds (pPort, &portBounds);
			deltaV = (portBounds.bottom - portBounds.top) - (pixBounds.bottom - pixBounds.top) +
			         (portBounds.bottom - portBounds.top - kContextHeight) / 2;
			deltaH = -(portBounds.right - portBounds.left - kContextWidth) / 2;
			if (deltaV || deltaH)
			{
				GrafPtr pPortSave;
				GetPort (&pPortSave);
				SetPort ((GrafPtr)pPort);
				// set origin to account for CG offset and if requested drawable smaller than screen rez
				SetOrigin (deltaH, deltaV);
				SetPort (pPortSave);
			}
		}
		return pPort;
	}
	else // Mac OS 9 or less
	{
		WindowPtr pWindow;
		Rect rectWin;
		RGBColor rgbSave;
		GrafPtr pGrafSave;
		// create a new window in our context 
		// note: OpenGL is expecting a window so it can enumerate the devices it spans, 
		// center window in our context's gdevice
		rectWin.top  = (short) ((**hGD).gdRect.top + ((**hGD).gdRect.bottom - (**hGD).gdRect.top) / 2);	  	// h center
		rectWin.top  -= (short) (kContextHeight / 2);
		rectWin.left  = (short) ((**hGD).gdRect.left + ((**hGD).gdRect.right - (**hGD).gdRect.left) / 2);	// v center
		rectWin.left  -= (short) (kContextWidth / 2);
		rectWin.right = (short) (rectWin.left + kContextWidth);
		rectWin.bottom = (short) (rectWin.top + kContextHeight);
		
		pWindow = NewCWindow (NULL, &rectWin, "\p", 0, plainDBox, (WindowPtr)-1, 0, 0);

		// paint back ground black before fade in to avoid white background flash
		ShowWindow(pWindow);
		GetPort (&pGrafSave);
		SetPortWindowPort (pWindow);
		GetForeColor (&rgbSave);
		RGBForeColor (&rgbBlack);
		{
			Rect paintRect;
			GetWindowPortBounds (pWindow, &paintRect);
			PaintRect (&paintRect);
		}
		RGBForeColor (&rgbSave);		// ensure color is reset for proper blitting
		SetPort (pGrafSave);
		return (GetWindowPort (pWindow));
	}
}

//-----------------------------------------------------------------------------------------------------------------------

// clean up DSp

void ShutdownDSp (CGrafPtr pDSpPort)
{
	if ((NULL != pDSpPort) && !CheckMacOSX ())
		DisposeWindow (GetWindowFromPort (pDSpPort));
	DSpContext_SetState( gContext, kDSpContextState_Inactive);
	DSpContext_Release (gContext);
}

#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Setup


AGLContext SetupAGL (GDHandle hGD, AGLDrawable drawable)
{
	GLint			attrib[64];
	
	AGLPixelFormat 	fmt;
	AGLContext     	ctx;

// different possible pixel format choices for different renderers 
// basics requirements are RGBA and double buffer
// OpenGLz will select acclerated context if available

	short i = 0;
	attrib [i++] = AGL_RGBA; // red green blue and alpha
	attrib [i++] = AGL_DOUBLEBUFFER; // double buffered
	attrib [i++] = AGL_DEPTH_SIZE; 
	attrib [i++] = 8; 
	attrib [i++] = AGL_ACCELERATED; // HWA pixel format only
	attrib [i++] = AGL_NONE;

	if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) // check for existance of OpenGL
	{
		ReportError ("OpenGL not installed");
		return NULL;
	}	

	if (hGD)
		fmt = aglChoosePixelFormat (&hGD, 1, attrib); // get an appropriate pixel format
	else
		fmt = aglChoosePixelFormat(NULL, 0, attrib); // get an appropriate pixel format
	aglDebugStr ();
	if (NULL == fmt) 
	{
		ReportError("Could not find valid pixel format");
		return NULL;
	}

	ctx = aglCreateContext (fmt, NULL); // Create an AGL context
	aglDebugStr ();
	if (NULL == ctx)
	{
		ReportError ("Could not create context");
		return NULL;
	}

	if (!aglSetDrawable (ctx, drawable)) // attach the window to the context
	{
		ReportError ("SetDrawable failed");
		aglDebugStr ();
		return NULL;
	}


	if (!aglSetCurrentContext (ctx)) // make the context the current context
	{
		aglDebugStr ();
		aglSetDrawable (ctx, NULL);
		return NULL;
	}

	aglDestroyPixelFormat(fmt); // pixel format is no longer needed

	return ctx;
}

//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Setup

AGLContext SetupAGLFullScreen (GDHandle display, short * pDepth, short * pWidth, short * pHeight)
{
	//GLint			attrib[64];
	GLint attrib[] ={AGL_RGBA, AGL_DOUBLEBUFFER,AGL_DEPTH_SIZE, 16, AGL_FULLSCREEN,AGL_ALL_RENDERERS, AGL_NONE};

	AGLPixelFormat 	fmt;
	AGLContext     	ctx;


	if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) aglChoosePixelFormat) // check for existance of OpenGL
	{
		ReportError ("OpenGL not installed");
		return NULL;
	}	

	fmt = aglChoosePixelFormat(&display, 1, attrib); // this may fail if looking for acclerated across multiple monitors
	if (NULL == fmt) 
	{
		ReportError("Could not find valid pixel format");
		aglDebugStr ();
		return NULL;
	}

	ctx = aglCreateContext (fmt, NULL); // Create an AGL context
	if (NULL == ctx)
	{
		ReportError ("Could not create context");
		aglDebugStr ();
		return NULL;
	}

	if (!aglSetFullScreen (ctx, *pWidth, *pHeight, 60, 0))
	{
		ReportError ("SetFullScreen failed");
		aglDebugStr ();
		return NULL;
	}

	if (!aglSetCurrentContext (ctx)) // make the context the current context
	{
		ReportError ("SetCurrentContext failed");
		aglDebugStr ();
		aglSetDrawable (ctx, NULL); // turn off full screen
		return NULL;
	}

	aglDestroyPixelFormat(fmt); // pixel format is no longer needed

	return ctx;
}

//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Cleanup

void CleanupAGL(AGLContext ctx)
{
	aglSetDrawable (ctx, NULL);
	aglSetCurrentContext (NULL);
	aglDestroyContext (ctx);
}

//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Drawing

void DrawGL (Rect * pRectPort)
{
	if (gaglContext == 0)
		return;
	aglSetCurrentContext (gaglContext); // ensure our context is current prior to drawing
	
    {
        GLint width = pRectPort->right - pRectPort->left;
        GLint height = pRectPort->bottom - pRectPort->top;
        screenwidth=width;
        screenheight=height;
       /* glViewport ((width - (width * 1)) / 2, (height - (height * 1)) / 2,
                    width * 1, height * 1);*/
     }
    
	game.DrawGLScene();
	
	
	//glDebugStr ();

	// draw info
	/*{
		GLint matrixMode;
        glViewport (0, 0, pRectPort->right - pRectPort->left, pRectPort->bottom - pRectPort->top);
		glGetIntegerv (GL_MATRIX_MODE, &matrixMode);
		glMatrixMode (GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity ();
			glMatrixMode (GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity ();
				glScalef (2.0 / (pRectPort->right - pRectPort->left), -2.0 /  (pRectPort->bottom - pRectPort->top), 1.0);
				glTranslatef (-(pRectPort->right - pRectPort->left) / 2.0, -(pRectPort->bottom - pRectPort->top) / 2.0, 0.0);
				glColor3f (1.0, 1.0, 1.0);
				glRasterPos3d (10, 12, 0); 
				DrawFrameRate (gFontList);
				glRasterPos3d (10, 24, 0); 
				DrawCStringGL (gcstrMode, gFontList);
				glRasterPos3d (10, (pRectPort->bottom - pRectPort->top) - 15, 0); 
				DrawCStringGL ((char*) glGetString (GL_VENDOR), gFontList);
				glRasterPos3d (10, (pRectPort->bottom - pRectPort->top) - 3, 0); 
				DrawCStringGL ((char*) glGetString (GL_RENDERER), gFontList);
				static char aChar[256] = "";
				sprintf (aChar, "     Mac OS X: %d",  RunningOnCarbonX());
				DrawCStringGL (aChar, gFontList);
			glPopMatrix();
			glMatrixMode (GL_PROJECTION);
		glPopMatrix();
		glMatrixMode (matrixMode);
	}*/
	//glDebugStr ();
	//aglSwapBuffers(gaglContext); // send swap command
}

#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

static pascal OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, SInt32 refcon )
{
	#pragma unused (appleEvt, reply, refcon)
	//gDone =  true;
	game.tryquit=1;
	return false;
}

//-----------------------------------------------------------------------------------------------------------------------

void InitToolbox(void)
{
	OSErr err;
	long response;
	MenuHandle menu;
	
	InitCursor();
	
// profile
#if __profile__
// 	prototype:
//		ProfilerInit (collection method, time base, num funcs, stack depth)
	// default call
	gProfErr = ProfilerInit (collectDetailed, bestTimeBase, 20, 10); // set up profiling
	// something that you may need to do (may need more memory)
//	gProfErr = ProfilerInit (collectSummary, bestTimeBase, 1000, 100); // set up profiling
// Note: application will likely run slower, but still should be useful info
	if (noErr == gProfErr)
		ProfilerSetStatus(FALSE);
#endif
	
	// Init Menus
	menu = NewMenu (kMenuApple, "\p\024");			// new  apple menu
	InsertMenu (menu, 0);							// add menu to end

	menu = NewMenu (kMenuFile, "\pFile");			// new menu
	InsertMenu (menu, 0);							// add menu to end

	// insert application menus here
	
	// add quit if not under Mac OS X
	err = Gestalt (gestaltMenuMgrAttr, &response);
	if ((err == noErr) && !(response & gestaltMenuMgrAquaLayoutMask))
			AppendMenu (menu, "\pQuit/Q"); 					// add quit

	DrawMenuBar();
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false );
	if (err != noErr)
		ExitToShell();
}

// --------------------------------------------------------------------------

static Point gMidPoint;

Boolean SetUp (void)
{
	char string[10];
	
	SetQDGlobalsRandomSeed(TickCount());
	
	osx = 0;
	if(CheckMacOSX ())osx = 1;
	ifstream ipstream(":Data:config.txt");
	detail=1;
	ismotionblur=0;
	usermousesensitivity=1;
	kContextWidth=640;
	kContextHeight=480;
	kBitsPerPixel = 32;
	floatjump=0;
	cellophane=0;
	texdetail=4;
	autoslomo=1;
	decals=1;
	invertmouse=0;
	bloodtoggle=0;
	terraindetail=2;
	foliage=1;
	musictoggle=1;
	trilinear=1;
	gamespeed=1;
	difficulty=1;
	damageeffects=0;
	texttoggle=1;
	alwaysblur=0;
	showpoints=0;
	immediate=0;
	velocityblur=0;
	slomospeed=0.25;
	slomofreq=8012;
	
	game.crouchkey=MAC_SHIFT_KEY;
    game.jumpkey=MAC_SPACE_KEY;
    game.leftkey=MAC_A_KEY;
    game.forwardkey=MAC_W_KEY;
    game.backkey=MAC_S_KEY;
    game.rightkey=MAC_D_KEY;
    game.drawkey=MAC_E_KEY;
    game.throwkey=MAC_Q_KEY;
    game.attackkey=MAC_MOUSEBUTTON1;
    game.chatkey=MAC_T_KEY;
    numplayers=1;
    ambientsound=1;
    vblsync=0;
    debugmode=0;
    if(!ipstream) {
        ofstream opstream(":Data:config.txt"); 
        opstream << "Screenwidth:\n";
        opstream << kContextWidth;
        opstream << "\nScreenheight:\n";
	    opstream << kContextHeight;
	    opstream << "\nMouse sensitivity:\n";
	    opstream << usermousesensitivity;
	    opstream << "\nBlur(0,1):\n";
	    opstream << ismotionblur;
		opstream << "\nOverall Detail(0,1,2) higher=better:\n";
	    opstream << detail;
	    opstream << "\nFloating jump:\n";
	    opstream << floatjump;
	    opstream << "\nMouse jump:\n";
	    opstream << mousejump;
	    opstream << "\nAmbient sound:\n";
	    opstream << ambientsound;
	   	opstream << "\nBlood (0,1,2):\n";
        opstream << bloodtoggle;
        opstream << "\nAuto slomo:\n";
        opstream << autoslomo;
	    opstream << "\nFoliage:\n";
        opstream << foliage;
        opstream << "\nMusic:\n";
        opstream << musictoggle;
        opstream << "\nTrilinear:\n";
        opstream << trilinear;
	    opstream << "\nDecals(shadows,blood puddles,etc):\n";
        opstream << decals;
	    opstream << "\nInvert mouse:\n";
        opstream << invertmouse;
	    opstream << "\nGamespeed:\n";
        opstream << gamespeed;
        opstream << "\nDifficulty(0,1,2) higher=harder:\n";
        opstream << difficulty;
        opstream << "\nDamage effects(blackout, doublevision):\n";
        opstream << damageeffects;
	    opstream << "\nText:\n";
        opstream << texttoggle;
	    opstream << "\nDebug:\n";
        opstream << debugmode;
	    opstream << "\nVBL Sync:\n";
        opstream << vblsync;
        opstream << "\nShow Points:\n";
        opstream << showpoints;
        opstream << "\nAlways Blur:\n";
        opstream << alwaysblur;
	    opstream << "\nImmediate mode (turn on on G5):\n";
        opstream << immediate;
	    opstream << "\nVelocity blur:\n";
        opstream << velocityblur;
	    opstream << "\nForward key:\n";
	    opstream << KeyToChar(game.forwardkey);
	    opstream << "\nBack key:\n";
	    opstream << KeyToChar(game.backkey);
	    opstream << "\nLeft key:\n";
	    opstream << KeyToChar(game.leftkey);
	    opstream << "\nRight key:\n";
	    opstream << KeyToChar(game.rightkey);
	    opstream << "\nJump key:\n";
	    opstream << KeyToChar(game.jumpkey);
	    opstream << "\nCrouch key:\n";
	    opstream << KeyToChar(game.crouchkey);
	    opstream << "\nDraw key:\n";
	    opstream << KeyToChar(game.drawkey);
	    opstream << "\nThrow key:\n";
	    opstream << KeyToChar(game.throwkey);
	    opstream << "\nAttack key:\n";
	    opstream << KeyToChar(game.attackkey);
	    opstream << "\nChat key:\n";
	    opstream << KeyToChar(game.chatkey);
	    opstream.close();
    }
    if(ipstream){
		ipstream.ignore(256,'\n');
	    ipstream >> kContextWidth;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> kContextHeight;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> usermousesensitivity;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> ismotionblur;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> detail;
	    if(detail!=0)kBitsPerPixel=32;
	    else kBitsPerPixel=16;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> floatjump;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> mousejump;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> ambientsound;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> bloodtoggle;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> autoslomo;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> foliage;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> musictoggle;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> trilinear;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> decals;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> invertmouse;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> gamespeed;
	    oldgamespeed=gamespeed;
	    if(oldgamespeed==0){
	    	gamespeed=1;
	    	oldgamespeed=1;
	    }
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> difficulty;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> damageeffects;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> texttoggle;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> debugmode;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> vblsync;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> showpoints;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> alwaysblur;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> immediate;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> velocityblur;
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n'); 
	    ipstream >> string;
	    game.forwardkey=CharToKey(string);
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.backkey=CharToKey(string);
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.leftkey=CharToKey(string);
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.rightkey=CharToKey(string);
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.jumpkey=CharToKey(string);
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.crouchkey=CharToKey(string);
	    ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.drawkey=CharToKey(string);
	     ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.throwkey=CharToKey(string);
	      ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.attackkey=CharToKey(string);
	     ipstream.ignore(256,'\n');
	    ipstream.ignore(256,'\n');
	    ipstream >> string;
	    game.chatkey=CharToKey(string);
	    ipstream.close();
	    
	    if(detail>2)detail=2;
	    if(detail<0)detail=0;
	    if(screenwidth>3000)screenwidth=640;
		if(screenwidth<0)screenwidth=640;
		if(screenheight>3000)screenheight=480;
		if(screenheight<0)screenheight=480;
	}

	
	if(vblsync){	
		GLint swapInt = 1;
 	   	aglSetInteger(gaglContext, AGL_SWAP_INTERVAL, &swapInt);
	}
	
	if(kBitsPerPixel!=32&&kBitsPerPixel!=16){
		kBitsPerPixel=16;
	}

	
	GDHandle hGD, hTargetDevice = NULL;
	short numDevices = 0;
	short fNum = 0;
	short whichDevice = 0; // number of device to try (0 = 1st device)

	InitToolbox ();
	
	gpDSpPort = NULL;
	gaglContext = 0;
	
	// check for DSp
	if ((Ptr) kUnresolvedCFragSymbolAddress == (Ptr) DSpStartup) 
		ReportError ("DSp not installed");

	if (noErr != DSpDebugStr (DSpStartup()))
		return NULL;
//	DSpSetDebugMode (true);
#if defined kUseFades
	DSpDebugStr (DSpContext_FadeGammaOut (NULL, NULL)); // fade display, remove for debug
#endif
	
	hGD = DMGetFirstScreenDevice (true); // check number of screens
	hTargetDevice = hGD; // default to first device							
	do
	{
		if (numDevices == whichDevice)
			hTargetDevice = hGD; // if device number matches						
		numDevices++;
		hGD = DMGetNextScreenDevice (hGD, true);
	}
	while (hGD);

#if defined (kUseAGLFullScreenX)
	if (CheckMacOSX ()) // Try AGL full screen on Mac OS X
	{
		short depth = kBitsPerPixel, width = kContextWidth, height = kContextHeight; 
		gaglContext = SetupAGLFullScreen (hTargetDevice, &depth, &width, &height); // Setup the OpenGL context
		SetRect (&gRectPort, 0, 0, width, height); // l, t, r, b
		sprintf (gcstrMode, "AGL Full Screen: %d x %d x %d", width, height, depth);
	}
#endif
	if (!gaglContext) //try DSp
	{
		if (NULL != (gpDSpPort = SetupDSpFullScreen (hTargetDevice))) // Setup DSp for OpenGL sets hTargetDeviceto device actually used 
		{
			GetPortBounds (gpDSpPort, &gRectPort);
			sprintf (gcstrMode, "DrawSprocket Full Screen: %d x %d x %d", gRectPort.right - gRectPort.left, gRectPort.bottom - gRectPort.top, (**(**hTargetDevice).gdPMap).pixelSize);
			gaglContext = SetupAGL (hTargetDevice, gpDSpPort);
		}
	}
	if (gaglContext)
	{
		GetFNum("\pMonaco", &fNum);									// build font
		gFontList = BuildFontGL (gaglContext, fNum, normal, 9);
		//InstallEventLoopTimer (GetCurrentEventLoop(), 0, 0.0001, GetTimerUPP (), 0, &gTimer);
	}
	
	//gMidPoint.h = (gRectPort.left + gRectPort.right)/2;
	//gMidPoint.v = (gRectPort.top + gRectPort.bottom)/2;
	gMidPoint.h = 200;
	gMidPoint.v = 200;
	GLint swapInt = 1;
 	//aglSetInteger(gaglContext, AGL_SWAP_INTERVAL, &swapInt);
	
#if defined kUseFades
	DSpDebugStr (DSpContext_FadeGammaIn (NULL, NULL));
#endif
	
	
	HideCursor();
	
	
    GLint width = gRectPort.right - gRectPort.left;
    GLint height = gRectPort.bottom - gRectPort.top;
    screenwidth=width;
    screenheight=height;
     
    game.newdetail=detail;
	game.newscreenwidth=screenwidth;
	game.newscreenheight=screenheight;
     
	game.InitGame();
	
	if (gaglContext)
		return true;
	else
		return false;
	
}

static Boolean RunningOnCarbonX(void)
{
	static Boolean first = true;
	static Boolean result = false;

	if (first)
	{
		UInt32 response;

		first = false;

		result = (Gestalt(gestaltSystemVersion, 
                    (SInt32 *) &response) == noErr)
                && (response >= 0x01000);
	}
    return result;
}

static OSStatus LoadFrameworkBundle(CFStringRef pFrameworkCFStrRef, CFBundleRef *pFrameworkCFBndlRef)
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	if (nil == pFrameworkCFBndlRef) return paramErr;
	
	*pFrameworkCFBndlRef = nil;
	
	baseURL = nil;
	bundleURL = nil;
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, pFrameworkCFStrRef, false);
		if (bundleURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		*pFrameworkCFBndlRef = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*pFrameworkCFBndlRef == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
	    if ( ! CFBundleLoadExecutable( *pFrameworkCFBndlRef ) ) {
			err = coreFoundationUnknownErr;
	    }
	}

	// Clean up.
	
	if (err != noErr && *pFrameworkCFBndlRef != nil) {
		CFRelease(*pFrameworkCFBndlRef);
		*pFrameworkCFBndlRef = nil;
	}
	if (bundleURL != nil) {
		CFRelease(bundleURL);
	}	
	if (baseURL != nil) {
		CFRelease(baseURL);
	}	
	
	return err;
}



static CFragConnectionID gCFragConnectionID = 0;

typedef pascal OSErr (*CDM_NewDeviceProcPtr)(CursorDevicePtr * ourDevice);

static OSErr MyCursorDeviceNewDevice(CursorDevicePtr * ourDevice)
{
	static CDM_NewDeviceProcPtr sCDM_NewDeviceProcPtr = nil;
	OSStatus anErr = noErr;

	if (nil == ourDevice) return paramErr;

	if (!RunningOnCarbonX())
	{
		if (!sCDM_NewDeviceProcPtr)
		{
			if (!gCFragConnectionID)
			{
				Ptr		mainAddr;
				Str255  errName;

				anErr = GetSharedLibrary("\pInterfaceLib", kPowerPCCFragArch,
					 kFindCFrag, &gCFragConnectionID, &mainAddr, errName);
				if (noErr != anErr) return anErr;
			}

			if (gCFragConnectionID)
			{
				CFragSymbolClass symClass;

				anErr = FindSymbol(gCFragConnectionID, "\pCrsrDevNewDevice",
					(Ptr*) &sCDM_NewDeviceProcPtr, &symClass);
				if (noErr != anErr) return anErr;
			}
		}
		if (sCDM_NewDeviceProcPtr)
			anErr = (*sCDM_NewDeviceProcPtr)(ourDevice);
		else
			anErr = paramErr;
	}

	return anErr;
}

typedef pascal OSErr (*CDM_CursorDeviceMoveToProcPtr)(
							CursorDevicePtr   ourDevice,
							long              absX,
							long              absY);

typedef UInt32 (*CGWarpMouseCursorPositionProcPtr)(CGPoint newCursorPosition);
typedef UInt32 (*CGSetLocalEventsSuppressionIntervalProcPtr)(double pSeconds);

static OSErr MyCursorDeviceMoveTo(	CursorDevicePtr   ourDevice,
							long              absX,
							long              absY)
{
	OSStatus anErr = noErr;

	if (RunningOnCarbonX())
	{
		static CGWarpMouseCursorPositionProcPtr sCGWarpMouseCursorPositionProcPtr = nil;

		if (nil == sCGWarpMouseCursorPositionProcPtr)
		{
			CFBundleRef tCFBundleRef;

			anErr = LoadFrameworkBundle(CFSTR("ApplicationServices.framework"), &tCFBundleRef);

			if (noErr == anErr)
			{
				CGSetLocalEventsSuppressionIntervalProcPtr tCGSetLocalEventsSuppressionIntervalProcPtr = nil;

				sCGWarpMouseCursorPositionProcPtr = (CGWarpMouseCursorPositionProcPtr)
					CFBundleGetFunctionPointerForName( tCFBundleRef, CFSTR("CGWarpMouseCursorPosition") );
				if (nil == sCGWarpMouseCursorPositionProcPtr)
					anErr = cfragNoSymbolErr;

				tCGSetLocalEventsSuppressionIntervalProcPtr = (CGSetLocalEventsSuppressionIntervalProcPtr)
					CFBundleGetFunctionPointerForName(tCFBundleRef,CFSTR("CGSetLocalEventsSuppressionInterval"));
				if (nil != tCGSetLocalEventsSuppressionIntervalProcPtr)
					(*tCGSetLocalEventsSuppressionIntervalProcPtr)(0.0f);
			}
		}

		if (nil != sCGWarpMouseCursorPositionProcPtr)
		{
			CGPoint tCGPoint;

			tCGPoint.x = absX;
			tCGPoint.y = absY;

			(*sCGWarpMouseCursorPositionProcPtr)(tCGPoint);
		}
	}
	else
	{
		static CDM_CursorDeviceMoveToProcPtr sCDM_CursorDeviceMoveToProcPtr = nil;

		if (!ourDevice) return paramErr;

		if (!sCDM_CursorDeviceMoveToProcPtr)
		{
			if (!gCFragConnectionID)
			{
				Ptr		mainAddr;
				Str255  errName;

				anErr = GetSharedLibrary("\pInterfaceLib", kPowerPCCFragArch,
					 kFindCFrag, &gCFragConnectionID, &mainAddr, errName);
				if (noErr != anErr) return anErr;
			}

			if (gCFragConnectionID)
			{
				CFragSymbolClass symClass;

				anErr = FindSymbol(gCFragConnectionID, "\pCrsrDevMoveTo",
					(Ptr*) &sCDM_CursorDeviceMoveToProcPtr, &symClass);
				if (noErr != anErr) return anErr;
			}
		}
		if (sCDM_CursorDeviceMoveToProcPtr)
			anErr = (*sCDM_CursorDeviceMoveToProcPtr)(ourDevice,absX,absY);
		else
			anErr = paramErr;
	}

	return anErr;
}

static void DoMouse(void)
{
	static CursorDevicePtr virtualCursorDevicePtr = nil;
	OSStatus anErr = noErr;

	 HideCursor();

	if (nil == virtualCursorDevicePtr)
		anErr = MyCursorDeviceNewDevice(&virtualCursorDevicePtr);

	{
		static Point lastMouse = {-1,-1};
		Point globalMouse;

		GetGlobalMouse(&globalMouse);
	
		if (EqualPt(lastMouse, globalMouse))
		{
			game.deltah=0;
			game.deltav=0;
		}

		if (!EqualPt(lastMouse, globalMouse))
		{
			static Point virtualMouse = {0,0};
			delta = globalMouse;

			SubPt(lastMouse, &delta);
			GetGlobalMouse(&lastMouse);
			//AddPt(delta, &virtualMouse);
			
			if(mainmenu||(abs(delta.h)<10*realmultiplier*1000&&abs(delta.v)<10*realmultiplier*1000)){
			game.deltah=delta.h*usermousesensitivity;
			game.deltav=delta.v*usermousesensitivity;
			game.mousecoordh=globalMouse.h;
			game.mousecoordv=globalMouse.v;
			}
#if 1
			//printf("\nMouse Moved: {%d,%d}.",globalMouse.h,globalMouse.v);
			//printf("\tdelta: {%d,%d}.",delta.h,delta.v);
			//printf("\tvirtual: {%d,%d}.",virtualMouse.h,virtualMouse.v);
#endif
			if(!mainmenu)
			if(lastMouse.h>gMidPoint.h+100||lastMouse.h<gMidPoint.h-100||lastMouse.v>gMidPoint.v+100||lastMouse.v<gMidPoint.v-100){
				MyCursorDeviceMoveTo(virtualCursorDevicePtr,gMidPoint.h,gMidPoint.v);
				lastMouse = gMidPoint;
			}
		}
	}
}


// --------------------------------------------------------------------------

void DoMenu (SInt32 menuResult)
{
	SInt16 theMenu;
	SInt16 theItem;
	MenuRef theMenuHandle;
		
	theMenu = HiWord(menuResult);
	theItem = LoWord(menuResult);
	theMenuHandle = GetMenuHandle(theMenu);

	switch (theMenu)
	{
		case kMenuApple:
			switch (theItem)
			{
				case kAppleAbout:
					break;
				default:
					break;
			}
			break;
		case kMenuFile:
			switch (theItem)
			{
				case kFileQuit:
					game.quit = true;
					break;
			}
			break;
	}
	HiliteMenu(0);
	DrawMenuBar();
}

// --------------------------------------------------------------------------

void DoKey (SInt8 theKey, SInt8 theCode)
{
	#pragma unused (theCode, theKey)
    /*if ((theKey == '=') || (theKey == '+'))
        gfScale *= 1.1;
    if (theKey == '-')
        gfScale /= 1.1;*/
	// do nothing
}

// --------------------------------------------------------------------------



/*void DoUpdate (void)
{
	static float sps=200;
	static int count;
	static float oldmult;

	DoFrameRate();
	
	count = multiplier*sps;
	if(count<2)count=2;
	
	oldmult=multiplier;
	multiplier/=count;
	
	for(int i=0;i<count;i++){
		game.Tick();
	}
	multiplier=oldmult;
	
	game.TickOnce();
	
	DoMouse();

	DrawGL (&gRectPort);
}*/

void DoUpdate (void)
{
	static float sps=200;
	static int count;
	static float oldmult;
	
	DoFrameRate(1);
	
	if(multiplier>.6)multiplier=.6;
	
	game.fps=1/multiplier;

	count = multiplier*sps;
	if(count<2)count=2;
	//if(count>10)count=10;
	
	realmultiplier=multiplier;
	multiplier*=gamespeed;
	if(difficulty==1)multiplier*=.9;
	if(difficulty==0)multiplier*=.8;
	
	if(game.loading==4)multiplier*=.00001;

	//multiplier*.9;
	if(slomo&&!mainmenu)multiplier*=slomospeed;
	//if(freeze)multiplier*=0.00001;
	oldmult=multiplier;
	multiplier/=(float)count;
	
	DoMouse();
	
	game.TickOnce();
	
	for(int i=0;i<count;i++){
		game.Tick();
	}
	multiplier=oldmult;
	
	game.TickOnceAfter();
	
	DrawGL (&gRectPort);
}

// --------------------------------------------------------------------------

Boolean WaitNextEventWrapper (EventMask eventMask, EventRecord *theEvent, unsigned long sleep,RgnHandle mouseRgn)
{
	return WaitNextEvent (eventMask, theEvent, sleep, mouseRgn);
}

// --------------------------------------------------------------------------

OSStatus DSpProcessEventWrapper (EventRecord *inEvent, Boolean *outEventWasProcessed)
{
	return DSpProcessEvent (inEvent, outEventWasProcessed);
}

// --------------------------------------------------------------------------

void UpdateWrapper (EventRecord *theEvent)
{
	WindowRef whichWindow;
	GrafPtr pGrafSave;
	
	whichWindow = (WindowRef) theEvent->message;
	GetPort (&pGrafSave);
	SetPort((GrafPtr) GetWindowPort(whichWindow));
	BeginUpdate(whichWindow);
	DoUpdate();
	SetPort((GrafPtr) GetWindowPort(whichWindow));
	EndUpdate(whichWindow);
	SetPort (pGrafSave);
}

// --------------------------------------------------------------------------

pascal void IdleTimer (EventLoopTimerRef inTimer, void* userData)
{
	#pragma unused (inTimer, userData)
	if(!stillloading)DoUpdate ();
}

// --------------------------------------------------------------------------

EventLoopTimerUPP GetTimerUPP (void)
{
	static EventLoopTimerUPP	sTimerUPP = NULL;
	
	if (sTimerUPP == NULL)
		sTimerUPP = NewEventLoopTimerUPP (IdleTimer);
	
	return sTimerUPP;
}

// --------------------------------------------------------------------------

pascal OSStatus mDown (EventHandlerCallRef ref, EventRef e,void * thedata) {
	EventMouseButton button;
	OSStatus status;
	
	status = GetEventParameter (e, kEventParamMouseButton, typeMouseButton, NULL, 
		sizeof (button), NULL, &button);
	
	buttons[button-1] = true;		
	
	return eventNotHandledErr;
}

pascal OSStatus mUp (EventHandlerCallRef ref, EventRef e,void * thedata) {
	EventMouseButton button = 0;
	OSStatus status;
	
	status = GetEventParameter (e, kEventParamMouseButton, typeMouseButton, NULL, 
		sizeof (EventMouseButton), NULL, &button);
	
	buttons[button-1] = false;	
	
	return eventNotHandledErr;	
}

void InstallHandlers(void){
	OSStatus status;
	EventTypeSpec spec;
	
	spec.eventClass = kEventClassMouse;
	spec.eventKind = kEventMouseDown;

	status = InstallEventHandler (GetApplicationEventTarget(), 
		NewEventHandlerUPP (mDown), 1, &spec, NULL, NULL);
	
	
	spec.eventKind = kEventMouseUp;
	
	status = InstallEventHandler (GetApplicationEventTarget(), 
		NewEventHandlerUPP (mUp), 1, &spec, NULL, NULL);

}

void NewEvent(void) {
	EventRecord e;
	OSStatus s;
	
	
	/*ReceiveNextEvent (0, NULL, 1, true, &er);
	
	s = SendEventToEventTarget (er, GetEventDispatcherTarget());*/
	WaitNextEvent (everyEvent, &e, 0, NULL);
	
	DoUpdate();
}

void DoEvent (void)
{
	EventRecord theEvent;
	WindowRef whichWindow;
	SInt16 whatPart;
	Boolean fProcessed;
	
// profile
#if __profile__
	if (noErr == gProfErr)
		ProfilerSetStatus(TRUE); // turn on profiling
#endif

	if (WaitNextEventWrapper (everyEvent, &theEvent, gSleepTime, NULL))
	{
		DSpProcessEventWrapper (&theEvent, &fProcessed);
		if (!fProcessed)
		{
			switch (theEvent.what)
			{
				case mouseDown:
					whatPart = FindWindow (theEvent.where, &whichWindow);
					switch (whatPart)
					{
						case inContent:
//							if (GetWindowFromPort (gpDSpPort) != FrontWindow()) 
//								SelectWindow (GetWindowFromPort (gpDSpPort));
							break;
						case inMenuBar:
						{
							SInt32 menuResult = MenuSelect (theEvent.where);
							if (HiWord (menuResult) != 0)
								DoMenu (menuResult);
						}
							break;
						case inDrag:
							{
								// full screen no drag
							}
							break;
						case inGrow:
							{
								// full screen no grow
							}
							break;
						case inGoAway:
							{
								// full screen no go away
							}
							break;
						case inZoomIn:
						case inZoomOut:
							{
								// full screen no zoom
							}
							break;
						case inSysWindow:
							break;
					}
					break;
				case keyDown:
				case autoKey:
				{
					SInt8 theKey;
					SInt8 theCode;
					theKey = theEvent.message & charCodeMask;
					theCode = (theEvent.message & keyCodeMask) >> 8;
					if ((theEvent.modifiers & cmdKey) != 0)
					{
						SInt32 menuResult = MenuKey (theKey);
						if (HiWord(menuResult) != 0)
							DoMenu (menuResult);
					}
					else
						DoKey (theKey, theCode);
				}
					break;
				case updateEvt:
				{
					UpdateWrapper (&theEvent);
				}
					break;
				case diskEvt:
					break;
				/*case osEvt:
					if (theEvent.message & 0x01000000)		//	Suspend/resume event
					{
						if (theEvent.message & 0x00000001)	//	Resume
						{
							gSleepTime = kForegroundSleep;	
							gfFrontProcess = true;
						}
						else
						{
							gSleepTime = kBackgroundSleep;	//	Suspend
							gfFrontProcess = false;
						}
					}
					break;*/

				case kHighLevelEvent:
					AEProcessAppleEvent (&theEvent);
					break;
			}
		}
	}
// profile
#if __profile__
	if (noErr == gProfErr)
		ProfilerSetStatus(FALSE); // turn profiling off again
#endif
}

// --------------------------------------------------------------------------

void CleanUp (void)
{
	MenuHandle hMenu;
	
// profile
#if __profile__
	if (noErr == gProfErr)
	{
		ProfilerDump ("\pGL DSp Carbon.prof");
		ProfilerTerm ();
	}
#endif

	game.Dispose();
	
#if defined kUseFades
	DSpDebugStr (DSpContext_FadeGammaOut (NULL, NULL)); // fade display, remove for debug
#endif

	RemoveEventLoopTimer(gTimer);
	gTimer = NULL;

	DeleteFontGL (gFontList);
	CleanupAGL (gaglContext); // Cleanup the OpenGL context
	gaglContext = 0;
	if (gpDSpPort)
	{
		ShutdownDSp (gpDSpPort); // DSp shutdown
		gpDSpPort = NULL;
	}

	
	hMenu = GetMenuHandle (kMenuFile);
	DeleteMenu (kMenuFile);
	DisposeMenu (hMenu);

	hMenu = GetMenuHandle (kMenuApple);
	DeleteMenu (kMenuApple);
	DisposeMenu (hMenu);

#if defined kUseFades
	DSpDebugStr (DSpContext_FadeGammaIn (NULL, NULL)); // fade display, remove for debug
#endif
	DSpShutdown ();
	
	ShowCursor();
}

// --------------------------------------------------------------------------
#include <iostream>
#include <InternetConfig.h>

/*void openURL()
{
#ifdef Q_WS_MAC
	QString url("http://www.wolfire.com/");
	ICInstance icInstance
	OSType psiSignature = 'Psi ';
	OSStatus error = ::ICStart( &icInstance, psiSignature);
	if(error=noErr){
		ConstStr255Param hint( 0x0 );
		const char* data = url.latin1();
		long length = url.length();
		long start( 0 );
		long end( length );
		::ICLaunchURL( icInstance, hint, data, length, &start, &end);
		ICStop( icInstance);
	}
	#endif
}*/

void LaunchURL(char *url)
{
	ICInstance gICInstance;
	ICStart (&gICInstance, 0);
	long int start,fin;
	start=0;
	fin=strlen(url);
	ICLaunchURL(gICInstance, "\p" ,url, fin, &start, &fin);
}
	
	
	
int main (void)
{
	try { 
		if (SetUp ())	{
			InstallHandlers();
			while (!gDone&&!game.quit&&(!game.tryquit||!game.registered)) {
				NewEvent ();
			}
		}
		CleanUp ();
		if(game.registernow){
			char url[100];
			sprintf(url,"http://www.wolfire.com/register.html");
			LaunchURL(url);
		}
		return 0;
	} catch (const std::exception& error) { 
		ofstream opstream("error.txt"); 
        opstream << "Caught exception: " << error.what() << std::endl;
	    opstream.close();
	}	
}
