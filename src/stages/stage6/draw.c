/*
 * This software is licensed under the terms of the MIT License.
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2019, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2019, Andrei Alexeyev <akari@taisei-project.org>.
 */

#include "taisei.h"

#include "draw.h"

#include "global.h"
#include "util/glm.h"

MODERNIZE_THIS_FILE_AND_REMOVE_ME

static Stage6DrawData *stage6_draw_data;

Stage6DrawData *stage6_get_draw_data(void) {
	return NOT_NULL(stage6_draw_data);
}

void stage6_drawsys_init(void) {
	stage6_draw_data = calloc(1, sizeof(*stage6_draw_data));
	// TODO: make this background slightly less horribly inefficient
	stage3d_init(&stage_3d_context, 128);


	for(int i = 0; i < NUM_STARS; i++) {
		float x,y,z,r;

		x = y = z = 0;
		for(int c = 0; c < 10; c++) {
			x += nfrand();
			y += nfrand();
			z += nfrand();
		} // now x,y,z are approximately gaussian
		vec3 p = {x, y, z};
		// normalize them and it’s evenly distributed on a sphere
		glm_vec3_normalize(p);
		glm_vec3_copy(p, stage6_draw_data->stars.position[i]);
	}


	FBAttachmentConfig cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.attachment = FRAMEBUFFER_ATTACH_COLOR0;
	cfg.tex_params.type = TEX_TYPE_RGBA;
	cfg.tex_params.filter.min = TEX_FILTER_LINEAR;
	cfg.tex_params.filter.mag = TEX_FILTER_LINEAR;
	cfg.tex_params.wrap.s = TEX_WRAP_MIRROR;
	cfg.tex_params.wrap.t = TEX_WRAP_MIRROR;
	stage6_draw_data->baryon.fbpair.front = stage_add_background_framebuffer("Baryon FB 1", 0.25, 0.5, 1, &cfg);
	stage6_draw_data->baryon.fbpair.back = stage_add_background_framebuffer("Baryon FB 2", 0.25, 0.5, 1, &cfg);
	stage6_draw_data->baryon.aux_fb = stage_add_background_framebuffer("Baryon FB AUX", 0.25, 0.5, 1, &cfg);
}

void stage6_drawsys_shutdown(void) {
	stage3d_shutdown(&stage_3d_context);
	free(stage6_draw_data);
	stage6_draw_data = NULL;
}

static void stage6_bg_setup_pbr_lighting(void) {
	Camera3D *cam = &stage_3d_context.cam;

	PointLight3D lights[] = {
		{ { 0, 10, 100 }, { 10000, 10000, 10000 } },
	};

	camera3d_set_point_light_uniforms(cam, ARRAY_SIZE(lights), lights);
	r_uniform_vec3("ambient_color", 1, 1, 1);
}

static uint stage6_towertop_pos(Stage3D *s3d, vec3 pos, float maxrange) {
	vec3 p = {0, 0, 0};
	return single3dpos(s3d, pos, maxrange, p);
}

static void stage6_towertop_draw(vec3 pos) {
	r_state_push();
	r_mat_mv_push();
	r_mat_mv_translate(pos[0], pos[1], pos[2]);

	r_shader("pbr_envmap");
	stage6_bg_setup_pbr_lighting();
	r_uniform_sampler("envmap", "stage6/sky");

	mat4 camera_trans, inv_camera_trans;
	glm_mat4_identity(camera_trans);
	camera3d_apply_transforms(&stage_3d_context.cam, camera_trans);
	glm_mat4_inv_fast(camera_trans, inv_camera_trans);
	r_uniform_mat4("inv_camera_transform", inv_camera_trans);

	r_uniform_float("metallic", 1);
	//r_color(RGBA(0.1, 0.1, 0.1, 1));
	r_uniform_sampler("tex", "stage6/stairs_diffuse");
	r_uniform_sampler("roughness_map", "stage6/stairs_roughness");
	r_uniform_sampler("normal_map", "stage6/stairs_normal");
	r_uniform_sampler("ambient_map", "stage6/stairs_ambient");
	r_draw_model("stage6/stairs");
	r_uniform_float("metallic", 0);

	r_uniform_sampler("tex", "stage6/tower_diffuse");
	r_uniform_sampler("roughness_map", "stage6/tower_roughness");
	r_uniform_sampler("normal_map", "stage6/tower_normal");
	r_uniform_sampler("ambient_map", "stage6/tower_ambient");
	r_draw_model("stage6/tower");

	// optimize: draw tower bottom only after falling off the tower
	r_uniform_sampler("tex", "stage6/tower_bottom_diffuse");
	r_uniform_sampler("roughness_map", "stage6/tower_bottom_roughness");
	r_uniform_sampler("normal_map", "stage6/tower_bottom_normal");
	r_uniform_sampler("ambient_map", "stage6/tower_bottom_ambient");
	r_draw_model("stage6/tower_bottom");

	r_uniform_sampler("tex", "stage6/rim_diffuse");
	r_uniform_sampler("roughness_map", "stage6/rim_roughness");
	r_uniform_sampler("normal_map", "stage6/rim_normal");
	r_uniform_sampler("ambient_map", "stage6/rim_ambient");
	//r_uniform_vec3("ambient_color", 0, 0, 0);
	r_draw_model("stage6/rim");

	r_uniform_sampler("tex", "stage6/spires_diffuse");
	r_uniform_sampler("roughness_map", "stage6/spires_roughness");
	r_uniform_sampler("normal_map", "stage6/spires_normal");
	r_uniform_vec3("ambient_color", 0, 0, 0);
	r_draw_model("stage6/spires");

	r_shader("envmap_reflect");
	r_uniform_sampler("envmap", "stage6/sky");

	r_uniform_mat4("inv_camera_transform", inv_camera_trans);


	r_draw_model("stage6/top_plate");

	r_disable(RCAP_CULL_FACE);
	//r_disable(RCAP_DEPTH_WRITE);
	r_mat_mv_translate(0, 0, 6);
	r_mat_mv_scale(0.7, 0.7, 0.7);
	//r_mat_mv_translate(stage_3d_context.cam.pos[0], stage_3d_context.cam.pos[1], stage_3d_context.cam.pos[2]);
	r_shader("calabi-yau-quintic");
	//r_mat_mv_rotate(-global.frames*0.03, 0, 0, 1);
	r_mat_mv_rotate(global.frames*0.01, 0, 1, 0);
	r_uniform_float("alpha", global.frames*0.03);
	r_draw_model("stage6/calabi-yau-quintic");
	r_mat_mv_pop();
	r_state_pop();
}

