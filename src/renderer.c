#include "renderer.h"
#include "canvas.h"
#include "scene.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <dirent.h>

typedef struct renderer_texture
{
	i32 width;
	i32 height;
	u32 *pixels;
} renderer_texture;

typedef struct renderer_overlay
{
	real32 width;
	real32 height;
	i32 x;
	i32 y;
	i32 *textIds;
	i32 textCount;
	i32 backgroundTextureId;
} renderer_overlay;

struct raytracer_renderer
{
	renderer_texture *textures;
	i32 textureCount;
	renderer_overlay *overlays;
	i32 overlayCount;
	i32 activeOverlayId;
	color32 backgroundColor;
	b32 isSaveNextFrame;
	char saveNextFrameFileName[50];
};

raytracer_renderer *
renderer_init()
{
	raytracer_renderer *r = malloc(sizeof(raytracer_renderer));
	r->backgroundColor = 0x0;
	r->isSaveNextFrame = B32_FALSE;
	r->textures = NULL;
	r->textureCount = 0;
	r->overlays = NULL;
	r->overlayCount = 0;
	r->activeOverlayId = RENDERER_OVERLAY_NULL;

	return r;
}

void
renderer_draw_scene(raytracer_renderer *renderer, raytracer_canvas *canvas, 
		raytracer_scene *scene)
{
	i32 width = canvas_get_width(canvas);
	i32 height = canvas_get_height(canvas);

	if(renderer->activeOverlayId == RENDERER_OVERLAY_NULL)
	{
		real32 pixelSize = scene_get_pixel_size(scene);

		i32 partitionWidth = width*(pixelSize/width);
		i32 partitionHeight = height*(pixelSize/height);

		i32 xPartitionCount = width/partitionWidth;
		i32 yPartitionCount = height/partitionHeight;

		i32 xMin = 0;
		i32 yMin = 0;
		i32 xMax = xPartitionCount;
		i32 yMax = yPartitionCount;

		for(i32 _y = yMin; _y < yMax; ++_y)
		{
			i32 y = _y*partitionHeight;

			for(i32 _x = xMin; _x < xMax; ++_x)
			{
				i32 x = _x*partitionWidth;

				for(i32 pY = 0; pY < partitionHeight; ++pY)
				{
					for(i32 pX = 0; pX < partitionWidth; ++pX)
					{
						v4 cameraPosition;
						scene_get_camera_position(scene, &cameraPosition);

						v4 viewportPoint;
						scene_canvas_to_world_coordinates(scene, canvas, x+partitionWidth/2, y+partitionHeight/2, &viewportPoint);

						color32 result;

						if(scene_trace_ray(scene, &viewportPoint, &result))
						{
							canvas_put_pixel(canvas, x+pX, y+pY, result);
						}
						else
						{
							canvas_put_pixel(canvas, x+pX, y+pY, renderer->backgroundColor);
						}
					}
				}
			}
		}

		if(renderer->isSaveNextFrame)
		{
			struct stat st;
			if(stat("screenshots", &st) == -1)
			{
				mkdir("screenshots", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			}

			i32 fileIndex = 0;

			DIR *d = opendir("screenshots");
			struct dirent *dir;
			if(d)
			{
				while((dir = readdir(d)) != NULL)
				{
					if(strcmp(dir->d_name, renderer->saveNextFrameFileName) >= 0)
					{
						++fileIndex;
					}
				}

				closedir(d);
			}

			char nameBuffer[100];
			sprintf(nameBuffer, "screenshots/");

			strcat(nameBuffer, renderer->saveNextFrameFileName);
			strcat(nameBuffer, "_");
			char *c;
			for(c = nameBuffer; *c; ++c);
			sprintf(c, "%d", fileIndex);
			strcat(nameBuffer, ".scrn");

			printf("Opening file '%s' for writing image!\n", nameBuffer);

			FILE *file = fopen(nameBuffer, "wb");
			if(!file)
			{
				fprintf(stderr, "Cannot open file '%s' to write image!\n", nameBuffer);
				return;
			}

			u32 *canvasBuffer = canvas_get_buffer(canvas);

			struct
			{
				u16 imageFormatCode;
				i16 imageWidth;
				i16 imageHeight;
				i16 padding1;
			} header = {
				(u16)0xDEAD, width, height, 0
			};

			fwrite(&header, sizeof(header), 1, file);
			fwrite(canvasBuffer, sizeof(u32)*width, height, file);

			fclose(file);

			renderer->isSaveNextFrame = B32_FALSE;
		}
	}
	else
	{
		renderer_overlay *overlay = &renderer->overlays[renderer->activeOverlayId];
		renderer_texture *texture = &renderer->textures[overlay->backgroundTextureId];

		real32 pixelWidth = (real32)width/(real32)texture->width;
		real32 pixelHeight = (real32)height/(real32)texture->height;

		i32 _width = (i32)(width/pixelWidth);
		i32 _height = (i32)(height/pixelHeight);

		for(i32 y = 0; y < _height; ++y)
		{
			i32 _y = y*pixelHeight;

			for(i32 x = 0; x < _width; ++x)
			{
				i32 _x = x*pixelWidth;

				for(i32 partitionY = 0; partitionY < (i32)(pixelHeight); ++partitionY)
				{
					for(i32 partitionX = 0; partitionX < (i32)(pixelWidth); ++partitionX)
					{
						canvas_put_pixel(canvas, _x+partitionX, _y+partitionY, 
								texture->pixels[y*texture->width + x]);
					}
				}
			}
		}
	}
}

void
renderer_draw_texture(raytracer_renderer *renderer, raytracer_canvas *canvas, 
		i32 textureId)
{
	i32 canvasWidth = canvas_get_width(canvas);
	i32 canvasHeight = canvas_get_height(canvas);
	renderer_texture *texture = &renderer->textures[textureId];

	for(i32 y = 0; y < canvasHeight; ++y)
	{
		for(i32 x = 0; x < canvasWidth; ++x)
		{
			canvas_put_pixel(canvas, x, y, (color32)texture->pixels[y*texture->width + x]);
		}
	}
}

void
renderer_save_next_frame(raytracer_renderer *renderer, const char *filename)
{
	strcpy(renderer->saveNextFrameFileName, filename);
	renderer->isSaveNextFrame = B32_TRUE;
}

i32
renderer_create_texture(raytracer_renderer *renderer, i32 width, i32 height)
{
	i32 index = renderer->textureCount;

	if(renderer->textureCount > 0)
	{
		renderer->textures = realloc(renderer->textures, sizeof(renderer_texture)*
				(++renderer->textureCount));
	}
	else
	{
		renderer->textures = malloc(sizeof(renderer_texture)*(++renderer->textureCount));
	}

	renderer->textures[index].width = width; 
	renderer->textures[index].height = height; 
	renderer->textures[index].pixels = malloc(sizeof(u32)*width*height); 

	return index;
}

i32
renderer_create_texture_from_file(raytracer_renderer *renderer, const char *path)
{
	FILE *file = fopen(path, "rb");
	if(!file)
	{
		fprintf(stderr, "Failure to create texture from file '%s'. Could not be found!\n", path);
		return RENDERER_TEXTURE_NULL;
	}

	struct
	{
		u16 imageFormatCode;
		i16 imageWidth;
		i16 imageHeight;
		i16 padding1;
	} header = {};

	size_t readBytes = fread(&header, sizeof(header), 1, file);

	if(header.imageFormatCode != 0xDEAD)
	{
		fclose(file);
		fprintf(stderr, "Could not read file header of '%s' for loading a texture! Wrong format.\n", 
				path);

		return RENDERER_TEXTURE_NULL;
	}

	i32 textureSize = header.imageWidth*header.imageHeight*sizeof(u32);

	void *buffer = malloc(textureSize);
	readBytes = fread(buffer, sizeof(u32), header.imageWidth*header.imageHeight, file);

	if(readBytes < (header.imageWidth*header.imageHeight))
	{
		free(buffer);
		fclose(file);
		fprintf(stderr, "Could not read file header of '%s' for loading a texture! "
				"Data appears to be corrupted.\n", path);

		return RENDERER_TEXTURE_NULL;
	}
	
	fclose(file);
	
	i32 index = renderer->textureCount;

	if(renderer->textureCount > 0)
	{
		renderer->textures = realloc(renderer->textures, sizeof(renderer_texture)*
				(++renderer->textureCount));
	}
	else
	{
		renderer->textures = malloc(sizeof(renderer_texture)*(++renderer->textureCount));
	}

	renderer->textures[index].width = header.imageWidth;
	renderer->textures[index].height = header.imageHeight;
	renderer->textures[index].pixels = buffer;

	return index;
}

i32
renderer_get_texture_width(raytracer_renderer *renderer, i32 textureId)
{
	return renderer->textures[textureId].width;
}

i32
renderer_get_texture_height(raytracer_renderer *renderer, i32 textureId)
{
	return renderer->textures[textureId].height;
}

void
renderer_write_texture(raytracer_renderer *renderer, i32 textureId, i32 dX, i32 dY,
	i32 width, i32 height, u32 *pixels)
{
	renderer_texture *texture = &renderer->textures[textureId];

	for(i32 y = 0; y < height; ++y)
	{
		i32 _dY = dY + y;
		if(_dY >= texture->height)
		{
			break;
		}

		for(i32 x = 0; x < width; ++x)
		{
			i32 _dX = dX + x;
			if(_dX >= texture->width)
			{
				break;
			}

			texture->pixels[_dY*texture->width + x] = pixels[y*width + x];
		}
	}
}

i32
renderer_create_overlay(raytracer_renderer *renderer, i32 x, i32 y, i32 width, 
		i32 height, i32 backgroundTextureId)
{
	i32 index = renderer->overlayCount;

	if(renderer->overlayCount > 0)
	{
		renderer->overlays = realloc(renderer->overlays, sizeof(renderer_overlay)*
				(++renderer->overlayCount));
	}
	else
	{
		renderer->overlays = malloc(sizeof(renderer_overlay)*(++renderer->overlayCount));
	}

	renderer->overlays[index].width = width; 
	renderer->overlays[index].height = width; 
	renderer->overlays[index].x = x; 
	renderer->overlays[index].y = y; 
	renderer->overlays[index].textIds = NULL; 
	renderer->overlays[index].textCount = 0; 
	renderer->overlays[index].backgroundTextureId = backgroundTextureId; 

	return index;
}

void
renderer_toggle_overlay(raytracer_renderer *renderer, i32 overlayId)
{
	if(renderer->activeOverlayId != overlayId)
	{
		renderer->activeOverlayId = overlayId;
	}
	else
	{
		renderer->activeOverlayId = RENDERER_OVERLAY_NULL;
	}
}
