#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>

#define MAX_PATH 1024
char lib_path[1024];


/* Reads library info (path, etc) from config file */
int read_config(void) {
  printf("read_config has been reached\n");
}

int main(void) {
  int thread_ret;
  pthread_t rd_cfg;

  

  printf("Please enter path to music library:\n  ");
  fgets(lib_path, MAX_PATH, stdin);
  printf("%s\n", lib_path);

  thread_ret = pthread_create(&rd_cfg, NULL, (void* (*) (void*)) read_config, 
    NULL);

  if (thread_ret != 0) {
    printf("Thread error\n");
  }
  return 0;
}
