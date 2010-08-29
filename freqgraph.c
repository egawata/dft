#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define HZ_TOP  600
#define ZOOM    200


typedef struct _ampdata {
    int     freq;
    double  amp;
} Ampdata;


typedef struct _freqdata {
    long        sample_point;
    GPtrArray   *ampdata_ary;
    int         index_next;
} Freqdata;


typedef struct _freqdata_list {
    long        num_sample;
    int         interval;
    GPtrArray   *list;
} FreqdataList;


Ampdata *ampdata_new(void) {
    Ampdata *data = malloc(sizeof(Ampdata));
    if (!data) {
        perror("Failed to allocate memory for ampdata");
        exit(EXIT_FAILURE);
    }
    return data;
}


Ampdata *ampdata_new_with_param(int freq, double amp) 
{
    Ampdata *data = ampdata_new();
    data->freq = freq;
    data->amp  = amp;
    return data;
}


void ampdata_free(Ampdata *data) 
{
    free(data);
}


void g_ampdata_free(gpointer data)
{
    ampdata_free((Ampdata *)data);
}


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


Freqdata *freqdata_new_with_param(long sample_point) {
    Freqdata *data = freqdata_new();
    data->sample_point = sample_point;
    return data;
}


Ampdata *freqdata_next_ampdata(Freqdata *data) 
{
    if (data->index_next > data->ampdata_ary->len) {
        return (Ampdata *)g_ptr_array_index(data->ampdata_ary, data->index_next++);
    } else {
        return NULL;
    }
}


void freqdata_add_ampdata(Freqdata *data, int freq, double amp)
{
    Ampdata *ampdata = ampdata_new_with_param(freq, amp);
    g_ptr_array_add(data->ampdata_ary, (gpointer)ampdata);
}


void freqdata_free(Freqdata *data) 
{
    g_ptr_array_free((gpointer)data->ampdata_ary, TRUE);
    free(data);
}

void g_freqdata_free(gpointer data) 
{
    freqdata_free((Freqdata *)data);
}


void print_g_ptr_array(GPtrArray *array)
{
    int f, a;
    for (f=0; f < array->len; f++) {
        Freqdata *freq = g_ptr_array_index(array, f);
        printf("#%ld\n", freq->sample_point);
        for (a=0; a < freq->ampdata_ary->len; a++) {
            Ampdata *amp = g_ptr_array_index(freq->ampdata_ary, a);
            printf("%d %8.5f\n", amp->freq, amp->amp);
        }
        printf("\n");
    }
}

