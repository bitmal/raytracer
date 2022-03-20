#include "scene.h"
#include "canvas.h"
#include "rt_math.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct camera_viewport
{
	real32 left;
	real32 right;
	real32 top;
	real32 bottom;
	real32 front;
	real32 back;
	real32 fov;
} camera_viewport;

typedef struct scene_camera
{
	camera_viewport viewport;
	v4 position;
	v4 direction;
} scene_camera;

typedef struct scene_sphere
{
	v4 position;
	real32 radius;
	color32 color;
	real32 albedo;
} scene_sphere;

typedef struct scene_object
{
	scene_object_t type;
	v4 position;
	color32 color;
	real32 albedo;
	real32 sphereRadius;
	real32 boxWidth;
	real32 boxHeight;
	real32 boxDepth;
} scene_object;

typedef struct scene_light
{
	scene_light_t type;
	v4 position;
	v4 direction;
	color32 color;
	real32 intensity;
	real32 range;
} scene_light;

struct raytracer_scene
{
	scene_camera camera;
	scene_light *lights;
	i32 lightCount;
	scene_object *objects;
	i32 objectCount;
	real32 pixelSize;
};

raytracer_scene *
scene_init()
{
	raytracer_scene *scene = malloc(sizeof(raytracer_scene));
	scene->camera.viewport.left = -1.f;
	scene->camera.viewport.right = 1.f;
	scene->camera.viewport.top = 1.f;
	scene->camera.viewport.bottom = -1.f;
	scene->camera.viewport.front = 1.f;
	scene->camera.viewport.back = scene->camera.viewport.front + 100.f;
	scene->camera.viewport.fov = 90.f;
	scene->camera.position = vec4_init(0.f, 0.f, 0.f, 0.f);
	scene->camera.direction = vec4_init(0.f, 0.f, 1.f, 0.f);

	scene->lights = NULL;
	scene->lightCount = 0;
	scene->pixelSize = 1.f;
	scene->objects = NULL;
	scene->objectCount = 0;

	return scene;
}

void
scene_set_camera_viewport(raytracer_scene *scene, real32 left, real32 right, real32 top,
		real32 bottom, real32 front, real32 distance, real32 fov)
{
	scene->camera.viewport.left = left;
	scene->camera.viewport.right = right;
	scene->camera.viewport.top = top;
	scene->camera.viewport.bottom = bottom;
	scene->camera.viewport.front = front;
	scene->camera.viewport.back = scene->camera.viewport.front + distance;
	scene->camera.viewport.fov = fov;
}

void
scene_set_camera_position(raytracer_scene *scene, const v4 *position)
{
	scene->camera.position = *position;
}

void
scene_set_pixel_size(raytracer_scene *scene, real32 size)
{
	scene->pixelSize = size;
}

real32
scene_get_pixel_size(raytracer_scene *scene)
{
	return scene->pixelSize;
}

void
scene_get_camera_position(raytracer_scene *scene, v4 *out)
{
	*out = scene->camera.position;
}

void
scene_canvas_to_world_coordinates(raytracer_scene *scene, raytracer_canvas *canvas, 
		i32 x, i32 y, v4 *out)
{
	i32 width = canvas_get_width(canvas);
	i32 height = canvas_get_height(canvas);

	v4 position = {{
		(real32)x*((scene->camera.viewport.right - scene->camera.viewport.left)/
			(real32)width) + (scene->camera.viewport.left),
		(real32)-y*((scene->camera.viewport.top - scene->camera.viewport.bottom)/
			(real32)height) - (scene->camera.viewport.bottom),
		scene->camera.viewport.front,
		0.f
	}};

	*out = position;
}

i32
scene_world_to_canvas_x(raytracer_scene *scene, raytracer_canvas *canvas,
		const v4 *worldCoords)
{
	// TODO: fix
	i32 width = canvas_get_width(canvas);

	return (i32)((worldCoords->x - scene->camera.viewport.left)*((real32)width/
				(scene->camera.viewport.right - scene->camera.viewport.left)));
}

