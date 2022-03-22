#ifndef __CANVAS_H
#define __CANVAS_H

#include "stdinc.h"

#include <X11/Xlib.h>

typedef struct raytracer_canvas raytracer_canvas;

#define CANVAS_TEXT_NULL (-1)

extern raytracer_canvas *
canvas_create(Display *display, raytracer_canvas *parent, i32 width, i32 height);

extern void
canvas_show(raytracer_canvas *canvas);

extern void
canvas_hide(raytracer_canvas *canvas);

extern b32
canvas_is_show(raytracer_canvas *canvas);

extern void
canvas_resize(raytracer_canvas *canvas, i32 width, i32 height);

extern void
canvas_put_pixel(raytracer_canvas *canvas, i32 x, i32 y, color32 c);

extern void
canvas_put_square(raytracer_canvas *canvas, i32 x, i32 y, 
		i32 width, i32 height, color32 c);

extern i32
canvas_text_create(raytracer_canvas *canvas);

extern void
canvas_text_set(raytracer_canvas *canvas, i32 textId, i32 x, i32 y, const char *str);

extern void
canvas_text_toggle(raytracer_canvas *canvas, i32 textId);

extern b32
canvas_text_is_show(raytracer_canvas *canvas, i32 textId);

extern Window
canvas_get_window(raytracer_canvas *canvas);

extern i32
canvas_get_width(raytracer_canvas *canvas);

extern i32
canvas_get_height(raytracer_canvas *canvas);

extern u32 *
canvas_get_buffer(raytracer_canvas *canvas);

extern void
canvas_flip(raytracer_canvas *canvas);

#endif
