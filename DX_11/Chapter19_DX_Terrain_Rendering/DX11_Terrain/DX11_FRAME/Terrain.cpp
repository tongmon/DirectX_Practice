//***************************************************************************************
// Terrain.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Terrain.h"
#include "Camera.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include <fstream>
#include <sstream>

Terrain::Terrain() : 
	mQuadPatchVB(0), 
	mQuadPatchIB(0), 
	mLayerMapArraySRV(0), 
	mBlendMapSRV(0), 
	mHeightMapSRV(0),
	mNumPatchVertices(0),
	mNumPatchQuadFaces(0),
	mNumPatchVertRows(0),
	mNumPatchVertCols(0)
{
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());

	mMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
	mMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

Terrain::~Terrain()
{
	ReleaseCOM(mQuadPatchVB);
	ReleaseCOM(mQuadPatchIB);
	ReleaseCOM(mLayerMapArraySRV);
	ReleaseCOM(mBlendMapSRV);
	ReleaseCOM(mHeightMapSRV);
}

float Terrain::GetWidth()const
{
	// 전체 지형 너비(가로)
	return (mInfo.HeightmapWidth-1)*mInfo.CellSpacing;
}

float Terrain::GetDepth()const
{
	// 전체 지형 깊이(세로)
	return (mInfo.HeightmapHeight-1)*mInfo.CellSpacing;
}

float Terrain::GetHeight(float x, float z)const
{
	// Transform from terrain local space to "cell" space.
	float c = (x + 0.5f*GetWidth()) /  mInfo.CellSpacing;
	float d = (z - 0.5f*GetDepth()) / -mInfo.CellSpacing;

	// Get the row and column we are in.
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	// Grab the heights of the cell we are in.
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D
	float A = mHeightmap[row*mInfo.HeightmapWidth + col];
	float B = mHeightmap[row*mInfo.HeightmapWidth + col + 1];
	float C = mHeightmap[(row+1)*mInfo.HeightmapWidth + col];
	float D = mHeightmap[(row+1)*mInfo.HeightmapWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	// If upper triangle ABC.
	if( s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return A + s*uy + t*vy;
	}
	else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f-s)*uy + (1.0f-t)*vy;
	}
}

XMMATRIX Terrain::GetWorld()const
{
	return XMLoadFloat4x4(&mWorld);
}

void Terrain::SetWorld(CXMMATRIX M)
{
	XMStoreFloat4x4(&mWorld, M);
}

void Terrain::Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo)
{
	mInfo = initInfo;

	// 높이맵을 패치들의 격자로 분할하되, 격자의 각 패치의 지형 낱칸 개수가
	// CellsPerPatch가 되게 한다. (정점이라서 +1)
	// 부연설명을 하면 mInfo.HeightmapHeight 얘는 정점 단위로 높이가 매겨져 있는 높이값이다.
	// 여기서 1을 빼야 낱칸 단위가 된다.
	// CellsPerPatch 얘는 낱칸 단위로 64가 매겨져 있는데 패치 하나당 최대 테셀레이션이 되면
	// 하나의 사각형이 64개의 낱칸을 가진 사각형이 되기에 64로 지정이 되어 있는 것이다.
	// 그러면 (mInfo.HeightmapHeight-1) / CellsPerPatch 이 녀석은 높이맵을 64길이 단위로
	// 묶음을 짓는 것이고 여기에 + 1을 한다는 것은 낱칸 단위에서 정점 단위로 바꾸기 때문이다.
	// 예를 들어 1920이 mInfo.HeightmapHeight 얘면 1919 / 64 = 29 따라서 29개의 높이 방향의 낱칸이 생기고
	// 이 낱칸을 생성하기 위해서 30개의 높이 방향의 정점이 필요하다.
	mNumPatchVertRows = ((mInfo.HeightmapHeight-1) / CellsPerPatch) + 1;
	mNumPatchVertCols = ((mInfo.HeightmapWidth-1) / CellsPerPatch) + 1;

	mNumPatchVertices  = mNumPatchVertRows*mNumPatchVertCols; // 높이맵 정점 개수
	mNumPatchQuadFaces = (mNumPatchVertRows-1)*(mNumPatchVertCols-1); // 높이맵 격자(낱칸) 개수

	LoadHeightmap();
	Smooth();
	CalcAllPatchBoundsY();

	BuildQuadPatchVB(device);
	BuildQuadPatchIB(device);
	BuildHeightmapSRV(device);

	std::vector<std::wstring> layerFilenames;
	layerFilenames.push_back(mInfo.LayerMapFilename0);
	layerFilenames.push_back(mInfo.LayerMapFilename1);
	layerFilenames.push_back(mInfo.LayerMapFilename2);
	layerFilenames.push_back(mInfo.LayerMapFilename3);
	layerFilenames.push_back(mInfo.LayerMapFilename4);
	mLayerMapArraySRV = d3dHelper::CreateTexture2DArraySRV(device, dc, layerFilenames);

	HR(D3DX11CreateShaderResourceViewFromFile(device, 
		mInfo.BlendMapFilename.c_str(), 0, 0, &mBlendMapSRV, 0));
}

