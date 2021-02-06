#version 330 core

#include "lib/render_context.glslh"
#include "interface/ssr.glslh"
#include "lib/util.glslh"

const int steps = 10;


// way to optimize: do projection matrix in the vertex shader
vec3 pos_to_texcoord(vec3 pos) {
	vec4 tmp = r_projectionMatrix * vec4(pos, 1);
	return 0.5*tmp.xyz/tmp.w + vec3(0.5);
}
	

// Based on https://www.shadertoy.com/view/Xl2XWz

float smoothNoise(vec2 p) {
	vec2 f = fract(p);
	p -= f;
	f *= f * (3 - f - f);

	// Intel's sin() function breaks down with values over 1e4 (yes, really)
	// get rid of some bits here w/o sacrificing the nice effect
	vec4 a = vec4(0, 1, 27, 28) + p.x + p.y * 27;
	vec4 b = mod(a, tau);

	// WARNING: Some versions of the Windows AMD driver choke on temp_mat = mat2(temp_vec)
	vec4 temp_vec = fract(sin(b) * 1e5);
	mat2 temp_mat = mat2(temp_vec.x, temp_vec.y, temp_vec.z, temp_vec.w);

	return dot(temp_mat * vec2(1 - f.y, f.y), vec2(1 - f.x, f.x));
}

float fractalNoise(vec2 p) {
	return
		smoothNoise(p)     * 0.5333 +
		smoothNoise(p * 2) * 0.2667 +
		smoothNoise(p * 4) * 0.1333 +
		smoothNoise(p * 8) * 0.0667;
}

float warpedNoise(vec2 p) {
	vec2 m = vec2(0.0, -time);
	float x = fractalNoise(p + m);
	float y = fractalNoise(p + m.yx + x);
	float z = fractalNoise(p - m - x);
	return fractalNoise(p + vec2(x, y) + vec2(y, z) + vec2(z, x) + length(vec3(x, y, z)) * 0.1);
}

void main(void) {
	vec2 uv = flip_native_to_bottomleft(texCoord - wave_offset);

	float height = 0.01*warpedNoise(uv * 4);

	vec2 duvdx = dFdx(uv);
	vec2 duvdy = dFdy(uv);
	mat2 duvdxy = mat2(duvdx,duvdy);

	float dheightdx = dFdx(height);
	float dheightdy = dFdy(height);

	vec2 dheightduv = inverse(duvdxy)*vec2(dheightdx,dheightdy);

	mat3 tbn = mat3(normalize(tangent), normalize(bitangent), normalize(normal));
	vec3 n = normalize(tbn*vec3(-dheightduv, 1));

	//vec3 n = normalize(normal
	//	+0.05*sin(100*sin(dot(texCoord,vec2(1,1))))+0.05*cos(10*texCoord.x));

	vec3 reflected_ray = pos - 2 * dot(n, pos) * n;
	reflected_ray /= length(reflected_ray.xy);
	float step_size = 15./steps;

	fragColor = r_color;
	int ihit = 0;
	
	for(int i = 1; i < steps; i++) {
		vec3 raypos = pos + i * step_size * reflected_ray;
		vec3 raycoord = pos_to_texcoord(raypos);
		float bgdepth = texture(depth, raycoord.xy).r;
		if(bgdepth < raycoord.z) {
			ihit = i;
			break; // sorry gpu
		}
	}

	if(ihit == 0) {
		return;
	}
	for(int j = 0; j < steps; j++) {
		vec3 raypos = pos + (ihit-1+j*1.0/(steps-1)) * step_size * reflected_ray;
		vec3 raycoord = pos_to_texcoord(raypos);
		float bgdepth = texture(depth, raycoord.xy).r;
		if(bgdepth < raycoord.z) {
			fragColor += 0.8 * texture(tex, raycoord.xy);
			break; // sorry gpu
		}
	}
		
	//fragColor = texture(tex, pos_to_texcoord(pos).xy);
	//fragColor = vec4(mod(pos2.x,1),pos2.x>0,0,1);

}
