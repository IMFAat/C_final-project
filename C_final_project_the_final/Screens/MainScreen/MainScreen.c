#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sqlite3.h>
#include "../../sqlite/daily/daily.h"
#include "../../sqlite/weekly/weekly.h"
#define ms *1000000

// Structure to store daily mission data and associated GTK widgets
typedef struct
{
    int year;                 // Year of the mission
    int month;                // Month of the mission
    int day;                  // Day of the mission
    int begin_hour;           // Start hour of the mission
    int end_hour;             // End hour of the mission
    int begin_minute;         // Start minute of the mission
    int end_minute;           // End minute of the mission
    char *activity;           // Activity name
    int frequency;            // Total frequency (e.g., push-ups "30" times)
    int now_frequency;        // Current execution count
    int week_of_year;         // Week of the year
    GtkWidget *Add_frequency; // Widget to add frequency (GtkButton)
    GtkWidget *RemoveButton;  // Widget for removing the mission (GtkButton)
    GtkWidget *Display_Box;   // Widget to display mission information (GtkBox)
    GtkButton *revise;        // Button to revise added mission.
} Daily_Mission_Data_widget;

// Structure to manage daily mission input widgets
typedef struct
{
    GtkEntry *Activity;          // Entry for activity name
    GtkSpinButton *Frequency;    // Spin button for frequency
    GtkSpinButton *Begin_hour;   // Spin button for start hour
    GtkSpinButton *End_hour;     // Spin button for end hour
    GtkSpinButton *Begin_minute; // Spin button for start minute
    GtkSpinButton *End_minute;   // Spin button for end minute
    GtkButton *Add;              // Button to add a new daily mission
} S_Daily_Widget;

// Structure for managing animation data of the bottom box (+ Button)
struct BottomBoxAnimationData
{
    GtkWidget *backBox;          // Background box widget (GtkBox)
    GtkWidget *button;           // Button widget (GtkButton)
    GtkWidget *contentBox;       // Content box widget (GtkBox)
    GtkWidget *Content_Scrolled; // Scrolled content widget (GtKScrolledWindow)
    int type;                    // 0 for daily, 1 for weekly
    int flag;                    // Animation state flag
};

// Structure to manage weekly mission input widgets
typedef struct
{
    GtkEntry *Activity;       // Entry for activity name
    GtkSpinButton *frequency; // Spin button for frequency
    GtkButton *Add;           // Button to add a new weekly mission
} S_Weekly_Widget;

// Structure to store weekly mission data and associated GTK widgets
typedef struct
{
    int year;                 // Year of creation
    int month;                // Month of creation
    int day;                  // Day of creation
    int day_of_week;          // Day of the week (0=Sun, ..., 6=Sat)
    char *activity;           // Activity name
    int frequency;            // Total frequency
    int now_frequency;        // Current execution count
    int week_of_year;         // Week of the year
    GtkButton *Add_frequency; // Widget to add frequency
    GtkWidget *RemoveButton;  // Widget for removing the mission
    GtkWidget *Display_Box;   // Widget to display mission information
    GtkButton *revise;        // Flag indicating existence of the mission
    bool exists;
} Weekly_Mission_Data_widget;

// Structure to combine daily and weekly mission data and widgets
typedef struct
{
    S_Daily_Widget *daily_widget;                         // Daily widget management
    S_Weekly_Widget *weekly_widget;                       // Weekly widget management
    GArray *daily_data;                                   // Array to store daily mission data
    GArray *weekly_data;                                  // Array to store weekly mission data
    GtkBox *daily_added_box;                              // Box to display added daily missions
    GtkBox *weekly_added_box;                             // Box to display added weekly mission
    struct BottomBoxAnimationData *daily_animation_data;  // Animation data for daily missions
    struct BottomBoxAnimationData *weekly_animation_data; // Animation data for weekly missions
    GtkWidget *drawer;                                    // Box to contain drawer_context
    GtkWidget *drawer_context;                            // Box to contain calendar and label
} Mission_Combination;

// The click event for drawer button
void DrawerButtonOnClick(GtkWidget *button, gpointer data);

// Display the context of drawer (Label + Calendar)
void showDrawerContext(gpointer drawer);

// The select event for calendar in drawer
void drawerCalendarSelect(GtkWidget *calendar, gpointer data);

// Set rounded of widget (Using css)
void set_all_rounded_radius(GtkWidget *widget, float radius, int type);

// Set margin of widget
void set_margin(GtkWidget *widget, float margin, int type);

// The click event for bottom add button
void AddButtonOnClick(GtkWidget *button, gpointer data);

// Add activity to daily
void v_daily_add(GtkWidget *button, gpointer data);

// Destroy signal connect of add process button and remove activity button of daily and weekly activities
void v_signal_destroy(gpointer data);

// Print data info (Use in debug)
void v_daily_data_check(gpointer data);

// Add activity to weekly
void v_weekly_add(GtkWidget *button, gpointer data);

// Create daily activity (DisplayBox) to box
void v_D_create_widget(gpointer data);

// The click event for process +1 (Daily)
void v_D_update_widget(GtkButton *button, gpointer data);

// The click event for remove button (Daily)
void v_D_remove_widget(GtkButton *button, gpointer data);

// Create weekly activity (DisplayBox) to box
void v_W_create_widget(gpointer data);

// The click event for process +1 (Weekly)
void v_W_update_widget(GtkButton *button, gpointer data);

// The click event for remove button (Weekly)
void v_W_remove_widget(GtkButton *button, gpointer data);

// Synchronous daily and weekly
void v_sync(GtkButton *button, gpointer data);

// Connect signal connect of add process button and remove activity button of daily and weekly activities
void v_signal_connect(gpointer data);

// To get the week of this year
int calculate_iso_week_number(int year, int month, int day);

// The event triggered when the daily revise button is clicked
void v_D_revise(GtkButton *button, gpointer data);

// The event triggered when revise the daily activity
void v_D_revise_btn(GtkButton *button, gpointer data);

// The event triggered when the weekly revise button is clicked
void v_W_revise(GtkButton *button, gpointer data);

// The event triggered when revise the daily activity
void v_W_revise_btn(GtkButton *button, gpointer data);

// Check each activity is timeout or not
gboolean v_clock(gpointer data);

// The animation of add button
gboolean AddButton_Animation(gpointer data);

// The animation of drawer
gboolean drawerAnimation(gpointer data);

// The animation of bottom button
gboolean bottomButtonAnimation(gpointer data);

// Round css provider(daily,weekly)
GtkCssProvider *cssProviderRound[2];

// Margin css provider(daily,weekly)
GtkCssProvider *cssProviderMargin[2];

int select_year = 0, select_month = 0, select_day = 0; // The calendar selected date

