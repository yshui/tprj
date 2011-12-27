#!/usr/bin/python2
import dbus
bus = dbus.SystemBus()
udisks = bus.get_object('org.freedesktop.UDisks',
		'/org/freedesktop/UDisks')
dev = udisks.EnumerateDevices(dbus_interface='org.freedesktop.UDisks');
for obj_path in dev:

print dev;