void Terrain::Draw(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3])
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	dc->IASetInputLayout(InputLayouts::Terrain);

	UINT stride = sizeof(Vertex::Terrain);
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);
	dc->IASetIndexBuffer(mQuadPatchIB, DXGI_FORMAT_R16_UINT, 0);

	XMMATRIX viewProj = cam.ViewProj();
	XMMATRIX world  = XMLoadFloat4x4(&mWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world*viewProj;

	XMFLOAT4 worldPlanes[6];
	ExtractFrustumPlanes(worldPlanes, viewProj);

	// Set per frame constants.
	Effects::TerrainFX->SetViewProj(viewProj);
	Effects::TerrainFX->SetEyePosW(cam.GetPosition());
	Effects::TerrainFX->SetDirLights(lights);
	Effects::TerrainFX->SetFogColor(Colors::Silver);
	Effects::TerrainFX->SetFogStart(15.0f);
	Effects::TerrainFX->SetFogRange(175.0f);
	Effects::TerrainFX->SetMinDist(20.0f);
	Effects::TerrainFX->SetMaxDist(500.0f);
	Effects::TerrainFX->SetMinTess(0.0f);
	Effects::TerrainFX->SetMaxTess(6.0f);
	Effects::TerrainFX->SetTexelCellSpaceU(1.0f / mInfo.HeightmapWidth);
	Effects::TerrainFX->SetTexelCellSpaceV(1.0f / mInfo.HeightmapHeight);
	Effects::TerrainFX->SetWorldCellSpace(mInfo.CellSpacing);
	Effects::TerrainFX->SetWorldFrustumPlanes(worldPlanes);
	
	Effects::TerrainFX->SetLayerMapArray(mLayerMapArraySRV);
	Effects::TerrainFX->SetBlendMap(mBlendMapSRV);
	Effects::TerrainFX->SetHeightMap(mHeightMapSRV);

	Effects::TerrainFX->SetMaterial(mMat);

	ID3DX11EffectTechnique* tech = Effects::TerrainFX->Light1Tech;
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc( &techDesc );

    for(UINT i = 0; i < techDesc.Passes; ++i)
    {
        ID3DX11EffectPass* pass = tech->GetPassByIndex(i);
		pass->Apply(0, dc);

		dc->DrawIndexed(mNumPatchQuadFaces*4, 0, 0);
	}	

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	dc->HSSetShader(0, 0, 0);
	dc->DSSetShader(0, 0, 0);
}

