/* libmpd (high level libmpdclient library)
 * Copyright (C) 2004-2009 Qball Cow <qball@sarine.nl>
 * Project homepage: http://gmpcwiki.sarine.nl/
 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "libmpd.h"
#include "debug_printf.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv)
{
	MpdObj *obj = mpd_new("192.150.0.50", 6600, "pass");
	debug_set_level(DEBUG_INFO);	
	if(!mpd_connect(obj))
//	for(i=0;i<1000;i++)
	{
		mpd_send_password(obj);
		int id = mpd_playlist_add_get_id(obj, "Sorted/0-K/C/Clapton, Eric/1992 - Unplugged/14 - Rollin' & Tumblin'.flac");
		printf("%i\n", id);
		mpd_player_play_id(obj, id);
	/*	MpdData * data = mpd_playlist_find(obj, MPD_TABLE_ARTIST,"(General)", TRUE);
		while(data != NULL)
		{
			char buffer[1024];
			mpd_song_markup(buffer, 1024,"[%name%: &[%artist% - ]%title%]|%name%|[%artist% - ]%title% &[(%time%)]|%shortfile%", data->song);
			printf("%s\n", buffer);


			data = mpd_data_get_next(data);
		}
*/
	}
	mpd_free(obj);
}
