#include <gtk/gtk.h>
#include "Screens/MainScreen/MainScreen.h"

// App觸發器
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWindow *window = (GtkWindow *) gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Daily Planner");
    gtk_widget_set_size_request(GTK_WIDGET(window),800,500);
    // gtk_window_set_decorated(window,false);

    GtkStack *stack = (GtkStack *) gtk_stack_new();
    GtkBuilder *mainBuilder = gtk_builder_new_from_file(ROOT_PATH"/src/MainScreen/MainScreen.ui");

    GtkWidget *main_screen = GTK_WIDGET(gtk_builder_get_object(mainBuilder, "MainScreen"));
    if (main_screen) {
        gtk_stack_add_named(stack, main_screen, "MainScreen");

        MainScreen(mainBuilder);
    } else {
        g_warning("Not found MainScreen Widget");
    }

    const char cssPath[]=ROOT_PATH"/src/MainScreen/MainScreen.css";
    GtkCssProvider * cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, cssPath);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),GTK_STYLE_PROVIDER(cssProvider),GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(stack));
    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_window_present(GTK_WINDOW(window));

    g_object_unref(mainBuilder);
}

int main(int argc, char **argv) {
    // GtkApplication *app;

    GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS); //創建app實例
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL); //連接觸發器
    int status = g_application_run(G_APPLICATION(app), argc, argv); //Run app
    g_object_unref(app); //釋放app

    return status;
}
