// John Thoms

#include <stdio.h>
#include <music_defs.h>
#include <pthread.h>
#include <string.h>
#include <glib.h>
#include <bsd/string.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

/* LIB (library browser) or CONT (music controls) */
/* dictates which thread will be alerted to the current command */
int current_focus; 

pthread_mutex_t song_choice_tex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t play_cmd_tex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lib_cmd_tex = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t player_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t control_sleep = PTHREAD_COND_INITIALIZER;
pthread_cond_t lib_sleep = PTHREAD_COND_INITIALIZER;

/* The music library will be held in this array, which is indexed by the ascii
 * code of the first letter of the artist's name. Each contains a linked list
 * of artists, each of which contains a linked list of albums, each of which
 * contains a linked list of songs. */
/* As of now, the starting letter of the artist can only be from ascii
 * characters a to z */
GList* library[27] = {0,};

/* holds the pid for each thread */
pthread_t thread_table[3];

/* vars needed for miniaudio functionality */
ma_engine* engine;
ma_sound sound;

/* linked list of songs from chosen album. Should be accessed with mutex */
GList* songs = NULL;

/* holds the current command. All threads can read, only thread that is in
 * current_focus will read */
char command[256];

/* used for condition variable sync */
int play_cmd = 0;
int lib_cmd = 0;

int new_song = 0;
int first_song = 0;

/* initializes miniaudio engine */
ma_result init_engine() {
  ma_result result;

  /* Allocating sound engine */
  engine = malloc(sizeof(ma_engine));
  result = ma_engine_init(NULL, engine);
  return result;
} 


/* This thread plays the audio using what is currently in the `songs` linked
 * list */
void* play_audio(void* arg) {
  song* cur_song;
  ma_result result;
  int paused ;

  (void) arg; /*compiler warning */
  paused = 0;

  for(;;) {
    /* No songs currently, go to sleep */
    pthread_mutex_lock(&song_choice_tex);
    while (!songs) {
      pthread_cond_wait(&player_sleep, &song_choice_tex);
      if (play_cmd && !strcmp(command, ":exit")) {
        pthread_exit(0);
      }
    }
    //pthread_mutex_unlock(&song_choice_tex);

    cur_song = (song*) songs->data;
    result = ma_sound_init_from_file(engine, cur_song->path, 0, NULL, NULL,
      &sound); 

    if (result != MA_SUCCESS) {
      printf("uh oh");
      pthread_exit(0);
    }

    ma_sound_start(&sound);
    pthread_mutex_unlock(&song_choice_tex);

    /* song successfully started */

    while(!ma_sound_at_end(&sound)) {

      /* if this thread is currently in focus, print the current song.
       * Should not be printed if user is currently browsing library */
      if (current_focus == CONT) {
        printf("%s - %s: %d - %s    command:  ", cur_song->artist, 
          cur_song->album, cur_song->track, cur_song->title);
        fflush(stdout);
      }

    /* Wait for command. Waiting on two separate conditions, so will just busy
     * wait */
    while (!ma_sound_at_end(&sound) && !play_cmd) {}

      /* The user has input a command */
      if (play_cmd) {

        /* will reprint the currently playing song. Used for when user switches
         * from library view to song control view */
        if (!strcmp(command, ":refresh")) {
          break;
        }

        else if (!strcmp(command, ":next")) {
          pthread_mutex_lock(&song_choice_tex);
          songs = songs->next;
          paused = 0;
          break;
        }

        else if (!strcmp(command, ":prev")) {
          pthread_mutex_lock(&song_choice_tex);
          songs = songs->prev;
          paused = 0;
          break;
        }

        else if (!strcmp(command, ":pause") && !paused) {
          ma_sound_stop(&sound);
          paused = 1;
        }

        else if (!strcmp(command, ":unpause") && paused) {
          paused = 0;
          ma_sound_start(&sound);
        }

        else if (!strcmp(command, ":exit")) {
          ma_sound_stop(&sound);
          ma_sound_uninit(&sound);
          free(engine);
          pthread_exit(0);
        }

        pthread_mutex_lock(&play_cmd_tex);
        play_cmd = 0;
        pthread_cond_signal(&control_sleep);
        pthread_mutex_unlock(&play_cmd_tex);
      }
    }
    
    ma_sound_stop(&sound);
    ma_sound_uninit(&sound);

    if (play_cmd) {
      pthread_mutex_lock(&play_cmd_tex);
      play_cmd = 0;
      pthread_cond_signal(&control_sleep);
      pthread_mutex_unlock(&play_cmd_tex);
      pthread_mutex_unlock(&song_choice_tex);
    }

    /* if the user did not input a command, default functionality is to go to
     * next song in album */
    else {
      pthread_mutex_lock(&song_choice_tex);
      songs = songs->next;
      pthread_mutex_unlock(&song_choice_tex);
      if (current_focus == CONT) printf("\n");
    }
  }
}



/* This thread takes user input, and depending on the value in current_focus
 * will either alert the library browser thread (cursor) or audio thread */
void* controls(void* arg) {
  current_focus = LIB;
  (void) arg; /*compiler warning */
  for(;;) {
    /* **************** special cases **************************** */

    /* if the current focus is the audio controller but no song is selected.
     * Nothing can happen, the user must go back to library view */
    if (current_focus == CONT && songs == NULL) {
      printf("Nothing currently playing\n");
    }

    /* alerts the player that a song has been chosen if nothing is playing */
    else if (first_song) {
      current_focus = CONT;
      first_song = 0;
      pthread_cond_signal(&player_sleep);
      continue;
    }

    else if (new_song) { 
      current_focus = CONT;
      new_song = 0;
      strcpy(command, ":refresh");

      pthread_mutex_lock(&play_cmd_tex);
        play_cmd = 1;

        if (play_cmd) {
          pthread_cond_wait(&control_sleep, &play_cmd_tex);
        }

      pthread_mutex_unlock(&play_cmd_tex);
      continue;
     }

    /* ******************** regular commands *********************** */

    fgets(command, 256, stdin);
    command[strcspn(command, "\n")] = 0;

    if (!strcmp(command, ":lib") && current_focus != LIB) {
        strcpy(command, ":refresh"); /* reprint last library message */
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
  char path[MAX_PATH];

  if (argc != 2) {
    printf("Usage: music_player <library path>\n");
    return 0;
  }
  if (strlcpy(path, argv[1], MAX_PATH) >= MAX_PATH) {
    printf("Invalid path\n");
    return 0;
  }

  init_engine();
  scan_folder(path);
  pthread_create(thread_table+1, NULL, cursor, NULL);
  pthread_create(thread_table, NULL, play_audio, NULL);
  pthread_create(thread_table+2, NULL, controls, NULL);

  pthread_join(thread_table[2], NULL);

  free_lib();
  return 0;
}
