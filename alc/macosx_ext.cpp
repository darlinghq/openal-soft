#include "config.h"

#include <atomic>
#include <cmath>
#include <vector>

#include "alcontext.h"
#include "alexcpt.h"
#include "macosx_ext.h"

namespace {

/* Mixer settings of ALC_EXT_MAC_OSX. They are process-wide and take effect
 * for contexts created afterwards. The defaults are the documented ones:
 * high-quality spatial rendering is 'rqhi', multichannel rendering is
 * 'rcmc', and the mixer bus count defaults to 64. An output rate of 0 means
 * "use the device's own rate".
 */
std::atomic<ALint> gRenderingQuality{'rqhi'};
std::atomic<ALint> gRenderChannelCount{'rcmc'};
std::atomic<ALint> gMixerMaxBusses{64};
std::atomic<ALdouble> gMixerOutputRate{0.0};

} // namespace

std::vector<ALCint> MacOSXAdjustContextAttrs(const ALCint *attrList)
{
    std::vector<ALCint> attrs;
    const ALdouble rate{gMixerOutputRate.load()};
    if(!(rate >= 1.0) || rate > 2147483647.0)
        return attrs;

    if(attrList)
    {
        for(const ALCint *a{attrList}; a[0]; a += 2)
        {
            if(a[0] == ALC_FREQUENCY)
                return attrs;
            attrs.push_back(a[0]);
            attrs.push_back(a[1]);
        }
    }
    attrs.push_back(ALC_FREQUENCY);
    attrs.push_back(static_cast<ALCint>(std::lround(rate)));
    attrs.push_back(0);
    return attrs;
}

extern "C" {

ALC_API ALvoid ALC_APIENTRY alcMacOSXRenderingQuality(ALint value)
{ gRenderingQuality.store(value); }

ALC_API ALint ALC_APIENTRY alcMacOSXGetRenderingQuality(void)
{ return gRenderingQuality.load(); }

ALC_API ALvoid ALC_APIENTRY alMacOSXRenderChannelCount(ALint value)
{ gRenderChannelCount.store(value); }

ALC_API ALint ALC_APIENTRY alMacOSXGetRenderChannelCount(void)
{ return gRenderChannelCount.load(); }

ALC_API ALvoid ALC_APIENTRY alcMacOSXMixerMaxiumumBusses(ALint value)
{ gMixerMaxBusses.store(value); }

ALC_API ALint ALC_APIENTRY alcMacOSXGetMixerMaxiumumBusses(void)
{ return gMixerMaxBusses.load(); }

ALC_API ALvoid ALC_APIENTRY alcMacOSXMixerOutputRate(ALdouble value)
{ gMixerOutputRate.store(value); }

ALC_API ALdouble ALC_APIENTRY alcMacOSXGetMixerOutputRate(void)
{
    /* Report the rate of the running mixer when there is a current context. */
    ContextRef context{GetContextRef()};
    if(context)
        return static_cast<ALdouble>(context->mDevice->Frequency);
    return gMixerOutputRate.load();
}

/* AL_EXT_STATIC_BUFFER: the buffer data is copied here, as alBufferData does.
 * The extension allows an implementation to read the caller's memory
 * directly; copying is observably the same except for callers that rewrite
 * the memory after the call and expect to be heard.
 */
AL_API ALvoid AL_APIENTRY alBufferDataStatic(const ALint bid, ALenum format, ALvoid *data,
    ALsizei size, ALsizei freq)
{
    alBufferData(static_cast<ALuint>(bid), format, data, size, freq);
}

/* ALC_EXT_ASA (reverb, occlusion, obstruction): not implemented. The entry
 * points exist so that alcGetProcAddress callers get a usable pointer, and
 * they report AL_INVALID_OPERATION, which the extension string does not
 * advertise.
 */
ALC_API ALenum ALC_APIENTRY alcASAGetSource(ALuint, ALuint, ALvoid*, ALuint*)
{ return AL_INVALID_OPERATION; }

ALC_API ALenum ALC_APIENTRY alcASASetSource(ALuint, ALuint, ALvoid*, ALuint)
{ return AL_INVALID_OPERATION; }

ALC_API ALenum ALC_APIENTRY alcASAGetListener(ALuint, ALvoid*, ALuint*)
{ return AL_INVALID_OPERATION; }

ALC_API ALenum ALC_APIENTRY alcASASetListener(ALuint, ALvoid*, ALuint)
{ return AL_INVALID_OPERATION; }

}
