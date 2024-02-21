#include <gtk/gtk.h>

#include "generate.hpp"
#include "evaluate.hpp"

bool preventNextLengthChange = false;

GtkWidget* genWindow;

GtkWidget* checkWindow;
GtkWidget* checkScoreLabel;
GtkWidget* checkLengthLabel;
GtkWidget* checkDigitLabel;
GtkWidget* checkLowerLabel;
GtkWidget* checkUpperLabel;
GtkWidget* checkSpecialLabel;

GtkWidget* nameEntry;
GtkWidget* passwdEntry;

static void generate(GtkWidget* widget, gpointer data) {
	gtk_window_present(GTK_WINDOW(genWindow));
}

static void check(GtkWidget* widget, gpointer data) {
	passEval result = evalPass((char*)gtk_editable_get_text(GTK_EDITABLE(passwdEntry)));

	gtk_label_set_text(GTK_LABEL(checkScoreLabel), g_strdup_printf("Score: %i%%", int(result.score * 100)));
	gtk_label_set_text(GTK_LABEL(checkLengthLabel), g_strdup_printf("Length: %i%%", int(result.len * 100)));
	gtk_label_set_text(GTK_LABEL(checkDigitLabel), g_strdup_printf("Digits: %i%%", int(result.digit * 100)));
	gtk_label_set_text(GTK_LABEL(checkLowerLabel), g_strdup_printf("Lowercase: %i%%", int(result.lower * 100)));
	gtk_label_set_text(GTK_LABEL(checkUpperLabel), g_strdup_printf("Uppercase: %i%%", int(result.upper * 100)));
	gtk_label_set_text(GTK_LABEL(checkSpecialLabel), g_strdup_printf("Symbols: %i%%", int(result.special * 100)));

	gtk_window_present(GTK_WINDOW(checkWindow));
}

static void save(GtkWidget* widget, gpointer data) {
	g_print("save\n");
}

static void load(GtkWidget* widget, gpointer data) {
	g_print("load\n");
}

