#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>
//#include <glib.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>


pthread_mutex_t cursor_sleep = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t control_sleep = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t control_cmd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t player_sleep = PTHREAD_MUTEX_INITIALIZER;
/* The music library will be held in this array, which is indexed by the ascii
 * code of the first letter of the artist's name. Each contains a linked list
 * of artists, each of which contains a linked list of albums */

/* As of now, the starting letter of the artist can only be from ascii
 * characters A to z */
GList library[81];

ma_engine* engine;
ma_sound sound;
char command[256];
int cmd_alert;

int paused;

ma_result init_engine() {
  ma_result result;

  /* Allocating sound engine */
  engine = malloc(sizeof(ma_engine));
  result = ma_engine_init(NULL, engine);
  return result;
}

int play_audio(GList* songs) {
  ma_result result;
  song* cur_song;

  while(songs != NULL) {
    cur_song = (song*) songs->data;
    result = ma_sound_init_from_file(engine, cur_song->path, 0, NULL, NULL,
      &sound);
    if (result != MA_SUCCESS) {
      return result;
    }

    while(!ma_sound_at_end(&sound)) {
      /* refresh line */
      printf("\r                                                           "
        "                                                                  ");
      printf("\r%s - %s: %d - %s    command:  ", cur_song->artist, 
        cur_song->album, cur_song->track, cur_song->title);
      fflush(stdout);

      if(cmd_alert) {

        if (strcmp(command, "next") == 0 || !strcmp(command, "prev")) {
          break;
        }

        if (!strcmp(command, "pause") && !paused) {
          ma_sound_stop(&sound);
          paused = 1;
        }

        if (!strcmp(command, "unpause") && paused) {
          paused = 0;
          ma_sound_start(&sound);
        }

        cmd_alert = 0;
        pthread_mutex_unlock(&control_cmd);
      }
    }

    if (cmd_alert) {
      if (!strcmp(command, "prev")) {
        songs = songs->prev;
      }

      if (!strcmp(command, "next")) {
        paused = 0;
        songs = songs->next;
      }
      cmd_alert = 0;
      pthread_mutex_unlock(&control_cmd);
    }

    else {
      /* default functionality is to automatically play next song in album */
      songs = songs->next;
    }

    ma_sound_stop(&sound);
    ma_sound_uninit(&sound);

  }
}

/* plays specified album, starting at specified track number */
int controls(album* albm, int start_track) {

  for(;;) {
    fgets(command, 256, stdin);

    command[strlen(command) - 1] = 0;
    cmd_alert = 1;
    pthread_mutex_lock(&control_cmd);
  }
}


int main(int argc, char** argv) {
  char* path;
  pthread_t thread_table[3];
  int i;

  path = (char*) malloc(strlen(argv[1]));
  strcpy(path, argv[1]);
  init_engine();
  scan_folder(path);
  pthread_create(thread_table, NULL, cursor, NULL);
  pthread_create(thread_table+1, NULL, play_audio, NULL);
  pthread_create(thread_table+2, NULL, controls, NULL);

  for(i=0; i < 3; i++) {
    pthread_join(thread_table[i], NULL);
  }

  return 0;
}
