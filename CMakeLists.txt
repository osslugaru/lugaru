project(lugaru)

cmake_minimum_required(VERSION 3.5)

include(FindPkgConfig)
include(GNUInstallDirs)


### Helpers

set(SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()


### CMake config

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-parentheses -pedantic --std=gnu++11 ${CMAKE_CXX_FLAGS}")

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for OSX")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING
        "Minimum OS X version to target for deployment (at runtime); newer APIs weak linked. Set to empty string for default value")
    set(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.11.sdk" CACHE PATH
        "The product will be built against the headers and libraries located inside the indicated SDK.")
endif()

if(WIN32 AND MINGW)
    # Recent mingw requires this, see https://sourceforge.net/p/mingw-w64/bugs/818/
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
endif()

if(LINUX)
    option(SYSTEM_INSTALL "Enable system-wide installation, with hardcoded data directory defined with CMAKE_INSTALL_DATADIR" OFF)
endif()


### Version

# Version for the current (stable) or next (development) release
set(LUGARU_VERSION_MAJOR 1)
set(LUGARU_VERSION_MINOR 3)
set(LUGARU_VERSION_PATCH 0)

# MAJOR.MINOR, or MAJOR.MINOR.PATCH if PATCH != 0
set(LUGARU_VERSION_NUMBER "${LUGARU_VERSION_MAJOR}.${LUGARU_VERSION_MINOR}")
if(LUGARU_VERSION_PATCH)
    set(LUGARU_VERSION_NUMBER "${LUGARU_VERSION_NUMBER}.${LUGARU_VERSION_PATCH}")
endif()

# Set to "" for stable (tagged) builds, "-dev" for dev builds
set(LUGARU_VERSION_SUFFIX "-dev")  # development
#set(LUGARU_VERSION_SUFFIX "")  # stable

