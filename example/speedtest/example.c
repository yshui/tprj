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

#include "../../src/libmpdclient.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
	char *string;
	mpd_Connection * conn;
	conn = mpd_newConnection("192.150.0.108",6600,10);

	if(conn->error) {
		fprintf(stderr,"%s\n",conn->errorStr);
		mpd_closeConnection(conn);
		return -1;
	}
	int i=0;
//	for(i=0;i<1000;i++)
//	{
/*/	
	mpd_sendCommandsCommand(conn);
	while (( string = mpd_getNextCommand(conn)) != NULL)
	{
		//printf("%s\n", string);
		free(string);
	}
	mpd_finishCommand(conn);
    */
//	}
//
//
//
//



    
    for(i=0;i<100;i++)
    {
        mpd_sendPlChangesCommand(conn,0); 
        mpd_InfoEntity *ent = NULL;
        while((ent = mpd_getNextInfoEntity(conn)) != NULL){
//            		printf("%s\n", ent->info.song->artist);
            mpd_freeInfoEntity(ent);
        }
    }
	mpd_closeConnection(conn);

	return 0;
}
