#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sqlite3.h>
#ifndef WEEKLY_H
#define WEEKLY_H

void w_initialize_database();
void w_create_entry(gpointer data);
void w_read_entries();
void w_update_entry(gpointer data);
void w_delete_entry(gpointer data);
void w_load(gpointer data, int year, int week_of_year);


#endif 