// Main Screen Function
void MainScreen(GtkBuilder *builder)
{
    d_initialize_database();
    w_initialize_database();

    time_t timer;
    struct tm *now;
    time(&timer);
    now = localtime(&timer);

    char now_week[3];
    strftime(now_week, 3, "%W", now);
    char *now_week_copy = now_week;
    int i_now_week = atoi(now_week_copy);

    // 取得 DrawerIconBox 並加上圖示
    GtkBox *drawerIconBox = (GtkBox *)gtk_builder_get_object(builder, "DrawerIconBox");
    GtkWidget *drawerIconImage = gtk_image_new_from_file(ROOT_PATH "/src/Icons/menu.png");
    gtk_image_set_icon_size((GtkImage *)drawerIconImage, 500);
    gtk_box_append(drawerIconBox, drawerIconImage);

    // 取得按鈕
    GtkWidget *drawerButton = (GtkWidget *)gtk_builder_get_object(builder, "DrawerButton");
    GtkWidget *drawerBox = (GtkWidget *)gtk_builder_get_object(builder, "DrawerBox");

    // Setting spinbutton.
    GtkAdjustment *adjustment_Begin_hour = (GtkAdjustment *)gtk_adjustment_new(0, 0, 23, 1, 0, 0);
    GtkAdjustment *adjustment_Begin_minute = (GtkAdjustment *)gtk_adjustment_new(0, 0, 59, 1, 0, 0);
    GtkAdjustment *adjustment_End_hour = (GtkAdjustment *)gtk_adjustment_new(0, 0, 23, 1, 0, 0);
    GtkAdjustment *adjustment_End_minute = (GtkAdjustment *)gtk_adjustment_new(0, 0, 59, 1, 0, 0);
    GtkAdjustment *adjustment_1 = (GtkAdjustment *)gtk_adjustment_new(1, 1, 9999, 1, 0, 0);
    GtkAdjustment *adjustment_2 = (GtkAdjustment *)gtk_adjustment_new(1, 1, 9999, 1, 0, 0);
    GtkSpinButton *Begin_hour = (GtkSpinButton *)gtk_builder_get_object(builder, "begin_hour");
    GtkSpinButton *Begin_minute = (GtkSpinButton *)gtk_builder_get_object(builder, "begin_minute");
    GtkSpinButton *End_hour = (GtkSpinButton *)gtk_builder_get_object(builder, "end_hour");
    GtkSpinButton *End_minute = (GtkSpinButton *)gtk_builder_get_object(builder, "end_minute");
    GtkSpinButton *Daily_frequency = (GtkSpinButton *)gtk_builder_get_object(builder, "Add_Frequency");
    GtkSpinButton *Weekly_frequency = (GtkSpinButton *)gtk_builder_get_object(builder, "Weekly_Activity_Frequency_TV");

    // Connect adjustment.
    gtk_spin_button_set_adjustment(Begin_hour, adjustment_Begin_hour);
    gtk_spin_button_set_adjustment(Begin_minute, adjustment_Begin_minute);
    gtk_spin_button_set_adjustment(End_hour, adjustment_End_hour);
    gtk_spin_button_set_adjustment(End_minute, adjustment_End_minute);
    gtk_spin_button_set_adjustment(Daily_frequency, adjustment_1);
    gtk_spin_button_set_adjustment(Weekly_frequency, adjustment_2);

    // 禁止手動輸入
    gtk_widget_set_can_focus((GtkWidget *)Begin_hour, FALSE);
    gtk_widget_set_can_focus((GtkWidget *)Begin_minute, FALSE);
    gtk_widget_set_can_focus((GtkWidget *)End_hour, FALSE);
    gtk_widget_set_can_focus((GtkWidget *)End_minute, FALSE);
    gtk_widget_set_can_focus((GtkWidget *)Daily_frequency, FALSE);
    gtk_widget_set_can_focus((GtkWidget *)Weekly_frequency, FALSE);

    // 定義按鈕
    GtkButton *Add_Daily_Activity = (GtkButton *)gtk_builder_get_object(builder, "Add_Daliy_Activity_BT");
    GtkButton *Add_Weekly_Activity = (GtkButton *)gtk_builder_get_object(builder, "Add_Weekly_Activity_BT");

    GtkEntry *TV_Daily_Activity = (GtkEntry *)gtk_builder_get_object(builder, "Add_Daliy_Activity_TV");
    GtkEntry *TV_weekly_Activity = (GtkEntry *)gtk_builder_get_object(builder, "Add_Weekly_Activity_TV");

    // initialize Daily_Widget
    S_Daily_Widget *Daily_Widget = g_new0(S_Daily_Widget, 1);

    Daily_Widget->Activity = (GtkEntry *)TV_Daily_Activity;
    Daily_Widget->Add = (GtkButton *)Add_Daily_Activity;
    Daily_Widget->Begin_hour = (GtkSpinButton *)Begin_hour;
    Daily_Widget->Begin_minute = (GtkSpinButton *)Begin_minute;
    Daily_Widget->End_hour = (GtkSpinButton *)End_hour;
    Daily_Widget->End_minute = (GtkSpinButton *)End_minute;
    Daily_Widget->Frequency = (GtkSpinButton *)Daily_frequency;

    // initialize Weekly_Widget
    S_Weekly_Widget *Weekly_Widget = g_new0(S_Weekly_Widget, 1);

    Weekly_Widget->Activity = (GtkEntry *)TV_weekly_Activity;
    Weekly_Widget->Add = (GtkButton *)Add_Weekly_Activity;
    Weekly_Widget->frequency = (GtkSpinButton *)Weekly_frequency;

    GtkScrolledWindow *Add_Daily_scrolledwindow = (GtkScrolledWindow *)gtk_builder_get_object(
        builder, "Added_Daily_Activity_scrollwindow");
    GtkScrolledWindow *Add_Weekly_scrolledwindow = (GtkScrolledWindow *)gtk_builder_get_object(
        builder, "Added_Weekly_Activity_scrollwindow");
    gtk_scrolled_window_set_policy(Add_Daily_scrolledwindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_overlay_scrolling(Add_Daily_scrolledwindow, TRUE);
    gtk_scrolled_window_set_policy(Add_Weekly_scrolledwindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_overlay_scrolling(Add_Weekly_scrolledwindow, TRUE);

    struct BottomBoxAnimationData *dailyAddButtonAnimationData = g_new0(struct BottomBoxAnimationData, 1);
    dailyAddButtonAnimationData->button = (GtkWidget *)gtk_builder_get_object(builder, "DailyAddButton");
    dailyAddButtonAnimationData->backBox = (GtkWidget *)gtk_builder_get_object(builder, "DailyButtonBox");
    dailyAddButtonAnimationData->contentBox = (GtkWidget *)gtk_builder_get_object(builder, "DailyContentBox");
    dailyAddButtonAnimationData->Content_Scrolled = (GtkWidget *)gtk_builder_get_object(
        builder, "Daily_Content_Scrolled");
    gtk_scrolled_window_set_policy((GtkScrolledWindow *)dailyAddButtonAnimationData->Content_Scrolled,
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_has_frame((GtkScrolledWindow *)dailyAddButtonAnimationData->Content_Scrolled, false);
    dailyAddButtonAnimationData->type = 0;
    dailyAddButtonAnimationData->flag = 0;

    gtk_widget_set_visible((GtkWidget *)dailyAddButtonAnimationData->contentBox, FALSE);
    gtk_widget_set_opacity((GtkWidget *)dailyAddButtonAnimationData->Content_Scrolled, 0);
    gtk_widget_set_opacity((GtkWidget *)dailyAddButtonAnimationData->contentBox, 0);

    struct BottomBoxAnimationData *weeklyAddButtonAnimationData = g_new0(struct BottomBoxAnimationData, 1);
    weeklyAddButtonAnimationData->button = (GtkWidget *)gtk_builder_get_object(builder, "WeeklyAddButton");
    weeklyAddButtonAnimationData->backBox = (GtkWidget *)gtk_builder_get_object(builder, "WeeklyButtonBox");
    weeklyAddButtonAnimationData->contentBox = (GtkWidget *)gtk_builder_get_object(builder, "WeeklyContentBox");
    weeklyAddButtonAnimationData->Content_Scrolled = (GtkWidget *)gtk_builder_get_object(
        builder, "Weekly_Content_Scrolled");
    gtk_scrolled_window_set_policy((GtkScrolledWindow *)weeklyAddButtonAnimationData->Content_Scrolled,
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_has_frame((GtkScrolledWindow *)weeklyAddButtonAnimationData->Content_Scrolled, false);
    weeklyAddButtonAnimationData->type = 1;
    weeklyAddButtonAnimationData->flag = 0;

    gtk_widget_set_visible((GtkWidget *)weeklyAddButtonAnimationData->contentBox, FALSE);
    gtk_widget_set_opacity((GtkWidget *)weeklyAddButtonAnimationData->Content_Scrolled, 0);
    gtk_widget_set_opacity((GtkWidget *)weeklyAddButtonAnimationData->contentBox, 0);

    GArray *g_reg_daily = g_array_new(false, false, sizeof(Daily_Mission_Data_widget));
    GArray *g_reg_weekly = g_array_new(false, false, sizeof(Weekly_Mission_Data_widget));

    Mission_Combination *combine = g_new0(Mission_Combination, 1);
    combine->daily_widget = Daily_Widget;
    combine->weekly_widget = Weekly_Widget;
    combine->daily_data = g_reg_daily;
    combine->weekly_data = g_reg_weekly;
    combine->daily_added_box = (GtkBox *)gtk_builder_get_object(builder, "Added_Daily_Activity_Box");
    combine->weekly_added_box = (GtkBox *)gtk_builder_get_object(builder, "Added_Weekly_Activity_Box");
    combine->daily_animation_data = dailyAddButtonAnimationData;
    combine->weekly_animation_data = weeklyAddButtonAnimationData;
    combine->drawer = drawerBox;

    // 更新信號連接
    g_signal_connect(Daily_Widget->Add, "clicked", G_CALLBACK(v_daily_add), combine);
    g_signal_connect(Weekly_Widget->Add, "clicked", G_CALLBACK(v_weekly_add), combine);
    g_signal_connect(Daily_Widget->Add, "clicked", G_CALLBACK(v_D_revise_btn), combine);
    g_signal_connect(Weekly_Widget->Add, "clicked", G_CALLBACK(v_W_revise_btn), combine);

    // 給定新增事件

    g_signal_connect(dailyAddButtonAnimationData->button, "clicked", G_CALLBACK(AddButtonOnClick),
                     dailyAddButtonAnimationData);
    g_signal_connect(weeklyAddButtonAnimationData->button, "clicked", G_CALLBACK(AddButtonOnClick),
                     weeklyAddButtonAnimationData);

    // 按鈕連接點及觸發器
    g_signal_connect(drawerButton, "clicked", G_CALLBACK(DrawerButtonOnClick), combine);

    cssProviderRound[0] = gtk_css_provider_new();
    cssProviderRound[1] = gtk_css_provider_new();

    cssProviderMargin[0] = gtk_css_provider_new();
    cssProviderMargin[1] = gtk_css_provider_new();

    g_timeout_add(300, (GSourceFunc)v_clock, combine);
    d_load_entries(g_reg_daily, now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
    for (int i = 0; i < g_reg_daily->len; i++)
    {
        Daily_Mission_Data_widget *d_regis = &g_array_index(g_reg_daily, Daily_Mission_Data_widget, i);
        v_D_create_widget(d_regis);
        gtk_box_append((GtkBox *)combine->daily_added_box, (GtkWidget *)d_regis->Display_Box);
    }
    w_load(g_reg_weekly, now->tm_year + 1900, i_now_week);
    for (int i = 0; i < g_reg_weekly->len; i++)
    {
        Weekly_Mission_Data_widget *w_regis = &g_array_index(g_reg_weekly, Weekly_Mission_Data_widget, i);
        v_W_create_widget(w_regis);
        gtk_box_append((GtkBox *)combine->weekly_added_box, (GtkWidget *)w_regis->Display_Box);
    }
    v_signal_connect(combine);

    select_day = now->tm_mday;
    select_month = now->tm_mon + 1;
    select_year = now->tm_year + 1900;
}

bool isDrawerOpen = false;                     // Tracks weather the drawer is open
bool isAnimating = false;                      // Tracks weather the drawer animation is ongoing
bool isDrawerContextShow = false;              // Tracks weather the drawer context is visible
GtkWidget *drawerContextScrolledWindow = NULL; // The ScrolledWindow for drawer context

// The click event for drawer button
void DrawerButtonOnClick(GtkWidget *button, gpointer data)
{
    // 如果動畫有在進行的話就不重複進行動畫
    if (isAnimating)
        return;
    isAnimating = true;
    // 將動畫的 function 每10秒一次執行
    g_timeout_add(10, (GSourceFunc)drawerAnimation, data);
}

// The animation of drawer
gboolean drawerAnimation(gpointer data)
{
    Mission_Combination *combine = data;
    int step = 0;
    GtkWidget *drawer = combine->drawer;

    if (!isDrawerContextShow)
    {
        drawerContextScrolledWindow = (GtkWidget *)gtk_scrolled_window_new();
        GtkWidget *drawerContext = (GtkWidget *)gtk_box_new((GtkOrientation)GTK_ORIENTATION_VERTICAL, 10);
        combine->drawer_context = drawerContext;
        gtk_scrolled_window_set_child((GtkScrolledWindow *)drawerContextScrolledWindow, drawerContext);
        gtk_scrolled_window_set_policy((GtkScrolledWindow *)drawerContextScrolledWindow, GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_NEVER);
        gtk_scrolled_window_set_has_frame((GtkScrolledWindow *)drawerContextScrolledWindow, false);
        gtk_widget_set_name((GtkWidget *)drawerContextScrolledWindow, "DrawerContextScrolledWindow");
        gtk_widget_set_vexpand(drawerContextScrolledWindow, true);

        gtk_box_append((GtkBox *)drawer, drawerContextScrolledWindow);
        showDrawerContext(data);
        isDrawerContextShow = true;
    }

    int dWidth = 20 * (isDrawerOpen ? -1 : 1);
    step++;

    int width = gtk_widget_get_width(drawer);
    width += dWidth;

    if (width > 330)
    {
        width = 330;
        step = 0;
    }
    if (width < 65)
    {
        width = 65;
        step = 0;
    }

    double drawerOpacity = (width - 65) * 1. / (300 - 65);
    gtk_widget_set_size_request(drawer, width, -1);
    gtk_widget_set_opacity(drawerContextScrolledWindow, drawerOpacity);

    if (step == 0)
    {
        isDrawerOpen = !isDrawerOpen;
        if (!isDrawerOpen)
        {
            gtk_box_remove((GtkBox *)drawer, drawerContextScrolledWindow);
        }
        isDrawerContextShow = isDrawerOpen;
        isAnimating = false;
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

// Display the context of drawer (Label + Calendar)
void showDrawerContext(gpointer data)
{
    Mission_Combination *combine = data;
    GtkWidget *drawer = combine->drawer_context;
    GtkWidget *title = gtk_label_new("Select Date");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_widget_set_name(title, "SelectDateTitle");
    gtk_box_append((GtkBox *)drawer, title);

    GtkWidget *calendar = gtk_calendar_new();
    GDateTime *date = g_date_time_new_local(select_year, select_month, select_day, 0, 0, 0);
    gtk_calendar_select_day((GtkCalendar *)calendar, date);
    gtk_widget_set_name(calendar, "SelectCalendar");
    g_signal_connect(calendar, "day-selected", G_CALLBACK(drawerCalendarSelect), data);
    gtk_box_append((GtkBox *)drawer, calendar);
}

int addButtonAnimationDuration = 2;

// The click event for bottom add button
void AddButtonOnClick(GtkWidget *button, gpointer data)
{
    g_timeout_add(2, (GSourceFunc)bottomButtonAnimation, data);
}

// The select event for calendar in drawer
void drawerCalendarSelect(GtkWidget *calendar, gpointer data)
{
    time_t timer;
    struct tm *now;
    time(&timer);
    now = localtime(&timer);

    char now_week[3];
    strftime(now_week, 3, "%W", now);
    char *now_week_copy = now_week;
    int i_now_week = atoi(now_week_copy);

    select_year = gtk_calendar_get_year((GtkCalendar *)calendar);
    select_month = gtk_calendar_get_month((GtkCalendar *)calendar) + 1;
    select_day = gtk_calendar_get_day((GtkCalendar *)calendar);
    int week = calculate_iso_week_number(select_year, select_month, select_day);
    Mission_Combination *combine = data;
    GArray *d_gdata = combine->daily_data;
    GArray *w_gdata = combine->weekly_data;
    GtkBox *daily_added_box = combine->daily_added_box;
    GtkBox *weekly_added_box = combine->weekly_added_box;
    struct BottomBoxAnimationData *DB_widget = combine->daily_animation_data;
    struct BottomBoxAnimationData *WB_widget = combine->weekly_animation_data;
    for (int i = 0; i < d_gdata->len; i++)
    {
        Daily_Mission_Data_widget *d_regis = &g_array_index(d_gdata, Daily_Mission_Data_widget, i);
        gtk_box_remove(daily_added_box, d_regis->Display_Box);
    }
    for (int i = 0; i < w_gdata->len; i++)
    {
        Weekly_Mission_Data_widget *w_regis = &g_array_index(w_gdata, Weekly_Mission_Data_widget, i);
        gtk_box_remove(weekly_added_box, w_regis->Display_Box);
    }
    while (d_gdata->len > 0)
    {
        g_array_remove_index(d_gdata, 0);
    }
    while (w_gdata->len > 0)
    {
        g_array_remove_index(w_gdata, 0);
    }
    d_load_entries(d_gdata, select_year, select_month, select_day);
    for (int i = 0; i < d_gdata->len; i++)
    {
        Daily_Mission_Data_widget *d_regis = &g_array_index(d_gdata, Daily_Mission_Data_widget, i);
        v_D_create_widget(d_regis);
        gtk_box_append((GtkBox *)combine->daily_added_box, (GtkWidget *)d_regis->Display_Box);
    }
    w_load(w_gdata, select_year, week);
    for (int i = 0; i < w_gdata->len; i++)
    {
        Weekly_Mission_Data_widget *w_regis = &g_array_index(w_gdata, Weekly_Mission_Data_widget, i);
        v_W_create_widget(w_regis);
        gtk_box_append((GtkBox *)combine->weekly_added_box, (GtkWidget *)w_regis->Display_Box);
    }
    v_signal_connect(combine);
    if (select_year != now->tm_year + 1900 || select_month != now->tm_mon + 1 || select_day != now->tm_mday)
    {
        gtk_widget_set_sensitive((GtkWidget *)DB_widget->button, false);
        for (int i = 0; i < d_gdata->len; i++)
        {
            Daily_Mission_Data_widget *d_regis = &g_array_index(d_gdata, Daily_Mission_Data_widget, i);
            gtk_widget_set_sensitive((GtkWidget *)d_regis->Add_frequency, false);
            gtk_widget_set_sensitive((GtkWidget *)d_regis->RemoveButton, false);
            gtk_widget_set_sensitive((GtkWidget *)d_regis->revise, false);
        }
    }
    else
    {
        gtk_widget_set_sensitive((GtkWidget *)DB_widget->button, true);
    }
    if (select_year != now->tm_year + 1900 || week != i_now_week)
    {
        gtk_widget_set_sensitive((GtkWidget *)WB_widget->button, false);
        for (int i = 0; i < w_gdata->len; i++)
        {
            Weekly_Mission_Data_widget *w_regis = &g_array_index(w_gdata, Weekly_Mission_Data_widget, i);
            gtk_widget_set_sensitive((GtkWidget *)w_regis->Add_frequency, false);
            gtk_widget_set_sensitive((GtkWidget *)w_regis->RemoveButton, false);
            gtk_widget_set_sensitive((GtkWidget *)w_regis->revise, false);
        }
    }
    else
    {
        gtk_widget_set_sensitive((GtkWidget *)WB_widget->button, true);
    }
    if (DB_widget->flag == 1)
    {
        g_timeout_add(1, (GSourceFunc)bottomButtonAnimation, combine->daily_animation_data);
    }
    if (WB_widget->flag == 1)
    {
        g_timeout_add(1, (GSourceFunc)bottomButtonAnimation, combine->weekly_animation_data);
    }
}

// Set rounded of widget (Using css)
void set_all_rounded_radius(GtkWidget *widget, float radius, int type)
{
    char *class = (char *)malloc(sizeof(char) * 64);
    snprintf(class, 64, "roundedAnimation%d", (type == 0 ? 0 : 1));
    if (!gtk_widget_has_css_class(widget, class))
    {
        gtk_widget_add_css_class(widget, class);
    }
    char *css = (char *)malloc(sizeof(char) * 128);
    snprintf(css, 128, ".roundedAnimation%d { border-radius: %.2fpx; }", (type == 0 ? 0 : 1), radius);
    gtk_css_provider_load_from_string(cssProviderRound[type], css);
    gtk_style_context_remove_provider_for_display(gdk_display_get_default(), (GtkStyleProvider *)cssProviderRound[type]);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(cssProviderRound[type]),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER);

    free(css);
    free(class);
}

// Set margin of widget
void set_margin(GtkWidget *widget, float margin, int type)
{
    char *class = (char *)malloc(sizeof(char) * 64);
    snprintf(class, 64, "marginAnimation%d", (type == 0 ? 0 : 1));
    gtk_widget_add_css_class(widget, class); // 針對widget的動態css
    char *css = (char *)malloc(sizeof(char) * 64);
    snprintf(css, 64, ".marginAnimation%d { margin: %fpx; }", (type == 0 ? 0 : 1), margin);
    gtk_css_provider_load_from_string(cssProviderMargin[type], css);
    gtk_style_context_remove_provider_for_display(gdk_display_get_default(), (GtkStyleProvider *)cssProviderMargin[type]);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(cssProviderMargin[type]),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER);
    free(css);
    free(class);
}

int counter_expand = 50, counter_back = 400;

// The animation of add button
gboolean AddButton_Animation(gpointer data)
{
    struct BottomBoxAnimationData *tisData = (struct BottomBoxAnimationData *)data;
    if (tisData->flag == 1)
    {
        if (counter_expand < 400)
        {
            float margin = 7.5 * (counter_expand - 50) / 350;
            set_margin(tisData->button, margin, tisData->type);
            counter_expand += 25;
            return G_SOURCE_CONTINUE;
        }
        else
        {
            counter_expand = 50;
            return G_SOURCE_REMOVE;
        }
    }
    if (tisData->flag == 0)
    {
        if (counter_back > 50)
        {
            float margin = 7.5 * (counter_back - 50) / 350;
            set_margin(tisData->button, margin, tisData->type);
            counter_back -= 25;
            return G_SOURCE_CONTINUE;
        }
        else
        {
            counter_back = 400;
            return G_SOURCE_REMOVE;
        }
    }
}

int maxBack = 400;
int expand = 50, back = 400;

// The animation of bottom button
gboolean bottomButtonAnimation(gpointer data)
{
    struct BottomBoxAnimationData *tisData = (struct BottomBoxAnimationData *)data;
    if (tisData == NULL || tisData->backBox == NULL || tisData->button == NULL || tisData->contentBox == NULL)
    {
        g_free(tisData);
        printf("ERROR tisData\n");
        return G_SOURCE_REMOVE;
    }
    if (tisData->flag == 0)
    {
        // Open
        gtk_button_set_label((GtkButton *)tisData->button, "-");
        if (expand < 400)
        {
            expand += 6;
            gtk_widget_set_size_request(tisData->backBox, -1, expand);
            float dailyBoxRadius = (float)35.0 + (float)(20 - 40) * (float)(expand - 50) / 350;
            // 50+(15-30)*(h-50)/(300-50)
            set_all_rounded_radius(tisData->backBox, dailyBoxRadius, tisData->type);
            gtk_widget_set_visible(tisData->Content_Scrolled, TRUE);
            gtk_widget_set_visible(tisData->contentBox, TRUE);
            gtk_widget_set_opacity((GtkWidget *)tisData->Content_Scrolled, (float)(expand - 50) / 350);
            gtk_widget_set_opacity((GtkWidget *)tisData->contentBox, (float)(expand - 50) / 350);
            gtk_widget_set_size_request(tisData->contentBox, -1, expand - 50);
            gtk_widget_set_size_request(tisData->Content_Scrolled, -1, expand);
            return G_SOURCE_CONTINUE;
        }
        else
        {
            // Finish Animation
            gtk_widget_set_size_request(tisData->backBox, -1, 400);
            gtk_widget_set_size_request(tisData->contentBox, -1, 400);
            gtk_widget_set_opacity((GtkWidget *)tisData->Content_Scrolled, 1);
            gtk_widget_set_opacity((GtkWidget *)tisData->contentBox, 1);
            g_timeout_add(5, (GSourceFunc)AddButton_Animation, tisData);
            expand = 50;
            tisData->flag = 1;
        }
    }
    else
    {
        // Close
        gtk_button_set_label((GtkButton *)tisData->button, "+");
        if (back > 50)
        {
            back -= 6;
            gtk_widget_set_size_request(tisData->backBox, -1, back);
            float dailyBoxRadius = (float)35.0 + (float)(20 - 40) * (float)(back - 50) / 350;
            set_all_rounded_radius(tisData->backBox, dailyBoxRadius, tisData->type);
            gtk_widget_set_opacity((GtkWidget *)tisData->Content_Scrolled, (float)(back - 50) / 350);
            gtk_widget_set_opacity((GtkWidget *)tisData->contentBox, (float)(back - 50) / 350);
            gtk_widget_set_size_request(tisData->contentBox, -1, back - 50);
            gtk_widget_set_size_request(tisData->Content_Scrolled, -1, back);
            return G_SOURCE_CONTINUE;
        }
        else
        {
            // Finish
            gtk_widget_set_size_request(tisData->backBox, -1, 50);
            g_timeout_add(5, (GSourceFunc)AddButton_Animation, tisData);
            gtk_widget_set_visible(tisData->contentBox, FALSE);
            gtk_widget_set_visible(tisData->Content_Scrolled, FALSE);
            gtk_widget_set_opacity((GtkWidget *)tisData->Content_Scrolled, 0);
            gtk_widget_set_opacity((GtkWidget *)tisData->contentBox, 0);
            back = 400;
            tisData->flag = 0;
        }
    }
    return G_SOURCE_REMOVE;
}

// Add activity to daily
void v_daily_add(GtkWidget *button, gpointer data)
{
    time_t timer;
    struct tm *Now;
    time(&timer);
    Now = localtime(&timer);
    Mission_Combination *combine = data;
    S_Daily_Widget *SD_widget = combine->daily_widget;
    if (!strcmp(gtk_button_get_label((GtkButton *)SD_widget->Add), "ADD") || !strcmp(gtk_button_get_label((GtkButton *)SD_widget->Add), "Invalid time.") || !strcmp(gtk_button_get_label((GtkButton *)SD_widget->Add), "Activity exist.") || !strcmp(gtk_button_get_label((GtkButton *)SD_widget->Add), "Activity can't be null."))
    {
        if (combine->daily_data == NULL || combine->daily_widget == NULL)
        {
            printf("ERROR\n");
            return;
        }
        GArray *g_data = combine->daily_data;
        S_Daily_Widget *widget = combine->daily_widget;

        char now_week[3];
        strftime(now_week, 3, "%W", Now);
        char *now_week_copy = now_week;
        int i_now_week = atoi(now_week_copy);

        Daily_Mission_Data_widget D_data_regis;

        D_data_regis.year = Now->tm_year + 1900;
        D_data_regis.month = Now->tm_mon + 1;
        D_data_regis.day = Now->tm_mday;
        D_data_regis.week_of_year = i_now_week;
        D_data_regis.begin_hour = gtk_spin_button_get_value(widget->Begin_hour);
        D_data_regis.begin_minute = gtk_spin_button_get_value(widget->Begin_minute);
        D_data_regis.end_hour = gtk_spin_button_get_value(widget->End_hour);
        D_data_regis.end_minute = gtk_spin_button_get_value(widget->End_minute);
        D_data_regis.frequency = gtk_spin_button_get_value(widget->Frequency);
        D_data_regis.now_frequency = 0;
        D_data_regis.Add_frequency = (GtkWidget *)gtk_button_new();
        D_data_regis.RemoveButton = (GtkWidget *)gtk_button_new();
        D_data_regis.Display_Box = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
        D_data_regis.revise = (GtkButton *)gtk_button_new();

        if (D_data_regis.begin_hour * 60 + D_data_regis.begin_minute >= D_data_regis.end_hour * 60 + D_data_regis.end_minute)
        {
            gtk_button_set_label(widget->Add, "Invalid time.");
            return;
        }

        gchar *text;
        text = gtk_editable_get_text((GtkEditable *)widget->Activity);
        gchar *save_text;
        save_text = g_malloc(strlen(text) + 1);
        strcpy(save_text, text); // 手動分配靜態記憶體
        D_data_regis.activity = save_text;

        for (int i = 0; i < g_data->len; i++)
        {
            Daily_Mission_Data_widget *regis = &g_array_index(g_data, Daily_Mission_Data_widget, i);
            if (!strcmp(D_data_regis.activity, regis->activity))
            {
                gtk_button_set_label(widget->Add, "Activity exist.");
                return;
            }
        }

        for (int i = 0; i < strlen(D_data_regis.activity); i++)
        {
            if (D_data_regis.activity[i] != ' ')
            {
                break;
            }
            if (i == strlen(D_data_regis.activity) - 1)
            {
                gtk_button_set_label(widget->Add, "Activity can't be null.");
                return;
            }
        }
        if (!strcmp(D_data_regis.activity, ""))
        {
            gtk_button_set_label(widget->Add, "Activity can't be null.");
            return;
        }

        v_D_create_widget(&D_data_regis);

        g_array_append_val(g_data, D_data_regis);

        Daily_Mission_Data_widget *regis = &g_array_index(g_data, Daily_Mission_Data_widget, g_data->len - 1);

        gtk_box_append((GtkBox *)combine->daily_added_box, (GtkWidget *)regis->Display_Box);

        d_create_entry(&g_array_index(g_data, Daily_Mission_Data_widget, g_data->len - 1));

        v_signal_destroy(data);
        v_signal_connect(data);

        gtk_button_set_label(widget->Add, "ADD");
        g_timeout_add(addButtonAnimationDuration, (GSourceFunc)bottomButtonAnimation, combine->daily_animation_data);
        gtk_spin_button_set_value(widget->Begin_hour, 0);
        gtk_spin_button_set_value(widget->Begin_minute, 0);
        gtk_spin_button_set_value(widget->End_hour, 0);
        gtk_spin_button_set_value(widget->End_minute, 0);
        gtk_spin_button_set_value(widget->Frequency, 1);
        gtk_editable_set_text((GtkEditable *)widget->Activity, "");
        d_read_entries();
    }
}

// Add activity to weekly
void v_weekly_add(GtkWidget *button, gpointer data)
{
    time_t t;
    struct tm *Now;
    time(&t);
    Now = localtime(&t);
    Mission_Combination *combine = data;
    if (!strcmp(gtk_button_get_label((GtkButton *)combine->weekly_widget->Add), "ADD") || !strcmp(gtk_button_get_label((GtkButton *)combine->weekly_widget->Add), "Activity exist.") || !strcmp(gtk_button_get_label((GtkButton *)combine->weekly_widget->Add), "Activity can't be null."))
    {
        if (combine->weekly_data == NULL || combine->weekly_widget == NULL)
        {
            printf("ERROR\n");
            return;
        }
        GArray *g_data = combine->weekly_data;
        S_Weekly_Widget *widget = combine->weekly_widget;

        gchar *text, *save_text;
        text = (char *)gtk_editable_get_text((GtkEditable *)widget->Activity);
        save_text = g_malloc(strlen(text) + 1);
        strcpy(save_text, text);

        char now_week[3];
        strftime(now_week, 3, "%W", Now);
        char *now_week_copy = now_week;
        int i_now_week = atoi(now_week_copy);

        Weekly_Mission_Data_widget regis;
        regis.year = Now->tm_year + 1900;
        regis.month = Now->tm_mon + 1;
        regis.day_of_week = Now->tm_wday;
        regis.day = Now->tm_mday;
        regis.week_of_year = i_now_week;
        regis.activity = save_text;
        regis.frequency = gtk_spin_button_get_value((GtkSpinButton *)widget->frequency);
        regis.now_frequency = 0;
        regis.exists = true;
        regis.Add_frequency = (GtkButton *)gtk_button_new();
        regis.Display_Box = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
        regis.RemoveButton = (GtkWidget *)gtk_button_new();
        regis.revise = (GtkButton *)gtk_button_new();

        for (int i = 0; i < g_data->len; i++)
        {
            Weekly_Mission_Data_widget *g_regis = &g_array_index(g_data, Weekly_Mission_Data_widget, i);
            if (!strcmp(g_regis->activity, regis.activity))
            {
                gtk_button_set_label(widget->Add, "Activity exist.");
                return;
            }
        }

        for (int i = 0; i < strlen(regis.activity); i++)
        {
            if (regis.activity[i] != ' ')
            {
                break;
            }
            if (i == strlen(regis.activity) - 1)
            {
                gtk_button_set_label(widget->Add, "Activity can't be null.");
                return;
            }
        }
        if (!strcmp(regis.activity, ""))
        {
            gtk_button_set_label(widget->Add, "Activity can't be null.");
            return;
        }

        g_timeout_add(addButtonAnimationDuration, (GSourceFunc)bottomButtonAnimation, combine->weekly_animation_data);
        gtk_spin_button_set_value(widget->frequency, 1);
        gtk_editable_set_text((GtkEditable *)widget->Activity, "");

        v_W_create_widget(&regis);

        g_array_append_val(g_data, regis);

        Weekly_Mission_Data_widget *g_regis = &g_array_index(g_data, Weekly_Mission_Data_widget, g_data->len - 1);

        gtk_box_append(combine->weekly_added_box, (GtkWidget *)g_regis->Display_Box);

        w_create_entry(&g_array_index(g_data, Weekly_Mission_Data_widget, g_data->len - 1));

        v_signal_destroy(data);
        v_signal_connect(data);

        gtk_button_set_label(widget->Add, "ADD");

        w_read_entries();
    }
}

// Create daily activity (DisplayBox) to box
void v_D_create_widget(gpointer data)
{
    Daily_Mission_Data_widget *regis = data;
    GtkWidget *firstLineHoriBox = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name((GtkWidget *)firstLineHoriBox, "FirstLineHoriBox");

    GtkWidget *activityNameLabel = gtk_label_new(regis->activity);
    gtk_widget_set_name((GtkWidget *)activityNameLabel, "ActivityName");
    gtk_widget_add_css_class((GtkWidget *)activityNameLabel, "DailyScrolledLabel");

    gtk_button_set_label((GtkButton *)regis->Add_frequency, "Process +1");
    gtk_widget_add_css_class((GtkWidget *)regis->Add_frequency, "ProcessButton");

    GtkWidget *removeButton = regis->RemoveButton;
    gtk_widget_add_css_class((GtkWidget *)removeButton, "DailyRemoveButton");

    gtk_box_append((GtkBox *)firstLineHoriBox, activityNameLabel);
    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)regis->Add_frequency);
    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)removeButton);

    gtk_widget_add_css_class((GtkWidget *)regis->revise, "revise_btn");
    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)regis->revise);

    GtkWidget *spaceBox = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spaceBox, true);

    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)spaceBox);

    int frequencyLength = 1, frequency = regis->frequency;
    while (frequency / 10 != 0)
    {
        frequencyLength++;
        frequency /= 10;
    }

    // set and add x/y label
    char frequencyText[frequencyLength * 2 + 4];
    sprintf(frequencyText, "%d / %d", regis->now_frequency, regis->frequency);
    GtkWidget *frequencyLabel = gtk_label_new(frequencyText);
    gtk_widget_set_name(frequencyLabel, "FrequencyLabel");
    gtk_widget_add_css_class((GtkWidget *)frequencyLabel, "DailyScrolledLabel");
    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)frequencyLabel);

    GtkWidget *processBar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction((GtkProgressBar *)processBar, regis->now_frequency * 1. / regis->frequency);
    gtk_widget_add_css_class(processBar, "ProcessBar");
    gtk_widget_set_name(processBar, "ProcessBar");

    GtkWidget *thirdLineHoriBox = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(thirdLineHoriBox, "thirdLineHoriBox");

    char beginTime[6];
    sprintf(beginTime, "%02d:%02d", regis->begin_hour, regis->begin_minute);
    GtkWidget *beginTimeLabel = (GtkWidget *)gtk_label_new(beginTime);
    gtk_widget_set_name(beginTimeLabel, "begin_time_label");
    gtk_box_append((GtkBox *)thirdLineHoriBox, beginTimeLabel);

    GtkWidget *thirdSpaceBox = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(thirdSpaceBox, true);
    gtk_box_append((GtkBox *)thirdLineHoriBox, thirdSpaceBox);

    char endTime[6];
    sprintf(endTime, "%02d:%02d", regis->end_hour, regis->end_minute);
    GtkWidget *endTimeLabel = (GtkWidget *)gtk_label_new(endTime);
    gtk_widget_set_name(endTimeLabel, "end_time_label");
    gtk_box_append((GtkBox *)thirdLineHoriBox, endTimeLabel);

    gtk_box_append((GtkBox *)regis->Display_Box, (GtkWidget *)firstLineHoriBox);
    gtk_box_append((GtkBox *)regis->Display_Box, (GtkWidget *)processBar);
    gtk_box_append((GtkBox *)regis->Display_Box, (GtkWidget *)thirdLineHoriBox);

    gtk_widget_add_css_class(regis->Display_Box, "DailyMissionBoxWidget");

    if (regis->now_frequency >= regis->frequency)
    {
        gtk_button_set_label((GtkButton *)regis->Add_frequency, "Done");
        gtk_widget_set_sensitive((GtkWidget *)regis->Add_frequency, false);
    }
}

