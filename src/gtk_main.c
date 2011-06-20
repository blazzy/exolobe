#include <gtk/gtk.h>

#include "mcdb.h"


mcdb_Connection *conn;


static void destroy( GtkWidget *widget ) {
  gtk_main_quit ();
}


static void searchEntryChanged( GtkWidget *widget, GtkListStore *listStore ) {
  const char *text;
  GtkTreeIter iter;

  gtk_list_store_clear( listStore );
  text = gtk_entry_get_text( GTK_ENTRY( widget ) );

  mcdb_Result *result;

  mcdb_find( conn, text, &result );

  if( result ) {
    while( !mcdb_next( conn, result ) ) {
      fprintf( stderr, "%s : %s\n", result->a, result->b );
      gtk_list_store_append( listStore, &iter );
      gtk_list_store_set( listStore, &iter, 0, result->a, -1 );
    }
  } else {
    fprintf( stderr, "No Results\n" );
  }

  mcdb_freeResult(result);
}


static void layoutWindow( GtkWidget *window, GtkWidget *searchEntry, GtkWidget *nameList, GtkWidget *textView ) {
  GtkWidget *table            = gtk_table_new( 3, 3, FALSE );
  GtkWidget *textViewScroller = gtk_scrolled_window_new( NULL, NULL );
  GtkWidget *nameListScroller = gtk_scrolled_window_new( NULL, NULL );

  gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );

  gtk_table_attach_defaults( GTK_TABLE( table ), searchEntry      , 0 , 3, 0, 1 );
  gtk_table_attach_defaults( GTK_TABLE( table ), nameListScroller , 0 , 1, 1, 3 );
  gtk_table_attach_defaults( GTK_TABLE( table ), textViewScroller , 1 , 3, 1, 3 );

  gtk_container_add( GTK_CONTAINER( textViewScroller ), textView );
  gtk_container_add( GTK_CONTAINER( nameListScroller ), nameList );

  gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );

  gtk_container_add( GTK_CONTAINER( window ), table );
}


static GtkWidget *textViewNew( ) {
  GtkWidget     *textView   = gtk_text_view_new ();
  gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( textView), GTK_WRAP_WORD_CHAR );

  return textView;
}


int main( int argc, char **argv ) {
  GtkWidget         *window;
  GtkWidget         *textView;
  GtkWidget         *searchEntry;
  GtkWidget         *nameList;
  GtkListStore      *nameStore;
  GtkCellRenderer   *nameRenderer;
  GtkTreeViewColumn *column;

  gtk_init( &argc, &argv );

  mcdb_init( &conn );

  window       = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  searchEntry  = gtk_entry_new();
  textView     = textViewNew();
  nameStore    = gtk_list_store_new( 1, G_TYPE_STRING );
  nameList     = gtk_tree_view_new_with_model( GTK_TREE_MODEL( nameStore ) );
  nameRenderer = gtk_cell_renderer_text_new();
  column       = gtk_tree_view_column_new_with_attributes( "Keys", nameRenderer, "text", 0,  NULL );

  gtk_tree_view_append_column( GTK_TREE_VIEW( nameList ), column );

  g_object_unref( G_OBJECT( nameStore ) );

  layoutWindow( window, searchEntry, nameList, textView );

  g_signal_connect( searchEntry, "changed", G_CALLBACK( searchEntryChanged ), nameStore );

  g_signal_connect( window, "destroy", G_CALLBACK( destroy ), NULL );
  gtk_window_set_title( GTK_WINDOW( window ), "Meta Cortex" );

  gtk_widget_show_all( window );

  gtk_main();

  mcdb_shutdown( conn );

  return 0;
}
