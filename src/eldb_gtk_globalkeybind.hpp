//Copyright (c) 2011 Krishna Rajendran <krishna@emptybox.org>.
//Licensed under an MIT/X11 license. See LICENSE file for details.

#ifndef _GTK_GLOBALBINDING_H_
#define _GTK_GLOBALBINDING_H_

struct GlobalKeyHandler {
  virtual void handleGlobalKey( int mask, int keysym ) = 0;
};

void globalBinding( GlobalKeyHandler *handler, int modMask, int keySym );
void x11Init();
int lookupKey( const char * key );
void raiseWindow( GtkWindow *gtkWindow );

#endif
