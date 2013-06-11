#include <gtk/gtk.h>
#include <stdio.h>
#include <gnome.h>
#include <libgnomeui/libgnomeui.h>
#include <libelektro/libresistance.h>
#include <libelektro/libpower.h>
#include <libelektro/libcomponent.h>

// Struct för textfälten
struct textEntries{
GtkWidget *entry1;
GtkWidget *entry2;
GtkWidget *entry3;
GtkWidget *voltage;
};

// Variabler för labels och combobox
GtkWidget *totalresistancelabel;
GtkWidget *e12label;
GtkWidget *effectlabel;
GtkWidget *combobox;

// Variabel för dialogfönster
GtkWidget *dialog;

// Variabler för övrigt.
static int count = 0;
int i;
int amount = 3;
float totalresistance;
float e12[3];
int e12count;
struct textEntries entries;
float componentsresistance[3];
char conn;

// Funktion som anropas när en användare vill avsluta programmet.
void closeApp( GtkWidget *window, gpointer data )
{
	gtk_main_quit(); // Avslutar programmet.
} 

// Information för uppbyggnad av meny.
static GnomeUIInfo filemenu[] = {
	GNOMEUIINFO_MENU_EXIT_ITEM (closeApp, NULL),
	GNOMEUIINFO_END
};

// Information för uppbyggnad av meny.
static GnomeUIInfo menubar[] = {
	GNOMEUIINFO_MENU_FILE_TREE (filemenu),
	GNOMEUIINFO_END
};

// Anropas om antalet komponenter ändras.
void value_changed(GtkSpinButton *spinbutton, gpointer data)
{
	amount = gtk_spin_button_get_value_as_int(spinbutton); // Sätter variabeln till antal utvalda komponenter

	// Om utvalda komponenter är 1 så gömmer den textfält 2 och 3 och nollställer dem. Om utvalda komponenter är 2 så gömmer den textfält 3 och nollställer den. Annars visar den alla fält.
	if(gtk_spin_button_get_value_as_int(spinbutton) == 1) {
		componentsresistance[1] = 0; 
		componentsresistance[2] = 0;
		gtk_entry_set_text((GtkEntry *) entries.entry2, "");
		gtk_entry_set_text((GtkEntry *) entries.entry3, "");
		gtk_widget_hide(entries.entry2);
		gtk_widget_hide(entries.entry3);
	}
	else if (gtk_spin_button_get_value_as_int(spinbutton) == 2) {
		componentsresistance[2] = 0; 
		gtk_entry_set_text((GtkEntry *) entries.entry3, "");
		gtk_widget_hide(entries.entry3);
		gtk_widget_show(entries.entry2);
	}
	else {
		gtk_widget_show(entries.entry1);
		gtk_widget_show(entries.entry2);
		gtk_widget_show(entries.entry3);
	}
}

// Om Beräkna knappen trycks.
void button_clicked(GtkWidget *button, gpointer data)
{
	/* Om textfält 1-3 innehåller något så läggs dem in i componentsresistance variabeln.
		g_ascii_strtod funktionen omvandlar en string/character till integer.

		Om något textfält inte är ifyllt så nollställs componentsresistance indexen för det textfältet. */
	if(strlen(gtk_entry_get_text((GtkEntry *)entries.entry1)) != 0)
	{
		componentsresistance[0] = g_ascii_strtod(gtk_entry_get_text((GtkEntry *)entries.entry1), NULL);
	}
	else
	{
		componentsresistance[0] = 0;
	}

	if(strlen(gtk_entry_get_text((GtkEntry *)entries.entry2)) != 0)
	{
		componentsresistance[1] = g_ascii_strtod(gtk_entry_get_text((GtkEntry *)entries.entry2), NULL);
	}
	else
	{
		componentsresistance[1] = 0;
	}

	if(strlen(gtk_entry_get_text((GtkEntry *)entries.entry3)) != 0)
	{
		componentsresistance[2] = g_ascii_strtod(gtk_entry_get_text((GtkEntry *)entries.entry3), NULL);
	}
	else
	{
		componentsresistance[2] = 0;
	}
	
	char conn;

	// Om combobox valet är 0 (Seriell) så sätts variabeln conn till s. Annars sätts den till p.
	if(gtk_combo_box_get_active((GtkComboBox *)combobox) == 0) {
		conn = 's';
	}
	else {
		conn = 'p';
	}
	totalresistance = calc_resistans(amount, conn, componentsresistance); // Använd calc_resistans för att räkna ut den totala resistansen och sätt det i totalresistance variabeln.
	gchar *str; // Skapa gchar för att kunna ändra labels.
	
	// Om totalresistans inte är tom så visar den resultatet.
	if(totalresistance > 0) {
		// g_strdup_printf används för att kunna använda printf för en label.
		str = g_strdup_printf("Ersättningsresistans: %g ohm", totalresistance);
		gtk_label_set_text( (GtkLabel *) totalresistancelabel, str); // Används för att sätta texten på en label.

		str = g_strdup_printf("Effekt: %5.2f W", calc_power_r(g_ascii_strtod(gtk_entry_get_text((GtkEntry *)entries.voltage), NULL), totalresistance)); // calc_power_r för att räkna ut effekten.
		gtk_label_set_text( (GtkLabel *) effectlabel, str);

		e12count = e_resistance(totalresistance, e12); // Använd e_resistance och lägg i e12 arrayn och lägger sedan antal komponenter i e12count.	
	}
	// Om totalresistans är tom. Dvs något av fälten är inkorrekta så visas ett felmeddelande.
	else {
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_hide(dialog);
	}

	// Om e12count är 1, visa 1 resistans etc.
	if(e12count == 1) {
		str = g_strdup_printf("Ersättningsresistanser i E12-serien kopplade i serie: %g", e12[0]);
		gtk_label_set_text( (GtkLabel *) e12label, str);
	}
	else if(e12count == 2) {
		str = g_strdup_printf("Ersättningsresistanser i E12-serien kopplade i serie: %g, %g", e12[0], e12[1]);
		gtk_label_set_text( (GtkLabel *) e12label, str);
	}
	else if(e12count == 3) {
		str = g_strdup_printf("Ersättningsresistanser i E12-serien kopplade i serie: %g, %g, %g", e12[0], e12[1], e12[2]);
		gtk_label_set_text( (GtkLabel *) e12label, str);		
	}
	

}

