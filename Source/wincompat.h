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

#if !defined(WINCOMPAT_INCLUDED) && !defined(PLATFORM_WINDOWS) && !defined(WIN32) && !defined(WINDOWS) && !defined(__WIN32__)
#define WINCOMPAT_INCLUDED

/**
 * 
 * Author: Magnus Naeslund (mag@fbab.net, mag@bahnhof.se)
 * (c) 2000 Magnus Naeslund, all rights reserved
 * 
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef TRUE
  #define TRUE 1
#endif
#ifndef FALSE
  #define FALSE 0
#endif

#define _kbhit kbhit
#define stricmp strcasecmp
#define strnicmp strncasecmp

#define Sleep(x) usleep((x)*1000)

static int            inited=0;
static struct termios ori;

static void tcatexit(){
   tcsetattr(0,0,&ori);
}

static void init_terminal(){
   struct termios t;
   tcgetattr(0,&t);
   tcgetattr(0,&ori);
   t.c_lflag &= ~(ICANON);
   tcsetattr(0,0,&t);
   atexit(tcatexit);
}

static inline int kbhit(){
  fd_set rfds;
  struct timeval tv;

   if (!inited){
	  inited=1;
	  init_terminal();
   }
   
   FD_ZERO(&rfds);
   FD_SET(0, &rfds);
   tv.tv_sec = 0;
   tv.tv_usec = 10*1000;
   return select(1, &rfds, NULL, NULL, &tv)>0;
}

static inline int getch(){
   fd_set rfds;
   
   if (!inited){
	  inited=1;
	  init_terminal();
   }
   
   FD_ZERO(&rfds);
   FD_SET(0, &rfds);
   if (select(1, &rfds, NULL, NULL, NULL)>0)
	 return getchar();
   else{
	  printf("wincompat.h: select() on fd 0 failed\n");
	  return 0xDeadBeef;
   }	 
}

#endif

