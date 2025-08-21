#version 330

in vec2 fragTexCoord;

uniform vec2 resolution;
uniform vec2 playerPos;
uniform vec2 playerDir;
uniform vec2 cameraPlane;
uniform sampler2D textureMap;
uniform int micState;

out vec4 finalColor;

const float MIC_RADIUS = 1.5;

void main()
{
    float y_coord = fragTexCoord.y - 0.5;

    bool isCeiling = y_coord < 0.0;

    float p = y_coord * resolution.y;

    if (isCeiling) {
        p = -p;
    }

    if (p == 0.0) {
        discard;
    }

    vec2 rayDir0 = playerDir - cameraPlane;
    vec2 rayDir1 = playerDir + cameraPlane;

    float posZ = 0.5 * resolution.y;
    float rowDistance = posZ / p;

    vec2 floorStep = (rayDir1 - rayDir0) * (rowDistance / resolution.x);
    vec2 floorPos = playerPos + rayDir0 * rowDistance;
    floorPos += floorStep * gl_FragCoord.x;

    vec2 textureCoord = fract(floorPos);
    vec2 samplePos = vec2((textureCoord.x + 1.0) / 8.0, textureCoord.y);
    vec4 texColor = texture(textureMap, samplePos);

    if (!isCeiling && micState == 1) {
        float dist_sq = dot(playerPos - floorPos, playerPos - floorPos);
        if (dist_sq <= MIC_RADIUS * MIC_RADIUS) {
            vec3 auraColor = vec3(0.0, 1.0, 0.0);
            float auraAlpha = 0.27;
            texColor.rgb = mix(texColor.rgb, auraColor, auraAlpha);
        }
    }

	float fogAmount = clamp(rowDistance / 15.0, 0.0, 1.0);
    vec3 fogColor = vec3(0.0, 0.0, 0.0); // Fading to black
    texColor.rgb = mix(texColor.rgb, fogColor, fogAmount);
    
    finalColor = texColor;
}
