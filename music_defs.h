/* John Thoms */


typedef struct song {
  char* path;
  char* title;
  char* artist;
  char* genre;
  int year;
  int track;
  struct song* next_song;
} song;


typedef struct album {
  char* title;
  char* artist;
  char* release_date;
  char* length;
  unsigned int num_songs;
  song* songs;
} album;
