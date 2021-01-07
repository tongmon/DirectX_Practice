//***************************************************************************************
// LightHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper classes for lighting.
//***************************************************************************************

#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <Windows.h>
#include <xnamath.h>

// Note: Make sure structure alignment agrees with HLSL structure padding rules. 
//   Elements are packed into 4D vectors with the restriction that an element
//   cannot straddle a 4D vector boundary.

struct DirectionalLight // 직접광(태양광)
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient; // 주변광
	XMFLOAT4 Diffuse; // 빛 색상
	XMFLOAT4 Specular; // 반영광(물체에 빛이 강하게 비추면 생기는 흰 색 부분)
	XMFLOAT3 Direction; // 방향
	float Pad; // 필요하다면 빛들의 배열을 설정할 수 있게 마지막에 float 하나의 자리를 마련함.
};

struct PointLight // 점광(백열전구)
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// (Position, Range) 형태로 하나의 4차원 벡터에 채워 넣음.
	XMFLOAT3 Position; // 빛 원
	float Range; // 빛 범위

	// (A0, A1, A2, Pad) 형태로 하나의 4차원 벡터에 채워 넣음.
	// 빛의 세기가 거리에 따라 감소하는데 이 식에서 3개의 계수를 의미
	// 점광은 빛의 세기를 A0 + A1 * 거리 + A2 * 거리 * 거리를 빛 값에 나누어 조절함
	XMFLOAT3 Att;
	float Pad; // 필요하다면 빛들의 배열을 설정할 수 있게 마지막에 float 하나의 자리를 마련함.
};

struct SpotLight // 스포트라이트(손전등, 점광에서 범위 제한)
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// (Position, Range) 형태로 하나의 4차원 벡터에 채워 넣음.
	// 점광과 같음
	XMFLOAT3 Position;
	float Range;

	// (Direction, Spot) 형태로 하나의 4차원 벡터에 채워 넣음.
	// 점광에서 방향 추가
	XMFLOAT3 Direction;
	float Spot; // 점점광의 빛 범위는 원뿔 모양인데 이를 제어

	// (Att, Pad) 형태로 하나의 4차원 벡터에 채워 넣음.
	XMFLOAT3 Att;
	float Pad; // 필요하다면 빛들의 배열을 설정할 수 있게 마지막에 float 하나의 자리를 마련함.
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect;
};

#endif // LIGHTHELPER_H