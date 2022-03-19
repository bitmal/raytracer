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
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

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

	raytracer_canvas *mainCanvas = canvas_create(display, NULL, WINDOW_WIDTH, WINDOW_HEIGHT);
	raytracer_canvas *screenshotCanvas = canvas_create(display, mainCanvas, WINDOW_WIDTH, WINDOW_HEIGHT);
	raytracer_scene *scene = scene_init();
	raytracer_renderer *renderer = renderer_init(mainCanvas);

	i32 *screenshotTextures = NULL;
	i32 screenshotTextureCount = 0;
	i32 currentScreenshot = -1;

	scene_set_pixel_size(scene, PIXEL_SIZE);

#define SCENE_0

// SCENE_0
#ifdef SCENE_0
	i32 ambientLight = scene_create_light(scene, LIGHT_AMBIENT);
	real32 ambientIntensity = 0.1f;
	light_set_value(scene, ambientLight, LIGHT_VALUE_INTENSITY, &ambientIntensity);

#if 0
	i32 directionalLight = scene_create_light(scene, LIGHT_DIRECTIONAL);
	v4 directionalDir = {{0.f, 0.f, 1.f, 0.f}};
	light_set_value(scene, directionalLight, LIGHT_VALUE_DIRECTION, &directionalDir);
	real32 directionalIntensity = 0.5f;
	color32 directionalColor = ((u32)(0xFF*directionalIntensity) << 16) | 
		((u32)(0xFF*directionalIntensity) << 8) | ((u32)(0xFF*directionalIntensity));
	light_set_value(scene, directionalLight, LIGHT_VALUE_COLOR, &directionalColor);
#endif

#if 1
	i32 pointLight = scene_create_light(scene, LIGHT_POINT);
	v4 pointPosition = {{-1.f, 0.f, 1.5f, 0.f}};
	light_set_value(scene, pointLight, LIGHT_VALUE_POSITION, &pointPosition);
	real32 pointIntensity = 1.f;
	color32 pointColor = ((u32)(0xFF*pointIntensity) << 16) | 
		((u32)(0x0*pointIntensity) << 8) | ((u32)(0xFF*pointIntensity));
	light_set_value(scene, pointLight, LIGHT_VALUE_COLOR, &pointColor);
	real32 pointRange = 100.f;
	light_set_value(scene, pointLight, LIGHT_VALUE_RANGE, &pointRange);
#endif

	i32 sphere = scene_create_object(scene, SCENE_OBJECT_SPHERE);
	color32 sphereColor = 0xFFFFFF;
	scene_object_set_value(scene, sphere, SCENE_OBJECT_VALUE_COLOR, &sphereColor);
	v4 spherePosition = {{0.f, 0.f, 2.5f, 0.f}};
	scene_object_set_value(scene, sphere, SCENE_OBJECT_VALUE_POSITION, &spherePosition);
	real32 sphereAlbedo = 32.f;
	scene_object_set_value(scene, sphere, SCENE_OBJECT_VALUE_ALBEDO, &sphereAlbedo);
#endif

	i32 fpsText = canvas_text_create(mainCanvas);
	i32 camText = canvas_text_create(mainCanvas);
	i32 screenshotText = canvas_text_create(screenshotCanvas);
	v4 camPosition = {{0, 0, 0, 0}};