// The click event for process +1 (Daily)
void v_D_update_widget(GtkButton *button, gpointer data)
{
    Daily_Mission_Data_widget *regis = data;
    GtkWidget *box = regis->Display_Box;
    GtkWidget *firstLineHoriBox = NULL;
    GtkWidget *frequencyLabel = NULL;
    GtkWidget *processBar = NULL;
    GtkWidget *thirdLineHoriBox = NULL;
    GtkWidget *begin_time_label = NULL;
    GtkWidget *end_time_label = NULL;

    for (GtkWidget *child = gtk_widget_get_first_child(box); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        const char *name = gtk_widget_get_name(child);
        if (strcmp(name, "FirstLineHoriBox") == 0)
            firstLineHoriBox = child;
        if (strcasecmp(name, "ProcessBar") == 0)
            processBar = child;
        if (strcasecmp(name, "thirdLineHoriBox") == 0)
            thirdLineHoriBox = child;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(firstLineHoriBox); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        const char *name = gtk_widget_get_name(child);
        if (strcmp(name, "FrequencyLabel") == 0)
            frequencyLabel = child;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(thirdLineHoriBox); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        const char *name = gtk_widget_get_name(child);
        if (strcmp(name, "begin_time_label") == 0)
            begin_time_label = child;
        if (strcmp(name, "end_time_label") == 0)
            end_time_label = child;
    }
    char beginTime[6];
    sprintf(beginTime, "%02d:%02d", regis->begin_hour, regis->begin_minute);
    gtk_label_set_label((GtkLabel *)begin_time_label, beginTime);

    char endTime[6];
    sprintf(endTime, "%02d:%02d", regis->end_hour, regis->end_minute);
    gtk_label_set_label((GtkLabel *)end_time_label, endTime);

    regis->now_frequency++;

    gtk_progress_bar_set_fraction((GtkProgressBar *)processBar, regis->now_frequency * 1. / regis->frequency);

    int frequencyLength = 1, frequency = regis->frequency;
    while (frequency / 10 != 0)
    {
        frequencyLength++;
        frequency /= 10;
    }
    char frequencyText[frequencyLength * 2 + 4];
    sprintf(frequencyText, "%d / %d", regis->now_frequency, regis->frequency);

    gtk_label_set_label((GtkLabel *)frequencyLabel, frequencyText);

    if (regis->now_frequency >= regis->frequency)
    {
        gtk_widget_set_sensitive((GtkWidget *)regis->Add_frequency, false);
        gtk_button_set_label((GtkButton *)regis->Add_frequency, "Done");
    }

    d_update_entry(data);
    d_read_entries();
}

