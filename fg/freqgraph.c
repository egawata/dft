#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ampdata.h"
#include "freqdata.h"
#include "freqdatalist.h"
#include "graphview.h"


//  デバッグ用
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


static void destroy(GtkWidget *widget, GraphView *gv) 
{
    graphview_free_with_samples(gv);
    gtk_main_quit();
}



int main(int argc, char *argv[])
{
    GtkWidget *window;
    FreqdataList fl;
    GraphView *gv;

    read_file(argv[1], &fl);

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Frequency Graph");
    gtk_widget_set_size_request(window, 600, 400);

    gv = graphview_new(window);
    graphview_set_sample_data(gv, &fl);
    
    g_signal_connect(G_OBJECT(window), "destroy", 
                     G_CALLBACK(destroy), (gpointer)gv);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}


