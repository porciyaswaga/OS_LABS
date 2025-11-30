#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main() {
    char buffer[BUFFER_SIZE];
    char result[BUFFER_SIZE];
    ssize_t bytes_read;
    
    bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        
        int j = 0;
        int prev_space = 0;
        
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (isspace(buffer[i])) {
                if (!prev_space) {
                    result[j++] = ' ';
                    prev_space = 1;
                }
            } else {
                result[j++] = buffer[i];
                prev_space = 0;
            }
        }
        result[j] = '\0';
        
        write(STDOUT_FILENO, result, strlen(result) + 1);
    }
    
    return 0;
}
