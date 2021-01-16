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

// ���� �޾Ƽ� ������� �� ���� �����ϴ� ���� ���̴�
// �� ���� ���̴��� ����ϱ� ���ؼ��� ������ ���� ������ �ʿ��ϴ�.
// ���� ������ ���� ��ġ, ũ�Ⱑ �ʿ��ѵ� �� �� ũ��� 2���� ������
// �ʿ��ѵ� ������� ����, ����� ���� ������ �̿� �ش�ȴ�.
// ���� �ۼ��� �ڵ�� ó���� �������͵� �ʿ��ϰ� ũ�⵵ �����̽� ��������
// �ʿ��� �� �˾Ƽ� ���� ������ ��ġ, ����, ũ��(float3)���� ¥���ִµ�
// ���� ¥���� ������ �̺��� �� ���� �ʿ�� �ϴ���...
// TriangleStream�� ����� ������ �ְ� �Ǹ� �ﰢ�� �� ���·� ����� �ȴ�.
// �׷��� ���� �ڵ�� �� ���� ������ �� �����ش�.
[maxvertexcount(40)]
void GS(line VertexOut gin[2], uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream) // �ϼ��� ������ �ﰢ�� ��Ʈ���� ��´�.
{
    float Hlen = gin[0].SizeW.x; // ����� ����
    float Cx = gin[0].PosW.x, Cy, Cz = gin[0].PosW.z; // ���� ���� ����
    float Cx2 = gin[1].PosW.x, Cz2 = gin[1].PosW.z; // ���� �� ����
    int StackCnt = gin[0].SizeW.z; // ���� ����
    float3 N = normalize(float3(Cx, 0, Cz)), N2 = normalize(float3(Cx2, 0, Cz2)); // �� ������ ���� ����
    
    GeoOut gout;
    gout.PrimID = primID;
    
    // ���� �޾Ƽ� ������� �Ѹ��� �����ϴ� 
    for (int k = 0; k <= StackCnt; k++)
    {
        // ���� ����
        Cy = -Hlen / 2 + Hlen / StackCnt * k;
        
        // ���� ù��° ���� �ְ�
        gout.PosW = mul(float4(Cx, Cy, Cz, 1.0f), gWorld).xyz;
        gout.NormalW = mul(N, (float3x3) gWorldInvTranspose);
        gout.PosH = mul(float4(Cx, Cy, Cz, 1.0f), gWorldViewProj);
        triStream.Append(gout);
    
        // ���� �ι�° ���� �ְ�
        gout.PosW = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorld).xyz;
        gout.NormalW = mul(N2, (float3x3) gWorldInvTranspose);
        gout.PosH = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorldViewProj);
        triStream.Append(gout);
    }
}

/*
// ���ǥ�ð� �ߴ� ���� ���̴� �ڵ�
// ��� ��ο��� gout[]�� ���� ���� �� ���ٰ� ������ ���մµ�
// �� �׷��� ������ �𸣰ڴ�.
// ���� �����÷ο쿡�� ã�ƺ��� �ȳ��´�.
[maxvertexcount(40)]
void GS(line VertexOut gin[2], uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream) // �ϼ��� ������ �ﰢ�� ��Ʈ���� ��´�.
{
    float Hlen = gin[0].SizeW.x;
    float Cx = gin[0].PosW.x, Cy = -Hlen / 2, Cz = gin[0].PosW.z;
    float Cx2 = gin[1].PosW.x, Cz2 = gin[1].PosW.z;
    int StackCnt = gin[0].SizeW.z;
    float3 N = normalize(float3(Cx, 0, Cz)), N2 = normalize(float3(Cx2, 0, Cz2));
    GeoOut gout[4];
    gout[0].PrimID = gout[1].PrimID = gout[2].PrimID = gout[3].PrimID = primID;
    
    // ù��° ���� �ְ�
    gout[0].PosW = mul(float4(Cx, Cy, Cz, 1.0f), gWorld).xyz;
    gout[0].NormalW = mul(N, (float3x3) gWorldInvTranspose);
    gout[0].PosH = mul(float4(Cx, Cy, Cz, 1.0f), gWorldViewProj);
    
    // �ι�° ���� �ְ�
    gout[1].PosW = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorld).xyz;
    gout[1].NormalW = mul(N2, (float3x3) gWorldInvTranspose);
    gout[1].PosH = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorldViewProj);
    
    for (int k = 1; k <= StackCnt; k++)
    {
        // ���� ����
        Cy = -Hlen / 2 + Hlen / StackCnt * k;
        
        // ����° ���� �ְ�
        gout[2].PosW = mul(float4(Cx, Cy, Cz, 1.0f), gWorld).xyz;
        gout[2].NormalW = mul(N, (float3x3) gWorldInvTranspose);
        gout[2].PosH = mul(float4(Cx, Cy, Cz, 1.0f), gWorldViewProj);
    
        // �׹�° ���� �ְ�
        gout[3].PosW = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorld).xyz;
        gout[3].NormalW = mul(N2, (float3x3) gWorldInvTranspose);
        gout[3].PosH = mul(float4(Cx2, Cy, Cz2, 1.0f), gWorldViewProj);
        
        for (int j = 0; j < 4; j++)
        {
            triStream.Append(gout[j]);
        }
		triStream.RestartStrip();
        gout[0] = gout[2];
        gout[1] = gout[3];
    }
}
*/

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

technique11 Light3Fog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, true)));
    }
}

technique11 Light3AlphaClipFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, true, true)));
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