void Terrain::LoadHeightmap()
{
	// 각 정점의 높이를 담는 배열(벡터).
	std::vector<unsigned char> in(mInfo.HeightmapWidth * mInfo.HeightmapHeight);

	// 높이맵 파일을 연다.
	std::ifstream inFile;
	inFile.open(mInfo.HeightMapFilename.c_str(), std::ios_base::binary);

	if (inFile)
	{
		// RAW 파일의 바이트들을 읽어 들인다.
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		// 파일을 다 사용했음.
		inFile.close();
	}

	// 배열 자료를 부동 소수점 배열에 비례해서 복사한다.
	// 이렇게 저장해야 나중에 255 값보다 더 큰 높이를 그릴 수 있다.
	// 왜냐하면 초반 높이맵의 범위는 0 ~ 255까지니까....
	mHeightmap.resize(mInfo.HeightmapHeight * mInfo.HeightmapWidth, 0);
	for (UINT i = 0; i < mInfo.HeightmapHeight * mInfo.HeightmapWidth; ++i)
	{
		mHeightmap[i] = (in[i] / 255.0f) * mInfo.HeightScale;
	}
}

// 각 높이맵 픽셀에 Average를 적용하는 함수
void Terrain::Smooth()
{
	std::vector<float> dest( mHeightmap.size() );

	for(UINT i = 0; i < mInfo.HeightmapHeight; ++i)
	{
		for(UINT j = 0; j < mInfo.HeightmapWidth; ++j)
		{
			dest[i*mInfo.HeightmapWidth+j] = Average(i,j);
		}
	}

	// Replace the old heightmap with the filtered one.
	mHeightmap = dest;
}

bool Terrain::InBounds(int i, int j)
{
	// i와 j가 유효한 색인이면 true 아니면 false.
	return 
		i >= 0 && i < (int)mInfo.HeightmapHeight && 
		j >= 0 && j < (int)mInfo.HeightmapWidth;
}

