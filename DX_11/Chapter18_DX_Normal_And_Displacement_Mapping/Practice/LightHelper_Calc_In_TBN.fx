// 기존의 광원 계산 방식이 세계 공간에서 이루어졌다면
// 여기서는 접공간에서 이루어진다.

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{ 
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
	float4 Reflect;
};

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(Material mat, DirectionalLight L, 
                             float3 normal, float3 toEye,
					         out float4 ambient,
						     out float4 diffuse,
						     out float4 spec)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -L.Direction;

	// Add ambient term.
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	
	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
}

// 접공간에서 빛 계산이 이루어진다.
void ComputeDirectionalLightNormal(Material mat, DirectionalLight L,
                             float3 normal, float3 toEye,
							 float3x3 revTBN,
					         out float4 ambient,
						     out float4 diffuse,
						     out float4 spec)
{
	// Initialize outputs.
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.Direction;
	
	// 시점과 빛 벡터를 접공간 기준으로 바꾼다.
    lightVec = mul(lightVec, revTBN);
    toEye = mul(toEye, revTBN);
	
	// 각 성분을 0 ~ 1(텍스쳐 좌표계 범위) 에서 -1 ~ 1로 변환한다.
    normal = 2.0f * normal - 1.0f;
	
	// Add ambient term.
    ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	
    float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFactor * mat.Specular * L.Specular;
    }
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
				   out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
	float d = length(lightVec);
	
	// Range test.
	if( d > L.Range )
		return;
		
	// Normalize the light vector.
	lightVec /= d; 
	
	// Ambient term.
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}

	// Attenuate
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d*d));

	diffuse *= att;
	spec    *= att;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
				  out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVec = L.Position - pos;
		
	// The distance from surface to light.
	float d = length(lightVec);
	
	// Range test.
	if( d > L.Range )
		return;
		
	// Normalize the light vector.
	lightVec /= d; 
	
	// Ambient term.
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
	
	// Scale by spotlight factor and attenuate.
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// Scale by spotlight factor and attenuate.
	float att = spot / dot(L.Att, float3(1.0f, d, d*d));

	ambient *= spot;
	diffuse *= att;
	spec    *= att;
}

//---------------------------------------------------------------------------------------
// 법선 맵 표본을 세계 공간으로 변환한다.
// normalMapSample 텍스쳐에서의 법선 맵
// unitNormalW 정점의 법선
// tangentW 정점의 접벡터
// 만약에 텍스쳐에 회전 변환 행렬을 곱했다면
// 여기서 추가적으로 접공간을 변환해야 한다. (이 함수에 변환 행렬 인자를 추가해야한다.)
// 방법은 현재 세계 공간으로 정의되어 있는 unitNormalW, tangentW 얘네를
// 접공간으로 끌어와야한다.
// 세계 공간 --> 접공간은 밑에 나온 TBN 벡터의 전치행렬이다. (TBN 벡터의 전치 행렬 == TBN 벡터의 역행렬)
// 끌어온 접공간의 N, T 벡터에 텍스쳐 회전 변환을 적용하고 다시 원래의
// TBN 벡터를 곱해서 접공간 --> 세계 공간으로 돌려놓는다.
// 그러면 지금 회전된 결과의 N, T, B 벡터가 새롭게 구해지고 얘네가 새로운 TBN 행렬을
// 구축하게 된다.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// 각 성분을 0 ~ 1(텍스쳐 좌표계 범위) 에서 -1 ~ 1로 변환한다.
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// 정규직교 기저를 구축한다.
	float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N); // tangentW, unitNormalW 사이의 각을 90도로 만들기 위한 변형
	float3 B = cross(N, T);

	// T(x), T(y), T(z)
	// B(x), B(y), B(z)
	// N(x), N(y), N(z)
	// TBN 벡터들은 정점 셰이더 단계에서 이미 세계 행렬이 모두 곱해진 상태의
	// unitNormalW, tangentW를 이용하여 구해졌기에
	// TBN은 세계공간 기준에서의 3개의 축 벡터이다.
	// 즉 TBN은 현재 세계 공간 좌표계 기준으로 서술이 되어 있는 상태이다.
	float3x3 TBN = float3x3(T, B, N);

	// 접공간에서 세계공간으로 변환한다.
	// 위의 TBN 행렬은 세계 공간 좌표계 기준으로 서술이 되어 있고
	// 텍스쳐 기준 좌표를 이 행렬과 곱해주면 기준 좌표계가
	// 세계 기준 좌표계의 TBN 축으로 서술이 된다.
	// 따라서 이 연산 후에 bumpedNormalW는 세계 공간 기준 좌표계로 서술이 된 상태이다.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}