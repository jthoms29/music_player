/* John Thoms */


typedef struct song {
  char* path;
  char* title;
  char* artist;
  char* release_date;
  char* length;
} song;


typedef struct album {
  char* title;
  char* artist;
  char* release_date;
  char* length;
  unsigned int num_songs;
  song* songs;
} album;
