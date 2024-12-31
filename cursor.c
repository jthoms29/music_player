#include <music_defs.h>
#include <stdio.h>

extern GList* library[27];

extern GList* songs;
extern int current_focus;
extern int swap;

extern int cur_window;
extern int new_song;
extern int first_song;
//artist* current_artist;
//album* current_album;

extern pthread_mutex_t lib_cmd_tex;
extern pthread_mutex_t control_tex;

extern pthread_cond_t lib_sleep;
extern pthread_cond_t control_sleep;
extern int lib_cmd;

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

void cursor(void) {
  GList *found_artist, *found_album, *found_song;
  artist* cur_artist;
  album* cur_album;

  int lib_index;
  int track_check;

  for(;;) {
    pthread_mutex_lock(&lib_cmd_tex);
    while (!found_artist) {


      print_artists();
      printf("Choose artist: \n");
      //fgets(choice, MAX_TITLE, stdin);
      lib_cmd = 0;

      pthread_cond_signal(&control_sleep);

      pthread_cond_wait(&lib_sleep, &lib_cmd_tex);
     
      if (!strcmp(command, "refresh")) {
        pthread_mutex_unlock(&lib_cmd_tex);
        continue;
      }
      else if (!strcmp(command, ":exit")) {
        pthread_exit(0);
      }

//      else if (!strcmp(command, ":back")) {
 //       break;
  //    }

      lib_index = tolower(command[0]) - 97;

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

          if (songs == NULL) first_song = 1;
          else new_song = 1;
 
          songs = found_song;
          
          lib_cmd = 0;
          pthread_cond_signal(&control_sleep);
          pthread_cond_wait(&lib_sleep, &lib_cmd_tex);

          if (!strcmp(command, ":exit")) {
            pthread_exit(0);
          }
        }
      }
    }
  }
}
