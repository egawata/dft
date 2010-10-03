/*
 *  fft.c
 *
 *  高速フーリエ変換を実行する
 *
 */

#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>

#define     PI      3.14159
#define     PI2     (PI * 2)

/*
 *  ビット反転させる。
 *  ここで言う「ビット反転」とは、ある数 n を
 *  桁数 log2n の2進数で表記し、その0と1の並びを
 *  上位と下位で反転させる、という意味。
 *
 *  例えば n = 6, log2n = 3 のとき、
 *
 *    n = 6 を、3桁(=log2n)の2進数で表記すると、110 
 *    これの上位と下位を反転させると、011 となり、これが戻り値となる。
 *
 *  高速フーリエ変換のバタフライ演算において、
 *  結果の並び順が入れ替わるため、求めたい結果の正しい要素番号を得るために
 *  用いる。
 *
 */
int bit_reverse(int n, int log2n)
{
    int result = 0;
    int j;

    for ( j=0; j < log2n ; j++ ) {
        int tmp = n & 1;
        result <<= 1;
        result |= tmp;
        n >>= 1;
    }
    
    return result;
}
    

/* 
 *  高速フーリエ変換を行う。
 *
 *  実際にこの fft 関数が行うのは、1段階のFFTのみで、
 *  多段で実行するために、再帰呼び出しが行われている。
 *  TODO: 再帰を使わないほうが効率良さそうな気がするので、改良の余地あり。
 *
 *  n     : 求めたい周波数成分( 0 <= n < 2^log2n )
 *  depth : 段数。再帰呼び出しの途中で fft()関数の内部から呼び出されるのでない限り、
 *          つまりfft() 関数の外部から呼び出される限りは、0 を指定。
 *  log2n : 入力データの数を、2の対数表記したもの。
 *          入力データ数が64個であれば、log2n = 6 となる。(2^6 = 64)
 *  data  : 入力データを格納する配列の先頭アドレス。
 *          この配列には 2 ^ log2n 個のデータが格納されていなければならない。
 *
 *  戻り値
 *    演算結果が complex 型で返される。 
 *
 */
complex _fft(int n, int depth, int log2n, double *data) 
{
    if (depth == log2n) {
        return *(data + n);
    }
    else {
        int m = 1 << depth;     //  2 の depth 乗
        int m2 = m << 1;

        if (n & m) {
            complex W = cexp(- PI2 * I * (n & (m-1)) / m2);
            return W * ( 
                      _fft(n - m, depth+1, log2n, data) 
                    - _fft(n    , depth+1, log2n, data)
            ) ;
        } else {
            return _fft(n    , depth+1, log2n, data) 
                 + _fft(n + m, depth+1, log2n, data);
        }
    }
}


complex fft(int n, int log2n, double *data) 
{
    if ( n < 0 ||  n >= (1 << (log2n) ) ) {
        fprintf(stderr, "_fft() : index no. [%d] is invalid.", n);
        exit(1);
    }

    n = bit_reverse(n, log2n);

    return _fft(n, 0, log2n, data);
}


#ifdef FFT_SAMPLE
/*
 * 以下はサンプルプログラム。
 * いくつかの周波数成分を混合させた波形データを生成し、
 * それをfft()関数で高速フーリエ解析している。
 *
 * サンプルプログラムを実行するようにコンパイルするには
 * -DFFT_SAMPLE オプションをコンパイル時引数に渡す。
 *
 */

#define DATASIZE_LOG2   6

int main(int argc, char *argv[]) 
{
    const int datasize = 1 << DATASIZE_LOG2;
    double *wavdata = malloc(sizeof(double) * datasize);
    int i;

    for (i=0; i < datasize; i++) {
        wavdata[i] =   sin(PI2 * i * 3 / datasize) * 0.5
                     + sin(PI2 * i * 2 / datasize)
                     + cos(PI2 * i * 6 / datasize) * 0.4
                     + sin(PI2 * i * 6 / datasize) * 0.2
                     ;
        printf("Wavdata[%d] : %f\n", i, wavdata[i]);
    }

    for (i=0; i < datasize / 2; i++) {
        complex result = fft(i, DATASIZE_LOG2, wavdata);
        double real = creal(result);
        double imag = cimag(result);
        double size = sqrt(real * real + imag * imag);
        printf("F[%d] : %f\n", i, size );
    }

    free(wavdata);
    return 0;
}

#endif  // FFT_SAMPLE




