#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* needed for nanosleep and timespec:
* https://stackoverflow.com/questions/3875197/
* gcc-with-std-c99-complains-about-not-knowing-struct-timespec
*/

#include "stdinc.h"
#include "rt_math.h"
#include "canvas.h"
#include "scene.h"
#include "renderer.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tgmath.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define PIXEL_DENSITY 10.f 
#define MS_DELAY 100

int
main(int argc, char **argv)
{
	Display *display = XOpenDisplay(NULL);
	if(!display)
	{
		fprintf(stderr, "Unable to open X Display!\n");
		return -1;
	}

	raytracer_canvas *canvas = canvas_create(display, WINDOW_WIDTH, WINDOW_HEIGHT);
	raytracer_scene *scene = scene_init();
	raytracer_renderer *renderer = renderer_init(canvas);

#define SCENE_1

// SCENE_0
#ifdef SCENE_0 
	scene_set_pixel_density(scene, PIXEL_DENSITY);

	v4 dirLightDirection = {{0.f, -1.f, 0.f, 0.f}};
	i32 dirLight = scene_create_directional_light(scene, &dirLightDirection, 1.f);

	v4 spherePosition = {{0, -1, 3, 0.f}};
	i32 sphereId = scene_create_sphere(scene, &spherePosition, 1.f, 0xFF0000);
	renderer_push_sphere(renderer, sphereId);
	v4 spherePosition1 = {{-1.5, 0, 4, 0.f}};
	i32 sphereId1 = scene_create_sphere(scene, &spherePosition1, 1.f, 0xFFFF);
	renderer_push_sphere(renderer, sphereId1);
	v4 spherePosition2 = {{-1, -1, 3, 0.f}};
	i32 sphereId2 = scene_create_sphere(scene, &spherePosition2, 1.f, 0xFF00);
	renderer_push_sphere(renderer, sphereId2);
	v4 spherePosition3 = {{0.75f, -1, 2.5f, 0.f}};
	i32 sphereId3 = scene_create_sphere(scene, &spherePosition3, 0.75f, 0xFF00FF);
	renderer_push_sphere(renderer, sphereId3);
	v4 spherePosition4 = {{0.75f, -0.25f, 2.5f, 0.f}};
	i32 sphereId4 = scene_create_sphere(scene, &spherePosition4, 0.0625f, 0xFFFF00);
	renderer_push_sphere(renderer, sphereId4);
#endif

// SCENE_1
#ifdef SCENE_1
#define OBJECT_COUNT 10 
	v4 dirLightDirection = {{0.f, -1.f, 0.f, 0.f}};
	i32 dirLight = scene_create_directional_light(scene, &dirLightDirection, 1.f);

	i32 partitionWidth = WINDOW_WIDTH/(OBJECT_COUNT/2);
	i32 partitionHeight = WINDOW_HEIGHT/(OBJECT_COUNT/2);

	i32 objectCountHorizontal = OBJECT_COUNT/2;
	i32 objectCountVertical = OBJECT_COUNT/2;

	for(i32 objectY = 0; objectY < objectCountVertical; ++objectY)
	{
		i32 y = partitionHeight*objectY + partitionHeight/2;

		for(i32 objectX = 0; objectX < objectCountHorizontal; ++objectX)
		{
			i32 x = partitionWidth*objectX + partitionWidth/2;

			v4 spherePosition;
			scene_canvas_to_world_coordinates(scene, canvas, x, y, &spherePosition);

			i32 sphereId = scene_create_sphere(scene, &spherePosition, (real32)(OBJECT_COUNT)/partitionWidth, 0xFF00FF);
			renderer_push_sphere(renderer, sphereId);
		}
	}
#endif

	XMapWindow(display, canvas_get_window(canvas));
	XSync(display, False);

	b32 isRunning = B32_TRUE;

#ifdef SCENE_0
	real32 lightIntensity = 1.f;
#endif

	while (isRunning)
	{
		XEvent evt;

		while(XPending(display))
		{
			XNextEvent(display, &evt);

			switch(evt.type)
			{
				case ButtonPress:
				{
				} break;

				case KeyPress:
				{
#ifdef SCENE_0 
					KeySym sym = XkbKeycodeToKeysym(display, evt.xkey.keycode, 0, 0);

					if(sym == XK_a)
					{
						lightIntensity -= 0.1f;

						if(lightIntensity < 0.f)
						{
							lightIntensity = 0.f;
						}

						scene_set_directional_light_intensity(scene, dirLight, lightIntensity);
					}
					else if(sym == XK_d)
					{
						lightIntensity += 0.1f;

						if(lightIntensity > 1.f)
						{
							lightIntensity = 1.f;
						}

						scene_set_directional_light_intensity(scene, dirLight, lightIntensity);
					}
#endif
				} break;

				case KeyRelease:
				{
					KeySym sym = XkbKeycodeToKeysym(display, evt.xkey.keycode, 0, 0);

					if(sym == XK_Escape)
					{
						isRunning = B32_FALSE;
					}
				} break;
			}
		}

		renderer_draw(renderer, scene);
		canvas_flip(canvas);

		struct timespec delayTime = {0, 1000000*MS_DELAY};
		nanosleep(&delayTime, NULL);
	}

	return 0;
}

#include "rt_math.c"
#include "canvas.c"
#include "scene.c"
#include "renderer.c"
