#version 410

in vec2 vTexCoord;
in vec3 vNormal;
in vec4 vPosition;

uniform Ia; // ambient light colour
uniform Id; // diffuse light colour
uniform Is; // specular light colour
uniform vec3 LightDirection;
uniform sampler2D diffuseTexture;
uniform vec3 cameraPosition;

out vec4 FragColour;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float specularPower;

void main()
{
    // Ensure normal/lightDir are normalized
    vec3 N = normalize(vNormal);
    vec3 L = normalize(LightDirection);

    // Calc lambert term (negative light dir)
    float lambertTerm = max(0, min(1, dot(N, -L)));

    // Calc view vector & reflection vector
    vec3 V = normalize(cameraPosition - vPosition);
    vec3 R = reflect(L, N);

    // Calc colour properties
    float ambient = Ia * Ka;
    vec3 diffuse = Id * lambertTerm;

    // output
    FragColour = vec4 (ambient + diffuse, 1);
    // FragColour = texture(diffuseTexture, vTexCoord);
}