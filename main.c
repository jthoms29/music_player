#include <stdio.h>
#include <music_defs.h>

#define MAX_PATH 1024
char lib_path[1024];


int main(void) {
  printf("Please enter path to music library:\n  ");
  fgets(lib_path, MAX_PATH, stdin);
  printf("%s\n", lib_path);
  return 0;
}
