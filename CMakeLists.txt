project(lugaru)

cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0004 OLD)

include(FindPkgConfig)

set(SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")


### CMake config

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wno-parentheses -pedantic -g -pg --std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wno-parentheses -O2 -std=c++11")

if(NOT CMAKE_INSTALL_PREFIX AND WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/Lugaru")
endif(NOT CMAKE_INSTALL_PREFIX AND WIN32)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for OSX")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING
        "Minimum OS X version to target for deployment (at runtime); newer APIs weak linked. Set to empty string for default value")
    set(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.10.sdk" CACHE PATH
        "The product will be built against the headers and libraries located inside the indicated SDK.")
endif(APPLE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


### Sources

set(LUGARU_SRCS
    ${SRCDIR}/main.cpp
    ${SRCDIR}/Frustum.cpp
    ${SRCDIR}/Account.cpp
    ${SRCDIR}/ConsoleCmds.cpp
    ${SRCDIR}/Dialog.cpp
    ${SRCDIR}/Game.cpp
    ${SRCDIR}/GameDraw.cpp
    ${SRCDIR}/GameInitDispose.cpp
    ${SRCDIR}/GameTick.cpp
    ${SRCDIR}/Globals.cpp
    ${SRCDIR}/Lights.cpp
    ${SRCDIR}/Menu.cpp
    ${SRCDIR}/Models.cpp
    ${SRCDIR}/Objects.cpp
    ${SRCDIR}/pack.c
    ${SRCDIR}/Person.cpp
    ${SRCDIR}/private.c
    ${SRCDIR}/Quaternions.cpp
    ${SRCDIR}/Skeleton.cpp
    ${SRCDIR}/Skybox.cpp
    ${SRCDIR}/Sprite.cpp
    ${SRCDIR}/Terrain.cpp
    ${SRCDIR}/Texture.cpp
    ${SRCDIR}/Text.cpp
    ${SRCDIR}/ImageIO.cpp
    ${SRCDIR}/unpack.c
    ${SRCDIR}/Weapons.cpp
    ${SRCDIR}/openal_wrapper.cpp
    ${SRCDIR}/Input.cpp
    ${SRCDIR}/Settings.cpp
    ${SRCDIR}/Stereo.cpp
    ${SRCDIR}/Animation.cpp
    ${SRCDIR}/Sounds.cpp
    ${SRCDIR}/Awards.cpp
)

set(LUGARU_H
    ${SRCDIR}/Frustum.h
    ${SRCDIR}/Account.h
    ${SRCDIR}/ConsoleCmds.h
    ${SRCDIR}/Dialog.h
    ${SRCDIR}/Game.h
    ${SRCDIR}/Lights.h
    ${SRCDIR}/Menu.h
    ${SRCDIR}/Models.h
    ${SRCDIR}/Objects.h
    ${SRCDIR}/Person.h
    ${SRCDIR}/PhysicsMath.h
    ${SRCDIR}/Quaternions.h
    ${SRCDIR}/Random.h
    ${SRCDIR}/Skeleton.h
    ${SRCDIR}/Skybox.h
    ${SRCDIR}/Sprite.h
    ${SRCDIR}/ImageIO.h
    ${SRCDIR}/Terrain.h
    ${SRCDIR}/Texture.h
    ${SRCDIR}/Text.h
    ${SRCDIR}/Weapons.h
    ${SRCDIR}/Input.h
    ${SRCDIR}/binio.h
    ${SRCDIR}/openal_wrapper.h
    ${SRCDIR}/optionparser.h
    ${SRCDIR}/gamegl.h
    ${SRCDIR}/glstubs.h
    ${SRCDIR}/private.h
    ${SRCDIR}/Settings.h
    ${SRCDIR}/Stereo.h
    ${SRCDIR}/Animation.h
    ${SRCDIR}/Sounds.h
)

if(UNIX)
    set(LUGARU_SRCS
        ${LUGARU_SRCS}
        ${SRCDIR}/MacCompatibility.cpp
    )
    set(LUGARU_H
        ${LUGARU_H}
        ${SRCDIR}/MacCompatibility.h
    )
endif(UNIX)

if(WIN32)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lugaru_resource.obj
                       COMMAND ${CMAKE_RC_COMPILER}
                       -I${SRCDIR}/win-res
                       -o ${CMAKE_CURRENT_BINARY_DIR}/lugaru_resource.obj
                       -i${SRCDIR}/win-res/Lugaru.rc
                       DEPENDS ${SRCDIR}/win-res/Lugaru.rc
    )

    # FIXME: get rid of this.
    set(LUGARU_SRCS
        ${LUGARU_SRCS}
        ${SRCDIR}/WinDefs.cpp)

    set(LUGARU_H
        ${LUGARU_H}
        ${SRCDIR}/WinDefs.h
        ${SRCDIR}/win-res/resource.h)
endif(WIN32)

if(APPLE)
    set(PLATFORM_LIBS "-framework Carbon -framework Cocoa -framework OpenGL -framework OpenAL")
endif(APPLE)


### Dependencies

find_package(OpenGL REQUIRED)

# Windows is funky about OpenAL detection
if(WIN32)
    pkg_check_modules(OPENAL openal REQUIRED)
    set(OPENAL_LIBRARY ${OPENAL_LIBRARIES})
else(WIN32)
    find_package(OpenAL REQUIRED)
endif(WIN32)

find_package(sdl2 REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OggVorbis REQUIRED)

include_directories(
    ${OPENAL_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${VORBISFILE_INCLUDE_DIR}
    ${OGG_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/Source
)

set(LUGARU_LIBS ${OPENAL_LIBRARY} ${PNG_LIBRARY} ${JPEG_LIBRARY} ${ZLIB_LIBRARIES} ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${VORBISFILE_LIBRARY} ${OGG_LIBRARY} ${PLATFORM_LIBS})


### Definitions

if(WIN32)
    add_executable(lugaru ${LUGARU_SRCS} ${LUGARU_H} lugaru_resource.obj)
else(WIN32)
    add_executable(lugaru ${LUGARU_SRCS} ${LUGARU_H})
endif(WIN32)

target_link_libraries(lugaru ${LUGARU_LIBS})

if(WIN32)
    add_definitions(-DBinIO_STDINT_HEADER=<stdint.h>)
    if(MINGW)
        # An alternative would be to use _WIN32 consistently instead of WIN32
        add_definitions(-DWIN32)
    endif(MINGW)
else(WIN32)
    add_definitions(-DPLATFORM_LINUX=1 -DPLATFORM_UNIX=1 -DBinIO_STDINT_HEADER=<stdint.h>)
endif(WIN32)


### Installation

if(APPLE)
    set(APPS_ROOT "${CMAKE_INSTALL_PREFIX}/Lugaru.app")
    set(APPS_BIN "${APPS_ROOT}/Contents/MacOS")
    set(APPS_DATA "${APPS_ROOT}/Contents/Resources")
endif(APPLE)

if(WIN32)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lugaru.exe DESTINATION ${CMAKE_INSTALL_PREFIX})
    if(MINGW)
        # Based off Mageia/Fedora MinGW toolchain, might not work on other distros or Windows
        set(LIBGCC_S libgcc_s_sjlj-1.dll)
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") # MinGW64
            set(LIBGCC_S libgcc_s_seh-1.dll)
        endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        # FIXME: Filter out unneeded DLLs when building against some internal deps
        set(DLL_ROOT ${CMAKE_FIND_ROOT_PATH}/bin)
        install(FILES ${DLL_ROOT}/${LIBGCC_S}
                      ${DLL_ROOT}/libjpeg-62.dll
                      ${DLL_ROOT}/libogg-0.dll
                      ${DLL_ROOT}/libpng16-16.dll
                      ${DLL_ROOT}/libstdc++-6.dll
                      ${DLL_ROOT}/libvorbis-0.dll
                      ${DLL_ROOT}/libvorbisfile-3.dll
                      ${DLL_ROOT}/libwinpthread-1.dll
                      ${DLL_ROOT}/OpenAL32.dll
                      ${DLL_ROOT}/SDL2.dll
                      ${DLL_ROOT}/zlib1.dll
                DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif(MINGW)
else(WIN32)
    if(APPLE)
        set(CMAKE_INSTALL_PREFIX "${APPS_BIN}")
        install(FILES ${SRCDIR}/mac-res/lugaru.icns DESTINATION ${APPS_DATA})
        install(FILES ${SRCDIR}/mac-res/Info.plist DESTINATION ${APPS_ROOT}/Contents)
    endif(APPLE)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lugaru DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(WIN32)

if(NOT APPLE)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(FILES ${CMAKE_SOURCE_DIR}/README.md
                  ${CMAKE_SOURCE_DIR}/COPYING.txt
                  ${CMAKE_SOURCE_DIR}/CONTENT-LICENSE.txt
            DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(NOT APPLE)

if(APPLE)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data DESTINATION ${APPS_ROOT})
endif(APPLE)
