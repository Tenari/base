#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include "all.h"
#include "pthread_barrier.h"

global pthread_barrier_t macos_thread_barrier;

fn Barrier osBarrierAlloc(u64 count) {
  pthread_barrier_init(&macos_thread_barrier, NULL, count);
  Barrier result = {(u64)&macos_thread_barrier};
  return result;
}

fn void osBarrierRelease(Barrier barrier) {
  pthread_barrier_t* addr = (pthread_barrier_t*)barrier.a[0];
  pthread_barrier_destroy(addr);
}

fn void osBarrierWait(Barrier barrier) {
  pthread_barrier_t* addr = (pthread_barrier_t*)barrier.a[0];
  pthread_barrier_wait(addr);
}

// Time
fn u64 osTimeMicrosecondsNow() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return ((u64)ts.tv_sec * 1000000) + ((u64)ts.tv_nsec / 1000000);
}

fn void osSleepMicroseconds(u32 t) {
  usleep(t);
}

// Misc
fn void osDebugPrint(bool debug_mode, const char * format, ... ) {
  if (debug_mode) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
  }
}
