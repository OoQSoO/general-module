#ifndef __AVI_CODE_H__
#define __AVI_CODE_H__

/**
 * RIFF_LIST ("AVI ")
 *  ├─ HEAR_LIST ("hdrl")
 *  │   ├─ AVIH_LIST ("avih")            -> AVIMAINHEADER
 *  │   ├─ VIDEO_LIST ("strl")           -> 视频流
 *  │   │   ├─ VSTRH_LIST ("strh")       -> 视频流头信息 VSTRH_INFO
 *  │   │   └─ VSTRF_LIST ("strf")       -> 视频格式 VSTRF_INFO (BITMAPINFOHEADER)
 *  │   └─ AUDIO_LIST ("strl")           -> 音频流
 *  │       ├─ ASTRH_LIST ("strh")       -> 音频流头信息 ASTRH_INFO
 *  │       └─ ASTRF_LIST ("strf")       -> 音频格式 ASTRF_INFO (WAVEFORMAT)
 *  ├─ JUNK_CHNK                         -> 对齐块 (播放器或编辑器使用，常见2048对齐)
 *  ├─ MOVI_LIST ("movi")                -> 实际音视频帧数据
 *  └─ IDX1_CHNK ("idx1")                -> 索引表 (索引每帧的偏移、大小、标志等)
 *      ├─ IDX1_ENTRY[0]              -> 第一帧（视频或音频）
 *      ├─ IDX1_ENTRY[1]              -> 第二帧
 *      └─ ...
 * 
 * 需要进行结构体对齐，否则编译器会自动对齐导致文件头字节数对不上
 * 
 *  * fccHandler
 * | FourCC | 格式名称             | 说明                      |
 * | ------ | ---------------- | ----------------------- |
 * | `MJPG` | Motion JPEG      | 每帧是 JPEG 编码图像（你目前用的）    |
 * | `DIVX` | DivX MPEG-4      | MPEG-4 格式的一种，常见于压缩视频    |
 * | `XVID` | Xvid MPEG-4      | 开源 MPEG-4 解码器           |
 * | `H264` | H.264 / AVC      | 高压缩率，现代广泛使用             |
 * | `MP4V` | MPEG-4           | 早期 MPEG-4 视频编码          |
 * | `AVC1` | H.264            | 和 H264 类似，某些播放器更喜欢 AVC1 |
 * | `YUY2` | YUY2 raw video   | 原始 YUV 4:2:2 格式         |
 * | `I420` | YUV 4:2:0 planar | 原始 YUV 格式               |
 * | `RGB ` | RGB raw video    | 原始 RGB 数据               |
 * 
 *  * wLanguage 
 * | 语言                   | wLanguage（16进制） | 说明       |
 * | -------------------- | --------------- | -------- |
 * | English (US)         | `0x0409`        | 最常用的英语编码 |
 * | Chinese (Simplified) | `0x0804`        | 简体中文     |
 * | Japanese             | `0x0411`        | 日语       |
 * | French               | `0x040C`        | 法语       |
 * | German               | `0x0407`        | 德语       |
 * .......
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint16_t     WORD ;   // 2字节
typedef uint32_t    DWORD ;   // 4字节
typedef uint32_t   FOURCC ;   // 4字节

#define CC(s) (*((uint32_t *)(s)))

#define FOURCC_JUNK CC("JUNK")
#define FOURCC_LIST CC("LIST")
#define FOURCC_RIFF CC("RIFF")
#define FOURCC_AVI  CC("AVI ")
#define FOURCC_INFO CC("INFO")
#define FOURCC_DXDT CC("DXDT")
#define FOURCC_MJPG CC("MJPG")
#define FOURCC_HDRL CC("hdrl")
#define FOURCC_AVIH CC("avih")
#define FOURCC_STRL CC("strl")
#define FOURCC_STRH CC("strh")
#define FOURCC_STRF CC("strf")
#define FOURCC_STRD CC("strd")
#define FOURCC_STRN CC("strn")
#define FOURCC_VIDS CC("vids")
#define FOURCC_AUDS CC("auds")
#define FOURCC_ODML CC("odml")
#define FOURCC_DMLH CC("dmlh")
#define FOURCC_MOVI CC("movi")
#define FOURCC_IDX1 CC("idx1")
#define FOURCC_VPRP CC("vprp")

#define FOURCC_WAVE CC("WAVE")
#define FOURCC_FMT  CC("fmt ")
#define FOURCC_DATA CC("data")

typedef struct {
	FOURCC   fcc;
	uint32_t size;
} __attribute__((packed)) CHNK;

typedef struct {
    uint32_t fcc  ;
    uint32_t size ;      
    uint32_t type ;
} __attribute__((packed)) LIST;

typedef enum{
    AVIF_HASINDEX       = 0x00000010, // 有索引表（idx1），播放器可以快速跳转。几乎必须设置
    AVIF_MUSTUSEINDEX   = 0x00000020, // 播放器必须使用索引表，不能尝试流式扫描。较少见
    AVIF_ISINTERLEAVED  = 0x00000100, // 音频/视频数据交叉存储，提升播放性能。适合同步播放音视频
    AVIF_TRUSTCKTYPE    = 0x00000800, // 播放器可根据 chunk 类型（如 00dc）判断媒体类型
    AVIF_WASCAPTUREFILE = 0x00010000, // 文件是实时采集的（如录制），某些软件识别时用
    AVIF_COPYRIGHTED    = 0x00020000, // 标记为有版权的文件
}AVI_DWFLAGES;

// "avih"子块的内容可由如下的结构定义：
typedef struct _avimainheader 
{
    FOURCC fcc; // 'avih' fixed
    DWORD cb ; // 56 fixed
    DWORD dwMicroSecPerFrame ; // 显示每帧所需的时间us，定义avi的显示速率 帧率倒数 不可信
    DWORD dwMaxBytesPerSec; // 最大数据传输率：每秒最大字节数（大致为音频+视频每秒总大小）
    DWORD dwPaddingGranularity; // 数据对齐(2,512,2048),仅供播放器参考,数据帧按照此对齐可加快解码,起码2字节对齐
    DWORD dwFlages; // AVI文件的特殊属性，如是否包含索引块，音视频数据是否交叉存储
    DWORD dwTotalFrame; // 文件总帧数 不可信
    DWORD dwInitialFrames; // 播放器预加载帧数,音视频建议1-5,纯视频0
    DWORD dwStreams; // 文件数据流,音频 视频 字幕 1-3
    DWORD dwSuggestedBufferSize; // 最大数据帧大小,供播发器使用 通常为存储一帧图像以及同步声音所需要的数据之和
    DWORD dwWidth; // 图像宽
    DWORD dwHeight; // 图像高
    DWORD dwReserved[4]; // 保留值
} __attribute__((packed)) AVIMAINHEADER;

// 视频图像的有效区域（相对于输出窗口）
typedef struct 
{
    WORD left;
    WORD top;
    WORD right;
    WORD bottom;
} __attribute__((packed)) RECT;

/**
 * "strl" LIST块用于记录AVI数据流，每一种数据流都在该LIST块中占有3个子块，他们的ID分别是"strh","strf", "strd"；
 * "strh"子块由如下结构定义。
*/
typedef struct 
{
    FOURCC fccType;// vids 视频流 auds 音频流 txts 文本流
    FOURCC fccHandler;// 表示数据流解压缩的驱动程序代号
    DWORD dwFlags; // 数据流控制,写0即可
    WORD wPriority; // 此数据流的播放优先级
    WORD wLanguage; // 音频语言,视频写0即可,对播放没有影响只是标注
    DWORD dwInitalFrames;// 播放器预加载帧数,音视频建议1-5,纯视频0
    DWORD dwScale; // 时间单位  dwScale/dwRate = 采样率(帧速率)
    DWORD dwRate;  // 视频帧率,(采样率)
    DWORD dwStart; // 数据流开始播放的位置，以dwScale为单位
    DWORD dwLength; // 该流包含的样本数量 总数据帧
    DWORD dwSuggestedBufferSize; // 建议缓冲区的大小
    DWORD dwQuality; // 解压缩质量参数，值越大，质量越好,jpeg、pcm无用,其他编码需要考虑 0-10000
    DWORD dwSampleSize; // 0代表可变,代表每个数据帧大小
    RECT rcFrame; // 视频图像所占的矩形
} __attribute__((packed)) AVISTREAMHEADER;

