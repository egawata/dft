/*
 * freqdatalist.h
 *   Freqdata 構造体の配列を扱う。
 */

#ifndef __FREQDATALIST_H__
#define __FREQDATALIST_H__

#include <glib.h>

//  FreqdataList 構造体
typedef struct _freqdata_list {
    long        num_sample;     //  このリストに含まれるFreqdata構造体の数
    int         interval;       //  あるサンプルから次のサンプルまでの間隔(サンプル数)
    GPtrArray   *list;          //  Freqdata 構造体のポインタを保持する配列
} FreqdataList;


#endif

