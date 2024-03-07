#include <gtk/gtk.h>

#include <string>

#include "generate.hpp"
#include "evaluate.hpp"
#include "store.hpp"

char* storeName = (char*)".passwds-passkey-store-tmp";

bool preventNextLengthChange = false;
int startWpasswdEntries = 0;
gchar* revert;

GtkApplication* app;

GtkWidget* window;

GtkWidget* deathWindow;

GtkWidget* genWindow;

GtkWidget* selWindow;
GtkWidget* selListBox;

GtkWidget* startWindow;
GtkWidget* startPasswdEntry;
GtkWidget* startLabel;

GtkWidget* checkWindow;
GtkWidget* checkScoreLabel;
GtkWidget* checkLengthLabel;
GtkWidget* checkDigitLabel;
GtkWidget* checkLowerLabel;
GtkWidget* checkUpperLabel;
GtkWidget* checkSpecialLabel;

GtkWidget* nameEntry;
GtkWidget* passwdEntry;

void selUpdate() {
	for (int i = 0; i < 32767; i++) {
		GtkListBoxRow* item = gtk_list_box_get_row_at_index(GTK_LIST_BOX(selListBox), 0);
		if (item == nullptr) {
			break;
		}
		gtk_list_box_remove(GTK_LIST_BOX(selListBox), GTK_WIDGET(item));
	}

	std::string buildName;
	for (int i = 0; i < iosecure::contents.length(); i++) {
		if (iosecure::contents[i] == '\2') {
			buildName = "";
		}
		else if (iosecure::contents[i] == '\3') {
			GtkWidget* child = gtk_label_new_with_mnemonic(buildName.c_str());
			gtk_list_box_append(GTK_LIST_BOX(selListBox), child);
		}
		else {
			buildName += iosecure::contents[i];
		}
	}
}

void passUpdate(const char* _name) {
	bool match = false;
	bool _n = false;
	std::string buildName;
	std::string buildPass;
	for (int i = 0; i < iosecure::contents.length(); i++) {
		if (iosecure::contents[i] == '\2' || i == iosecure::contents.length()) {
			if (match) {
				break;
			}

			_n = true;
			buildName = "";
			match = false;
		}
		else if (iosecure::contents[i] == '\3') {
			_n = false;
			if (buildName == std::string(_name)) {
				match = true;
			}
			buildPass = "";
		}
		else {
			if (_n) {
				buildName += iosecure::contents[i];
			}
			else {
				buildPass += iosecure::contents[i];
			}
		}
	}

	gtk_editable_set_text(GTK_EDITABLE(nameEntry), g_strdup_printf("%s", buildName.c_str()));
	gtk_editable_set_text(GTK_EDITABLE(passwdEntry), g_strdup_printf("%s", buildPass.c_str()));
}

static gboolean revertGc(gpointer data) {
	gtk_label_set_text(GTK_LABEL(startLabel), revert);
	return true;
}

void startDisplayInfo(gchar* msg, gpointer data) {
	revert = g_strdup_printf("%s", gtk_label_get_text(GTK_LABEL(startLabel)));
	gtk_label_set_text(GTK_LABEL(startLabel), msg);
	g_timeout_add(1000, revertGc, data);
}

