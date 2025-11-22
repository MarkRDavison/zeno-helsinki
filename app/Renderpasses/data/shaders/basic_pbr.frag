#version 450

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform LightingUBO {
    vec3 ambientColor;

    vec3 cameraPos;

    int numDirLights;
    vec3 dirLightDirections[8];
    vec3 dirLightColors[8];
} lighting;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main()
{

    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(lighting.cameraPos - fragWorldPos);

    vec4 texColor = texture(texSampler, fragTexCoord);

    vec3 ambient = lighting.ambientColor;

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < lighting.numDirLights; ++i) 
    {
        vec3 lightDir = normalize(-lighting.dirLightDirections[i]);

        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);
        diffuse += diff * lighting.dirLightColors[i];

        // Blinn-Phong specular
        vec3 halfway = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfway), 0.0), 32.0);
        specular += spec * lighting.dirLightColors[i];
    }

    vec3 finalColor = texColor.rgb * (ambient + diffuse) + specular;
    outColor = vec4(finalColor, texColor.a);
}