// The click event for remove button (Daily)
void v_D_remove_widget(GtkButton *button, gpointer data)
{
    Mission_Combination *combine = data;
    GArray *g_data = combine->daily_data;
    Daily_Mission_Data_widget *d_regis;
    for (int i = 0; i < g_data->len; i++)
    {
        d_regis = &g_array_index(g_data, Daily_Mission_Data_widget, i);
        if (d_regis->RemoveButton == button)
        {
            break;
        }
    }
    int removeIndex = 0;
    for (int i = 0; i < g_data->len; i++)
    {
        Daily_Mission_Data_widget *d = &g_array_index(g_data, Daily_Mission_Data_widget, i);
        if (d->Display_Box == d_regis->Display_Box)
        {
            gtk_box_remove((GtkBox *)combine->daily_added_box, (GtkWidget *)d->Display_Box);
            removeIndex = i;
            break;
        }
    }
    d_delete_entry(d_regis);
    d_read_entries();
    g_array_remove_index(g_data, removeIndex);

    v_signal_destroy(data);
    v_signal_connect(data);
}

// Create weekly activity (DisplayBox) to box
void v_W_create_widget(gpointer data)
{
    Weekly_Mission_Data_widget *regis = data;
    GtkWidget *firstLineHoriBox = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name((GtkWidget *)firstLineHoriBox, "FirstLineHoriBox");

    GtkWidget *activityNameLabel = gtk_label_new(regis->activity);
    gtk_widget_set_name((GtkWidget *)activityNameLabel, "ActivityName");
    gtk_widget_add_css_class((GtkWidget *)activityNameLabel, "DailyScrolledLabel");

    gtk_button_set_label((GtkButton *)regis->Add_frequency, "Process +1");
    gtk_widget_add_css_class((GtkWidget *)regis->Add_frequency, "ProcessButton");

    GtkWidget *removeButton = regis->RemoveButton;
    gtk_widget_add_css_class((GtkWidget *)removeButton, "DailyRemoveButton");

    gtk_widget_add_css_class((GtkWidget *)regis->revise, "revise_btn");

    gtk_box_append((GtkBox *)firstLineHoriBox, activityNameLabel);
    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)regis->Add_frequency);
    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)removeButton);
    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)regis->revise);

    GtkWidget *spaceBox = (GtkWidget *)gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spaceBox, true);

    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)spaceBox);

    int frequencyLength = 1, frequency = regis->frequency;
    while (frequency / 10 != 0)
    {
        frequencyLength++;
        frequency /= 10;
    }

    // set and add x/y label
    char frequencyText[frequencyLength * 2 + 4];
    sprintf(frequencyText, "%d / %d", regis->now_frequency, regis->frequency);
    GtkWidget *frequencyLabel = gtk_label_new(frequencyText);
    gtk_widget_set_name(frequencyLabel, "FrequencyLabel");
    gtk_widget_add_css_class((GtkWidget *)frequencyLabel, "DailyScrolledLabel");
    gtk_box_append((GtkBox *)firstLineHoriBox, (GtkWidget *)frequencyLabel);

    GtkWidget *processBar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction((GtkProgressBar *)processBar, regis->now_frequency * 1. / regis->frequency);
    gtk_widget_add_css_class(processBar, "ProcessBar");
    gtk_widget_set_name(processBar, "ProcessBar");

    gtk_box_append((GtkBox *)regis->Display_Box, (GtkWidget *)firstLineHoriBox);
    gtk_box_append((GtkBox *)regis->Display_Box, (GtkWidget *)processBar);

    gtk_widget_add_css_class(regis->Display_Box, "DailyMissionBoxWidget");

    if (regis->now_frequency >= regis->frequency)
    {
        gtk_button_set_label((GtkButton *)regis->Add_frequency, "Done");
        gtk_widget_set_sensitive((GtkWidget *)regis->Add_frequency, false);
    }
}

