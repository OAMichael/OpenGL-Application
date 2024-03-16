#include "../GLSLversion.h"

#include "../Constants.h"

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec4 outColor;


uniform samplerCube uSamplerSkybox;
uniform sampler2D uSamplerEquirect;

uniform uint uEnvironmentType;


void main() {
    vec3 N = normalize(inPos);
    vec3 irradiance = vec3(0.0);

    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            vec3 col = vec3(0.0);
            if (uEnvironmentType == SKYBOX) {
                irradiance += texture(uSamplerSkybox, sampleVec).rgb * cos(theta) * sin(theta);
            }
            else if (uEnvironmentType == EQUIRECTANGULAR) {
                irradiance += texture(uSamplerEquirect, CubemapToEquirect(sampleVec)).rgb * cos(theta) * sin(theta);
            }
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    outColor = vec4(irradiance, 1.0);
}
