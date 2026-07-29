# Music Terminal Player

A terminal music player written in C++. Playback is simulated — each song "plays" over its length with a live text progress bar — while a second thread reads keyboard commands, so you can skip, pause, shuffle, adjust volume/EQ, and edit the playlist without stopping the music.

## How it works

- **Two threads**: the main thread runs the playback loop and draws the progress bar; a separate input thread reads keypresses and turns them into commands. They communicate through a mutex-protected command queue, so input never blocks playback.
- **Playback queue**: songs live in a `TrackList` that keeps the library separate from the play order, so shuffle can reorder playback without losing the original list.
- **History**: a stack tracks previously played songs so `previous` can walk backward.
- **Shared console**: all terminal output goes through a single mutex so the progress bar and command responses never overwrite each other mid-write.

## Commands

| Key | Action |
|---|---|
| `s` | Skip to next song |
| `b` | Previous song |
| `p` | Pause / resume |
| `S` | Toggle shuffle |
| `R one` / `R all` / `R off` | Set repeat mode |
| `>` / `<` | Increase / decrease volume |
| `e` / `E` | View / adjust EQ (bass, mids, treble) |
| `l` | List songs |
| `f` | Find a song by ID |
| `a` | Add a song |
| `r` | Remove a song by ID |
| `h` | Show the command list |
| `q` | Quit |

## Build & run

```
g++ -std=c++17 -pthread Song.cpp AudioSettings.cpp TrackList.cpp ConsoleUI.cpp Player.cpp CommandQueue.cpp Utilities.cpp main.cpp -o MusicPlayer
./MusicPlayer
```

The player starts with a small built-in playlist.

## Some constraints

- Playback is simulated, not real audio — a song's "length" is just how long its progress bar runs. Volume and EQ are display-only.
- The progress bar and your keystrokes share one terminal line, so typing mid-playback can look visually jumbled even though the input still registers. A raw-terminal (termios) input mode would fix this and is noted as a future improvement.
- Loading a playlist clears the current one *before* parsing the file, so if a load fails partway through, the existing playlist is lost with nothing loaded in its place.
- Saving refuses to overwrite an existing file — you can't re-save or update a playlist under a name that already exists; you have to pick a new name.
