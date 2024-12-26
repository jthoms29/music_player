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

ma_result init_engine() {
  ma_result result;

  /* Allocating sound engine */
  engine = malloc(sizeof(*engine));

  result = ma_engine_init(NULL, engine);
  return result;
}

/* plays specified album, starting at specified track number */
int play_audio(album* albm, int start_track) {
  int total_tracks, i;
  GList* song_walker;
  song* cur_song;
  song_walker = albm->songs;
  ma_result result;
  ma_sound sound;

  for (i = 0; i < start_track; i++) {
    song_walker = song_walker->next;
  }


  while (song_walker != NULL) {

    cur_song = (song*) song_walker->data;
    result = ma_sound_init_from_file(engine, cur_song->path, 0, NULL, NULL,
      &sound);
    if (result != MA_SUCCESS) {
      return result;
    }

    printf("\r%s - %s: %d - %s", albm->artist, albm->title,
      cur_song->track, cur_song->title);
    fflush(stdout);

    ma_sound_start(&sound);

    while (!ma_sound_at_end(&sound)) {};

    ma_sound_stop(&sound);
    ma_sound_uninit(&sound);
    
    song_walker = song_walker->next;

  }


}



int main(int argc, char** argv) {
 char* path;
 path = (char*) malloc(strlen(argv[1]));
 strcpy(path, argv[1]);
 printf("%s\n", path);

 init_engine();
 scan_folder(path);

 cursor();

 return 0;
}
