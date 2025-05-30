#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// Original functions
static int (*orig_connect)(int, const struct sockaddr*, socklen_t) = NULL;
static ssize_t (*orig_send)(int, const void*, size_t, int) = NULL;
static ssize_t (*orig_recv)(int, void*, size_t, int) = NULL;
static struct hostent* (*orig_gethostbyname)(const char*) = NULL;

// Blocking start time
static struct timeval start_time;
static bool initialized = false;
static long blocking_duration = 10; // Default 10 secs

// Time initialization
__attribute__((constructor))
static void init() {
    gettimeofday(&start_time, NULL);
    initialized = true;

    // Read HOYO_TIMEOUT environment variable
    const char* env_timeout = getenv("HOYO_TIMEOUT");
    if (env_timeout) {
        char *endptr;
        long value = strtol(env_timeout, &endptr, 10);

        if (endptr != env_timeout && *endptr == '\0' && value > 0) {
            blocking_duration = value;
        } else {
            fprintf(stderr, "libnetblock: Invalid HOYO_TIMEOUT value. Using default %ld seconds\n", blocking_duration);
        }
    }
}

// Check if blocking period is over
static bool is_blocking_period_over() {
    if (!initialized) return false;

    struct timeval now;
    gettimeofday(&now, NULL);

    return (now.tv_sec - start_time.tv_sec) >= blocking_duration;
}

// Check socket type
static bool is_internet_socket(int fd) {
    int domain;
    socklen_t len = sizeof(domain);
    return getsockopt(fd, SOL_SOCKET, SO_DOMAIN, &domain, &len) == 0 &&
    (domain == AF_INET || domain == AF_INET6);
}

// Intercepted functions ======================================================

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (!orig_connect)
        orig_connect = dlsym(RTLD_NEXT, "connect");

    if (!is_blocking_period_over() && addr->sa_family != AF_UNIX) {
        errno = ENETUNREACH;  // Network is unreachable
        return -1;
    }

    return orig_connect(sockfd, addr, addrlen);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    if (!orig_send)
        orig_send = dlsym(RTLD_NEXT, "send");

    if (!is_blocking_period_over() && is_internet_socket(sockfd)) {
        errno = ENETDOWN;  // Network is down
        return -1;
    }

    return orig_send(sockfd, buf, len, flags);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    if (!orig_recv)
        orig_recv = dlsym(RTLD_NEXT, "recv");

    if (!is_blocking_period_over() && is_internet_socket(sockfd)) {
        errno = ENETDOWN;
        return -1;
    }

    return orig_recv(sockfd, buf, len, flags);
}

struct hostent *gethostbyname(const char *name) {
    if (!orig_gethostbyname)
        orig_gethostbyname = dlsym(RTLD_NEXT, "gethostbyname");

    if (!is_blocking_period_over()) {
        // Emulate DNS failure
        return NULL;
    }

    return orig_gethostbyname(name);
}