# Set to 7-char git commit hash if available, otherwise "".
# On stable (tagged) builds, this is ignored.
set(LUGARU_VERSION_HASH "")
if(LUGARU_VERSION_SUFFIX STREQUAL "-dev" AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
    find_package(Git)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE "LUGARU_VERSION_HASH"
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        message(STATUS "Git commit hash: ${LUGARU_VERSION_HASH}")
    endif()
endif()

set(LUGARU_VERSION_RELEASE "" CACHE STRING "Optional release string, e.g. for distro packages release number")

# Final string built from the above constants, following the scheme:
# MAJOR.MINOR[.PATCH][-dev] [(git HASH)] [[RELEASE]]
set(LUGARU_VERSION_STRING "${LUGARU_VERSION_NUMBER}${LUGARU_VERSION_SUFFIX}")
if(NOT LUGARU_VERSION_HASH STREQUAL "")
    set(LUGARU_VERSION_STRING "${LUGARU_VERSION_STRING} (git ${LUGARU_VERSION_HASH})")
endif()
if(NOT LUGARU_VERSION_RELEASE STREQUAL "")
    set(LUGARU_VERSION_STRING "${LUGARU_VERSION_STRING} [${LUGARU_VERSION_RELEASE}]")
endif()

message(STATUS "Version string: ${LUGARU_VERSION_STRING}")
configure_file(${SRCDIR}/Version.hpp.in ${SRCDIR}/Version.hpp ESCAPE_QUOTES @ONLY)


### Sources

set(LUGARU_SRCS
    ${SRCDIR}/main.cpp
    ${SRCDIR}/Animation/Animation.cpp
    ${SRCDIR}/Animation/Joint.cpp
    ${SRCDIR}/Animation/Muscle.cpp
    ${SRCDIR}/Animation/Skeleton.cpp
    ${SRCDIR}/Audio/openal_wrapper.cpp
    ${SRCDIR}/Audio/Sounds.cpp
    ${SRCDIR}/Devtools/ConsoleCmds.cpp
    ${SRCDIR}/Environment/Lights.cpp
    ${SRCDIR}/Environment/Skybox.cpp
    ${SRCDIR}/Environment/Terrain.cpp
    ${SRCDIR}/Graphic/Decal.cpp
    ${SRCDIR}/Graphic/Models.cpp
    ${SRCDIR}/Graphic/Sprite.cpp
    ${SRCDIR}/Graphic/Stereo.cpp
    ${SRCDIR}/Graphic/Text.cpp
    ${SRCDIR}/Graphic/Texture.cpp
    ${SRCDIR}/Level/Awards.cpp
    ${SRCDIR}/Level/Campaign.cpp
    ${SRCDIR}/Level/Dialog.cpp
    ${SRCDIR}/Level/Hotspot.cpp
    ${SRCDIR}/Math/Frustum.cpp
    ${SRCDIR}/Math/XYZ.cpp
    ${SRCDIR}/Menu/Menu.cpp
    ${SRCDIR}/Objects/Object.cpp
    ${SRCDIR}/Objects/Person.cpp
    ${SRCDIR}/Objects/PersonType.cpp
    ${SRCDIR}/Objects/Weapons.cpp
    ${SRCDIR}/Platform/PlatformUnix.cpp
    ${SRCDIR}/Platform/PlatformWindows.cpp
    ${SRCDIR}/User/Account.cpp
    ${SRCDIR}/User/Settings.cpp
    ${SRCDIR}/Utils/Folders.cpp
    ${SRCDIR}/Utils/ImageIO.cpp
    ${SRCDIR}/Utils/Input.cpp
    ${SRCDIR}/Utils/pack.c
    ${SRCDIR}/Utils/private.c
    ${SRCDIR}/Utils/unpack.c
    ${SRCDIR}/Game.cpp
    ${SRCDIR}/GameDraw.cpp
    ${SRCDIR}/GameInitDispose.cpp
    ${SRCDIR}/GameTick.cpp
    ${SRCDIR}/Globals.cpp
    ${SRCDIR}/Tutorial.cpp

)

set(LUGARU_H
    ${SRCDIR}/Animation/Animation.hpp
    ${SRCDIR}/Animation/Joint.hpp
    ${SRCDIR}/Animation/Muscle.hpp
    ${SRCDIR}/Animation/Skeleton.hpp
    ${SRCDIR}/Audio/openal_wrapper.hpp
    ${SRCDIR}/Audio/Sounds.hpp
    ${SRCDIR}/Devtools/ConsoleCmds.hpp
    ${SRCDIR}/Environment/Lights.hpp
    ${SRCDIR}/Environment/Skybox.hpp
    ${SRCDIR}/Environment/Terrain.hpp
    ${SRCDIR}/Graphic/Decal.hpp
    ${SRCDIR}/Graphic/gamegl.hpp
    ${SRCDIR}/Graphic/Models.hpp
    ${SRCDIR}/Graphic/Sprite.hpp
    ${SRCDIR}/Graphic/Stereo.hpp
    ${SRCDIR}/Graphic/Text.hpp
    ${SRCDIR}/Graphic/Texture.hpp
    ${SRCDIR}/Level/Campaign.hpp
    ${SRCDIR}/Level/Dialog.hpp
    ${SRCDIR}/Level/Hotspot.hpp
    ${SRCDIR}/Math/Frustum.hpp
    ${SRCDIR}/Math/XYZ.hpp
    ${SRCDIR}/Math/Random.hpp
    ${SRCDIR}/Menu/Menu.hpp
    ${SRCDIR}/Objects/Object.hpp
    ${SRCDIR}/Objects/Person.hpp
    ${SRCDIR}/Objects/PersonType.hpp
    ${SRCDIR}/Objects/Weapons.hpp
    ${SRCDIR}/Platform/Platform.hpp
    ${SRCDIR}/Thirdparty/optionparser.h
    ${SRCDIR}/User/Account.hpp
    ${SRCDIR}/User/Settings.hpp
    ${SRCDIR}/Utils/binio.h
    ${SRCDIR}/Utils/Folders.hpp
    ${SRCDIR}/Utils/ImageIO.hpp
    ${SRCDIR}/Utils/Input.hpp
    ${SRCDIR}/Utils/private.h
    ${SRCDIR}/Game.hpp
    ${SRCDIR}/Tutorial.hpp

)

set(LUGARU_OBJS "")
if(WIN32)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Lugaru.res
                       COMMAND ${CMAKE_RC_COMPILER}
                       -o ${CMAKE_CURRENT_BINARY_DIR}/Lugaru.res
                       -i${SRCDIR}/Lugaru.rc
                       DEPENDS ${SRCDIR}/Lugaru.rc
    )
    set(LUGARU_OBJS "Lugaru.res")
endif()

if(APPLE)
    set(PLATFORM_LIBS "-framework Carbon -framework Cocoa -framework OpenGL -framework OpenAL")
endif()


### Dependencies

# TODO: Work on supporting "GLVND" instead.
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)

# Windows is funky about OpenAL detection
if(WIN32)
    pkg_check_modules(OPENAL openal REQUIRED)
    set(OPENAL_LIBRARY ${OPENAL_LIBRARIES})
else()
    find_package(OpenAL REQUIRED)
endif()

# macOS has problems with using pkgconfig to find SDL2
if(APPLE)
    find_package(sdl2 REQUIRED)
else()
    pkg_check_modules(SDL2 sdl2 REQUIRED)
endif()

find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

pkg_check_modules(VORBISFILE vorbisfile REQUIRED)
pkg_check_modules(JSONCPP jsoncpp REQUIRED)

