#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
gchar **enumerate_device(gsize *len, GError **err){
	GDBusProxy *proxy = NULL;
	proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			"org.freedesktop.UDisks",
			"/org/freedesktop/UDisks",
			"org.freedesktop.UDisks",
			NULL,
			err);
	if (proxy == NULL) return NULL;
	GVariant *res = g_dbus_proxy_call_sync(proxy,
			"EnumerateDevices",
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			err);
	GVariantIter *iter = g_variant_iter_new(res);
	GVariant *res2 = g_variant_iter_next_value(iter);
	gchar **ret = g_variant_dup_objv(res2, len);
	g_variant_unref(res2);
	g_variant_iter_free(iter);
	g_variant_unref(res);
	g_object_unref(proxy);
	return ret;
}
GDBusProxy **match_uid(GDBusProxy **pro, guint uid){
	int i;
	GDBusProxy **ret = g_malloc0(sizeof(GDBusProxy *)*10);
	int count=0, size=10;
	for(i=0; pro[i] != NULL; i++){
		GVariant *res2 = g_dbus_proxy_get_cached_property(pro[i], "DeviceMountedByUid");
		guint muid = g_variant_get_uint32(res2);
		if(muid == uid || uid == 0){
			ret[count++] = pro[i];
			if(count >= size){
				ret = g_realloc(ret, sizeof(void *)*size*2);
				size = size*2;
			}
		}
		g_variant_unref(res2);
	}
	ret[count] = NULL;
	return ret;
}
GDBusProxy **get_mounted(GDBusProxy **pro){
	int i;
	GDBusProxy **ret = g_malloc0(sizeof(GDBusProxy *)*10);
	int count=0, size=10;
	for(i=0; pro[i] != NULL; i++){
		GVariant *res = g_dbus_proxy_get_cached_property(pro[i], "DeviceIsMounted");
		gboolean b = g_variant_get_boolean(res);
		if(b){
			ret[count++] = pro[i];
			if(count >= size){
				ret = realloc(ret, sizeof(void *)*size*2);
				size = size*2;
			}
		}
		g_variant_unref(res);
	}
	ret[count] = NULL;
	return ret;
}
GDBusProxy *get_proxy_by_obj(const gchar *obj, GError **err){
	GDBusProxy *pro = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			"org.freedesktop.UDisks",
			obj,
			"org.freedesktop.UDisks.Device",
			NULL,
			err);
	return pro;
}
gchar *get_device_file(GDBusProxy *pro){
	if(pro == NULL) return NULL;
	GVariant *res = g_dbus_proxy_get_cached_property(pro, "DeviceFile");
	gchar *df = g_variant_dup_string(res, NULL);
	g_variant_unref(res);
	return df;
}
gchar *get_obj_by_df(const gchar *obj, GError **err){
	GDBusProxy *proxy = NULL;
	proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			"org.freedesktop.UDisks",
			"/org/freedesktop/UDisks",
			"org.freedesktop.UDisks",
			NULL,
			err);
	if (proxy == NULL) return NULL;
	GVariant *res = g_dbus_proxy_call_sync(proxy,
			"FindDeviceByDeviceFile",
			g_variant_new("(s)",obj),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			err);
	GVariantIter *iter = g_variant_iter_new(res);
	GVariant *res2 = g_variant_iter_next_value(iter);
	gchar *ret = g_variant_dup_string(res2, NULL);
	g_variant_unref(res2);
	g_variant_iter_free(iter);
	g_variant_unref(res);
	g_object_unref(proxy);
	return ret;
}
gchar *get_parent_obj(GDBusProxy *pro){
	return g_variant_get_string(g_dbus_proxy_get_cached_property(pro, "PartitionSlave"), NULL);
}
void do_detach(GDBusProxy *pro, GError **err){
	g_dbus_proxy_call_sync(pro,
			"DriveDetach",
			g_variant_new("(as)", NULL),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			err);
}
void do_unmount(GDBusProxy *pro, GError **err){
	GVariant *res = g_dbus_proxy_call_sync(pro,
			"FilesystemUnmount",
			g_variant_new("(as)", NULL),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			err);
	if(res)
		printf("%s\n", g_variant_get_type_string(res));
}

