#version 120

uniform sampler2D cameraFrame;
uniform sampler2D sceneDepth;
uniform sampler2D actorImage;
uniform vec4 actorRect;
uniform float actorDepth;
uniform sampler2D bonnyImage;
uniform vec4 bonnyRect;
uniform float bonnyDepth;
uniform sampler2D chicaImage;
uniform vec4 chicaRect;
uniform float chicaDepth;

vec4 sampleActor(sampler2D image, vec4 rectangle, float depth,
                 vec2 uv, float visibleDepth, float beam)
{
    if (depth <= 0.0 || visibleDepth + 0.1 < depth)
        return vec4(0.0);

    vec2 spriteUV = (uv - rectangle.xy) / rectangle.zw;
    if (any(lessThan(spriteUV, vec2(0.0))) || any(greaterThan(spriteUV, vec2(1.0))))
        return vec4(0.0);

    vec4 actor = texture2D(image, spriteUV);
    float infrared = 0.24 + 2.45 * beam * beam / (1.0 + depth * depth / 2900.0);
    float luma = dot(actor.rgb, vec3(0.30, 0.56, 0.14));
    vec3 lit = vec3(0.87, 1.0, 0.92) * (0.008 + luma * infrared * 0.75);
    return vec4(clamp(lit, 0.0, 1.0), actor.a);
}

void farToNear(inout vec4 first, inout float firstDepth,
               inout vec4 second, inout float secondDepth)
{
    if (firstDepth < secondDepth)
    {
        vec4 actor = first;
        float depth = firstDepth;
        first = second;
        firstDepth = secondDepth;
        second = actor;
        secondDepth = depth;
    }
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 background = texture2D(cameraFrame, uv);
    vec2 packedDepth = floor(texture2D(sceneDepth, uv).rg * 255.0 + 0.5);
    float visibleDepth = dot(packedDepth, vec2(256.0, 1.0)) / 16.0;
    vec2 centre = uv - vec2(0.5);
    float beam = 1.0 / (1.0 + 5.8 * centre.x * centre.x + 3.6 * centre.y * centre.y);
    vec4 fedor = sampleActor(actorImage, actorRect, actorDepth, uv, visibleDepth, beam);
    vec4 bonny = sampleActor(bonnyImage, bonnyRect, bonnyDepth, uv, visibleDepth, beam);
    vec4 chica = sampleActor(chicaImage, chicaRect, chicaDepth, uv, visibleDepth, beam);
    float farDepth = actorDepth, middleDepth = bonnyDepth, nearDepth = chicaDepth;
    farToNear(fedor, farDepth, bonny, middleDepth);
    farToNear(bonny, middleDepth, chica, nearDepth);
    farToNear(fedor, farDepth, bonny, middleDepth);
    vec3 color = mix(background.rgb, fedor.rgb, fedor.a);
    color = mix(color, bonny.rgb, bonny.a);
    gl_FragColor = vec4(mix(color, chica.rgb, chica.a), 1.0);
}
