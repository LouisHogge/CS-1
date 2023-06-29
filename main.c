#include <stdio.h>
#include <stdlib.h>

#include "PNM.h"
#include "SeamCarving.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: ./SeamCarving input.pnm output.pnm k\n");
        return -1;
    }

    // Load image
    PNMImage* original = readPNM(argv[1]);
    if (!original) {
        fprintf(stderr, "Cannot load image '%s'\n", argv[1]);
        return -1;
    }

    // Seam carving
    int k = atoi(argv[3]);

    if (k >= (int)original->width) {
      fprintf(stderr, "k should be strictly smaller than image width\n");
      freePNM(original);
      return -1;
    }
    
    PNMImage* new = shrinkImage(original, k);
    
    if (!new) {
        fprintf(stderr, "Cannot build new image\n");
        freePNM(original);
        return -1;
    }

    // Save and free
    writePNM(argv[2], new);
    freePNM(original);
    if (new != original)
      freePNM(new);

    return 0;
}