int main (int argc, char *argv[]) 
{
	// Variabler för widgets.
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *button, *quitbutton;
	GtkWidget *spinbutton;
	GtkObject *adjustment;
	GtkWidget *voltagelabel, *connlabel, *componentlabel;

	gnome_program_init ("gnome1", "0.1", LIBGNOMEUI_MODULE, argc, argv, GNOME_PARAM_NONE); // Förbered gnome app.
	window = gnome_app_new("gnome1", "Elektrotestgtk"); // Skapa fönster objektet med titeln Electro.

	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); // Sätt position för fönstret i mitten av skärmen.
	gtk_window_set_default_size(GTK_WINDOW(window), 380, 200); // Sätt fönsterstorlek 380x200

	g_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(closeApp), NULL); // Skapa signal för krysset i hörnet att anropa closeApp funktionen när den anropas.
	
	// Skapa widgets och ge dem en GtkWidget variabel
	button = gtk_button_new_with_label("Beräkna"); // Beräkna knapp
	quitbutton = gtk_button_new_with_label("Avsluta"); // Avsluta knapp
	adjustment = gtk_adjustment_new(3, 1, 3, 1, 1, 0); // Adjustment behövs för spinner
	spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 0.01, 0); // Spinner som används för val av komponenter.
	totalresistancelabel = gtk_label_new(0); // Tom label som ändras under programmets gång.
	effectlabel = gtk_label_new(0); // Tom label som ändras under programmets gång.
	e12label = gtk_label_new(0); // Tom label som ändras under programmets gång.
	voltagelabel = gtk_label_new("Ange spänningskälla i V:"); // Label för textfält där man skriver i antal volt.
	connlabel = gtk_label_new("Ange koppling:"); // Label för combobox för att välja kopplingstyp.
	componentlabel = gtk_label_new("Antal komponenter:"); // Label för spinner där man väljer antal komponenter
	combobox = gtk_combo_box_new_text(); // Combobox för kopplingstyp
	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Var god skriv in alla fält"); // Dialogfönster om något går fel.
	

	vbox = gtk_vbox_new(FALSE, 0); // Skapa en vertikal låda som håller alla widgets.

	gtk_box_pack_start(GTK_BOX(vbox), voltagelabel, FALSE, FALSE, 5); // Packa in voltagelabel i vbox.
	entries.voltage = gtk_entry_new(); // Skapa textfält för volt.
	gtk_box_pack_start(GTK_BOX(vbox), entries.voltage, TRUE, FALSE, 0);	 // Packa in i täxtfältet i vbox.

	gtk_box_pack_start(GTK_BOX(vbox), connlabel, FALSE, FALSE, 5); // Packa in connlabel i vbox.
	gtk_box_pack_start(GTK_BOX(vbox), combobox, FALSE, FALSE, 5); // Packa in combobox i vbox.
	
	gtk_combo_box_insert_text( GTK_COMBO_BOX( combobox ), 0, "Seriell" ); // Fyll combobox med val. Val 0 blir Seriell.
	gtk_combo_box_insert_text( GTK_COMBO_BOX( combobox ), 1, "Parallel" ); // Val 1 blir Parallel.
	gtk_combo_box_set_active( GTK_COMBO_BOX( combobox ), 0); // Ställ default till 0 (Seriell).

	gtk_box_pack_start(GTK_BOX(vbox), componentlabel, FALSE, FALSE, 5); // Packa in componentlabel i vbox.
	gtk_box_pack_start(GTK_BOX(vbox), spinbutton, FALSE, FALSE, 5); // Packa in spinner i vbox.

	// Skapa 3 täxtfält och packa in i vbox.
	entries.entry1 = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox), entries.entry1, TRUE, FALSE, 0);	

	entries.entry2 = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox), entries.entry2, TRUE, FALSE, 0);

	entries.entry3 = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox), entries.entry3, TRUE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 5); // Packa in beräkningsknappen i vbox.

	// Packa in labels i vbox.
	gtk_box_pack_start(GTK_BOX(vbox), totalresistancelabel, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), effectlabel, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), e12label, FALSE, FALSE, 5);

	gtk_box_pack_start(GTK_BOX(vbox), quitbutton, FALSE, FALSE, 5); // Packa in quit knappen i vbox.
	
	g_signal_connect(GTK_OBJECT (spinbutton), "changed", GTK_SIGNAL_FUNC (value_changed), "Spin"); // Signal som anropar value_changed funktionen om spinner ändras.
	g_signal_connect(GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (button_clicked), "Button 1"); // Signal som anropar button_clicked funktionen om button trycks.
	g_signal_connect(GTK_OBJECT (quitbutton), "clicked", GTK_SIGNAL_FUNC (closeApp), "Button 2"); // Signal som anropar closeApp om quitbutton trycks.


	gnome_app_set_contents (GNOME_APP (window), vbox); // Berätta för gnome fönstret att vbox har innehållet.
	gnome_app_create_menus ( GNOME_APP(window), menubar); // Skapa meny med information från menubar.

	gtk_widget_show_all(window); // Visa alla widgets i fönstret.
	gtk_main (); // Main loop för GTK.

	return 0;
}