i32
scene_world_to_canvas_y(raytracer_scene *scene, raytracer_canvas *canvas,
		const v4 *worldCoords)
{
	// TODO: fix
	i32 height = canvas_get_height(canvas);

	return (i32)(-(worldCoords->y + scene->camera.viewport.bottom)*((real32)height/
				(scene->camera.viewport.top - scene->camera.viewport.bottom)));
}

i32
scene_create_object(raytracer_scene *scene, scene_object_t type)
{
	i32 index = scene->objectCount;
	
	if(scene->objectCount > 0)
	{
		scene->objects = realloc(scene->objects, sizeof(scene_object)*(++scene->objectCount));
	}
	else
	{
		scene->objects = malloc(sizeof(scene_object)*(++scene->objectCount));
	}

	scene_object *object = &scene->objects[index];
	object->type = type;
	object->position = vec4_init(0, 0, 0, 0);
	object->color = 0xFFFFFF;
	object->albedo = 1.f;
	object->sphereRadius = 1.f;
	object->boxWidth = 1.f;
	object->boxHeight = 1.f;
	object->boxDepth = 1.f;

	return index;
}

void
scene_object_set_values(raytracer_scene *scene, i32 objectId, u32 valueFlags, 
		const void **values)
{
	scene_object *obj = &scene->objects[objectId];

	i32 valuesSet = 0;

	for(i32 i = 0; i < 32; ++i)
	{
		u32 flag = valueFlags & (1 << i);

		if(flag)
		{
			switch(flag)
			{
				case SCENE_OBJECT_VALUE_TYPE:
				{
					obj->type = *((scene_object_t **)values)[valuesSet++];
				} break;
				
				case SCENE_OBJECT_VALUE_POSITION:
				{
					obj->position = *((v4 **)values)[valuesSet++];
				} break;
				
				case SCENE_OBJECT_VALUE_COLOR:
				{
					obj->color = *((color32 **)values)[valuesSet++];
				} break;
				
				case SCENE_OBJECT_VALUE_ALBEDO:
				{
					obj->color = *((real32 **)values)[valuesSet++];
				} break;
				
				case SCENE_OBJECT_VALUE_SPHERE_RADIUS:
				{
					obj->sphereRadius = *((real32 **)values)[valuesSet++];
				} break;
				
				case SCENE_OBJECT_VALUE_BOX_WIDTH:
				{
					obj->boxWidth = *((real32 **)values)[valuesSet++];
				} break;
				
				case SCENE_OBJECT_VALUE_BOX_HEIGHT:
				{
					obj->boxHeight = *((real32 **)values)[valuesSet++];
				} break;
				
				case SCENE_OBJECT_VALUE_BOX_DEPTH:
				{
					obj->boxDepth = *((real32 **)values)[valuesSet++];
				} break;

				default:
				{
					printf("Cannot set unknown scene object value!\n");
				} break;
			}
		}
	}
}

void
scene_object_set_value(raytracer_scene *scene, i32 objectId, u32 valueFlag, 
		const void *value)
{
	scene_object *obj = &scene->objects[objectId];

	switch(valueFlag)
	{
		case SCENE_OBJECT_VALUE_TYPE:
		{
			obj->type = *(scene_object_t *)value;
		} break;
		
		case SCENE_OBJECT_VALUE_POSITION:
		{
			obj->position = *(v4 *)value;
		} break;
		
		case SCENE_OBJECT_VALUE_COLOR:
		{
			obj->color = *(color32 *)value;
		} break;
		
		case SCENE_OBJECT_VALUE_ALBEDO:
		{
			obj->albedo = *(real32 *)value;
		} break;
		
		case SCENE_OBJECT_VALUE_SPHERE_RADIUS:
		{
			obj->sphereRadius = *(real32 *)value;
		} break;
		
		case SCENE_OBJECT_VALUE_BOX_WIDTH:
		{
			obj->boxWidth = *(real32 *)value;
		} break;
		
		case SCENE_OBJECT_VALUE_BOX_HEIGHT:
		{
			obj->boxHeight = *(real32 *)value;
		} break;
		
		case SCENE_OBJECT_VALUE_BOX_DEPTH:
		{
			obj->boxDepth = *(real32 *)value;
		} break;

		default:
		{
			printf("Cannot set unknown scene object value!\n");
		} break;
	}
}

