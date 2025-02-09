#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec2 fragTexCoord2;
in vec4 fragColor;
in vec3 fragNormal;

uniform vec3 sunVector;


// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord2);

    vec3 normal = normalize(fragNormal);

    vec3 light = normalize(sunVector);

    float NdotL = max(dot(normal, light), 0.0);

    vec3 lightDot = vec3(1,1,1) * NdotL;

    // NOTE: Implement here your fragment shader code

    // final color is the color from the texture 
    //    times the tint color (colDiffuse)
    //    times the fragment color (interpolated vertex color)
    //finalColor = texelColor*colDiffuse*fragColor;

    vec4 tint = colDiffuse * fragColor;

    float ambient = 6.0f;
    vec3 specular = vec3(0.0);

    finalColor = (texelColor*((tint + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient/10.0)*tint;

  //  finalColor = vec4(normal,1);
}

