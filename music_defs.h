/* John Thoms */

#ifndef MUSIC_DEFS_H
#define MUSIC_DEFS_H

#include <glib.h>
#define MAX_TITLE 64
#define MAX_PATH 128

typedef struct song {
  char path[MAX_PATH];
  char title[MAX_TITLE];
  char artist[MAX_TITLE];
  char album[MAX_TITLE];
  char genre[MAX_TITLE];
  unsigned int year;
  unsigned int track;
  struct song* next_song;
} song;


typedef struct album {
  char* title;
  char* artist;
  char* release_date;
  char* length;
  unsigned int num_songs;
  song* songs;
} album;


typedef struct artist {
  char name[MAX_TITLE];
  GList* albums;
} artist;


/*library_operations */
song* read_tag(char* path);
int print_song_data(song* sng);
void scan_folder(char* path);

#endif
