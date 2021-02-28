//***************************************************************************************
// Terrain.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Class that renders a terrain using hardware tessellation and multitexturing.
//***************************************************************************************

#ifndef TERRAIN_H
#define TERRAIN_H

#include "d3dUtil.h"

class Camera;
struct DirectionalLight;

class Terrain
{
public:
	struct InitInfo
	{
		std::wstring HeightMapFilename; // RAW 파일 이름

		// 지형맵의 텍스쳐 파일 이름들
		std::wstring LayerMapFilename0;
		std::wstring LayerMapFilename1;
		std::wstring LayerMapFilename2;
		std::wstring LayerMapFilename3;
		std::wstring LayerMapFilename4;
		std::wstring BlendMapFilename;

		// 높이 비례 계수
		float HeightScale;
		// 높이맵 크기
		UINT HeightmapWidth;
		UINT HeightmapHeight;
		// 격자 각 칸의 크기, Grid 생성시에 사용하던 ux, uz를 의미
		float CellSpacing;
	};

public:
	Terrain();
	~Terrain();

	float GetWidth()const;
	float GetDepth()const;
	float GetHeight(float x, float z)const;

	XMMATRIX GetWorld()const;
	void SetWorld(CXMMATRIX M);

	void Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo);

	void Draw(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3]);

private:
	void LoadHeightmap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	void BuildQuadPatchVB(ID3D11Device* device);
	void BuildQuadPatchIB(ID3D11Device* device);
	void BuildHeightmapSRV(ID3D11Device* device);

private:

	// 최대 테셀레이션 계수가 64이다.
	static const int CellsPerPatch = 64;

	ID3D11Buffer* mQuadPatchVB;
	ID3D11Buffer* mQuadPatchIB;

	ID3D11ShaderResourceView* mLayerMapArraySRV;
	ID3D11ShaderResourceView* mBlendMapSRV;
	ID3D11ShaderResourceView* mHeightMapSRV;

	InitInfo mInfo;

	UINT mNumPatchVertices;
	UINT mNumPatchQuadFaces;

	UINT mNumPatchVertRows;
	UINT mNumPatchVertCols;

	XMFLOAT4X4 mWorld;

	Material mMat;

	std::vector<XMFLOAT2> mPatchBoundsY; // x성분에 y의 최솟값, y성분에 y의 최댓값을 저장한다.
	std::vector<float> mHeightmap;
};

#endif // TERRAIN_H