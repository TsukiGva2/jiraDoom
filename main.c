#include "raylib2d.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

typedef uint32_t uint;

#define FOR(I, n) for(uint I=0;I<n;I++)
#define FROM(I, v, n) for(uint I=v;I<n;I++)

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
float wall_distances[SCREEN_WIDTH];

// Player
const float bob_speed = 0.05;
const float player_radius = 0.4f;

float     steps;
float     mouse_yaw;
Vector2   pos;
Vector2   dir;
Vector2   plane;
Texture2D hand_overlay;

// Textures
Texture2D texture_map;

// Floor/Ceiling
Shader floor_shader;
int floor_res_loc;
int floor_pos_loc;
int floor_dir_loc;
int floor_plane_loc;
int floor_mic_state_loc;

// Mic
const float mic_radius = 1.5f;
Texture2D   mic_ui;
      int   mic_state;
const uint  mic_ui_elem_size = 64;

// NPCs
Texture2D npc_texture_map;
#define npc_texture_pixels 64
#define NPC_CAP 20
struct NPC
{
	Vector2 pos;
	float   distance;
	int     texture_id;
	int     npc_id;
};
typedef struct NPC NPC;
int npc_count = 0;
int npc_heap_size = 0;

NPC  npcs[NPC_CAP]; // actual npc data
NPC* npc_heap[NPC_CAP];
NPC* npc_list[NPC_CAP]; // sorted npcs (starts from closest)

void spawn_npc(NPC n)
{
	if (npc_count >= NPC_CAP)
		return;

	npcs[npc_count++] = n;
}

inline void swap_heap_npc(int i, int j)
{
	NPC* t;
	t=npc_heap[i];
	npc_heap[i]=npc_heap[j];
	npc_heap[j]=t;
}

void insert_heap_npc(NPC* n)
{
#define PARENT(i) ((i - 1)/2)
	int index = npc_heap_size++;
	npc_heap[index] = n;

	while (index > 0)
	{
		if (npc_heap[PARENT(index)]->distance > npc_heap[index]->distance)
			break;

        swap_heap_npc(index, PARENT(index));

        index = PARENT(index);
    }
#undef PARENT
}

NPC* pop_heap_npc()
{
	if (npc_heap_size < 0) return NULL;
	return npc_heap[npc_heap_size--];
}

// heapsort
void sort_npcs()
{
	if (npc_count <= 0) return;

	FOR (i, npc_count)
		npcs[i].distance =
			Vector2Distance(npcs[i].pos, pos);

	npc_heap_size = 0;

	FOR (i, npc_count)
		insert_heap_npc(&npcs[i]);

	npc_heap_size--;

	NPC* n;
	int i = 0;
	while ((n = pop_heap_npc()) != NULL)
	{
		npc_list[i++] = n;
	}
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
	pos   = (Vector2){22.0, 12.0};
	dir   = (Vector2){-1, 0};
	plane = (Vector2){0, 0.66};
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

	float resolution[2] = { W, H };
	SetShaderValue(
			floor_shader, floor_res_loc, resolution, SHADER_UNIFORM_VEC2);
}

