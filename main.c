#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>
//#include <glib.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

int current_focus; /* LIB (library browser) or CONT (music controls) */

pthread_mutex_t song_check_tex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t play_cmd_tex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lib_cmd_tex = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t player_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t control_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t lib_sleep = PTHREAD_COND_INITIALIZER;
/* The music library will be held in this array, which is indexed by the ascii
 * code of the first letter of the artist's name. Each contains a linked list
 * of artists, each of which contains a linked list of albums */

/* As of now, the starting letter of the artist can only be from ascii
 * characters a to z */
GList library[27];


pthread_t thread_table[3];

ma_engine* engine;
ma_sound sound;

GList* songs = NULL;

char command[256];

/* used for condition variable sync */
int play_cmd = 0;
int lib_cmd = 0;

int new_song = 0;
int first_song = 0;

ma_result init_engine() {
  ma_result result;

  /* Allocating sound engine */
  engine = malloc(sizeof(ma_engine));
  result = ma_engine_init(NULL, engine);
  return result;
} 


void end_check() {
  while (!ma_sound_at_end(&sound)) {}
  pthread_mutex_lock(&play_cmd_tex);
  strcpy(command, "next");
  play_cmd = 1;
  pthread_cond_signal(&player_sleep);
  if (play_cmd) {
    pthread_cond_wait(&control_sleep, &play_cmd_tex);
  }
  pthread_mutex_unlock(&play_cmd_tex);
}


void play_audio() {
  song* cur_song;
  ma_result result;
  pthread_t check;
  int paused = 0;

    pthread_mutex_lock(&play_cmd_tex);
  for(;;) {
    /* No songs currently, go to sleep */
    pthread_mutex_lock(&song_check_tex);
    while (!songs) {
      pthread_cond_wait(&player_sleep, &song_check_tex);
      if (play_cmd && !strcmp(command, ":exit")) {
        pthread_exit(0);
      }
    }
    pthread_mutex_unlock(&song_check_tex);

    cur_song = (song*) songs->data;
    result = ma_sound_init_from_file(engine, cur_song->path, 0, NULL, NULL,
      &sound); 

    if (result != MA_SUCCESS) {
      printf("uh oh");
      pthread_exit(0);
    }

    ma_sound_start(&sound);
    pthread_create(&check, NULL, end_check, NULL);
    while(!ma_sound_at_end(&sound)) {
      /* refresh line */

      if (current_focus == CONT) {

        printf("%s - %s: %d - %s    command:  ", cur_song->artist, 
          cur_song->album, cur_song->track, cur_song->title);
        fflush(stdout);
      }

//    while (!ma_sound_at_end(&sound) && !play_cmd) {}
         
      pthread_cond_wait(&player_sleep, &play_cmd_tex);
      if (play_cmd) {

        if (!strcmp(command, "refresh")) {
          break;
        }

        else if (!strcmp(command, "next")) {
          songs = songs->next;
          paused = 0;
          break;
        }

        else if (!strcmp(command, "prev")) {
          songs = songs->prev;
          paused = 0;
          break;
        }

        else if (!strcmp(command, "pause") && !paused) {
          ma_sound_stop(&sound);
          paused = 1;
        }

        else if (!strcmp(command, "unpause") && paused) {
          paused = 0;
          ma_sound_start(&sound);
        }

        else if (!strcmp(command, ":exit")) {
          ma_sound_stop(&sound);
          ma_sound_uninit(&sound);
          free(engine);
          pthread_exit(0);
        }

   //     pthread_mutex_lock(&play_cmd_tex);
        play_cmd = 0;
        pthread_cond_signal(&control_sleep);
  //      pthread_mutex_unlock(&play_cmd_tex);
      }
    }
    
    ma_sound_stop(&sound);
    ma_sound_uninit(&sound);
   // sound = NULL;

    if (play_cmd) {
//      pthread_mutex_lock(&play_cmd_tex);
      play_cmd = 0;
      pthread_cond_signal(&control_sleep);
 //     pthread_mutex_unlock(&play_cmd_tex);
    }

    else {
      songs = songs->next;
      if (current_focus == CONT) printf("\n");
    }
  }
}




/* plays specified album, starting at specified track number */
void controls(album* albm, int start_track) {
  current_focus = LIB;
  for(;;) {

    if (current_focus == CONT && songs == NULL) {
      printf("Nothing currently playing\n");
    }

    else if (first_song) {
      current_focus = CONT;
      first_song = 0;
      pthread_cond_signal(&player_sleep);
      continue;
    }

    else if (new_song) { 
      current_focus = CONT;
      new_song = 0;
      strcpy(command, "refresh");

      pthread_mutex_lock(&play_cmd_tex);
        play_cmd = 1;

        if (play_cmd) {
          pthread_cond_wait(&control_sleep, &play_cmd_tex);
        }

      pthread_mutex_unlock(&play_cmd_tex);
      continue;
     }


    fgets(command, 256, stdin);
    command[strcspn(command, "\n")] = 0;

    if (!strcmp(command, ":lib") && current_focus != LIB) {
        strcpy(command, "refresh"); /* reprint last library message */
        current_focus = LIB;
    }

    else if (!strcmp(command, ":cont") && current_focus != CONT) {
      strcpy(command, "nothing");
      current_focus = CONT;
    }

    else if (!strcmp(command, ":exit")) {
          lib_cmd = 1;
          play_cmd = 1;
          pthread_cond_signal(&player_sleep);
          pthread_join(thread_table[0], NULL);
          pthread_cond_signal(&lib_sleep);

          pthread_join(thread_table[1], NULL);

          pthread_exit(0);
    }
    
    if (current_focus == LIB) {
      pthread_mutex_lock(&lib_cmd_tex);
      pthread_cond_signal(&lib_sleep);
        lib_cmd = 1;
        if (lib_cmd) {
          pthread_cond_wait(&control_sleep, &lib_cmd_tex);
        }
      pthread_mutex_unlock(&lib_cmd_tex);
    }

    else if (current_focus == CONT && songs != NULL) {
      pthread_mutex_lock(&play_cmd_tex);
      pthread_cond_signal(&player_sleep);
      play_cmd = 1;
      if (play_cmd) {
        pthread_cond_wait(&control_sleep, &play_cmd_tex);
      }
      pthread_mutex_unlock(&play_cmd_tex);
    }
  }
}


int main(int argc, char** argv) {
  char* path;
  //pthread_t thread_table[3];
  int i;
  path = (char*) malloc(strlen(argv[1]));
  strcpy(path, argv[1]);
  init_engine();
  scan_folder(path);
  pthread_create(thread_table+1, NULL, cursor, NULL);
  pthread_create(thread_table, NULL, play_audio, NULL);
  pthread_create(thread_table+2, NULL, controls, NULL);

  pthread_join(thread_table[2], NULL);

  free_lib();
  return 0;
}
