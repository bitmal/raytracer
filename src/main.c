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

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

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

	v4 dirLightDirection = {{0.f, -1.f, 0.f, 0.f}};
	i32 dirLight = scene_create_directional_light(scene, &dirLightDirection, 1.f);

	v4 spherePosition = {{0, -1, 3, 0.f}};
	i32 sphereId = scene_create_sphere(scene, &spherePosition, 1.f, 0xFF0000);
	renderer_push_sphere(renderer, sphereId);
	v4 spherePosition1 = {{2, 0, 4, 0.f}};
	i32 sphereId1 = scene_create_sphere(scene, &spherePosition1, 1.f, 0xFFFF);
	renderer_push_sphere(renderer, sphereId1);
	v4 spherePosition2 = {{-1, -1, 3, 0.f}};
	i32 sphereId2 = scene_create_sphere(scene, &spherePosition2, 1.f, 0xFF00);
	renderer_push_sphere(renderer, sphereId2);

	XMapWindow(display, canvas_get_window(canvas));
	XSync(display, False);

	b32 isRunning = B32_TRUE;
	real32 lightIntensity = 1.f;

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

		struct timespec delayTime = {0, 15000000};
		nanosleep(&delayTime, NULL);
	}

	return 0;
}

#include "rt_math.c"
#include "canvas.c"
#include "scene.c"
#include "renderer.c"
