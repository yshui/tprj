#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
#include "uu.h"
#include "uud.h"
#include "defs.h"
int main(){
	g_type_init();
	GError *err=NULL;
	gchar **a;
	uuUDisks *uu = uu_udisks_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
			G_DBUS_CONNECTION_FLAGS_NONE,
			UDISKS_NAME,
			UDISKS_OBJ,
			NULL,
			&err);
	uu_udisks_call_enumerate_devices_sync(uu, &a, NULL, &err);
	if(err)goto gerr;
	int i;
	int uid = (int)getuid();
	//g_free(u);
	for(i=0;a[i];i++){
		uudUDisksDevice *tmp_uud = uud_udisks_device_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
				G_DBUS_CONNECTION_FLAGS_NONE,
				UDISKS_NAME,
				a[i],
				NULL,
				&err);
		if(err)goto gerr;
		gboolean b = uud_udisks_device_get_device_is_mounted(tmp_uud);
		guint muid = uud_udisks_device_get_device_mounted_by_uid(tmp_uud);
		if(b && (muid == uid))
			printf("%s\n", uud_udisks_device_get_device_file(tmp_uud));
	}
	return 0;
gerr:
	g_printerr("%s\n", err->message);
	return 1;
}
