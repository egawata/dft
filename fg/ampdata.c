/*
 *  ampdata.c
 */ 

#include <stdlib.h>
#include <stdio.h>
#include "ampdata.h"


/*
 *  ampdataオブジェクトを新規作成
 */
Ampdata *ampdata_new(void) {
    Ampdata *data = malloc(sizeof(Ampdata));
    if (!data) {
        perror("Failed to allocate memory for ampdata");
        exit(EXIT_FAILURE);
    }
    return data;
}  


/*
 *  ampdataオブジェクトを新規作成(パラメータ指定つき)
 *
 *  引数：
 *    freq : 周波数
 *    amp  : 周波数 freq における音量
 *
 */
Ampdata *ampdata_new_with_param(int freq, double amp)
{
    Ampdata *data = ampdata_new();
    data->freq = freq;
    data->amp  = amp;
    return data;
}  

/*
 * ampdataオブジェクトを開放する
 *
 * 引数：
 *   data : 開放したいampdataオブジェクト
 */
void ampdata_free(Ampdata *data)
{
    free(data);
}  


/*
 * ampdataオブジェクトを開放する
 *
 * ampdata_free を、GPtrArray が element_free_func で
 * 指定できるようにしたもの。
 */
void g_ampdata_free(gpointer data)
{
    ampdata_free((Ampdata *)data);
}



