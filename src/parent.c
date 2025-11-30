#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() 
{
    char buffer[BUFFER_SIZE];
    int pipe1[2], pipe2[2], pipe3[2];
    
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1 || pipe(pipe3) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    /*
    pipe1: parent -> child_1
    pipe2: child_1 -> child_2  
    pipe3: child_2 -> parent
    */

    // дочерний процесс child_1
    pid_t child_1 = fork();
    if (child_1 == 0) {
        close(pipe1[1]); close(pipe2[0]); close(pipe3[0]); close(pipe3[1]);
        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);
        execl("./child_1", "child_1", NULL);
        perror("execl child_1 failed");
        exit(EXIT_FAILURE);
    }
    
    // дочерний процесс child_2
    pid_t child_2 = fork();
    if (child_2 == 0) {
        close(pipe2[1]); close(pipe3[0]); close(pipe1[0]); close(pipe1[1]);
        dup2(pipe2[0], STDIN_FILENO);
        dup2(pipe3[1], STDOUT_FILENO);
        execl("./child_2", "child_2", NULL);
        perror("execl child_2 failed");
        exit(EXIT_FAILURE);
    }

    close(pipe1[0]); close(pipe2[0]); close(pipe2[1]); close(pipe3[1]);
    
    //родительйский процесс parent
    printf("Введите текст для обработки: ");
    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
        printf("Ошибка ввода!\n");
        exit(EXIT_FAILURE);
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    
    write(pipe1[1], buffer, strlen(buffer) + 1);
    close(pipe1[1]);
    
    char result[BUFFER_SIZE];
    ssize_t bytes_read = read(pipe3[0], result, BUFFER_SIZE - 1);
    
    if (bytes_read == -1) {
        perror("Ошибка чтения из pipe3");
        exit(EXIT_FAILURE);
    } else if (bytes_read == 0) {
        printf("pipe3 закрыт, данных нет\n");
    } else {
        result[bytes_read] = '\0';
        printf("Получен результат из child_2: '%s'\n", result);
    }
    
    close(pipe3[0]);
    
    waitpid(child_1, NULL, 0);
    waitpid(child_2, NULL, 0);
    
    printf("Программа завершена.\n");
    return 0;
}
