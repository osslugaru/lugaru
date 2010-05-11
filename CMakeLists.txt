project(lugaru)

cmake_minimum_required(VERSION 2.6)

find_package(OpenAL REQUIRED)
find_package(BZip2 REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLU REQUIRED)
find_package(SDL REQUIRED)

include_directories(
	${OPENAL_INCLUDES}
	${BZIP2_INCLUDE_DIR}
	${JPEG_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${GLU_INCLUDE_DIR}
	${SDL_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/Source)

set(LUGARU_LIBS ${OPENAL_LIBRARY} ${PNG_LIBRARY} ${JPEG_LIBRARY} ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES} ${SDL_LIBRARY} ${GLU_LIBRARY} ${OPENGL_LIBRARY} -lvorbisfile -logg)

add_subdirectory(Source)
