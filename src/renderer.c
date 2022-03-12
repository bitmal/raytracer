#include "renderer.h"
#include "canvas.h"
#include "scene.h"

#include <stdlib.h>

struct raytracer_renderer
{
	raytracer_canvas *canvas;
	i32 *sphereIds;
	i32 sphereIdCount;
	color32 backgroundColor;
};

raytracer_renderer *
renderer_init(raytracer_canvas *canvas)
{
	raytracer_renderer *r = malloc(sizeof(raytracer_renderer));
	r->canvas = canvas;
	r->sphereIds = NULL;
	r->sphereIdCount = 0;

	return r;
}

void
renderer_push_sphere(raytracer_renderer *renderer, i32 sphereId)
{
	i32 index = renderer->sphereIdCount;

	if(renderer->sphereIdCount > 0)
	{
		renderer->sphereIds = realloc(renderer->sphereIds, 
				sizeof(i32)*(++renderer->sphereIdCount));
	}
	else
	{
		renderer->sphereIds = malloc(sizeof(i32)*(++renderer->sphereIdCount));
	}

	renderer->sphereIds[index] = sphereId;
}

void
renderer_draw(raytracer_renderer *renderer, raytracer_scene *scene)
{
	real32 pixelDensity = scene_get_pixel_density(scene);
	i32 width = canvas_get_width(renderer->canvas);
	i32 height = canvas_get_height(renderer->canvas);

#if 1
	// interpolate color to background
	i32 partitionWidth = width*(pixelDensity/width);
	i32 partitionHeight = height*(pixelDensity/height);

	i32 xPartitionCount = width/partitionWidth;
	i32 yPartitionCount = height/partitionHeight;

	for(i32 _y = 0; _y < yPartitionCount; ++_y)
	{
		i32 y = _y*partitionHeight;

		for(i32 _x = 0; _x < xPartitionCount; ++_x)
		{
			i32 x = _x*partitionWidth;

			for(i32 pY = 0; pY < partitionHeight; ++pY)
			{
				for(i32 pX = 0; pX < partitionWidth; ++pX)
				{
					v4 cameraPosition;
					scene_get_camera_position(scene, &cameraPosition);

					v4 viewportPoint;
					scene_canvas_to_world_coordinates(scene, renderer->canvas, x, y, &viewportPoint);

					color32 result;

					if(scene_trace_ray(scene, &viewportPoint, &result))
					{
						canvas_put_pixel(renderer->canvas, x+pX, y+pY, result);
					}
					else
					{
						canvas_put_pixel(renderer->canvas, x+pX, y+pY, 0x0);
					}
				}
			}
		}
	}
#endif
}
