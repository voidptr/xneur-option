#ifndef KEYLAYOUT_X11_H
#define KEYLAYOUT_X11_H
extern "C"
{
#include <X11/XKBlib.h>
}
int get_active_kbd_group(Display *dpy);
int get_kbd_group_count(Display *dpy);
int set_next_kbd_group(Display *dpy);

#endif // KEYLAYOUT_X11_H
