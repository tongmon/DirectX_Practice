//***************************************************************************************
// RenderStates.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullRS = 0;

ID3D11BlendState* RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState* RenderStates::TransparentBS = 0;

ID3D11DepthStencilState* RenderStates::NormalDSS = 0;
ID3D11DepthStencilState* RenderStates::IncStencilDSS = 0;
ID3D11DepthStencilState* RenderStates::DistStencilDSS = 0;

void RenderStates::InitAll(ID3D11Device* device)
{
	//
	// 레스터화기 단계, 와이어 프레임, 후면 제거
	//
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&wireframeDesc, &WireframeRS));

	//
	// 솔리드, 후면 제거 없음, 상자 그릴 때 투명한 부분이 있기에 이렇게 처리
	//
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullDesc, &NoCullRS));

	//
	// AlphaToCoverageBS
	//

	D3D11_BLEND_DESC alphaToCoverageDesc = { 0 };
	alphaToCoverageDesc.AlphaToCoverageEnable = true; // 식물이나 창살문 텍스처 렌더링에 유용한 다중표본화 기법을 킬지 말지 결정
	alphaToCoverageDesc.IndependentBlendEnable = false; // 각 렌더 대상마다 개별적인 혼합을 진행할 것인지 아닌지 결정

	// RenderTarget은 원소 8개짜리 배열인데 인덱스는 렌더 대상 순서를 의미한다.
	// 따라서 IndependentBlendEnable를 켰다면 이 RenderTarget인덱스에 따라 옵션이 다르게 적용되겠지만
	// 끄면 0번째 인덱스 설정이 나머지 렌더 대상에게 모두 적용된다.
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false; // 혼합을 활성화하려면 true 아니면 false
	// 렌더 대상 마스크 쓰기 설정
	// 마스크를 적절히 설정하여 RGB 채널에 결과가 반영되지 않게 하며서 알파값에만 반영이 되게 한다던지
	// 그 반대를 적용 한다던지, 밑과 같이 정상적용을 한다던지가 가능하다.
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	// 나머지 옵션들
	// SrcBlend = RGB 성분 혼합의 원본 혼합 계수 F(src)를 의미한다.
	// DestBlend = RGB 성분 혼합의 대상 혼합 계수 F(dst)를 의미한다.
	// BlendOp = RGB 성분 혼합 연산자를 의미한다.
	// SrcBlendAlpha = 알파 성분 혼합의 원본 혼합 계수 F(src)를 의미한다.
	// DestBlendAlpha = 알파 성분 혼합의 대상 혼합 계수 F(dst)를 의미한다.
	// BlendOpAlpha = 알파 성분 혼합의 혼합 연산자를 의미한다.

	// 혼합 상태 설정 함수
	// 생성할 혼합 생태를 서술하는 구조체, 생성된 혼합 상태 인터페이스가 담기는 포인터
	// 위와 같은 인자들로 구성이 된다.
	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	//
	// 투명 혼합 상태
	// 일반적인 혼합 식 --> F(src) * C(src) + F(dst) * C(dst)
	// 사이에 박힌 연산자들은 밑의 옵션을 통해 바꿔줄 수 있다.
	//

	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 원본 픽셀의 불투명도
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 원본 픽셀의 투명도, 이 투명도 만큼 뒤에 가려지는 대상 픽셀의 색이 표현됨
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // D3D11_BLEND_ZERO, D3D11_BLEND_ONE
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));

	D3D11_DEPTH_STENCIL_DESC IncDesc;
	IncDesc.DepthEnable = true;
	IncDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	IncDesc.DepthFunc = D3D11_COMPARISON_LESS;
	IncDesc.StencilEnable = true;
	IncDesc.StencilReadMask = 0xff;
	IncDesc.StencilWriteMask = 0xff;

	IncDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	IncDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	IncDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR; // 스텐실과 깊이 버퍼 판정이 모두 성공했으면 스텐실 버퍼 증가시킴
	IncDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // 0으로 같으면 픽셀을 그림자 색으로 혼합

	// 후면 삼각형에 대한 스텐실 버퍼 적용 방식 설정, 보통 후면 제거를 하기에 안쓰인다.
	IncDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	IncDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	IncDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	IncDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&IncDesc, &IncStencilDSS));

	D3D11_DEPTH_STENCIL_DESC distDesc;
	distDesc.DepthEnable = true;
	distDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	distDesc.DepthFunc = D3D11_COMPARISON_LESS;
	distDesc.StencilEnable = true;
	distDesc.StencilReadMask = 0xff;
	distDesc.StencilWriteMask = 0xff;

	distDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	distDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	distDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; // 스텐실과 깊이 버퍼 판정이 모두 성공했으면 스텐실 버퍼 증가시킴
	distDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL; // 0으로 같으면 픽셀을 그림자 색으로 혼합

	// 후면 삼각형에 대한 스텐실 버퍼 적용 방식 설정, 보통 후면 제거를 하기에 안쓰인다.
	distDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	distDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	distDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	distDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&distDesc, &DistStencilDSS));
}

void RenderStates::DestroyAll()
{
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
	ReleaseCOM(IncStencilDSS);
	ReleaseCOM(DistStencilDSS);
}