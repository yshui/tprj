#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
#include "uud.h"
#include "uu.h"
#include "defs.h"
const gchar *const null_option[] = {NULL};
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
	uudUDisksDevice *uud;
	uuUDisks *uu;
	uu = uu_udisks_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
			G_DBUS_CONNECTION_FLAGS_NONE,
			UDISKS_NAME,
			UDISKS_OBJ,
			NULL,
			&err);
	if(err)goto gerr;
	gchar *a;
       	uu_udisks_call_find_device_by_device_file_sync(uu, df, &a, NULL, &err);
	if(err)goto gerr;
	uud = uud_udisks_device_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
			G_DBUS_CONNECTION_FLAGS_NONE,
			UDISKS_NAME,
			a,
			NULL,
			&err);
	if(err)goto gerr;
	g_free(a);
//Check if df is mounted
	gboolean ismounted = uud_udisks_device_get_device_is_mounted(uud);
	if(!ismounted){
		fprintf(stderr, "%s not mounted\n", df);
		return 1;
	}
	uud_udisks_device_call_filesystem_unmount_sync(uud, null_option, NULL, &err);
	if(err)goto gerr;
	if(if_detach){
		//Detach self first
		gboolean detachable = uud_udisks_device_get_device_is_drive(uud) && uud_udisks_device_get_drive_can_detach(uud);
		if(detachable)
			uud_udisks_device_call_drive_detach_sync(uud, null_option, NULL, &err);
		if(err)goto gerr;
		a = uud_udisks_device_get_partition_slave(uud);
		uudUDisksDevice *puud = uud_udisks_device_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
				G_DBUS_CONNECTION_FLAGS_NONE,
				UDISKS_NAME,
				a,
				NULL,
				&err);
		if(err)goto gerr;
		detachable = uud_udisks_device_get_drive_can_detach(puud);
		if(detachable)
			uud_udisks_device_call_drive_detach_sync(puud, null_option, NULL, &err);
		if(err)goto gerr;
	}
	return 0;
gerr:
	g_printerr("%s\n", err->message);
	return 1;
}
