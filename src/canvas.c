#include "canvas.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdlib.h>

struct raytracer_canvas
{
	i32 width;
	i32 height;
	
	struct
	{
		Display *display;
		Window window;
		int screen;
		int depth;
		Pixmap bg_pixmap;
	} xlib;

	struct
	{
		i32 size;
		XImage *xImage;
		u32 *_;
		b32 isDirty;
	} buffer;
};

raytracer_canvas *
canvas_create(Display *display, i32 width, i32 height)
{
	raytracer_canvas *canvas = malloc(sizeof(raytracer_canvas));
	canvas->width = width;
	canvas->height = height;
	canvas->xlib.display = display;
	canvas->xlib.screen = DefaultScreen(display);
	canvas->xlib.depth = DefaultDepth(display, canvas->xlib.screen);
	canvas->xlib.bg_pixmap = XCreatePixmap(display, RootWindow(display, canvas->xlib.screen),
			width, height, canvas->xlib.depth);
	
	XSetWindowAttributes windowAttribs = {};
	windowAttribs.background_pixmap = canvas->xlib.bg_pixmap;
	windowAttribs.background_pixel = BlackPixel(display, canvas->xlib.screen);
	windowAttribs.event_mask = KeyPress | KeyRelease | ButtonPress | ButtonRelease;

	canvas->xlib.window = XCreateWindow(display, 
			RootWindow(display, canvas->xlib.screen), 
			0, 0, width, height, 0, 
			CopyFromParent, CopyFromParent, 
			CopyFromParent, CWBackPixel | CWBackPixmap | CWEventMask,
			&windowAttribs);

	XSizeHints windowHints;
	windowHints.flags = PMinSize | PMaxSize;
	windowHints.min_width = width;
	windowHints.max_width = width;
	windowHints.min_height = height;
	windowHints.max_height = height;

	XSetWMNormalHints(display, canvas->xlib.window, &windowHints);
	
	XStoreName(display, canvas->xlib.window, "Raytracer");

	canvas->buffer.isDirty = B32_TRUE;
	canvas->buffer.size = width*height*sizeof(u32);
	canvas->buffer._ = calloc(1, sizeof(u32)*width*height);
	canvas->buffer.xImage = XCreateImage(display, CopyFromParent, canvas->xlib.depth, 
			ZPixmap, 0, (char *)canvas->buffer._, width, height, 32, 0);

	return canvas;
}

void
canvas_put_pixel(raytracer_canvas *canvas, i32 x, i32 y, color32 c)
{
	canvas->buffer._[canvas->width*y + x] = c;
	canvas->buffer.isDirty = B32_TRUE;
}

void
canvas_put_square(raytracer_canvas *canvas, i32 x, i32 y, 
		i32 width, i32 height, color32 c)
{
	i32 maxX = x + width - 1;
	i32 maxY = y + height - 1;

	for(i32 _y = y; _y <= maxY; ++_y)
	{
		for(i32 _x = x; _x <= maxX; ++_x)
		{
			canvas->buffer._[canvas->width*_y + _x] = c;
		}
	}

	canvas->buffer.isDirty = B32_TRUE;
}

Window
canvas_get_window(raytracer_canvas *canvas)
{
	return canvas->xlib.window;
}

i32
canvas_get_width(raytracer_canvas *canvas)
{
	return canvas->width;
}

i32
canvas_get_height(raytracer_canvas *canvas)
{
	return canvas->height;
}

u32 *
canvas_get_buffer(raytracer_canvas *canvas)
{
	canvas->buffer.isDirty = B32_TRUE;
	return canvas->buffer._;
}

void
canvas_flip(raytracer_canvas *canvas)
{
#define CANVAS_INTERVAL 20

	if (canvas->buffer.isDirty)
	{
		for(i32 i = 0; i < canvas->height; i += CANVAS_INTERVAL)
		{
			XPutImage(canvas->xlib.display, canvas->xlib.window, DefaultGC(canvas->xlib.display, 
						canvas->xlib.screen), canvas->buffer.xImage, 0, i, 0, i, canvas->width, 
					CANVAS_INTERVAL);
		}

		i32 modInterval = canvas->height%CANVAS_INTERVAL;

		if(modInterval > 0)
		{
			i32 startHeight = canvas->height - modInterval - 1;

			XPutImage(canvas->xlib.display, canvas->xlib.window, DefaultGC(canvas->xlib.display, 
						canvas->xlib.screen), canvas->buffer.xImage, 0, startHeight, 0, startHeight, 
					canvas->width, modInterval);
		}

		canvas->buffer.isDirty = B32_FALSE;
	}
}
