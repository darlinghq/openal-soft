#ifndef ALC_MACOSX_EXT_H
#define ALC_MACOSX_EXT_H

/* Apple's OpenAL extensions (ALC_EXT_MAC_OSX, AL_EXT_STATIC_BUFFER and the
 * ALC_EXT_ASA entry points), as declared by the OpenAL framework's public
 * header MacOSX_OALExtensions.h. The functions are only reachable through
 * alGetProcAddress/alcGetProcAddress, like the proc pointer typedefs in that
 * header, and are not part of the OpenAL 1.1 API.
 */

#include <vector>

#include "AL/al.h"
#include "AL/alc.h"

extern "C" {

ALC_API ALvoid ALC_APIENTRY alcMacOSXRenderingQuality(ALint value);
ALC_API ALint ALC_APIENTRY alcMacOSXGetRenderingQuality(void);
ALC_API ALvoid ALC_APIENTRY alMacOSXRenderChannelCount(ALint value);
ALC_API ALint ALC_APIENTRY alMacOSXGetRenderChannelCount(void);
ALC_API ALvoid ALC_APIENTRY alcMacOSXMixerMaxiumumBusses(ALint value);
ALC_API ALint ALC_APIENTRY alcMacOSXGetMixerMaxiumumBusses(void);
ALC_API ALvoid ALC_APIENTRY alcMacOSXMixerOutputRate(ALdouble value);
ALC_API ALdouble ALC_APIENTRY alcMacOSXGetMixerOutputRate(void);

AL_API ALvoid AL_APIENTRY alBufferDataStatic(const ALint bid, ALenum format, ALvoid *data,
    ALsizei size, ALsizei freq);

ALC_API ALenum ALC_APIENTRY alcASAGetSource(ALuint property, ALuint source, ALvoid *data,
    ALuint *dataSize);
ALC_API ALenum ALC_APIENTRY alcASASetSource(ALuint property, ALuint source, ALvoid *data,
    ALuint dataSize);
ALC_API ALenum ALC_APIENTRY alcASAGetListener(ALuint property, ALvoid *data, ALuint *dataSize);
ALC_API ALenum ALC_APIENTRY alcASASetListener(ALuint property, ALvoid *data, ALuint dataSize);

}

/* Returns the attribute list for alcCreateContext with the output rate set by
 * alcMacOSXMixerOutputRate added, unless the caller gave ALC_FREQUENCY itself.
 * Returns an empty vector when nothing needs to change.
 */
std::vector<ALCint> MacOSXAdjustContextAttrs(const ALCint *attrList);

#endif /* ALC_MACOSX_EXT_H */
