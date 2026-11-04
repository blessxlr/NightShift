#version 120

uniform sampler2D originalTexture;
uniform sampler2D warmTexture;
uniform vec2 panoramaSize;
uniform float leftClosed;
uniform float rightClosed;
uniform bool lampPass;
uniform float lampBrightness;

float doorwayMask(vec2 p)
{
    float left = smoothstep(165.0, 190.0, p.x);
    float right = 1.0 - smoothstep(440.0, 465.0, p.x);
    float top = smoothstep(100.0, 125.0, p.y);
    return left * right * top;
}

float shadowMask(vec2 p, float closed)
{
    float thresholdY = 563.0 - (p.x - 222.0) * (15.0 / 194.0);
    float depth = p.y - thresholdY;
    float reach = 180.0;
    float feather = 10.0;
    float edge = reach * (1.0 - closed);

    float swept = smoothstep(edge - feather, edge + feather, depth);
    float atOpen = smoothstep(reach - feather, reach + feather, depth);
    float atClosed = smoothstep(-feather, feather, depth);
    float floorShadow = clamp((swept - atOpen) /
                              max(atClosed - atOpen, 0.0001), 0.0, 1.0);
    float frameShadow = smoothstep(0.0, 1.0, closed) *
                        smoothstep(210.0, 260.0, p.y);
    float floorRegion = smoothstep(0.0, 8.0, depth);
    return doorwayMask(p) * mix(frameShadow, floorShadow, floorRegion);
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec2 p = uv * panoramaSize;
    p *= vec2(2172.0, 724.0) / panoramaSize;
    
    if (lampPass)
{
    p.y = 724.0 - p.y;

    bool nearLeftDoor =
        p.x >= 165.0 && p.x <= 465.0;

    bool nearRightDoor =
        p.x >= 1707.0 && p.x <= 2007.0;

    if (!nearLeftDoor && !nearRightDoor)
    {
        vec4 room = texture2D(originalTexture, uv);

        gl_FragColor = vec4(
            room.rgb * lampBrightness,
            room.a
        ) * gl_Color;

        return;
    }
}

    float leftShadow = shadowMask(p, clamp(leftClosed, 0.0, 1.0));
    float rightShadow = shadowMask(vec2(2172.0 - p.x, p.y), clamp(rightClosed, 0.0, 1.0));
    float weight = clamp(leftShadow + rightShadow, 0.0, 1.0);

    vec4 original = texture2D(originalTexture, uv);
    
    if (lampPass)
{
    float corridorArea =
        doorwayMask(p)
        + doorwayMask(vec2(2172.0 - p.x, p.y));

    float corridorLight = clamp(
        corridorArea - weight,
        0.0,
        1.0
    );

    float brightness = mix(
        lampBrightness,
        1.0,
        corridorLight
    );

    gl_FragColor = vec4(
        original.rgb * brightness,
        original.a
    ) * gl_Color;

    return;
}
    
    vec4 warm = texture2D(warmTexture, uv);
    gl_FragColor = mix(original, warm, weight) * gl_Color;
}
