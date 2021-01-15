#include "Vertex.h"
#include "Effects.h"

#pragma region InputLayoutDesc

// 기본 도형 정점 구조
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Basic32[3] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

// 나무 정점 구조
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::TreePointSprite[2] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* InputLayouts::Basic32 = 0;
ID3D11InputLayout* InputLayouts::TreePointSprite = 0;

void InputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

	//
	// Basic32
	//

	Effects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::Basic32, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Basic32));

	//
	// TreePointSprite
	// 효과 객체 서술 구조체를 획득하고 그 구조체로부터 정점 쉐이더의 입력 서명에 접근 
	// TreeSpriteFX->Light3Tech->GetPassByIndex(0)->GetDesc(&passDesc)의 의미는
	// 효과 객체 안의 --> 효과 테크닉에서 -> 인덱스에 해당하는 패스 획득 -> 획득한 패스를 패스 포인터에 저장
	// 위와 같고 저장된 패스 포인터를 이용해서 fx와 cpp가 연동된 입력 레이아웃을
	// CreateInputLayout를 사용하여 생성
	//

	Effects::TreeSpriteFX->Light3Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::TreePointSprite, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &TreePointSprite));
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(Basic32);
	ReleaseCOM(TreePointSprite);
}

#pragma endregion
