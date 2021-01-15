//***************************************************************************************
// d3dUtil.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "d3dUtil.h"

ID3D11ShaderResourceView* d3dHelper::CreateTexture2DArraySRV(
		ID3D11Device* device, ID3D11DeviceContext* context,
		std::vector<std::wstring>& filenames,
		DXGI_FORMAT format,
		UINT filter, 
		UINT mipFilter)
{
	//
	// 각 텍스쳐 원소를 파일로부터 개별적으로 적재한다.
	// 이 텍스쳐들은 GPU가 사용하는 것이 아니다 (결속 플래그가 0)
	// 이들은 그냥 파일ㅇ서 이미지 자료를 적재하기 위한 것일 뿐이다.
	// CPU에서 자원을 읽어야 하므로 용도를 예비(staging)로 설정한다.
	//

	UINT size = filenames.size();

	std::vector<ID3D11Texture2D*> srcTex(size);
	for(UINT i = 0; i < size; ++i)
	{
		D3DX11_IMAGE_LOAD_INFO loadInfo;

        loadInfo.Width  = D3DX11_FROM_FILE;
        loadInfo.Height = D3DX11_FROM_FILE;
        loadInfo.Depth  = D3DX11_FROM_FILE;
        loadInfo.FirstMipLevel = 0;
        loadInfo.MipLevels = D3DX11_FROM_FILE;
        loadInfo.Usage = D3D11_USAGE_STAGING;
        loadInfo.BindFlags = 0;
        loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        loadInfo.MiscFlags = 0;
        loadInfo.Format = format;
        loadInfo.Filter = filter;
        loadInfo.MipFilter = mipFilter;
		loadInfo.pSrcInfo  = 0;

		// 각 이미지 자원 할당
        HR(D3DX11CreateTextureFromFile(device, filenames[i].c_str(), 
			&loadInfo, 0, (ID3D11Resource**)&srcTex[i], 0));
	}

	//
	// 텍스쳐 배열을 생성한다.
	// 텍스쳐 배열의 모든 원소는 형식과 크기가 동일하다.
	//

	D3D11_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width              = texElementDesc.Width;
	texArrayDesc.Height             = texElementDesc.Height;
	texArrayDesc.MipLevels          = texElementDesc.MipLevels;
	texArrayDesc.ArraySize          = size;
	texArrayDesc.Format             = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count   = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage              = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags     = 0;
	texArrayDesc.MiscFlags          = 0;

	ID3D11Texture2D* texArray = 0;
	HR(device->CreateTexture2D( &texArrayDesc, 0, &texArray));

	//
	// 개별 텍스쳐 원소를 텍스쳐 배열에 복사한다.
	//

	// 각각의 텍스쳐 원소에 대해
	for(UINT texElement = 0; texElement < size; ++texElement)
	{
		// 각각 밉맵 수준에 대해
		for(UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HR(context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

			// D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels) 함수는
			// 텍스쳐 배열의 어떤 한 부분에 대한 인덱스 계산을 해주는 함수이다.
			// index = arraySlice * mipLevels + mipSlice
			// mipLevels은 밉맵이 몇개가 있느냐, arraySlice는 현재까지 지나온 텍스쳐 종류, mipSlice는 현재 밉맵 위치
			// 밑의 코드는 각 이미지의 밉맵 자원을 할당하는 것
			context->UpdateSubresource(texArray, 
				D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
				0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

			context->Unmap(srcTex[texElement], mipLevel);
		}
	}	

	//
	// 텍스쳐 배열에 대한 자원 뷰를 생성한다.
	//
	
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = size;

	ID3D11ShaderResourceView* texArraySRV = 0;
	HR(device->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

	//
	// 마지막까지 필요한 것은 자원뷰 뿐이기에 나머지 할당 자원들 해제
	//

	ReleaseCOM(texArray);

	for(UINT i = 0; i < size; ++i)
		ReleaseCOM(srcTex[i]);

	return texArraySRV;
}

ID3D11ShaderResourceView* d3dHelper::CreateRandomTexture1DSRV(ID3D11Device* device)
{
	// 
	// Create the random data.
	//
	XMFLOAT4 randomValues[1024];

	for(int i = 0; i < 1024; ++i)
	{
		randomValues[i].x = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].y = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].z = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].w = MathHelper::RandF(-1.0f, 1.0f);
	}

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024*sizeof(XMFLOAT4);
    initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = 1024;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

	ID3D11Texture1D* randomTex = 0;
    HR(device->CreateTexture1D(&texDesc, &initData, &randomTex));

	//
	// Create the resource view.
	//
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;
	
	ID3D11ShaderResourceView* randomTexSRV = 0;
    HR(device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV));

	ReleaseCOM(randomTex);

	return randomTexSRV;
}

void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M)
{
	//
	// Left
	//
	planes[0].x = M(0,3) + M(0,0);
	planes[0].y = M(1,3) + M(1,0);
	planes[0].z = M(2,3) + M(2,0);
	planes[0].w = M(3,3) + M(3,0);

	//
	// Right
	//
	planes[1].x = M(0,3) - M(0,0);
	planes[1].y = M(1,3) - M(1,0);
	planes[1].z = M(2,3) - M(2,0);
	planes[1].w = M(3,3) - M(3,0);

	//
	// Bottom
	//
	planes[2].x = M(0,3) + M(0,1);
	planes[2].y = M(1,3) + M(1,1);
	planes[2].z = M(2,3) + M(2,1);
	planes[2].w = M(3,3) + M(3,1);

	//
	// Top
	//
	planes[3].x = M(0,3) - M(0,1);
	planes[3].y = M(1,3) - M(1,1);
	planes[3].z = M(2,3) - M(2,1);
	planes[3].w = M(3,3) - M(3,1);

	//
	// Near
	//
	planes[4].x = M(0,2);
	planes[4].y = M(1,2);
	planes[4].z = M(2,2);
	planes[4].w = M(3,2);

	//
	// Far
	//
	planes[5].x = M(0,3) - M(0,2);
	planes[5].y = M(1,3) - M(1,2);
	planes[5].z = M(2,3) - M(2,2);
	planes[5].w = M(3,3) - M(3,2);

	// Normalize the plane equations.
	for(int i = 0; i < 6; ++i)
	{
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
		XMStoreFloat4(&planes[i], v);
	}
}