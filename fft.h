/*
 *  fft.h
 *
 *  高速フーリエ変換を実行する
 *
 */

#ifndef __FFT_H__
#define __FFT_H__

/* 
 *  高速フーリエ変換を行う。
 *
 *  実際にこの fft 関数が行うのは、1段階のFFTのみで、
 *  多段で実行するために、再帰呼び出しが行われている。
 *  TODO: 再帰を使わないほうが効率良さそうな気がするので、改良の余地あり。
 *
 *  n     : 求めたい周波数成分( 0 <= n < 2^log2n )
 *  log2n : 入力データの数を、2の対数表記したもの。
 *          入力データ数が64個であれば、log2n = 6 となる。(2^6 = 64)
 *  data  : 入力データを格納する配列の先頭アドレス。
 *          この配列には 2 ^ log2n 個のデータが格納されていなければならない。
 *
 *  戻り値
 *    演算結果が complex 型で返される。 
 *
 */
complex fft(int n, int log2n, double *data);


#endif  //  __FFT_H__



