/* John Thoms */
#include <stdio.h>
#include <music_defs.h>
#include <string.h>
#include <tag_c.h>
#include <dirent.h>

extern GList* library[81];

gint find_artist(gconstpointer list_artist, gconstpointer my_artist_str) {
  const char* str_ref = (char*) my_artist_str;
  const artist* art_ref = (artist*) list_artist;
  return strcmp(str_ref, art_ref->name);
}

gint insert_artist(gconstpointer list_artist, gconstpointer my_artist) {
  const artist* list_art_ref = (artist*) list_artist;
  const artist* my_art_ref = (artist*) my_artist;
  return strcmp(my_art_ref->name, list_art_ref->name);
}

gint find_album(gconstpointer list_album, gconstpointer my_album_str) {
  const char* str_ref = (char*) my_album_str;
  const album* alb_ref = (album*) list_album;
  return strcmp(str_ref, alb_ref->title);
}

gint insert_album(gconstpointer list_album, gconstpointer my_album) {
  const album* list_alb_ref = (album*) list_album;
  const album* my_alb_ref = (album*) my_album;
  return list_alb_ref->year - my_alb_ref->year; 

}

gint insert_song(gconstpointer list_song, gconstpointer my_song) {
  const song* list_song_ref = (song*) list_song;
  const song* my_song_ref = (song*) my_song;
  return list_song_ref->track - my_song_ref->track;

}


int song_to_lib(song* sng) {
  char* sng_artist;
  GList *found_artist_loc, *found_album_loc, *found_song_loc;
  GList *artist_start, *album_start, *song_start;
  artist *found_artist, *new_artist;
  album *found_album, *new_album;
  char start_letter; 
  int index;

  /* The first letter of the artist's name will be used to index into the
   * global list array*/
  start_letter = sng->artist[0];
  index = start_letter - 41;

  found_artist_loc = g_list_find_custom(library[index], 
    sng->artist, (GCompareFunc) find_artist); 

  if (found_artist_loc) {
      found_artist = (artist*) found_artist_loc->data;
  }

  /* If the artist was not found in the library, they must be added */
  else {
    new_artist = (artist*) malloc(sizeof(artist));
    strcpy(new_artist->name, sng->artist);
    /* Will be inserted in alphabetical order */
    artist_start = g_list_insert_sorted(library[index], 
      (gpointer) new_artist, (GCompareFunc) insert_artist);

    /* set library index to possible new start of list */
    library[index] = artist_start;

    found_artist = new_artist;
  }



  /* Now, we must do something similar for the song's album */
  found_album_loc = g_list_find_custom(found_artist->albums, sng->album,
    (GCompareFunc) find_album);

  if (found_album_loc) {
    found_album = (album*) found_album_loc->data;
  }

  /* Album was not found in artist, must be added */
  else {
    new_album = (album*) malloc(sizeof(album));
    strcpy(new_album->title, sng->album);
    strcpy(new_album->artist, sng->artist);
    strcpy(new_album->genre, sng->genre);
    new_album->year = sng->year;
    new_album->tracks = 0;

    album_start = g_list_insert_sorted(found_artist->albums,
      (gpointer) new_album, (GCompareFunc) insert_album);
    found_artist->albums = album_start;

    found_album = new_album;
  }

  /* And now the song must be put in the album */
  found_song_loc = g_list_insert_sorted(found_album->songs, (gpointer) sng,
    (GCompareFunc) insert_song);

  found_album->songs = found_song_loc;

  return 0;
}

song* read_tag(char* path) {
  song* song_ret;
  TagLib_File *file;
  TagLib_Tag *tag;

  if((file = taglib_file_new(path)) == 0)
      return NULL;
  if((tag = taglib_file_tag(file)) == 0)
      return NULL;

  /* Might not be ideal to allocate each song individually, may find better mem
   * allocation method. */
  song_ret = (song*) malloc(sizeof(song));

  /* malloc failure */
  if (!song_ret)
    return NULL;

  /* add tag data to song struct and return */
  strcpy(song_ret->path, path);
  strcpy(song_ret->title, taglib_tag_title(tag));
  strcpy(song_ret->artist, taglib_tag_artist(tag));
  strcpy(song_ret->album, taglib_tag_album(tag));
  strcpy(song_ret->genre, taglib_tag_genre(tag));
  song_ret->year = taglib_tag_year(tag);
  song_ret->track = taglib_tag_track(tag);

  return song_ret;
}

void lib_test(void) {
  GList* artWalker;
  GList* albWalker;
  GList* songWalker;
  artist* testArt;
  album* testAlb;
  song* testSong;
  char full[256];
  for (int i = 0; i < 81; i++) {
    artWalker = library[i];
    while (artWalker != NULL) {
      testArt = (artist*) artWalker->data;
      printf("%s\n", testArt->name);

      albWalker = testArt->albums;

      while(albWalker != NULL) {
        testAlb = (album*) albWalker->data;
        printf("%s\n", testAlb->title);


        songWalker = testAlb->songs;

        while(songWalker != NULL) {
          testSong = (song*) songWalker->data;
          printf("%s\n", testSong->title);

          songWalker = songWalker->next;
          strcpy(full, testSong->path);

        }
//        miniaudio_init(full);
        albWalker = albWalker->next;
      }
      artWalker = artWalker->next;
    }
  }

}


int print_song_data(song* sng) {
  if (!sng) {
    return -1;
  }

  printf("Title: %s\n", sng->title);
  printf("Artist: %s\n", sng->artist);
  printf("Album: %s\n", sng->album);
  printf("Track: %d\n", sng->track);
  printf("Year: %d\n", sng->year);
  printf("Genre: %s\n", sng->genre);
  printf("Path: %s\n", sng->path);
  return 0;

}

void scan_folder(char* path) {
  struct dirent *de; /*pointer for directory entry */
  DIR *dir;           /* DIR pointer, what opendir returns */
  char full[256];     /* Used to hold Full path of file */
  song* tag_return;


  dir = opendir(path);

  if (!dir) {
    printf("Invalid path\n");
    return;
  }
    
  while((de = readdir(dir)) != NULL) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
      continue;
    }

    strcpy(full, path);
    strcat(full, "/");
    strcat(full, de->d_name);

    /* If this file is a directory, scan recursively */
    if (de->d_type == DT_DIR) {
      scan_folder(full);
    }
    else {
      tag_return = read_tag(full);
      if (tag_return) {
        song_to_lib(tag_return);
      }
    }
  }
  closedir(dir);

}

