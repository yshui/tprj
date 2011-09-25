#ifndef _MPD_H_
#define _MPD_H_

struct mpd_song;

typedef int (*mpd_closure)(struct mpd_song *, void *);

char *get_song_name_clever(struct mpd_song *);
void mpd_main_loop(mpd_closure, void *);
int mpd_init();

#endif
