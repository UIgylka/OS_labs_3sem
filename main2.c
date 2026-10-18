#include <stdlib.h> 
#include <pthread.h> 
#include <sys/time.h> 
#include <unistd.h> 

int str_len(const char *str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

void print_str(const char *str) {
    write(1, str, str_len(str));
}

void ll_to_str(long long val, char *buf) {
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

void double_to_str(double val, char *buf) {
    long long int_part = (long long)val;
    long long frac_part = (long long)((val - int_part) * 100); 
    if (frac_part < 0) frac_part = -frac_part;
    char int_buf[32];
    ll_to_str(int_part, int_buf);
    int i = 0, j = 0;
    while (int_buf[j] != '\0') buf[i++] = int_buf[j++];
    buf[i++] = '.';
    buf[i++] = (frac_part / 10) + '0';
    buf[i++] = (frac_part % 10) + '0';
    buf[i] = '\0';
}

int str_to_int(const char *str) {
    int val = 0;
    for (int i = 0; str[i] >= '0' && str[i] <= '9'; i++) {
        val = val * 10 + (str[i] - '0');
    }
    return val;
}

typedef struct {
    int *array; 
    int left; 
    int right; 
    int current_threads; 
    int max_threads; 
} ThreadArgs;

void merge(int *arr, int l, int m, int r) {
    int i, j, k;
    int n1 = m - l + 1; 
    int n2 = r - m;
    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));
    for (i = 0; i < n1; i++) L[i] = arr[l + i];
    for (j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

    i = 0; j = 0; k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) { arr[k] = L[i]; i++; k++; }
    while (j < n2) { arr[k] = R[j]; j++; k++; }

    free(L);
    free(R);
}

void merge_sort_sequential(int *arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        merge_sort_sequential(arr, l, m);
        merge_sort_sequential(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

void *merge_sort_parallel(void *args) {
    ThreadArgs *targs = (ThreadArgs *)args; 
    int l = targs->left;
    int r = targs->right;

    if (l < r) {
        int m = l + (r - l) / 2;
        if (targs->current_threads * 2 <= targs->max_threads) {
            pthread_t thread1, thread2;
            ThreadArgs args1 = {targs->array, l, m, targs->current_threads * 2, targs->max_threads};
            ThreadArgs args2 = {targs->array, m + 1, r, targs->current_threads * 2 + 1, targs->max_threads};

            pthread_create(&thread1, NULL, merge_sort_parallel, &args1);
            pthread_create(&thread2, NULL, merge_sort_parallel, &args2);
            pthread_join(thread1, NULL);
            pthread_join(thread2, NULL);
        } else {
            merge_sort_sequential(targs->array, l, m);
            merge_sort_sequential(targs->array, m + 1, r);
        }
        merge(targs->array, l, m, r);
    }
    return NULL;
}

long long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_str("Ошибка: укажите количество потоков при запуске!\n");
        return 1;
    }
    int max_threads = str_to_int(argv[1]);
    int n = 5000000; 
    int *arr_seq = (int *)malloc(n * sizeof(int));
    int *arr_par = (int *)malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        int val = rand() % 100000;
        arr_seq[i] = val;
        arr_par[i] = val; 
    }

    long long start_seq = get_time_ms();
    merge_sort_sequential(arr_seq, 0, n - 1);
    long long end_seq = get_time_ms();
    long long time_seq = end_seq - start_seq;

    long long start_par = get_time_ms();
    ThreadArgs initial_args = {arr_par, 0, n - 1, 1, max_threads};
    merge_sort_parallel(&initial_args); // Запуск главного потока
    long long end_par = get_time_ms();
    long long time_par = end_par - start_par;

    double speedup = (double)time_seq / time_par; 
    double efficiency = speedup / max_threads;

    char buf[64];
    
    print_str("\nelements:  ");
    ll_to_str(n, buf);
    print_str(buf);
    print_str(" | flows ");
    ll_to_str(max_threads, buf);
    print_str(buf);
    print_str("\n------------------------------------\n");


    print_str("consistent time: ");
    ll_to_str(time_seq, buf);
    print_str(buf);
    print_str("\n------------------------------------\n");

    print_str("Parallel time: ");
    ll_to_str(time_par, buf);
    print_str(buf);
    print_str("\n------------------------------------\n");

    print_str("boost: ");
    double_to_str(speedup, buf);
    print_str(buf);
    print_str("\n");
    print_str("\n------------------------------------\n");

    print_str("Effeciency: ");
    double_to_str(efficiency, buf);
    print_str(buf);
    print_str("\n\n");
    free(arr_seq);
    free(arr_par);
    
    return 0;
}