void
scene_object_get_value(raytracer_scene *scene, i32 objectId, u32 valueFlag, 
		void *outValue)
{
	switch(valueFlag)
	{
		case SCENE_OBJECT_VALUE_TYPE:
		{
			*(scene_object_t *)outValue = scene->objects[objectId].type;
		} break;
		
		case SCENE_OBJECT_VALUE_POSITION:
		{
			*(v4 *)outValue = scene->objects[objectId].position;
		} break;
		
		case SCENE_OBJECT_VALUE_COLOR:
		{
			*(color32 *)outValue = scene->objects[objectId].color;
		} break;
		
		case SCENE_OBJECT_VALUE_ALBEDO:
		{
			*(real32 *)outValue = scene->objects[objectId].albedo;
		} break;
		
		case SCENE_OBJECT_VALUE_SPHERE_RADIUS:
		{
			*(real32 *)outValue = scene->objects[objectId].sphereRadius;
		} break;
		
		case SCENE_OBJECT_VALUE_BOX_WIDTH:
		{
			*(real32 *)outValue = scene->objects[objectId].boxWidth;
		} break;
		
		case SCENE_OBJECT_VALUE_BOX_HEIGHT:
		{
			*(real32 *)outValue = scene->objects[objectId].boxHeight;
		} break;
		
		case SCENE_OBJECT_VALUE_BOX_DEPTH:
		{
			*(real32 *)outValue = scene->objects[objectId].boxDepth;
		} break;

		default:
		{
			fprintf(stderr, "Cannot get unknown value from scene object!\n");
		} break;
	}
}

i32
scene_create_light(raytracer_scene *scene, scene_light_t type)
{
	i32 index = scene->lightCount;

	if(scene->lightCount > 0)
	{
		scene->lights = realloc(scene->lights, sizeof(scene_light)*(++scene->lightCount));
	}
	else
	{
		scene->lights = malloc(sizeof(scene_light)*(++scene->lightCount));
	}

	scene_light *light = &scene->lights[index];
	light->type = type;
	light->position = vec4_init(0, 0, 0, 0);
	light->direction = vec4_init(0, -1.f, 0, 0);
	light->intensity = 1.f;
	light->range = 1.f;
	light->color = 0xFFFFFF;

	return index;
}

void
light_set_values(raytracer_scene *scene, i32 lightId, u32 valueFlags, 
		const void **values)
{
	scene_light *light = &scene->lights[lightId];

	i32 valuesSet = 0;

	for(i32 i = 0; i < 32; ++i)
	{
		u32 flag = valueFlags & (1 << i);

		if(flag)
		{
			switch(flag)
			{
				case LIGHT_VALUE_TYPE:
				{
					light->type = *((real32 **)values)[valuesSet++];
				} break;
				
				case LIGHT_VALUE_POSITION:
				{
					light->position = *((v4 **)values)[valuesSet++];
				} break;
				
				case LIGHT_VALUE_DIRECTION:
				{
					light->direction = *((v4 **)values)[valuesSet++];
					vec4_normal(&light->direction, &light->direction);
				} break;
				
				case LIGHT_VALUE_COLOR:
				{
					light->color = *((color32 **)values)[valuesSet++];
				} break;
				
				case LIGHT_VALUE_INTENSITY:
				{
					light->intensity = *((real32 **)values)[valuesSet++];
				} break;
				
				case LIGHT_VALUE_RANGE:
				{
					light->range = *((real32 **)values)[valuesSet++];
				} break;

				default:
				{
					printf("Cannot set unknown light value!\n");
				} break;
			}
		}
	}
}

