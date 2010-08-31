/*
 * freqdata.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "freqdata.h"


/*
 * freqdata オブジェクトを新規作成する
 */
Freqdata *freqdata_new(void) {
    Freqdata *data = malloc(sizeof(Freqdata));
    if (!data) {
        perror("Failed to allocate memory for freqdata");
        exit(EXIT_FAILURE);
    }
    data->ampdata_ary = g_ptr_array_new_with_free_func(g_ampdata_free);
    data->index_next = 0;
    return data;
}


/*
 * freqdata オブジェクトを新規作成する(パラメータ指定あり)
 *
 * 引数：
 *   sample_point : 対象のサンプルポイント(データ開始時点からのサンプル数)
 *
 */
Freqdata *freqdata_new_with_param(long sample_point) {
    Freqdata *data = freqdata_new();
    data->sample_point = sample_point;
    return data;
}  


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
Ampdata *freqdata_next_ampdata(Freqdata *data)
{
    if (data->index_next > data->ampdata_ary->len) {
        return (Ampdata *)g_ptr_array_index(data->ampdata_ary, data->index_next++);
    } else {
        return NULL;
    }
}  


/*
 * 新しい周波数・音量の組を追加する
 *
 * 引数：
 *   data : 追加対象の Freqdata 構造体
 *   freq : 追加するデータの周波数
 *   amp  : 追加するデータの音量
 *
 */
void freqdata_add_ampdata(Freqdata *data, int freq, double amp)
{
    Ampdata *ampdata = ampdata_new_with_param(freq, amp);
    g_ptr_array_add(data->ampdata_ary, (gpointer)ampdata);
}  


/*
 * Freqdata 構造体を開放する
 */
void freqdata_free(Freqdata *data)
{
    g_ptr_array_free((gpointer)data->ampdata_ary, TRUE);
    free(data);
}


/* 
 * Freqdata 構造体を開放する
 * GPtrArray の element_free_func として利用可能なバージョン
 */
void g_freqdata_free(gpointer data)
{
    freqdata_free((Freqdata *)data);
}  





