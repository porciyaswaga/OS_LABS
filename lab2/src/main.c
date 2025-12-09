#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    int *arr;
    int start;
    int len;
} RunParams;

typedef struct {
    int *arr;
    int start1;
    int len1;
    int start2;
    int len2;
} MergeParams;

void* MergeRuns(void* args) {
    MergeParams* params = (MergeParams*) args;
    int* arr = params->arr;
    int start1 = params->start1;
    int len1 = params->len1;
    int start2 = params->start2;
    int len2 = params->len2;
    int* temp = malloc((len1 + len2) * sizeof(int));
    int i = 0, j = 0, k = 0;
    while(i < len1 && j < len2) {
        if (arr[start1 + i] <= arr[start2 + j]) {
            temp[k++] = arr[start1 + i++];
        } else {
            temp[k++] = arr[start2 + j++];
        }
    }
    while (i < len1) temp[k++] = arr[start1 + i++];
    while (j < len2) temp[k++] = arr[start2 + j++];
    for (int x = 0; x < len1 + len2; x++)
        arr[start1 + x] = temp[x];
    free(temp);
    return NULL;
}

void* IntertionalSort(void* args) {
    RunParams* params = (RunParams*) args;
    int* arr = params->arr;
    int start = params->start;
    int len   = params->len;
    for (int i = start + 1; i < start + len; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= start && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
    return NULL;
}

int MaxSizeOfRuns(int size) {
    int n = size, r = 0;
    while (n >= 64) {
        r |= (n & 1);
        n >>= 1;
    }
    return n + r;
}

int count_numbers_in_file(FILE* file) {
    int count = 0;
    int num;
    while (fscanf(file, "%d", &num) == 1) {
        count++;
    }
    rewind(file);
    return count;
}

int main(int argc, char* argv[]) {  
    int max_count_of_threads = 0;
    char* filename = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 && (i + 1) < argc) {
            max_count_of_threads = atoi(argv[i + 1]);
            i++;
        } else {
            filename = argv[i];
        }
    }
    printf("Максимально число потоков: %d\n", max_count_of_threads);
    FILE* input = stdin;
    if (filename != NULL) {
        input = fopen(filename, "r");
        if (!input) {
            perror("Не удалось открыть файл");
            return 1;
        }
    }
    int size;
    if (input == stdin) {
        if (fscanf(input, "%d", &size) != 1) {
            printf("Ошибка чтения размера массива!\n");
            return 1;
        }
    } else {
        size = count_numbers_in_file(input);
        printf("Обнаружено %d чисел в файле\n", size);
    }
    int *arr = malloc(size * sizeof(int));
    if (!arr) {
        printf("Ошибка выделения памяти!\n");
        if (input != stdin) fclose(input);
        return 1;
    }
    for (int i = 0; i < size; i++) {
        if (fscanf(input, "%d", &arr[i]) != 1) {
            printf("Ошибка чтения элемента массива! i=%d\n", i);
            free(arr);
            if (input != stdin) fclose(input);
            return 1;
        }
    }
    if (input != stdin) fclose(input);
    int min_size_of_run = MaxSizeOfRuns(size);
    printf("Минимальный размер каждого run'а: %d\n", min_size_of_run);
    int count_of_runs = (size + min_size_of_run - 1) / min_size_of_run;
    printf("Количество подмассивов run: %d\n", count_of_runs);
    RunParams run_params[count_of_runs];
    for (int i = 0; i < count_of_runs; i++) {
        run_params[i].arr = arr;
        run_params[i].start = i * min_size_of_run;
        run_params[i].len = (i == count_of_runs - 1) ? (size - i * min_size_of_run) : min_size_of_run;
    }
    pthread_t* threads = malloc(sizeof(pthread_t) * (max_count_of_threads > 0 ? max_count_of_threads : 1));
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);  
    if (max_count_of_threads >= count_of_runs) {
        for (int i = 0; i < count_of_runs; i++)
            pthread_create(&threads[i], NULL, IntertionalSort, &run_params[i]);
        for (int i = 0; i < count_of_runs; i++)
            pthread_join(threads[i], NULL);
        int current_count_of_runs = count_of_runs;
        RunParams* current_runs = run_params;
        while (current_count_of_runs > 1) {
            int run_pairs = current_count_of_runs / 2;
            MergeParams* merge_params[run_pairs];
            for (int i = 0; i < run_pairs; i++) {
                merge_params[i] = malloc(sizeof(MergeParams));
                merge_params[i]->arr = arr;
                merge_params[i]->start1 = current_runs[2*i].start;
                merge_params[i]->len1 = current_runs[2*i].len;
                merge_params[i]->start2 = current_runs[2*i+1].start;
                merge_params[i]->len2 = current_runs[2*i+1].len;
                pthread_create(&threads[i], NULL, MergeRuns, merge_params[i]);
            }
            for (int i = 0; i < run_pairs; i++)
                pthread_join(threads[i], NULL);
            for (int i = 0; i < run_pairs; i++)
                free(merge_params[i]);
            for (int i = 0; i < run_pairs; i++) {
                current_runs[i].start = current_runs[2*i].start;
                current_runs[i].len = current_runs[2*i].len + current_runs[2*i+1].len;
            }
            if (current_count_of_runs % 2 == 1)
                current_runs[run_pairs] = current_runs[current_count_of_runs - 1];
            current_count_of_runs = run_pairs + (current_count_of_runs % 2);
        }
    } else {
        int next_run = 0;
        while (next_run < count_of_runs) {
            int active = 0;
            while (active < max_count_of_threads && next_run < count_of_runs) {
                pthread_create(&threads[active], NULL, IntertionalSort, &run_params[next_run]);
                active++;
                next_run++;
            }
            for (int i = 0; i < active; i++)
                pthread_join(threads[i], NULL);
        }
        int current_count_of_runs = count_of_runs;
        RunParams* current_runs = run_params;
        while (current_count_of_runs > 1) {
            int run_pairs = current_count_of_runs / 2;
            for (int i = 0; i < run_pairs; i++) {
                MergeParams mp;
                mp.arr = arr;
                mp.start1 = current_runs[2*i].start;
                mp.len1 = current_runs[2*i].len;
                mp.start2 = current_runs[2*i+1].start;
                mp.len2 = current_runs[2*i+1].len;
                MergeRuns(&mp);
                current_runs[i].start = current_runs[2*i].start;
                current_runs[i].len = current_runs[2*i].len + current_runs[2*i+1].len;
            }
            if (current_count_of_runs % 2 == 1)
                current_runs[run_pairs] = current_runs[current_count_of_runs - 1];
            current_count_of_runs = run_pairs + (current_count_of_runs % 2);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    printf("Отсортированный массив: [");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i != size - 1) printf(" ");
    }
    printf("]\n");

    double elapsed_sec = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Время работы: %.6f секунд\n", elapsed_sec);

    free(arr);
    free(threads);
    return 0;
}
