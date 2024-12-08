#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>
#include <tag_c.h>


#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>


/* TODO wip, shouldn't loop forever */
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

void read_tag(char* path) {
  TagLib_File *file;
  TagLib_Tag *tag;

  file = taglib_file_new(path);
  tag = taglib_file_tag(file);

  printf("%s\n", taglib_tag_title(tag));

  printf("%s\n", taglib_tag_artist(tag));

  printf("%s\n", taglib_tag_album(tag));
  printf("%d\n", taglib_tag_year(tag));
  printf("%d\n", taglib_tag_track(tag));
  printf("%s\n", taglib_tag_genre(tag));
}

int main(int argc, char** argv) {
 char path[256];
 strcpy(path, argv[1]);
 printf("%s\n", path);
 read_tag(path);
 miniaudio_init(path);

}
