#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec2 fragTexCoord2;
in vec4 fragColor;
in vec3 fragNormal;

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

uniform int materialCount;

uniform sampler2D splatmap;

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
    vec4 splatColor = texture(splatmap, fragTexCoord);

    // TODO< check the splat map to see if we even need the base layer

    vec4 splatmapColor = vec4(1,0,1,1);
    if (materialCount >= 1)
    {
        splatmapColor = texture(matDiffuse0, fragTexCoord2)* matTint0;
    }

     if (materialCount >= 2)
     {
         splatmapColor = mix(texture(matDiffuse1, fragTexCoord2)* matTint1, splatmapColor,  splatColor.r);
     }
    
    if (materialCount >= 3)
    {
        splatmapColor = mix(texture(matDiffuse2, fragTexCoord2)* matTint2, splatmapColor,  splatColor.g);
    }
   //
   // if (materialCount >= 4 && splatColor.b > 0)
   // {
   //     splatmapColor = mix(texture(matDiffuse3, fragTexCoord2)* matTint3, splatmapColor,  splatColor.b);
   // }
   //
   // if (materialCount >= 5 && splatColor.a > 0)
   // {
   //     splatmapColor = mix(texture(matDiffuse4, fragTexCoord2)* matTint4, splatmapColor,  splatColor.a);
   // }

    if (materialCount > 0)
        texelColor = splatmapColor;

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

