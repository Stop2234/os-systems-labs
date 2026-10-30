#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static volatile sig_atomic_t bit_count = 0;
static volatile sig_atomic_t done = 0;

static uint32_t acc = 0;       
static pid_t sender_pid = -1;   

static void on_bit(int sig) {

    int bit = (sig == SIGUSR2) ? 1 : 0;

    if (bit_count < 32) {
        acc = (acc << 1) | (uint32_t)bit;
        bit_count++;
    }

    if (sender_pid > 0) {
        (void)kill(sender_pid, SIGUSR1); 
    }
}

static void on_finish(int sig) {
    (void)sig;
    done = 1;
}

int main(void) {
    printf("RECEIVER PID = %d\n", (int)getpid());
    printf("Enter SENDER PID: ");
    fflush(stdout);

    if (scanf("%d", (int*)&sender_pid) != 1 || sender_pid <= 0) {
        fprintf(stderr, "Bad sender PID\n");
        return 1;
    }

    struct sigaction sa_bit;
    sa_bit.sa_handler = on_bit;
    sigemptyset(&sa_bit.sa_mask);
    sa_bit.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa_bit, NULL) < 0) { perror("sigaction SIGUSR1"); return 1; }
    if (sigaction(SIGUSR2, &sa_bit, NULL) < 0) { perror("sigaction SIGUSR2"); return 1; }

    struct sigaction sa_fin;
    sa_fin.sa_handler = on_finish;
    sigemptyset(&sa_fin.sa_mask);
    sa_fin.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa_fin, NULL) < 0) { perror("sigaction SIGINT"); return 1; }

    printf("Waiting bits... (finish by SIGINT from sender)\n");
    fflush(stdout);

    while (!done) {
        pause();
    }

    if (bit_count != 32) {
        fprintf(stderr, "ERROR: received %d bits instead of 32\n", (int)bit_count);
        return 2;
    }

    int32_t result = (int32_t)acc; 
    printf("Received number: %d\n", (int)result);
    return 0;
}
