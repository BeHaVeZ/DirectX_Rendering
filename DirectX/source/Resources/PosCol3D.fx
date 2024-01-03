//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//// Shader Purpose: This shader performs lighting calculations using diffuse, normal, and specular maps.
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

///Global Variables
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : World;
float4x4 gViewInverseMatrix : ViewInverse;

/// Textures
Texture2D gDiffuseMap : DiffuseMap; // Diffuse map for surface color
Texture2D gNormalMap : NormalMap; // Normal map for surface normals
Texture2D gSpecularMap : SpecularMap; // Specular map for surface highlights
Texture2D gGlossinessMap : GlossinessMap; // Glossiness map for specular shininess

/// Mathematical Constants
float gPI = 3.14159265358979311600; //Speaks for itself I hope
float3 gLightDirection = normalize(float3(0.577f, -0.577f, 0.577f)); // Normalized light direction
float gLightIntensity = 7.0f; // Intensity of the light source
float gShininess = 25.0f; // Shininess factor for specular highlights

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// SAMPLERSTATES for texture filtering
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

SamplerState gSamPoint : SampleState
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; // or Mirror, Clamp, Border
    AddressV = Wrap; // or Mirror, Clamp, Border
};

SamplerState gSamLinear : SampleState
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap; // or Mirror, Clamp, Border
    AddressV = Wrap; // or Mirror, Clamp, Border
};

SamplerState gSamAnisotropic : SampleState
{
    Filter = ANISOTROPIC;
    AddressU = Wrap; // or Mirror, Clamp, Border
    AddressV = Wrap; // or Mirror, Clamp, Border
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Input/Output Structures
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct VS_INPUT
{
    float3 Position : POSITION; // Vertex position in object space
    float2 UV : TEXCOORD; // Texture coordinates
    float3 Normal : NORMAL; // Vertex normal
    float3 Tangent : TANGENT; // Vertex tangent
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION0; // Transformed vertex position for rasterization
    float4 WorldPosition : TEXCOORD0; // World position of the vertex
    float2 UV : TEXCOORD1; // Output texture coordinates
    float3 Normal : NORMAL; // Transformed vertex normal
    float3 Tangent : TANGENT; // Transformed vertex tangent
};



//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// BRDF Functions
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

//Lambertian reflectance
float4 CalculateLambert(float kd, float4 cd)
{
    float invPI = 1.f / gPI;
    return cd * kd * invPI;
}

// Calculate Phong reflection
float CalculatePhong(float ks, float exp, float3 l, float3 v, float3 n)
{
    const float3 reflectVec = reflect(l, n); // Reflected light vector
    const float alfa = saturate(dot(reflectVec, v)); // Angle between reflection and view vector

    // Simplify conditional calculation, avoiding unnecessary pow operations
    float phong = (alfa > 0) ? ks * pow(alfa, exp) : 0.0f; //phong specular reflection calculation

    return phong;
}

// Pixel shader performing lighting calculations
float4 PS_Phong(VS_OUTPUT input, SamplerState state) : SV_TARGET
{
    // Tangent space transformation for normal mapping
    const float3 binormal = cross(input.Normal, input.Tangent);
    const float4x4 tangentSpaceAxis = float4x4(float4(input.Tangent, 0.0f), float4(binormal, 0.0f), float4(input.Normal, 0.0), float4(0.0f, 0.0f, 0.0f, 1.0f));
    const float3 currentNormalMap = 2.0f * gNormalMap.Sample(state, input.UV).rgb - float3(1.0f, 1.0f, 1.0f);
    const float3 normal = mul(float4(currentNormalMap, 0.0f), tangentSpaceAxis);

    const float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);

    // Lighting calculations
    const float observedArea = saturate(dot(normal, -gLightDirection));
    const float4 lambert = CalculateLambert(1.0f, gDiffuseMap.Sample(state, input.UV));
    const float specularExp = gShininess * gGlossinessMap.Sample(state, input.UV).r;
    const float4 specular = gSpecularMap.Sample(state, input.UV) * CalculatePhong(1.0f, specularExp, -gLightDirection, viewDirection, input.Normal);

    return (gLightIntensity * lambert + specular) * observedArea; // Final lighting calculation
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// VERTEX SHADER
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Position = mul(float4(input.Position, 1.f),gWorldViewProj);
    output.UV = input.UV; //UV -> Pass UV to pixel shader
    output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
    return output;
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// PIXEL SHADERS
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
float4 PixelShader_Point(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input, gSamPoint);
}

float4 PixelShader_Linear(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input, gSamLinear);
}

float4 PixelShader_Anisotropic(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input, gSamAnisotropic);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// TECHNIQUES
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
technique11 PointFilterTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PixelShader_Point()));
    }
}
technique11 LinearFilterTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PixelShader_Linear()));
    }
}
technique11 AnisotropicFilterTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PixelShader_Anisotropic()));
    }
}