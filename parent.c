#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

int str_len(const char *str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

void print_str(const char *str) {
    write(1, str, str_len(str));
}

int main() {
    char filename[256];
    char buffer[1024];
    int pipe1[2]; 
    int pipe2[2]; 

    print_str("Введите имя файла: ");
    
    int bytes_read = read(0, filename, sizeof(filename) - 1);
    if (bytes_read <= 0) {
        print_str("Ошибка чтения имени файла\n");
        return 1;
    }
    
    for (int i = 0; i < bytes_read; i++) {
        if (filename[i] == '\n') {
            filename[i] = '\0';
            break;
        }
    }

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        print_str("Ошибка создания каналов\n");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        print_str("Ошибка fork\n");
        return 1;
    }

    if (pid == 0) {
        close(pipe1[1]); 
        close(pipe2[0]); 

        dup2(pipe1[0], 0);
        close(pipe1[0]);

        char pipe2_fd_str[10];
        int fd = pipe2[1], idx = 0;
        if (fd == 0) pipe2_fd_str[idx++] = '0';
        while (fd > 0) { pipe2_fd_str[idx++] = (fd % 10) + '0'; fd /= 10; }
        pipe2_fd_str[idx] = '\0';
        
        for (int i = 0; i < idx / 2; i++) {
            char temp = pipe2_fd_str[i];
            pipe2_fd_str[i] = pipe2_fd_str[idx - 1 - i];
            pipe2_fd_str[idx - 1 - i] = temp;
        }

        char *args[] = {"./child", filename, pipe2_fd_str, NULL};
        char *envp[] = {NULL};

        execve("./child", args, envp);
        
        print_str("Ошибка execve (файл child не найден)\n");
        _exit(1);
    } else {
        close(pipe1[0]); 
        close(pipe2[1]); 

        print_str("Введите числа (через пробел): \n");

        while (1) {
            int status;
            if (waitpid(pid, &status, WNOHANG) == pid) {
                print_str("\nДочерний процесс завершен (деление на 0).\n");
                break;
            }

            bytes_read = read(0, buffer, sizeof(buffer));
            if (bytes_read <= 0) break; 
            int written = write(pipe1[1], buffer, bytes_read);
            if (written < 0) { 
                print_str("\n[!] Канал разорван. Дочерний процесс завершен (деление на 0).\n");
                break;
            }
            usleep(1000); 
            if (waitpid(pid, &status, WNOHANG) == pid) {
                print_str("\n[!] Дочерний процесс завершен (деление на 0).\n");
                break;
            }
        }

        close(pipe1[1]);
        close(pipe2[0]);
        waitpid(pid, NULL, 0); 
    }

    return 0;
}
