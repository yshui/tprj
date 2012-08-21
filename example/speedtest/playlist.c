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

#include <libmpd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv)
{
	int i=0;
	MpdObj *obj = mpd_new("192.150.0.114", 6600, NULL);
	if(!mpd_connect(obj))
	for(i=0;i<10;i++)
	{
		MpdData *data;
		data = mpd_playlist_get_changes(obj,-1);
		while(data != NULL)
		{
			data = mpd_data_get_next(data);
		}
	}
	mpd_free(obj);
}
