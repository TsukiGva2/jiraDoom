#include "raylib2d.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

typedef uint32_t uint;

#define FOR(I, n) for(uint I=0;I<n;I++)
#define FROM(I, v, n) for(uint I=v;I<n;I++)
#define MIN(x,y) ((x)<(y)?(x):(y))

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
const uint W = SCREEN_WIDTH;
const uint H = SCREEN_HEIGHT;

#define map_w 24
#define map_h 24

#define MIC_MUTED 0
#define MIC_UNMUTED 1

#define texture_pixels 64

int world_map[map_w][map_h] =
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// World
scalar wall_distances[SCREEN_WIDTH];

// Player
const scalar bob_speed = 0.05;
const scalar player_radius = 0.4f;

scalar steps;
scalar mouse_yaw;
vec    pos;
vec    dir;
vec    plane;
scalar moving_speed = 1.0;
tex hand_overlay;

// Textures
tex texture_map;

// Floor/Ceiling
Shader floor_shader;
int floor_res_loc;
int floor_pos_loc;
int floor_dir_loc;
int floor_plane_loc;
int floor_mic_state_loc;

// Mic
const scalar mic_radius = 1.5f;
      tex    mic_ui;
      int    mic_state;
const uint   mic_ui_elem_size = 64;

// NPCs
tex npc_texture_map;
#define npc_texture_pixels 64
#define NPC_CAP 20
struct NPC
{
	vec   pos;
	scalar distance;
	scalar radius;
	int    texture_id;
	int    npc_id;
};
typedef struct NPC NPC;
int npc_count = 0;

NPC  npcs[NPC_CAP]; // actual npc data
NPC* npc_list[NPC_CAP]; // sorted npcs (starts from closest)

// npc id map
#define NPC_TAKI    1
#define NPC_XYNO    2
#define NPC_ACRYLIC 3
#define NPC_EEEZOE  4

// texture indices
#define IMG_EEEZOE  4
#define IMG_TAKI    3
#define IMG_XYNO    2
#define IMG_ACRYLIC 1

const char* npc_names[5] = {"", "TAKI", "XYNO", "ACRYLIC", "EEEZOE"};

// Lua
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

lua_State* Lua;

void spawn_npc(NPC n)
{
	if (npc_count >= NPC_CAP)
		return;

	npcs[npc_count++] = n;
}

void log_npcs()
{
	FOR (i, npc_count)
	{
		NPC* npc = npc_list[i];

		screen_log(W - 400, 50 + (40 * i), 30, "%s, %f <%d,%d>\n", npc_names[npc->npc_id],
				npc->distance, (int)npc->pos.x, (int)npc->pos.y);
	}

	screen_log(W - 400, 50 + (40 * npc_count), 30, "<%f,%f>\n", pos.x, pos.y);
}

void _swap_npcs(int i, int j)
{
	NPC*t=npc_list[i];
	npc_list[i]=npc_list[j];
	npc_list[j]=t;
}

int _partition(int left, int right)
{
	NPC* pivot = npc_list[left];

	int i = left;

	FROM (j, left + 1, right)
	{
		if (npc_list[j]->distance > pivot->distance)
		{
			i++;
			_swap_npcs(i, j);
		}
	}

	_swap_npcs(left, i);

	return i;
}

void _quick_sort_npcs(int left, int right)
{
	if (left < right)
	{
		int i = _partition(left, right);

		_quick_sort_npcs(left, i - 1);
		_quick_sort_npcs(i + 1, right);
	}
}

void sort_npcs()
{
	if (npc_count <= 0) return;

	FOR (i, npc_count)
	{
		npcs[i].distance =
			vec_dist(npcs[i].pos, pos);

		npc_list[i] = &npcs[i];
	}

	_quick_sort_npcs(0, npc_count);
}

// create a cool fog effect based on distance
inline Color fogify(scalar distance, scalar rate)
{
	int level = 255 / Clamp(distance * rate, 1, 255);

	return (Color){level, level, level, 255};
}

inline int world_at(vec spot)
{
	return world_map[(int)spot.x][(int)spot.y];
}

void check_npc_collision(vec move_end)
{
	FOR (i, npc_count)
	{
		NPC* npc = npc_list[i];

		vec dist = vec_sub(npc->pos, move_end);

		if (dist.x*dist.x + dist.y*dist.y <= npc->radius*npc->radius)
		{
			// do something
		}
	}
}

vec move_entity(vec *pos, vec delta, scalar radius)
{
	vec offset = VEC
	(
			(delta.x > 0) ? radius : -radius,
			(delta.y > 0) ? radius : -radius
	);

	vec move_end = vec_add(vec_add(*pos, delta), offset);

	if (world_at(VEC(move_end.x, pos->y)) == 0)
	{
		pos->x += delta.x;
	}

	if (world_at(VEC(pos->x, move_end.y)) == 0)
	{
		pos->y += delta.y;
	}

	return move_end;
}

