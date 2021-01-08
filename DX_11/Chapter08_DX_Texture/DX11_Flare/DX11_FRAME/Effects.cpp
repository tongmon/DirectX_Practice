//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;
	HRESULT hr = D3DX11CompileFromFile(filename.c_str(), 0, 0, 0, "fx_5_0", shaderFlags,
		0, 0, &compiledShader, &compilationMsgs, 0);

	// compilationMsgs에 오류 메시지나 경고 메시지가 저장되어 있을 수 있다.
	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	// compilationMsgs에 오류 메시지가 없다고 해도 명시적으로 컴파일 오류 여부를 검사
	if (FAILED(hr))
	{
		DXTrace((WCHAR*)__FILEW__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
		0, device, &mFX)); // 이펙트를 mFX에 저장

	// 컴파일된 셰이더를 다 사용해서 해제함
	ReleaseCOM(compiledShader);
}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");
	DiffuseMap[0] = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource(); // flare 텍스쳐 가리키는 포인터
	DiffuseMap[1] = mFX->GetVariableByName("gDiffuseMap_alpha")->AsShaderResource(); // flare_alpha 텍스쳐 가리키는 포인터
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region Effects

BasicEffect* Effects::BasicFX = 0;

void Effects::InitAll(ID3D11Device* device)
{
	BasicFX = new BasicEffect(device, L"FX/Basic.fx");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
}
#pragma endregion