#include <mpd/client.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <poll.h>
#include <libgen.h>
#include "mpd.h"
static struct mpd_connection *mpd_conn;
const enum mpd_tag_type title_types[] = {
	MPD_TAG_TITLE,
	MPD_TAG_TRACK,
	MPD_TAG_NAME,
};
const int title_type_count = 3;
const enum mpd_tag_type artist_types[] = {
	MPD_TAG_ARTIST,
	MPD_TAG_ALBUM_ARTIST,
	MPD_TAG_COMPOSER,
	MPD_TAG_PERFORMER,
};
const int artist_type_count = 4;
gchar *get_song_name_clever(struct mpd_song *song){
	//Title
	const gchar *title = NULL;
	int i = 0;
	while(!title && i<title_type_count)
		title = mpd_song_get_tag(song, title_types[i++], 0);
	//Artist
	const gchar *artist = NULL;
	i = 0;
	while(!artist && i<artist_type_count)
		artist = mpd_song_get_tag(song, artist_types[i++], 0);
	//Album
	const gchar *album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
	if(!album) album = mpd_song_get_tag(song, MPD_TAG_DISC, 0);
	gchar *song_name;
	if(album || artist || title)
		song_name = g_strdup_printf("`%s' from `%s' by `%s'", title?title:"", album?album:"", artist?artist:"");
	else{
		const gchar *uri = mpd_song_get_uri(song);
		gchar *tmp_uri = g_strdup(uri);
		const gchar *file_name = basename(tmp_uri);
		g_free(tmp_uri);
		song_name = g_strdup(file_name);
	}
	return song_name;
}
void mpd_main_loop(mpd_closure user_fun, void *user_data){
	mpd_send_idle_mask(mpd_conn, MPD_IDLE_PLAYER);
	int fd = mpd_connection_get_fd(mpd_conn);
	struct pollfd fds[1];
	fds[0].fd = fd;
	fds[0].events = POLLIN;
	struct mpd_song *song;
	while(poll(fds, 1, -1)){
		if(fds[0].revents | POLLIN){
			mpd_recv_idle(mpd_conn, 1);
			song = mpd_run_current_song(mpd_conn);
			user_fun(song, user_data);
			mpd_send_idle_mask(mpd_conn, MPD_IDLE_PLAYER);
		}
	}
	mpd_connection_free(mpd_conn);
}
int mpd_init(){
	mpd_conn = mpd_connection_new(NULL, 0, 0);
	if(mpd_connection_get_error(mpd_conn) != MPD_ERROR_SUCCESS){
		const gchar *message = mpd_connection_get_error_message(mpd_conn);
		g_print("error: %s\n", message);
		return -1;
	}
	int i;
	struct mpd_song *song = mpd_run_current_song(mpd_conn);
	for(i=1; i<MPD_TAG_COUNT; i++){
		const gchar *tmp = mpd_song_get_tag(song, i, 0);
		if(tmp)
			g_print("%d %s\n", i, tmp);
	}
	return 0;
}

