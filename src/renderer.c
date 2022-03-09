#include "renderer.h"
#include "canvas.h"

#include <stdlib.h>

struct raytracer_renderer
{
	raytracer_canvas *canvas;
};

raytracer_renderer *
renderer_init(raytracer_canvas *canvas)
{
	raytracer_renderer *r = malloc(sizeof(raytracer_renderer));
	r->canvas = canvas;

	return r;
}

void
renderer_draw(raytracer_renderer *renderer)
{
	i32 width = canvas_get_width(renderer->canvas);
	i32 height = canvas_get_height(renderer->canvas);

	color32 startColor = 0xFF;
	color32 endColor = 0xFF00FF;
	i16 dR = (i16)((endColor >> 16) & 0xFF) - (i16)((startColor >> 16) & 0xFF);
	i16 dG = (i16)((endColor >> 8) & 0xFF) - (i16)((startColor >> 8) & 0xFF);
	i16 dB = (i16)((endColor) & 0xFF) - (i16)((startColor) & 0xFF);

	for(i32 y = 0; y < height; ++y)
	{
		for(i32 x = 0; x < width; ++x)
		{
			real32 horiPerc = (real32)(x+1)/(real32)width;

			canvas_put_pixel(renderer->canvas, x, y, ((u32)(((startColor >> 16) & 0xFF) + 
						(i16)(dR*horiPerc)) << 16) |
				((u32)(((startColor >> 8) & 0xFF) + (i16)(horiPerc*dG)) << 8) |
				((u32)(((startColor) & 0xFF) + (i16)(horiPerc*dB))));
		}
	}
}
