#version 330 core

#include "lib/render_context.glslh"
#include "interface/standard.glslh"

void main(void) {
	fragColor = 0.2*vec4(texCoord, 1, 0);
}
