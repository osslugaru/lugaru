# Lugaru HD

[![build status](https://gitlab.com/osslugaru/lugaru/badges/master/build.svg)](https://gitlab.com/osslugaru/lugaru/commits/master)

This repository holds the official development effort of the open source game
Lugaru (and in particular of its HD version) for Linux, Mac OSX and Windows.

Developments from various forks have been integrated in the same code base in
an attempt to get a unified game to further develop (e.g. with new features,
new target platforms and new campaigns) and distribute.

This is the same game as the one distributed by Wolfire Games on
[Lugaru HD's website](http://www.wolfire.com/lugaru), though this repository
contains some further developments which are not (yet) featured in the
commercial version.

## Project links

This project is community-driven and uses the following communication channels:
- GitLab repository (including issues): https://gitlab.com/osslugaru/lugaru
- Project website: https://osslugaru.gitlab.io
- Mailing list: http://icculus.org/mailman/listinfo/lugaru
- IRC channel: [#lugaru on Freenode](http://webchat.freenode.net/?channels=lugaru)

## The game

Lugaru (pronounced Loo-GAH-roo) is a cross-platform third-person action game.
The main character, Turner, is an anthropomorphic rebel bunny rabbit with
impressive combat skills. In his quest to find those responsible for
slaughtering his village, he uncovers a far-reaching conspiracy involving the
corrupt leaders of the rabbit republic and the starving wolves from a nearby
den. Turner takes it upon himself to fight against their plot and save his
fellow rabbits from slavery.

## History

Lugaru was originally developed by David Rosen of [Wolfire Games](http://www.wolfire.com)
and was [open sourced in 2010](http://blog.wolfire.com/2010/05/Lugaru-goes-open-source).
It was made cross-platform with the help of [Ryan C. Gordon](http://icculus.org).

Various forks were made at that time, and the most interesting developments
were put back together under the OSS Lugaru organization, originally on
[Google Code](https://code.google.com/p/lugaru) and then on
[Bitbucket](https://bitbucket.org/osslugaru/lugaru).

This new repository on [GitLab](https://gitlab.com/osslugaru/lugaru) is run
by the same team, and aims at revitalizing the development effort to clean
things up, ensuring the code base builds and runs fine on all supported
platforms, and easing the packaging of Lugaru in Linux distributions.

Ideally, the updated code base could also be used to update the Lugaru HD
version sold by Wolfire Games once proven better than the current commercial
builds.

## Compiling

See the `COMPILING.md` for compilation instructions on all supported
platforms.

## Licensing

The source code is distributed under the GNU General Public License version 2
or (at your option) any later version (GPLv2+).

The assets (campaigns, graphical and audio assets, etc.) in the `Data` folder
are distributed under the Creative Commons Attribution - Share Alike license,
some in version 3.0 Unported (CC-BY-SA 3.0) and others in version 4.0
International (CC-BY-SA 4.0) as described in the `CONTENT-LICENSE.txt` file.
