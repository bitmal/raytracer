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
#include "work.h"
#include "canvas.h"
#include "scene.h"
#include "renderer.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <tgmath.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define COMMAND_BAR_STR_LENGTH 200

typedef struct command_bar
{
	i32 x;
	i32 y;
	i32 width;
	i32 height;
	i32 cursorSize;
	i32 cursorLocation;
	color32 cursorColor;
	color32 backgroundColor;
	b32 isShow;
	i32 textObject;
	i32 bufferStrSize;
	char textBuffer[COMMAND_BAR_STR_LENGTH];
} command_bar;

command_bar *
command_bar_get();

b32
command_bar_execute(command_bar *bar, raytracer_renderer *renderer, raytracer_scene *scene, 
		raytracer_canvas *screenshotCanvas, raytracer_canvas *canvas);

void
command_bar_write(command_bar *bar, raytracer_canvas *canvas, const char *str);

void
command_bar_replace(command_bar *bar, raytracer_canvas *canvas, i32 start, 
		const char *str);

void
command_bar_remove_at(command_bar *bar, raytracer_canvas *canvas, i32 position);

void
command_bar_toggle(command_bar *bar, raytracer_canvas *canvas);

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
	raytracer_renderer *renderer = renderer_init();

	i32 *screenshotTextures = NULL;
	i32 screenshotTextureCount = 0;
	i32 currentScreenshot = -1;

	scene_set_pixel_size(scene, PIXEL_SIZE);

#define SCENE_0

// SCENE_0
#ifdef SCENE_0
	i32 ambientLight = scene_create_light(scene, LIGHT_AMBIENT);
	real32 ambientIntensity = 0.15f;
	light_set_value(scene, ambientLight, LIGHT_VALUE_INTENSITY, &ambientIntensity);

#if 1
	i32 directionalLight = scene_create_light(scene, LIGHT_DIRECTIONAL);
	v4 directionalDir = {{0.f, 1.f, 0.f, 0.f}};
	light_set_value(scene, directionalLight, LIGHT_VALUE_DIRECTION, &directionalDir);
	real32 directionalIntensity = 1.f;
	color32 directionalColor = ((u32)(0xFF*directionalIntensity) << 16) | 
		((u32)(0xFF*directionalIntensity) << 8) | ((u32)(0xFF*directionalIntensity));
	light_set_value(scene, directionalLight, LIGHT_VALUE_COLOR, &directionalColor);
#endif

#if 0
	i32 pointLight = scene_create_light(scene, LIGHT_POINT);
	v4 pointPosition = {{0.f, -2.f, 2.5f, 0.f}};
	light_set_value(scene, pointLight, LIGHT_VALUE_POSITION, &pointPosition);
	real32 pointIntensity = 1.f;
	color32 pointColor = ((u32)(0xFF*pointIntensity) << 16) | 
		((u32)(0x0*pointIntensity) << 8) | ((u32)(0xFF*pointIntensity));
	light_set_value(scene, pointLight, LIGHT_VALUE_COLOR, &pointColor);
	real32 pointRange = 1000.f;
	light_set_value(scene, pointLight, LIGHT_VALUE_RANGE, &pointRange);
#endif

	i32 sphere = scene_create_object(scene, SCENE_OBJECT_SPHERE);
	color32 sphereColor = 0xFFFFFF;
	scene_object_set_value(scene, sphere, SCENE_OBJECT_VALUE_COLOR, &sphereColor);
	real32 sphereAlbedo = 32.f;
	scene_object_set_value(scene, sphere, SCENE_OBJECT_VALUE_ALBEDO, &sphereAlbedo);
	v4 spherePosition = {{-0.55f, -1.f, 2.5f, 0.f}};
	scene_object_set_value(scene, sphere, SCENE_OBJECT_VALUE_POSITION, &spherePosition);
	real32 sphereRadius = 0.5f;
	scene_object_set_value(scene, sphere, SCENE_OBJECT_VALUE_SPHERE_RADIUS, &sphereRadius);

	i32 sphere1 = scene_create_object(scene, SCENE_OBJECT_BOX);
	color32 sphere1Color = 0xFF0000;
	scene_object_set_value(scene, sphere1, SCENE_OBJECT_VALUE_COLOR, &sphere1Color);
	real32 sphere1Albedo = 32.f;
	scene_object_set_value(scene, sphere1, SCENE_OBJECT_VALUE_ALBEDO, &sphere1Albedo);
	v4 sphere1Position = {{-0.55f, 0.f, 2.5f, 0.f}};
	scene_object_set_value(scene, sphere1, SCENE_OBJECT_VALUE_POSITION, &sphere1Position);
	real32 sphere1Radius = 0.5f;
	scene_object_set_value(scene, sphere1, SCENE_OBJECT_VALUE_SPHERE_RADIUS, &sphere1Radius);
