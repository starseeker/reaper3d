


#include "hw/compat.h"

#include <string>

#include "hw/gfx.h"
#include "hw/gfx_driver.h"
#include "hw/gfx_types.h"
#include "hw/debug.h"

#include "hw/interfaces.h"
#include "hw/exceptions.h"

#include "hw/gl.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <GL/glx.h>

#ifdef XF86VM_EXT
#include <X11/extensions/xf86vmode.h>
#endif




namespace reaper
{
namespace hw
{
namespace lowlevel
{


using std::string;


struct MotifWmHints {
	long flags;
	long functions;
	long decorations;
	long input_mode;
};
#define MWM_HINTS_DECORATIONS 2

#ifdef XF86VM_EXT
typedef XF86VidModeModeInfo video_mode;
#else
typedef int video_mode;
#endif

class Gfx_x11
 : public Gfx_driver
{
	Display* display;
	int screen;
	Window root;
	Window window;
	XVisualInfo* visual;
	GC gc;
	Colormap colormap;
	GLXContext glxcontext;
	int num_modes;
	bool fullscreen;
	video_mode** modes;
	Gfx_driver_data data;
	ifs::Gfx* main;
	bool nograb;

	void fix_scr(int width, int height);
public:
	Gfx_x11(ifs::Gfx* m);
	~Gfx_x11();
	Gfx_driver_data* internal_data() { return &data; }
	void update_screen();
	bool setup_mode(const gfx::VideoMode&);
	void restore_mode();
};

Gfx_x11::Gfx_x11(ifs::Gfx* m)
 : display(0), screen(0), visual(0), main(m), nograb(false)
{

	if (! (display = X11::XOpenDisplay(0))) {
		throw hw_fatal_error("Cannot open display: " +
				     string(X11::XDisplayName(0)));
	}
	data.display = display;
	screen = DefaultScreen(display);
	root = XRootWindow(display, screen);
	window = 0;

	data.desktop.width  = DisplayWidth(display, screen);
	data.desktop.height = DisplayHeight(display, screen);
	data.desktop.depth  = DefaultDepth(display, screen);
#ifdef XF86VM_EXT
	data.desktop.fullscreen = true;
	XF86VidModeGetAllModeLines(display, screen, &num_modes, &modes);

	for (int i = 0; i < num_modes; i++) {
		gfx::VideoMode mode( modes[i]->hdisplay,
				modes[i]->vdisplay,
				data.desktop.depth, true, 0, i);
		main->add_mode(mode);
	}
#else
	data.desktop.fullscreen = false;
	num_modes = 0;
#endif
	nograb = main->config("nograb") == "true";
	main->derr() << nograb << " initialized\n";
}

Gfx_x11::~Gfx_x11()
{
	main->derr() << "shutdown\n";
}


int attribs[][7] = {
	{ GLX_RGBA, GLX_ALPHA_SIZE, 8, GLX_STENCIL_SIZE, 8, GLX_DOUBLEBUFFER, None },
	{ GLX_RGBA, GLX_ALPHA_SIZE, 8, GLX_DOUBLEBUFFER, None, 0,0 },
	{ GLX_RGBA, GLX_DOUBLEBUFFER, None, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 }
};
        


bool Gfx_x11::setup_mode(const gfx::VideoMode& mode)
{
	visual = 0;
	for (int i = 0; attribs[i][0] && !visual; i++) {
		data.alpha = attribs[i][1] == GLX_ALPHA_SIZE;
		data.stencil = attribs[i][3] == GLX_STENCIL_SIZE;
		visual = glXChooseVisual(display, screen, attribs[i]);
	}
	if (!visual)
		throw hw_error("glXChooseVisual failed");


	glxcontext = glXCreateContext(display, visual, 0, 1);
	if (!glxcontext)
		throw hw_error("glXCreateContext error");

	XSetWindowAttributes attr;
	attr.background_pixel = BlackPixel(display, screen);
	attr.colormap = XCreateColormap(display, root,
					visual->visual, AllocNone);
	unsigned long vmask = CWBackPixel | CWColormap;

	if (mode.fullscreen) {
		vmask |= CWOverrideRedirect;
		attr.override_redirect = 1;
	}
	
	window = XCreateWindow(display, root, 0, 0,
			mode.width, mode.height, 0, 
			visual->depth, InputOutput, 
			visual->visual, vmask, &attr);

	long eventmask  = KeyPressMask | KeyReleaseMask | StructureNotifyMask
			| ExposureMask | PointerMotionMask | FocusChangeMask
			| ButtonPressMask | ButtonReleaseMask;

	XSelectInput(display, window, eventmask);

	Pixmap pix;
	XColor dmy;
	char dt[1] = {0};
	Cursor cursor;
	pix = XCreateBitmapFromData(display, window, dt, 1, 1);
	cursor = XCreatePixmapCursor(display, pix, pix, &dmy, &dmy, 0, 0);
	XFreePixmap(display, pix);

	XDefineCursor(display, window, cursor);

	glXMakeCurrent(display, window, glxcontext);
	glXWaitX();
	XMapWindow(display, window);

	if (mode.fullscreen) {

		Atom motifhints = XInternAtom(display, "_MOTIF_WM_HINTS", 0);
		MotifWmHints wmhints;
		wmhints.flags = MWM_HINTS_DECORATIONS;
		wmhints.decorations = 0;
		XChangeProperty(display, window, motifhints,
				motifhints, 32, PropModeReplace,
				(unsigned char*) &wmhints, 4);


#ifdef XF86VM_EXT
		XF86VidModeSwitchToMode(display, screen, modes[mode.id]);
		XF86VidModeSetViewPort(display, screen, 0, 0);
#endif



		XGrabPointer(display, window, True, 0, 
			      GrabModeAsync, GrabModeAsync, window, 
			      None, CurrentTime);

		XGrabKeyboard(display, window, True,
			      GrabModeAsync, GrabModeAsync, CurrentTime);
	}
	fullscreen = mode.fullscreen;
	data.current = mode;

	const char *pr = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	data.is_accelerated = ! strstr(pr, "Mesa");

	return true;
}

void Gfx_x11::fix_scr(int width, int height)
{
	glXWaitX();
	data.current.width = width;
	data.current.height = height;
	glXSwapBuffers(display, window);
}

void Gfx_x11::restore_mode() {
	if (window) {
		if (fullscreen) {
			XUngrabPointer(display, CurrentTime);
			XUngrabKeyboard(display, CurrentTime);
#ifdef XF86VM_EXT
			XF86VidModeSwitchToMode(display, screen, modes[0]);
#endif
			glXWaitX();
		}
		glXMakeCurrent(display, None, 0);
		XUnmapWindow(display, window);
		XDestroyWindow(display, window);
		XEvent ev;
		while (XCheckMaskEvent(display, StructureNotifyMask, &ev))
			;
	}
	fullscreen = false;
}

void Gfx_x11::update_screen() {
	XEvent ev;
	while (XCheckMaskEvent(display, FocusChangeMask|StructureNotifyMask|ExposureMask, &ev)) {
		switch (ev.type) {
//		case MotionNotify:    main->derr() << "MotionNotify\n";    break;
//		case Expose: 	      main->derr() << "Expose\n";          break;
//		case UnmapNotify:     main->derr() << "UnmapNotify\n";     break;
		case FocusIn:
		case MapNotify:
			if (!nograb) {
				XGrabPointer(display, window, True, 0, 
					GrabModeAsync, GrabModeAsync, window, 
					None, CurrentTime);
			}
//			main->derr() << "MapNotify\n";
			break;
		case FocusOut:
			XUngrabPointer(display, CurrentTime);
			break;
//		case MapRequest:      main->derr() << "MapRequest\n";      break;
		case ConfigureNotify:
			fix_scr(ev.xconfigure.width, ev.xconfigure.height);
//			main->derr() << "ConfigureNotify\n";
			break;
//		case ReparentNotify:  main->derr() << "ReparentNotify\n";  break;
		default:
			main->derr() << "msg: " << ev.type << '\n';
			break;
		}
	}
	glXSwapBuffers(display, window);
}


}
}
}

extern "C" {
void* create_gfx_x11(reaper::hw::ifs::Gfx* m)
{
	return new reaper::hw::lowlevel::Gfx_x11(m);
}
}

