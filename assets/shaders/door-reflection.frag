#version 120
uniform sampler2D doorTexture;
uniform vec2 texel;

vec3 sampleDoor(vec2 uv)
{
    return texture2D(doorTexture, clamp(uv, texel * 0.5, vec2(1.0) - texel * 0.5)).rgb;
}

void main()
{
    float depth = gl_Color.r;
    float across = gl_Color.g;
    vec2 uv = gl_TexCoord[0].xy;
    uv.x += sin(uv.y * 170.0) * (0.0005 + 0.002 * depth);
    vec2 radius = texel * (2.0 + 12.0 * depth);
    vec3 color = sampleDoor(uv) * 0.24;
    color += sampleDoor(uv + vec2(radius.x, 0.0)) * 0.12;
    color += sampleDoor(uv - vec2(radius.x, 0.0)) * 0.12;
    color += sampleDoor(uv + vec2(0.0, radius.y)) * 0.12;
    color += sampleDoor(uv - vec2(0.0, radius.y)) * 0.12;
    color += sampleDoor(uv + radius) * 0.07;
    color += sampleDoor(uv - radius) * 0.07;
    color += sampleDoor(uv + vec2(radius.x, -radius.y)) * 0.07;
    color += sampleDoor(uv + vec2(-radius.x, radius.y)) * 0.07;
    float sideFade = smoothstep(0.0, 0.05, across) *
                     (1.0 - smoothstep(0.95, 1.0, across));
    float distanceFade = pow(max(0.0, 1.0 - depth), 1.35);
    float roughness = 0.85 + 0.15 * sin(uv.y * 260.0 + sin(uv.x * 110.0));
    gl_FragColor = vec4(color, 0.42 * distanceFade * sideFade * roughness);
}