void init_npcs()
{
	npc_count = 0;
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

void update_mouse()
{
	Vector2 mouse_pos = GetMousePosition();

	float delta_x = mouse_pos.x - W / 2;
	float sensitivity = 0.003;

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

	dir = Vector2Rotate(dir, -mouse_yaw);
	plane = Vector2Rotate(plane, -mouse_yaw);

	float dt = GetFrameTime();
	float move_speed = 4.0 * dt;

	float imm_bob_speed = bob_speed;

	if (IsKeyDown(KEY_LEFT_SHIFT))
	{
		move_speed *= 2;
		imm_bob_speed *= 2;
	}

	Vector2 mov = Vector2Scale(dir, move_speed);
	Vector2 strafe = Vector2Scale(plane, move_speed);

	Vector2 mov_collision = Vector2Scale(dir, player_radius);
	Vector2 strafe_collision = Vector2Scale(plane, player_radius);

	Vector2 delta = {0.0f, 0.0f};
	int moved = 0;

	if (IsKeyDown(KEY_W))
	{
		delta.x += mov.x;
		delta.y += mov.y;
		moved = 1;
	}
	if (IsKeyDown(KEY_S))
	{
		delta.x -= mov.x;
		delta.y -= mov.y;
		moved = 1;
	}
	if (IsKeyDown(KEY_D))
	{
		delta.x += strafe.x;
		delta.y += strafe.y;
		moved = 1;
	}
	if (IsKeyDown(KEY_A))
	{
		delta.x -= strafe.x;
		delta.y -= strafe.y;
		moved = 1;
	}

	if (moved) steps += imm_bob_speed;

	float offset_x = (delta.x > 0) ? player_radius : -player_radius;
	float offset_y = (delta.y > 0) ? player_radius : -player_radius;

	if (world_map[(int)(pos.x + delta.x + offset_x)][(int)pos.y] == 0)
	{
		pos.x += delta.x;
	}

	if (world_map[(int)pos.x][(int)(pos.y + delta.y + offset_y)] == 0)
	{
		pos.y += delta.y;
	}
}

void draw_ui_mic()
{
	int mic_texture_column = 0;
	if (mic_state == MIC_UNMUTED)
		mic_texture_column = mic_ui_elem_size;

	Rectangle source = (Rectangle){
		mic_texture_column, 0,
		mic_ui_elem_size,
		mic_ui_elem_size
	};

	Rectangle dest = (Rectangle){
		W / 2 - mic_ui_elem_size / 2,
		H - mic_ui_elem_size - 20,
		mic_ui_elem_size,
		mic_ui_elem_size
	};

	DrawTexturePro(
			mic_ui,
			source,
			dest,
			(Vector2){0},
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

	Rectangle source =
	(Rectangle){
		0.0f, 0.0f,
		texture_map.width,
		texture_map.height
	};
	
	Rectangle dest =
	(Rectangle){
		0.0f, 0.0f,
		W, H
	};

	DrawTexturePro(texture_map, source, dest, (Vector2){0}, 0.0f, WHITE);
	EndShaderMode();
}

void draw_player()
{
	FOR (x, W)
	{
		// makes the center of screen 0
		// right side 1, and left side -1
		Vector2 angle_vec = Vector2Scale(plane, 2 * x / (float) W - 1);
		Vector2 ray_dir = Vector2Add(dir, angle_vec);

		// integer part of position
		Vector2 map_pos = (Vector2){
			floorf(pos.x), floorf(pos.y)
		};

		Vector2 delta_dist = (Vector2){
			ray_dir.x == 0 ? 1e30 : fabs(1 / ray_dir.x),
			ray_dir.y == 0 ? 1e30 : fabs(1 / ray_dir.y)
		};

		Vector2 side_dist;

		float perp_wall_dist;

		Vector2 step = (Vector2){0, 0};

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

				float texture_y_adjusted = 0.0f;

				if (draw_start < 0)
				{
					texture_y_adjusted = -draw_start *
						(texture_pixels / (float)line_height);
					draw_start = 0;
				}

				if (draw_end >= H)
					draw_end = H;

				float wall_column;

				if (side == 0)
					wall_column = pos.y + perp_wall_dist * ray_dir.y;
				else
					wall_column = pos.x + perp_wall_dist * ray_dir.x;
				wall_column -= floorf(wall_column);

				int texture_offset =
					(int)(wall_column * (float)texture_pixels);
				
				if (side == 0 && ray_dir.x > 0)
					texture_offset = texture_pixels - texture_offset - 1;
				if (side == 1 && ray_dir.y < 0)
					texture_offset = texture_pixels - texture_offset - 1;

				int texture_column =
					texture_id * texture_pixels + texture_offset;

				Rectangle source = (Rectangle){
					(float)texture_column,
					texture_y_adjusted,
					1.0f,
					((float)(draw_end - draw_start) / (float)line_height) *
						(float)texture_pixels
				};

				// our scaling factor
				Rectangle dest = (Rectangle){
					x, draw_start,
					1,
					(draw_end - draw_start)
				};

				DrawTexturePro(
						texture_map,
						source,
						dest,
						(Vector2){0},
						0, WHITE
				);

				wall_distances[x] = perp_wall_dist;

				hit = 1;
			}
		}
	}
}

void draw_npcs()
{
	// https://lodev.org/cgtutor/raycasting3.html
	FOR (i, npc_count)
	{
		NPC* npc = npc_list[i];

		Vector2 sprite_pos = Vector2Subtract(npc->pos, pos);

		float inv_det = 1.0f / (plane.x * dir.y - dir.x * plane.y);

		float transform_x =
			inv_det * (dir.y * sprite_pos.x - dir.x * sprite_pos.y);

		float transform_y =
			inv_det * (-plane.y * sprite_pos.x + plane.x * sprite_pos.y);

		if (transform_y <= 0)
			continue;

		int sprite_screen_x =
			(W / 2) * (1 + transform_x / transform_y);

		int sprite_height = abs((int)(H / transform_y));
		int sprite_width = abs((int)(H / transform_y));

		int draw_start_y = -sprite_height / 2 + H / 2;
		if (draw_start_y < 0) draw_start_y = 0;
		int draw_end_y = sprite_height / 2 + H / 2;
		if (draw_end_y >= H) draw_end_y = H - 1;

		int draw_start_x = -sprite_width / 2 + sprite_screen_x;
		if (draw_start_x < 0) draw_start_x = 0;
		int draw_end_x = sprite_width / 2 + sprite_screen_x;
		if (draw_end_x >= W) draw_end_x = W - 1;

		FROM (stripe, draw_start_x, draw_end_x)
		{
			if (transform_y >= wall_distances[stripe])
				continue;

			int tex_x =
			   	(stripe - (-sprite_width / 2 + sprite_screen_x))
					* npc_texture_pixels / sprite_width;
			
			int texture_column =
				(npc->texture_id * npc_texture_pixels) + tex_x;

			Rectangle source = {
				texture_column,
				0.0f,
				1.0f,
				npc_texture_pixels
			};

			Rectangle dest = {
				stripe,
				draw_start_y,
				1.0f,
				(draw_end_y - draw_start_y)
			};

			DrawTexturePro(
				npc_texture_map,
				source,
				dest,
				(Vector2){0},
				0,
				WHITE
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

	NPC n = (NPC)
	{
		(Vector2){15, 10},
		0,
		1,
		0
	};
	spawn_npc(n);

	while (!WindowShouldClose())
	{
		update_mouse();

		update_player();

		// sort npcs by distance
		sort_npcs();

		update_floor();

		BeginDrawing();
			
			ClearBackground(WHITE);

			draw_floor();

			draw_player();

			draw_npcs();

			draw_ui();

		EndDrawing();
	}

	clean_mic();

	clean_floor();

	clean_player();

	clean_textures();

	clean_window();

	return 0;
}

