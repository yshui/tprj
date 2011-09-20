#include <gio/gio.h>
gchar **get_available_accounts(GError **error){
	GDBusProxy *proxy = NULL;
	proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			"org.freedesktop.Telepathy.AccountManager",
			"/org/freedesktop/Telepathy/AccountManager",
			"org.freedesktop.Telepathy.AccountManager",
			NULL,
			error);
	if (proxy == NULL) return NULL;
	GVariant *res = g_dbus_proxy_get_cached_property(proxy, "ValidAccounts");
	gchar **ret = g_variant_dup_objv(res, NULL);
	g_variant_unref(res);
	g_object_unref(proxy);
	return ret;
}
gchar *get_connection(const gchar *obj, GError **error){
	GDBusProxy *proxy = NULL;
	proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			"org.freedesktop.Telepathy.AccountManager",
			obj,
			"org.freedesktop.Telepathy.Account",
			NULL,
			error);
	if (proxy == NULL) return NULL;
	GVariant *res = g_dbus_proxy_get_cached_property(proxy, "Connection");
	gchar *ret = g_variant_dup_string(res, NULL);
	g_object_unref(proxy);
	return ret;
}
gchar *get_connection_bus_name(const gchar *obj){
	static GRegex *regex1 = NULL, *regex2 = NULL;
	GError *error = NULL;
	if(regex1 == NULL)
		regex1 = g_regex_new("^/", 0, 0, &error);
	if(regex2 == NULL)
		regex2 = g_regex_new("/", 0, 0, &error);
	gchar *tmp1 = g_regex_replace_literal(regex1, obj, -1, 0, "", 0, &error);
	gchar *tmp2 = g_regex_replace_literal(regex2, tmp1, -1, 0, ".", 0, &error);
	g_free(tmp1);
	return tmp2;
}
GDBusProxy *get_connection_dbus_proxy(const gchar *obj, GError **error){
	gchar *tmp = get_connection_bus_name(obj);
	GDBusProxy *proxy = NULL;
	g_print("name: %s\n", tmp);
	proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			tmp,
			obj,
			"org.freedesktop.Telepathy.Connection",
			NULL,
			error);
	return proxy;
}
GDBusProxy *get_simplepresence_dbus_proxy(const gchar *obj, GError **error){
	gchar *tmp = get_connection_bus_name(obj);
	GDBusProxy *proxy = NULL;
	g_print("name: %s\n", tmp);
	proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			tmp,
			obj,
			"org.freedesktop.Telepathy.Connection.Interface.SimplePresence",
			NULL,
			error);
	return proxy;
}
int main(int argc, const char **argv){
/*
	if(argc!=2){
	g_printerr("Usage: %s [status messgae]\n", argv[0]);
	return 0;
	}
*/
	GError *error = NULL;

	g_type_init();

	gchar **accounts = get_available_accounts(&error);
	if(!accounts){
		g_printerr("%s\n", error->message);
		g_error_free(error);
		return 0;
	}
	for(;*accounts;){
		g_print("obj path: %s\n",*accounts);
		gchar *tmp = get_connection(*accounts, &error);
		if(tmp){
			g_print("connection: %s\n", tmp);
			GDBusProxy *tmpproxy = get_simplepresence_dbus_proxy(tmp, &error);
			if(!tmpproxy){
				if(error){
					g_printerr("%s\n", error->message);
					g_error_free(error);
					error = NULL;
				}
			}else{
				g_dbus_proxy_call_sync(
						tmpproxy,
						"SetPresence",
						g_variant_new("(ss)",
							"available",
							argv[1]),
						G_DBUS_CALL_FLAGS_NONE,
						-1,
						NULL,
						&error);
				if(error){
					g_printerr("%s\n", error->message);
					g_error_free(error);
					error = NULL;
				}
				g_object_unref(tmpproxy);
				tmpproxy = NULL;
			}
		}
		else g_printerr("%s\n", error->message);
		accounts++;
	}
	return 0;
}