static void activate(GtkApplication* app, gpointer user_data) {
	// main window
	GtkWidget* window = gtk_application_window_new(app);
	gtk_window_set_title (GTK_WINDOW(window), "Passwds");
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 16);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), true);
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), true);
	gtk_widget_set_margin_top(grid, 8);
	gtk_widget_set_margin_bottom(grid, 8);
	gtk_widget_set_margin_start(grid, 8);
	gtk_widget_set_margin_end(grid, 8);
	gtk_window_set_child(GTK_WINDOW(window), grid);

	nameEntry = gtk_entry_new();
	GtkWidget* nameLabel = gtk_label_new_with_mnemonic("Name: ");
	gtk_label_set_mnemonic_widget(GTK_LABEL(nameLabel), nameEntry);
	gtk_label_set_xalign(GTK_LABEL(nameLabel), 1.0f);
	gtk_grid_attach(GTK_GRID(grid), nameLabel, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), nameEntry, 1, 0, 3, 1);

	passwdEntry = gtk_entry_new();
	GtkWidget* passwdLabel = gtk_label_new_with_mnemonic("Password: ");
	gtk_label_set_mnemonic_widget(GTK_LABEL(passwdLabel), passwdEntry);
	gtk_label_set_xalign(GTK_LABEL(passwdLabel), 1.0f);
	gtk_grid_attach(GTK_GRID(grid), passwdLabel, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), passwdEntry, 1, 1, 3, 1);

	GtkWidget* generateButton = gtk_button_new_with_label("Generate");
	g_signal_connect(generateButton, "clicked", G_CALLBACK(generate), NULL);
	gtk_grid_attach(GTK_GRID(grid), generateButton, 0, 2, 2, 1);

	GtkWidget* checkButton = gtk_button_new_with_label("Check");
	g_signal_connect(checkButton, "clicked", G_CALLBACK(check), NULL);
	gtk_grid_attach(GTK_GRID(grid), checkButton, 2, 2, 2, 1);

	GtkWidget* saveButton = gtk_button_new_with_label("Save");
	g_signal_connect(saveButton, "clicked", G_CALLBACK(save), NULL);
	gtk_grid_attach(GTK_GRID(grid), saveButton, 0, 3, 2, 1);

	GtkWidget* loadButton = gtk_button_new_with_label("Load");
	g_signal_connect(loadButton, "clicked", G_CALLBACK(load), NULL);
	gtk_grid_attach(GTK_GRID(grid), loadButton, 2, 3, 2, 1);


	// generate window
	genWindow = gtk_window_new();
	gtk_window_set_title (GTK_WINDOW(genWindow), "Generate");
	gtk_window_set_default_size(GTK_WINDOW(genWindow), 300, 200);
	gtk_window_set_hide_on_close(GTK_WINDOW(genWindow), true);

	GtkWidget* genGrid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(genGrid), 16);
	gtk_grid_set_row_spacing(GTK_GRID(genGrid), 16);
	gtk_grid_set_column_homogeneous(GTK_GRID(genGrid), true);
	gtk_grid_set_row_homogeneous(GTK_GRID(genGrid), true);
	gtk_widget_set_margin_top(genGrid, 8);
	gtk_widget_set_margin_bottom(genGrid, 8);
	gtk_widget_set_margin_start(genGrid, 8);
	gtk_widget_set_margin_end(genGrid, 8);
	gtk_window_set_child(GTK_WINDOW(genWindow), genGrid);

	GtkWidget* lengthLabel = gtk_label_new_with_mnemonic("Length: ");
	static GtkWidget* lengthSlider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1.0, 24.0, 1.0);
	static GtkWidget* lengthEntry = gtk_entry_new();
	gtk_label_set_mnemonic_widget(GTK_LABEL(lengthLabel), lengthSlider);
	gtk_label_set_xalign(GTK_LABEL(lengthLabel), 1.0f);
	g_signal_connect(lengthSlider, "value-changed", G_CALLBACK(+[](GtkWidget* widget, gpointer data) {
		if (preventNextLengthChange) {
			preventNextLengthChange = false;
			return;
		}
		int val = int(gtk_range_get_value(GTK_RANGE(lengthSlider)));
		gtk_editable_set_text(GTK_EDITABLE(lengthEntry), g_strdup_printf("%i", val));
	}), NULL);
	g_signal_connect(lengthEntry, "changed", G_CALLBACK(+[](GtkWidget* widget, gpointer data) {
		preventNextLengthChange = true;
		int val = atoi(gtk_editable_get_text(GTK_EDITABLE(lengthEntry)));
		gtk_range_set_value(GTK_RANGE(lengthSlider), val);
	}), NULL);
	gtk_grid_attach(GTK_GRID(genGrid), lengthLabel, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(genGrid), lengthSlider, 1, 0, 2, 1);
	gtk_grid_attach(GTK_GRID(genGrid), lengthEntry, 3, 0, 1, 1);

	GtkWidget* genButton = gtk_button_new_with_label("Generate");
	g_signal_connect(genButton, "clicked", G_CALLBACK(+[](GtkWidget* widget, gpointer data) {
		int len = atoi(gtk_editable_get_text(GTK_EDITABLE(lengthEntry)));
		char randPass[len + 1];
		populateRand(randPass, len + 1);
		gtk_editable_set_text(GTK_EDITABLE(passwdEntry), g_strdup_printf("%s", (char*)randPass));
	}), NULL);
	gtk_grid_attach(GTK_GRID(genGrid), genButton, 0, 1, 4, 1);

	gtk_editable_set_text(GTK_EDITABLE(lengthEntry), g_strdup_printf("24"));


	// check window
	checkWindow = gtk_window_new();
	gtk_window_set_title (GTK_WINDOW(checkWindow), "Generate");
	gtk_window_set_default_size(GTK_WINDOW(checkWindow), 200, 300);
	gtk_window_set_hide_on_close(GTK_WINDOW(checkWindow), true);

	GtkWidget* checkGrid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(checkGrid), 16);
	gtk_grid_set_row_spacing(GTK_GRID(checkGrid), 16);
	gtk_grid_set_column_homogeneous(GTK_GRID(checkGrid), true);
	gtk_grid_set_row_homogeneous(GTK_GRID(checkGrid), true);
	gtk_widget_set_margin_top(checkGrid, 8);
	gtk_widget_set_margin_bottom(checkGrid, 8);
	gtk_widget_set_margin_start(checkGrid, 8);
	gtk_widget_set_margin_end(checkGrid, 8);
	gtk_window_set_child(GTK_WINDOW(checkWindow), checkGrid);

	checkScoreLabel = gtk_label_new_with_mnemonic("Score: ");
	gtk_label_set_xalign(GTK_LABEL(checkScoreLabel), 0.0f);
	gtk_grid_attach(GTK_GRID(checkGrid), checkScoreLabel, 0, 0, 1, 2);

	checkLengthLabel = gtk_label_new_with_mnemonic("Length: ");
	gtk_label_set_xalign(GTK_LABEL(checkLengthLabel), 0.0f);
	gtk_grid_attach(GTK_GRID(checkGrid), checkLengthLabel, 0, 2, 1, 1);

	checkDigitLabel = gtk_label_new_with_mnemonic("Digits: ");
	gtk_label_set_xalign(GTK_LABEL(checkDigitLabel), 0.0f);
	gtk_grid_attach(GTK_GRID(checkGrid), checkDigitLabel, 0, 3, 1, 1);

	checkLowerLabel = gtk_label_new_with_mnemonic("Lowercase: ");
	gtk_label_set_xalign(GTK_LABEL(checkLowerLabel), 0.0f);
	gtk_grid_attach(GTK_GRID(checkGrid), checkLowerLabel, 0, 4, 1, 1);

	checkUpperLabel = gtk_label_new_with_mnemonic("Uppercase: ");
	gtk_label_set_xalign(GTK_LABEL(checkUpperLabel), 0.0f);
	gtk_grid_attach(GTK_GRID(checkGrid), checkUpperLabel, 0, 5, 1, 1);

	checkSpecialLabel = gtk_label_new_with_mnemonic("Symbols: ");
	gtk_label_set_xalign(GTK_LABEL(checkSpecialLabel), 0.0f);
	gtk_grid_attach(GTK_GRID(checkGrid), checkSpecialLabel, 0, 6, 1, 1);


	gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char** argv) {
	#if GLIB_CHECK_VERSION(2, 74, 0)
		GtkApplication* app = gtk_application_new("org.tld.passwd", G_APPLICATION_DEFAULT_FLAGS);
	#else
		GtkApplication* app = gtk_application_new("org.tld.passwd", G_APPLICATION_FLAGS_NONE);
	#endif
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