// The click event for process +1 (Weekly)
void v_W_update_widget(GtkButton *button, gpointer data)
{
    Weekly_Mission_Data_widget *regis = data;
    GtkWidget *box = regis->Display_Box;
    GtkWidget *firstLineHoriBox = NULL;
    GtkWidget *frequencyLabel = NULL;
    GtkWidget *processBar = NULL;

    for (GtkWidget *child = gtk_widget_get_first_child(box); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        const char *name = gtk_widget_get_name(child);
        if (strcmp(name, "FirstLineHoriBox") == 0)
            firstLineHoriBox = child;
        if (strcasecmp(name, "ProcessBar") == 0)
            processBar = child;
    }
    for (GtkWidget *child = gtk_widget_get_first_child(firstLineHoriBox); child != NULL;
         child = gtk_widget_get_next_sibling(child))
    {
        const char *name = gtk_widget_get_name(child);
        if (strcmp(name, "FrequencyLabel") == 0)
            frequencyLabel = child;
    }

    regis->now_frequency++;

    gtk_progress_bar_set_fraction((GtkProgressBar *)processBar, regis->now_frequency * 1. / regis->frequency);

    int frequencyLength = 1, frequency = regis->frequency;
    while (frequency / 10 != 0)
    {
        frequencyLength++;
        frequency /= 10;
    }
    char frequencyText[frequencyLength * 2 + 4];
    sprintf(frequencyText, "%d / %d", regis->now_frequency, regis->frequency);

    gtk_label_set_label((GtkLabel *)frequencyLabel, frequencyText);

    if (regis->now_frequency >= regis->frequency)
    {
        gtk_widget_set_sensitive((GtkWidget *)regis->Add_frequency, false);
        gtk_button_set_label((GtkButton *)regis->Add_frequency, "Done");
    }
    else
    {
        gtk_widget_set_sensitive((GtkWidget *)regis->Add_frequency, true);
        gtk_button_set_label((GtkButton *)regis->Add_frequency, "Process +1");
    }

    w_update_entry(data);
    w_read_entries();
}