float Terrain::Average(int i, int j)
{
	// ij번째 픽셀의 평균 높이를 계산하는 함수.
	// ij번째 픽셀 및 그 이웃의 여덟 픽셀의 평균을 구한다.
	// 가장자리 픽셀들의 경우, 해당 방향의 이웃 픽셀이 모자르다면
	// 그냥 평균에 포함시키지 않고 넘어감.
	//
	// ----------
	// | 1| 2| 3|
	// ----------
	// |4 |ij| 6|
	// ----------
	// | 7| 8| 9|
	// ----------

	float avg = 0.0f;
	float num = 0.0f;

	// 음의 값을 허용하기 위해 int를 사용한다.
	// i나 j가 0인 경우도 고려해야 한다.
	for(int m = i-1; m <= i+1; ++m)
	{
		for(int n = j-1; n <= j+1; ++n)
		{
			if( InBounds(m,n) )
			{
				avg += mHeightmap[m*mInfo.HeightmapWidth + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void Terrain::CalcAllPatchBoundsY()
{
	mPatchBoundsY.resize(mNumPatchQuadFaces);

	// For each patch
	for(UINT i = 0; i < mNumPatchVertRows-1; ++i)
	{
		for(UINT j = 0; j < mNumPatchVertCols-1; ++j)
		{
			CalcPatchBoundsY(i, j);
		}
	}
}

void Terrain::CalcPatchBoundsY(UINT i, UINT j)
{
	// 이 패치가 포괄하는 높이맵 원소들을 훑으면서
	// 최대, 최소 높이를 구한다.

	UINT x0 = j * CellsPerPatch;
	UINT x1 = (j + 1) * CellsPerPatch;

	UINT y0 = i * CellsPerPatch;
	UINT y1 = (i + 1) * CellsPerPatch;

	// 한 패치(64x64)에 해당하는 AABB 충돌 박스 생성
	float minY = +MathHelper::Infinity;
	float maxY = -MathHelper::Infinity;
	for (UINT y = y0; y <= y1; ++y)
	{
		for (UINT x = x0; x <= x1; ++x)
		{
			UINT k = y * mInfo.HeightmapWidth + x;
			minY = MathHelper::Min(minY, mHeightmap[k]);
			maxY = MathHelper::Max(maxY, mHeightmap[k]);
		}
	}

	UINT patchID = i * (mNumPatchVertCols - 1) + j;
	mPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}

void Terrain::BuildQuadPatchVB(ID3D11Device* device)
{
	std::vector<Vertex::Terrain> patchVertices(mNumPatchVertRows*mNumPatchVertCols); // 높이맵 정점들

	// 원래 지형 격자 만들때와 동일하게 진행
	float halfWidth = 0.5f*GetWidth();
	float halfDepth = 0.5f*GetDepth();

	// 낱칸 가로, 세로 길이
	float patchWidth = GetWidth() / (mNumPatchVertCols-1);
	float patchDepth = GetDepth() / (mNumPatchVertRows-1);
	// 텍스쳐 대응 좌표 가로, 세로 길이
	float du = 1.0f / (mNumPatchVertCols-1);
	float dv = 1.0f / (mNumPatchVertRows-1);

	for(UINT i = 0; i < mNumPatchVertRows; ++i)
	{
		float z = halfDepth - i*patchDepth;
		for(UINT j = 0; j < mNumPatchVertCols; ++j)
		{
			float x = -halfWidth + j*patchWidth;

			patchVertices[i*mNumPatchVertCols+j].Pos = XMFLOAT3(x, 0.0f, z);

			// 텍스쳐가 격자 전체에 입혀지게 적절한 Tex 좌표 할당
			patchVertices[i*mNumPatchVertCols+j].Tex.x = j*du;
			patchVertices[i*mNumPatchVertCols+j].Tex.y = i*dv;
		}
	}

	// 축 정렬 경계상자의 Y 경계들을 왼쪽 상단 모퉁이 패치에 저장해 둔다.
	for(UINT i = 0; i < mNumPatchVertRows-1; ++i)
	{
		for(UINT j = 0; j < mNumPatchVertCols-1; ++j)
		{
			UINT patchID = i*(mNumPatchVertCols-1)+j;
			patchVertices[i*mNumPatchVertCols+j].BoundsY = mPatchBoundsY[patchID];
		}
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Terrain) * patchVertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &patchVertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &mQuadPatchVB));
}

void Terrain::BuildQuadPatchIB(ID3D11Device* device)
{
	// 테셀레이션을 할 것이기 때문에 기존의 격자 인덱스 계산이 삼각형으로
	// 즉 k += 6 이렇게 계산이 되었지만 테셀레이션 제어점 4개 사각형으로 그릴 것이기에
	// 인덱스 계산이 k += 4 방식으로 이루어진다.
	std::vector<USHORT> indices(mNumPatchQuadFaces * 4); // 사각형 패치의 4개의 인덱스

	// 각 사각형 마다 색인들을 계산한다.
	int k = 0;
	for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
		{
			// 2x2 사각형 패치의 위쪽 정점 두 개
			indices[k] = i * mNumPatchVertCols + j;
			indices[k + 1] = i * mNumPatchVertCols + j + 1;

			// 2x2 사각형 패치의 아래쪽 정점 두 개
			indices[k + 2] = (i + 1) * mNumPatchVertCols + j;
			indices[k + 3] = (i + 1) * mNumPatchVertCols + j + 1;

			k += 4; // 다음 사각형으로 이동
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(device->CreateBuffer(&ibd, &iinitData, &mQuadPatchIB));
}

void Terrain::BuildHeightmapSRV(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mInfo.HeightmapWidth;
	texDesc.Height = mInfo.HeightmapHeight;
    texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format    = DXGI_FORMAT_R16_FLOAT; // 16비트 부동소수점 사용
	texDesc.SampleDesc.Count   = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// HALF는 16비트 부동소수점 저장을 위한 형식으로 xnamath.h에 정의됨.
	std::vector<HALF> hmap(mHeightmap.size());
	std::transform(mHeightmap.begin(), mHeightmap.end(), hmap.begin(), XMConvertFloatToHalf);
	
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &hmap[0];
    data.SysMemPitch = mInfo.HeightmapWidth*sizeof(HALF);
    data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = 0;
	HR(device->CreateTexture2D(&texDesc, &data, &hmapTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	HR(device->CreateShaderResourceView(hmapTex, &srvDesc, &mHeightMapSRV));

	// SRV saves reference.
	ReleaseCOM(hmapTex);
}