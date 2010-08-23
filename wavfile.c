/*
 *      wavfile.c
 *      WAVファイルを扱う
 *
 */


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "wavfile.h"



#define WRITE_SIZE 1024 
#define FormatID "fmt "
#define DataID "data"  /* chunk ID for data Chunk */ 


//  fread のラッパ関数
//  エラー処理を加えただけのもの。
//  引数は fread() と同一。
static size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t read_size;

    read_size = fread(ptr, size, nmemb, stream);
    if ( ferror(stream) ) {
        perror("Error while reading\n");
        exit(EXIT_FAILURE);
    }

    return read_size;
}


//  size バイトのデータをファイル stream から読み込み、
//  その内容が指定された文字列 expected と同一かどうかを調べる。
//  同一でない場合はエラー。
//  またこの関数は、ファイルから読み込んだ文字列を呼び出し元に返さず、
//  ファイルポインタのみ前に進める。
//
//  制限として、sizeは16バイト未満である必要がある。
static void read_cmp(size_t size, FILE *stream, const char *expected) 
{
    char tmp[16];

    assert(size < 16);
    Fread(tmp, size, 1, stream);
    if (strncmp(tmp, expected, size) != 0) {
        fprintf(stderr, "Format error: %s\n", expected);
        exit(EXIT_FAILURE);
    }
}


//  WAVファイルを読み込み、wav構造体にデータを格納していく。
//  ここで、wavデータ本体を格納するためのメモリも同時に確保される。
//  この領域は、呼び出し元で適切に開放される必要がある。
static void read_wav(const char *filename, WavData *wav)
{
    char tmp[16];

    if ((wav->fp = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "Failed to open file %s: %d\n", filename, errno);
        exit(EXIT_FAILURE);
    }
    assert(wav->fp != NULL);

    //  RIFF header
    read_cmp(4, wav->fp, "RIFF");

    //  file size after that
    Fread(tmp, 4, 1, wav->fp);

    //  WAVE header
    read_cmp(4, wav->fp, "WAVE");

    //  Start of fmt chunk
    read_cmp(4, wav->fp, FormatID);

    //  sizeof wav chunk(16 bits when linear PCM)
    Fread(&wav->fmtChunkSize, 4, 1, wav->fp);

    //  Format ID
    Fread(&wav->wFormatTag, 2, 1, wav->fp);

    //  channels
    Fread(&wav->wChannels, 2, 1, wav->fp);
   
    //  Sampling rate
    Fread(&wav->dwSamplesPerSec, 4, 1, wav->fp);
    
    //  AvgBytes/sec
    Fread(&wav->dwAvgBytesPerSec, 4, 1, wav->fp);
    
    //  Block Size
    Fread(&wav->wBlockAlign, 2, 1, wav->fp);

    //  bit/sample
    Fread(&wav->wBitsPerSample, 2, 1, wav->fp);

    //  Start of Data chunk
    read_cmp(4, wav->fp, DataID);

    //  data bytes
    Fread(&wav->dataChunkSize, 4, 1, wav->fp);

}


WavData *open_wavfile(const char *filename) {
    WavData *wav = malloc(sizeof(WavData));
    read_wav(filename, wav);

    return wav;
}


void close_wavfile(WavData *wav) {
    if (wav && wav->fp) {
        fclose(wav->fp);
    }

    if (wav) {
        free(wav);
    }
}


/*
 * wavファイルからデータを size で指定した個数だけ読み込む。
 * 1サンプルが2バイトの場合、実際に読み込まれるのは size * 2 バイト。
 * 戻り値として、実際に読み込んだサンプル数を返す。
 */
size_t read_data(WavData *wav, void *buf, size_t size) {
    size_t num_read = Fread(buf, 1, 2 * size, wav->fp);

    //  num_read はバイト数。戻り値は読み込んだサンプル数にする。
    return num_read / 2;
}

