#version 330 core

#include "lib/render_context.glslh"
#include "interface/ssr.glslh"
#include "lib/util.glslh"

UNIFORM(4) mat4 inv_camera_transform;

void main(void) {

	vec3 reflected_ray = mat3(inv_camera_transform)*(pos - 2 * dot(normal, pos) * normal);

	float phi = atan(reflected_ray.y, reflected_ray.x);
	float theta = acos(reflected_ray.z/length(reflected_ray));

	vec2 texCoord = vec2(0.5+phi/tau, 1-theta/pi);

	fragColor = texture(tex, texCoord);
	fragColor.r = linear_to_srgb(fragColor.r);
	fragColor.g = linear_to_srgb(fragColor.g);
	fragColor.b = linear_to_srgb(fragColor.b);
}
