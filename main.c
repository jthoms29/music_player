#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

ma_result miniaudio_init(char* path) {
  ma_result result;
  ma_engine engine;

  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    return result;
  }


  ma_engine_play_sound(&engine, path, NULL);
  for (;;) {}


}

int main(int argc, char** argv) {
 char path[256];
 strcpy(path, argv[1]);
 printf("%s\n", path);
 miniaudio_init(path);

}
