#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct _ampdata {
    int     freq;
    double  amp;
} Ampdata;


typedef struct _freqdata {
    long        sample_point;
    GPtrArray   *ampdata_ary;
    int         index_next;
} Freqdata;


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

GPtrArray *read_file(const char *filename) 
{
    FILE *fp;
    GPtrArray *freqdata_list = g_ptr_array_new_with_free_func(g_freqdata_free);

    if (!(fp = fopen(filename, "r"))) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    while(1) {
        char buf[256];
        
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

            g_ptr_array_add(freqdata_list, (gpointer)freqdata);

        } else {
            fprintf(stderr, "Invalid format %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    print_g_ptr_array(freqdata_list);

    fclose(fp);
    return freqdata_list;
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


static gboolean draw_graph(GtkWidget *graph, GdkEventExpose *event, gpointer data)
{
    GdkGC       *gc;
    GPtrArray   *freqdata_list = (GPtrArray *)data;

    gc = gdk_gc_new(graph->window);

    //　全体を白で塗りつぶす
    set_gc_color(gc, 65535, 65535, 65535);
    gdk_draw_rectangle( graph->window,
                        gc,
                        TRUE,       //  filled
                        0, 0,       //  x, y
                        600, 400    //  width, height
    );

    //  (0,200)-(599,200) にグレーのラインを引く
    set_gc_color(gc, 32768, 32768, 32768);
    gdk_draw_line( graph->window, 
                   gc,
                   0, 200,
                   599, 200
    );
    
    //  音を描画していく
    Freqdata *last_freq = g_ptr_array_index(freqdata_list, freqdata_list->len - 1);
    long last_sample_point = last_freq->sample_point;

    int f, a;
    for (f=0; f < freqdata_list->len; f++) {
        Freqdata *freq = g_ptr_array_index(freqdata_list, f);
    
        for (a=0; a < freq->ampdata_ary->len; a++) {
            Ampdata *amp = g_ptr_array_index(freq->ampdata_ary, a);
    
            set_gc_color(gc, 65535, 40000, 40000);
            gdk_draw_rectangle( graph->window, 
                                gc, 
                                TRUE,
                                freq->sample_point * 600 / last_sample_point,
                                - (amp->freq - 600) ,
                                amp->amp * 5,
                                amp->amp * 5
            );
        }
    }

    g_object_unref(gc); 

    return TRUE;
}


int main(int argc, char *argv[])
{
    GtkWidget *window, *graph;

    GPtrArray *freqdata_list = read_file(argv[1]);

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Frequency Graph");
    gtk_widget_set_size_request(window, 600, 400);

    g_signal_connect(G_OBJECT(window), "destroy", 
                     G_CALLBACK(destroy), NULL);

    graph = gtk_drawing_area_new();
    g_signal_connect(G_OBJECT(graph), "expose_event",
                     G_CALLBACK(draw_graph), (gpointer)freqdata_list);
    gtk_container_add(GTK_CONTAINER(window), graph);
    gtk_widget_show_all(window);

    gtk_main();

    g_ptr_array_free(freqdata_list, TRUE);
    return 0;
}


