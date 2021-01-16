//***************************************************************************************
// TreeSprite.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// ���� ���̴��� �̿��ؼ� �� ��������Ʈ�� y�࿡ ���ĵ� ī�޶� ���� ������ �簢������ Ȯ���Ѵ�.
//***************************************************************************************

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
};

// ���ġ ������ cbuffer�� �߰��� �� ����
Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float3 SizeL   : SIZE;
};

struct VertexOut
{
	float3 PosW    : POSITION;
	float3 NormalW : NORMAL;
	float3 SizeW   : SIZE;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	// �� fx�� �ѹ� ����ؼ� �ѹ��� �������� ��ü�� �׸��� �� ��ü �� ��ŭ
	// fx ������ �ڵ����� �ε����� ���� ��Ű�鼭 �ش� �׷�ġ�� ��ü�� ���� ID�� �����Ѵ�.
    uint   PrimID  : SV_PrimitiveID; // ���� ��ü ID
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// ���� ���̴����� �� ���� �ʰ� �״�� ���� ���̴��� �ѱ��.
	vout.PosW = vin.PosL;
	vout.NormalW = vin.NormalL;
	vout.SizeW   = vin.SizeL;

	return vout;
}

[maxvertexcount(100)]
void GS(line VertexOut gin[8], uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream) // �ϼ��� ������ �ﰢ�� ��Ʈ���� ��´�.
{
    float3 PosNomal[100][2];
    float4 posH[100];
    float height = gin[0].SizeW.x;
    int StackCnt = gin[0].SizeW.z;
    int VertSize = StackCnt * 8;
    int bufIndex = 0;
    for (int k = 0; k <= StackCnt; k++)
    {
        int Cy = -height / 2 + height / StackCnt * k;
		for (int j = 0; j < 8; j++)
        {
            float Cx = gin[j].PosW.x;
            float Cz = gin[j].PosW.z;
            PosNomal[bufIndex][0] = mul(float4(Cx, Cy, Cz, 1.0f), gWorld).xyz;

            float3 T = float3(-Cz, 0, Cx);
            float3 B = float3(0, -Cy, 0);
            float3 N = cross(T, B);
            N = normalize(N);		
            PosNomal[bufIndex][1] = mul(N, (float3x3) gWorldInvTranspose);
			
            posH[bufIndex++] = mul(float4(Cx, Cy, Cz, 1.0f), gWorldViewProj);
        }
    }
    GeoOut gout;
    gout.PrimID = primID;
    for (int i = 0;i<StackCnt;i++)
    {
        for (int j = 0; j < 8;j++)
        {
            gout.NormalW = PosNomal[i * 8 + j][1];
            gout.PosW = PosNomal[i * 8 + j][0];
            gout.PosH = posH[i * 8 + j];
            triStream.Append(gout);
            gout.NormalW = PosNomal[(i + 1) * 8 + j][1];
            gout.PosW = PosNomal[(i + 1) * 8 + j][0];
            gout.PosH = posH[(i + 1) * 8 + j];
            triStream.Append(gout);
            gout.NormalW = PosNomal[(i + 1) * 8 + j + 1][1];
            gout.PosW = PosNomal[(i + 1) * 8 + j + 1][0];
            gout.PosH = posH[(i + 1) * 8 + j + 1];
            triStream.Append(gout);
            triStream.RestartStrip();
			
            gout.NormalW = PosNomal[i * 8 + j][1];
            gout.PosW = PosNomal[i * 8 + j][0];
            gout.PosH = posH[i * 8 + j];
            triStream.Append(gout);
            gout.NormalW = PosNomal[(i + 1) * 8 + j + 1][1];
            gout.PosW = PosNomal[(i + 1) * 8 + j + 1][0];
            gout.PosH = posH[(i + 1) * 8 + j + 1];
            triStream.Append(gout);
            gout.NormalW = PosNomal[i * 8 + j + 1][1];
            gout.PosW = PosNomal[i * 8 + j + 1][0];
            gout.PosH = posH[i * 8 + j + 1];
            triStream.Append(gout);
            triStream.RestartStrip();
        }
    }
}

float4 PS(GeoOut pin, uniform int gLightCount, uniform bool gUseTexure, uniform bool gAlphaClip, uniform bool gFogEnabled) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
   
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		texColor = gDiffuseMap.Sample(samAnisotropic, float2(0, 0));

		if(gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.05.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.05f);
		}
	}

	//
	// Lighting.
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{
		// Start with a sum of zero.
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		[unroll]
		for(int i = 0; i < gLightCount; ++i)
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		// Modulate with late add.
		litColor = texColor*(ambient + diffuse) + spec;
	}

	//
	// Fogging
	//

	if( gFogEnabled )
	{
		float fogLerp = saturate( (distToEye - gFogStart) / gFogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

//---------------------------------------------------------------------------------------
// Techniques-- �ʿ��ϸ� �˾Ƽ� �˸°� �߰�, �����ϸ� �ȴ�.
//---------------------------------------------------------------------------------------
technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, false) ) );
    }
}

technique11 Light3TexAlphaClip
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, false) ) );
    }
}
            
technique11 Light3TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, true) ) );
    }
}
