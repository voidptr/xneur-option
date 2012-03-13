#include "keylayout_x11.h"
#include <string.h>

int get_active_kbd_group(Display *dpy)
{
    if (dpy == NULL)
        return -1;
    XkbStateRec xkbState;
    XkbGetState(dpy, XkbUseCoreKbd, &xkbState);

    return xkbState.group;
}

int get_kbd_group_count(Display *dpy)
{
    if (dpy == NULL)
        return -1;

    XkbDescRec desc[1];
    int gc;
    memset(desc, 0, sizeof(desc));
    desc->device_spec = XkbUseCoreKbd;
    XkbGetControls(dpy, XkbGroupsWrapMask, desc);
    XkbGetNames(dpy, XkbGroupNamesMask, desc);
    gc = desc->ctrls->num_groups;
    XkbFreeControls(desc, XkbGroupsWrapMask, True);
    XkbFreeNames(desc, XkbGroupNamesMask, True);

    return gc;
}

int set_next_kbd_group(Display *dpy)
{
    if (dpy == NULL)
        return -1;

    int active_layout_group = get_active_kbd_group(dpy);

    int new_layout_group = active_layout_group + 1;
    if (new_layout_group == get_kbd_group_count(dpy))
        new_layout_group = 0;

    XkbLockGroup(dpy, XkbUseCoreKbd, new_layout_group);

    return 1;
}