static void entry(GtkWidget* widget, gpointer data) {
	switch (iosecure::getFiletype(storeName)) {
		case -1:
			if (startWpasswdEntries > 0) {
				if (iosecure::master == std::string(gtk_editable_get_text(GTK_EDITABLE(startPasswdEntry)))) {
					iosecure::open(storeName);
					iosecure::close();
					gtk_window_present(GTK_WINDOW(window));
					gtk_widget_hide(GTK_WIDGET(startWindow));
				}
				else {
					startDisplayInfo(g_strdup_printf("Passwords do not match"), data);
					startWpasswdEntries = 0;
				}
			}
			else {
				iosecure::master = std::string(gtk_editable_get_text(GTK_EDITABLE(startPasswdEntry)));
				startWpasswdEntries++;
				startDisplayInfo(g_strdup_printf("Re-enter password"), data);
				gtk_editable_set_text(GTK_EDITABLE(startPasswdEntry), "");
			}
			break;
		case 0:
			iosecure::master = std::string(gtk_editable_get_text(GTK_EDITABLE(startPasswdEntry)));
			iosecure::filename = storeName;
			if (iosecure::checkPasswd() == 0) {
				iosecure::open(storeName);
				gtk_window_present(GTK_WINDOW(window));
				gtk_widget_hide(GTK_WIDGET(startWindow));
			}
			else {
				startDisplayInfo(g_strdup_printf("Incorrect password"), data);
			}
			break;
		case 1:
			g_print("Error: a directory named %s exists in the base directory", storeName);
			startDisplayInfo(g_strdup_printf("Error: %s exists", storeName), data);
			break;
	}
}

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
	std::string cmpName = gtk_editable_get_text(GTK_EDITABLE(nameEntry));
	std::string cmpPass = gtk_editable_get_text(GTK_EDITABLE(passwdEntry));

	int startCut = -1;
	bool doCut = false;
	std::string buildName;
	for (int i = 0; i < iosecure::contents.length(); i++) {
		if (iosecure::contents[i] == '\2' || i == iosecure::contents.length() - 1) {
			if (doCut) {
				iosecure::contents.erase(iosecure::contents.begin()+startCut, iosecure::contents.begin()+i);
			}

			buildName = "";
			doCut = false;
			startCut = i;
		}
		else if (iosecure::contents[i] == '\3') {
			if (buildName == cmpName) {
				doCut = true;
			}
		}
		else {
			buildName += iosecure::contents[i];
		}
	}

	if (cmpPass != "") {
		iosecure::contents += "\2" + cmpName + "\3" + cmpPass;
	}
}

static void load(GtkWidget* widget, gpointer data) {
	selUpdate();
	gtk_window_present(GTK_WINDOW(selWindow));
	gtk_list_box_unselect_all(GTK_LIST_BOX(selListBox));
}

