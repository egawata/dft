/*
 * graphview.c
 *   グラフ表示を行うビューに関するデータおよびメソッドを提供する
 */


#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "graphview.h"
#include "freqdata.h"
#include "ampdata.h"

#define log2(x)     log(x)/log(2)

#define SCREEN_TOP_HZLOG2   11.0
#define SCREEN_LEFT_SAMPLEPOINT    0
#define ZOOM_X              0.0025
#define ZOOM_Y              200

#define HZ_TOP_INIT         600
#define HZ_LOG2_TOP_INIT    log2(HZ_TOP_INIT)

#define H_STEP_INCREMENT    100
#define H_PAGE_INCREMENT    300
#define H_PAGE_SIZE         600
#define H_INIT_VALUE        0

#define V_STEP_INCREMENT    -0.1
#define V_PAGE_INCREMENT    -0.5
#define V_PAGE_SIZE         log2(HZ_TOP_INIT)-log2(HZ_TOP_INIT-400)
#define V_INIT_VALUE        HZ_LOG2_TOP_INIT



static void _set_gc_color(GdkGC *gc, guint16 red, guint16 green, guint16 blue)
{
    GdkColor    color;

    color.red   = red;
    color.green = green;
    color.blue  = blue;

    gdk_gc_set_rgb_fg_color(gc, &color);
}



static void _horizontal_changed(GtkAdjustment *horizontal, GraphView *gv)
{
    gint width, height;
    gdk_drawable_get_size(gv->graph->window, &width, &height);

    gv->screen_left_samplepoint = gtk_adjustment_get_value(horizontal);
    gtk_widget_queue_draw_area(gv->graph, 0, 0, width, height);
}  


static void _vertical_changed(GtkAdjustment *vertical, GraphView *gv)
{
    gint width, height;
    gdk_drawable_get_size(gv->graph->window, &width, &height);

    gv->screen_top_hzlog2 = gtk_adjustment_get_value(vertical);
    gtk_widget_queue_draw_area(gv->graph, 0, 0, width, height);
}



//  水平スクロールバーの初期設定
void _init_hadjustment(GraphView *gv)
{
    GtkAdjustment *horizontal 
        = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(gv->swin));
    
    gtk_adjustment_set_lower(horizontal, 0);
    //gtk_adjustment_set_upper();   上限は設定しない
    gtk_adjustment_set_step_increment(horizontal, H_STEP_INCREMENT);
    gtk_adjustment_set_page_increment(horizontal, H_PAGE_INCREMENT);
    gtk_adjustment_set_page_size(horizontal, H_PAGE_SIZE);
    gtk_adjustment_set_value(horizontal, H_INIT_VALUE);
    g_signal_connect(G_OBJECT(horizontal), "value-changed",
                     G_CALLBACK(_horizontal_changed), (gpointer)gv);
}  

//  垂直スクロールバーの初期設定
void _init_vadjustment(GraphView *gv)
{
    GtkAdjustment *vertical 
        = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(gv->swin));
    
    gtk_adjustment_set_lower(vertical, 0);
    //gtk_adjustment_set_upper();   上限は設定しない
    gtk_adjustment_set_step_increment(vertical, V_STEP_INCREMENT);
    gtk_adjustment_set_page_increment(vertical, V_PAGE_INCREMENT);
    gtk_adjustment_set_page_size(vertical, V_PAGE_SIZE);
    gtk_adjustment_set_value(vertical, V_INIT_VALUE);
    g_signal_connect(G_OBJECT(vertical), "value-changed",
                     G_CALLBACK(_vertical_changed), (gpointer)gv);
}  


gint _get_y_from_hz(GraphView *gv, gdouble hz) 
{
    return 
        (gv->screen_top_hzlog2 - log2(hz)) * gv->zoom_y ;
}

/*
 *  音程を表すガイド線を引く
 *
 *    C の位置に濃い横線、
 *    D, E, F, G, A, B の位置に薄い横線を引く。
 *
 *    各音程の周波数は A=55 * octave Hz を基準にする。
 *
 *    hz_high_limit : 画面上端の周波数
 *    zoom : 拡大率。1オクターブを画面上のpix数で表したもの
 */ 
