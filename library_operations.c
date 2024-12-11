/* John Thoms */
#include <stdio.h>
#include <music_defs.h>
#include <string.h>
#include <tag_c.h>
#include <dirent.h>
//#include <glib.h>

extern GList library[81];

gint find_artist(gconstpointer list_artist, gconstpointer my_artist_str) {
  const char* str_ref = (char*) my_artist_str;
  const artist* art_ref = (artist*) list_artist;
  return strcmp(str_ref, art_ref->name);
}

gint insert_artist(gconstpointer my_artist, gconstpointer list_artist) {
  const artist* my_art_ref = (artist*) my_artist;
  const artist* list_art_ref = (artist*) list_artist;
  return strcmp(my_art_ref->name, list_art_ref->name);
}


int song_to_lib(song* sng) {
  char* sng_artist;
  GList* found_artist_loc;
  artist* new_artist;
  album *found_album, *new_album;
  char start_letter; 
  int index;

  /* The first letter of the artist's name will be used to index into the
   * global list array*/
  start_letter = sng->artist[0];
  index = start_letter - 41;

  /* check if list exists */
  if (library[index]) {
    found_artist_loc = g_list_find_custom(library+index, 
      sng->artist, (GCompareFunc) find_artist); 
  }

  /* If the artist was not found in the library, they must be added */
  if (!found_artist_loc) {
    new_artist = (artist*) malloc(sizeof(artist));
    strcpy(new_artist->name, sng->artist);
    /* Will be inserted in alphabetical order */
    found_artist_loc = g_list_insert_sorted(library+index, 
      (gpointer) new_artist, (GCompareFunc) insert_artist);
  }

  char* str = ((artist*) library['T'-41].data)->name;
  printf("yuppp %s\n", str);

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

  song_to_lib(song_ret);
  return song_ret;
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


  char* full;
  dir = opendir(path);

  if (!dir) {
    printf("Invalid path\n");
    return;
  }
    
  song *ret;
  song *ret2;
  full = (char*) malloc(strlen(path));
  while((de = readdir(dir)) != NULL) {
    full = (char*) realloc(full, strlen(path) + strlen(de->d_name));
    strcpy(full, path);
    strcat(full, de->d_name);
    ret = read_tag(full);
    if (ret) {
      print_song_data(ret);
    }
  }

  closedir(dir);
}