#define LIGHT_DELTA_MAGNITUDE 0.01f
#define CAM_MOVEMENT 0.1f

	XMapWindow(display, canvas_get_window(mainCanvas));
	XSync(display, False);

	b32 isRunning = B32_TRUE;

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
					else if (sym == XK_h)
					{
						camPosition.x -= CAM_MOVEMENT;
						scene_set_camera_position(scene, &camPosition);
					}
					else if (sym == XK_l)
					{
						camPosition.x += CAM_MOVEMENT;
						scene_set_camera_position(scene, &camPosition);
					}
					else if (sym == XK_j)
					{
						camPosition.y -= CAM_MOVEMENT;
						scene_set_camera_position(scene, &camPosition);
					}
					else if (sym == XK_k)
					{
						camPosition.y += CAM_MOVEMENT;
						scene_set_camera_position(scene, &camPosition);
					}
					else if (sym == XK_u)
					{
						camPosition.z += CAM_MOVEMENT;
						scene_set_camera_position(scene, &camPosition);
					}
					else if (sym == XK_n)
					{
						camPosition.z -= CAM_MOVEMENT;
						scene_set_camera_position(scene, &camPosition);
					}
					else if(sym == XK_space)
					{
						if(!canvas_is_show(screenshotCanvas))
						{
							if(!screenshotTextures)
							{
								DIR *d = opendir("screenshots");
								if(d)
								{
									const char *filename = NULL;

									struct dirent *dir;
									while((dir = readdir(d)))
									{
										if(strcmp(dir->d_name, ".scrn") > 0)
										{
											filename = dir->d_name;
											break;
										}
									}

									if(filename)
									{
										char pathBuffer[50];
										sprintf(pathBuffer, "screenshots/%s", filename);

										screenshotTextures = malloc(sizeof(i32));
										screenshotTextures[0] = renderer_create_texture_from_file(renderer, pathBuffer);
										++screenshotTextureCount;

										currentScreenshot = 0;
									}
									else
									{
										fprintf(stderr, "No screenshots to show!\n");
									}

									closedir(d);
								}
								else
								{
									fprintf(stderr, "No screenshots to show!\n");
								}
							}

							if(currentScreenshot > -1)
							{
								i32 textureWidth = renderer_get_texture_width(renderer, screenshotTextures[currentScreenshot]);
								i32 textureHeight = renderer_get_texture_height(renderer, screenshotTextures[currentScreenshot]);

								canvas_resize(screenshotCanvas, textureWidth, textureHeight);
								canvas_show(screenshotCanvas);
							}
							else
							{
								fprintf(stderr, "No screenshots to show!\n");
							}
						}
						else
						{
							canvas_hide(screenshotCanvas);
						}
					}
					else if(sym == XK_x)
					{
						if(screenshotTextureCount > 0)
						{
							++currentScreenshot;
							if(currentScreenshot == screenshotTextureCount)
							{
								const char *filename = NULL;
								DIR *d = opendir("screenshots");
								struct dirent *dir;

								for(i32 i = 0; i < screenshotTextureCount;)
								{
									if((dir = readdir(d)))
									{
										if(strcmp(dir->d_name, ".scrn") > 0)
										{
											++i;
										}
									}
									else
									{
										break;
									}
								}

								while((dir = readdir(d)))
								{
									if(strcmp(dir->d_name, ".scrn") > 0)
									{
										filename = dir->d_name;
										break;
									}
								}

								closedir(d);

								if(filename)
								{
									char pathBuffer[50];
									sprintf(pathBuffer, "screenshots/%s", filename);

									screenshotTextures = realloc(screenshotTextures, sizeof(i32)*(++screenshotTextureCount));
									screenshotTextures[currentScreenshot] = renderer_create_texture_from_file(renderer, pathBuffer);
								}
								else
								{
									currentScreenshot = 0;
								}
							}

							i32 textureWidth = renderer_get_texture_width(renderer, screenshotTextures[currentScreenshot]);
							i32 textureHeight = renderer_get_texture_height(renderer, screenshotTextures[currentScreenshot]);

							canvas_resize(screenshotCanvas, textureWidth, textureHeight);
						}
					}

#ifdef SCENE_0 
					else if(sym == XK_a)
					{
					}
					else if(sym == XK_d)
					{
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

		renderer_draw_scene(renderer, mainCanvas, scene);
		
		struct timespec currentTime;
		clock_gettime(CLOCK_MONOTONIC, &currentTime);

		u64 elapsedNanoSeconds = (1000000000*currentTime.tv_sec + currentTime.tv_nsec) - 
			(1000000000*prevTime.tv_sec + prevTime.tv_nsec);

		char fpsBuffer[50];
		sprintf(fpsBuffer, "FPS: %ld", lround(1000.f/(elapsedNanoSeconds/1000000)));
		
		prevTime = currentTime;

		canvas_text_set(mainCanvas, fpsText, 50, 50, fpsBuffer);

		i32 canvasWidth = canvas_get_width(mainCanvas);
		//i32 canvasHeight = canvas_get_height(mainCanvas);

		char camTextBuffer[100];
		sprintf(camTextBuffer, "cam position: (%.1f, %.1f, %.1f)", camPosition.x, 
				camPosition.y, camPosition.z);

		real32 splitWidth = canvasWidth/4.f;
		//real32 splitHeight = canvasHeight/4.f;

		canvas_text_set(mainCanvas, camText, (splitWidth*3.f)-1, 50, camTextBuffer);

		canvas_flip(mainCanvas);

		if(canvas_is_show(screenshotCanvas))
		{
			if(currentScreenshot > -1)
			{
				renderer_draw_texture(renderer, screenshotCanvas, screenshotTextures[currentScreenshot]);

				char screenshotNameBuffer[100];
				sprintf(screenshotNameBuffer, "screenshot: %d", currentScreenshot);
				
				canvas_text_set(screenshotCanvas, screenshotText, 50, 50, screenshotNameBuffer);
				canvas_flip(screenshotCanvas);
			}
		}

		struct timespec delayTime = {0, 1000000*MS_DELAY};
		nanosleep(&delayTime, NULL);
	}

	return 0;
}

#include "rt_math.c"
#include "canvas.c"
#include "scene.c"
#include "renderer.c"
