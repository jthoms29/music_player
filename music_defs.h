/* John Thoms */

#ifndef MUSIC_DEFS_H
#define MUSIC_DEFS_H

#include <glib.h>
#define MAX_TITLE 256
#define MAX_PATH 512

#define LIB 0
#define CONT 1


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
  char title[MAX_TITLE];
  char artist[MAX_TITLE];
  char genre[MAX_TITLE];
  unsigned int year;
  unsigned int tracks;
  GList* songs;
} album;


typedef struct artist {
  char name[MAX_TITLE];
  GList* albums;
} artist;


void* play_audio(void* arg);
void free_lib();
/*library_operations */
song* read_tag(char* path);
int print_song_data(song* sng);
void scan_folder(char* path);
void lib_test(void);


/* cursor */
void* cursor(void* arg);
gint find_artist(gconstpointer list_artist, gconstpointer my_artist_str);
gint find_album(gconstpointer list_album, gconstpointer my_album_str);
gint find_track(gconstpointer alb_track, gconstpointer track_idx);
#endif