#endif

	i32 fpsText = canvas_text_create(mainCanvas);
	i32 camText = canvas_text_create(mainCanvas);
	i32 screenshotText = canvas_text_create(screenshotCanvas);
	v4 camPosition = {{0, 0, 0.f, 0}};
	scene_set_camera_position(scene, &camPosition);

#define CAM_MOVEMENT 0.1f

	XMapWindow(display, canvas_get_window(mainCanvas));
	XSync(display, False);

	b32 isRunning = B32_TRUE;

	b32 isFirstKeystroke;
	{
		char keys[32];
		XQueryKeymap(display, keys);

		for(i32 i = 0; i < 8; ++i)
		{
			const u32 *pointer = &((u32 *)keys)[i];

			isFirstKeystroke = (b32)((*pointer) != 0);
		}
	}

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
					if(isFirstKeystroke)
					{
						isFirstKeystroke = B32_FALSE;
						continue;
					}

					KeySym sym = XkbKeycodeToKeysym(display, evt.xkey.keycode, 0, 0);

					if(!command_bar_get()->isShow)
					{
						if (sym == XK_Escape)
						{
							isRunning = B32_FALSE;
						}
						else if(sym == XK_Return)
						{
							command_bar *commandBar = command_bar_get();
							command_bar_toggle(commandBar, mainCanvas);
						}
						else if(sym == XK_w)
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
					}
					else
					{
						if(sym == XK_Escape)
						{
							command_bar *commandBar = command_bar_get();
							command_bar_toggle(commandBar, mainCanvas);
						}
						else if(sym == XK_BackSpace)
						{
							command_bar *commandBar = command_bar_get();
							i32 length = strlen(commandBar->textBuffer);
							command_bar_remove_at(commandBar, mainCanvas, length - 1);
						}
						else if((sym >= XK_a && sym <= XK_z) || (sym >= XK_A && sym <= XK_Z) ||
								(sym == XK_slash || sym == XK_space) || (sym == XK_equal || sym == XK_period ||
									sym == XK_comma || sym == XK_semicolon) || (sym >= XK_0 && sym <= XK_9) ||
								(sym == XK_minus))
						{
							command_bar *commandBar = command_bar_get();

							const char cmdBuffer[] = {
								(char)sym, '\0'
							};

							command_bar_write(commandBar, mainCanvas, (const char *)cmdBuffer);
						}
						else if(sym == XK_Return)
						{
							command_bar *commandBar = command_bar_get();

							command_bar_execute(commandBar, renderer, scene, screenshotCanvas, mainCanvas);
							command_bar_toggle(commandBar, mainCanvas);
						}
					}
				} break;

				case KeyRelease:
				{
					if(isFirstKeystroke)
					{
						isFirstKeystroke = B32_FALSE;
						continue;
					}

					//KeySym sym = XkbKeycodeToKeysym(display, evt.xkey.keycode, 0, 0);
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

		command_bar *commandBar = command_bar_get();
		if(commandBar->isShow)
		{
			canvas_put_square(mainCanvas, commandBar->x, commandBar->y, commandBar->width,
					commandBar->height, commandBar->backgroundColor);

			canvas_put_square(mainCanvas, commandBar->x + commandBar->cursorSize*
					commandBar->cursorLocation, commandBar->y, commandBar->cursorSize,
					commandBar->height, commandBar->cursorColor);
		}

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
#include "work.c"
#include "canvas.c"
#include "scene.c"
#include "renderer.c"

command_bar *
command_bar_get()
{
	static command_bar *bar = NULL;

	if(bar)
	{
		return bar;
	}

	bar = malloc(sizeof(command_bar));
	bar->x = 0;
	bar->y = 0;
	bar->width = 300;
	bar->height = 20;
	bar->backgroundColor = 0xCC00; // green
	bar->bufferStrSize = 1;
	bar->textBuffer[0] = '\0';
	bar->isShow = B32_FALSE;
	bar->cursorLocation = 0;
	bar->cursorSize = 20;
	bar->cursorColor = (color32)((real32)(bar->backgroundColor >> 8)*0.5f) << 8;
	bar->textObject = CANVAS_TEXT_NULL;

	return bar;
}

b32
command_bar_execute(command_bar *bar, raytracer_renderer *renderer, raytracer_scene *scene, 
		raytracer_canvas *screenshotCanvas, raytracer_canvas *canvas)
{
	char commandBuffer[50];
	commandBuffer[0] = '\0';
	char **args = NULL;
	i32 argCount = 0;

	const char *iter = bar->textBuffer;

	if(*iter == '\0')
	{
		return B32_TRUE;
	}

 	if(*iter != '/')
 	{
 		fprintf(stderr, "Commands should start with '/'.\n");
 		return B32_FALSE;
 	}

 	++iter;
 	
 	size_t span = strcspn(iter, " \n\t");

 	if(span <= 1)
 	{
 		fprintf(stderr, "No command entered after '/'.\n");

 		return B32_FALSE;
 	}

 	memcpy(commandBuffer, iter, span);

 	commandBuffer[span] = '\0';

 	iter += span;

	while(B32_TRUE)
	{
		for(; *iter;)
		{
			if(isspace(*iter))
			{
				++iter;
			}
			else
			{
				break;
			}
		}

		if(*iter == '\0')
		{
			break;
		}

		span = strcspn(iter, " \n\t");

		i32 index = argCount;

		if(argCount > 0)
		{
			args = realloc(args, sizeof(char *)*(++argCount));
		}
		else
		{
			args = malloc(sizeof(char *)*(++argCount));
		}

		args[index] = malloc(span);

		memcpy(args[index], iter, span);

		args[index][span] = '\0';

		iter += span;
	}

	if(!strcmp(commandBuffer, "test"))
	{
		printf("Executing 'test' command.\n");

		b32 isObj = B32_TRUE;
		scene_object_t objType = SCENE_OBJECT_SPHERE;
		real32 objX = 0;
		real32 objY = 0;
		real32 objZ = 0;
		real32 size = 0.5f;
		color32 color = 0xFFFFFF;

		for(i32 i = 0; i < argCount; ++i)
		{
			const char *iter = args[i];

			if(!strncmp(iter, "type", sizeof("type") - 1))
			{
				if(iter[sizeof("type") - 1] == '=')
				{
					iter += sizeof("type");

					if(!strcmp(iter, "box"))
					{
						objType = SCENE_OBJECT_BOX;
					}
					else if(!strcmp(iter, "sphere"))
					{
						objType = SCENE_OBJECT_SPHERE;
					}
					else
					{
						fprintf(stderr, "Invalid 'type' arg value for test command.\n");
					}
				}
			}
			else if(!strncmp(iter, "size", sizeof("size") - 1))
			{
				if(iter[sizeof("size") - 1] == '=')
				{
					iter += sizeof("size");

					size = atof(iter);
				}
			}
			else if(!strncmp(iter, "xpos", sizeof("xpos") - 1))
			{
				if(iter[sizeof("xpos") - 1] == '=')
				{
					iter += sizeof("xpos");

					objX = atof(iter);
				}
			}
			else if(!strncmp(iter, "ypos", sizeof("ypos") - 1))
			{
				if(iter[sizeof("ypos") - 1] == '=')
				{
					iter += sizeof("ypos");

					objY = atof(iter);
				}
			}
			else if(!strncmp(iter, "zpos", sizeof("zpos") - 1))
			{
				if(iter[sizeof("zpos") - 1] == '=')
				{
					iter += sizeof("zpos");

					objZ = atof(iter);
				}
			}
			else if(!strncmp(iter, "color", sizeof("color") - 1))
			{
				if(iter[sizeof("color") - 1] == '=')
				{
					iter += sizeof("color");

					color = strtoul(iter, NULL, 16);
				}
			}
			else
			{
				fprintf(stderr, "Invalid argument for test command: '%s'\n", iter);
			}
		}

		if(isObj)
		{
			i32 obj = scene_create_object(scene, objType);

			v4 objPos = vec4_init(objX, objY, objZ, 0);
			scene_object_set_value(scene, obj, SCENE_OBJECT_VALUE_POSITION, &objPos);

			if(objType == SCENE_OBJECT_SPHERE)
			{
				scene_object_set_value(scene, obj, SCENE_OBJECT_VALUE_SPHERE_RADIUS, &size);
			}
			else
			{
				scene_object_set_value(scene, obj, SCENE_OBJECT_VALUE_BOX_WIDTH, &size);
				scene_object_set_value(scene, obj, SCENE_OBJECT_VALUE_BOX_HEIGHT, &size);
				scene_object_set_value(scene, obj, SCENE_OBJECT_VALUE_BOX_DEPTH, &size);
			}
			
			scene_object_set_value(scene, obj, SCENE_OBJECT_VALUE_COLOR, &color);
		}
	}

	for(i32 i = 0; i < argCount; ++i)
	{
		free(args[i]);
	}
	free(args);

	return B32_TRUE;
}

void
command_bar_write(command_bar *bar, raytracer_canvas *canvas, const char *str)
{
	if(bar->textObject == CANVAS_TEXT_NULL)
	{
		bar->textObject = canvas_text_create(canvas);

		if(canvas_text_is_show(canvas, bar->textObject) != bar->isShow)
		{
			canvas_text_toggle(canvas, bar->textObject);
		}
	}

	i32 bufferLength = strlen(bar->textBuffer);
	i32 strLength = strlen(str);
	i32 length = bufferLength + strLength + 1;

	if(length <= COMMAND_BAR_STR_LENGTH)
	{
		strcat(bar->textBuffer, str);
		bar->cursorLocation = length;
		bar->bufferStrSize = length;

		canvas_text_set(canvas, bar->textObject, bar->x, bar->y + bar->height, bar->textBuffer);
	}
	else
	{
		fprintf(stderr, "Command buffer full!\n");
		return;
	}
}

void
command_bar_replace(command_bar *bar, raytracer_canvas *canvas, i32 start, 
		const char *str)
{
	if(bar->textObject == CANVAS_TEXT_NULL)
	{
		bar->textObject = canvas_text_create(canvas);

		if(canvas_text_is_show(canvas, bar->textObject) != bar->isShow)
		{
			canvas_text_toggle(canvas, bar->textObject);
		}
	}
}

void
command_bar_remove_at(command_bar *bar, raytracer_canvas *canvas, i32 position)
{
	if(bar->textObject == CANVAS_TEXT_NULL)
	{
		bar->textObject = canvas_text_create(canvas);

		if(canvas_text_is_show(canvas, bar->textObject) != bar->isShow)
		{
			canvas_text_toggle(canvas, bar->textObject);
		}
	}

	i32 bufferLength = strlen(bar->textBuffer);

	if(position < bufferLength && bufferLength > 0)
	{
		i32 shiftCount = bufferLength - (position + 1);

		for(i32 i = position; i <= (position + shiftCount); ++i)
		{
			bar->textBuffer[i] = bar->textBuffer[i + 1];
		}

		bar->cursorLocation = bufferLength - 1;

		canvas_text_set(canvas, bar->textObject, bar->x, bar->y + bar->height, bar->textBuffer);
	}
	else
	{
		fprintf(stderr, "Cannot remove character after end of string.\n");
	}
}

void
command_bar_toggle(command_bar *bar, raytracer_canvas *canvas)
{
	bar->isShow = !bar->isShow;

	if(bar->textObject == CANVAS_TEXT_NULL)
	{
		bar->textObject = canvas_text_create(canvas);

		if(canvas_text_is_show(canvas, bar->textObject) != bar->isShow)
		{
			canvas_text_toggle(canvas, bar->textObject);
		}
	}

#if 0 // Under construction
	XTextExtents(XFontStruct *font_struct, char *string, int nchars, 
			int *direction_return, int *font_ascent_return, int *font_descent_return, 
			XCharStruct *overall_return)
#endif

	bar->textBuffer[0] = '\0';
	canvas_text_set(canvas, bar->textObject, bar->x, bar->y + bar->height, bar->textBuffer);
	bar->cursorLocation = 0;
}
