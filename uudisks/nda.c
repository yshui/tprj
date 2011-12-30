#include "uu.h"
#include "uud.h"
#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
const gchar *null_option[1]={NULL};
const gchar **get_filesystem_specific_options(const gchar *fstype){
	//TODO Suport read options from config file
	return null_option;
}
static void device_added(uuUDisks *ud, const gchar *dev){
	GError *err = NULL;
	uudUDisksDevice *uud = uud_udisks_device_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			"org.freedesktop.UDisks",
			dev,
			NULL,
			&err);
	if(err)goto end;
	gboolean b = uud_udisks_device_get_device_is_partition(uud);
	if(b || (0 == strcmp("filesystem", uud_udisks_device_get_id_usage(uud)))){
		const gchar *b2 = uud_udisks_device_get_device_file(uud);
		gchar *mount_path;
		const gchar * const *mount_opt = null_option;
		const gchar *fstype = "auto";
		if(0 == strcmp("filesystem", uud_udisks_device_get_id_usage(uud))){
			fstype = uud_udisks_device_get_id_type(uud);
			mount_opt = get_filesystem_specific_options(fstype);
		}
		uud_udisks_device_call_filesystem_mount_sync(uud, fstype, mount_opt, &mount_path, NULL, &err);
		printf("Mounted at %s\n", mount_path);
		if(err)goto end;
		g_free(mount_path);
	}
end:
	if(err)g_error_free(err);
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
	//Make sure udisks-daemon is started;
	gchar **b;
	uu_udisks_call_enumerate_devices_sync(ud, &b, NULL, &err);
	g_signal_connect(ud,
			"device-added",
			G_CALLBACK(device_added),
			NULL);
	g_main_loop_run(loop);
	g_main_loop_unref(loop);
	g_object_unref(ud);
}
