//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

extern "C" {
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
}

#include "eldb.hpp"
#include "eldb_gtk_globalkeybind.hpp"

namespace elgtk {

struct KeyList;
struct ValueList;


struct Window {
  eldb::Eldb eldb;

  GtkWidget *window;
  GtkWidget *searchEntry;
  KeyList   *keyList;
  ValueList *valueList;

  Window();

  static void destroy();
  static void searchEntryChanged( GtkWidget *widget, Window *window );
  static int keySnooper( GtkWidget *widget, GdkEventKey *event, gpointer data );
  void layout();
};


struct KeyList {
  GtkWidget         *treeView;
  GtkListStore      *listStore;
  GtkTreeViewColumn *column;
  GtkCellRenderer   *nameRenderer;
  GtkTreeSelection  *select;

  enum Columns {
    KEY_COLUMN = 0,
    VALUE_COLUMN
  };

  KeyList( ValueList *valueList );
  static void changed( GtkTreeSelection *widget, ValueList *valueList );
};


struct ValueList {
  GtkWidget* vBox;
  ValueList();
};


Window::Window() {

  if ( !eldb.init( "" ) ) {
    exit(1);
  }

  window      = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  searchEntry = gtk_entry_new();
  valueList   = new ValueList();
  keyList     = new KeyList( valueList );
  layout();

  g_signal_connect( searchEntry, "changed", G_CALLBACK( searchEntryChanged ), this );

  g_signal_connect( window, "destroy", G_CALLBACK( destroy ), NULL );
  gtk_window_set_title( GTK_WINDOW( window ), "The Exo Lobe" );

  gtk_widget_show_all( window );
  gtk_window_stick( GTK_WINDOW( window ) );
  gtk_window_set_keep_above( GTK_WINDOW( window ), 1 );

  searchEntryChanged( searchEntry, this );

  gtk_key_snooper_install( keySnooper, window );
  gtk_window_set_focus( GTK_WINDOW( window ), 0 );

  globalBinding( GTK_WINDOW( window ) );
}


KeyList::KeyList( ValueList *valueList ) {
  listStore    = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );
  treeView     = gtk_tree_view_new_with_model( GTK_TREE_MODEL( listStore ) );
  nameRenderer = gtk_cell_renderer_text_new();

  column       = gtk_tree_view_column_new_with_attributes( "Keys", nameRenderer, "text", KEY_COLUMN,  (void *)NULL );
  gtk_tree_view_append_column( GTK_TREE_VIEW( treeView ), column );

  g_object_unref( G_OBJECT( listStore ) );


  select = gtk_tree_view_get_selection( GTK_TREE_VIEW ( treeView ) );
  gtk_tree_selection_set_mode( select, GTK_SELECTION_SINGLE );

  g_signal_connect( select, "changed", G_CALLBACK( KeyList::changed ), valueList );
}


void KeyList::changed( GtkTreeSelection *selection, ValueList *valueList ) {
  GtkTreeIter iter;
  GtkTreeModel *model;
  //const char *key;

  if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
/*    gtk_tree_model_get( model, &iter, VALUE_COLUMN, &key, -1 );
    GtkTextBuffer *buffer = gtk_text_view_get_buffer( textView );
    gtk_text_buffer_set_text( buffer, key, -1 );*/
  }
}


ValueList::ValueList() {
  vBox = gtk_vbox_new( FALSE, 3 );
  //TODO gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( textView ), GTK_WRAP_WORD_CHAR );
}


void Window::destroy( ) {
  gtk_main_quit ();
}


void Window::searchEntryChanged( GtkWidget *widget, Window *window ) {
  const char *text;
  GtkTreeIter iter;
  GtkListStore *keyStore =  window->keyList->listStore;


  gtk_list_store_clear( keyStore );
  text = gtk_entry_get_text( GTK_ENTRY( widget ) );

  auto result = window->eldb.find( text );

  if ( result->count ) {
    for ( auto key: result->keys ) {
      fprintf( stdout, "%s :\n", key->key.c_str() );
      
      for ( auto value: key->values ) {
        fprintf( stdout, "\t%s\n", value->value.c_str() );

        gtk_list_store_append( keyStore, &iter );
        gtk_list_store_set( keyStore, &iter,
          KeyList::KEY_COLUMN  , key->key.c_str(),
          KeyList::VALUE_COLUMN, value->value.c_str(),
          -1 );
      }
    }
  } else {
    fprintf( stderr, "No Results\n" );
  }
}


void Window::layout() {
  GtkWidget *table            = gtk_table_new( 3, 3, FALSE );
  GtkWidget *textViewScroller = gtk_scrolled_window_new( NULL, NULL );
  GtkWidget *keyListScroller  = gtk_scrolled_window_new( NULL, NULL );

  gtk_container_set_border_width( GTK_CONTAINER( window ), 5 );

  gtk_table_attach( GTK_TABLE( table ), searchEntry, 0, 3, 0, 1, GtkAttachOptions( GTK_EXPAND | GTK_FILL ), GTK_SHRINK, 0, 0 );
  gtk_table_attach_defaults( GTK_TABLE( table ), keyListScroller, 0, 1, 1, 3 );
  gtk_table_attach_defaults( GTK_TABLE( table ), textViewScroller, 1, 3, 1, 3 );

  gtk_container_add( GTK_CONTAINER( keyListScroller ), keyList->treeView );
  gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW( textViewScroller ), valueList->vBox );

  gtk_table_set_row_spacings( GTK_TABLE( table ), 2 );
  gtk_table_set_col_spacings( GTK_TABLE( table ), 2 );

  gtk_container_add( GTK_CONTAINER( window ), table );
}



int Window::keySnooper( GtkWidget *widget, GdkEventKey *event, gpointer data ) {
  if ( event->keyval == GDK_KEY_q && event->state & GDK_CONTROL_MASK ) {
    destroy( );
  }

  if ( event->keyval == GDK_KEY_Escape ) {
    gtk_window_set_focus( GTK_WINDOW( data ), 0 );
  }

  return 0;
}

} //namespace elgtk

int main( int argc, char **argv ) {

  gtk_init( &argc, &argv );

  elgtk::Window window;

  gtk_main();

  return 0;
}