void
light_set_value(raytracer_scene *scene, i32 lightId, u32 valueFlag, const void *value)
{
	switch(valueFlag)
	{
		case LIGHT_VALUE_TYPE:
		{
			scene->lights[lightId].type = *(scene_light_t *)value;
		} break;
		
		case LIGHT_VALUE_POSITION:
		{
			scene->lights[lightId].position = *(v4 *)value;
		} break;
		
		case LIGHT_VALUE_DIRECTION:
		{
			scene->lights[lightId].direction = *(v4 *)value;
			vec4_normal(&scene->lights[lightId].direction, &scene->lights[lightId].direction);
		} break;
		
		case LIGHT_VALUE_COLOR:
		{
			scene->lights[lightId].color = *(color32 *)value;
		} break;
		
		case LIGHT_VALUE_INTENSITY:
		{
			scene->lights[lightId].intensity = *(real32 *)value;
		} break;
		
		case LIGHT_VALUE_RANGE:
		{
			scene->lights[lightId].range = *(real32 *)value;
		} break;

		default:
		{
			fprintf(stderr, "Cannot set unknown value of light!\n");
		} break;
	}
}

void
light_get_value(raytracer_scene *scene, i32 lightId, u32 valueFlag, void *outValue)
{
	switch(valueFlag)
	{
		case LIGHT_VALUE_TYPE:
		{
			*(scene_light_t *)outValue = scene->lights[lightId].type;
		} break;
		
		case LIGHT_VALUE_POSITION:
		{
			*(v4 *)outValue = scene->lights[lightId].position;
		} break;
		
		case LIGHT_VALUE_DIRECTION:
		{
			*(v4 *)outValue = scene->lights[lightId].direction;
		} break;
		
		case LIGHT_VALUE_COLOR:
		{
			*(color32 *)outValue = scene->lights[lightId].color;
		} break;
		
		case LIGHT_VALUE_INTENSITY:
		{
			*(real32 *)outValue = scene->lights[lightId].intensity;
		} break;
		
		case LIGHT_VALUE_RANGE:
		{
			*(real32 *)outValue = scene->lights[lightId].range;
		} break;

		default:
		{
			fprintf(stderr, "Cannot get unknown value from light!\n");
		} break;
	}
}

static i32
_scene_get_ray_sphere_intersection(raytracer_scene *scene, scene_object *object, 
		const v4 *viewportPosition, const v4 *origin, real32 *out0, real32 *out1)
{
	v4 objPosition;
	vec4_add3(&object->position, &scene->camera.position, &objPosition);

	v4 CO;
	vec4_subtract3(origin, &objPosition, &CO);

	real32 a = vec4_dot3(viewportPosition, viewportPosition);
	real32 b = 2.f*vec4_dot3(&CO, viewportPosition);
	real32 c = vec4_dot3(&CO, &CO) - object->sphereRadius*object->sphereRadius;

	real32 discriminant = b*b - 4*a*c;

	if(discriminant < 0)
	{
		return 0;
	}

	real32 d = sqrtf(discriminant);
	*out0 = (-b + d) / (2.f*a);
	*out1 = (-b - d) / (2.f*a);

	return 2;
}