void init_window()
{
	InitWindow(W, H, "Jira Doom");
	SetTargetFPS(60);
	DisableCursor();
	SetMousePosition(W / 2, H / 2);
}

void init_textures()
{
	mic_ui          = LoadTexture("assets/mic_ui.png");
	texture_map     = LoadTexture("assets/walltext.png");
	hand_overlay    = LoadTexture("assets/hand_overlay.png");
	npc_texture_map = LoadTexture("assets/mobtex.png");
}

void init_player()
{
	pos   = VEC(22.0, 12.0);
	dir   = VEC(-1, 0);
	plane = VEC(0, 0.66);
}

void init_mic()
{
	mic_state = MIC_MUTED;

	// TODO: init mic lib
}

void init_floor()
{
	floor_shader        = LoadShader(0, "floor.fs");
    floor_res_loc       = GetShaderLocation(floor_shader, "resolution");
    floor_pos_loc       = GetShaderLocation(floor_shader, "player_pos");
    floor_dir_loc       = GetShaderLocation(floor_shader, "player_dir");
    floor_plane_loc     = GetShaderLocation(floor_shader, "camera_plane");
    floor_mic_state_loc = GetShaderLocation(floor_shader, "mic_state");

	scalar resolution[2] = { W, H };
	SetShaderValue(
			floor_shader, floor_res_loc, resolution, SHADER_UNIFORM_VEC2);
}

void init_npcs()
{
	// random seed is the loading time of the game LOL
	SetRandomSeed(GetTime());

	npc_count = 0;

	NPC n = (NPC) // TAKI
	{
		VEC(15, 10),
		0,
		.5,
		IMG_TAKI,
		NPC_TAKI
	};
	spawn_npc(n);

	n = (NPC) // XYNO
	{
		VEC(16, 12),
		0,
		.5,
		IMG_XYNO,
		NPC_XYNO
	};
	spawn_npc(n);

	n = (NPC) // ACRYLIC
	{
		VEC(15, 13),
		0,
		.5,
		IMG_ACRYLIC,
		NPC_ACRYLIC
	};
	spawn_npc(n);

	n = (NPC) // EEEZOE
	{
		VEC(16, 14),
		0,
		.5,
		IMG_EEEZOE,
		NPC_EEEZOE
	};
	spawn_npc(n);
}

void init_lua()
{
	Lua = luaL_newstate();

	luaL_openlibs(Lua);

	BeginDrawing();
		
		ClearBackground(BLACK);

		screen_log(0, 0, 30, "Initializing LUA %d", 1);

		if (luaL_loadfile(Lua, "assets/scripts/base.lua") || lua_pcall(Lua, 0, 0, 0))
		{
			screen_log(0, H/2, 30, "cannot run configuration file:\n%s",
					lua_tostring(Lua, -1));

			WaitTime(2);
		}
		else
		{
			lua_getglobal(Lua, "moving_speed");

			if (lua_isnumber(Lua, -1))
				moving_speed = (scalar)lua_tonumber(Lua, -1);
		}

	EndDrawing();

	WaitTime(0.5);
}

void clean_mic()
{
	// TODO: clean up mic lib
}

void clean_floor()
{
	UnloadShader(floor_shader);
}

void clean_player()
{
	// ...
}

void clean_textures()
{
	UnloadTexture(mic_ui);
	UnloadTexture(hand_overlay);
	UnloadTexture(texture_map);
	UnloadTexture(npc_texture_map);
}

void clean_window()
{
	CloseWindow();
}

void clean_lua()
{
	lua_close(Lua);
}

void update_floor()
{
	SetShaderValue(
			floor_shader, floor_pos_loc, &pos, SHADER_UNIFORM_VEC2);
	SetShaderValue(
			floor_shader, floor_dir_loc, &dir, SHADER_UNIFORM_VEC2);
	SetShaderValue(
			floor_shader, floor_plane_loc, &plane, SHADER_UNIFORM_VEC2);
	SetShaderValue(
			floor_shader,
			floor_mic_state_loc, &mic_state, SHADER_UNIFORM_INT);
}

void update_npcs()
{
	// sort npcs by distance
	sort_npcs();

	FOR (i, npc_count)
	{
		NPC* npc = npc_list[i];

		scalar dt = GetFrameTime();

		vec delta = {(scalar)GetRandomValue(-1,1), (scalar)GetRandomValue(-1,1)};

		move_entity(&(npc->pos), vec_scale(delta, dt), npc->radius);
	}
}

