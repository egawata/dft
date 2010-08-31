/*
 * graphview.h
 *   グラフ表示を行うビューに関するデータおよびメソッドを提供する
 */

#ifndef __GRAPHVIEW_H__
#define __GRAPHVIEW_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>

#include "freqdatalist.h"


typedef struct _graphview {
    FreqdataList    *samples;                   // 　サンプルデータ全体
    gdouble         screen_top_hzlog2;          //  画面上端の周波数(2の対数)
    glong           screen_left_samplepoint;    //  画面左端のサンプルポイント
    gdouble         zoom_x;                     //   x軸ズームレベル(pixel/sample)
    gdouble         zoom_y;                     //   y軸ズームレベル(pixel/octave)
    GtkWidget       *swin;                      //   スクロールウィンドウ
    GtkWidget       *graph;                     //   グラフを表示するエリア
} GraphView;


/*
 * Graphviewオブジェクトを新規作成
 *
 * 引数：
 *   window : このビューを表示する大本のウィンドウ
 *
 */
GraphView *graphview_new(GtkWidget *window);

/*
 * サンプルデータをセットする
 *
 * 引数：
 *   gv     : セットする対象の GraphView オブジェクト
 *   sample : セットするサンプルデータ
 *
 * 注意：
 *   FreqdataList には、生成済みのリストへのポインタを指定する。
 *   ここで指定したものは、GraphView オブジェクトを開放するときには
 *   自動的に開放されない。したがって sample については
 *   呼び出し元で適切に開放するか、開放時に 
 *   graphview_free_with_samples() を使用すること。
 *
 */ 
void graphview_set_sample_data(GraphView *gv, FreqdataList *sample);
    

/*
 * Graphview オブジェクトを開放する
 */
void graphview_free(GraphView *gv);


/*
 * GraphView オブジェクトを開放すると同時に、
 * 保持している sample も開放する。
 */
void graphview_free_with_samples(GraphView *gv);


#endif


