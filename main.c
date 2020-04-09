#include "mylib.c"
#include <time.h>

int main()
{
    char *src = "abc";
    int size_src = strlen(src);
    char *compressed = malloc(size_src * 2);
    memset(compressed, 0, size_src * 2);
    printf("to compress src: %s\n", src);
    printf("to compress src size: %d\n", size_src);

    clock_t start, finish; 
    start = clock();
    for (int i = 0; i < 100000; i++)
    {
        int gzSize = gzCompress(src, size_src, compressed, size_src * 2);
        if (gzSize <= 0)
        {
            printf("compress error.\n");
            return -1;
        }
    }
    finish = clock(); 
    printf("time=%fs\n", (double)(finish - start) / CLOCKS_PER_SEC);

    free(compressed);
    return 0;
}