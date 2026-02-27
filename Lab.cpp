#include <iostream>

const int MAXN = 65000;

struct Object {
    char key[33];
    char value[2049];
};

int hex(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return 0;
}

void countingSort(int* idx, int* buf, Object* a, int n, int pos) {
    int cnt[16] = {0};

    for (int i = 0; i < n; i++) {
        cnt[hex(a[idx[i]].key[pos])]++;
    }

    for (int i = 1; i < 16; i++) {
        cnt[i] += cnt[i - 1];
    }

    for (int i = n - 1; i >= 0; i--) {
        buf[--cnt[hex(a[idx[i]].key[pos])]] = idx[i];
    }

    for (int i = 0; i < n; i++) {
        idx[i] = buf[i];
    }
}

void radixSort(Object* a, int n) {
    int* idx = new int[n];
    int* buf = new int[n];

    for (int i = 0; i < n; i++) {
        idx[i] = i;
    }

    for (int pos = 31; pos >= 0; pos--) {
        countingSort(idx, buf, a, n, pos);
    }

    for (int i = 0; i < n; i++) {
        while (idx[i] != i) {
            std::swap(a[i], a[idx[i]]);
            std::swap(idx[i], idx[idx[i]]);
        }
    }

    delete[] idx;
    delete[] buf;
}

int main() {
    Object* a = new Object[MAXN];
    char line[2082];
    int n = 0;

    while (n < MAXN && std::cin.getline(line, 2082)) {
        if (!line[0]) {
            continue;
        }

        for (int i = 0; i < 32; i++) {
            a[n].key[i] = line[i];
        }
        a[n].key[32] = 0;

        int p = 32;
        if (line[p] == '\t') {
            p++;
        }

        int j = 0;
        while (line[p] && j < 2048) {
            a[n].value[j] = line[p];
            j++;
            p++;
        }
        a[n].value[j] = 0;

        n++;
    }

    radixSort(a, n);

    for (int i = 0; i < n; i++) {
        std::cout << a[i].key << '\t' << a[i].value << '\n';
    }

    delete[] a;
    return 0;
}
