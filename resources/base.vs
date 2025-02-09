#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec2 vertexTexCoord2;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec2 fragTexCoord2;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add here your custom variables

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragTexCoord2 = vertexTexCoord2;
    fragColor = vertexColor;
    fragNormal = vertexNormal;

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}