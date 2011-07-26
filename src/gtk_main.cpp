extern "C" {
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "mcdb.h"
}

struct NameList;


struct Window {
  mcdb_Connection *conn;

  GtkWidget *window;
  GtkWidget *textView;
  GtkWidget *searchEntry;
  NameList  *nameList;

  Window();
  ~Window();
  static void destroy();
  static void searchEntryChanged( GtkWidget *widget, Window *window );
  static int keySnooper( GtkWidget *widget, GdkEventKey *event, gpointer data );
  void layout();
};


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


Window::Window() {
  mcdb_init( &conn );

  window       = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  searchEntry  = gtk_entry_new();
  textView     = gtk_text_view_new ();
  nameList     = new NameList( textView );

  gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( textView), GTK_WRAP_WORD_CHAR );


  layout();

  g_signal_connect( searchEntry, "changed", G_CALLBACK( searchEntryChanged ), this );

  g_signal_connect( window, "destroy", G_CALLBACK( destroy ), NULL );
  gtk_window_set_title( GTK_WINDOW( window ), "Meta Cortex" );

  gtk_widget_show_all( window );

  searchEntryChanged( searchEntry, this );

  gtk_key_snooper_install( keySnooper, window );
  gtk_window_set_focus( GTK_WINDOW( window ), 0 );
}


Window::~Window() {
  mcdb_shutdown( conn );
}


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


void Window::destroy( ) {
  gtk_main_quit ();
}


void Window::searchEntryChanged( GtkWidget *widget, Window *window ) {
  const char *text;
  GtkTreeIter iter;
  GtkListStore *keyStore =  window->nameList->listStore;


  gtk_list_store_clear( keyStore );
  text = gtk_entry_get_text( GTK_ENTRY( widget ) );

  mcdb_Result *result;

  mcdb_find( window->conn, text, &result );

  if( result ) {
    while( !mcdb_next( window->conn, result ) ) {
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


void Window::layout() {
  GtkWidget *table            = gtk_table_new( 3, 3, FALSE );
  GtkWidget *textViewScroller = gtk_scrolled_window_new( NULL, NULL );
  GtkWidget *nameListScroller = gtk_scrolled_window_new( NULL, NULL );

  gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );

  gtk_table_attach_defaults( GTK_TABLE( table ), searchEntry      , 0 , 3, 0, 1 );
  gtk_table_attach_defaults( GTK_TABLE( table ), nameListScroller , 0 , 1, 1, 3 );
  gtk_table_attach_defaults( GTK_TABLE( table ), textViewScroller , 1 , 3, 1, 3 );

  gtk_container_add( GTK_CONTAINER( textViewScroller ), textView );
  gtk_container_add( GTK_CONTAINER( nameListScroller ), nameList->treeView );

  gtk_table_set_col_spacings( GTK_TABLE( table ), 5 );

  gtk_container_add( GTK_CONTAINER( window ), table );
}



int Window::keySnooper( GtkWidget *widget, GdkEventKey *event, gpointer data ) {
  if( event->keyval == GDK_KEY_q && event->state & GDK_CONTROL_MASK ) {
    fprintf( stderr, "Quit Key" );
    destroy( );
  }

  if( event->keyval == GDK_KEY_Escape ) {
    gtk_window_set_focus( GTK_WINDOW( data ), 0 );
  }

  return 0;
}


int main( int argc, char **argv ) {

  gtk_init( &argc, &argv );

  Window window;

  gtk_main();

  return 0;
}