void read_file(const char *filename, FreqdataList *fl) 
{
    FILE *fp;
    char buf[256];

    fl->list = g_ptr_array_new_with_free_func(g_freqdata_free);

    if (!(fp = fopen(filename, "r"))) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    //  総サンプル数
    fgets(buf, 256, fp);
    fl->num_sample = atol(buf);

    //  サンプル間隔
    fgets(buf, 256, fp);
    fl->interval = atoi(buf);

    while(1) {
        
        if ( !fgets(buf, 256, fp) ) {
            if (feof(fp)) 
                break;
            else {
                fprintf(stderr, "Failed to read from file");
                exit(EXIT_FAILURE);
            }
        }

        if (buf[0] == '#') {    //  Sample point

            long sample_point = atol(buf + 1);
            Freqdata *freqdata = freqdata_new_with_param(sample_point);
            
            //  周波数＋音量データを追加していく
            //　データの終端は、改行文字(\n) + \0 で、文字列長は1。これが出たら終了。
            while ( fgets(buf, 256, fp) && strlen(buf) > 1 ) {
                //  改行文字＋スペースは、\0に変換
                char *buf_amp = strchr(buf, ' ');
                *buf_amp = '\0';
                buf_amp++;

                int freq = atoi(buf);
                double amp = atof(buf_amp);
                Ampdata *ampdata = ampdata_new_with_param(freq, amp); 
                g_ptr_array_add(freqdata->ampdata_ary, (gpointer)ampdata);
            }

            g_ptr_array_add(fl->list, (gpointer)freqdata);

        } else {
            fprintf(stderr, "Invalid format %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    print_g_ptr_array(fl->list);

    fclose(fp);
}


static void destroy(GtkWidget *widget, gpointer data) 
{
    gtk_main_quit();
}


static void set_gc_color(GdkGC *gc, guint16 red, guint16 green, guint16 blue)
{
    GdkColor    color;

    color.red   = red;
    color.green = green;
    color.blue  = blue;

    gdk_gc_set_rgb_fg_color(gc, &color);
}


/*
 *  特定の周波数の、画面上のy座標を求める
 *
 *  　y座標は、画面上端の周波数(hz_top)、ズームレベル(zoom)、
 *    およびy座標を求めたい周波数(hz_req)から
 *  　導くことができる。
 */ 
static double get_y_from_hz(double hz_req, double hz_top, double zoom) {
    return zoom * ( (log(hz_top) - log(hz_req)) / log(2) );
}

/*
 *  特定のy座標が表す周波数を求める
 *    get_y_from_hz の逆を行えばよい。
 */
static double get_hz_from_y(double y, double hz_top, double zoom) {
    return log(hz_top) - ( y * log(2) ) / zoom ;
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
static void draw_tone_lines(GtkWidget *graph, int hz_high_limit, int zoom) 
{
    GdkGC *gc = gdk_gc_new(graph->window);

    //  表示領域の水平座標の範囲
    //  TODO: あとで引数で指定できるようにする
    gint screen_left   =   0;
    gint screen_right;

    //  表示領域の垂直座標の範囲
    //  TODO: あとで引数で指定できるようにする
    gint screen_top    =   0;
    gint screen_bottom;

    gdk_drawable_get_size(graph->window, &screen_right, &screen_bottom);

    //  画面下端の周波数を求める
    int hz_low_limit  = get_hz_from_y(screen_bottom - screen_top, hz_high_limit, zoom);

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
                set_gc_color(gc, 40000, 40000, 40000);
            else 
                set_gc_color(gc, 55000, 55000, 55000);
            
            double hz_curr = hz_a * pow( HALFTONE, guide_tone[t] );
             
            //  周波数を、画面上のy座標に変換
            int y = get_y_from_hz(hz_curr, hz_high_limit, zoom);
            if ( y <= screen_bottom && y >= screen_top ) {
                gdk_draw_line( graph->window, gc,
                        screen_left, y,
                        screen_right, y
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


static gboolean draw_graph(GtkWidget *graph, GdkEventExpose *event, gpointer data)
{
    GdkGC       *gc;
    FreqdataList *fl = (FreqdataList *)data;

    gint width, height;
    gdk_drawable_get_size(graph->window, &width, &height);

    gc = gdk_gc_new(graph->window);

    //　全体を白で塗りつぶす
    set_gc_color(gc, 65535, 65535, 65535);
    gdk_draw_rectangle( graph->window,
                        gc,
                        TRUE,       //  filled
                        0, 0,       //  x, y
                        width, height //  width, height
    );

    //  基準音にグレーのラインを引く
    draw_tone_lines(graph, HZ_TOP, ZOOM);
    
    //  音を描画していく
    int f, a;
    for (f=0; f < fl->list->len; f++) {
        Freqdata *freq = g_ptr_array_index(fl->list, f);
    
        for (a=0; a < freq->ampdata_ary->len; a++) {
            Ampdata *amp = g_ptr_array_index(freq->ampdata_ary, a);
    
            //  音量が強いほど原色に近くする
            int red     = 65535 - (amp->amp * 30000);
            int green   = 65535 - (amp->amp * 15000);
            int blue    = 65535 - (amp->amp * 30000); 
            if (red   < 0)  red   = 0;
            if (green < 0)  green = 0;
            if (blue  < 0)  blue  = 0;
            set_gc_color(gc, red, green, blue);
            
            gdk_draw_rectangle( graph->window, 
                                gc, 
                                TRUE,
                                freq->sample_point * 800 / fl->num_sample,
                                get_y_from_hz(amp->freq, HZ_TOP, ZOOM),
                                fl->interval * 800 / fl->num_sample,
                                3
            );
        }
    }

    g_object_unref(gc); 

    return TRUE;
}


int main(int argc, char *argv[])
{
    GtkWidget *window, *graph;
    FreqdataList fl;

    read_file(argv[1], &fl);

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Frequency Graph");
    gtk_widget_set_size_request(window, 600, 400);

    g_signal_connect(G_OBJECT(window), "destroy", 
                     G_CALLBACK(destroy), NULL);

    graph = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(graph), "expose_event",
                     G_CALLBACK(draw_graph), (gpointer)&fl);
    gtk_container_add(GTK_CONTAINER(window), graph);
    gtk_widget_show_all(window);

    gtk_main();

    g_ptr_array_free(fl.list, TRUE);
    return 0;
}