// The click event for remove button (Weekly)
void v_W_remove_widget(GtkButton *button, gpointer data)
{
    Mission_Combination *combine = data;
    GArray *g_data = combine->weekly_data;
    Weekly_Mission_Data_widget *w_regis;
    for (int i = 0; i < g_data->len; i++)
    {
        w_regis = &g_array_index(g_data, Weekly_Mission_Data_widget, i);
        if (w_regis->RemoveButton == button)
        {
            break;
        }
    }
    int removeIndex = 0;
    for (int i = 0; i < g_data->len; i++)
    {
        Weekly_Mission_Data_widget *d = &g_array_index(g_data, Weekly_Mission_Data_widget, i);
        if (d->Display_Box == w_regis->Display_Box)
        {
            gtk_box_remove((GtkBox *)combine->weekly_added_box, (GtkWidget *)d->Display_Box);
            removeIndex = i;
            break;
        }
    }
    g_array_remove_index(g_data, removeIndex);

    w_delete_entry(w_regis);
    w_read_entries();

    v_signal_destroy(data);
    v_signal_connect(data);
}

// Check each activity is timeout or not
gboolean v_clock(gpointer data)
{
    Mission_Combination *combine = data;
    GArray *g_data = combine->daily_data;
    time_t timer;
    struct tm *Now;
    GtkButton *button;
    time(&timer);
    Now = localtime(&timer);
    for (int i = 0; i < g_data->len; i++)
    {
        Daily_Mission_Data_widget *regis = &g_array_index(g_data, Daily_Mission_Data_widget, i);
        if (regis->end_hour * 60 + regis->end_minute < Now->tm_hour * 60 + Now->tm_min)
        {
            if (regis->now_frequency >= regis->frequency)
            {
                gtk_button_set_label((GtkButton *)regis->Add_frequency, "Done");
                gtk_widget_set_sensitive(regis->Add_frequency, false);
            }
            else
            {
                gtk_button_set_label((GtkButton *)regis->Add_frequency, "time is over");
                gtk_widget_set_sensitive(regis->Add_frequency, false);
            }
        }
        else if (regis->begin_hour * 60 + regis->begin_minute >= Now->tm_hour * 60 + Now->tm_min)
        {
            gtk_button_set_label((GtkButton *)regis->Add_frequency, "Not started yet");
            gtk_widget_set_sensitive(regis->Add_frequency, false);
        }
        else
        {
            if (regis->now_frequency >= regis->frequency)
            {
                gtk_button_set_label((GtkButton *)regis->Add_frequency, "Done");
                gtk_widget_set_sensitive(regis->Add_frequency, false);
            }
            else
            {
                gtk_button_set_label((GtkButton *)regis->Add_frequency, "Process +1");
                gtk_widget_set_sensitive(regis->Add_frequency, true);
            }
        }
    }
    return G_SOURCE_CONTINUE;
}
// Synchronous daily and weekly
void v_sync(GtkButton *button, gpointer data)
{
    Mission_Combination *combine = data;
    GArray *daily_data = g_array_new(FALSE, FALSE, sizeof(Daily_Mission_Data_widget));
    GArray *weekly_data = g_array_new(FALSE, FALSE, sizeof(Weekly_Mission_Data_widget));
    daily_data = combine->daily_data;
    weekly_data = combine->weekly_data;
    Daily_Mission_Data_widget *d_regis;
    Weekly_Mission_Data_widget *w_regis;
    for (int i = 0; i < daily_data->len; i++)
    {
        d_regis = &g_array_index(daily_data, Daily_Mission_Data_widget, i);
        if (button == d_regis->Add_frequency)
        {
            break;
        }
    }
    for (int i = 0; i < weekly_data->len; i++)
    {
        w_regis = &g_array_index(weekly_data, Weekly_Mission_Data_widget, i);
        if (!strcmp(d_regis->activity, w_regis->activity))
        {
            break;
        }
        if (strcmp(d_regis->activity, w_regis->activity) && i == weekly_data->len - 1)
        {
            w_regis = NULL;
        }
    }
    if (w_regis == NULL)
    {
        return;
    }
    if (w_regis->now_frequency + 1 <= w_regis->frequency)
    {
        printf("d %s\n", d_regis->activity);
        printf("d %s\n", w_regis->activity);
        v_W_update_widget(w_regis->Add_frequency, w_regis);
    }
}

