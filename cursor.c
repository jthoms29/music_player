#include <music_defs.h>
#include <stdio.h>

extern GList* library[81];

artist* current_artist;
album* current_album;

/* Go through library, printing the name of each artist */
void print_artists(void) {
  GList* art_walker;
  artist* cur_artist;
  int i;
  for (i = 0; i < 81; i++) {
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

  char choice[MAX_TITLE];
  int track_choice;
  int lib_index;

  for(;;) {
    print_artists();

    printf("Choose artist: \n");
    fgets(choice, MAX_TITLE, stdin);

    choice[strlen(choice) -1] = 0;
    printf("%s\n", choice);

    lib_index = choice[0] - 41;

    found_artist = g_list_find_custom(library[lib_index], choice,
      (GCompareFunc) find_artist);

    if (!found_artist) {
      printf("No such artist\n\n");
      continue;
    }

    cur_artist = (artist*) found_artist->data;
    printf("%s:\n", cur_artist->name);
    print_albums(cur_artist);

    fgets(choice, MAX_TITLE, stdin);
    choice[strlen(choice) -1] = 0;

    found_album = g_list_find_custom(cur_artist->albums, choice,
      (GCompareFunc) find_album);

    if (!found_album) {
        printf("No such album\n\n");
        continue;
    }

    cur_album = (album*) found_album->data;

    print_songs(cur_album);

    scanf("%d", &track_choice);

    play_audio(cur_album, track_choice);

  }
}
