#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>
//#include <glib.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

pthread_cond_t cursor_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t control_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t control_cmd_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t player_sleep = PTHREAD_COND_INITIALIZER;


pthread_mutex_t cursor_tex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t control_tex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t control_cmd_tex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t player_tex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t song_choice_tex = PTHREAD_MUTEX_INITIALIZER;
/* The music library will be held in this array, which is indexed by the ascii
 * code of the first letter of the artist's name. Each contains a linked list
 * of artists, each of which contains a linked list of albums */

/* As of now, the starting letter of the artist can only be from ascii
 * characters A to z */
GList library[81];

ma_engine* engine;
ma_sound sound;

GList* songs = 0;
int track_choice = 0;

char command[256];
int cmd_alert = 0;

int paused = 0;

ma_result init_engine() {
  ma_result result;

  /* Allocating sound engine */
  engine = malloc(sizeof(ma_engine));
  result = ma_engine_init(NULL, engine);
  return result;
}

int play_audio() {
  song* cur_song;
  ma_result result;
  /* needed for cond variable */
  pthread_mutex_lock(&player_tex);
  for(;;) {
    /* No songs currently, go to sleep */
    pthread_mutex_lock(&song_choice_tex);
    if (!songs) {
      printf("no songs\n");
      pthread_mutex_unlock(&song_choice_tex);
      pthread_cond_wait(&player_sleep, &player_tex);
      printf("i'm uyp");
    }
    else {
      pthread_mutex_unlock(&song_choice_tex);
    }
    printf("yep");
    cur_song = (song*) songs->data;
    result = ma_sound_init_from_file(engine, cur_song->path, 0, NULL, NULL,
      &sound);
    if (result != MA_SUCCESS) {
        printf("nope");
      return result;
    }

    ma_sound_start(&sound);

    while(!ma_sound_at_end(&sound)) {
      /* refresh line */
      printf("\r                                                           "
        "                                                                  ");
      printf("\r%s - %s: %d - %s    command:  ", cur_song->artist, 
        cur_song->album, cur_song->track, cur_song->title);
      fflush(stdout);

      while (!cmd_alert && !ma_sound_at_end(&sound)) {
        sleep(2);
        ma_sound_stop(&sound);
        ma_sound_start(&sound);
      }

      if (cmd_alert) {

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
        pthread_cond_signal(&control_cmd_sleep);
      }
    }

    
    ma_sound_stop(&sound);
    ma_sound_uninit(&sound);

    if (cmd_alert) {
     if (!strcmp(command, "prev")) {
        songs = songs->prev;
      }

      if (!strcmp(command, "next")) {
        paused = 0;
        songs = songs->next;
      }
      cmd_alert = 0;
      pthread_cond_signal(&control_cmd_sleep);
    }

    else {
      /* default functionality is to automatically play next song in album */
      songs = songs->next;
    }


  }
  pthread_mutex_unlock(&player_tex);
}

/* plays specified album, starting at specified track number */
int controls(album* albm, int start_track) {
  pthread_mutex_lock(&control_tex);
  /* this thread should immediately go to sleep, nothing currently playing
   * on startup */
  pthread_cond_wait(&control_sleep, &control_tex);
  for(;;) {
    fgets(command, 256, stdin);

    command[strlen(command) - 1] = 0;
    cmd_alert = 1;
    pthread_cond_wait(&control_cmd_sleep, &control_tex);
  }
  pthread_mutex_unlock(&control_tex);
}


int main(int argc, char** argv) {
  char* path;
  pthread_t thread_table[3];
  int i;
  path = (char*) malloc(strlen(argv[1]));
  strcpy(path, argv[1]);
  init_engine();
  scan_folder(path);
  pthread_create(thread_table+1, NULL, play_audio, NULL);
  pthread_create(thread_table+2, NULL, controls, NULL);
  pthread_create(thread_table, NULL, cursor, NULL);

  for(i=0; i < 3; i++) {
    pthread_join(thread_table[i], NULL);
  }

  return 0;
}
