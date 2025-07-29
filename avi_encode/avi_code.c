/**
 * @file avi_code.c
 * @author ZXX
 * @brief 
 * @version 0.1
 * @date 2025-07-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "avi_code.h"
#include <stddef.h>

#define NAX_AVIFPS  25
#define NAX_AVITIME 60

static void write_chunk_id(FILE *fp, const char *id) {
    fwrite(id, 1, 4, fp);
}

static void write_u32(FILE *fp, uint32_t v) {
    fwrite(&v, 4, 1, fp);
}

static void write_u16(FILE *fp, uint16_t v) {
    fwrite(&v, 2, 1, fp);
}

static void write_avi_header() {

    FILE *fp = NULL;

    uint32_t ad = sizeof(AVIMAINHEADER) // avih
                + sizeof(LIST) + sizeof(CHNK) + sizeof(AVISTREAMHEADER) + sizeof(CHNK) + sizeof(BITMAPINFOHEADER)   // 视频
                + sizeof(LIST) + sizeof(CHNK) + sizeof(AVISTREAMHEADER) + sizeof(CHNK) + sizeof(WAVEFORMAT);        // 音频 
    avi_filehead_t avi_head = {

        .RIFF_LIST = {
            .fcc = FOURCC_RIFF,
            .size = 0xffffffff,    // 文件总大小-8,待定
            .type = FOURCC_AVI,
        },

        .HEAR_LIST = {
            .fcc = FOURCC_LIST,
            .size = ad, // hdrl 块大小总和
            .type = FOURCC_HDRL,
        },

        .AVIH_LIST = {
            .fcc = FOURCC_AVIH,
            .cb = 56,
            .dwMicroSecPerFrame = 1000000/25,   // 1000000 / fps
            .dwMaxBytesPerSec = 25*25*1024,     // 最大数据传输率：每秒最大字节数（大致为音频+视频每秒总大小）
            .dwPaddingGranularity = 2,
            .dwFlages = AVIF_HASINDEX | AVIF_ISINTERLEAVED,
            .dwTotalFrame = 0xffffffff,              // 视频总帧数 待定
            .dwInitialFrames = 0,
            .dwStreams = 2,
            .dwSuggestedBufferSize = 25*1024,
            .dwWidth = 640,
            .dwHeight = 480,
            .dwReserved = {0x00, 0x00, 0x00, 0x00},
        },

        .VIDEO_LIST = {
            .fcc = FOURCC_LIST,
            .size = 116,
            .type = FOURCC_STRL,
        },

        .VSTRH_LIST = {
            .fcc = FOURCC_STRH,
            .size = 56,
        },

        .VSTRH_INFO = {
            .fccType = FOURCC_VIDS,
            .fccHandler = FOURCC_MJPG,
            .dwFlags = 0,
            .wPriority = 0,
            .wLanguage = 0,
            .dwInitalFrames = 0,
            .dwScale = 1,
            .dwRate = 25,   // fps
            .dwStart = 0,
            .dwLength = 0xffffffff,    // 待定
            .dwSuggestedBufferSize = 25*1024,
            .dwQuality = 0,
            .dwSampleSize = 0,
            .rcFrame = {
                .left = 0,
                .top = 0,
                .right = 640,
                .bottom = 480,
            },
        },     

        .VSTRF_LIST = {
            .fcc = FOURCC_STRF,
            .size = 40,
        },

        .VSTRF_INFO = {
            .biSize = 40,
            .biWidth = 640,
            .biHeight = 480,
            .biPlanes = 1,
            .biBitCount = 16,
            .biCompression = FOURCC_MJPG,
            .biSizeImage = 25*1024,
            .biXPelsPerMeter = 0,
            .biYPelsPerMeter = 0,
            .biClrUsed = 0,
            .biClrImportant = 0,
        },

        .AUDIO_LIST = {
            .fcc = FOURCC_LIST,
            .size = 92,
            .type = FOURCC_STRL,
        },

        .ASTRH_LIST = {
            .fcc = FOURCC_STRH,
            .size = 56,
        },

        .ASTRH_INFO = {
            .fccType = FOURCC_AUDS,
            .fccHandler = 0,
            .dwFlags = 0,
            .wPriority = 0,
            .wLanguage = 0,
            .dwInitalFrames = 0,
            .dwScale = 320,                   // 每个音频块表示 320 采样点 1
            .dwRate = 8000,                 // 每秒 8000 个采样点（即音频速率）16000
            .dwStart = 0,
            .dwLength = 0xffffffff,          // 待定
            .dwSuggestedBufferSize = 16000, // 推荐缓存块大小 640
            .dwQuality = 0,
            .dwSampleSize = 0,              
            .rcFrame = {
                .left = 0,
                .top = 0,
                .right = 0,
                .bottom = 0,
            },
        },

        .ASTRF_LIST = {
            .fcc = FOURCC_STRF,
            .size = 16,
        },

        .ASTRF_INFO = {
            .wFormatTag = 1,
            .nChannels = 1,
            .nSamplesPerSec = 8000,
            .nAvgBytesPerSec = 16000,
            .nBlockAlign = 2,
            .wBitsPerSample = 16,
        },
        
        .J_JUNK_CHNK = {
            .fcc = FOURCC_JUNK,
            .size = 1716,
        },
        // J_PACK

        .MOVI_LIST = {
            .fcc = FOURCC_LIST,
            .size = 0xffffffff,   // movi块大小总和未知 待定
            .type = FOURCC_MOVI,
        },
    };
    avi_head.RIFF_LIST.fcc = FOURCC_AUDS;
    // fopen()
    // fwrite(&avi_head,sizeof(avi_head), 1, fp);

}

int avi_writer_init(avi_writer_t *ctx, const char *filename, int w, int h, int fps) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->width = w;
    ctx->height = h;
    ctx->fps = fps;
    ctx->index_cap = 1000;
    ctx->index = calloc(ctx->index_cap, sizeof(*ctx->index));
    ctx->fp = fopen(filename, "wb");
    if (!ctx->fp) return -1;
    write_avi_header();
    return 0;
}

int avi_writer_add_frame(avi_writer_t *ctx, const uint8_t *data, int len) {
    if (ctx->frame_count >= ctx->index_cap) return -1;
    write_chunk_id(ctx->fp, "00dc");
    write_u32(ctx->fp, len);
    uint32_t offset = ftell(ctx->fp);
    fwrite(data, 1, len, ctx->fp);
    if (len & 1) fputc(0, ctx->fp);

    ctx->index[ctx->frame_count].offset = offset - ctx->movi_start_pos - 4;
    ctx->index[ctx->frame_count].size = len;
    ctx->frame_count++;
    return 0;
}

int avi_writer_close(avi_writer_t *ctx) {
    uint32_t movi_end = ftell(ctx->fp);

    // Update movi LIST size
    fseek(ctx->fp, ctx->movi_start_pos - 4, SEEK_SET);
    write_u32(ctx->fp, movi_end - ctx->movi_start_pos);
    fseek(ctx->fp, movi_end, SEEK_SET);

    // Write idx1
    write_chunk_id(ctx->fp, "idx1");
    write_u32(ctx->fp, ctx->frame_count * 16);
    for (int i = 0; i < ctx->frame_count; ++i) {
        fwrite("00dc", 1, 4, ctx->fp);
        write_u32(ctx->fp, 0x10); // flags: keyframe
        write_u32(ctx->fp, ctx->index[i].offset);
        write_u32(ctx->fp, ctx->index[i].size);
    }

    // Update RIFF size
    uint32_t file_end = ftell(ctx->fp);
    fseek(ctx->fp, ctx->avi_header_pos, SEEK_SET);
    write_u32(ctx->fp, file_end - ctx->avi_header_pos + 4);

    fclose(ctx->fp);
    free(ctx->index);
    return 0;
}

int avi_write_init(const char *filename)
{

}

void record_avi_offsets(long avi_head_pos, AviOffsets *offsets)
{
    offsets->riff_size_pos   = avi_head_pos + offsetof(avi_filehead_t, RIFF_LIST) + offsetof(LIST, size);
    offsets->total_frame_pos = avi_head_pos + offsetof(avi_filehead_t, AVIH_LIST) + offsetof(AVIMAINHEADER, dwTotalFrame);
    offsets->video_len_pos   = avi_head_pos + offsetof(avi_filehead_t, VSTRH_INFO) + offsetof(AVISTREAMHEADER, dwLength);
    offsets->audio_len_pos   = avi_head_pos + offsetof(avi_filehead_t, ASTRH_INFO) + offsetof(AVISTREAMHEADER, dwLength);
    offsets->movi_size_pos   = avi_head_pos + offsetof(avi_filehead_t, MOVI_LIST) + offsetof(LIST, size);

    // N_Deb("%ld\n",offsets->riff_size_pos);
    // N_Deb("%ld\n",offsets->total_frame_pos);
    // N_Deb("%ld\n",offsets->video_len_pos);
    // N_Deb("%ld\n",offsets->audio_len_pos);
    // N_Deb("%ld\n",offsets->movi_size_pos);
    // 初始化 frame 和 len 数值
    offsets->riff_len     = 0;
    offsets->total_frame  = 0;
    offsets->video_frame  = 0;
    offsets->audio_frame  = 0;
    offsets->movi_len     = 0;
}