static b32
_scene_get_ray_box_intersection(scene_object *object, 
		const v4 *viewportPosition, const v4 *origin, v4 *outNormal, real32 *outDistance)
{
	v4 rayDirection;
	vec4_direction(origin, viewportPosition, &rayDirection);

	real32 halfWidth = object->boxWidth/2.f;
	real32 halfHeight = object->boxHeight/2.f;
	real32 halfDepth = object->boxDepth/2.f;

	real32 xBoundsMin = object->position.x - halfWidth;
	real32 xBoundsMax = object->position.x + halfWidth;
	real32 yBoundsMin = object->position.y - halfHeight;
	real32 yBoundsMax = object->position.y + halfHeight;
	real32 zBoundsMin = object->position.z - halfDepth;
	real32 zBoundsMax = object->position.z + halfDepth;

	v4 point;

	real32 t0 = (xBoundsMin - origin->x)/rayDirection.x;
	real32 t1 = (xBoundsMax + origin->x)/rayDirection.x;

	if(t0 > t1)
	{
		real32 swap = t0;
		t0 = t1;
		t1 = swap;
	}

	vec4_scalar3(&rayDirection, t0, &point);
	vec4_add3(origin, &point, &point);

	if(point.x < object->position.x)
	{
		*outNormal = vec4_init(-1.f, 0.f, 0.f, 0.f);
	}
	else if(point.x > object->position.x)
	{
		*outNormal = vec4_init(1.f, 0.f, 0.f, 0.f);
	}

	real32 tY0 = (yBoundsMin - origin->y)/rayDirection.y;
	real32 tY1 = (yBoundsMax + origin->y)/rayDirection.y;

	if(tY0 > tY1)
	{
		real32 swap = tY0;
		tY0 = tY1;
		tY1 = swap;
	}

	if((t0 > tY1) || (tY0 > t1))
	{
		return B32_FALSE;
	}

	if(tY0 > t0)
	{
		t0 = tY0;

		vec4_scalar3(&rayDirection, t0, &point);
		vec4_add3(origin, &point, &point);

		if(point.y < object->position.y)
		{
			*outNormal = vec4_init(0.f, -1.f, 0.f, 0.f);
		}
		else if(point.y > object->position.y)
		{
			*outNormal = vec4_init(0.f, 1.f, 0.f, 0.f);
		}
	}
	
	if(tY1 < t1)
	{
		t1 = tY1;
		
		vec4_scalar3(&rayDirection, t1, &point);
		vec4_add3(origin, &point, &point);

		if(point.y < object->position.y)
		{
			*outNormal = vec4_init(0.f, -1.f, 0.f, 0.f);
		}
		else if(point.y > object->position.y)
		{
			*outNormal = vec4_init(0.f, 1.f, 0.f, 0.f);
		}
	}
	
	real32 tZ0 = (zBoundsMin - origin->z)/rayDirection.z;
	real32 tZ1 = (zBoundsMax + origin->z)/rayDirection.z;
	
	if(tZ0 > tZ1)
	{
		real32 swap = tZ0;
		tZ0 = tZ1;
		tZ1 = swap;
	}

	if((t0 > tZ1) || (tZ0 > t1))
	{
		return B32_FALSE;
	}

	if(tZ0 > t0)
	{
		t0 = tZ0;

		vec4_scalar3(&rayDirection, t0, &point);
		vec4_add3(origin, &point, &point);
	
		if(point.z < object->position.z)
		{
			*outNormal = vec4_init(0.f, 0.f, -1.f, 0.f);
		}
		else if(point.z > object->position.z)
		{
			*outNormal = vec4_init(0.f, 0.f, 1.f, 0.f);
		}
	}
	
	if(tZ1 < t1)
	{
		t1 = tZ1;

		vec4_scalar3(&rayDirection, t1, &point);
		vec4_add3(origin, &point, &point);
	
		if(point.z < object->position.z)
		{
			*outNormal = vec4_init(0.f, 0.f, -1.f, 0.f);
		}
		else if(point.z > object->position.z)
		{
			*outNormal = vec4_init(0.f, 0.f, 1.f, 0.f);
		}
	}

	*outDistance = vec4_distance3(origin, &point);

	return B32_TRUE;
}

