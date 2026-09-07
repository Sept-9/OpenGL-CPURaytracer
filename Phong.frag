#version 330 core
// Interpolated values from the vertex shaders
in vec2 UV;
in vec4 vertexPosition_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 Normal_cameraspace;
// Ouput data
out vec3 color;
struct Material{
    vec3 emissiveColor;
    vec3 diffuseColor;
    vec3 ambientColor;
    vec3 specularColor;
    float shininess;
};
struct PointLight{
    vec3 position;
    vec3 color;
};
// Values that stay constant for the whole mesh.
uniform mat4 V;
uniform mat4 M;
uniform Material meshMaterial;
#define MAX_NR_POINT_LIGHTS 3
uniform PointLight[MAX_NR_POINT_LIGHTS] lights;
uniform int nLights;
void main(){
    color = meshMaterial.emissiveColor;
    for(int i = 0; i < nLights; i++)
    {
        vec3 n = normalize(Normal_cameraspace);
        vec4 lp = V*M*vec4(lights[i].position,1);
        vec3 l =normalize(lp.xyz/lp.w -
        vertexPosition_cameraspace.xyz/vertexPosition_cameraspace.w);
        vec3 e = normalize(EyeDirection_cameraspace);
        //Diffuse
        float cosTheta = clamp( dot( n,l ), 0,1 );
        vec3 diffuse = meshMaterial.diffuseColor * lights[i].color
        * cosTheta ;
        vec3 ambient = meshMaterial.ambientColor * lights[i].color;
        //Specular
        vec3 B = normalize(l + e);
        float cosB = clamp(dot(n,B),0,1);
        cosB = clamp(pow(cosB,meshMaterial.shininess),0,1);
        cosB = cosB * cosTheta * (meshMaterial.shininess+2)/(2*radians(180.0f));
        vec3 specular = meshMaterial.specularColor* lights[i].color *cosB;
        color = color +ambient +diffuse + specular;
    }
}