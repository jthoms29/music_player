# `music_player`
A simple terminal music player


## Dependencies
- miniaudio.h - included in repo
- 

## Guide

### Commands
- `:cont` - move focus to audio player controls
- `:lib` - move focus to library browser



#### Library browser commands
Input artist/album name (case insensitive) to move to that specific menu.
Within the album view, songs are specified by their track number.

- `:back` - moves back to previous menu

#### Audio commands
- `:pause` - in audio player view, pauses currently playing song
- `:unapuse` - in audio player view, unpauses currently playing song
- `:next` - in audio player view, moves to next song in album
- `:prev` - in audio player view, moves to previous song in album

## Current limitations
- Only allows artists within alphabetical range a-z
- Static path and name sizes, both currently can't exceed a particlar length

