/* alogg_impl.cpp
 *
 * A self-contained, Linux-native reimplementation of the tiny subset of the
 * "alogg" (Allegro OGG) streaming API used by Eador (God).
 *
 * The original alogg library was Windows-only and bound to the old Allegro 4
 * DMA-style audio stream API.  Allegro 4.4 instead provides double-buffered
 * audio streams (play_audio_stream / get_audio_stream_buffer /
 * free_audio_stream_buffer), so we decode Ogg Vorbis with libvorbisfile and
 * feed the decoded PCM into one of those streams.
 *
 * Only the functions the game actually references are implemented here.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vorbis/vorbisfile.h>

#define ALOGG_DLL_DECLSPEC
#include <alogg.h>

/* ------------------------------------------------------------------ */
/*  In-memory Ogg source (callbacks for libvorbisfile)                 */
/* ------------------------------------------------------------------ */

struct alogg_membuf {
    const unsigned char *data;
    long                size;
    long                pos;
};

static size_t alogg_read_cb(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    struct alogg_membuf *m = (struct alogg_membuf *)datasource;
    size_t requested = size * nmemb;
    long avail = m->size - m->pos;
    size_t got = (requested < (size_t)avail) ? requested : (size_t)avail;

    if (got > 0) {
        memcpy(ptr, m->data + m->pos, got);
        m->pos += (long)got;
    }
    return got;
}

static int alogg_seek_cb(void *datasource, ogg_int64_t offset, int whence)
{
    struct alogg_membuf *m = (struct alogg_membuf *)datasource;
    long base;

    switch (whence) {
        case SEEK_SET: base = 0; break;
        case SEEK_CUR: base = m->pos; break;
        case SEEK_END: base = m->size; break;
        default: return -1;
    }

    long np = base + (long)offset;
    if (np < 0) np = 0;
    if (np > m->size) np = m->size;
    m->pos = np;
    return 0;
}

static int alogg_close_cb(void *datasource)
{
    (void)datasource;
    return 0;
}

static long alogg_tell_cb(void *datasource)
{
    struct alogg_membuf *m = (struct alogg_membuf *)datasource;
    return m->pos;
}

/* ------------------------------------------------------------------ */
/*  ALOGG_OGG                                                          */
/* ------------------------------------------------------------------ */

struct ALOGG_OGG {
    unsigned char      *data;
    long               data_len;
    struct alogg_membuf mem;

    OggVorbis_File     vf;
    int                vf_initialized;

    AUDIOSTREAM        *stream;
    int                sample_len;    /* PCM frames per stream fragment */
    int                stereo;
    int                freq;
    int                volume, pan, speed, loop;
    int                playing;
    int                finished;

    ogg_int64_t        total_samples; /* for length queries */
};

static ALOGG_OGG *alogg_alloc_and_open(void *data, int data_len)
{
    ALOGG_OGG *ogg = (ALOGG_OGG *)calloc(1, sizeof(ALOGG_OGG));
    if (!ogg)
        return NULL;

    ogg->data = (unsigned char *)malloc(data_len > 0 ? (size_t)data_len : 1);
    if (!ogg->data) {
        free(ogg);
        return NULL;
    }
    memcpy(ogg->data, data, data_len);
    ogg->data_len = data_len;
    ogg->mem.data = ogg->data;
    ogg->mem.size = data_len;
    ogg->mem.pos  = 0;

    ov_callbacks cb;
    cb.read_func  = alogg_read_cb;
    cb.seek_func  = alogg_seek_cb;
    cb.close_func = alogg_close_cb;
    cb.tell_func  = alogg_tell_cb;

    if (ov_open_callbacks(&ogg->mem, &ogg->vf, NULL, 0, cb) != 0) {
        free(ogg->data);
        free(ogg);
        return NULL;
    }
    ogg->vf_initialized = 1;

    vorbis_info *vi = ov_info(&ogg->vf, -1);
    if (!vi) {
        ov_clear(&ogg->vf);
        ogg->vf_initialized = 0;
        free(ogg->data);
        free(ogg);
        return NULL;
    }
    ogg->freq   = vi->rate;
    ogg->stereo = (vi->channels > 1);
    ogg->total_samples = ov_pcm_total(&ogg->vf, -1);

    return ogg;
}

ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len)
{
    if (!data || data_len <= 0)
        return NULL;
    return alogg_alloc_and_open(data, data_len);
}

ALOGG_OGG *alogg_create_ogg_from_file(FILE *f)
{
    (void)f;
    return NULL;
}

void alogg_destroy_ogg(ALOGG_OGG *ogg)
{
    if (!ogg)
        return;
    if (ogg->stream)
        stop_audio_stream(ogg->stream);
    if (ogg->vf_initialized)
        ov_clear(&ogg->vf);
    if (ogg->data)
        free(ogg->data);
    free(ogg);
}

/* ------------------------------------------------------------------ */
/*  Playback                                                           */
/* ------------------------------------------------------------------ */

int alogg_play_ex_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed, int loop)
{
    if (!ogg || ogg->playing)
        return -1;

    if (buffer_len < 256)
        buffer_len = 256;

    AUDIOSTREAM *st = play_audio_stream(buffer_len, 16, ogg->stereo, ogg->freq, vol, pan);
    if (!st && ogg->stereo) {
        /* Some drivers are mono-only; fall back to a mono stream. */
        st = play_audio_stream(buffer_len, 16, 0, ogg->freq, vol, pan);
        ogg->stereo = 0;
    }
    if (!st)
        return -1;

    ogg->stream     = st;
    ogg->sample_len = buffer_len;
    ogg->volume     = vol;
    ogg->pan        = pan;
    ogg->speed      = speed;
    ogg->loop       = loop;
    ogg->playing    = 1;
    ogg->finished   = 0;

    ov_pcm_seek(&ogg->vf, 0);

    /* Fill the first fragment(s) so playback starts immediately. */
    alogg_poll_ogg(ogg);
    return 0;
}

