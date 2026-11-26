#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>

#define SHM_SIZE 4096

struct SharedData {
    char buffer[1024];
    int  length;
    int  div_by_zero;
};

static int str_len(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int str_to_int(const char *s, int *out) {
    int val = 0, sign = 1, i = 0;
    if (s[i] == '-') { sign = -1; i++; }
    else if (s[i] == '+') i++;
    while (s[i] >= '0' && s[i] <= '9') { val = val * 10 + (s[i] - '0'); i++; }
    *out = val * sign;
    return i;
}

static void int_to_str(int v, char *buf) {
    int i = 0;
    if (v == 0) { buf[i++] = '0'; buf[i] = '\0'; return; }
    if (v < 0) { buf[i++] = '-'; v = -v; }
    int start = i;
    while (v > 0) { buf[i++] = '0' + (v % 10); v /= 10; }
    buf[i] = '\0';
    int end = i - 1;
    while (start < end) { char t = buf[start]; buf[start] = buf[end]; buf[end] = t; start++; end--; }
}

int main(int argc, char *argv[]) {
    if (argc < 5) _exit(1);

    const char *shm_name  = argv[1];
    const char *sem_empty_name = argv[2];
    const char *sem_full_name  = argv[3];
    const char *filename  = argv[4];

    int shm_fd = shm_open(shm_name, O_RDWR, 0600);
    if (shm_fd < 0) _exit(1);

    struct SharedData *shared = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) _exit(1);
    close(shm_fd);

    sem_t *sem_empty = sem_open(sem_empty_name, 0);
    sem_t *sem_full  = sem_open(sem_full_name, 0);
    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED) _exit(1);

    int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) _exit(1);

    while (1) {
        sem_wait(sem_full);
        int len = shared->length;
        if (len == 0) { sem_post(sem_empty); break; }

        int i = 0;
        int first_in_expr = 1;
        int result = 0;

        while (i < len) {
            while (i < len && (shared->buffer[i] == ' ' || shared->buffer[i] == '\t')) i++;
            if (i >= len) break;

            if (shared->buffer[i] == '\n') {
                if (!first_in_expr) {
                    char out[32];
                    int_to_str(result, out);
                    ssize_t w;
                    do { w = write(file_fd, out, str_len(out)); } while (w < 0 && errno == EINTR);
                    do { w = write(file_fd, "\n", 1); } while (w < 0 && errno == EINTR);
                }
                first_in_expr = 1;
                result = 0;
                i++;
                continue;
            }

            int num = 0;
            int parsed = str_to_int(&shared->buffer[i], &num);
            if (parsed > 0) {
                if (first_in_expr) { result = num; first_in_expr = 0; }
                else {
                    if (num == 0) {
                        shared->div_by_zero = 1;
                        sem_post(sem_empty);
                        close(file_fd);
                        munmap(shared, SHM_SIZE);
                        sem_close(sem_empty);
                        sem_close(sem_full);
                        _exit(1);
                    }
                    result /= num;
                }
                i += parsed;
            } else {
                i++;
            }
        }

        if (!first_in_expr) {
            char out[32];
            int_to_str(result, out);
            ssize_t w;
            do { w = write(file_fd, out, str_len(out)); } while (w < 0 && errno == EINTR);
            do { w = write(file_fd, "\n", 1); } while (w < 0 && errno == EINTR);
        }

        sem_post(sem_empty);
    }

    close(file_fd);
    munmap(shared, SHM_SIZE);
    sem_close(sem_empty);
    sem_close(sem_full);
    return 0;
}