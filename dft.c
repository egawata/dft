/*
 * dft.c
 * wavファイルに対してフーリエ解析を行い、結果を出力する
 *
 * wavファイルのフォーマットは、モノラル・16bitのみ。
 *
 * 結果は標準出力に出力される。
 * 
 * 1行目は全体のサンプル数と、解析を行う各サンプルのサンプル間隔。
 * (カンマ区切り)
 * 
 * 2行目以降は、解析対象のサンプル位置と、解析結果。
 * まずサンプル位置を1行で出力。
 * その後、周波数 - 音量の組を、スペース区切りで出力。
 * (出力は、しきい値(MIN_AMP)を超えるもののみ行われる)
 * すべての組を出力し終わったら、空行で終了。
 * 
 * これを、サンプルの末尾まで繰り返す。
 *
 * [使用例]
 *   dft test.wav
 *
 */   

#include "wavfile.h"
#include <stdio.h>
#include <math.h>

//  NUM_SAMPLE : 解析1回あたりのサンプル数
#define DELTA       5
#define NUM_SAMPLE  44100/DELTA

//  解析結果として得たい最高の周波数。
#define MAX_FREQ    2000

//  波形データの最大振幅。16bitなら32768でよい。
#define MAX_SINT    32768

#define PI          3.14159

//  出力対象の音量のしきい値。
//  これを超える音量を持つ結果のみ出力される。
#define MIN_AMP     0.01


void dft(short *sample, size_t num_sample, double *result, double max_freq, double delta) 
{
    double s = 2 * PI / num_sample;
    int w, t;

    for (w = 1; w <= max_freq / delta; w++) {
        double real = 0.0, imag = 0.0;

        for (t = 0; t < num_sample; t++) {
            double theta = w * s * t;
            real += sample[t] * cos(theta);
            imag += sample[t] * sin(theta);
        }
        real *= 2 * PI / num_sample / MAX_SINT;
        imag *= 2 * PI / num_sample / MAX_SINT;
        
        result[w - 1] = sqrt(real * real + imag * imag);
    }
}



int main(int argc, char *argv[])
{
    short buf[NUM_SAMPLE];

    if (!argv[1]) {
        printf("Usage: dft [filename]\n");
        return 1;
    }

    //  wavファイル読み込み
    WavData *wav = open_wavfile(argv[1]);

    //  データ全長(1サンプルは2bytes)
    printf("%ld\n", wav->dataChunkSize / 2); 
    
    //  サンプル間隔
    printf("%d\n", NUM_SAMPLE);

    //  解析 -> 結果出力
    long current_ptr = 0;
    size_t size;
    double result[MAX_FREQ / DELTA];
    while ((size = read_data(wav, buf, NUM_SAMPLE)) > 0) {
        //  サンプル位置
        printf("#%ld\n", current_ptr);

        //  フーリエ解析
        dft(buf, size, result, MAX_FREQ, DELTA);
        
        //  周波数＋音量 出力
        int r;
        for (r = 0; r < MAX_FREQ / DELTA; r++) {
            if (result[r] > MIN_AMP)
                printf("%d %f\n", (r+1) * DELTA, result[r]);
        }

        //  空行で終了
        printf("\n");

        current_ptr += size;
    }

    close_wavfile(wav);

    return 0;
}