int alogg_play_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan)
{
    return alogg_play_ex_ogg(ogg, buffer_len, vol, pan, 1000, 0);
}

void alogg_adjust_ogg(ALOGG_OGG *ogg, int vol, int pan, int speed, int loop)
{
    if (!ogg)
        return;
    ogg->volume = vol;
    ogg->pan    = pan;
    ogg->speed  = speed;
    ogg->loop   = loop;
    if (ogg->stream && ogg->stream->voice >= 0) {
        voice_set_volume(ogg->stream->voice, vol);
        voice_set_pan(ogg->stream->voice, pan);
    }
}

void alogg_stop_ogg(ALOGG_OGG *ogg)
{
    if (!ogg)
        return;
    if (ogg->stream) {
        stop_audio_stream(ogg->stream);
        ogg->stream = NULL;
        ogg->playing = 0;
        ogg->finished = 0;
    }
}

int alogg_poll_ogg(ALOGG_OGG *ogg)
{
    int result;

    if (!ogg || !ogg->playing || !ogg->stream)
        return ALOGG_POLL_NOTPLAYING;

    result = ALOGG_OK;

    for (;;) {
        void *buf = get_audio_stream_buffer(ogg->stream);
        if (!buf)
            break;

        int frames_needed = ogg->sample_len;      /* PCM frames per fragment */
        int frames_got = 0;
        char *out = (char *)buf;

        while (frames_got < frames_needed) {
            int bytes_out = (frames_needed - frames_got) * 2 * (ogg->stereo ? 2 : 1);
            int n = ov_read(&ogg->vf, out, bytes_out, 0, 2, 1, NULL);

            if (n > 0) {
                frames_got += n / (2 * (ogg->stereo ? 2 : 1));
                out   += n;
            }
            else if (n == 0) {
                /* End of stream. */
                if (ogg->loop) {
                    if (ov_pcm_seek(&ogg->vf, 0) != 0)
                        break;
                    continue;
                }
                /* Zero-pad the tail and report the track as finished. */
                if (bytes_out > 0)
                    memset(out, 0, (size_t)bytes_out);
                frames_got = frames_needed;
                ogg->finished = 1;
                result = ALOGG_POLL_PLAYJUSTFINISHED;
                break;
            }
            else if (n == OV_HOLE) {
                /* Corrupted frame; skip forward and keep going. */
                continue;
            }
            else {
                /* Real decoder error: end the track. */
                ogg->finished = 1;
                result = ALOGG_POLL_PLAYJUSTFINISHED;
                break;
            }
        }

        /* Allegro 4 stores its 16-bit sample data in "unsigned, 0x8000 =
         * silence" form (see load_wav_pf: signed ^ 0x8000; the mixer reads it
         * back as (u16 - 0x8000)).  ov_read gives us plain signed 16-bit PCM,
         * so convert every decoded sample before handing it to the stream. */
        {
            short *dst = (short *)buf;
            int nshorts = frames_got * (ogg->stereo ? 2 : 1);
            for (int i = 0; i < nshorts; i++)
                dst[i] = (short)(dst[i] ^ 0x8000);
        }

        free_audio_stream_buffer(ogg->stream);

        if (result != ALOGG_OK)
            break;
    }

    return result;
}

void alogg_rewind_ogg(ALOGG_OGG *ogg)
{
    if (!ogg || !ogg->vf_initialized)
        return;
    ov_pcm_seek(&ogg->vf, 0);
}

int alogg_is_playing_ogg(ALOGG_OGG *ogg)
{
    if (!ogg)
        return 0;
    return ogg->playing && !ogg->finished;
}

int alogg_is_looping_ogg(ALOGG_OGG *ogg)
{
    if (!ogg)
        return 0;
    return ogg->loop;
}

/* ------------------------------------------------------------------ */
/*  Introspection (used by the game for length checks)                 */
/* ------------------------------------------------------------------ */

int alogg_get_length_msecs_ogg(ALOGG_OGG *ogg)
{
    if (!ogg || !ogg->vf_initialized || ogg->freq <= 0)
        return 0;
    return (int)(ogg->total_samples * 1000 / ogg->freq);
}

int alogg_get_pos_msecs_ogg(ALOGG_OGG *ogg)
{
    if (!ogg || !ogg->vf_initialized || ogg->freq <= 0)
        return 0;
    ogg_int64_t pos = ov_pcm_tell(&ogg->vf);
    if (pos < 0)
        pos = 0;
    return (int)(pos * 1000 / ogg->freq);
}

int alogg_get_wave_freq_ogg(ALOGG_OGG *ogg)
{
    return ogg ? ogg->freq : 0;
}

int alogg_get_wave_is_stereo_ogg(ALOGG_OGG *ogg)
{
    return ogg ? ogg->stereo : 0;
}

int alogg_get_wave_bits_ogg(ALOGG_OGG *ogg)
{
    (void)ogg;
    return 16;
}

/* The remaining ALOGG_OGGSTREAM interface is not used by the game. */