// "strf"子块紧跟在"strh"子块之后，其结构视"strh"子块的类型而定，如下所述；如果 strh子块是视频数据流，则 strf子块的内容是一个与windows设备无关位图的BIMAPINFO结构，如下：

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;   // fixed 40
    DWORD biWidth;   // 图像宽度（像素）
    DWORD biHeight;  // 图像高度（像素），注意：正数表示 倒序图像（从底向上），负数表示从上向下。
    WORD biPlanes;  // 位图的平面数，必须为1
    WORD biBitCount; // 每个像素的位数，如 24 表示 RGB888 yuyv 16
    FOURCC biCompression; // 图像压缩类型，如 BI_RGB 或 'MJPG'
    DWORD biSizeImage;  // 通常等于 width * height * (bitcount / 8)，压缩图像可为估算值或0
    DWORD biXPelsPerMeter; // 水平方向分辨率（像素/米）可设为0
    DWORD biYPelsPerMeter; // 垂直方向分辨率（像素/米）可设为0
    DWORD biClrUsed;    // 实际使用的颜色表条目数（0=默认）
    DWORD biClrImportant; // 重要颜色条目数（0=所有颜色都重要）
} __attribute__((packed)) BITMAPINFOHEADER;

/**
 * 只有在图像是 8 位或更低（比如 8 位灰度图、4 位、1 位图）时，BITMAPINFO 才是必须的。
 * 可以理解为BITMAPINFO都需要这个结构体,图像是 8 位或更低（比如 8 位灰度图、4 位、1 位图）时才需要RGBQUAD颜色表。
*/
typedef struct tagBITMAPINFO
{
    BITMAPINFOHEADER bmiHeader;
    // RGBQUAD bmiColors[1]; // 颜色表
} __attribute__((packed)) BITMAPINFO;

