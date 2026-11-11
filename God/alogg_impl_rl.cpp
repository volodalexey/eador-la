// alogg on top of raylib Music streams (RAYLIB.md, stage 8).
// This file reimplements the tiny subset of alogg used by Eador.
// raylib decodes Ogg Vorbis itself (stb_vorbis), so no libvorbis is needed.

#include <cstdlib>
#include <cstring>
// raylib.h must come before alogg.h → allegro_rl.h: Allegro KEY_* macros
// would otherwise clobber raylib's KeyboardKey enum.
#include <raylib.h>

#define ALOGG_DLL_DECLSPEC
#include <alogg.h>

struct ALOGG_OGG {
    // stb_vorbis reads straight from this buffer, it must outlive the Music.
    unsigned char *data;
    int data_len;

    Music music;
    int loaded;
    int buffer_len;       // frames per sub-buffer the Music was loaded with

    int volume, pan, speed, loop;
    int playing;
};

// Sub-buffer size is fixed when the stream is created, so the Music is
// (re)loaded with the buffer_len requested by alogg_play_ex_ogg.
static int alogg_load_music(ALOGG_OGG *ogg, int buffer_len) {
    if (ogg->loaded && ogg->buffer_len == buffer_len) return 1;
    if (ogg->loaded) {
        UnloadMusicStream(ogg->music);
        ogg->loaded = 0;
    }
    SetAudioStreamBufferSizeDefault(buffer_len);
    ogg->music = LoadMusicStreamFromMemory(".ogg", ogg->data, ogg->data_len);
    SetAudioStreamBufferSizeDefault(0);
    if (!IsMusicValid(ogg->music)) return 0;
    ogg->loaded = 1;
    ogg->buffer_len = buffer_len;
    return 1;
}

static void alogg_apply_params(ALOGG_OGG *ogg) {
    // vol 0-255 → 0.0-1.0, pan 128=center → 0.0, speed 1000=normal → pitch 1.0
    SetMusicVolume(ogg->music, ogg->volume / 255.0f);
    SetMusicPan(ogg->music, (ogg->pan - 128) / 128.0f);
    SetMusicPitch(ogg->music, ogg->speed / 1000.0f);
    ogg->music.looping = ogg->loop != 0;
}

ALOGG_DLL_DECLSPEC ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len) {
    if (!data || data_len <= 0) return nullptr;

    ALOGG_OGG *ogg = (ALOGG_OGG *)calloc(1, sizeof(ALOGG_OGG));
    if (!ogg) return nullptr;

    ogg->data = (unsigned char *)malloc((size_t)data_len);
    if (!ogg->data) { free(ogg); return nullptr; }
    memcpy(ogg->data, data, (size_t)data_len);
    ogg->data_len = data_len;

    // Load right away to reject broken data, as alogg does.
    if (!alogg_load_music(ogg, 0)) {
        free(ogg->data);
        free(ogg);
        return nullptr;
    }
    return ogg;
}

ALOGG_DLL_DECLSPEC void alogg_destroy_ogg(ALOGG_OGG *ogg) {
    if (!ogg) return;
    if (ogg->loaded) {
        StopMusicStream(ogg->music);
        UnloadMusicStream(ogg->music);
    }
    free(ogg->data);
    free(ogg);
}

ALOGG_DLL_DECLSPEC int alogg_play_ex_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed, int loop) {
    if (!ogg || ogg->playing) return -1;
    if (buffer_len < 256) buffer_len = 256;
    if (!alogg_load_music(ogg, buffer_len)) return -1;

    ogg->volume = vol;
    ogg->pan = pan;
    ogg->speed = speed;
    ogg->loop = loop;
    alogg_apply_params(ogg);

    PlayMusicStream(ogg->music);
    ogg->playing = 1;
    UpdateMusicStream(ogg->music);
    return 0;
}

ALOGG_DLL_DECLSPEC int alogg_play_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan) {
    return alogg_play_ex_ogg(ogg, buffer_len, vol, pan, 1000, 0);
}

ALOGG_DLL_DECLSPEC void alogg_adjust_ogg(ALOGG_OGG *ogg, int vol, int pan, int speed, int loop) {
    if (!ogg) return;
    ogg->volume = vol;
    ogg->pan = pan;
    ogg->speed = speed;
    ogg->loop = loop;
    if (ogg->loaded) alogg_apply_params(ogg);
}

ALOGG_DLL_DECLSPEC void alogg_stop_ogg(ALOGG_OGG *ogg) {
    if (!ogg || !ogg->playing) return;
    StopMusicStream(ogg->music);
    ogg->playing = 0;
}

ALOGG_DLL_DECLSPEC int alogg_poll_ogg(ALOGG_OGG *ogg) {
    if (!ogg || !ogg->playing) return ALOGG_POLL_NOTPLAYING;

    UpdateMusicStream(ogg->music);
    // UpdateMusicStream stops a non-looping Music once both sub-buffers are played.
    if (!IsMusicStreamPlaying(ogg->music)) {
        ogg->playing = 0;
        return ALOGG_POLL_PLAYJUSTFINISHED;
    }
    return ALOGG_OK;
}

ALOGG_DLL_DECLSPEC void alogg_rewind_ogg(ALOGG_OGG *ogg) {
    if (!ogg || !ogg->loaded) return;
    SeekMusicStream(ogg->music, 0.0f);
}

ALOGG_DLL_DECLSPEC int alogg_is_playing_ogg(ALOGG_OGG *ogg) {
    return ogg ? ogg->playing : 0;
}

ALOGG_DLL_DECLSPEC int alogg_is_looping_ogg(ALOGG_OGG *ogg) {
    return ogg ? ogg->loop : 0;
}

ALOGG_DLL_DECLSPEC int alogg_get_length_msecs_ogg(ALOGG_OGG *ogg) {
    if (!ogg || !ogg->loaded) return 0;
    return (int)(GetMusicTimeLength(ogg->music) * 1000.0f);
}

ALOGG_DLL_DECLSPEC int alogg_get_pos_msecs_ogg(ALOGG_OGG *ogg) {
    if (!ogg || !ogg->loaded) return 0;
    return (int)(GetMusicTimePlayed(ogg->music) * 1000.0f);
}

ALOGG_DLL_DECLSPEC int alogg_get_wave_freq_ogg(ALOGG_OGG *ogg) {
    return (ogg && ogg->loaded) ? (int)ogg->music.stream.sampleRate : 0;
}

ALOGG_DLL_DECLSPEC int alogg_get_wave_is_stereo_ogg(ALOGG_OGG *ogg) {
    return (ogg && ogg->loaded) ? (ogg->music.stream.channels > 1) : 0;
}

ALOGG_DLL_DECLSPEC int alogg_get_wave_bits_ogg(ALOGG_OGG *ogg) {
    (void)ogg;
    return 16;
}
