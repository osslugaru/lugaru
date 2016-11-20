/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Lugaru is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lugaru.  If not, see <http://www.gnu.org/licenses/>.
*/


AL_FUNC(AL, void, alGenSources, (ALsizei n, ALuint* sources), (n, sources),)
AL_FUNC(AL, void, alDeleteSources, (ALsizei n, ALuint* sources), (n, sources),)
AL_FUNC(AL, void, alSourcei, (ALuint sid, ALenum param, ALint value), (sid, param, value),)
AL_FUNC(AL, void, alSourcef, (ALuint sid, ALenum param, ALfloat value), (sid, param, value),)
AL_FUNC(AL, void, alSource3f, (ALuint sid, ALenum param, ALfloat f1, ALfloat f2, ALfloat f3), (sid, param, f1, f2, f3),)
AL_FUNC(AL, void, alGetSourceiv, (ALuint sid, ALenum pname, ALint* values), (sid, pname, values),)
AL_FUNC(AL, void, alSourcePlay, (ALuint sid), (sid),)
AL_FUNC(AL, void, alSourcePause, (ALuint sid), (sid),)
AL_FUNC(AL, void, alSourceStop, (ALuint sid), (sid),)
AL_FUNC(AL, void, alGenBuffers, (ALsizei n, ALuint* buffers), (n, buffers),)
AL_FUNC(AL, void, alDeleteBuffers, (ALsizei n, ALuint* buffers), (n, buffers),)
AL_FUNC(AL, void, alBufferData, (ALuint buffer, ALenum format, ALvoid *data, ALsizei size, ALsizei freq), (buffer, format, data, size, freq),)
AL_FUNC(AL, void, alListenerfv, (ALenum pname, ALfloat* param), (pname, param),)
AL_FUNC(AL, void, alListener3f, (ALenum pname, ALfloat f1, ALfloat f2, ALfloat f3), (pname, f1, f2, f3),)
AL_FUNC(AL, void, alListenerf, (ALenum pname, ALfloat param), (pname, param),)
AL_FUNC(AL, ALenum, alGetError, (ALvoid), (), return)
AL_FUNC(AL, const ALubyte*, alGetString, (ALenum param), (param), return)
AL_FUNC(AL, ALboolean, alIsExtensionPresent, (const ALubyte* fname), (fname), return)
AL_FUNC(AL, ALenum, alGetEnumValue, (const ALubyte* ename), (ename), return)
AL_FUNC(ALC, ALCcontext *, alcCreateContext, (ALCdevice *dev, ALint* attrlist), (dev, attrlist), return)
AL_FUNC(ALC, ALCenum, alcMakeContextCurrent, (ALCcontext *alcHandle), (alcHandle), return)
AL_FUNC(ALC, ALCcontext *, alcProcessContext, (ALCcontext *alcHandle), (alcHandle), return)
AL_FUNC(ALC, void, alcSuspendContext, (ALCcontext *alcHandle), (alcHandle),)
AL_FUNC(ALC, ALCenum, alcDestroyContext, (ALCcontext *alcHandle), (alcHandle), return)
AL_FUNC(ALC, ALCenum, alcGetError, (ALCdevice *dev), (dev), return)
AL_FUNC(ALC, ALCcontext *, alcGetCurrentContext, (ALvoid), (), return)
AL_FUNC(ALC, ALCdevice *, alcOpenDevice, (const ALubyte *tokstr), (tokstr), return)
AL_FUNC(ALC, void, alcCloseDevice, (ALCdevice *dev), (dev),)
AL_FUNC(ALC, ALCdevice*, alcGetContextsDevice, (ALCcontext *context), (context), return)
