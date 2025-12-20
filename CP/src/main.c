#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <json-c/json.h>

#define MAX_JOBS 32
#define MAX_MUTEX 16
#define MAX_DEPS 10

typedef struct Job {
    char id[32];
    char mutex_name[32];
    char deps[MAX_DEPS][32];
    int deps_count;
    int completed_deps;
    int work_time;
    int done;
    pthread_t thread;
} Job;

typedef struct {
    char name[32];
    pthread_mutex_t mutex;
} NamedMutex;

Job jobs[MAX_JOBS];
NamedMutex mutexes[MAX_MUTEX];

int job_count = 0;
int mutex_count = 0;

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t* find_mutex(const char* name) {
    for (int i = 0; i < mutex_count; i++) {
        if (strcmp(mutexes[i].name, name) == 0)
            return &mutexes[i].mutex;
    }
    return NULL;
}

int deps_done(Job* job) {
    return job->completed_deps == job->deps_count;
}

int check_no_cycles() {
    int in_degree[MAX_JOBS] = {0};

    for (int i = 0; i < job_count; i++) {
        for (int d = 0; d < jobs[i].deps_count; d++) {
            int found = 0;
            for (int j = 0; j < job_count; j++) {
                if (strcmp(jobs[j].id, jobs[i].deps[d]) == 0) {
                    in_degree[i]++;
                    found = 1;
                }
            }
            if (!found) {
                printf("ОШИБКА: задача %s зависит от неизвестной задачи %s\n",
                       jobs[i].id, jobs[i].deps[d]);
                return 0;
            }
        }
    }

    int queue[MAX_JOBS];
    int qh = 0, qt = 0;

    for (int i = 0; i < job_count; i++)
        if (in_degree[i] == 0)
            queue[qt++] = i;

    int processed = 0;

    while (qh < qt) {
        int u = queue[qh++];
        processed++;

        for (int i = 0; i < job_count; i++) {
            for (int d = 0; d < jobs[i].deps_count; d++) {
                if (strcmp(jobs[i].deps[d], jobs[u].id) == 0) {
                    if (--in_degree[i] == 0)
                        queue[qt++] = i;
                }
            }
        }
    }

    if (processed != job_count) {
        printf("ОШИБКА: обнаружен цикл в DAG\n");
        return 0;
    }

    return 1;
}

void dfs(int v, int visited[]) {
    visited[v] = 1;

    for (int i = 0; i < job_count; i++) {
        for (int d = 0; d < jobs[i].deps_count; d++) {
            if (strcmp(jobs[i].deps[d], jobs[v].id) == 0 && !visited[i])
                dfs(i, visited);
        }
        for (int d = 0; d < jobs[v].deps_count; d++) {
            if (strcmp(jobs[v].deps[d], jobs[i].id) == 0 && !visited[i])
                dfs(i, visited);
        }
    }
}

int check_single_connected_component() {
    int visited[MAX_JOBS] = {0};
    dfs(0, visited);

    for (int i = 0; i < job_count; i++) {
        if (!visited[i]) {
            printf("ОШИБКА: DAG состоит из нескольких связанных компонентов\n");
            return 0;
        }
    }
    return 1;
}

int check_start_and_end_jobs() {
    int start = 0, end = 0;

    for (int i = 0; i < job_count; i++)
        if (jobs[i].deps_count == 0)
            start++;

    for (int i = 0; i < job_count; i++) {
        int is_end = 1;
        for (int j = 0; j < job_count; j++)
            for (int d = 0; d < jobs[j].deps_count; d++)
                if (strcmp(jobs[j].deps[d], jobs[i].id) == 0)
                    is_end = 0;
        if (is_end)
            end++;
    }

    if (start == 0) {
        printf("ОШИБКА: отсутствуют стартовые задачи\n");
        return 0;
    }

    if (end == 0) {
        printf("ОШИБКА: отсутствуют конечные задачи\n");
        return 0;
    }

    return 1;
}

void* job_runner(void* arg) {
    Job* job = (Job*)arg;

    pthread_mutex_lock(&global_lock);
    while (!deps_done(job))
        pthread_cond_wait(&cond, &global_lock);
    pthread_mutex_unlock(&global_lock);

    pthread_mutex_t* mtx = NULL;
    if (strlen(job->mutex_name) > 0)
        mtx = find_mutex(job->mutex_name);

    if (mtx) pthread_mutex_lock(mtx);

    printf("[START] Job %s\n", job->id);
    sleep(job->work_time);
    printf("[END]   Job %s\n", job->id);

    if (mtx) pthread_mutex_unlock(mtx);

    pthread_mutex_lock(&global_lock);
    job->done = 1;

    for (int i = 0; i < job_count; i++)
        for (int d = 0; d < jobs[i].deps_count; d++)
            if (strcmp(jobs[i].deps[d], job->id) == 0)
                jobs[i].completed_deps++;

    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&global_lock);

    return NULL;
}

void load_json(const char* filename) {
    struct json_object* root = json_object_from_file(filename);
    if (!root) {
        printf("Невозможно открыть JSON файл\n");
        exit(1);
    }

    /* mutexes */
    struct json_object* jmutexes;
    json_object_object_get_ex(root, "mutexes", &jmutexes);
    mutex_count = json_object_array_length(jmutexes);

    for (int i = 0; i < mutex_count; i++) {
        strcpy(mutexes[i].name,
               json_object_get_string(json_object_array_get_idx(jmutexes, i)));
        pthread_mutex_init(&mutexes[i].mutex, NULL);
    }

    /* jobs */
    struct json_object* jjobs;
    json_object_object_get_ex(root, "jobs", &jjobs);
    job_count = json_object_array_length(jjobs);

    for (int i = 0; i < job_count; i++) {
        memset(&jobs[i], 0, sizeof(Job));

        struct json_object* j = json_object_array_get_idx(jjobs, i);

        strcpy(jobs[i].id,
               json_object_get_string(json_object_object_get(j, "id")));

        struct json_object* jm;
        if (json_object_object_get_ex(j, "mutex", &jm))
            strcpy(jobs[i].mutex_name, json_object_get_string(jm));
        else
            jobs[i].mutex_name[0] = '\0';

        jobs[i].work_time =
            json_object_get_int(json_object_object_get(j, "work_time"));

        struct json_object* jdeps;
        json_object_object_get_ex(j, "deps", &jdeps);
        jobs[i].deps_count = json_object_array_length(jdeps);

        for (int d = 0; d < jobs[i].deps_count; d++) {
            strcpy(jobs[i].deps[d],
                   json_object_get_string(json_object_array_get_idx(jdeps, d)));
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Использование: %s jobs.json\n", argv[0]);
        return 1;
    }

    load_json(argv[1]);

    if (!check_no_cycles()) return 1;
    if (!check_single_connected_component()) return 1;
    if (!check_start_and_end_jobs()) return 1;

    printf("Проверка DAG пройдена успешно\n");

    for (int i = 0; i < job_count; i++)
        pthread_create(&jobs[i].thread, NULL, job_runner, &jobs[i]);

    for (int i = 0; i < job_count; i++)
        pthread_join(jobs[i].thread, NULL);

    printf("Все задачи выполнены успешно\n");
    return 0;
}
