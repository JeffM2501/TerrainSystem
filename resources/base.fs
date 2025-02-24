#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec2 fragTexCoord2;
in vec4 fragColor;
in vec3 fragNormal;

uniform int selected;
uniform vec4 selectedColor;

uniform int showSplat;

uniform vec3 sunVector;

uniform sampler2D matDiffuse0;
uniform vec4 matTint0;
uniform sampler2D matDiffuse1;
uniform vec4 matTint1;
uniform sampler2D matDiffuse2;
uniform vec4 matTint2;
uniform sampler2D matDiffuse3;
uniform vec4 matTint3;
uniform sampler2D matDiffuse4;
uniform vec4 matTint4;

uniform vec4 colDiffuse;

uniform int materialCount;

uniform sampler2D splatmap;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    if (selected == 1)
    {
        finalColor = selectedColor;
        return;
    }
    // Texel color fetching from texture sampler
    vec4 texelColor = vec4(1,0,1,1);
    vec4 splatColor = texture(splatmap, fragTexCoord);

    // TODO< check the splat map to see if we even need the base layer

    vec4 splatmapColor = vec4(1,0,1,1);

    int baseBlock = 0;
    if (splatColor.r >= 1 || splatColor.g >= 1 || splatColor.b >= 1 || splatColor.a >= 1)
    {
        baseBlock = 1;
    }

    if (materialCount >= 1 && baseBlock == 0)
    {
        splatmapColor = texture(matDiffuse0, fragTexCoord2)* matTint0;
    }

     if (materialCount >= 2 && splatColor.r > 0)
     {
        vec4 mat1Color = texture(matDiffuse1, fragTexCoord2) * matTint1;
        splatmapColor = mix(mat1Color, splatmapColor, 1-splatColor.r);
     }
    
     if (materialCount >= 3 && splatColor.g > 0)
     {
         vec4 mat2Color = texture(matDiffuse2, fragTexCoord2) * matTint2;
         splatmapColor = mix(mat2Color, splatmapColor, 1-splatColor.g);
     }
     
     if (materialCount >= 4 && splatColor.b > 0)
     {
         vec4 mat3Color = texture(matDiffuse3, fragTexCoord2) * matTint3;
         splatmapColor = mix(mat3Color, splatmapColor, 1-splatColor.b);
     }
     
    if (materialCount >= 5 && splatColor.a > 0)
    {
        vec4 mat4Color = texture(matDiffuse4, fragTexCoord2) * matTint4;
        splatmapColor = mix(mat4Color, splatmapColor, 1-splatColor.a);
    }

    if (materialCount > 0)
        texelColor = splatmapColor;

    if (showSplat == 1)
        texelColor = vec4(splatColor.r,splatColor.g,splatColor.b,1);

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

