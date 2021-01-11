//***************************************************************************************
// RenderStates.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullRS = 0;

ID3D11BlendState* RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState* RenderStates::TransparentBS = 0;

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
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));
}

void RenderStates::DestroyAll()
{
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
}