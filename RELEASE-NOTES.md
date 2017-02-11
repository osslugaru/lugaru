# OSS Lugaru Release Notes

This file contains the subsequent release notes for each version of Lugaru as
released by the OSS Lugaru team, in reverse chronological order.

The numbers preceded by a hash (e.g. #9) reference GitLab issue numbers on
the [OSS Lugaru repository](https://gitlab.com/osslugaru/lugaru).

## Version 1.2 - 11 February 2017

Second release of the open source Lugaru project! It continues the refactoring
work to make the old Lugaru code more modular and easier to maintain and
extend. A particular attention was given to the development tools (in game
editor) to improve the modability of the game.

### Highlights

- Keep track of and display the game's version number in the console and
  main menu.
- Many fixes and improvements to development tools, most of them should now
  work as described in the documentation.
- New monospace font for the console, more readable.
- Configurable campaign end message.
- Various fixes to mod levels and campaigns.
- A lot of behind-the-scenes refactoring that should help bring more
  interesting changes in future releases.

### Detailed changes

**Engine:**

- Fix displaying the localized key names instead of the QWERTY ones (#62)
- Fixed bushes appearing above ground on first level loaded
- Add a monospace font for the console text to increase legibility (#23)
- Devtools: improve handling of `map` command to avoid crashes (#23)
- Devtools: prevent the `clothes` command from saving invalid paths (#59)
  * Added `clotheslist` command to inspect player clothes
- Devtools: reviewed and fixed conflicting hotkeys and crashes (#73, #89)
- Devtools: fix regression from 1.1 in changing player type (wolf/rabbit) (#88)
  * Some bindings were changed, see `Docs/DEVTOOLS.txt` for the updated list.
- Campaign end message now configurable in the campaign definition file (#60)
- Fix long dialog lines printed out of the dialog area (#86)
- Lots of code refactoring and modernisation, fixed warnings
- Many bug fixes

**Game content:**

- Fix incorrect number of levels in the Seven Tasks campaign file (caused a
  crash on Win32) (#70)
- Fix crash in "Temple of the Wind" (sventemple) and "Snowy Grotto"
  (svengrotto) levels (#76, #79)
- Add and fix missing black fur texture for Ash in the Empire campaign (#84)

**Project:**

- Applied `clang-format` code formatting to the source, using a custom
  Mozilla-based style (#18, !14)
- Start properly versioning the game with version number displayed in the menu
  and console (#82, !15)


## Version 1.1 - 14 December 2016

First release of the open source Lugaru project! It contains most commits
made from various contributors since the open sourcing in 2010, and is
therefore more advanced than the preceding Lugaru HD release by Wolfire.

### Highlights

- Multiple campaigns support, making it easier to install and play mods!
- Those Lugaru mods are included by default as alternative to the Turner
  official campaign:
  * Temple, by Silb
  * Empire, by Jendraz
  * The Seven Tasks, by Philtron R.
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
- Added automatic progress saving upon level completion, and not only when
  closing the game.
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
