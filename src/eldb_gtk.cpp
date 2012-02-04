//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

extern "C" {
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
}

#include "eldb.hpp"
#include "eldb_gtk_globalkeybind.hpp"

#define TRAY_ICON INSTALL_PREFIX "/share/exolobe/exolobe_tray.png"
#define APP_ICON INSTALL_PREFIX "/share/exolobe/exolobe_tray.png"

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
  ~Window();
  void toggleVisibility();
  void layout();

  static void destroy();
  static void status_icon_click( GtkWidget *widget, Window *window );
  static void searchEntryChanged( GtkWidget *widget, Window *window );
  static int keySnooper( GtkWidget *widget, GdkEventKey *event, gpointer data );
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
  GtkWidget *vBox;
  GtkWidget *textView;
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

  GError *error;
  if ( ! gtk_window_set_icon_from_file( GTK_WINDOW( window ), APP_ICON, &error ) ) {
    fprintf( stderr, "%s\n", error->message );
    g_error_free( error );
  }

  g_signal_connect( searchEntry, "changed", G_CALLBACK( searchEntryChanged ), this );

  g_signal_connect( window, "destroy", G_CALLBACK( destroy ), NULL );
  gtk_window_set_title( GTK_WINDOW( window ), "Exo-Lobe" );
  gtk_window_set_default_size( GTK_WINDOW( window ), 800, 300 );

  searchEntryChanged( searchEntry, this );

  gtk_key_snooper_install( keySnooper, window );
  gtk_window_set_focus( GTK_WINDOW( window ), 0 );

  globalBinding( GTK_WINDOW( window ) );

  GtkStatusIcon *status_icon = gtk_status_icon_new();
  g_signal_connect( G_OBJECT(status_icon), "activate", G_CALLBACK( status_icon_click ), this );
  gtk_status_icon_set_from_file( status_icon, TRAY_ICON );
  gtk_status_icon_set_tooltip( status_icon, "Exo-Lobe (Alt-F9)" );
  gtk_status_icon_set_visible( status_icon, 1 );

  toggleVisibility();
}


Window::~Window() {
  delete valueList;
  delete keyList;
}


void Window::status_icon_click( GtkWidget *widget, Window *window ) {
  window->toggleVisibility();
}


void Window::toggleVisibility() {
  if ( gtk_widget_get_visible( window ) ) {
    gtk_widget_hide( window );
  } else {
    gtk_widget_show_all( window );
    gtk_window_present( GTK_WINDOW( window ) );
    gtk_window_stick( GTK_WINDOW( window ) );
    gtk_window_set_keep_above( GTK_WINDOW( window ), 1 );
  }
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
  const char *key;

  if ( gtk_tree_selection_get_selected( selection, &model, &iter ) ) {
    gtk_tree_model_get( model, &iter, VALUE_COLUMN, &key, -1 );
    GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( valueList->textView ) );
    gtk_text_buffer_set_text( buffer, key, -1 );

  }
}


ValueList::ValueList() {
  vBox = gtk_vbox_new( FALSE, 3 );
  textView = gtk_text_view_new();
  gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( textView ), GTK_WRAP_WORD_CHAR );

  gtk_container_add( GTK_CONTAINER( vBox ), textView );
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

      for ( auto value: key->values ) {

        gtk_list_store_append( keyStore, &iter );
        gtk_list_store_set( keyStore, &iter,
          KeyList::KEY_COLUMN  , key->key.c_str(),
          KeyList::VALUE_COLUMN, value->value.c_str(),
          -1 );
      }
    }
    GtkTreeSelection *selection =
      gtk_tree_view_get_selection( GTK_TREE_VIEW( window->keyList->treeView ) );
    gtk_tree_model_get_iter_first( GTK_TREE_MODEL( keyStore ), &iter );
    gtk_tree_selection_select_iter( selection, &iter );
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

