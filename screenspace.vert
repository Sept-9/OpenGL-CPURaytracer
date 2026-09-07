#version 420
out vec2 v2fTexCoord;
void main()
{
    v2fTexCoord = vec2(gl_VertexID & 2,(gl_VertexID << 1) & 2);
    gl_Position = vec4( 2.0 * v2fTexCoord - 1.0, 0.0, 1.0 );
}