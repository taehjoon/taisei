/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#include "taisei.h"

#include "background_anim.h"
#include "draw.h"

#include "global.h"
#include "stageutils.h"
#include "common_tasks.h"

MODERNIZE_THIS_FILE_AND_REMOVE_ME

void start_fall_over(void) { //troll
	Stage6DrawData *draw_data = stage6_get_draw_data();
}


static float ease_final(float t, float from, float to, float outfrac) {
	float slope = 2/(1 + outfrac);
	float deceleration = 1/(1-outfrac*outfrac);
	float v;
	if(t < outfrac) {
		v = slope * t;
	} else if(t < 1) {
		v = -deceleration*(1-t)*(1-t) + 1;
	} else {
		v = 1;
	}
		

	return from + (to-from)*v;
}
	

TASK(stage6_bg_update, NO_ARGS) {
	Camera3D *cam = &stage_3d_context.cam;
	float vel = 0.0026;
	float r = 8;
	float cam_rot_offset = 40;

	cam->pos[2] = -16.2;
	cam->vel[2] = 0*64*0.3/M_TAU*vel;

	int duration = 3000;
	
	INVOKE_SUBTASK_DELAYED(duration-300, common_easing_animate, &cam_rot_offset, 90, 500, glmc_ease_quad_inout);
	INVOKE_SUBTASK_DELAYED(duration-300, common_easing_animate, &r, 6, 500, glmc_ease_quad_inout);

	for(int i = 0;; i++) {
		float phi = ease_final(i/(float)duration, 70, 540, 0.7);
		cam->pos[2] = ease_final(i/(float)duration, -16.2, 8, 0.8);
		stage3d_update(&stage_3d_context);
		cam->rot.v[2] = phi + cam_rot_offset;
		cam->pos[0] = r*cos(phi*M_TAU/360);
		cam->pos[1] = r*sin(phi*M_TAU/360);

		

		YIELD;
	}
}

void stage6_bg_init_fullstage(void) {
	Camera3D *cam = &stage_3d_context.cam;
	cam->rot.v[0] = 90;
	cam->fovy = STAGE3D_DEFAULT_FOVY*1.5;
	INVOKE_TASK(stage6_bg_update);
}

void stage6_bg_init_spellpractice(void) {
	stage6_bg_init_fullstage();
}