// Connect signal connect of add process button and remove activity button of daily and weekly activities
void v_signal_connect(gpointer data)
{
    Mission_Combination *combine = data;
    GArray *a_d_regis = combine->daily_data;
    GArray *a_w_regis = combine->weekly_data;
    Daily_Mission_Data_widget *d_regis;
    Weekly_Mission_Data_widget *w_regis;
    for (int i = 0; i < a_d_regis->len; i++)
    {
        d_regis = &g_array_index(a_d_regis, Daily_Mission_Data_widget, i);
        g_signal_connect(d_regis->Add_frequency, "clicked", G_CALLBACK(v_D_update_widget), d_regis);
        g_signal_connect(d_regis->RemoveButton, "clicked", G_CALLBACK(v_D_remove_widget), data);
        g_signal_connect(d_regis->Add_frequency, "clicked", G_CALLBACK(v_sync), data);
        g_signal_connect(d_regis->revise, "clicked", G_CALLBACK(v_D_revise), data);
    }
    for (int i = 0; i < a_w_regis->len; i++)
    {
        w_regis = &g_array_index(a_w_regis, Weekly_Mission_Data_widget, i);
        g_signal_connect(w_regis->Add_frequency, "clicked", G_CALLBACK(v_W_update_widget), w_regis);
        g_signal_connect(w_regis->RemoveButton, "clicked", G_CALLBACK(v_W_remove_widget), data);
        g_signal_connect(w_regis->revise, "clicked", G_CALLBACK(v_W_revise), data);
    }
}