include_directories(
    ${OPENAL_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${VORBISFILE_INCLUDE_DIR}
    ${JSONCPP_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/Source
)

set(LUGARU_LIBS ${OPENAL_LIBRARY} ${PNG_LIBRARY} ${JPEG_LIBRARY} ${SDL2_LIBRARIES} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${VORBISFILE_LIBRARIES} ${JSONCPP_LIBRARIES} ${PLATFORM_LIBS})


### Definitions

add_executable(lugaru ${LUGARU_SRCS} ${LUGARU_H} ${LUGARU_OBJS})
target_link_libraries(lugaru ${LUGARU_LIBS})

if(WIN32)
    add_definitions(-DBinIO_STDINT_HEADER=<stdint.h>)
    if(MINGW)
        # An alternative would be to use _WIN32 consistently instead of WIN32
        add_definitions(-DWIN32)
    endif()
else()
    add_definitions(-DPLATFORM_LINUX=1 -DPLATFORM_UNIX=1 -DBinIO_STDINT_HEADER=<stdint.h>)
endif()


### Installation

if(NOT CMAKE_INSTALL_PREFIX AND WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/Lugaru")
endif()

# OS-specific installation paths

set(LUGARU_DOCDIR ${CMAKE_INSTALL_PREFIX})

if(APPLE)
    set(LUGARU_APP_ROOT ${CMAKE_INSTALL_PREFIX}/Lugaru.app)
    set(LUGARU_BINDIR ${LUGARU_APP_ROOT}/Contents/MacOS)
    set(LUGARU_RESDIR ${LUGARU_APP_ROOT}/Contents/Resources)
endif()

# Actual installation instructions

if(WIN32)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lugaru.exe DESTINATION ${CMAKE_INSTALL_PREFIX})
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data DESTINATION ${CMAKE_INSTALL_PREFIX})
    if(MINGW)
        set(DLL_ROOT ${CMAKE_FIND_ROOT_PATH}/bin)
        # We need the shared libgcc, but it can have different names based on arch/config/version.
        # Ideally we'd check what our deps and lugaru.exe actually require, but for now it's a hack.
        # Based off Mageia/Fedora MinGW toolchain, might not work on other distros or Windows
        if(EXISTS ${DLL_ROOT}/libgcc_s_seh-1.dll)
            set(LIBGCC_S libgcc_s_seh-1.dll)
        elseif(EXISTS ${DLL_ROOT}/libgcc_s_dw2-1.dll)
            set(LIBGCC_S libgcc_s_dw2-1.dll)
        elseif(EXISTS ${DLL_ROOT}/libgcc_s_sjlj-1.dll)
            set(LIBGCC_S libgcc_s_sjlj-1.dll)
        endif()
        # FIXME: Filter out unneeded DLLs when building against some internal deps
        install(FILES ${DLL_ROOT}/${LIBGCC_S}
                      ${DLL_ROOT}/libjpeg-62.dll
                      ${DLL_ROOT}/libjsoncpp.dll
                      ${DLL_ROOT}/libogg-0.dll
                      ${DLL_ROOT}/libpng16-16.dll
                      ${DLL_ROOT}/libssp-0.dll
                      ${DLL_ROOT}/libstdc++-6.dll
                      ${DLL_ROOT}/libvorbis-0.dll
                      ${DLL_ROOT}/libvorbisfile-3.dll
                      ${DLL_ROOT}/libwinpthread-1.dll
                      ${DLL_ROOT}/OpenAL32.dll
                      ${DLL_ROOT}/SDL2.dll
                      ${DLL_ROOT}/zlib1.dll
                DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()
endif()

if(LINUX)
    if(SYSTEM_INSTALL)
        add_definitions(-DDATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
        set(LUGARU_DOCDIR ${CMAKE_INSTALL_DOCDIR})
        install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lugaru DESTINATION ${CMAKE_INSTALL_BINDIR})
        # Trailing '/' is significant, it installs and _renames_ Data/ as the destination folder
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data/ DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
        install(FILES ${CMAKE_SOURCE_DIR}/Dist/Linux/lugaru.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
        install(FILES ${CMAKE_SOURCE_DIR}/Dist/Linux/lugaru.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
        install(FILES ${CMAKE_SOURCE_DIR}/Dist/Linux/lugaru.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps)
        install(FILES ${CMAKE_SOURCE_DIR}/Dist/Linux/lugaru.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
    else()
        message("You are building Lugaru without having enabled the SYSTEM_INSTALL option. It will default to looking for the data in the 'Data' directory next to the binary.")
        install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lugaru DESTINATION ${CMAKE_INSTALL_PREFIX})
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()
endif()

if(APPLE)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lugaru DESTINATION ${LUGARU_BINDIR})
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/Data DESTINATION ${LUGARU_APP_ROOT})
    install(FILES ${CMAKE_SOURCE_DIR}/Dist/OSX/Lugaru.icns DESTINATION ${LUGARU_RESDIR})
    install(FILES ${CMAKE_SOURCE_DIR}/Dist/OSX/Info.plist DESTINATION ${LUGARU_APP_ROOT}/Contents)
endif()

# Documentation

install(FILES ${CMAKE_SOURCE_DIR}/AUTHORS
              ${CMAKE_SOURCE_DIR}/CONTENT-LICENSE.txt
              ${CMAKE_SOURCE_DIR}/COPYING.txt
              ${CMAKE_SOURCE_DIR}/README.md
              ${CMAKE_SOURCE_DIR}/RELEASE-NOTES.md
              ${CMAKE_SOURCE_DIR}/Docs/DEVTOOLS.txt
              ${CMAKE_SOURCE_DIR}/Docs/README.Empire.txt
              ${CMAKE_SOURCE_DIR}/Docs/README.SevenTasks.txt
              ${CMAKE_SOURCE_DIR}/Docs/README.Temple.txt
        DESTINATION ${LUGARU_DOCDIR})
