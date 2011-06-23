extern "C" {
#include <gtk/gtk.h>
#include "mcdb.h"
}

struct NameList {
  GtkWidget         *treeView;
  GtkListStore      *listStore;
  GtkTreeViewColumn *column;
  GtkCellRenderer   *nameRenderer;
  GtkTreeSelection  *select;

  enum Columns {
    KEY_COLUMN = 0,
    VALUE_COLUMN,
  };

  NameList( GtkWidget *textView );
  static void changed( GtkTreeSelection *widget, GtkTextView *textView );
};


NameList::NameList( GtkWidget *textView ) {
  listStore    = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );
  treeView     = gtk_tree_view_new_with_model( GTK_TREE_MODEL( listStore ) );
  nameRenderer = gtk_cell_renderer_text_new();

  column       = gtk_tree_view_column_new_with_attributes( "Keys", nameRenderer, "text", KEY_COLUMN,  NULL );
  gtk_tree_view_append_column( GTK_TREE_VIEW( treeView ), column );

  g_object_unref( G_OBJECT( listStore ) );


  select = gtk_tree_view_get_selection( GTK_TREE_VIEW ( treeView ) );
  gtk_tree_selection_set_mode( select, GTK_SELECTION_SINGLE );

  g_signal_connect( select, "changed", G_CALLBACK( NameList::changed ), textView );
}


void NameList::changed( GtkTreeSelection *selection, GtkTextView *textView ) {
  GtkTreeIter iter;
  GtkTreeModel *model;
  const char *key;

  if( gtk_tree_selection_get_selected( selection, &model, &iter) ) {
    gtk_tree_model_get( model, &iter, VALUE_COLUMN, &key, -1 );
    GtkTextBuffer *buffer = gtk_text_view_get_buffer( textView );
    gtk_text_buffer_set_text( buffer, key, -1 );
  }
}


mcdb_Connection *conn;


static void destroy( GtkWidget *widget ) {
  gtk_main_quit ();
}


static void searchEntryChanged( GtkWidget *widget, GtkListStore *keyStore ) {
  const char *text;
  GtkTreeIter iter;

  gtk_list_store_clear( keyStore );
  text = gtk_entry_get_text( GTK_ENTRY( widget ) );

  mcdb_Result *result;

  mcdb_find( conn, text, &result );

  if( result ) {
    while( !mcdb_next( conn, result ) ) {
      fprintf( stderr, "%s : %s\n", result->a, result->b );
      gtk_list_store_append( keyStore, &iter );
      gtk_list_store_set( keyStore, &iter,
        NameList::KEY_COLUMN   , result->a,
        NameList::VALUE_COLUMN, result->b,
        -1 );
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

  gtk_init( &argc, &argv );

  mcdb_init( &conn );

  window       = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  searchEntry  = gtk_entry_new();
  textView     = textViewNew();

  NameList *nameList = new NameList( textView );

  layoutWindow( window, searchEntry, nameList->treeView, textView );

  g_signal_connect( searchEntry, "changed", G_CALLBACK( searchEntryChanged ), nameList->listStore );

  g_signal_connect( window, "destroy", G_CALLBACK( destroy ), NULL );
  gtk_window_set_title( GTK_WINDOW( window ), "Meta Cortex" );

  gtk_widget_show_all( window );

  searchEntryChanged( searchEntry, nameList->listStore );
  gtk_main();

  mcdb_shutdown( conn );

  return 0;
}
