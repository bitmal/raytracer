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

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

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

	v4 spherePosition = {{-0.25f, -0.25f, 2.f, 0.f}};
	i32 sphereId = scene_create_sphere(scene, &spherePosition, 0.25f, 0xFF0000);
	renderer_push_sphere(renderer, sphereId);
	v4 spherePosition1 = {{-0.125f, -0.25f, 1.f, 0.f}};
	i32 sphereId1 = scene_create_sphere(scene, &spherePosition1, 0.075f, 0xFF);
	renderer_push_sphere(renderer, sphereId1);

	XMapWindow(display, canvas_get_window(canvas));
	XSync(display, False);

	b32 isRunning = B32_TRUE;

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
