#include "wavfile.h"
#include <stdio.h>
#include <math.h>

#define DELTA       5
#define NUM_SAMPLE  44100/DELTA
#define MAX_FREQ    2000
#define MAX_SINT    32768
#define PI          3.14159
#define MIN_AMP     0.5 


void dft(short *sample, size_t num_sample, double *result, double max_freq, double delta) 
{
    double s = 2 * PI / num_sample;
    int w, t;

    for (w = 1; w <= max_freq / delta; w++) {
        double real = 0.0, imag = 0.0;

        for (t = 0; t < num_sample; t++) {
            double theta = w * s * t;
            real += sample[t] * cos(theta);
            imag += sample[t] * sin(theta);
        }
        real *= 2 * PI / num_sample / MAX_SINT;
        imag *= 2 * PI / num_sample / MAX_SINT;
        
        result[w - 1] = sqrt(real * real + imag * imag);
    }
}



int main(int argc, char *argv[])
{
    short buf[NUM_SAMPLE];

    if (!argv[1]) {
        printf("Usage: readwav.c [filename]\n");
        return 1;
    }

    WavData *wav = open_wavfile(argv[1]);

    long current_ptr = 0;
    size_t size;
    double result[MAX_FREQ / DELTA];
    while ((size = read_data(wav, buf, NUM_SAMPLE)) > 0) {
        printf("#%ld\n", current_ptr);
        dft(buf, size, result, MAX_FREQ, DELTA);
        int r;
        for (r = 0; r < MAX_FREQ / DELTA; r++) {
            if (result[r] > MIN_AMP)
                printf("%d %f\n", (r+1) * DELTA, result[r]);
        }
        printf("\n");
        current_ptr += size;
    }

    close_wavfile(wav);

    return 0;
}