void update_mouse()
{
	vec mouse_pos = GetMousePosition();

	scalar delta_x = mouse_pos.x - W / 2;
	scalar sensitivity = 0.003;

	mouse_yaw = delta_x * sensitivity;

	SetMousePosition(W / 2, H / 2);
}

void update_player()
{
	if (IsKeyPressed(KEY_R))
	{
		if (mic_state == MIC_MUTED) mic_state = MIC_UNMUTED;
		else mic_state = MIC_MUTED;
	}

	dir = vec_rot(dir, -mouse_yaw);
	plane = vec_rot(plane, -mouse_yaw);

	scalar dt = GetFrameTime();
	scalar move_speed = moving_speed * dt;

	scalar imm_bob_speed = bob_speed;

	if (IsKeyDown(KEY_LEFT_SHIFT))
	{
		move_speed *= 2;
		imm_bob_speed *= 2;
	}

	vec mov = vec_scale(dir, move_speed);
	vec strafe = vec_scale(plane, move_speed);

	vec mov_collision = vec_scale(dir, player_radius);
	vec strafe_collision = vec_scale(plane, player_radius);

	vec delta = {0.0f, 0.0f};
	int moved = 0;

	if (IsKeyDown(KEY_W))
	{
		delta = vec_add(delta, mov);
		moved = 1;
	}
	if (IsKeyDown(KEY_S))
	{
		delta = vec_sub(delta, mov);
		moved = 1;
	}
	if (IsKeyDown(KEY_D))
	{
		delta = vec_add(delta, strafe);
		moved = 1;
	}
	if (IsKeyDown(KEY_A))
	{
		delta = vec_sub(delta, strafe);
		moved = 1;
	}

	if (moved) steps += imm_bob_speed;

	vec move_end = move_entity(&pos, delta, player_radius);

	check_npc_collision(move_end);
}

void draw_ui_mic()
{
	int mic_texture_column = 0;
	if (mic_state == MIC_UNMUTED)
		mic_texture_column = mic_ui_elem_size;

	rect source = (rect){
		mic_texture_column, 0,
		mic_ui_elem_size,
		mic_ui_elem_size
	};

	rect dest = (rect){
		W / 2 - mic_ui_elem_size / 2,
		H - mic_ui_elem_size - 20,
		mic_ui_elem_size,
		mic_ui_elem_size
	};

	DrawTexturePro(
			mic_ui,
			source,
			dest,
			VEC(0,0),
			0, WHITE
	);
}

void draw_ui_hand()
{
	int bob =
		(int)(fabs(sin(steps)) * 8);

	DrawTexture(
		hand_overlay,
		// view bobbing
		W - hand_overlay.width,
		H - hand_overlay.height + bob,
		WHITE
	);
}

void draw_floor()
{
	BeginShaderMode(floor_shader);

	rect source =
	(rect){
		0.0f, 0.0f,
		texture_map.width,
		texture_map.height
	};
	
	rect dest =
	(rect){
		0.0f, 0.0f,
		W, H
	};

	DrawTexturePro(texture_map, source, dest, VEC(0,0), 0.0f, WHITE);
	EndShaderMode();
}

