#include <stdlib.h>
#include <string.h>
#include "lib.h"

int GCF(int A, int B) {
    while (B != 0) {
        int t = B;
        B = A % B;
        A = t;
    }
    return A;
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
        res[i++] = (x % 2) + '0';
        x /= 2;
    }
    res[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char tmp = res[j];
        res[j] = res[i - j - 1];
        res[i - j - 1] = tmp;
    }
    return res;
}
