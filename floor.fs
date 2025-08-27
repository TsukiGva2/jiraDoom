#version 330

in vec2 fragTexCoord;

uniform int mic_state;

uniform vec2 resolution;

uniform vec2 player_pos;
uniform vec2 player_dir;
uniform vec2 camera_plane;

uniform sampler2D texture_map;

out vec4 finalColor;

const float MIC_RADIUS = 1.5;

void main()
{
    float y_coord = fragTexCoord.y - 0.5;

    bool is_ceiling = y_coord < 0.0;

    float p = y_coord * resolution.y;

    if (is_ceiling) {
        p = -p;
    }

    if (p == 0.0) {
        discard;
    }

    vec2 ray_dir0 = player_dir - camera_plane;
    vec2 ray_dir1 = player_dir + camera_plane;

    float pos_z = 0.5 * resolution.y;
    float row_distance = pos_z / p;

    vec2 floor_step = (ray_dir1 - ray_dir0) * (row_distance / resolution.x);
    vec2 floor_pos = player_pos + ray_dir0 * row_distance;
    floor_pos += floor_step * gl_FragCoord.x;

    vec2 texture_coord = fract(floor_pos);
    vec2 sample_pos = vec2((texture_coord.x) / 5.0, texture_coord.y);
    vec4 tex_color = texture(texture_map, sample_pos);

    if (!is_ceiling && mic_state == 1) {
        float dist_sq = dot(player_pos - floor_pos, player_pos - floor_pos);
        if (dist_sq <= MIC_RADIUS * MIC_RADIUS) {
            vec3 aura_color = vec3(0.0, 1.0, 0.0);
            float aura_alpha = 0.27;
            tex_color.rgb = mix(tex_color.rgb, aura_color, aura_alpha);
        }
    }

	float fog_amount = clamp(row_distance / 6, 0.0, 1.0);
    vec3 fog_color = vec3(0.0, 0.0, 0.0);
    tex_color.rgb = mix(tex_color.rgb, fog_color, fog_amount);
    
    finalColor = tex_color;
}
