#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sqlite3.h>

#define DB_NAME "calendar.db"

typedef struct
{
    int year;
    int month;
    int day;
    int begin_hour;
    int end_hour;
    int begin_minute;
    int end_minute;
    char *activity;
    int frequency;
    int now_frequency;
    int week_of_year;
    GtkWidget *Add_frequency;
    GtkWidget *RemoveButton;
    GtkWidget *Display_Box;
    GtkButton *revise;
} Daily_Mission_Data_widget;

// Function to initialize the database and create the Calendar table
void d_initialize_database()
{
    sqlite3 *db;
    char *err_msg = NULL;

    const char *sql_create_table =
        "CREATE TABLE IF NOT EXISTS Calendar("
        "Year INTEGER,"
        "Month INTEGER,"
        "Day INTEGER,"
        "Begin_Hour INTEGER,"
        "Begin_Minute INTEGER,"
        "End_Hour INTEGER,"
        "End_Minute INTEGER,"
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
void d_create_entry(gpointer data)
{
    sqlite3 *db;
    char *err_msg = NULL;
    Daily_Mission_Data_widget *regis = data;
    int str_len = strlen(regis->activity);
    char sql[str_len + 200];
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    snprintf(sql, sizeof(sql),
             "INSERT INTO Calendar (Year, Month, Day, Begin_Hour, Begin_Minute, End_Hour, End_Minute, Activity, Frequency, Now_Frequency, Week_of_year) "
             "VALUES (%d, %d, %d, %d, %d, %d, %d, '%s', %d, %d, %d);",
             regis->year, regis->month, regis->day, regis->begin_hour, regis->begin_minute, regis->end_hour, regis->end_minute, regis->activity, regis->frequency, regis->now_frequency, regis->week_of_year);

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

// Function to read all entries from the Calendar table
void d_read_entries()
{
    sqlite3 *db;
    sqlite3_stmt *res;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql = "SELECT * FROM Calendar;";

    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("Year | Month | Day | Begin_Hour | Begin_Minute | End_Hour | End_Minute | Activity | Frequency | Now_Frequency | Week_of_year\n");
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        printf("%d | %d | %d | %d | %d | %d | %d | %s | %d | %d | %d \n",
               sqlite3_column_int(res, 0), sqlite3_column_int(res, 1), sqlite3_column_int(res, 2),
               sqlite3_column_int(res, 3), sqlite3_column_int(res, 4), sqlite3_column_int(res, 5),
               sqlite3_column_int(res, 6), sqlite3_column_text(res, 7), sqlite3_column_int(res, 8),
               sqlite3_column_int(res, 9), sqlite3_column_int(res, 10));
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}

// Function to update an entry based on year, month, activity, and day
void d_update_entry(gpointer data)
{
    sqlite3 *db;
    char *err_msg = NULL;
    Daily_Mission_Data_widget *d_regis = data;
    char sql[strlen(d_regis->activity) + 200];

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    snprintf(sql, sizeof(sql),
             "UPDATE Calendar SET Now_Frequency = %d ,Frequency=%d, Begin_Hour=%d , Begin_Minute=%d , End_Hour=%d , End_Minute=%d WHERE Year = %d AND Month = %d AND Day = %d AND Activity = '%s';",
             d_regis->now_frequency, d_regis->frequency, d_regis->begin_hour, d_regis->begin_minute, d_regis->end_hour, d_regis->end_minute, d_regis->year, d_regis->month, d_regis->day, d_regis->activity);

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    else
    {
        printf("Entry updated successfully.\n\n");
    }

    sqlite3_close(db);
}

// Function to delete an entry based on year, month, activity, and day
void d_delete_entry(gpointer data)
{
    sqlite3 *db;
    char *err_msg = NULL;
    Daily_Mission_Data_widget *d_regis = data;
    char sql[strlen(d_regis->activity) + 200];

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    snprintf(sql, sizeof(sql), "DELETE FROM Calendar WHERE Year = %d AND Month = %d AND Day = %d AND ACTIVITY='%s';", d_regis->year, d_regis->month, d_regis->day, d_regis->activity);

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
void d_load_entries(gpointer data, int year, int month, int day)
{
    sqlite3 *db;
    sqlite3_stmt *res;
    GArray *g_data = data;
    Daily_Mission_Data_widget regis;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM Calendar WHERE YEAR=%d AND MONTH=%d AND DAY=%d;", year, month, day);

    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    while (sqlite3_step(res) == SQLITE_ROW)
    {
        char *str;
        str = malloc(strlen(sqlite3_column_text(res, 7)) + 1);
        strcpy(str, sqlite3_column_text(res, 7));
        regis.year = sqlite3_column_int(res, 0);
        regis.month = sqlite3_column_int(res, 1);
        regis.day = sqlite3_column_int(res, 2);
        regis.begin_hour = sqlite3_column_int(res, 3);
        regis.begin_minute = sqlite3_column_int(res, 4);
        regis.end_hour = sqlite3_column_int(res, 5);
        regis.end_minute = sqlite3_column_int(res, 6);
        regis.activity = str;
        regis.frequency = sqlite3_column_int(res, 8);
        regis.now_frequency = sqlite3_column_int(res, 9);
        regis.week_of_year = sqlite3_column_int(res, 10);
        regis.Add_frequency = (GtkWidget *)gtk_button_new();
        regis.RemoveButton = (GtkWidget *)gtk_button_new();
        regis.Display_Box = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
        regis.revise = (GtkButton *)gtk_button_new();
        g_array_append_val(g_data, regis);
    }
    sqlite3_finalize(res);
    sqlite3_close(db);
}
