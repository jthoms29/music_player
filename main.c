#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>
//#include <glib.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>


/* The music library will be held in this array, which is indexed by the ascii
 * code of the first letter of the artist's name. Each contains a linked list
 * of artists, each of which contains a linked list of albums */

/* As of now, the starting letter of the artist can only be from ascii
 * characters A to z */
GList library[81];

ma_engine* engine;
int paused;


ma_result init_engine() {
  ma_result result;

  /* Allocating sound engine */
  engine = malloc(sizeof(ma_engine));
  result = ma_engine_init(NULL, engine);
  return result;
}

/* plays specified album, starting at specified track number */
int play_audio(album* albm, int start_track) {
  int total_tracks, i;
  int* track_ref;
  GList* song_walker;
  song* cur_song;
  song_walker = albm->songs;
  ma_result result;
  ma_sound sound;
  char command[256];

  paused = 0;
  track_ref = &start_track;
  song_walker = g_list_find_custom(albm->songs, (gconstpointer) track_ref,
    (GCompareFunc) find_track);

  if (!song_walker) {
    printf("Invalid track\n");
    return -1;
  }

  while (song_walker) {
    cur_song = (song*) song_walker->data;
    result = ma_sound_init_from_file(engine, cur_song->path, 0, NULL, NULL,
      &sound);
    if (result != MA_SUCCESS) {
      return result;
    }

//    ma_sound_set_stop_time_in_milliseconds(&sound, 1);
    ma_sound_start(&sound);

    while (!ma_sound_at_end(&sound)) {

      /* refresh line */
      printf("\r                                                           "
        "                                                                  ");
      printf("\r%s - %s: %d - %s    command:  ", albm->artist, albm->title,
        cur_song->track, cur_song->title);
      fflush(stdout);
      fgets(command, 256, stdin);

      command[strlen(command) - 1] = 0;
      if (strcmp(command, "next") == 0 || !strcmp(command, "prev")) {
        break;
      }
      if (!strcmp(command, "pause") && !paused) {
          ma_sound_stop(&sound);
          paused = 1;
      }

      if (!strcmp(command, "unpause") && paused) {
        ma_sound_start(&sound);
        paused = 0;
      }
    }

    if (!paused)
      ma_sound_stop(&sound);

    ma_sound_uninit(&sound);
    
    if (!strcmp(command, "next")) {
      paused = 0;
      song_walker = song_walker->next;

    }

    if (!strcmp(command, "prev"))
        song_walker = song_walker->prev;

  }

}

void server(void) {
  char message[256];
  fgets(message, 256, stdin);
  message[strlen(message)] = 0;



  if message = 



}

int main(int argc, char** argv) {
 char* path;
 path = (char*) malloc(strlen(argv[1]));
 strcpy(path, argv[1]);
 init_engine();
 scan_folder(path);
 cursor();

 return 0;
}
