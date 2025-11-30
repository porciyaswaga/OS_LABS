#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main() 
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        
        for (int i = 0; buffer[i] != '\0'; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        
        write(STDOUT_FILENO, buffer, strlen(buffer) + 1);
    }
    
    return 0;
}
