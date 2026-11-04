#version 120

uniform sampler2D texture;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 pixel = texture2D(texture, uv);

    if (uv.y > 0.5)
    {
        float brightness = dot(
            pixel.rgb,
            vec3(0.619, 0.527, 0.474)
        );

        vec3 steel = vec3(brightness)
                   * vec3(1.0, 0.99, 0.96);

        pixel.rgb = mix(pixel.rgb, steel, 0.88);

        pixel.rgb = clamp(
            pixel.rgb * 1.05 + vec3(0.025),
            0.0,
            1.0
        );
    }

    gl_FragColor = pixel * gl_Color;
}