// Destroy signal connect of add process button and remove activity button of daily and weekly activities
void v_signal_destroy(gpointer data)
{
    Mission_Combination *combine = data;
    GArray *a_d_regis = combine->daily_data;
    GArray *a_w_regis = combine->weekly_data;
    Daily_Mission_Data_widget *d_regis;
    Weekly_Mission_Data_widget *w_regis;
    for (int i = 0; i < a_d_regis->len; i++)
    {
        d_regis = &g_array_index(a_d_regis, Daily_Mission_Data_widget, i);
        g_signal_handlers_destroy(d_regis->Add_frequency);
        g_signal_handlers_destroy(d_regis->RemoveButton);
        g_signal_handlers_destroy(d_regis->revise);
    }
    for (int i = 0; i < a_w_regis->len; i++)
    {
        w_regis = &g_array_index(a_w_regis, Weekly_Mission_Data_widget, i);
        g_signal_handlers_destroy(w_regis->Add_frequency);
        g_signal_handlers_destroy(w_regis->RemoveButton);
        g_signal_handlers_destroy(w_regis->revise);
    }
}
// Calculate the current week number of the year
int calculate_iso_week_number(int year, int month, int day)
{
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = 12;

    if (mktime(&timeinfo) == -1)
    {
        printf("Error: mktime failed to convert date.\n");
        return 0;
    }

    char buffer[3];
    strftime(buffer, sizeof(buffer), "%W", &timeinfo);
    return atoi(buffer);
}

//Change the ADD button's label to REVISE, display the box for REVISE, and disable the SHOW button
void v_D_revise(GtkButton *button, gpointer data)
{
    Mission_Combination *combine = data;
    S_Daily_Widget *SD_widget = combine->daily_widget;
    gtk_button_set_label((GtkButton *)SD_widget->Add, "REVISE");
    if (combine->daily_animation_data->flag == 0)
    {
        g_timeout_add(2, (GSourceFunc)bottomButtonAnimation, combine->daily_animation_data);
    }
    GArray *d_gdata = combine->daily_data;
    Daily_Mission_Data_widget *d_regis;
    for (int i = 0; i < d_gdata->len; i++)
    {
        d_regis = &g_array_index(d_gdata, Daily_Mission_Data_widget, i);
        if (button == d_regis->revise)
        {
            break;
        }
    }
    gtk_editable_set_text((GtkEditable *)SD_widget->Activity, d_regis->activity);
    gtk_widget_set_sensitive((GtkWidget *)SD_widget->Activity, false);
    gtk_spin_button_set_value((GtkSpinButton *)SD_widget->Frequency, d_regis->frequency);
    gtk_spin_button_set_value((GtkSpinButton *)SD_widget->Begin_hour, d_regis->begin_hour);
    gtk_spin_button_set_value((GtkSpinButton *)SD_widget->Begin_minute, d_regis->begin_minute);
    gtk_spin_button_set_value((GtkSpinButton *)SD_widget->End_hour, d_regis->end_hour);
    gtk_spin_button_set_value((GtkSpinButton *)SD_widget->End_hour, d_regis->end_hour);
    gtk_widget_set_sensitive((GtkWidget *)combine->daily_animation_data->button, false);
}

//Modify the value in the array
void v_D_revise_btn(GtkButton *button, gpointer data)
{
    Mission_Combination *combine = data;
    GArray *d_gdata = combine->daily_data;
    Daily_Mission_Data_widget *d_regis;
    if (!strcmp(gtk_button_get_label((GtkButton *)combine->daily_widget->Add), "REVISE") || !strcmp(gtk_button_get_label((GtkButton *)combine->daily_widget->Add), "Invalid frequency.") || !strcmp(gtk_button_get_label((GtkButton *)combine->daily_widget->Add), "Invalid time setting."))
    {
        for (int i = 0; i < d_gdata->len; i++)
        {
            d_regis = &g_array_index(d_gdata, Daily_Mission_Data_widget, i);
            if (!strcmp(d_regis->activity, gtk_editable_get_text((GtkEditable *)combine->daily_widget->Activity)))
            {
                break;
            }
        }

        if (gtk_spin_button_get_value(combine->daily_widget->Frequency) < d_regis->now_frequency)
        {
            gtk_button_set_label((GtkButton *)combine->daily_widget->Add, "Invalid frequency.");
            return;
        }
        if (gtk_spin_button_get_value(combine->daily_widget->Begin_hour) * 60 + gtk_spin_button_get_value(combine->daily_widget->Begin_minute) >= gtk_spin_button_get_value(combine->daily_widget->End_hour) * 60 + gtk_spin_button_get_value(combine->daily_widget->End_minute))
        {
            gtk_button_set_label((GtkButton *)combine->daily_widget->Add, "Invalid time setting.");
            return;
        }
        d_regis->frequency = gtk_spin_button_get_value(combine->daily_widget->Frequency);
        d_regis->begin_hour = gtk_spin_button_get_value(combine->daily_widget->Begin_hour);
        d_regis->begin_minute = gtk_spin_button_get_value(combine->daily_widget->Begin_minute);
        d_regis->end_hour = gtk_spin_button_get_value(combine->daily_widget->End_hour);
        d_regis->end_minute = gtk_spin_button_get_value(combine->daily_widget->End_minute);
        d_regis->now_frequency--;

        g_timeout_add(2, (GSourceFunc)bottomButtonAnimation, combine->daily_animation_data);
        v_D_update_widget(button, d_regis);
        gtk_button_set_label((GtkButton *)combine->daily_widget->Add, "ADD");
        gtk_widget_set_sensitive((GtkWidget *)combine->daily_widget->Activity, true);
        gtk_editable_set_text((GtkEditable *)combine->daily_widget->Activity, "");
        gtk_spin_button_set_value(combine->daily_widget->Frequency, 1);
        gtk_spin_button_set_value(combine->daily_widget->Begin_hour, 0);
        gtk_spin_button_set_value(combine->daily_widget->Begin_minute, 0);
        gtk_spin_button_set_value(combine->daily_widget->End_hour, 0);
        gtk_spin_button_set_value(combine->daily_widget->End_minute, 0);
        gtk_widget_set_sensitive((GtkWidget *)combine->daily_animation_data->button, true);
    }
}

//Change the ADD button's label to REVISE, display the box for REVISE, and disable the SHOW button
void v_W_revise(GtkButton *button, gpointer data)
{
    Mission_Combination *combine = data;
    GArray *w_gdata = combine->weekly_data;
    Weekly_Mission_Data_widget *w_regis;
    for (int i = 0; i < w_gdata->len; i++)
    {
        w_regis = &g_array_index(w_gdata, Weekly_Mission_Data_widget, i);
        if (button == w_regis->revise)
        {
            break;
        }
    }
    gtk_button_set_label((GtkButton *)combine->weekly_widget->Add, "REVISE");
    gtk_widget_set_sensitive((GtkWidget *)combine->weekly_widget->Activity, false);
    gtk_editable_set_text((GtkEditable *)combine->weekly_widget->Activity, w_regis->activity);
    gtk_spin_button_set_value((GtkSpinButton *)combine->weekly_widget->frequency, w_regis->frequency);
    if (combine->weekly_animation_data->flag == 0)
    {
        g_timeout_add(2, (GSourceFunc)bottomButtonAnimation, combine->weekly_animation_data);
    }
    gtk_widget_set_sensitive((GtkWidget *)combine->weekly_animation_data->button, false);
}

//Modify the value in the array
void v_W_revise_btn(GtkButton *button, gpointer data)
{
    Mission_Combination *combine = data;
    GArray *w_gdata = combine->weekly_data;
    Weekly_Mission_Data_widget *w_regis;
    if (!strcmp(gtk_button_get_label((GtkButton *)combine->weekly_widget->Add), "REVISE") || !strcmp(gtk_button_get_label((GtkButton *)combine->weekly_widget->Add), "Invalid frequency."))
    {
        for (int i = 0; i < w_gdata->len; i++)
        {
            w_regis = &g_array_index(w_gdata, Weekly_Mission_Data_widget, i);
            if (!strcmp(w_regis->activity, gtk_editable_get_text((GtkEditable *)combine->weekly_widget->Activity)))
            {
                break;
            }
        }

        if (gtk_spin_button_get_value(combine->weekly_widget->frequency) < w_regis->now_frequency)
        {
            gtk_button_set_label((GtkButton *)combine->weekly_widget->Add, "Invalid frequency.");
            return;
        }
        w_regis->frequency = gtk_spin_button_get_value((GtkSpinButton *)combine->weekly_widget->frequency);
        w_regis->now_frequency--;

        g_timeout_add(2, (GSourceFunc)bottomButtonAnimation, combine->weekly_animation_data);
        v_W_update_widget(button, w_regis);
        gtk_button_set_label((GtkButton *)combine->weekly_widget->Add, "ADD");
        gtk_widget_set_sensitive((GtkWidget *)combine->weekly_widget->Activity, true);
        gtk_editable_set_text((GtkEditable *)combine->weekly_widget->Activity, "");
        gtk_spin_button_set_value(combine->weekly_widget->frequency, 0);
        gtk_widget_set_sensitive((GtkWidget *)combine->weekly_animation_data->button, true);
    }
}