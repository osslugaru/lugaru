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

#ifndef _FMOD_ERRORS_H
#define _FMOD_ERRORS_H

static char *FMOD_ErrorString(int errcode)
{
	switch (errcode)
	{
		case FMOD_ERR_NONE:				return "No errors";
		case FMOD_ERR_BUSY:				return "Cannot call this command after FSOUND_Init.  Call FSOUND_Close first.";
		case FMOD_ERR_UNINITIALIZED:	return "This command failed because FSOUND_Init was not called";
		case FMOD_ERR_PLAY:				return "Playing the sound failed.";
		case FMOD_ERR_INIT:				return "Error initializing output device.";
		case FMOD_ERR_ALLOCATED:		return "The output device is already in use and cannot be reused.";
		case FMOD_ERR_OUTPUT_FORMAT:	return "Soundcard does not support the features needed for this soundsystem (16bit stereo output)";
		case FMOD_ERR_COOPERATIVELEVEL:	return "Error setting cooperative level for hardware.";
		case FMOD_ERR_CREATEBUFFER:		return "Error creating hardware sound buffer.";
		case FMOD_ERR_FILE_NOTFOUND:	return "File not found";
		case FMOD_ERR_FILE_FORMAT:		return "Unknown file format";
		case FMOD_ERR_FILE_BAD:			return "Error loading file";
		case FMOD_ERR_MEMORY:			return "Not enough memory ";
		case FMOD_ERR_VERSION:			return "The version number of this file format is not supported";
		case FMOD_ERR_INVALID_PARAM:	return "An invalid parameter was passed to this function";
		case FMOD_ERR_NO_EAX:			return "Tried to use an EAX command on a non EAX enabled channel or output.";
		case FMOD_ERR_CHANNEL_ALLOC:	return "Failed to allocate a new channel";
		case FMOD_ERR_RECORD:			return "Recording not supported on this device";
		case FMOD_ERR_MEDIAPLAYER:		return "Required Mediaplayer codec is not installed";

		default :						return "Unknown error";
	};
}

#endif

