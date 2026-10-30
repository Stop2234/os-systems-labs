#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

static volatile sig_atomic_t acked = 0;
static volatile sig_atomic_t timed_out = 0;

static void on_ack(int sig) {
    (void)sig;
    acked = 1;
}

static void on_alarm(int sig) {
    (void)sig;
    timed_out = 1;
}

static void send_bit_with_ack(pid_t recv_pid, int bit) {
    sigset_t block_set, prev_set, suspend_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1); 
    sigaddset(&block_set, SIGALRM); 
    if (sigprocmask(SIG_BLOCK, &block_set, &prev_set) < 0) {
        perror("sigprocmask");
        exit(2);
    }

    acked = 0;
    timed_out = 0;

    alarm(2);

    int sig = bit ? SIGUSR2 : SIGUSR1;
    if (kill(recv_pid, sig) < 0) {
        perror("kill(data)");
        exit(2);
    }

    suspend_set = prev_set;
    sigdelset(&suspend_set, SIGUSR1);
    sigdelset(&suspend_set, SIGALRM);

    while (!acked && !timed_out) {
        sigsuspend(&suspend_set);
    }

    alarm(0);

    if (sigprocmask(SIG_SETMASK, &prev_set, NULL) < 0) {
        perror("sigprocmask restore");
        exit(2);
    }

    if (timed_out) {
        fprintf(stderr, "ERROR: no ACK from receiver (timeout)\n");
        exit(3);
    }
}


int main(void) {
    printf("SENDER PID = %d\n", (int)getpid());
    printf("Enter RECEIVER PID: ");
    fflush(stdout);

    pid_t recv_pid;
    if (scanf("%d", (int*)&recv_pid) != 1 || recv_pid <= 0) {
        fprintf(stderr, "Bad receiver PID\n");
        return 1;
    }

    printf("Enter integer (fits int32): ");
    fflush(stdout);

    long long x;
    if (scanf("%lld", &x) != 1) {
        fprintf(stderr, "Bad number\n");
        return 1;
    }
    if (x < INT32_MIN || x > INT32_MAX) {
        fprintf(stderr, "Out of int32 range\n");
        return 1;
    }

    int32_t num = (int32_t)x;
    uint32_t u = (uint32_t)num; 

    struct sigaction sa_ack;
    sa_ack.sa_handler = on_ack;
    sigemptyset(&sa_ack.sa_mask);
    sa_ack.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa_ack, NULL) < 0) { perror("sigaction ACK"); return 1; }

    struct sigaction sa_al;
    sa_al.sa_handler = on_alarm;
    sigemptyset(&sa_al.sa_mask);
    sa_al.sa_flags = SA_RESTART;
    if (sigaction(SIGALRM, &sa_al, NULL) < 0) { perror("sigaction ALRM"); return 1; }

    for (int i = 31; i >= 0; --i) {
        int bit = (u >> i) & 1u;
        send_bit_with_ack(recv_pid, bit);
    }

    if (kill(recv_pid, SIGINT) < 0) {
        perror("kill(SIGINT finish)");
        return 2;
    }

    printf("Done.\n");
    return 0;
}