void draw_player()
{
	FOR (x, W)
	{
		// makes the center of screen 0
		// right side 1, and left side -1
		vec angle_vec = vec_scale(plane, 2 * x / (scalar) W - 1);
		vec ray_dir = vec_add(dir, angle_vec);

		// integer part of position
		vec map_pos = (vec){
			floorf(pos.x), floorf(pos.y)
		};

		vec delta_dist = (vec){
			ray_dir.x == 0 ? 1e30 : fabs(1 / ray_dir.x),
			ray_dir.y == 0 ? 1e30 : fabs(1 / ray_dir.y)
		};

		vec side_dist;

		scalar perp_wall_dist;

		vec step = VEC(0, 0);

		int side;
		int hit = 0;

		// calculate initial side_dist
		if (ray_dir.x < 0)
		{
			step.x = -1;
			side_dist.x = (pos.x - map_pos.x) * delta_dist.x;
		}
		else
		{
			step.x = 1;
			side_dist.x = (map_pos.x + 1.0 - pos.x) * delta_dist.x;
		}
		if (ray_dir.y < 0)
		{
			step.y = -1;
			side_dist.y = (pos.y - map_pos.y) * delta_dist.y;
		}
		else
		{
			step.y = 1;
			side_dist.y = (map_pos.y + 1.0 - pos.y) * delta_dist.y;
		}

		while (hit == 0)
		{
			if (side_dist.x < side_dist.y)
			{
				side_dist.x += delta_dist.x;
				map_pos.x += step.x;
				side = 0;
			}
			else
			{
				side_dist.y += delta_dist.y;
				map_pos.y += step.y;
				side = 1;
			}

			int texture_id = world_map[(int)map_pos.x][(int)map_pos.y];
			if (texture_id > 0)
			{
				texture_id -= 1;

				if (side == 0)
					perp_wall_dist = (side_dist.x - delta_dist.x);
				else
					perp_wall_dist = (side_dist.y - delta_dist.y);

				int line_height = (int)(H / perp_wall_dist);

				int draw_start = -line_height / 2 + H / 2;
				int draw_end = line_height / 2 + H / 2;

				scalar texture_y_adjusted = 0.0f;

				if (draw_start < 0)
				{
					texture_y_adjusted = -draw_start *
						(texture_pixels / (scalar)line_height);
					draw_start = 0;
				}

				if (draw_end >= H)
					draw_end = H;

				scalar wall_column;

				if (side == 0)
					wall_column = pos.y + perp_wall_dist * ray_dir.y;
				else
					wall_column = pos.x + perp_wall_dist * ray_dir.x;
				wall_column -= floorf(wall_column);

				int texture_offset =
					(int)(wall_column * (scalar)texture_pixels);
				
				if (side == 0 && ray_dir.x > 0)
					texture_offset = texture_pixels - texture_offset - 1;
				if (side == 1 && ray_dir.y < 0)
					texture_offset = texture_pixels - texture_offset - 1;

				int texture_column =
					texture_id * texture_pixels + texture_offset;

				rect source = (rect){
					(scalar)texture_column,
					texture_y_adjusted,
					1.0f,
					((scalar)(draw_end - draw_start) / (scalar)line_height) *
						(scalar)texture_pixels
				};

				// our scaling factor
				rect dest = (rect){
					x, draw_start,
					1,
					(draw_end - draw_start)
				};

				DrawTexturePro(
						texture_map,
						source,
						dest,
						VEC(0,0),
						0, fogify(perp_wall_dist, 1)
				);

				wall_distances[x] = perp_wall_dist;

				hit = 1;
			}
		}
	}
}


void draw_npcs()
{
	//log_npcs();

	FOR (i, npc_count)
	{
		NPC* npc = npc_list[i];

		// https://lodev.org/cgtutor/raycasting3.html
		vec sprite_pos = vec_sub(npc->pos, pos);

		scalar inv_det = 1.0f / (plane.x * dir.y - dir.x * plane.y);

		scalar transform_x =
			inv_det * (dir.y * sprite_pos.x - dir.x * sprite_pos.y);

		scalar transform_y =
			inv_det * (-plane.y * sprite_pos.x + plane.x * sprite_pos.y);

		if (transform_y <= 0)
			continue;

		int sprite_screen_x =
			(W / 2) * (1 + transform_x / transform_y);

		int sprite_height = abs((int)(H / transform_y));
		int sprite_width = MIN(2000,abs((int)(H / transform_y)));

		int draw_start_y = -sprite_height / 2 + H / 2;
		if (draw_start_y < 0) draw_start_y = 0;
		int draw_start_x = -sprite_width / 2 + sprite_screen_x;
		if (draw_start_x < 0) draw_start_x = 0;

		int draw_end_y = sprite_height / 2 + H / 2;
		if (draw_end_y >= H) draw_end_y = H - 1;
		int draw_end_x = sprite_width / 2 + sprite_screen_x;
		if (draw_end_x >= W) draw_end_x = W - 1;

		if (sprite_screen_x < -sprite_width / 2) continue;

		FROM (stripe, draw_start_x, draw_end_x)
		{
			if (transform_y >= wall_distances[stripe])
				continue;

			int tex_x =
			   	(stripe - (-sprite_width / 2 + sprite_screen_x))
					* npc_texture_pixels / sprite_width;
			
			int texture_column =
				(npc->texture_id * npc_texture_pixels) + tex_x;

			rect source = {
				texture_column,
				0.0f,
				1.0f,
				npc_texture_pixels
			};

			rect dest = {
				stripe,
				draw_start_y,
				1.0f,
				(draw_end_y - draw_start_y)
			};


			DrawTexturePro(
				npc_texture_map,
				source,
				dest,
				VEC(0,0),
				0,
				fogify(npc->distance, 0.5)
			);
		}
	}
}

void draw_ui()
{
	draw_ui_hand();
	draw_ui_mic();
}

int main()
{
	init_window();

	init_textures();

	init_player();

	init_floor();

	init_mic();

	init_npcs();

	init_lua();

	while (!WindowShouldClose())
	{
		update_mouse();

		update_npcs();

		update_player();

		update_floor();

		BeginDrawing();
			
			ClearBackground(WHITE);

			draw_floor();

			draw_player();

			draw_npcs();

			draw_ui();

		EndDrawing();
	}

	clean_lua();

	clean_mic();

	clean_floor();

	clean_player();

	clean_textures();

	clean_window();

	return 0;
}

