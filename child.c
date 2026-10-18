#include <unistd.h>
#include <fcntl.h>

int str_len(const char *str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

int str_to_int(const char *str, int *out_val) {
    int val = 0;
    int sign = 1;
    int i = 0;
    if (str[i] == '-') { sign = -1; i++; }
    for (; str[i] >= '0' && str[i] <= '9'; i++) {
        val = val * 10 + (str[i] - '0');
    }
    *out_val = val * sign;
    return i;
}

void int_to_str(int val, char *buf) {
    int i = 0;
    if (val == 0) { buf[i++] = '0'; buf[i] = '\0'; return; }
    if (val < 0) { buf[i++] = '-'; val = -val; }
    
    int start = i;
    while (val > 0) {
        buf[i++] = (val % 10) + '0';
        val /= 10;
    }
    buf[i] = '\0';
    
    int end = i - 1;
    while (start < end) {
        char temp = buf[start];
        buf[start] = buf[end];
        buf[end] = temp;
        start++; end--;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) _exit(1);

    const char *filename = argv[1];
    int pipe2_fd;
    str_to_int(argv[2], &pipe2_fd);

    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) _exit(1);

    char buffer[1024];
    int bytes_read;

    int first_number = 1;
    int result = 0;
    int current_num = 0;

    while ((bytes_read = read(0, buffer, sizeof(buffer))) > 0) {
        int i = 0;
        
        while (i < bytes_read) {
            if (buffer[i] == ' ') {
                i++;
                continue;
            }
            if (buffer[i] == '\n') {
                if (!first_number) { 
                    char out_buf[32];
                    int_to_str(result, out_buf);
                    write(file_fd, out_buf, str_len(out_buf));
                    write(file_fd, "\n", 1);
                }
                first_number = 1;
                result = 0;
                i++;
                continue;
            }
            int parsed_len = str_to_int(&buffer[i], &current_num);
            if (parsed_len > 0) {
                if (first_number) {
                    result = current_num;
                    first_number = 0;
                } else {
                    if (current_num == 0) {
                        write(pipe2_fd, "DIV_BY_ZERO", 11);
                        close(file_fd);
                        _exit(1); 
                    }
                    result /= current_num;
                }
                i += parsed_len;
            } else {
                i++; 
            }
        }
    }

    close(file_fd);
    return 0;
}
