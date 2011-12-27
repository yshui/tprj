#include "uu.h"
#include "uud.h"
#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
static void device_added(uuUDisks *ud, const gchar *dev){
	GError *err = NULL;
	uudUDisksDevice *uud = uud_udisks_device_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			"org.freedesktop.UDisks",
			dev,
			NULL,
			&err);
	if(err)goto end;
	GVariant *res = g_dbus_proxy_get_cached_property(G_DBUS_PROXY(uud), "DeviceIsPartition");
	gboolean b = g_variant_get_boolean(res);
	if(b){
		GVariant *res2 = g_dbus_proxy_get_cached_property(G_DBUS_PROXY(uud), "DeviceFile");
		const gchar *b2 = g_variant_get_string(res2, NULL);
		gchar *a;
		gchar *b[1];
		b[0]=NULL;
		uud_udisks_device_call_filesystem_mount_sync(uud, "auto", b, &a, NULL, &err);
		printf("Mounted at %s\n", a);
		if(err)goto end;
		g_variant_unref(res2);
		g_free(a);
	}
end:
	if(err)g_error_free(err);
	if(res)g_variant_unref(res);
	if(uud)g_object_unref(uud);
}
int main(){
	g_type_init();
	GMainLoop *loop = g_main_loop_new(NULL, 0);
	GError *err = NULL;
	uuUDisks *ud = uu_udisks_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			"org.freedesktop.UDisks",
			"/org/freedesktop/UDisks",
			NULL,
			&err);
	g_signal_connect(ud,
			"device-added",
			G_CALLBACK(device_added),
			NULL);
/*	g_signal_connect(ud,
			"device-removed",
			G_CALLBACK(device_removed),
			NULL);
*/
	g_main_loop_run(loop);
	g_main_loop_unref(loop);
	g_object_unref(ud);
}