static void _draw_tone_lines(GraphView *gv) 
{
    GtkWidget *graph = gv->graph;
    GdkGC *gc = gdk_gc_new(graph->window);

    //  表示領域の水平,垂直座標の範囲
    gint screen_left   =   0;
    gint screen_top    =   0;
    gint screen_right;
    gint screen_bottom;

    gdk_drawable_get_size(graph->window, &screen_right, &screen_bottom);

    //  画面に表示される周波数の上限
    int hz_high_limit = pow(2.0, gv->screen_top_hzlog2);

    //  画面に表示される周波数の下限
    //  １より小さい場合は、1を下限とする。
    double hzlog2_low_limit = gv->screen_top_hzlog2 - (screen_bottom - screen_top) / gv->zoom_y;
    if (hzlog2_low_limit < 1.0) 
        hzlog2_low_limit = 1.0;
    
    int hz_low_limit  = pow(2.0, hzlog2_low_limit) ;

    const int HZ_A1 = 55;   //  一番低い A の音
    const double HALFTONE = 1.0594630943593;   //  半音上の音との周波数比率

    //  表示領域の下限より下の、もっとも下限に近い A の周波数を求める
    double hz_a = HZ_A1;
    while (hz_a * 2 < hz_low_limit)
        hz_a *= 2;

    int guide_tone[] = { 0, 2, 3, 5, 7, 8, 10 }; 
    while (hz_a <= hz_high_limit) {
        int t;
        for (t = 0; t < 7; t++) {
            //  C なら濃い色、それ以外は薄い線
            if (t == 2)
                _set_gc_color(gc, 40000, 40000, 40000);
            else 
                _set_gc_color(gc, 55000, 55000, 55000);
            
            double hz_curr = hz_a * pow( HALFTONE, guide_tone[t] );
             
            //  周波数を、画面上のy座標に変換
            int y = _get_y_from_hz(gv, hz_curr);

            //  y が表示領域内の場合のみ、描画を行う
            if ( y <= screen_bottom && y >= screen_top ) {
                gdk_draw_line(  graph->window, gc,
                                screen_left,   y,
                                screen_right,  y
                );
            } else if ( y < screen_top ) {
                //  上限まで描き終えたので、これ以上は描画の必要はない
                break;
            }
        } 
        hz_a *= 2;  //  基準のAを1オクターブ上げる
    }

    g_object_unref(gc);
}


static gboolean _draw_graph(GtkWidget *graph, GdkEventExpose *event, gpointer _gv)
{
    GdkGC       *gc;
    GraphView   *gv = _gv;
    FreqdataList *samples = gv->samples;

    printf("Top : %f, Left: %ld\n", gv->screen_top_hzlog2, gv->screen_left_samplepoint);

    //  表示画面の幅と高さ
    gint width, height;
    gdk_drawable_get_size(graph->window, &width, &height);

    gc = gdk_gc_new(graph->window);

    //　全体を白で塗りつぶす
    _set_gc_color(gc, 65535, 65535, 65535);
    gdk_draw_rectangle( graph->window,
                        gc,
                        TRUE,       //  filled
                        0, 0,       //  x, y
                        width, height //  width, height
    );

    //  基準音にグレーのラインを引く
    _draw_tone_lines(gv);
    
    //  音を描画していく
    int f, a;
    for (f=0; f < samples->list->len; f++) {
        Freqdata *freq = g_ptr_array_index(samples->list, f);
    
        for (a=0; a < freq->ampdata_ary->len; a++) {
            Ampdata *amp = g_ptr_array_index(freq->ampdata_ary, a);
    
            //  音量が強いほど原色に近くする
            int red     = 65535 - (amp->amp * 30000);
            int green   = 65535 - (amp->amp * 15000);
            int blue    = 65535 - (amp->amp * 30000); 
            if (red   < 0)  red   = 0;
            if (green < 0)  green = 0;
            if (blue  < 0)  blue  = 0;
            _set_gc_color(gc, red, green, blue);
            
            gdk_draw_rectangle( graph->window, 
                                gc, 
                                TRUE,
                                freq->sample_point * gv->zoom_x,
                                _get_y_from_hz(gv, amp->freq),
                                samples->interval * gv->zoom_x,
                                3
            );
        }
    }

    g_object_unref(gc); 

    return TRUE;
}

/*
 * Graphviewオブジェクトを新規作成
 *
 * 引数：
 *   window : このビューを表示する大本のウィンドウ
 *
 */
GraphView *graphview_new(GtkWidget *window) 
{
    GraphView *gv = malloc(sizeof(GraphView));
    
    if (!gv) {
        perror("Failed to alloc memory for GraphView");
        exit(EXIT_FAILURE);
    }

    //  初期値セット
    gv->screen_top_hzlog2 = SCREEN_TOP_HZLOG2;
    gv->screen_left_samplepoint = SCREEN_LEFT_SAMPLEPOINT;
    gv->zoom_x = ZOOM_X;
    gv->zoom_y = ZOOM_Y;

    //  スクロールウィンドウの生成
    gv->swin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(
                GTK_SCROLLED_WINDOW(gv->swin), 
                GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS );

    //　描画エリアの生成
    gv->graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(gv->graph, 640, 400);
    g_signal_connect(G_OBJECT(gv->graph), "expose_event",
                     G_CALLBACK(_draw_graph), (gpointer)gv );
    
    gtk_scrolled_window_add_with_viewport(
            GTK_SCROLLED_WINDOW(gv->swin), gv->graph ); 
    if (window) {
        gtk_container_add(GTK_CONTAINER(window), gv->swin);
    }

    //  スクロールバーの初期化
    _init_hadjustment(gv);
    _init_vadjustment(gv);

    return gv;
}



/*
 * サンプルデータをセットする
 *
 * 引数：
 *   gv     : セットする対象の GraphView オブジェクト
 *   sample : セットするサンプルデータ
 *
 */ 
void graphview_set_sample_data(GraphView *gv, FreqdataList *sample)
{
    gv->samples = sample;
}
    

/*
 * Graphview オブジェクトを開放する
 *
 */
void graphview_free(GraphView *gv) {
    free(gv);
}


/* 
 * GraphView オブジェクトを開放すると同時に、
 * 保持している sample も開放する。
 */
void graphview_free_with_samples(GraphView *gv)
{
    g_ptr_array_free(gv->samples->list, TRUE);
    graphview_free(gv);
}   



