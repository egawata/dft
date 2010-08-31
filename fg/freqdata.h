/*
 * freqdata.h
 */

#ifndef __FREQDATA_H__
#define __FREQDATA_H__

#include <glib.h>
#include "ampdata.h"


//  Freqdata 構造体
typedef struct _freqdata {
    long        sample_point;   //  開始からのサンプル数
    GPtrArray   *ampdata_ary;   //  sample_point における(周波数,音量)の組を保持する配列
    int         index_next;     //  現在の読み取り位置。
} Freqdata;


/*
 * freqdata オブジェクトを新規作成する
 */
Freqdata *freqdata_new(void); 


/*
 * freqdata オブジェクトを新規作成する(パラメータ指定あり)
 *
 * 引数：
 *   sample_point : 対象のサンプルポイント(データ開始時点からのサンプル数)
 *
 */
Freqdata *freqdata_new_with_param(long sample_point);


/*
 * freqdata に登録されている ampdata 構造体を順次取得する
 * 
 * 引数：
 *   data : Freqdata 構造体
 * 
 * 戻値：
 *   Ampdata 構造体を、先頭から順次返していく。
 *   最後に到達したあと、再度リクエストされたときは NULL を返す。
 */
Ampdata *freqdata_next_ampdata(Freqdata *data);


/*
 * 新しい周波数・音量の組を追加する
 *
 * 引数：
 *   data : 追加対象の Freqdata 構造体
 *   freq : 追加するデータの周波数
 *   amp  : 追加するデータの音量
 *
 */
void freqdata_add_ampdata(Freqdata *data, int freq, double amp);


/*
 * Freqdata 構造体を開放する
 */
void freqdata_free(Freqdata *data);


/* 
 * Freqdata 構造体を開放する
 * GPtrArray の element_free_func として利用可能なバージョン
 */
void g_freqdata_free(gpointer data);


#endif



