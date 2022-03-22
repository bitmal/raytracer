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
	i32 textBufferCapacity;
	b32 isShow;
} canvas_text;

struct raytracer_canvas
{
	raytracer_canvas *parent;
	raytracer_canvas *next;
	raytracer_canvas *children;
	i32 width;
	i32 height;
	canvas_text *texts;
	i32 textCount;
	b32 isShow;
	
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
canvas_create(Display *display, raytracer_canvas *parent, i32 width, i32 height)
{
	raytracer_canvas *canvas = malloc(sizeof(raytracer_canvas));
	canvas->parent = parent;
	canvas->children = NULL;

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
	
	if(parent)
	{
		canvas->isShow = B32_FALSE;
		canvas->next = parent->children;
		parent->children = canvas;
	}
	else
	{
		XMapWindow(display, canvas->xlib.window);

		canvas->isShow = B32_TRUE;
		canvas->next = NULL;
	}

	canvas->buffer.isDirty = B32_TRUE;
	canvas->buffer.size = width*height*sizeof(u32);
	canvas->buffer._ = calloc(1, sizeof(u32)*width*height);
	canvas->buffer.xImage = XCreateImage(display, CopyFromParent, canvas->xlib.depth, 
			ZPixmap, 0, (char *)canvas->buffer._, width, height, 32, 0);

	XInitImage(canvas->buffer.xImage);

	canvas->texts = NULL;
	canvas->textCount = 0;

	return canvas;
}

void
canvas_show(raytracer_canvas *canvas)
{
	if(canvas->parent)
	{
		if(!canvas->isShow)
		{
			XMapWindow(canvas->xlib.display, canvas->xlib.window);
			canvas->isShow = B32_TRUE;
		}
		else
		{
			fprintf(stderr, "Cannot show canvas. It is already being showed!\n");
		}
	}
	else
	{
		fprintf(stderr, "Cannot toggle main canvas!\n");
	}
}

void
canvas_hide(raytracer_canvas *canvas)
{
	if(canvas->parent)
	{
		if(canvas->isShow)
		{
			XUnmapWindow(canvas->xlib.display, canvas->xlib.window);
			canvas->isShow = B32_FALSE;
		}
		else
		{
			fprintf(stderr, "Cannot hide canvas. It is already hidden!\n");
		}
	}
	else
	{
		fprintf(stderr, "Cannot toggle main canvas!\n");
	}
}

b32
canvas_is_show(raytracer_canvas *canvas)
{
	return canvas->isShow;
}

void
canvas_resize(raytracer_canvas *canvas, i32 width, i32 height)
{
	XSizeHints windowHints;
	windowHints.flags = PMinSize | PMaxSize;
	windowHints.min_width = width;
	windowHints.max_width = width;
	windowHints.min_height = height;
	windowHints.max_height = height;

	XSetWMNormalHints(canvas->xlib.display, canvas->xlib.window, &windowHints);
	
	canvas->buffer.size = width*height*sizeof(u32);
	canvas->width = width;
	canvas->height = height;

	XDestroyImage(canvas->buffer.xImage);
	
	canvas->buffer._ = malloc(canvas->buffer.size);
	
	canvas->buffer.xImage = XCreateImage(canvas->xlib.display, CopyFromParent, canvas->xlib.depth, 
			ZPixmap, 0, (char *)canvas->buffer._, width, height, 32, 0);
	XInitImage(canvas->buffer.xImage);
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
	canvas->texts[index].textBufferCapacity = 100;
	canvas->texts[index].textBuffer = malloc(canvas->texts[index].textBufferCapacity);
	canvas->texts[index].textBuffer[0] = '\0';
	canvas->texts[index].textBufferSize = 1;
	canvas->texts[index].isShow = B32_TRUE;

	return index;
}

void
canvas_text_set(raytracer_canvas *canvas, i32 textId, i32 x, i32 y, const char *str)
{
	canvas_text *text = &canvas->texts[textId];

	text->x = x;
	text->y = y;

	i32 strLength = strlen(str);
	i32 totalLength = strLength + 1;

	if(totalLength > text->textBufferCapacity)
	{
		text->textBuffer = realloc(text->textBuffer, text->textBufferCapacity = totalLength + 
				25);
	}

	sprintf(text->textBuffer, "%s", str);
	text->textBufferSize = totalLength;
}

void
canvas_text_append(raytracer_canvas *canvas, i32 textId, const char *str)
{
	canvas_text *text = &canvas->texts[textId];

	i32 strLength = strlen(str) + 1;
	i32 textLength = text->textBufferSize;
	i32 totalLength = textLength + strLength - 1;

	if(totalLength > text->textBufferCapacity)
	{
		text->textBuffer = realloc(text->textBuffer, text->textBufferCapacity = totalLength + 
				25);
	}

	strcat(text->textBuffer, str);
	text->textBufferSize = totalLength;
}

void
canvas_text_toggle(raytracer_canvas *canvas, i32 textId)
{
	canvas->texts[textId].isShow = !canvas->texts[textId].isShow;
}

b32
canvas_text_is_show(raytracer_canvas *canvas, i32 textId)
{
	return canvas->texts[textId].isShow;
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

			if(text->isShow)
			{
				if(text->textBufferSize > 0)
				{
					i32 strLength = 0;
					for(const char *c = text->textBuffer; *c; ++c, ++strLength);

					XDrawImageString(canvas->xlib.display, canvas->xlib.window, 
							DefaultGC(canvas->xlib.display, canvas->xlib.screen), text->x, text->y, 
							text->textBuffer, strLength);
				}
			}
		}

		XFlush(canvas->xlib.display);

		canvas->buffer.isDirty = B32_FALSE;
	}
}
