#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>
#include <tag_c.h>
#include <dirent.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
int playing = 0;

/* TODO wip, shouldn't loop forever */
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



void read_tag(char* path) {
  TagLib_File *file;
  TagLib_Tag *tag;

  if((file = taglib_file_new(path)) == 0)
      return;
  if((tag = taglib_file_tag(file)) == 0)
      return;

  printf("%s\n", taglib_tag_title(tag));
  printf("%s\n", taglib_tag_artist(tag));
  printf("%s\n", taglib_tag_album(tag));
  printf("%d\n", taglib_tag_year(tag));
  printf("%d\n", taglib_tag_track(tag));
  printf("%s\n", taglib_tag_genre(tag));
}


void scan_folder(char* path) {
  struct dirent *de; /*pointer for directory entry */
  DIR *dir;           /* DIR pointer, what opendir returns */


  char* full;
  dir = opendir(path);

  if (!dir) {
    printf("Invalid path\n");
    return;
  }

  while((de = readdir(dir)) != NULL) {
    
    full = (char*) malloc(strlen(path) + strlen(de->d_name));
    strcpy(full, path);
    strcat(full, de->d_name);
    read_tag(full);
    miniaudio_init(full);
    free(full);
    full = NULL;
    }

  closedir(dir);
}

int main(int argc, char** argv) {
 char* path;
 path = (char*) malloc(strlen(argv[1]));
 strcpy(path, argv[1]);
 printf("%s\n", path);
 scan_folder(path);
// read_tag(path);
 //miniaudio_init(path);

}
