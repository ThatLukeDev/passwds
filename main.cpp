#include <gtk/gtk.h>

static void print_hello(GtkWidget *widget, gpointer data) {
	g_print("Hello World\n");
}

static void activate(GtkApplication *app, gpointer user_data) {
	GtkWidget* window = gtk_application_window_new(app);
	gtk_window_set_title (GTK_WINDOW(window), "Passwds");
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

	GtkWidget* grid = gtk_grid_new();
	gtk_window_set_child(GTK_WINDOW(window), grid);

	GtkWidget* button = gtk_button_new_with_label("Hello World");
	g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
	gtk_grid_attach(GTK_GRID(grid), button, 0, 0, 1, 1);

	gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char** argv) {
	#if GLIB_CHECK_VERSION(2, 74, 0)
		GtkApplication* app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	#else
		GtkApplication* app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	#endif
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