// 如果 strh子块是音频数据流，则strf子块的内容是一个WAVEFORMAT结构，如下：
typedef struct 
{
    WORD wFormatTag; 
    WORD nChannels; //声道数
    DWORD nSamplesPerSec; //采样率
    DWORD nAvgBytesPerSec; //WAVE声音中每秒的数据量
    WORD nBlockAlign; //数据块的对齐标志
    WORD  wBitsPerSample;
    // WORD cbSize; // wFormatTag不需要额外信息,则必须将此成员设置为0.WAVE_FORMAT_PCM格式要忽略此成员(wFormatTag == 1)
} __attribute__((packed)) WAVEFORMAT;

// 索引块
typedef struct 
{
    DWORD ckid; //记录数据块中子块的标记
    DWORD dwFlags; //表示chid所指子块的属性
    DWORD dwChunkOffset; //子块的相对位置
    DWORD dwChunkLength; //子块长度
} __attribute__((packed)) ssdq;

typedef struct {
    FILE *fp;
    int width, height, fps;
    int frame_count;
    uint32_t movi_start_pos;
    uint32_t avi_header_pos;
    struct {
        uint32_t offset;
        uint32_t size;
    } *index;
    int index_cap;
} avi_writer_t;


typedef struct {
    LIST RIFF_LIST;
    LIST HEAR_LIST;
    AVIMAINHEADER AVIH_LIST;
    LIST VIDEO_LIST;
    CHNK VSTRH_LIST;
    AVISTREAMHEADER VSTRH_INFO;
    CHNK VSTRF_LIST;
    BITMAPINFOHEADER VSTRF_INFO;

    LIST AUDIO_LIST;
    CHNK ASTRH_LIST;
    AVISTREAMHEADER ASTRH_INFO;
    CHNK ASTRF_LIST;
    WAVEFORMAT ASTRF_INFO;

    // 不应该这样申请,应该直接写入1716个字节长度
    CHNK    J_JUNK_CHNK;         /*JUNK块 用于播放器对齐,一般是2048对齐*/
    char    J_PACK[1716];		

	LIST    MOVI_LIST;

} __attribute__((packed)) avi_filehead_t;


typedef struct {
    long riff_size_pos; // 文件总大小
    long riff_len;
    long total_frame_pos;
    int  total_frame;   // 视频总帧数
    long video_len_pos;
    int  video_frame;   // 视频总帧数
    long audio_len_pos;
    int  audio_frame;   // 音频总帧数
    long movi_size_pos;
    long movi_len;      // movi块总大小
} AviOffsets;


#endif