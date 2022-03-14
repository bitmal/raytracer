#include "canvas.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdlib.h>
#include <string.h>

typedef struct canvas_text
{
	i32 x;
	i32 y;
	char *textBuffer;
	i32 textBufferSize;
} canvas_text;

struct raytracer_canvas
{
	i32 width;
	i32 height;
	canvas_text *texts;
	i32 textCount;
	
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

	canvas->texts = NULL;
	canvas->textCount = 0;

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

i32
canvas_text_create(raytracer_canvas *canvas)
{
	i32 index = canvas->textCount;

	if(canvas->textCount > 0)
	{
		canvas->texts = realloc(canvas->texts, sizeof(canvas_text)*(++canvas->textCount));
	}
	else
	{
		canvas->texts = malloc(sizeof(canvas_text)*(++canvas->textCount));
	}

	canvas->texts[index].x = 0;
	canvas->texts[index].y = 0;
	canvas->texts[index].textBuffer = NULL;
	canvas->texts[index].textBufferSize = 0;

	return index;
}

void
canvas_text_set(raytracer_canvas *canvas, i32 textId, i32 x, i32 y, const char *str)
{
	canvas_text *text = &canvas->texts[textId];

	text->x = x;
	text->y = y;

	i32 strLength = 0;
	for(const char *c = str; *c; ++c, ++strLength);

	if(strLength > 0)
	{
		if(strLength > (text->textBufferSize + 1))
		{
			if(text->textBufferSize > 0)
			{
				text->textBuffer = realloc(text->textBuffer, strLength + 1);
			}
			else
			{
				text->textBuffer = malloc(strLength + 1);
			}

			text->textBufferSize = strLength + 1;
			strcpy(text->textBuffer, str);
		}
		else
		{
			strcpy(text->textBuffer, str);
		}
	}
	else
	{
		if(text->textBufferSize < 1)
		{
			text->textBuffer = malloc(1);
			text->textBufferSize = 1;
		}

		text->textBuffer[0] = '\0';
	}
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
		
		XFlush(canvas->xlib.display);
		
		for(i32 i = 0; i < canvas->textCount; ++i)
		{
			canvas_text *text = &canvas->texts[i];

			if(text->textBufferSize > 0)
			{
				i32 strLength = 0;
				for(const char *c = text->textBuffer; *c; ++c, ++strLength);

				XDrawImageString(canvas->xlib.display, canvas->xlib.window, 
						DefaultGC(canvas->xlib.display, canvas->xlib.screen), text->x, text->y, 
						text->textBuffer, strLength);
			}
		}

		XFlush(canvas->xlib.display);

		canvas->buffer.isDirty = B32_FALSE;
	}
}
