set(LUGARU_SRCS
	Frustum.cpp
	GameDraw.cpp
	GameInitDispose.cpp
	GameTick.cpp
	Globals.cpp
	Lights.cpp
	Models.cpp
	Objects.cpp
	OpenGL_Full_Screen.cpp
	OpenGL_Windows.cpp
	Person.cpp
	Quaternions.cpp
	Random.c
	Skeleton.cpp
	Skybox.cpp
	Sprites.cpp
	TGALoader.cpp
	Terrain.cpp
	Text.cpp
	Weapons.cpp
	WinInput.cpp
	mmgr.cpp
	nsp_network.c
	openal_wrapper.cpp
	pack.c
	pack_private.c
	private.c
	unpack.c
	unpack_private.c)

set(LUGARU_SRCS
	Frustum.cpp
	GameDraw.cpp
	GameInitDispose.cpp
	GameTick.cpp
	Globals.cpp
	Lights.cpp
	Models.cpp
	Objects.cpp
	pack.c
	pack_private.c
	Person.cpp
	private.c
	Quaternions.cpp
	Random.c
	Skeleton.cpp
	Skybox.cpp
	Sprites.cpp
	Terrain.cpp
	Text.cpp
	TGALoader.cpp
	unpack.c
	unpack_private.c
	Weapons.cpp
	MacCompatibility.cpp
	OpenGL_Windows.cpp
	openal_wrapper.cpp
	WinInput.cpp)

set(LUGARU_H
	Constants.h
	Frustum.h
	Game.h
	Lights.h
	LinkedList.h
	MacCompatibility.h
	MacInput.h
	Models.h
	MoreFilesX.h
	Objects.h
	Person.h
	PhysicsMath.h
	Pointer.h
	Quaternions.h
	Random.h
	Skeleton.h
	Skybox.h
	Sprites.h
	TGALoader.h
	Terrain.h
	Text.h
	Weapons.h
	WinInput.h
	alstubs.h
	binio.h
	fmod.h
	fmod_errors.h
	fmod_header.h
	fmoddyn.h
	gamegl.h
	glstubs.h
	md5.h
	mmgr.h
	nommgr.h
	pack_private.h
	private.h
	unpack_private.h
	wincompat.h)

if(CMAKE_HOST_APPLE)
	set(LUGARU_SRCS
		${LUGARU_SRCS}
		MacCompatibility.cpp
		MoreFilesX.c
		MacInput.cpp)
	set(LUGARU_H
		${LUGARU_H}
		"Carbon Include.h"
		CarbonStdCLib.h)
endif(CMAKE_HOST_APPLE)

if(WIN32)
  if(CMAKE_CROSSCOMPILING)
    ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lugaru_resource.obj
						  COMMAND i686-pc-mingw32-windres
							  -I${CMAKE_CURRENT_SOURCE_DIR}/res
							  -o ${CMAKE_CURRENT_BINARY_DIR}/lugaru_resource.obj
							  -i${CMAKE_CURRENT_SOURCE_DIR}/res/lugaru.rc
					      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/lugaru.rc
						  )
  endif(CMAKE_CROSSCOMPILING)
  if(NOT CMAKE_CROSSCOMPILING)
    ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lugaru_resource.obj
						  COMMAND windres
							  -I${CMAKE_CURRENT_SOURCE_DIR}/res
							  -o ${CMAKE_CURRENT_BINARY_DIR}/lugaru_resource.obj
							  -i${CMAKE_CURRENT_SOURCE_DIR}/res/lugaru.rc
					      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/lugaru.rc
						  )
  endif(NOT CMAKE_CROSSCOMPILING)

  set(LUGARU_SRCS
	${LUGARU_SRCS}
	WinDefs.cpp)

  set(LUGARU_H
	${LUGARU_H}
	WinDefs.h
	res/lugaru.h)
endif(WIN32)

if(WIN32)
add_executable(lugaru ${LUGARU_SRCS} ${LUGARU_H} lugaru_resource.obj)
else(WIN32)
add_executable(lugaru ${LUGARU_SRCS} ${LUGARU_H})
endif(WIN32)

target_link_libraries(lugaru ${LUGARU_LIBS})

if(WIN32)
	add_definitions(-DUSE_OPENAL=1 -DUSE_SDL=1 -DBinIO_STDINT_HEADER=<stdint.h>)
else(WIN32)
	add_definitions(-DPLATFORM_LINUX=1 -DPLATFORM_UNIX=1 -DUSE_OPENAL=1 -DUSE_SDL=1 -DBinIO_STDINT_HEADER=<stdint.h>)
endif(WIN32)

