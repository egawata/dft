#ifndef __WAVFILE_H__
#define __WAVFILE_H__

#include <stdio.h>

typedef struct {
    long            fmtChunkSize;
    short           wFormatTag;
    unsigned short  wChannels;
    unsigned long   dwSamplesPerSec;
    unsigned long   dwAvgBytesPerSec;
    unsigned short  wBlockAlign;
    unsigned short  wBitsPerSample;
    long            dataChunkSize;
    FILE            *fp;
} WavData;


//  wavファイルをオープンする。
WavData* open_wavfile(const char *filename);

//  wavファイルをクローズし、使用を終了する。
//  オープンしたwavfileは必ずクローズして、使用していたメモリを開放すること。
void close_wavfile(WavData *wav); 

//  wavファイルから、sizeで指定されたサイズ分のデータを読み込む。
//  コール後、size には、実際に読み込んだサイズが入る。
size_t read_data(WavData *wav, void *buf, size_t size);


#endif