static void activate(GtkApplication* app, gpointer user_data) {
	// start window
	startWindow = gtk_application_window_new(app);
	gtk_window_set_title (GTK_WINDOW(startWindow), "Passwds");
	gtk_window_set_default_size(GTK_WINDOW(startWindow), 400, 200);

	GtkWidget* startGrid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(startGrid), 16);
	gtk_grid_set_row_spacing(GTK_GRID(startGrid), 16);
	gtk_grid_set_column_homogeneous(GTK_GRID(startGrid), true);
	gtk_grid_set_row_homogeneous(GTK_GRID(startGrid), true);
	gtk_widget_set_margin_top(startGrid, 8);
	gtk_widget_set_margin_bottom(startGrid, 8);
	gtk_widget_set_margin_start(startGrid, 8);
	gtk_widget_set_margin_end(startGrid, 8);
	gtk_window_set_child(GTK_WINDOW(startWindow), startGrid);

	startPasswdEntry = gtk_password_entry_new();
	gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(startPasswdEntry), true);
	GtkWidget* startPasswdLabel = gtk_label_new_with_mnemonic("Password: ");
	gtk_label_set_mnemonic_widget(GTK_LABEL(startPasswdLabel), startPasswdEntry);
	gtk_label_set_xalign(GTK_LABEL(startPasswdLabel), 1.0f);
	gtk_grid_attach(GTK_GRID(startGrid), startPasswdLabel, 0, 1, 1, 2);
	gtk_grid_attach(GTK_GRID(startGrid), startPasswdEntry, 1, 1, 3, 2);

	startLabel = gtk_label_new_with_mnemonic("Enter master password");
	gtk_label_set_xalign(GTK_LABEL(startLabel), 0.0f);
	gtk_grid_attach(GTK_GRID(startGrid), startLabel, 0, 3, 4, 1);

	GtkWidget* startSubmitButton = gtk_button_new_with_label("Enter");
	g_signal_connect(startSubmitButton, "clicked", G_CALLBACK(entry), NULL);
	gtk_grid_attach(GTK_GRID(startGrid), startSubmitButton, 0, 4, 4, 2);

	g_signal_connect(startWindow, "close-request", G_CALLBACK(+[](GtkWindow* widget, gpointer data) {
		g_application_quit(G_APPLICATION(gtk_window_get_application(widget)));
		return true;
	}), NULL);


	// main window
	window = gtk_application_window_new(app);
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

	g_signal_connect(window, "close-request", G_CALLBACK(+[](GtkWindow* widget, gpointer data) {
		gtk_window_present(GTK_WINDOW(deathWindow));
		return true;
	}), NULL);

	// death prompt window
	deathWindow = gtk_window_new();
	gtk_window_set_title (GTK_WINDOW(deathWindow), "Passwds");
	gtk_window_set_default_size(GTK_WINDOW(deathWindow), 200, 100);
	gtk_window_set_hide_on_close(GTK_WINDOW(deathWindow), true);

	GtkWidget* deathGrid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(deathGrid), 16);
	gtk_grid_set_row_spacing(GTK_GRID(deathGrid), 16);
	gtk_grid_set_column_homogeneous(GTK_GRID(deathGrid), true);
	gtk_grid_set_row_homogeneous(GTK_GRID(deathGrid), true);
	gtk_widget_set_margin_top(deathGrid, 8);
	gtk_widget_set_margin_bottom(deathGrid, 8);
	gtk_widget_set_margin_start(deathGrid, 8);
	gtk_widget_set_margin_end(deathGrid, 8);
	gtk_window_set_child(GTK_WINDOW(deathWindow), deathGrid);

	GtkWidget* deathLabel = gtk_label_new_with_mnemonic("Do you want to save changes?");
	gtk_grid_attach(GTK_GRID(deathGrid), deathLabel, 0, 0, 3, 1);

	GtkWidget* deathSaveButton = gtk_button_new_with_label("Save");
	g_signal_connect(deathSaveButton, "clicked", G_CALLBACK(+[](GtkWidget* widget, gpointer data) {
		iosecure::close();
		gpointer window = g_list_model_get_item(gtk_window_get_toplevels(), 0);
		g_application_quit(G_APPLICATION(gtk_window_get_application(GTK_WINDOW(window))));
	}), NULL);
	gtk_grid_attach(GTK_GRID(deathGrid), deathSaveButton, 0, 1, 1, 1);

	GtkWidget* deathDontSaveButton = gtk_button_new_with_label("Don't save");
	g_signal_connect(deathDontSaveButton, "clicked", G_CALLBACK(+[](GtkWidget* widget, gpointer data) {
		gpointer window = g_list_model_get_item(gtk_window_get_toplevels(), 0);
		g_application_quit(G_APPLICATION(gtk_window_get_application(GTK_WINDOW(window))));
	}), NULL);
	gtk_grid_attach(GTK_GRID(deathGrid), deathDontSaveButton, 1, 1, 1, 1);

	GtkWidget* deathCloseButton = gtk_button_new_with_label("Cancel");
	g_signal_connect(deathCloseButton, "clicked", G_CALLBACK(+[](GtkWidget* widget, gpointer data) {
		gtk_widget_hide(gtk_widget_get_parent(gtk_widget_get_parent(widget)));
	}), NULL);
	gtk_grid_attach(GTK_GRID(deathGrid), deathCloseButton, 2, 1, 1, 1);


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
	gtk_window_set_title (GTK_WINDOW(checkWindow), "Security");
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


	// load window
	selWindow = gtk_application_window_new(app);
	gtk_window_set_title (GTK_WINDOW(selWindow), "Select");
	gtk_window_set_default_size(GTK_WINDOW(selWindow), 300, 400);
	gtk_window_set_hide_on_close(GTK_WINDOW(selWindow), true);

	selListBox = gtk_list_box_new();
	gtk_window_set_child(GTK_WINDOW(selWindow), selListBox);
	gtk_list_box_set_selection_mode(GTK_LIST_BOX(selListBox), GTK_SELECTION_NONE);
	g_signal_connect(selListBox, "row-activated", G_CALLBACK(+[](GtkWidget* widget, GtkListBoxRow* row, gpointer data) {
		passUpdate(gtk_label_get_text(GTK_LABEL(gtk_list_box_row_get_child(row))));
	}), NULL);


	gtk_window_present(GTK_WINDOW(startWindow));
}

int main(int argc, char** argv) {
	#if GLIB_CHECK_VERSION(2, 74, 0)
		app = gtk_application_new("org.tld.passwd", G_APPLICATION_DEFAULT_FLAGS);
	#else
		app = gtk_application_new("org.tld.passwd", G_APPLICATION_FLAGS_NONE);
	#endif
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
