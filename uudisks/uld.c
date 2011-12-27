#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
#include "ludisks.h"
int main(int argc, const char **argv){
	g_type_init();
	int i;
	GError *err = NULL;
	int if_detach=0;
	const char *df = 0;
	for(i=1; i<argc; i++){
		if(argv[i][0] == '-'){
			int j;
			for(j=1; argv[i][j]; j++)
				switch(argv[i][j]){
					case 'd': if_detach=1;break;
				}
		}
		else df = argv[i];
	}
	if(!df)return 1;
	gchar *a = get_obj_by_df(df, &err);
	GDBusProxy *b = get_proxy_by_obj(a, &err);
	g_free(a);
	if(err)goto gerr;
	do_unmount(b, &err);
	if(err)goto gerr;
	if(if_detach){
		a = get_parent_obj(b);
		GDBusProxy *c = get_proxy_by_obj(a, &err);
		if(err)goto gerr;
		do_detach(c, &err);
		if(err)goto gerr;
	}
	return 0;
gerr:
	g_printerr("%s\n", err->message);
	return 1;
}
