#ifndef __ACC_H__
#define __ACC_H__
/**
 * 
 * PCM数据+AAC压缩算法->AAC数据
 * 
 * AAC封装 = ADTS(可选)+原始AAC数据
 * 
 * * profile_ObjectType
 * | profile_ObjectType | MPEG-2 profile | MPEG-4 boject type |
 * | ------------------ | -------------- | ------------------ |
 * | 0x00 | Main profile                         | AAC Main   |
 * | 0x01 | Low Complexity profile (LC)          | AAC LC     |
 * | 0x10 | Scalable Sampling Rate profile (SSR) | AAC SSR    |
 * | 0x11 | (reserved)                           | AAC LTP    |
 * 
 * * sampling_frequency_index
 * | sample | value |
 * | 0x0 | 96000 | 
 * | 0x1 | 88200 |
 * | 0x2 | 64000 |
 * | 0x3 | 48000 |
 * | 0x4 | 44100 |
 * | 0x5 | 32000 |
 * | 0x6 | 24000 |
 * | 0x7 | 22050 |
 * | 0x8 | 16000 |
 * | 0x9 | 12000 |
 * | 0xa | 11025 |
 * | 0xb | 8000  |
 * | 0xc | 7350  |
 * | 0xd | reserved |
 * | 0xe | reserved |
 * | 0xf | escape value |
 * 
 */
#include <stdint.h>

//ACC 格式ADTS头
struct adts_fixed_header
{
    uint16_t syncword                   :12; // 固定为0xff,代表一个ADTS帧的开始
    uint8_t id                          :1;  // 表示MPEG版本，0代表MPEG-4, 1代表MPEG-2，一般用 0，因为都是属于 MPEG 的规范.。
    uint8_t layer                       :2;  // 一般是0
    uint8_t protection_absent           :1;  // 设置 1 表示没有CRC，整个ADST头为7字节；0 表示有CRC，整个ADST头为9字节
    uint8_t profile_ObjectType          :2;  // 该字段的解释取决于ID位的值
    uint8_t sampling_frequency_index    :4;  // 表示采样率下标
    uint8_t private_bit                 :1;  // 编码时设置为0,详细解释参见 ISO/IEC 11172-3, subclause 2.4.2.3 (Table 8)
    uint8_t channel_configuration       :3;  // 声道
    uint8_t original_copy               :1;  // 编码时设置为0
    uint8_t home                        :1;  // 编码时设置为0
}__attribute__((packed));

struct adts_variable_header
{
    uint16_t copyright_identification_bit  : 1;  // 一般为0
    uint16_t copyright_identification_start: 1;  // 一般为0
    uint16_t aac_frame_length              : 13; // 当前ADTS帧长度（包括ADTS头+AAC音频数据）
    uint16_t adts_buffer_fullness          : 11; // 0x7FF表示VBR
    uint16_t number_of_raw_data_blocks_in_frame : 2; // AAC原始帧数（通常为0表示1个raw_data_block）
} __attribute__((packed));

// 如果 protection_absent = 0，才存在
struct adts_crc
{
    uint16_t crc_check;  // 16 bits = 2 bytes，标准 CRC 值（ISO/IEC 13818-7 规范）
} __attribute__((packed));

#endif