#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
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

static void print_str(const char *s) {
    int n = str_len(s);
    ssize_t w;
    do { w = write(STDOUT_FILENO, s, n); } while (w < 0 && errno == EINTR);
}

int main(void) {
    char filename[256];
    char line[1024];

    print_str("Enter file name: ");
    ssize_t n = read(STDIN_FILENO, filename, sizeof(filename) - 1);
    if (n <= 0) { print_str("Error reading file name\n"); return 1; }
    filename[n] = '\0';
    for (int i = 0; i < n; i++) if (filename[i] == '\n') { filename[i] = '\0'; break; }

    pid_t pid = getpid();
    char shm_name[64], sem_empty_name[64], sem_full_name[64];
    int j = 0;
    
    const char *p1 = "/shm_";
    j = 0;
    for (int i = 0; p1[i]; i++) shm_name[j++] = p1[i];
    {
        char num[16]; int k = 0, t = pid;
        if (t == 0) num[k++] = '0';
        while (t > 0) { num[k++] = '0' + (t % 10); t /= 10; }
        for (int i = k - 1; i >= 0; i--) shm_name[j++] = num[i];
    }
    shm_name[j] = '\0';

    const char *p2 = "/sem_empty_";
    j = 0;
    for (int i = 0; p2[i]; i++) sem_empty_name[j++] = p2[i];
    {
        char num[16]; int k = 0, t = pid;
        if (t == 0) num[k++] = '0';
        while (t > 0) { num[k++] = '0' + (t % 10); t /= 10; }
        for (int i = k - 1; i >= 0; i--) sem_empty_name[j++] = num[i];
    }
    sem_empty_name[j] = '\0';

    const char *p3 = "/sem_full_";
    j = 0;
    for (int i = 0; p3[i]; i++) sem_full_name[j++] = p3[i];
    {
        char num[16]; int k = 0, t = pid;
        if (t == 0) num[k++] = '0';
        while (t > 0) { num[k++] = '0' + (t % 10); t /= 10; }
        for (int i = k - 1; i >= 0; i--) sem_full_name[j++] = num[i];
    }
    sem_full_name[j] = '\0';

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0600);
    if (shm_fd < 0) { print_str("Error shm_open\n"); return 1; }
    if (ftruncate(shm_fd, SHM_SIZE) < 0) { print_str("Error ftruncate\n"); return 1; }

    struct SharedData *shared = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) { print_str("Error mmap\n"); return 1; }
    shared->length = 0;
    shared->div_by_zero = 0;

    sem_t *sem_empty = sem_open(sem_empty_name, O_CREAT, 0600, 1);
    sem_t *sem_full  = sem_open(sem_full_name,  O_CREAT, 0600, 0);
    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        print_str("Error sem_open\n"); return 1;
    }

    pid_t child = fork();
    if (child < 0) { print_str("Error fork\n"); return 1; }

    if (child == 0) {
        char *args[] = {"./child", shm_name, sem_empty_name, sem_full_name, filename, NULL};
        char *envp[] = {NULL};
        execve("./child", args, envp);
        print_str("Error execve\n");
        _exit(1);
    }

    print_str("Enter numbers (space separated, new line for new expression):\n");
    while (1) {
        int wstatus;
        if (waitpid(child, &wstatus, WNOHANG) == child) {
            if (shared->div_by_zero)
                print_str("\nChild process stopped: division by 0\n");
            break;
        }

        n = read(STDIN_FILENO, line, sizeof(line));
        if (n <= 0) {
            sem_wait(sem_empty);
            shared->length = 0;
            sem_post(sem_full);
            break;
        }

        sem_wait(sem_empty);
        int copy = (int)n;
        if (copy > (int)sizeof(shared->buffer)) copy = sizeof(shared->buffer);
        for (int i = 0; i < copy; i++) shared->buffer[i] = line[i];
        shared->length = copy;
        sem_post(sem_full);
    }

    waitpid(child, NULL, 0);

    munmap(shared, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_empty);
    sem_close(sem_full);
    shm_unlink(shm_name);
    sem_unlink(sem_empty_name);
    sem_unlink(sem_full_name);

    return 0;
}