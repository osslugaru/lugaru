# OSS Lugaru Release Notes

This file contains the subsequent release notes for each version of Lugaru as
released by the OSS Lugaru team, in reverse chronological order.

The numbers preceded by a hash (e.g. #9) reference GitLab issue numbers on
the [OSS Lugaru repository](https://gitlab.com/osslugaru/lugaru).

## Version 1.1

First release of the open source Lugaru project! It contains most commits
made from various contributors since the open sourcing in 2010, and is
therefore more advanced than the preceding Lugaru HD release by Wolfire.

### Highlights

- Multiple campaigns support, making it easier to install and play mods!
- Those Lugaru mods are included by default as alternative to the Turner
  official campaign:
  * Temple, by Silb
  * Empire, by Jendraz
- All assets (animations, textures, models, maps, sounds, etc.) of Lugaru HD
  by Wolfire are under a free and open source license (CC-BY-SA 3.0)!
- Window and input management ported from SDL 1.2 to SDL 2.0, improving the
  support for modern screen resolutions and fullscreen mode, as well as
  better input handling.

### Detailed changes

**Project:**

- Migrated the project from Bitbucket to GitLab.com, the latter being more
  interesting feature-wise and based on an open source core
- Migrated the project from mercurial to git as the latter is better known,
  and necessary as part of the GitLab migration
- Set up continuous integration (CI) and binaries deployment with gitlab-ci:
  Linux GCC, Linux Clang, Windows MinGW32, Windows MinGW64
- Removed obsolete bundled dependencies, both binaries and source code

**Engine:**

- Added support for installing and playing multiple campaigns
  * The current campaign can be changed in the level selection menu
- Migrated from SDL 1.2 to SDL 2 (#2), with in particular:
  * improved screen resolution detection and handling, especially in
    fullscreen
  * switched input method from keycodes to scancodes
- Dehardcoded game data loading, making proper system-wide installs possible
  on Linux (#21)
- Renamed debug mode to devtools and improved the corresponding documentation
  Devtools are toggled with `--devtools` switch or adding a `Devtools: 1`
  entry to the config.txt (#32)
- Use standard directories to store game config and user data (#22):
  * Linux: XDG_CONFIG_HOME (default `~/.config/lugaru`) and XDG_DATA_HOME
    (default `~/.local/share/lugaru`)
  * OSX: `~/Library/Application Support/Lugaru`
  * Windows: `%APPDATA%\Lugaru`, i.e. `C:\Users\<you>\AppData\Roaming\Lugaru`
- Add Linux installation instructions, together with desktop file, metainfo,
  icon and man page (#15, #27, #28)
- Improved command line options parsing and added `--help` output (#14)
- Fixed `nomousegrab`option (#5)
- Added Fullscreen parameter in the options GUI, and Alt+Return as hotkey to
  toggle fullscreen mode
- Increased default resolution from 640x480 to 1024x768 and bumped default
  visual details to the maximum level (#15)
- Added "stereo" anaglyph 3D display mode to play with 3D glasses
- Lots of code refactoring and modernisation, fixed warnings
- Lots of bug fixes
- Fixed compilation against modern compilers (GCC 5/6 - including MinGW - and
  Clang 3.8/3.9 so far)

**Game content:**

- All Wolfire assets are now licensed under CC-BY-SA 3.0 by David Rosen,
  making Lugaru fully free (relicensed in commit 51203b5)
- Added the Temple campaign by Silb to the main repo (licensed under CC-BY-SA
  3.0, http://forums.wolfire.com/viewtopic.php?f=7&t=1066)
- Fixed erroneous hardcoded paths to textures in some maps
