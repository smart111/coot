#include <gtk/gtk.h>
 
extern "C" G_MODULE_EXPORT void 
on_lbg_dialog_close (GtkDialog *dialog, gpointer user_data)
{
   // From a key binding
   printf("dialog close\n");
   gtk_main_quit();
}

extern "C" G_MODULE_EXPORT void
on_c5_toolbutton_clicked(GtkToolButton *button, gpointer user_data) {

   printf("c5 pressed\n");

}

extern "C" G_MODULE_EXPORT void
on_close_button_clicked(GtkButton *button, gpointer user_data) {
   printf("close clicked\n");
}

extern "C" G_MODULE_EXPORT void
on_carbon_toolbutton_clicked(GtkToolButton *button, gpointer user_data) {

   printf("C toolbutton clicked\n");

}

int
main (int argc, char *argv[])
{
        GtkBuilder              *builder;
        GtkWidget               *window;
        
        gtk_init (&argc, &argv);
        
        builder = gtk_builder_new ();
        gtk_builder_add_from_file (builder, "lbg.glade", NULL);

	if (! builder)
	  printf("Failed to generate builder\n");
 
        window = GTK_WIDGET (gtk_builder_get_object (builder, "lbg_dialog"));

	if (! window) {
	   g_object_unref (G_OBJECT (builder));
	   printf("Failed to generate window\n");
	} else {
	   gtk_builder_connect_signals (builder, NULL);          
	   g_object_unref (G_OBJECT (builder));
        
	   gtk_widget_show (window);       
	   gtk_main ();
	}
        
        return 0;
}

