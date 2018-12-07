//#include "frost.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_FILE_SIZE 1000000
#define BLOCK_SIZE 256
#define CHANNELS 4



int main(int argc, char const *argv[]) {
    unsigned char buffer[MAX_FILE_SIZE];
    int i, j, k, n, num_blocks;


    char *filenames[4];
    filenames[0] = (char*)"test.raw";
    filenames[1] = (char*)"test.raw";    
    filenames[2] = (char*)"test.raw";    
    filenames[3] = (char*)"test.raw";

    FILE *f = fopen(filenames[0], "r");
    if(!f) {
      printf("oh no\n");
      return -1;
    }

    n = fread(buffer, 1, MAX_FILE_SIZE, f);
    fclose(f);

    num_blocks = n/BLOCK_SIZE + 1;
    printf("num = %d\n", num_blocks);

    char blocks[CHANNELS][num_blocks][BLOCK_SIZE];
    printf("channels: %d, numbloks: %d, blocksize: %d\n", CHANNELS, num_blocks, BLOCK_SIZE);

    for(k=0; k < CHANNELS; k++) {
        f = fopen(filenames[k], "rb");
        n = fread(buffer, 1, MAX_FILE_SIZE, f);
        printf("%d\n", k);
        fclose(f);
        num_blocks = n/BLOCK_SIZE;
        for(i = 0; i < num_blocks; i++) {
            for (j = 0; j < BLOCK_SIZE; j++) {
                //printf("%d, %d, %d\n", k, i, j);
                blocks[k][i][j] = buffer[i*BLOCK_SIZE + j];
                if(blocks[k][i][j] != 0) {
                  //printf("%d, %d, %d: %d\n", k, i, j, blocks[k][i][j]);
                }
            }
        }
    }
    printf("donezo\n");

    // test that new breaking up into blocks didn't mess up the file
    FILE* newfile = fopen("new.raw", "w");
    size_t s = fwrite(blocks, num_blocks, BLOCK_SIZE, newfile);

    for(k=0; k < CHANNELS; k++) {
        char num[10];
        sprintf(num, "%d", k);
        char *filename = (char*)calloc(sizeof(char), 2);
        strcpy(filename, "new");
        filename = strncat(filename, num, 1);
        filename = strncat(filename, ".csv", 4);
        printf("%s\n", filename);

        FILE *fp = fopen(filename, "w+");
        for(i=0; i<num_blocks; i++) {
            for(j=0; j<BLOCK_SIZE; j++) {
                fprintf(fp, ",%d ", blocks[k][i][j]);
            }
        }
        fclose(fp);
    }

    return 0;
}
