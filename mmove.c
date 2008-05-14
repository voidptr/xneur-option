#include <X11/Xlib.h> 
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

#include <assert.h>   
#include <unistd.h>   
#include <stdio.h>
#include <stdlib.h>

#define MASK PointerMotionMask
#define MASK2 FocusChangeMask | LeaveWindowMask | EnterWindowMask 

//Window flag;


main()
{
	Display *dpy = XOpenDisplay(NULL);
	assert(dpy);

	Window w;
	int revert_to;
	XGetInputFocus(dpy, &w, &revert_to);

	int dummy;
	unsigned int dummyU;
	Window root_window, child_window;

	Window parent_window = w;			
	while (1)
	{
		XSelectInput(dpy, parent_window, MASK2);
			
		int is_same_screen = XQueryPointer(dpy, parent_window, &root_window, &child_window, 						&dummy, &dummy, &dummy, &dummy, &dummyU);
		if (!is_same_screen || child_window == None)
			break;
		parent_window = child_window;
	}

	XFlush(dpy);

	//XSelectInput(dpy, w, MASK);
	
	
	int root_x, root_y, win_x, win_y;
	XEvent e;
	while (1) 
	{
		XFlush (dpy);
		XNextEvent (dpy, &e);
		int type = e.type;
		switch (type)
		{
			case MotionNotify:
			{
				//update_flag(dpy);
				Window root_window, child_window;
				unsigned int dummyU;
				XQueryPointer(dpy, w, &root_window, &child_window, &root_x, &root_y, &win_x, &win_y, &dummyU);			
				XkbStateRec xkbState;
				XkbGetState(dpy, XkbUseCoreKbd, &xkbState);

				printf ("Mouse Cursor on X=%d and Y=%d (keyboard group %d)\n", root_x, root_y, xkbState.group);
				//XMoveWindow(dpy, flag, root_x+40, root_y+40);
				//XRaiseWindow(dpy, flag);
				break;	

			}
			case EnterNotify:
			{
				//update_flag(dpy);
				printf ("Enter Notify Event. Map Flag Window.\n");
				break;
			}
			case LeaveNotify:
			{
				//update_flag(dpy);
				printf ("Leave Notify Event. Unmap Flag Window.\n");
				break;
			}
			
			/*case FocusIn:
			{
				parent_window = w;			
				while (1)
				{
					XSelectInput(dpy, parent_window, MASK | MASK2);
						
					int is_same_screen = XQueryPointer(dpy, parent_window, &root_window, &child_window, 									&dummy, &dummy, &dummy, &dummy, &dummyU);
					if (!is_same_screen || child_window == None)
						break;
					parent_window = child_window;
				}
				break;
			}*/
			case FocusOut:
			{
				//update_flag(dpy);	
				printf ("Focus In/Out Event\n");
				
				parent_window = w;			
				while (1)
				{
					XSelectInput(dpy, parent_window, MASK2);
						
					int is_same_screen = XQueryPointer(dpy, parent_window, &root_window, &child_window, 									&dummy, &dummy, &dummy, &dummy, &dummyU);
					if (!is_same_screen || child_window == None)
						break;
					parent_window = child_window;
				}

				Window new_w;
				XGetInputFocus(dpy, &new_w, &revert_to);
				
				parent_window = new_w;			
				while (1)
				{
					int mask = MASK2;
					mask |= MASK;
					XSelectInput(dpy, parent_window, mask);
						
					int is_same_screen = XQueryPointer(dpy, parent_window, &root_window, &child_window, 									&dummy, &dummy, &dummy, &dummy, &dummyU);
					if (!is_same_screen || child_window == None)
						break;
					parent_window = child_window;
				}								

				w = new_w;
				//XRaiseWindow(dpy, flag);
				break;
				
			}
		}		
	}
}

