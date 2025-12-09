#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 256


struct MemoryShared
{
    sem_t parent_to_child_1;
    sem_t child_1_to_child_2;
    sem_t child_2_to_parent;
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    char buffer3[BUFFER_SIZE];
};


int main() {

    int fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
        if (fd == -1){
            perror("shm_open ошибка");
            exit(EXIT_FAILURE);
        }
    ftruncate(fd, sizeof(struct MemoryShared));

    struct MemoryShared *shared = mmap(NULL, sizeof(struct MemoryShared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap ошибка");
        exit(EXIT_FAILURE);
    }
    sem_init(&shared->parent_to_child_1, 1, 0);
    sem_init(&shared->child_1_to_child_2, 1, 0);
    sem_init(&shared->child_2_to_parent, 1, 0);
    char buffer[BUFFER_SIZE];

    printf("Введите строку для обработки: ");
    if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        perror("fgets ошибка");
        exit(EXIT_FAILURE);
    }
    strncpy(shared->buffer1, buffer, BUFFER_SIZE - 1);
    shared->buffer1[BUFFER_SIZE - 1] = '\0';
    sem_post(&shared->parent_to_child_1);

    pid_t child_1 = fork();
    if (child_1 == 0) {
        sem_wait(&shared->parent_to_child_1); 
        char child_1_buffer[BUFFER_SIZE];
        strncpy(child_1_buffer, shared->buffer1, BUFFER_SIZE - 1);
        child_1_buffer[BUFFER_SIZE - 1] = '\0';
        for (int i = 0; i < BUFFER_SIZE; i++) {
            child_1_buffer[i] = toupper(child_1_buffer[i]);
        }
        strncpy(shared->buffer2, child_1_buffer, BUFFER_SIZE - 1);
        shared->buffer2[BUFFER_SIZE - 1] = '\0';
        sem_post(&shared->child_1_to_child_2);
        exit(0);
    }

    pid_t child_2 = fork();
    if (child_2 == 0) {
        sem_wait(&shared->child_1_to_child_2);
        char child_2_buffer[BUFFER_SIZE];
        strncpy(child_2_buffer, shared->buffer2, BUFFER_SIZE - 1);
        child_2_buffer[BUFFER_SIZE - 1] = '\0';
        int j = 0;
        int prev_space = 0;
        for (int i = 0; child_2_buffer[i] != '\0'; ++i) {
            if (isspace(buffer[i])) {
                if (!prev_space) {
                    shared->buffer3[j++] = ' ';
                    prev_space = 1;
                }
            } else {
                shared->buffer3[j++] = child_2_buffer[i];
                prev_space = 0;
            }
        }
        shared->buffer3[j] = '\0';
        sem_post(&shared->child_2_to_parent);
        exit(0);
    }

    sem_wait(&shared->child_2_to_parent);
    waitpid(child_1, NULL, 0);
    waitpid(child_2, NULL, 0);
    printf("Результат: %s\n", shared->buffer3);

    munmap(shared, sizeof(struct MemoryShared));
    close(fd);
    shm_unlink("/my_shared_memory");

}
