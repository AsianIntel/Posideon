#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout(std140, set = 0, binding = 0) uniform buf {
    mat4 model;
} modelBuf;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = inColor;
	gl_Position = vec4(inPos, 1.0) + modelBuf.model[0];
	gl_Position.y = -gl_Position.y;	// Flip y-coords.
}