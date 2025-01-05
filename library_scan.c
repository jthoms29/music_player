//John Thoms
#include <stdio.h>
#include <music_defs.h>
#include <string.h>
#include <tag_c.h>
#include <dirent.h>
#include <ctype.h>
#include <strings.h>
#include <bsd/string.h>
extern GList* library[27];

/* functions for Glist insertion and searching *********************** */
gint find_artist(gconstpointer list_artist, gconstpointer my_artist_str) {
  const char* str_ref = (char*) my_artist_str;
  const artist* art_ref = (artist*) list_artist;

  return strcasecmp(str_ref, art_ref->name);
}

gint insert_artist(gconstpointer list_artist, gconstpointer my_artist) {
  const artist* list_art_ref = (artist*) list_artist;
  const artist* my_art_ref = (artist*) my_artist;
  return strcasecmp(my_art_ref->name, list_art_ref->name);
}

gint find_album(gconstpointer list_album, gconstpointer my_album_str) {
  const char* str_ref = (char*) my_album_str;
  const album* alb_ref = (album*) list_album;
  return strcasecmp(str_ref, alb_ref->title);
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

gint find_track(gconstpointer alb_song, gconstpointer track_idx) {
  const song* song_ref = (song*) alb_song;
  int trck = *((int*) track_idx);

  return song_ref->track - trck;
}

/* ************************************************************************ */


/* Adds a song struct to the global library array within the proper artist
 * and album sublists */
int song_to_lib(song* sng) {
  GList *found_artist_loc, *found_album_loc, *found_song_loc;
  GList *artist_start, *album_start;
  artist *found_artist, *new_artist;
  album *found_album, *new_album;
  char start_letter; 
  int index;

  /* The first letter of the artist's name will be used to index into the
   * global list array*/
  start_letter = tolower(sng->artist[0]);
  index = start_letter - 97;

  if (index < 0 || index > 26) {
    printf("%s: %d - ", sng->artist, index);
    printf("Invalid name\n");
    return 1;
  }


  found_artist_loc = g_list_find_custom(library[index], 
    sng->artist, (GCompareFunc) find_artist); 

  if (found_artist_loc) {
      found_artist = (artist*) found_artist_loc->data;
  }

  /* If the artist was not found in the library, they must be added */
  else {
    new_artist = (artist*) malloc(sizeof(artist));

    if (!new_artist) {
      printf("malloc failure\n");
      return 1;
    }
    if (strlcpy(new_artist->name, sng->artist, MAX_TITLE) >= MAX_TITLE) {
        return -1;
    }
    new_artist->albums = NULL;
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
    if (!new_album) {
      printf("malloc failure\n");
      return 1;
    }
    if (strlcpy(new_album->title, sng->album, MAX_TITLE) >= MAX_TITLE) return -1;
    if (strlcpy(new_album->artist, sng->artist, MAX_TITLE) >= MAX_TITLE) return -1;
    if (strlcpy(new_album->genre, sng->genre, MAX_TITLE) >= MAX_TITLE) return -1;
    new_album->year = sng->year;
    new_album->tracks = 0;
    new_album->songs = NULL;

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

/* testing, prints data from song struct */
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

/* will read tags from the audio file from the given path, creating a new song
 * struct with its data */
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

  if (strlcpy(song_ret->path, path, MAX_PATH) >= MAX_PATH) return 0;
  if (strlcpy(song_ret->title, taglib_tag_title(tag), MAX_TITLE) >= MAX_TITLE) return 0;
  if (strlcpy(song_ret->artist, taglib_tag_artist(tag), MAX_TITLE) >= MAX_TITLE) return 0;
  if (strlcpy(song_ret->album, taglib_tag_album(tag), MAX_TITLE) >= MAX_TITLE) return 0;
  if (strlcpy(song_ret->genre, taglib_tag_genre(tag), MAX_TITLE) >= MAX_TITLE) return 0;
  song_ret->year = taglib_tag_year(tag);
  song_ret->track = taglib_tag_track(tag);

//  print_song_data(song_ret);
  taglib_file_free(file);
  return song_ret;
}


/* recursively scans the given folder, adding any audio files found within to
 * the global library array */
void scan_folder(char* path) {
  struct dirent *de; /*pointer for directory entry */
  DIR *dir;           /* DIR pointer, what opendir returns */
  char full[MAX_PATH];     /* Used to hold Full path of file */
  song* tag_return;

  int attempt;
  dir = opendir(path);

  if (!dir) {
    printf("Invalid path\n");
    printf("%s - %s\n", path, strerror(errno));
    return;
  }

    
  while((de = readdir(dir)) != NULL) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
      continue;
    }

    if ((attempt = strlcpy(full, path, MAX_PATH)) >= MAX_PATH) {
      printf("1: Path too long - %s - %d\n", path, attempt);
      continue;
    }

    if ((attempt = strlcat(full, "/", MAX_PATH)) >= MAX_PATH) {
      printf("2: Path too long - %s - %d\n", path, attempt);
      continue;
    }

    if ((attempt = strlcat(full, de->d_name, MAX_PATH)) >= MAX_PATH) {
      printf("3: Path too long - %s - %d\n", path, attempt);
      continue;
    }


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

/* freeing functions **************************************** */

/* songs structs have no seperately malloced data, can just use free */

void free_album(void* albm) {
  album* cur_album = (album*) albm;
  g_list_free_full(cur_album->songs, free);
  free(cur_album);
}

void free_artist(void* artst) {
  artist* cur_artist = (artist*) artst;
  g_list_free_full(cur_artist->albums, free_album);
  free(cur_artist);
}

/* frees entire library. Use before exit */
void free_lib() {
  int i;
  for (i = 0; i < 27; i++) {
    g_list_free_full(library[i], free_artist);
  }
}


