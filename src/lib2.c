#include <stdlib.h>
#include "lib.h"

int GCF(int A, int B) {
    int g = 1;
    for (int i = 1; i <= A && i <= B; i++) {
        if (A % i == 0 && B % i == 0)
            g = i;
    }
    return g;
}

char* translation(long x) {
    char* res = malloc(65);
    int i = 0;

    if (x == 0) {
        res[0] = '0';
        res[1] = '\0';
        return res;
    }

    while (x > 0) {
        res[i++] = (x % 3) + '0';
        x /= 3;
    }
    res[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char tmp = res[j];
        res[j] = res[i - j - 1];
        res[i - j - 1] = tmp;
    }
    return res;
}
