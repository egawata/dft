/*
 *  ampdata.h
 *    周波数と、その周波数における音量の組を保持するオブジェクト
 */ 

#ifndef __AMPDATA_H__
#define __AMPDATA_H__

#include <glib.h>


//  Ampdata 構造体
typedef struct _ampdata {
    int     freq;
    double  amp;
} Ampdata;


/*
 *  ampdataオブジェクトを新規作成
 */
Ampdata *ampdata_new(void);

/*
 *  ampdataオブジェクトを新規作成(パラメータ指定つき)
 *
 *  引数：
 *    freq : 周波数
 *    amp  : 周波数 freq における音量
 *
 */
Ampdata *ampdata_new_with_param(int freq, double amp);

/*
 * ampdataオブジェクトを開放する
 *
 * 引数：
 *   data : 開放したいampdataオブジェクト
 */
void ampdata_free(Ampdata *data);

/*
 * ampdataオブジェクトを開放する
 *
 * ampdata_free を、GPtrArray が element_free_func で
 * 指定できるようにしたもの。
 */
void g_ampdata_free(gpointer data);


#endif


