struct Light
{
  vec3 position, color;
  float constFalloff, linearFalloff, squareFalloff;
};

struct Material
{
  vec3 ambient, diffuse, specular, emission;
  float shininess;
};

uniform Light uLights[10];
uniform int uNumLights;
uniform Light uLight;
uniform Material uMaterial;
uniform vec3 uCameraPos;

varying vec3 vWorldPosition;
varying vec3 vNormal;

void main()
{
  vec3 lNormal = normalize(vNormal);
  
  vec3 finalColor = vec3(0.0, 0.0, 0.0);
  for(int i = 0; i < uNumLights; ++i)
  {
    // Diffuse light
    vec3 toLight = uLights[i].position - vWorldPosition;
    float lightDistance = length(toLight);
    toLight = normalize(toLight);
    float diffuseAmount = max(dot(lNormal, toLight), 0.0);
  
    // Specular light
    vec3 toCamera = normalize(uCameraPos - vWorldPosition);
    vec3 reflection = normalize(2.0 * dot(toLight, lNormal) * lNormal - toLight);
    float specularAmount = pow(max(dot(toCamera, reflection), 0.0), uMaterial.shininess);
  
    // Falloff
    float falloff = 1.0 / (uLights[i].constFalloff
                          + uLights[i].linearFalloff * lightDistance
                          + uLights[i].squareFalloff * lightDistance * lightDistance);
  
    finalColor += ((uMaterial.diffuse * diffuseAmount
                  + uMaterial.specular * specularAmount) * falloff
                  + uMaterial.ambient) * uLights[i].color;
  }
  
  finalColor += uMaterial.emission;

  gl_FragColor = vec4(finalColor.r, finalColor.g, finalColor.b, 1);
}