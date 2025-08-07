#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform bool lightEnabled;
uniform bool useTexture;
uniform bool isSun;


uniform sampler2D tex;

void main() {
    vec3 norm = normalize(Normal);
    vec3 result = objectColor;

    if (lightEnabled && !isSun) { // a Napra ne hasson fény, ha BE van kapcsolva
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        vec3 ambient = 0.2 * objectColor;
        result = (ambient + diffuse) * objectColor;
    } else if (!lightEnabled && isSun) {
        // világítás kikapcsolva, a Nap kapjon fény-árnyékot
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        vec3 ambient = 0.2 * objectColor;
        result = (ambient + diffuse) * objectColor;
    }
    if (useTexture) {
        vec3 texColor = texture(tex, TexCoord).rgb;
        result *= texColor;
    }

    FragColor = vec4(result, 1.0);
}
