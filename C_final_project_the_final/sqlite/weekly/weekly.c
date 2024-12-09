#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sqlite3.h>

#define DB_NAME "Weekly.db"

typedef struct
{
    int year;
    int month;
    int day;
    int day_of_week;
    char *activity;
    int frequency;
    int now_frequency;
    int week_of_year;
    GtkButton *Add_frequency;
    GtkWidget *RemoveButton;
    GtkWidget *Display_Box;
    GtkButton *revise;
    bool exists;
} Weekly_Mission_Data_widget;

// Function to initialize the database and create the Weekly table
void w_initialize_database()
{
    sqlite3 *db;
    char *err_msg = NULL;

    const char *sql_create_table =
        "CREATE TABLE IF NOT EXISTS Weekly("
        "Year INTEGER,"
        "Month INTEGER,"
        "Day INTEGER,"
        "Day_of_week INTEGER,"
        "Activity TEXT,"
        "Frequency INTEGER,"
        "Now_Frequency INTEGER,"
        "Week_of_year INTEGER);";

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_exec(db, sql_create_table, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    else
    {
        printf("Table initialized successfully.\n");
    }

    sqlite3_close(db);
}

// Function to create a new calendar entry
void w_create_entry(gpointer data)
{
    sqlite3 *db;
    char *err_msg = NULL;
    Weekly_Mission_Data_widget *w_regis = data;

    char sql[strlen(w_regis->activity) + 200];

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    snprintf(sql, sizeof(sql),
             "INSERT INTO Weekly (Year, Month, Day, Day_of_week, Activity, Frequency, Now_Frequency, Week_of_year) "
             "VALUES (%d, %d, %d, %d, '%s', %d, %d, %d);",
             w_regis->year, w_regis->month, w_regis->day, w_regis->day_of_week, w_regis->activity, w_regis->frequency, w_regis->now_frequency, w_regis->week_of_year);

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    else
    {
        printf("Entry created successfully.\n");
    }

    sqlite3_close(db);
}

// Function to read all entries from the Weekly table
void w_read_entries()
{
    sqlite3 *db;
    sqlite3_stmt *res;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql = "SELECT * FROM Weekly;";

    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("Year | Month | Day | Day_of_week | Activity | Frequency | Now_Frequency | Week_of_year\n");
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        printf("%d | %d | %d | %d | %s | %d | %d | %d\n",
               sqlite3_column_int(res, 0), sqlite3_column_int(res, 1), sqlite3_column_int(res, 2),
               sqlite3_column_int(res, 3), sqlite3_column_text(res, 4), sqlite3_column_int(res, 5),
               sqlite3_column_int(res, 6), sqlite3_column_int(res, 7));
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}

// Function to update an entry based on year, month, day and activity
void w_update_entry(gpointer data)
{
    sqlite3 *db;
    char *err_msg = NULL;
    Weekly_Mission_Data_widget *w_regis = data;
    char sql[strlen(w_regis->activity) + 200];

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    snprintf(sql, sizeof(sql),
             "UPDATE Weekly SET Now_Frequency = %d , frequency = %d WHERE Year = %d AND Month = %d AND Day = %d AND Activity = '%s';",
             w_regis->now_frequency, w_regis->frequency, w_regis->year, w_regis->month, w_regis->day, w_regis->activity);

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    else
    {
        printf("Entry updated successfully.\n");
    }

    sqlite3_close(db);
}

// Function to delete an entry based on year, week_of_year and activity
void w_delete_entry(gpointer data)
{
    sqlite3 *db;
    char *err_msg = NULL;
    Weekly_Mission_Data_widget *w_regis = data;
    char sql[strlen(w_regis->activity) + 200];

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    snprintf(sql, sizeof(sql), "DELETE FROM Weekly WHERE Year = %d AND Week_of_year = %d AND Activity = '%s';", w_regis->year, w_regis->week_of_year, w_regis->activity);

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    else
    {
        printf("Entry deleted successfully.\n");
    }

    sqlite3_close(db);
}

// Load the existing data in the table
void w_load(gpointer data, int year, int week_of_year)
{
    sqlite3 *db;
    sqlite3_stmt *res;
    GArray *g_data = data;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char sql[256];

    snprintf(sql, sizeof(sql), "SELECT * FROM Weekly WHERE YEAR=%d AND Week_of_year=%d;", year, week_of_year);

    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        Weekly_Mission_Data_widget w_regis;
        char *str = malloc(strlen(sqlite3_column_text(res, 4)) + 1);
        strcpy(str, sqlite3_column_text(res, 4));
        w_regis.year = sqlite3_column_int(res, 0);
        w_regis.month = sqlite3_column_int(res, 1);
        w_regis.day = sqlite3_column_int(res, 2);
        w_regis.day_of_week = sqlite3_column_int(res, 3);
        w_regis.activity = str;
        w_regis.frequency = sqlite3_column_int(res, 5);
        w_regis.now_frequency = sqlite3_column_int(res, 6);
        w_regis.week_of_year = sqlite3_column_int(res, 7);
        w_regis.Add_frequency = (GtkButton *)gtk_button_new();
        w_regis.Display_Box = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
        w_regis.RemoveButton = (GtkWidget *)gtk_button_new();
        w_regis.revise = (GtkButton *)gtk_button_new();
        g_array_append_val(g_data, w_regis);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}

