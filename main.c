#include <raylib.h>
#include <raymath.h>
#include <inttypes.h>
#include <math.h>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

typedef uint32_t u32;

#define FOR(I, n) for(u32 I=0;I<n;I++)

const u32 W = 800;
const u32 H = 600;

const float player_radius = 0.2f;

#define map_w 24
#define map_h 24

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

Vector2   pos;
Vector2   dir;
Vector2 plane;

Texture2D texture_map;
Image     texture_map_pixel_data;

Image     floor_canvas;
Texture2D floor_canvas_render;

float mouse_yaw;

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
	update_mouse();

	dir = Vector2Rotate(dir, -mouse_yaw);
	plane = Vector2Rotate(plane, -mouse_yaw);

	float dt = GetFrameTime();
	float move_speed = 4.0 * dt;

	if (IsKeyDown(KEY_LEFT_SHIFT))
	{
		move_speed *= 2;
	}

	Vector2 mov = Vector2Scale(dir, move_speed);
	Vector2 strafe = Vector2Scale(plane, move_speed);

	Vector2 mov_collision = Vector2Scale(dir, player_radius);
	Vector2 strafe_collision = Vector2Scale(plane, player_radius);

	if (IsKeyDown(KEY_W))
	{
		if (world_map[(int)(pos.x + mov.x + mov_collision.x)][(int)pos.y] == 0)
			pos.x += mov.x;

		if (world_map[(int)pos.x][(int)(pos.y + mov.y + mov_collision.y)] == 0)
			pos.y += mov.y;
	}

	if (IsKeyDown(KEY_A))
	{
		if (world_map[(int)(pos.x - strafe.x - strafe_collision.x)][(int)pos.y] == 0)
			pos.x -= strafe.x;

		if (world_map[(int)pos.x][(int)(pos.y - strafe.y - strafe_collision.y)] == 0)
			pos.y -= strafe.y;
	}

	if (IsKeyDown(KEY_S))
	{
		if (world_map[(int)(pos.x - mov.x - mov_collision.x)][(int)pos.y] == 0)
			pos.x -= mov.x;

		if (world_map[(int)pos.x][(int)(pos.y - mov.y - mov_collision.y)] == 0)
			pos.y -= mov.y;
	}

	if (IsKeyDown(KEY_D))
	{
		if (world_map[(int)(pos.x + strafe.x + strafe_collision.x)][(int)pos.y] == 0)
			pos.x += strafe.x;

		if (world_map[(int)pos.x][(int)(pos.y + strafe.y + strafe_collision.y)] == 0)
			pos.y += strafe.y;
	}
}

void draw_floor()
{
	FOR(y, H)
	{
		Vector2 ray_dir0 = Vector2Subtract(dir, plane);
		Vector2 ray_dir1 = Vector2Add(dir, plane);

		int p = y - H / 2;

		float pos_z = .5f * H;
		float row_dist = pos_z / p;

		Vector2 floor_step =
			Vector2Scale(
					Vector2Scale(Vector2Subtract(ray_dir1, ray_dir0), 1.0f / W),
					row_dist
			);

		Vector2 floor = Vector2Add(
				pos,
				Vector2Scale(ray_dir0, row_dist)
		);

		FOR(x, W)
		{
			Vector2 cell = (Vector2){floorf(floor.x), floorf(floor.y)};

			int tx, ty;
			tx = (int)
				(texture_pixels * (floor.x - cell.x)) & (texture_pixels - 1);
			ty = (int)
				(texture_pixels * (floor.y - cell.y)) & (texture_pixels - 1);

			floor = Vector2Add(floor, floor_step);

			int floor_texture = 0;

			Color c = GetImageColor(texture_map_pixel_data, tx, ty);

			ImageDrawPixel(&floor_canvas, x, y, c);
			ImageDrawPixel(&floor_canvas, x, H - y - 1, c);
		}
	}

	UpdateTexture(floor_canvas_render, floor_canvas.data);
	DrawTexture(floor_canvas_render, 0, 0, WHITE);
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

				hit = 1;
			}
		}
	}
}

int main()
{
	pos   = (Vector2){22.0, 12.0};
	dir   = (Vector2){-1, 0};
	plane = (Vector2){0, 0.66};

	InitWindow(W, H, "Jira Doom");

	SetTargetFPS(60);

	DisableCursor();
	SetMousePosition(W / 2, H / 2);

	floor_canvas        = GenImageColor(W, H, WHITE);
	floor_canvas_render = LoadTextureFromImage(floor_canvas);

	texture_map            = LoadTexture("assets/walltext.png");
	texture_map_pixel_data = LoadImageFromTexture(texture_map);
	
	while (!WindowShouldClose())
	{
		update_player();

		BeginDrawing();
			
			ClearBackground(WHITE);

			draw_floor();
			draw_player();

		EndDrawing();
	}

	UnloadImage(texture_map_pixel_data);
	UnloadTexture(texture_map);

	CloseWindow();
}

