# RTP协议
RTP头 + 具体数据包

## RTP头
| 字段                  | 位数          | 说明
| --------------------  | ------------- | --------------------------------------------------------------------------------------------
| **版本号（V）**       | 2 bit         | 标识 RTP 协议版本，当前固定为 2。
| **填充位（P）**       | 1 bit         | 如果该位置位，RTP 包尾部包含额外的填充字节，用于对齐。
| **扩展位（X）**       | 1 bit         | 如果该位置位，固定 RTP 头部后面紧跟一个扩展头部。
| **CSRC 计数器（CC）** | 4 bit         | 指示固定头部后跟着的贡献源 CSRC 的个数（0～15 个）。
| **标记位（M）**       | 1 bit         | 标记用途由具体配置文件（Profile）定义，如视频帧结束标志等。
| **载荷类型（PT）**    | 7 bit         | 表示负载类型（编码格式），参考 RFC 3551 规定，如 H264、AAC、G711 等。
| **序列号（SN）**      | 16 bit        | 每发送一个 RTP 包递增 1，接收端用来检测丢包和排序，初值随机。
| **时间戳**            | 32 bit        | 表示该包中数据第一个字节采样时刻，同一帧的所有分片时间戳相同，用于同步和去抖动。
| **同步源标识符（SSRC）** | 32 bit      | 标识 RTP 包流的来源，保证同一 RTP 会话内唯一。
| **特约信源标识符（CSRC List）** | 0～15 × 32 bit | 列出混合器中新包贡献的所有 RTP 包的源标识符，用于指出交谈双方身份。


| PT 值 | 编码格式               | 备注                  |
| ---- | ------------------ | ------------------------- |
| 0    | PCMU (G.711 μ-law) | 8 kHz, 64 kbps            |
| 1    | Reserved           |                           |
| 2    | Reserved           |                           |
| 3    | GSM                | GSM 6.10                  |
| 4    | G723               | G.723                     |
| 5    | DVI4 8000 Hz       | IMA ADPCM                 |
| 6    | DVI4 16000 Hz      | IMA ADPCM                 |
| 7    | LPC                | Linear predictive coding  |
| 8    | PCMA (G.711 A-law) | 8 kHz, 64 kbps            |
| 9    | G722               | 7 kHz audio               |
| 10   | L16 Stereo         | 16-bit linear PCM, stereo |
| 11   | L16 Mono           | 16-bit linear PCM, mono   |
| 12   | QCELP              | Qualcomm codec            |
| 13   | CN                 | Comfort Noise             |
| 14   | MPA                | MPEG audio layer I, II    |
| 15   | G728               | 16 kbps speech codec      |
| 16   | DVI4 11025 Hz      | IMA ADPCM                 |
| 17   | DVI4 22050 Hz      | IMA ADPCM                 |
| 18   | G729               | 8 kbps speech codec       |
| 25   | CelB               | H.261 video               |
| 26   | JPEG               | JPEG video                |
| 28   | NV                 | H.263 video               |
| 31   | H261               | H.261 video               |
其余负载根据rtsp(sdp)沟通的协商定义

# 参考文章
https://datatracker.ietf.org/doc/html/rfc3551
https://blog.csdn.net/Chiang2018/article/details/122741850