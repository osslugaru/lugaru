project(lugaru)

cmake_minimum_required(VERSION 2.6)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel"
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(NOT LUGARU_INSTALL_PREFIX)
  if(WIN32)
     set(LUGARU_INSTALL_PREFIX C:\Lugaru CACHE PATH
     "LUGARU_INSTALL_PREFIX: Install path prefix, prepended onto install directories."
      FORCE)
  else(WIN32)
     set(LUGARU_INSTALL_PREFIX /usr/local/lugaru CACHE PATH
     "CMAKE_INSTALL_PREFIX: Install path prefix, prepended onto install directories."
      FORCE)
  endif(WIN32)
endif(NOT LUGARU_INSTALL_PREFIX)

set(CMAKE_INSTALL_PREFIX "${LUGARU_INSTALL_PREFIX}" CACHE INTERNAL "Prefix
prepended to install directories" FORCE)



set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OpenAL REQUIRED)
find_package(BZip2 REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)
find_package(SDL REQUIRED)
find_package(OggVorbis REQUIRED)

include_directories(
	${OPENAL_INCLUDES}
	${BZIP2_INCLUDE_DIR}
	${JPEG_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLU_INCLUDE_DIR}
	${SDL_INCLUDE_DIR}
	${VORBISFILE_INCLUDE_DIR}
	${OGG_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/Source)

set(LUGARU_LIBS ${OPENAL_LIBRARY} ${PNG_LIBRARY} ${JPEG_LIBRARY} ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES} ${SDL_LIBRARY} ${GLU_LIBRARY} ${OPENGL_LIBRARY} ${VORBISFILE_LIBRARY} ${OGG_LIBRARY})

add_subdirectory(Source)

# Install target
if(WIN32)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Source/lugaru.exe DESTINATION ${CMAKE_INSTALL_PREFIX})
else(WIN32)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Source/lugaru DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(WIN32)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data DESTINATION ${CMAKE_INSTALL_PREFIX})

