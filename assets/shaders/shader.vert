#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout(std140, set = 0, binding = 0) uniform buf {
    mat4 model;
} modelBuf;
layout(std140, set = 1, binding = 0) uniform camera {
    mat4 projection;
    mat4 view;
} cameraBuf;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = inColor;
	gl_Position = cameraBuf.projection * cameraBuf.view * modelBuf.model * vec4(inPos, 1.0);
}