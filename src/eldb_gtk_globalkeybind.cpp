//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

//TODO: This only works with X11. Support more platforms. 
//      Binding is hard coded. Make configurable.
//      Error checking

extern "C" {
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
}

#include "eldb_gtk_globalkeybind.hpp"


void raiseWindow( GtkWindow *gtkWindow ) {
  GdkScreen *screen = gtk_widget_get_screen( GTK_WIDGET( gtkWindow ) );
  Display   *dpy    = GDK_WINDOW_XDISPLAY( gdk_get_default_root_window() );
  Window    window  = GDK_WINDOW_XWINDOW( GTK_WIDGET( gtkWindow )->window );
  Window    root    = GDK_WINDOW_XWINDOW( gdk_get_default_root_window() );
  XEvent    msgEvent;

  if ( gdk_x11_screen_supports_net_wm_hint( screen, gdk_atom_intern_static_string( "_NET_ACTIVE_WINDOW" ) ) ) {
    msgEvent.xclient.type         = ClientMessage;
    msgEvent.xclient.serial       = 0;
    msgEvent.xclient.send_event   = True;
    msgEvent.xclient.display      = dpy;
    msgEvent.xclient.window       = window;
    msgEvent.xclient.message_type = gdk_x11_get_xatom_by_name( "_NET_ACTIVE_WINDOW" );
    msgEvent.xclient.format       = 32;
    msgEvent.xclient.data.l[0]    = 2;
    msgEvent.xclient.data.l[1]    = 0;//Do I need a timestamp?
    msgEvent.xclient.data.l[2]    = 0;
    msgEvent.xclient.data.l[3]    = 0;
    msgEvent.xclient.data.l[4]    = 0;

    XSendEvent( dpy, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &msgEvent );
  } else {
    XRaiseWindow( dpy, GDK_WINDOW_XID( gdk_get_default_root_window() ) );
  }
}


static GdkFilterReturn keyFilter( GdkXEvent *gdk_xevent, GdkEvent *event, gpointer handler ) {
  XKeyEvent *xevent = (XKeyEvent*) gdk_xevent;
    Display   *dpy  = GDK_WINDOW_XDISPLAY( gdk_get_default_root_window() );

  if ( xevent->type == KeyPress ) {

    int mask    = xevent->state & ~( GDK_LOCK_MASK | GDK_MOD5_MASK | GDK_MOD2_MASK );
    int keysym  = XKeycodeToKeysym( dpy, xevent->keycode, 0 );

    ( (GlobalKeyHandler*)handler )->handleGlobalKey( mask, keysym );
  }

  return GDK_FILTER_CONTINUE;
}


int lookupKey( const char * key ) {
  return XStringToKeysym( key );
}


void globalBinding( GlobalKeyHandler *handler, int modMask, int keySym ) {
  GdkWindow *root = gdk_get_default_root_window();

  //Bind to every possible modifier we want to ignore. NumLock, ScrollLock, and CapsLock.
  int modMasks [] = {
    0,
    GDK_MOD5_MASK, //Scroll Lock
    GDK_MOD2_MASK, //Num Lock
    GDK_LOCK_MASK, //Caps Lock
    GDK_MOD5_MASK | GDK_MOD2_MASK,
    GDK_MOD5_MASK | GDK_LOCK_MASK,
    GDK_MOD2_MASK | GDK_LOCK_MASK,
    GDK_MOD5_MASK | GDK_MOD2_MASK | GDK_LOCK_MASK,
  };

  for ( unsigned int i = 0; i < G_N_ELEMENTS( modMasks ); ++i ) {
    XGrabKey( GDK_WINDOW_XDISPLAY( root ),
              XKeysymToKeycode( GDK_WINDOW_XDISPLAY( root ) , keySym ),
              modMasks[i] | modMask,
              GDK_WINDOW_XID( root ),
              False,
              GrabModeAsync,
              GrabModeAsync );
  }

  gdk_window_add_filter( root, keyFilter, handler );
}


static int x11ErrorHandler( Display *dpy, XErrorEvent *error ) {
  int  buffer_len = 100;
  char *buffer    = new char[buffer_len];

  XGetErrorText( dpy, error->error_code, buffer, buffer_len );

  fprintf( stderr, "%s\n", buffer );

  delete buffer;

  return 1;
}


void x11Init() {
  XSetErrorHandler( x11ErrorHandler );
}
