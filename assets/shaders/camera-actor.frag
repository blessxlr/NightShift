#version 120

uniform sampler2D cameraFrame;
uniform sampler2D sceneDepth;
uniform sampler2D actorImage;
uniform vec4 actorRect;
uniform float actorDepth;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 background = texture2D(cameraFrame, uv);
    vec2 spriteUV = (uv - actorRect.xy) / actorRect.zw;
    if (any(lessThan(spriteUV, vec2(0.0))) || any(greaterThan(spriteUV, vec2(1.0))))
    {
        gl_FragColor = background;
        return;
    }

    vec2 packedDepth = floor(texture2D(sceneDepth, uv).rg * 255.0 + 0.5);
    float visibleDepth = dot(packedDepth, vec2(256.0, 1.0)) / 16.0;
    if (visibleDepth + 0.1 < actorDepth)
    {
        gl_FragColor = background;
        return;
    }

    vec4 actor = texture2D(actorImage, spriteUV);
    vec2 centre = uv - vec2(0.5);
    float beam = 1.0 / (1.0 + 5.8 * centre.x * centre.x + 3.6 * centre.y * centre.y);
    float infrared = 0.24 + 2.45 * beam * beam / (1.0 + actorDepth * actorDepth / 2900.0);
    float luma = dot(actor.rgb, vec3(0.30, 0.56, 0.14));
    vec3 lit = vec3(0.87, 1.0, 0.92) * (0.008 + luma * infrared * 0.75);
    gl_FragColor = vec4(mix(background.rgb, clamp(lit, 0.0, 1.0), actor.a), 1.0);
}