static uint stage6_skysphere_pos(Stage3D *s3d, vec3 pos, float maxrange) {
	return single3dpos(s3d, pos, maxrange, s3d->cam.pos);
}


static void stage6_skysphere_draw(vec3 pos) {
	r_state_push();

	r_disable(RCAP_DEPTH_TEST);
	r_cull(CULL_FRONT);
	r_shader("stage6_sky");
	r_uniform_sampler("skybox", "stage6/sky");

	r_mat_mv_push();
	r_mat_mv_translate_v(stage_3d_context.cam.pos);
	r_mat_mv_scale(50, 50, 50);
	r_draw_model("cube");

	r_shader("sprite_default");

	for(int i = 0; i < NUM_STARS; i++) {
		vec3 p;
		glm_vec3_copy(stage6_draw_data->stars.position[i], p);
		vec3 axis = { 1, 0, 0.2 };
		glm_vec3_rotate(p, global.frames*0.001, axis);
		if(p[2] < 0) {
			continue;
		}

		r_mat_mv_push();
		r_mat_mv_translate_v(p);
		r_mat_mv_rotate(acos(p[2]), -p[1], p[0], 0);
		r_mat_mv_scale(3e-4, 3e-4, 3e-4);
		r_draw_sprite(&(SpriteParams) {
			.sprite = "part/smoothdot",
			.color = RGBA_MUL_ALPHA(1, 1, 1, sqrt(p[2])),
		});
		r_mat_mv_pop();

	}

	r_mat_mv_pop();
	r_state_pop();
}

void stage6_draw(void) {
	Stage3DSegment segs[] = {
		/*
		 * TODO: Render the skybox last, so that most of it can be
		 * rejected by the depth test. This doesn't work right now
		 * for some reason.
		 */
		{ stage6_skysphere_draw, stage6_skysphere_pos },
		{ stage6_towertop_draw, stage6_towertop_pos },
	};

	stage3d_draw(&stage_3d_context, 100, ARRAY_SIZE(segs), segs);
}

static void draw_baryon_connector(cmplx a, cmplx b) {
	Sprite *spr = get_sprite("stage6/baryon_connector");
	r_draw_sprite(&(SpriteParams) {
		.sprite_ptr = spr,
		.pos = { creal(a + b) * 0.5, cimag(a + b) * 0.5 },
		.rotation.vector = { 0, 0, 1 },
		.rotation.angle = carg(a - b),
		.scale = { (cabs(a - b) - 70) / spr->w, 20 / spr->h },
	});
}

void baryon(Enemy *e, int t, bool render) {
	if(render) {
		// the center piece draws the nodes; applying the postprocessing effect is easier this way.
		return;
	}
}

static void draw_baryons(Enemy *bcenter, int t) {
	// r_color4(1, 1, 1, 0.8);

	r_mat_mv_push();
	r_mat_mv_translate(creal(bcenter->pos), cimag(bcenter->pos), 0);

	r_draw_sprite(&(SpriteParams) {
		.sprite = "stage6/baryon_center",
		.rotation.angle = DEG2RAD * 2 * t,
		.rotation.vector = { 0, 0, 1 },
	});

	r_draw_sprite(&(SpriteParams) {
		.sprite = "stage6/baryon",
	});

	r_mat_mv_pop();

	r_color4(1, 1, 1, 1);

	Enemy *link0 = REF(creal(bcenter->args[1]));
	Enemy *link1 = REF(cimag(bcenter->args[1]));

	if(link0 && link1) {
		draw_baryon_connector(bcenter->pos, link0->pos);
		draw_baryon_connector(bcenter->pos, link1->pos);
	}

	for(Enemy *e = global.enemies.first; e; e = e->next) {
		if(e->visual_rule == baryon) {
			r_draw_sprite(&(SpriteParams) {
				.pos = { creal(e->pos), cimag(e->pos) },
				.sprite = "stage6/baryon",
			});

			Enemy *n = REF(e->args[1]);

			if(n != NULL) {
				draw_baryon_connector(e->pos, n->pos);
			}
		}
	}
}

