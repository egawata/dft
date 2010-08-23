#include "wavfile.h"
#include <stdio.h>

#define NUM_SAMPLE 1024

int main(int argc, char *argv[])
{
    short buf[NUM_SAMPLE];
    int i;

    if (!argv[1]) {
        printf("Usage: readwav.c [filename]\n");
        return 1;
    }

    WavData *wav = open_wavfile(argv[1]);
    printf("Open %s\n", argv[1]);

    size_t size = read_data(wav, buf, NUM_SAMPLE);
    printf("Read: %d bytes\n", size);

    for (i=0; i < 1024; i++) {
        printf("Data: %d\n", buf[i]);
    }     

    close_wavfile(wav);
    printf("Closed\n");

    return 0;
}



