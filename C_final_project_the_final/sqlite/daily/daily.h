#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sqlite3.h>
#ifndef DAILY_H
#define DAILY_H

void d_create_entry(gpointer data);
void d_update_entry(gpointer data);
void d_delete_entry(gpointer data);
void d_load_entries(gpointer data,int year,int month,int day);
void d_initialize_database();
void d_read_entries();

#endif 