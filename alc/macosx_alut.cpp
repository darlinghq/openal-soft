#include "config.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "AL/al.h"

/* The alut WAVE loaders exported by the OpenAL framework (declared in its
 * alut.h): a minimal reader for uncompressed 8 and 16 bit mono and stereo
 * PCM in a RIFF/WAVE container. The sample data is returned in memory
 * allocated here and is released with alutUnloadWAV. On failure the outputs
 * are zeroed.
 */

namespace {

uint16_t rd16(const uint8_t *p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }
uint32_t rd32(const uint8_t *p)
{
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8)
        | (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

void clearOutputs(ALenum *format, ALvoid **data, ALsizei *size, ALsizei *freq)
{
    if(format) *format = 0;
    if(data) *data = nullptr;
    if(size) *size = 0;
    if(freq) *freq = 0;
}

/* len bounds all reads. */
bool parseWave(const uint8_t *mem, size_t len, ALenum *format, ALvoid **data, ALsizei *size,
    ALsizei *freq)
{
    if(len < 12 || memcmp(mem, "RIFF", 4) != 0 || memcmp(mem+8, "WAVE", 4) != 0)
        return false;

    size_t riffEnd{static_cast<size_t>(rd32(mem+4)) + 8};
    if(riffEnd > len || riffEnd < 12) riffEnd = len;

    bool haveFmt{false};
    unsigned channels{0}, bits{0};
    uint32_t rate{0};
    size_t pos{12};
    while(pos + 8 <= riffEnd)
    {
        const uint8_t *chunk{mem + pos};
        size_t chunkLen{rd32(chunk+4)};
        const size_t body{pos + 8};
        if(chunkLen > riffEnd - body) chunkLen = riffEnd - body;

        if(memcmp(chunk, "fmt ", 4) == 0)
        {
            if(chunkLen < 16) return false;
            unsigned tag{rd16(chunk+8)};
            if(tag == 0xFFFE && chunkLen >= 26)
                tag = rd16(chunk+8+24); /* WAVE_FORMAT_EXTENSIBLE: first word of the sub-format */
            if(tag != 1) return false; /* only PCM */
            channels = rd16(chunk+10);
            rate = rd32(chunk+12);
            bits = rd16(chunk+22);
            haveFmt = true;
        }
        else if(memcmp(chunk, "data", 4) == 0 && haveFmt)
        {
            ALenum fmt{0};
            if(channels == 1 && bits == 8) fmt = AL_FORMAT_MONO8;
            else if(channels == 1 && bits == 16) fmt = AL_FORMAT_MONO16;
            else if(channels == 2 && bits == 8) fmt = AL_FORMAT_STEREO8;
            else if(channels == 2 && bits == 16) fmt = AL_FORMAT_STEREO16;
            if(!fmt || rate == 0 || rate > 0x7fffffffu || chunkLen > 0x7fffffffu)
                return false;

            void *copy{malloc(chunkLen ? chunkLen : 1)};
            if(!copy) return false;
            memcpy(copy, mem + body, chunkLen);
            if(format) *format = fmt;
            if(data) *data = copy; else free(copy);
            if(size) *size = static_cast<ALsizei>(chunkLen);
            if(freq) *freq = static_cast<ALsizei>(rate);
            return true;
        }

        pos = body + chunkLen + (chunkLen & 1u); /* chunks are word aligned */
    }
    return false;
}

} // namespace

extern "C" {

AL_API ALvoid AL_APIENTRY alutInit(ALint*, ALbyte**)
{ }

AL_API ALvoid AL_APIENTRY alutExit(ALvoid)
{ }

AL_API ALvoid AL_APIENTRY alutLoadWAVFile(ALbyte *file, ALenum *format, ALvoid **data,
    ALsizei *size, ALsizei *freq)
{
    clearOutputs(format, data, size, freq);
    if(!file) return;

    FILE *f{fopen(reinterpret_cast<const char*>(file), "rb")};
    if(!f) return;
    std::vector<uint8_t> contents;
    uint8_t buf[4096];
    size_t got;
    while((got = fread(buf, 1, sizeof(buf), f)) > 0)
        contents.insert(contents.end(), buf, buf+got);
    fclose(f);

    if(!parseWave(contents.data(), contents.size(), format, data, size, freq))
        clearOutputs(format, data, size, freq);
}

AL_API ALvoid AL_APIENTRY alutLoadWAVMemory(ALbyte *memory, ALenum *format, ALvoid **data,
    ALsizei *size, ALsizei *freq)
{
    clearOutputs(format, data, size, freq);
    if(!memory) return;

    /* The caller gives no length; the RIFF header states it. */
    const uint8_t *mem{reinterpret_cast<const uint8_t*>(memory)};
    if(memcmp(mem, "RIFF", 4) != 0) return;
    const size_t len{static_cast<size_t>(rd32(mem+4)) + 8};
    if(!parseWave(mem, len, format, data, size, freq))
        clearOutputs(format, data, size, freq);
}

AL_API ALvoid AL_APIENTRY alutUnloadWAV(ALenum, ALvoid *data, ALsizei, ALsizei)
{
    free(data);
}

}
