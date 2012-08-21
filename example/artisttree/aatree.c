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

#include <stdio.h>
#include <stdlib.h>
#include <libmpd/libmpd.h>

int main(int argc, char **argv)
{
	/* the main structure */
	MpdObj *mo;
	/*
	 * A list that can hold the data that mpd returns
	 */
	MpdData *data = NULL;


	/* create a connection
	 * This defaults to:
	 * host=localhost
	 * port=6600
	 */
	mo = mpd_new_default();

	/* Make a connection and check for errors */
	if(mpd_connect(mo))
	{
		/*
		 * to print an error message, you need to connect the
		 * error signal and catch that
		 */
		/* clean up the connection and free the space needed by mpdObj */
		mpd_free(mo);
		printf("Failed to connect\n");
		return 1;
	}

	/* Get all artists *
	 * If the list is at the end, mpd_data_get_next will automatically free
	 * the list.
	 */
	for(data = mpd_database_get_artists(mo);data != NULL; data = mpd_data_get_next(data))
	{
		/* check if the entry is of the correct type */
		if(data->type == MPD_DATA_TYPE_TAG && data->tag_type == MPD_TAG_ITEM_ARTIST)
		{
			/* Another object to hold data */
			MpdData *albums = NULL;
			printf("%s\n", data->tag);
			for(albums = mpd_database_get_albums(mo, data->tag);
					albums != NULL;
					albums = mpd_data_get_next(albums))
			{
				if(albums->type == MPD_DATA_TYPE_TAG && albums->tag_type == MPD_TAG_ITEM_ALBUM)
				{
					printf("\t%s\n", albums->tag);
				}


			}


		}

	}
	/* clean up the connection and free the space needed by mpdObj */
	mpd_free(mo);
}
