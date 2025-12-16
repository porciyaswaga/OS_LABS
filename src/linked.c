#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

int main() {
    int cmd;
    while (scanf("%d", &cmd)) {
        if (cmd == 1) {
            int a, b;
            scanf("%d %d", &a, &b);
            printf("%d\n", GCF(a, b));
        } else if (cmd == 2) {
            long x;
            scanf("%ld", &x);
            char* s = translation(x);
            printf("%s\n", s);
            free(s);
        }
    }
}