b32
scene_trace_ray(raytracer_scene *scene, const v4 *viewportPosition, color32 *outColor)
{
	v4 origin = vec4_init(0.f, 0.f, 0.f, 0.f);
	v4 rayDirection;
	vec4_direction(&origin, viewportPosition, &rayDirection);

	scene_object *obj = NULL;
	real32 distance;
	v4 intersectionPoint;
	v4 surfaceNormal;
	v4 objPosition;

	for(i32 i = 0; i < scene->objectCount; ++i)
	{
		scene_object *o = &scene->objects[i];

		switch(o->type)
		{
			case SCENE_OBJECT_SPHERE:
			{
				real32 d[2];
				i32 intersectionCount = _scene_get_ray_sphere_intersection(scene, o, viewportPosition, 
						&origin, &d[0], &d[1]);

				if(intersectionCount > 0)
				{
					for(i32 j = 0; j < intersectionCount; ++j)
					{
						if(obj)
						{
							if(d[j] < distance)
							{
								obj = o;
								distance = d[j];

								vec4_scalar(&rayDirection, distance, &intersectionPoint);
								vec4_add3(&origin, &intersectionPoint, &intersectionPoint);
			
								vec4_add3(&obj->position, &scene->camera.position, &objPosition);
								vec4_direction(&objPosition, &intersectionPoint, &surfaceNormal);
							}
						}
						else
						{
							obj = o;
							distance = d[j];

							vec4_scalar(&rayDirection, distance, &intersectionPoint);
							vec4_add3(&origin, &intersectionPoint, &intersectionPoint);

							vec4_add3(&obj->position, &scene->camera.position, &objPosition);
							vec4_direction(&objPosition, &intersectionPoint, &surfaceNormal);
						}
					}
				}
			} break;

			case SCENE_OBJECT_BOX:
			{
				real32 d;
				v4 n;

				if(_scene_get_ray_box_intersection(o, viewportPosition, &origin, 
							&n, &d))
				{
					if(obj)
					{
						if(d < distance)
						{
							obj = o;
							distance = d;
					
							vec4_scalar(&rayDirection, distance, &intersectionPoint);
							vec4_add3(&origin, &intersectionPoint, &intersectionPoint);

							vec4_add3(&obj->position, &origin, &objPosition);

							surfaceNormal = n;
						}
					}
					else
					{
						obj = o;
						distance = d;
					
						vec4_scalar(&rayDirection, distance, &intersectionPoint);
						vec4_add3(&origin, &intersectionPoint, &intersectionPoint);

						vec4_add3(&obj->position, &origin, &objPosition);
						
						surfaceNormal = n;
					}
				}
			} break;

			default:
			{
				fprintf(stderr, "Unknown object type. Cannot trace ray!\n");
			} break;
		}
	}

	if(obj)
	{
		v4 specularColor = {};
		v4 colorIntensity = {};

		for(i32 i = 0; i < scene->lightCount; ++i)
		{
			scene_light *light = &scene->lights[i];

			switch(light->type)
			{
				case LIGHT_AMBIENT:
				{
					colorIntensity.r += light->intensity;
					colorIntensity.g += light->intensity;
					colorIntensity.b += light->intensity;
				} break;
				
				case LIGHT_DIRECTIONAL:
				{
					scene_light *light = &scene->lights[i];

					b32 isOccluded = B32_FALSE;
					for(i32 j = 0; j < scene->objectCount; ++j)
					{
						scene_object *o = &scene->objects[j];

						if(o == obj)
						{
							continue;
						}

						v4 invLightDirection;
						vec4_scalar3(&light->direction, -1.f, &invLightDirection);

						v4 lightPosition;
						vec4_add3(&intersectionPoint, &invLightDirection, &lightPosition);

						real32 d[2];
						i32 intersectionCount = _scene_get_ray_sphere_intersection(scene, o, &lightPosition, 
								&intersectionPoint, &d[0], &d[1]);

						if(intersectionCount > 0)
						{
							for(i32 k = 0; k < intersectionCount; ++k)
							{
								if(d[k] >= 0)
								{
									isOccluded = B32_TRUE;

									break;
								}
							}

							if(isOccluded)
							{
								break;
							}
						}
					}

					if(isOccluded)
					{
						continue;
					}

					real32 dot = vec4_dot3(&surfaceNormal, &light->direction);

					if(dot < 0.f)
					{
						real32 nLength = vec4_magnitude3(&surfaceNormal);
						real32 lLength = vec4_magnitude3(&light->direction);
						real32 coeff = -dot/(nLength*lLength);
						
						colorIntensity.r += coeff*((real32)((light->color >> 16) & 0xFF)/(real32)0xFF);
						colorIntensity.g += coeff*((real32)((light->color >> 8) & 0xFF)/(real32)0xFF);
						colorIntensity.b += coeff*((real32)((light->color) & 0xFF)/(real32)0xFF);
					}

					v4 vertexToEye;
					vec4_direction(&intersectionPoint, &origin, &vertexToEye);
					vec4_normal(&vertexToEye, &vertexToEye);

					v4 lightReflect;
					vec4_scalar3(&surfaceNormal, 2.f*vec4_dot3(&light->direction, &surfaceNormal), 
							&lightReflect);
					vec4_subtract3(&light->direction, &lightReflect, &lightReflect);

					real32 specularFactor = vec4_dot3(&vertexToEye, &lightReflect);

					if(specularFactor > 0.f)
					{
						specularFactor = pow(specularFactor, obj->albedo);

						specularColor.r += specularFactor*((real32)((light->color >> 16) & 0xFF)/(real32)0xFF);
						specularColor.g += specularFactor*((real32)((light->color >> 8) & 0xFF)/(real32)0xFF);
						specularColor.b += specularFactor*((real32)((light->color) & 0xFF)/(real32)0xFF);
					}
				} break;
				
				case LIGHT_POINT:
				{
					scene_light *light = &scene->lights[i];

					v4 lightPosition;
					vec4_add3(&light->position, &scene->camera.position, &lightPosition);
					
					b32 isOccluded = B32_FALSE;
					for(i32 j = 0; j < scene->objectCount; ++j)
					{
						scene_object *o = &scene->objects[j];

						if(o == obj)
						{
							continue;
						}

						v4 oPos;
						vec4_add3(&o->position, &scene->camera.position, &oPos);

						real32 d[2];
						i32 intersectionCount = _scene_get_ray_sphere_intersection(scene, o, &lightPosition, 
								&intersectionPoint, &d[0], &d[1]);

						if(intersectionCount > 0)
						{
							for(i32 k = 0; k < intersectionCount; ++k)
							{
								if(d[k] >= 0)
								{
									isOccluded = B32_TRUE;

									break;
								}
							}

							if(isOccluded)
							{
								break;
							}
						}
					}

					if(isOccluded)
					{
						continue;
					}

					v4 lightDirection;
					vec4_direction(&lightPosition, &intersectionPoint, &lightDirection);
					real32 lightDistance = vec4_distance3(&lightPosition, &intersectionPoint);
					real32 distanceCoeff = lightDistance <= light->range ? (1.f - lightDistance/light->range) : 0.f;

					real32 dot = vec4_dot3(&surfaceNormal, &lightDirection);

					if(dot < 0.f)
					{
						real32 nLength = vec4_magnitude3(&surfaceNormal);
						real32 lLength = vec4_magnitude3(&lightDirection);
						real32 coeff = -dot/(nLength*lLength)*distanceCoeff;
						
						colorIntensity.r += coeff*((real32)((light->color >> 16) & 0xFF)/(real32)0xFF);
						colorIntensity.g += coeff*((real32)((light->color >> 8) & 0xFF)/(real32)0xFF);
						colorIntensity.b += coeff*((real32)((light->color) & 0xFF)/(real32)0xFF);
					}

					v4 vertexToEye;
					vec4_direction(&intersectionPoint, &origin, &vertexToEye);
					vec4_normal(&vertexToEye, &vertexToEye);

					v4 lightReflect;
					vec4_scalar3(&surfaceNormal, 2.f*vec4_dot3(&lightDirection, &surfaceNormal), 
							&lightReflect);
					vec4_subtract3(&lightDirection, &lightReflect, &lightReflect);

					real32 specularFactor = vec4_dot3(&vertexToEye, &lightReflect)*distanceCoeff;

					if(specularFactor > 0.f)
					{
						specularFactor = pow(specularFactor, obj->albedo);

						specularColor.r += specularFactor*((real32)((light->color >> 16) & 0xFF)/(real32)0xFF);
						specularColor.g += specularFactor*((real32)((light->color >> 8) & 0xFF)/(real32)0xFF);
						specularColor.b += specularFactor*((real32)((light->color) & 0xFF)/(real32)0xFF);
					}
				} break;
			}
		}

		v4 c = {{((real32)((obj->color >> 16) & 0xFF)/(real32)0xFF)*colorIntensity.r,
			((real32)((obj->color >> 8) & 0xFF)/(real32)0xFF)*colorIntensity.g,
			((real32)((obj->color) & 0xFF)/(real32)0xFF)*colorIntensity.b,
			0.f}};

		vec4_add3(&c, &specularColor, &c);

		if(c.r > 1.f)
		{
			c.r = 1.f;
		}
		if(c.g > 1.f)
		{
			c.g = 1.f;
		}
		if(c.b > 1.f)
		{
			c.b = 1.f;
		}

		*outColor = ((u32)(c.r*0xFF) << 16) | ((u32)(c.g*0xFF) << 8) | (u32)(c.b*0xFF);

		return B32_TRUE;
	}

	return B32_FALSE;
}
