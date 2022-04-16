#version 330

// adapted from https://www.shadertoy.com/view/MlVSD3
// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float time;

// Output color
layout(location = 0) out vec4 color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(8, 50))) * 50000) * 2.0 - 1.0;
}

float offset(float blocks, vec2 uv) {
	return 0.4 * rand(vec2(time * 200, floor(uv.y * blocks)));
}

void main()
{
	vec2 uv = vec2(texcoord.x, texcoord.y);
	color = vec4(fcolor, 1.0) * texture(sampler0, uv);
    
	color.r = (vec4(fcolor, 1.0) * texture(sampler0, uv + vec2(offset(2.0, uv) * 0.02, 0.0))).r;	
	color.g = (vec4(fcolor, 1.0) * texture(sampler0, uv + vec2(offset(1.0, uv) * 0.01, 0.0))).g;
	color.b = (vec4(fcolor, 1.0) * texture(sampler0, uv + vec2(offset(1.0, uv) * 0.03, 0.0))).b;
}
