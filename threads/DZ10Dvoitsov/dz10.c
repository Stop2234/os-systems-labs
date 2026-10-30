#define _XOPEN_SOURCE 700
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdint.h>

enum { N_PRODUCERS = 100, BUF_CAP = 10000, MAX_ADDERS = 16 };

typedef struct {
    int data[BUF_CAP];
    int head, tail, size;
} queue_t;

static queue_t q;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cv  = PTHREAD_COND_INITIALIZER;

static int producers_done = 0;
static int active_adders  = 0;
static int stop_flag      = 0;
static long long final_result = 0;

static struct timespec t0;

static long long ms_since_start(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec - t0.tv_sec) * 1000LL + (t.tv_nsec - t0.tv_nsec) / 1000000LL;
}

static void tlog(const char *who, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stdout, "[%6lld ms] %-10s | ", ms_since_start(), who);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);
    va_end(ap);
}

static int rnd_range(unsigned *seed, int lo, int hi) {
    return lo + (int)(rand_r(seed) % (unsigned)(hi - lo + 1));
}

static void q_push(int v) {
    if (q.size >= BUF_CAP) {
        fprintf(stderr, "Buffer overflow\n");
        exit(1);
    }
    q.data[q.tail] = v;
    q.tail = (q.tail + 1) % BUF_CAP;
    q.size++;
}

static int q_pop(void) {
    if (q.size <= 0) {
        fprintf(stderr, "Buffer underflow\n");
        exit(1);
    }
    int v = q.data[q.head];
    q.head = (q.head + 1) % BUF_CAP;
    q.size--;
    return v;
}

static void try_finalize_locked(const char *by) {
    if (!stop_flag &&
        producers_done == N_PRODUCERS &&
        active_adders == 0 &&
        q.size == 1)
    {
        final_result = q.data[q.head];
        stop_flag = 1;
        tlog("SYSTEM", "FINAL READY by %s: result=%lld (buf=%d, adders=%d)",
             by, final_result, q.size, active_adders);
        pthread_cond_broadcast(&cv);
    }
}

typedef struct {
    int a, b;
    int id;
    unsigned seed;
} adder_args_t;

static void* adder_thread(void *arg) {
    adder_args_t *p = (adder_args_t*)arg;
    char who[32];
    snprintf(who, sizeof(who), "ADDER#%d", p->id);

    int d = rnd_range(&p->seed, 3, 6);
    tlog(who, "start sum %d + %d (sleep %ds)", p->a, p->b, d);
    sleep((unsigned)d);

    int s = p->a + p->b;

    pthread_mutex_lock(&mtx);
    q_push(s);
    active_adders--;
    tlog(who, "done %d + %d = %d -> pushed back | buf=%d adders=%d doneProd=%d",
         p->a, p->b, s, q.size, active_adders, producers_done);

    try_finalize_locked("adder");
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mtx);

    free(p);
    return NULL;
}

typedef struct {
    int id;
    unsigned seed;
} producer_args_t;

static void* producer_thread(void *arg) {
    producer_args_t *p = (producer_args_t*)arg;
    char who[32];
    snprintf(who, sizeof(who), "PROD#%03d", p->id);

    int d = rnd_range(&p->seed, 1, 7);
    sleep((unsigned)d);

    int v = rnd_range(&p->seed, 1, 100);

    pthread_mutex_lock(&mtx);
    q_push(v);
    tlog(who, "produced %d (sleep %ds) -> buf=%d", v, d, q.size);
    pthread_cond_broadcast(&cv);

    producers_done++;
    tlog(who, "finished -> doneProd=%d/%d", producers_done, N_PRODUCERS);

    try_finalize_locked("producer");
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mtx);

    free(p);
    return NULL;
}

static void* dispatcher_thread(void *arg) {
    (void)arg;
    unsigned seed = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)pthread_self();
    int adder_id = 1;

    tlog("DISPATCH", "started (MAX_ADDERS=%d)", MAX_ADDERS);

    while (1) {
        pthread_mutex_lock(&mtx);

        while (!stop_flag && (q.size < 2 || active_adders >= MAX_ADDERS)) {
            try_finalize_locked("dispatcher_wait");
            if (stop_flag) break;
            pthread_cond_wait(&cv, &mtx);
        }

        if (stop_flag) {
            pthread_mutex_unlock(&mtx);
            break;
        }

        int x = q_pop();
        int y = q_pop();

        adder_args_t *aa = (adder_args_t*)calloc(1, sizeof(*aa));
        aa->a = x; aa->b = y; aa->id = adder_id++;
        aa->seed = seed ^ (unsigned)(aa->id * 2654435761u);

        active_adders++;
        tlog("DISPATCH", "took (%d,%d) -> spawn ADDER#%d | buf=%d adders=%d",
             x, y, aa->id, q.size, active_adders);

        pthread_t th;
        int rc = pthread_create(&th, NULL, adder_thread, aa);
        if (rc != 0) {
            fprintf(stderr, "pthread_create(adder) failed: %d\n", rc);
            exit(2);
        }
        pthread_detach(th);

        pthread_mutex_unlock(&mtx);
    }

    tlog("DISPATCH", "exit");
    return NULL;
}

int main(void) {
    clock_gettime(CLOCK_MONOTONIC, &t0);

    pthread_t disp;
    if (pthread_create(&disp, NULL, dispatcher_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create dispatcher\n");
        return 1;
    }

    pthread_t prod[N_PRODUCERS];
    for (int i = 0; i < N_PRODUCERS; i++) {
        producer_args_t *pa = (producer_args_t*)calloc(1, sizeof(*pa));
        pa->id = i + 1;
        pa->seed = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)pthread_self() ^ (unsigned)(i * 1103515245u);

        if (pthread_create(&prod[i], NULL, producer_thread, pa) != 0) {
            fprintf(stderr, "Failed to create producer %d\n", i + 1);
            return 1;
        }
    }

    for (int i = 0; i < N_PRODUCERS; i++) pthread_join(prod[i], NULL);
    pthread_join(disp, NULL);

    printf("\n=== FINAL RESULT: %lld ===\n", final_result);
    return 0;
}
