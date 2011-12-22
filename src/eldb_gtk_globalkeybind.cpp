//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

//TODO: This only works with X11. Support more platforms. 
//      Binding is hard coded. Make configurable.
//      Error checking

extern "C" {
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
}


static GdkFilterReturn keyFilter( GdkXEvent *gdk_xevent, GdkEvent *event, gpointer gtkWindow ) {
  Window    window  = GDK_WINDOW_XWINDOW( GTK_WIDGET( gtkWindow )->window );
  Window    root    = GDK_WINDOW_XWINDOW( gdk_get_default_root_window() );
  Display   *dpy    = GDK_WINDOW_XDISPLAY( gdk_get_default_root_window() );
  XKeyEvent *xevent = (XKeyEvent*) gdk_xevent;
  GdkScreen *screen = gtk_widget_get_screen( GTK_WIDGET( gtkWindow ) );
  XEvent    msgEvent;

  if ( xevent->type == KeyPress ) {
    if ( gtk_widget_get_visible( GTK_WIDGET( gtkWindow ) ) ) {
      gtk_widget_hide( GTK_WIDGET( gtkWindow ) );
    } else {
      if ( XKeycodeToKeysym( dpy, xevent->keycode, 0 ) == XStringToKeysym( "F9" )
          && Mod1Mask == ( xevent->state & ~( GDK_LOCK_MASK | GDK_MOD5_MASK | GDK_MOD2_MASK ) ) ) {

        gtk_window_present( GTK_WINDOW( gtkWindow ) );

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
    }
  }
  return GDK_FILTER_CONTINUE;
}


void globalBinding( GtkWindow *window ) {
  GdkWindow *root = gdk_get_default_root_window();

  //Bind to every possible modifier we want to ignore. NumLock, ScrollLock, and CapsLock.
  int modMasks [] = {
    0,
    GDK_MOD5_MASK,
    GDK_MOD2_MASK,
    GDK_LOCK_MASK,
    GDK_MOD5_MASK | GDK_MOD2_MASK,
    GDK_MOD5_MASK | GDK_LOCK_MASK,
    GDK_MOD2_MASK | GDK_LOCK_MASK,
    GDK_MOD5_MASK | GDK_MOD2_MASK | GDK_LOCK_MASK,
  };

  for ( unsigned int i = 0; i < G_N_ELEMENTS( modMasks ); ++i ) {
    XGrabKey( GDK_WINDOW_XDISPLAY( root ), 
              XKeysymToKeycode( GDK_WINDOW_XDISPLAY( root ) , XStringToKeysym( "F9" ) ),
              modMasks[i] | GDK_MOD1_MASK, 
              GDK_WINDOW_XID( root ), 
              False, 
              GrabModeAsync,
              GrabModeAsync );
  }

  gdk_window_add_filter( root, keyFilter, window );
}
