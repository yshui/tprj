#include <gio/gio.h>
#include <glib/gtypes.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <string.h>
#include "mpd.h"
struct _account{
	GDBusProxy *acc;
	gchar *obj;
	gchar *accobj;
	gchar *status;
	gchar *status_message;
};
struct tp_presence{
	int accc;
	struct _account **acc;
};
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
GDBusProxy *get_account_dbus_proxy(const gchar *obj, GError **error){
	gchar *tmp = get_connection_bus_name(obj);
	GDBusProxy *proxy = NULL;
	g_print("name: %s\n", tmp);
	proxy = g_dbus_proxy_new_for_bus_sync(
			G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			"org.freedesktop.Telepathy.AccountManager",
			obj,
			"org.freedesktop.Telepathy.Account",
			NULL,
			error);
	if(*error)
		g_printerr("%s\n", (*error)->message);
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
void tp_get_presence(GDBusProxy *proxy, struct _account *acc){
	GVariant *res = g_dbus_proxy_get_cached_property(proxy, "CurrentPresence");
	guint32 type;
	gchar *status, *message;
	g_variant_get(res, "(uss)", &type, &status, &message);
	acc->status = status;
	acc->status_message = message;
}
int tp_set_presence(const gchar *obj, const gchar *s, const gchar *m){
	GError *error = NULL;
	GDBusProxy *proxy = get_simplepresence_dbus_proxy(obj, &error);
	if(error){
		g_printerr("%s\n", error->message);
		g_error_free(error);
		error = NULL;
		return -1;
	}
	g_dbus_proxy_call_sync(
			proxy,
			"SetPresence",
			g_variant_new("(ss)",
				s,
				m),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error);
	if(error){
		g_printerr("%s\n", error->message);
		g_error_free(error);
		error = NULL;
		return -1;
	}
	return 0;
}
int tp_presence_init(struct tp_presence **p){
	*p = (struct tp_presence *)malloc(sizeof(struct tp_presence));
	memset(*p, 0, sizeof(struct tp_presence));
	(*p)->acc = (struct _account **)malloc(sizeof(void *));
	int size = 1;
	GError *error = NULL;
	gchar **accounts = get_available_accounts(&error);
	if(!accounts){
		g_printerr("%s\n", error->message);
		g_error_free(error);
		return -1;
	}
	int accc=0;
	for(;*accounts;){
		g_print("obj path: %s\n",*accounts);
		gchar *tmp = get_connection(*accounts, &error);
		if(tmp){
			GDBusProxy *tmpproxy = get_account_dbus_proxy(*accounts, &error);
			if(!tmpproxy){
				if(error){
					g_printerr("%s\n", error->message);
					g_error_free(error);
					error = NULL;
				}
			}else{
				if(accc >= size){
					(*p)->acc = (struct _account **)realloc((*p)->acc, size*2*sizeof(void *));
					size <<=1;
				}
				(*p)->acc[accc] = (struct _account *)malloc(sizeof(struct _account));
				(*p)->acc[accc]->acc = tmpproxy;
				(*p)->acc[accc]->accobj = *accounts;
				tp_get_presence(tmpproxy, (*p)->acc[accc]);
				(*p)->acc[accc]->obj = tmp;
				accc++;
				tmpproxy = NULL;
			}
		}
		else g_printerr("%s\n", error->message);
		accounts++;
	}
	(*p)->accc = accc;
	return 0;
}
int update_status(struct mpd_song *s, void *pp){
	gchar *name = get_song_name_clever(s);
	struct tp_presence *p = pp;
	int i;
	for(i=0;i<p->accc;i++){
		gchar *new_message = g_strdup_printf("%s %s", p->acc[i]->status_message, name);
		g_print("%s\b", new_message);
		tp_set_presence(p->acc[i]->obj, p->acc[i]->status, new_message);
	}
	g_free(name);
	return 0;
}
int main(int argc, const char **argv){
/*
	if(argc!=2){
	g_printerr("Usage: %s [status messgae]\n", argv[0]);
	return 0;
	}
*/

	g_type_init();

	struct tp_presence *p;
	tp_presence_init(&p);
	mpd_init();
	mpd_main_loop(update_status, p);
	return 0;
}
