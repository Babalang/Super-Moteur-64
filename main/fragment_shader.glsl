#version 330 core

in vec2 TexCoordout;
in float height;
in vec3 worldPos;
in vec3 Normal;
uniform int useHeightMap;
uniform int isPBR;
uniform float GrassHeight;
uniform float RockHeight;
uniform vec3 camPos;
uniform vec3 lightPositions[1];
uniform vec3 lightColors[1];
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D hudTexture;
uniform sampler2D text;
uniform bool isHUD;
uniform vec4 hudColor;
uniform bool isText;
uniform bool isFond;
uniform bool isSkybox;
// Ouput data
out vec4 color;
const float PI = 3.14159265359;
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float DistributionBeckmann(vec3 N, vec3 H, float roughness) {
    float NdotH = max(dot(N, H), 0.0);
    float m = roughness * roughness;
    float m2 = m * m;
    float NdotH2 = NdotH * NdotH;

    float num = exp((NdotH2 - 1.0) / (m2 * NdotH2));
    float denom = PI * m2 * NdotH2 * NdotH2;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}
vec3 getNormalFromNormalMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoordout).rgb;
    tangentNormal = tangentNormal * 2.0 - 1.0;

    // Construct TBN (Tangent, Bitangent, Normal) matrix
    vec3 Q1 = dFdx(worldPos);
    vec3 Q2 = dFdy(worldPos);
    vec2 st1 = dFdx(TexCoordout);
    vec2 st2 = dFdy(TexCoordout);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


void main(){
        if(isHUD){
                if(isFond){
                        color=vec4(0.0,0.0,0.0,0.5);
                }else{
                        color=texture(hudTexture,TexCoordout);
                }
        }else if(isText){
                color=texture(text,TexCoordout);
        }else if(isSkybox){
                color=texture(albedoMap,TexCoordout);
        }        
        else{
                vec4 TexCoordout1=texture(albedoMap,TexCoordout);
                vec3 albedo =  pow(texture(albedoMap, TexCoordout).rgb, vec3(2.2));
                vec3 normal = normalize(Normal);
                float metallic = 0.4;
                float roughness = 0.2;
                float ao = 1.0;
                int test = 0;
                if (isPBR == 1){
                        normal     = getNormalFromNormalMap();
                        metallic  = texture(metallicMap, TexCoordout).r;
                        roughness = texture(roughnessMap, TexCoordout).r;
                        ao        = texture(aoMap, TexCoordout).r;
                        test = 1;
                } else if(useHeightMap == 1){
                        if(height<GrassHeight){
                                albedo = texture(albedoMap, TexCoordout).rgb;
                                metallic = 0.0;
                                roughness = 0.6;
                                ao = 0.3;
                        } else if(height<RockHeight){
                                albedo = texture(normalMap, TexCoordout).rgb;
                                metallic = 0.0;
                                roughness = 0.7;
                                ao = 0.4;
                        } else{
                                albedo = texture(roughnessMap, TexCoordout).rgb;
                                metallic = 0.0;
                                roughness = 0.8;
                                ao = 0.3;
                        }
                        test = 2;
                }

                vec3 N = normalize(Normal);
                vec3 V = normalize(camPos - worldPos);

                vec3 F0 = vec3(0.04); 
                F0 = mix(F0, albedo, metallic);
                                
                // reflectance equation
                vec3 Lo = vec3(0.0);
                for(int i = 0; i < 1; ++i) 
                {
                        // calculate per-light radiance
                        vec3 L = -normalize(lightPositions[i] - worldPos);
                        //vec3 L=normalize(vec3(1,2,1));
                        vec3 H = normalize(V + L);
                        float distance    = length(lightPositions[i] - worldPos);
                        float attenuation = 1.0 / (distance * distance);
                        vec3 radiance     = lightColors[i] * attenuation;        
                        
                        // cook-torrance brdf
                        float NDF = DistributionGGX(N, H, roughness);        
                        float G   = GeometrySmith(N, V, L, roughness);      
                        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
                        
                        vec3 kS = F;
                        vec3 kD = vec3(1.0) - kS;
                        kD *= 1.0 - metallic;	  
                        
                        vec3 numerator    = NDF * G * F;
                        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
                        vec3 specular     = numerator / denominator;  
                        
                        // add to outgoing radiance Lo
                        float NdotL = max(dot(N, L), 0.0);                
                        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
                }   
                
                vec3 ambient = vec3(0.3) * albedo * ao;
                vec3 colortmp = ambient + Lo;
                        
                colortmp = colortmp / (colortmp + vec3(1.0));
                colortmp = pow(colortmp, vec3(1.0/2.2));  
                
                color = vec4(colortmp,TexCoordout1[3]);
                color=TexCoordout1;
        }
        
}