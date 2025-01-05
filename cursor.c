// John Thoms

#include <music_defs.h>
#include <stdio.h>
#include <ctype.h>
#include <glib.h>
extern GList* library[27];

extern GList* songs;

/* used to indicate special commands to input thread that should be sent to
 * player */
extern int new_song;
extern int first_song;

extern pthread_mutex_t lib_cmd_tex;
extern pthread_mutex_t control_tex;

extern pthread_mutex_t song_choice_tex;

extern pthread_cond_t lib_sleep;
extern pthread_cond_t control_sleep;

extern int lib_cmd;

/* global command variable, cursor thread will read when signalled */
extern char command[256];

/* Go through library, printing the name of each artist */
void print_artists(void) {
  GList* art_walker;
  artist* cur_artist;
  int i;
  for (i = 0; i < 27; i++) {
    art_walker = library[i];
    while (art_walker != NULL) {
      cur_artist = (artist*) art_walker->data;
      printf("%s\n", cur_artist->name);
      art_walker = art_walker->next;
    }
  }
}

/* print all albums from specified artist */
void print_albums(artist* artst) {
    GList* alb_walker;
    album* cur_album;

    alb_walker = artst->albums;
    while (alb_walker != NULL) {
      cur_album = (album*) alb_walker->data;
      printf("  %s\n", cur_album->title);
      alb_walker = alb_walker->next;
    }
}

/* print all songs from specified album */
void print_songs(album* albm) {
  GList* song_walker;
  song* cur_song;

  song_walker = albm->songs;
  while (song_walker != NULL) {
    cur_song = (song*) song_walker->data;
    printf("    %d - %s\n", cur_song->track, cur_song->title);
    song_walker = song_walker->next;
  }

}

/* this thread moves through the library, taking user input from the control
 * thread in main.c */
void* cursor(void* arg) {
  GList *found_artist = 0, *found_album = 0, *found_song = 0;
  artist* cur_artist;
  album* cur_album;

  int lib_index;
  int track_check;
  (void) arg; /* compiler warning */
  for(;;) {
    pthread_mutex_lock(&lib_cmd_tex);
    while (!found_artist) {


      print_artists();
      printf("Choose artist: \n");
      lib_cmd = 0;

      /* this thread goes to sleep, waiting for user input */
      pthread_cond_signal(&control_sleep);
      pthread_cond_wait(&lib_sleep, &lib_cmd_tex);
     
      if (!strcmp(command, "refresh")) {
        pthread_mutex_unlock(&lib_cmd_tex);
        continue;
      }
      else if (!strcmp(command, ":exit")) {
        pthread_exit(0);
      }

      lib_index = tolower(command[0]) - 97;

      if (lib_index < 0 || lib_index > 26) {
        printf("No such artist\n\n");
        continue;
      }

      found_artist = g_list_find_custom(library[lib_index], command,
        (GCompareFunc) find_artist);

      if (!found_artist) {
        printf("No such artist\n\n");
        continue;
      }

      cur_artist = (artist*) found_artist->data;
      found_artist = 0;

      while (!found_album) {

        print_albums(cur_artist);
        lib_cmd = 0;
        pthread_cond_signal(&control_sleep);
        pthread_cond_wait(&lib_sleep, &lib_cmd_tex);

        if (!strcmp(command, "refresh")) {
          continue;
        }
        else if (!strcmp(command, ":back")) {
          break;
        }
        else if (!strcmp(command, ":exit")) {
          pthread_exit(0);
        }

        found_album = g_list_find_custom(cur_artist->albums, command,
          (GCompareFunc) find_album);

        if (!found_album) {
          printf("No such album\n\n");
          continue;
        }

        cur_album = (album*) found_album->data;
        found_album = 0;

        while (1) {
          print_songs(cur_album);

          lib_cmd = 0;
          pthread_cond_signal(&control_sleep);
          pthread_cond_wait(&lib_sleep, &lib_cmd_tex);

          
          if (!strcmp(command, "refresh")) {
            continue;
          }
          else if (!strcmp(command, ":back")) {
            break;
          }
          else if (!strcmp(command, ":exit")) {
            pthread_exit(0);
          }

          track_check = atoi(command);
          found_song = g_list_find_custom(cur_album->songs,
            (gconstpointer) &track_check, (GCompareFunc) find_track);

          if (!found_song) {
            printf("No such song\n\n");
            continue;
          }

          pthread_mutex_lock(&song_choice_tex);

          /* if nothing is currently playing, first_song must be set to 1 so
           * player thread can be signalled */
          if (songs == NULL) first_song = 1;

          /* lets controll function know it should send 'refresh' command to
           * player so that new song can be started */
          else new_song = 1;
 
          songs = found_song;
          pthread_mutex_unlock(&song_choice_tex);
          
          lib_cmd = 0;
          pthread_cond_signal(&control_sleep);
          pthread_cond_wait(&lib_sleep, &lib_cmd_tex);

          if (!strcmp(command, ":exit")) {
            pthread_exit(0);
          }
          break;
        }
      }
    }
  }
}
