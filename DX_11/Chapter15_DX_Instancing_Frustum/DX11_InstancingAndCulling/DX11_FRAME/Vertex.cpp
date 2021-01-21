#include "Vertex.h"
#include "Effects.h"

#pragma region InputLayoutDesc

// D3D11_INPUT_PER_VERTEX_DATA는 정점별 자료를 의미하고
// D3D11_INPUT_PER_INSTANCE_DATA는 인스턴스별 자료를 의미한다.
// 맨 마지막 옵션인 InstanceDataStepRate는 여기서는 인스턴스 자료들이 모두
// 1로 설정되어서 하나의 물체에 각기 다른 종류의 색상, 위치가 적용될 텐데
// 이게 2로 설정되면 하나의 월드 행렬 당 2개의 물체가 해당되게 된다.
// 그니까 3으로 설정하면 빨, 빨, 빨, 파, 파, 파, 녹... 이런식으로 진행된다.
// 인스턴싱 자료는 여기서 입력 슬록 인덱스가 1로 되어서 정점 구조와 구별된다.
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::InstancedBasic32[8] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* InputLayouts::InstancedBasic32 = 0;

void InputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

	//
	// InstancedBasic32
	//

	Effects::InstancedBasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::InstancedBasic32, 8, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &InstancedBasic32));
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(InstancedBasic32);
}

#pragma endregion
