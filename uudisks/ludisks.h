#pragma once
gchar **enumerate_device(gsize *, GError **);
GDBusProxy **match_uid(GDBusProxy **, guint);
GDBusProxy **get_mounted(GDBusProxy **);
GDBusProxy *get_proxy_by_obj(const gchar *, GError **);
gchar *get_device_file(GDBusProxy *);
gchar *get_parent_obj(GDBusProxy *);
void do_detach(GDBusProxy *, GError **);
gchar *get_obj_by_df(const gchar *, GError **);
