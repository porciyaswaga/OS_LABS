#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int (*gcf_f)(int, int);
typedef char* (*tr_f)(long);

int main() {
    void* handle = NULL;
    gcf_f GCF = NULL;
    tr_f translation = NULL;

    int current_lib = 1;

    handle = dlopen("./lib1.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return 1;
    }

    GCF = (gcf_f)dlsym(handle, "GCF");
    translation = (tr_f)dlsym(handle, "translation");

    if (!GCF || !translation) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    while (1) {
        int cmd;
        if (scanf("%d", &cmd) != 1)
            break;

        if (cmd == -1) {
            break;
        }

        if (cmd == 0) {
            dlclose(handle);

            if (current_lib == 1) {
                handle = dlopen("./lib2.so", RTLD_LAZY);
                current_lib = 2;
            } else {
                handle = dlopen("./lib1.so", RTLD_LAZY);
                current_lib = 1;
            }

            if (!handle) {
                fprintf(stderr, "dlopen error: %s\n", dlerror());
                return 1;
            }

            GCF = (gcf_f)dlsym(handle, "GCF");
            translation = (tr_f)dlsym(handle, "translation");

            if (!GCF || !translation) {
                fprintf(stderr, "dlsym error: %s\n", dlerror());
                dlclose(handle);
                return 1;
            }

            printf("СМЕНЯ ПРОГРАММЫ %d\n", current_lib);
            continue;
        }

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
        } else {
            printf("Unknown command\n");
        }
    }

    dlclose(handle);
    return 0;
}
