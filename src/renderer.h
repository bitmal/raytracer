#ifndef __RENDERER_H
#define __RENDERER_H

#include "stdinc.h"
#include "rt_math.h"

struct raytracer_canvas;
struct raytracer_scene;

typedef struct raytracer_renderer raytracer_renderer;

#define RENDERER_TEXTURE_NULL -1
#define RENDERER_OVERLAY_NULL -1

extern raytracer_renderer *
renderer_init();

extern void
renderer_draw_scene(raytracer_renderer *renderer, raytracer_canvas *canvas, 
		raytracer_scene *scene);

extern void
renderer_draw_texture(raytracer_renderer *renderer, raytracer_canvas *canvas, 
		i32 textureId);

extern void
renderer_save_next_frame(raytracer_renderer *renderer, const char *filename);

extern i32
renderer_create_texture(raytracer_renderer *renderer, i32 width, i32 height);

extern i32
renderer_create_texture_from_file(raytracer_renderer *renderer, const char *path);

extern i32
renderer_get_texture_width(raytracer_renderer *renderer, i32 textureId);

extern i32
renderer_get_texture_height(raytracer_renderer *renderer, i32 textureId);

extern void
renderer_write_texture(raytracer_renderer *renderer, i32 textureId, i32 dX, i32 dY,
	i32 width, i32 height, u32 *pixels);

extern i32
renderer_create_overlay(raytracer_renderer *renderer, i32 x, i32 y, i32 width, 
		i32 height, i32 backgroundTextureId);

extern void
renderer_toggle_overlay(raytracer_renderer *renderer, i32 overlayId);

#endif
