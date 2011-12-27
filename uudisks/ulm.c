#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
#include "ludisks.h"
int main(){
	g_type_init();
	GError *err=NULL;
	gsize len;
	gchar **a = enumerate_device(&len, &err);
	if(err){
		g_printerr("%s\n", err->message);
		return 1;
	}
	GDBusProxy **pros = g_malloc0(sizeof(GDBusProxy *)*(len+1));
	int i;
	for(i=0; i<len; i++){
		pros[i] = get_proxy_by_obj(a[i], &err);
		if(err)
			goto gerr;
	}
	int uid = (int)getuid();
	GDBusProxy **u = get_mounted(pros);
	GDBusProxy **v = match_uid(u, uid);
	//g_free(u);
	for(i=0;v[i];i++)
		printf("%s\n", get_device_file(v[i]));
	return 0;
gerr:
	g_printerr("%s\n", err->message);
	return 1;
}
