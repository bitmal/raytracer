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
#include <math.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define PIXEL_SIZE 1.f
#define MS_DELAY 32 

#define PIXEL_SIZE_SHIFT_WEIGHT 1.f

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

	scene_set_pixel_size(scene, PIXEL_SIZE);

#define SCENE_0

// SCENE_0
#ifdef SCENE_0
	i32 ambientLight = scene_create_light(scene, LIGHT_AMBIENT);

	void **ambientLightValues = malloc(sizeof(void *));
	ambientLightValues[0] = malloc(sizeof(real32));
	*(real32*)ambientLightValues[0] = 0.21f;

	light_set_values(scene, ambientLight, 0x4, ambientLightValues);

	free(ambientLightValues[0]);
	free(ambientLightValues);

#if 0	
	i32 directionalLight = scene_create_light(scene, LIGHT_DIRECTIONAL);

	void **directionalLightValues = malloc(sizeof(void *)*2);
	directionalLightValues[0] = malloc(sizeof(v4));
	*(v4 *)directionalLightValues[0] = vec4_init(0, -1, 0, 0);
	directionalLightValues[1] = malloc(sizeof(real32));
	*(real32 *)directionalLightValues[1] = 1.f;
	
	light_set_values(scene, directionalLight, 0x2 | 0x4, directionalLightValues);

	free(directionalLightValues[0]);
	free(directionalLightValues[1]);
	free(directionalLightValues);
#endif
	
	i32 pointLight = scene_create_light(scene, LIGHT_POINT);

	void **pointLightValues = malloc(sizeof(void *)*3);
	pointLightValues[0] = malloc(sizeof(v4));
	*(v4 *)pointLightValues[0] = vec4_init(0.5f, 1, 2, 0);
	pointLightValues[1] = malloc(sizeof(real32));
	*(real32 *)pointLightValues[1] = 1.f;
	pointLightValues[2] = malloc(sizeof(real32));
	*(real32 *)pointLightValues[2] = 5;
	
	light_set_values(scene, pointLight, 0x1 | 0x4 | 0x8, pointLightValues);

	free(pointLightValues[0]);
	free(pointLightValues[1]);
	free(pointLightValues[2]);
	free(pointLightValues);

	v4 spherePosition = {{0, -1, 3, 0.f}};
	i32 sphereId = scene_create_sphere(scene, &spherePosition, 1.f, 0xFF0000, 1.f);
	renderer_push_sphere(renderer, sphereId);
	v4 spherePosition1 = {{-1.5, 0, 4, 0.f}};
	i32 sphereId1 = scene_create_sphere(scene, &spherePosition1, 1.f, 0xFFFF, 1.f);
	renderer_push_sphere(renderer, sphereId1);
	v4 spherePosition2 = {{-1, -1, 3, 0.f}};
	i32 sphereId2 = scene_create_sphere(scene, &spherePosition2, 1.f, 0xFF00, 1.f);
	renderer_push_sphere(renderer, sphereId2);
	v4 spherePosition3 = {{0.75f, -1, 2.5f, 0.f}};
	i32 sphereId3 = scene_create_sphere(scene, &spherePosition3, 0.75f, 0xFF00FF, 1.f);
	renderer_push_sphere(renderer, sphereId3);
	v4 spherePosition4 = {{0.75f, -0.25f, 2.5f, 0.f}};
	i32 sphereId4 = scene_create_sphere(scene, &spherePosition4, 0.0625f, 0xFFFF00, 1.f);
	renderer_push_sphere(renderer, sphereId4);
#endif

// SCENE_1
#ifdef SCENE_1
#define OBJECT_COUNT 10 
	i32 ambientLight = scene_create_light(scene, LIGHT_AMBIENT);

	void **ambientLightValues = malloc(sizeof(void *));
	ambientLightValues[0] = malloc(sizeof(real32));
	*(real32*)ambientLightValues[0] = 0.21f;

	light_set_values(scene, ambientLight, 0x4, ambientLightValues);

	free(ambientLightValues[0]);
	free(ambientLightValues);

	i32 pointLight = scene_create_light(scene, LIGHT_POINT);

	void **pointLightValues = malloc(sizeof(void *)*2);
	pointLightValues[0] = malloc(sizeof(v4));
	*(v4 *)pointLightValues[0] = vec4_init(-1, 1, 2, 0);
	pointLightValues[1] = malloc(sizeof(real32));
	*(real32 *)pointLightValues[1] = 1.f;
	
	light_set_values(scene, pointLight, 0x1 | 0x4, pointLightValues);

	free(pointLightValues[0]);
	free(pointLightValues[1]);
	free(pointLightValues);

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

	i32 fpsText = canvas_text_create(canvas);

	XMapWindow(display, canvas_get_window(canvas));
	XSync(display, False);

	b32 isRunning = B32_TRUE;

#ifdef SCENE_0
	real32 lightIntensity = 1.f;
#endif

	struct timespec prevTime;
	clock_gettime(CLOCK_MONOTONIC, &prevTime);

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
					KeySym sym = XkbKeycodeToKeysym(display, evt.xkey.keycode, 0, 0);

					if(sym == XK_w)
					{
						real32 pixelSize = scene_get_pixel_size(scene);
						pixelSize += PIXEL_SIZE_SHIFT_WEIGHT;

						scene_set_pixel_size(scene, pixelSize);
					}

#ifdef SCENE_0 
					else if(sym == XK_a)
					{
						lightIntensity -= 0.1f;

						if(lightIntensity < 0.f)
						{
							lightIntensity = 0.f;
						}

						light_set_value(scene, pointLight, 0x4, &lightIntensity);
					}
					else if(sym == XK_d)
					{
						lightIntensity += 0.1f;

						if(lightIntensity > 1.f)
						{
							lightIntensity = 1.f;
						}
						
						light_set_value(scene, pointLight, 0x4, &lightIntensity);
					}
					else if(sym == XK_s)
					{
						real32 pixelSize = scene_get_pixel_size(scene);
						pixelSize -= PIXEL_SIZE_SHIFT_WEIGHT;

						if(pixelSize < 1.f)
						{
							pixelSize = 1.f;
						}

						scene_set_pixel_size(scene, pixelSize);
					}
					else if(sym == XK_w)
					{
						real32 pixelSize = scene_get_pixel_size(scene);
						pixelSize += PIXEL_SIZE_SHIFT_WEIGHT;

						scene_set_pixel_size(scene, pixelSize);
					}
					else if(sym == XK_Print)
					{
						renderer_save_next_frame(renderer, "screenshot");
					}
#endif

#ifdef SCENE_1
					else if(sym == XK_s)
					{
						real32 pixelSize = scene_get_pixel_size(scene);
						pixelSize -= PIXEL_SIZE_SHIFT_WEIGHT;

						if(pixelSize < 1.f)
						{
							pixelSize = 1.f;
						}

						scene_set_pixel_size(scene, pixelSize);
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
		
		struct timespec currentTime;
		clock_gettime(CLOCK_MONOTONIC, &currentTime);

		u64 elapsedNanoSeconds = (1000000000*currentTime.tv_sec + currentTime.tv_nsec) - (1000000000*prevTime.tv_sec + prevTime.tv_nsec);

		char fpsBuffer[50];
		sprintf(fpsBuffer, "FPS: %ld", lround(1000.f/(elapsedNanoSeconds/1000000)));
		
		prevTime = currentTime;

		canvas_text_set(canvas, fpsText, 50, 50, fpsBuffer);

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
