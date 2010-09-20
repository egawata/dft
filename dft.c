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
#include <string.h>
#include <stdlib.h>
#include <math.h>

//  SAMPLE_RATE: サンプルレート
#define SAMPLE_RATE 44100

//  NUM_SAMPLE : 解析1回あたりのサンプル数
#define DELTA       1
#define NUM_SAMPLE  2205

//  解析結果として得たい最高の周波数。
#define MAX_FREQ    2000

//  波形データの最大振幅。16bitなら32768でよい。
#define MAX_SINT    32768

#define PI          3.14159

//  出力対象の音量のしきい値。
//  これを超える音量を持つ結果のみ出力される。
#define MIN_AMP     0.01

#define NUM_REDUCE  10


/*
 * サンプル数が足りない場合に、十分な長さ(num_rep_sample)のサンプルを作成する。
 *
 * 基本的には、元の波形を繰り返しつなげていき、要求されるサンプル数まで
 * 伸長する。
 * ただし、つなぎ目のところで急激な音量の変化が発生すると正しい解析が
 * 行えないため、つなぎ目部分の音量を抑えるようにする。
 *
 * sample         : 元のサンプルデータ
 * num_sample     : 元のサンプルデータのサンプル数
 * rep_sample     : 伸長したサンプルを格納する場所。領域はあらかじめ呼び出し元で確保すること。
 * num_rep_sample : 要求されるサンプル数。
 *
 */
void make_repeated_sample
(const short *sample, const size_t num_sample, short *rep_sample, size_t num_rep_sample)
{
    //  num_rep_sample : コピーが必要な残りサンプル数。 
    while (num_rep_sample > 0) {
        //  この回で実際にコピーするサンプル数
        size_t num_copied = (num_sample < num_rep_sample) ? num_sample : num_rep_sample;

        memcpy(rep_sample, sample, num_copied * sizeof(short));
        
        //  元サンプルの両側の音量を1/10にする。
        //  (サンプルの切れ目をなめらかにするため)
        int i;
        for (i = 0; i < NUM_REDUCE; i++ ) {
            double rate = i / 10;
            if (i < num_copied) 
                rep_sample[i] *= rate;
            if (num_sample - i - 1 < num_copied) 
                rep_sample[num_sample - i - 1] *= rate;
        }
        
        rep_sample  += num_copied;
        num_rep_sample -= num_copied;
    }
}


/*
 *  離散フーリエ解析を行う。
 *
 *  sample     : 解析対象のサンプルデータ
 *  num_sample : サンプルの数
 *  result     : 解析結果の格納先。
 *               double 型の配列であり、1Hz～max_freq Hzまでの解析結果を
 *               各要素に格納していく。
 *  max_freq   : 解析を行う上限の周波数
 *  delta      : 解析を行う周波数の粒度。
 *               例えば、max_freq = 2000, delta = 5 のとき、
 *               解析は 5Hz, 10Hz, ... , 1995Hz, 2000Hz となり,
 *               結果は result[0], result[1], ... result[399] に格納される。
 *
 */
void dft(short *sample, size_t num_sample, double *result, double max_freq, double delta) 
{
    double span_per_sample = 2 * PI / num_sample;   // 全体の長さを2πとしたときの、1サンプルあたりの横軸方向の長さ
    int w, t;

    //  もし十分なサンプル長が存在しない場合は、波形を繰り返して
    //  1秒分のサンプルを作る
    size_t num_rep_sample = SAMPLE_RATE;             
    short *rep_sample = malloc(sizeof(short) * num_rep_sample);
    
    make_repeated_sample(sample, num_sample, rep_sample, num_rep_sample); 

    for (w = 1; w <= max_freq / delta; w++) {
        double real = 0.0, imag = 0.0;

        for (t = 0; t < num_rep_sample; t++) {
            double theta = w * span_per_sample * t;
            real += rep_sample[t] * cos(theta);
            imag += rep_sample[t] * sin(theta);
        }
        real *= 2 * PI / num_rep_sample / MAX_SINT;
        imag *= 2 * PI / num_rep_sample / MAX_SINT;
        
        result[w - 1] = sqrt(real * real + imag * imag);
        printf("%d: %f\n", w-1, result[w-1]);
    }

    free(rep_sample);
}



int main(int argc, char *argv[])
{
    short buf[NUM_SAMPLE];
    size_t max_size = -1;

    if (!argv[1]) {
        printf("Usage: dft [filename]\n");
        return 1;
    }
    if (argv[2]) {
        max_size = atoi(argv[2]);
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
    while ((size = read_data(wav, buf, NUM_SAMPLE)) > 0) {  // size はサンプル数(not bytes)
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

        if (max_size != -1 && current_ptr > max_size) 
            break;
    }

    close_wavfile(wav);

    return 0;
}



