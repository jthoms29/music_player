#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>
//#include <glib.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>


/* The music library will be held in this array, which is indexed by the ascii
 * code of the first letter of the artist's name. Each contains a linked list
 * of artists */

/* As of now, the starting letter of the artist can only be from ascii
 * characters A to z */
GList library[81];


ma_result miniaudio_init(char* path) {
  ma_result result;
  ma_engine engine;
  ma_sound sound;


  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    return result;
  }

  printf("%s\n", path);
  result = ma_sound_init_from_file(&engine, path, 0, NULL, NULL, &sound);
  if (result != MA_SUCCESS) {
    return result;
  }

  ma_sound_start(&sound);

  sleep(4);
  ma_sound(uninit());
}



int main(int argc, char** argv) {
 char* path;
 path = (char*) malloc(strlen(argv[1]));
 strcpy(path, argv[1]);
 printf("%s\n", path);
 scan_folder(path);
 return 0;
}