void baryon_center_draw(Enemy *bcenter, int t, bool render) {
	if(!render) {
		return;
	}

	if(config_get_int(CONFIG_POSTPROCESS) < 1) {
		draw_baryons(bcenter, t);
		return;
	}

	stage_draw_begin_noshake();
	r_state_push();

	r_shader("baryon_feedback");
	r_uniform_vec2("blur_resolution", 0.5*VIEWPORT_W, 0.5*VIEWPORT_H);
	r_uniform_float("hue_shift", 0);
	r_uniform_float("time", t/60.0);

	r_framebuffer(stage6_draw_data->baryon.aux_fb);
	r_blend(BLEND_NONE);
	r_uniform_vec2("blur_direction", 1, 0);
	r_uniform_float("hue_shift", 0.01);
	r_color4(0.95, 0.88, 0.9, 0.5);

	draw_framebuffer_tex(stage6_draw_data->baryon.fbpair.front, VIEWPORT_W, VIEWPORT_H);

	fbpair_swap(&stage6_draw_data->baryon.fbpair);

	r_framebuffer(stage6_draw_data->baryon.fbpair.back);
	r_uniform_vec2("blur_direction", 0, 1);
	r_color4(1, 1, 1, 1);
	draw_framebuffer_tex(stage6_draw_data->baryon.aux_fb, VIEWPORT_W, VIEWPORT_H);

	r_state_pop();
	stage_draw_end_noshake();

	r_shader("sprite_default");
	draw_baryons(bcenter, t);

	stage_draw_begin_noshake();

	r_shader_standard();
	fbpair_swap(&stage6_draw_data->baryon.fbpair);
	r_color4(0.7, 0.7, 0.7, 0.7);
	draw_framebuffer_tex(stage6_draw_data->baryon.fbpair.front, VIEWPORT_W, VIEWPORT_H);

	stage_draw_end_noshake();

	r_color4(1, 1, 1, 1);
	r_framebuffer(stage6_draw_data->baryon.fbpair.front);
	r_shader("sprite_default");
	draw_baryons(bcenter, t);

	for(Enemy *e = global.enemies.first; e; e = e->next) {
		if(e->visual_rule == baryon) {
			cmplx p = e->pos; //+10*frand()*cexp(2.0*I*M_PI*frand());

			r_draw_sprite(&(SpriteParams) {
				.sprite = "part/myon",
				.color = RGBA(1, 0.2, 1.0, 0.7),
				.pos = { creal(p), cimag(p) },
				.rotation.angle = (creal(e->args[0]) - t) / 16.0, // frand()*M_PI*2,
				.scale.both = 2,
			});
		}
	}
}

void elly_spellbg_classic(Boss *b, int t) {
	fill_viewport(0,0,0.7,"stage6/spellbg_classic");
	r_blend(BLEND_MOD);
	r_color4(1, 1, 1, 0);
	fill_viewport(0,-t*0.005,0.7,"stage6/spellbg_chalk");
	r_blend(BLEND_PREMUL_ALPHA);
	r_color4(1, 1, 1, 1);
}

void elly_spellbg_modern(Boss *b, int t) {
	fill_viewport(0,0,0.6,"stage6/spellbg_modern");
	r_blend(BLEND_MOD);
	r_color4(1, 1, 1, 0);
	fill_viewport(0,-t*0.005,0.7,"stage6/spellbg_chalk");
	r_blend(BLEND_PREMUL_ALPHA);
	r_color4(1, 1, 1, 1);
}

void elly_spellbg_modern_dark(Boss *b, int t) {
	elly_spellbg_modern(b, t);
	fade_out(0.75 * fmin(1, t / 300.0));
}

void elly_global_rule(Boss *b, int time) {
	global.boss->glowcolor = *HSL(time/120.0, 1.0, 0.25);
	global.boss->shadowcolor = *HSLA_MUL_ALPHA((time+20)/120.0, 1.0, 0.25, 0.5);
}

static bool stage6_fog(Framebuffer *fb) {
	r_shader("zbuf_fog");
	r_uniform_sampler("depth", r_framebuffer_get_attachment(fb, FRAMEBUFFER_ATTACH_DEPTH));
	r_uniform_vec4_rgba("fog_color", RGB(0.1,0.3,0.8));
	r_uniform_float("start", 0.2);
	r_uniform_float("end", 5);
	r_uniform_float("exponent", 3.0);
	r_uniform_float("sphereness", 0);
	draw_framebuffer_tex(fb, VIEWPORT_W, VIEWPORT_H);
	r_shader_standard();
	return true;
}

ShaderRule stage6_bg_effects[] = {
	stage6_fog,
	NULL
};
