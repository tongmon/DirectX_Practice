//***************************************************************************************
// ParticleSystem.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "ParticleSystem.h"
#include "TextureMgr.h"
#include "Vertex.h"
#include "Effects.h"
#include "Camera.h"
 
ParticleSystem::ParticleSystem()
: mInitVB(0), mDrawVB(0), mStreamOutVB(0), mTexArraySRV(0), mRandomTexSRV(0)
{
	mFirstRun = true;
	mGameTime = 0.0f;
	mTimeStep = 0.0f;
	mAge      = 0.0f;

	mEyePosW  = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitPosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

ParticleSystem::~ParticleSystem()
{
	ReleaseCOM(mInitVB);
	ReleaseCOM(mDrawVB);
	ReleaseCOM(mStreamOutVB);
}

float ParticleSystem::GetAge()const
{
	return mAge;
}

void ParticleSystem::SetEyePos(const XMFLOAT3& eyePosW)
{
	mEyePosW = eyePosW;
}

void ParticleSystem::SetEmitPos(const XMFLOAT3& emitPosW)
{
	mEmitPosW = emitPosW;
}

void ParticleSystem::SetEmitDir(const XMFLOAT3& emitDirW)
{
	mEmitDirW = emitDirW;
}

void ParticleSystem::Init(ID3D11Device* device, ParticleEffect* fx, ID3D11ShaderResourceView* texArraySRV, 
	                      ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles)
{
	mMaxParticles = maxParticles;

	mFX = fx;

	mTexArraySRV  = texArraySRV;
	mRandomTexSRV = randomTexSRV; 

	BuildVB(device);
}

void ParticleSystem::Reset()
{
	mFirstRun = true;
	mAge      = 0.0f;
}

void ParticleSystem::Update(float dt, float gameTime)
{
	mGameTime = gameTime;
	mTimeStep = dt;

	mAge += dt;
}

// 입자 시스템은 스트림 출력으로 기하 셰이더에서 변형한 정점을 GPU에 담고 (렌더링 X)
// 이 담은 정점을 정점 버퍼로 하여 실제로 렌더링하여 그리는 2개의 처리 과정이 필요하다.
void ParticleSystem::Draw(ID3D11DeviceContext* dc, const Camera& cam)
{
	XMMATRIX VP = cam.ViewProj();

	//
	// 상수들을 설정
	//
	mFX->SetViewProj(VP);
	mFX->SetGameTime(mGameTime);
	mFX->SetTimeStep(mTimeStep);
	mFX->SetEyePosW(mEyePosW);
	mFX->SetEmitPosW(mEmitPosW);
	mFX->SetEmitDirW(mEmitDirW);
	mFX->SetTexArray(mTexArraySRV);
	mFX->SetRandomTex(mRandomTexSRV);

	//
	// 입력 조립기 단계를 설정
	//
	dc->IASetInputLayout(InputLayouts::Particle);
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(Vertex::Particle);
    UINT offset = 0;

	// 최초 실행이면 초기화용 정점 버퍼를 사용하고, 그렇지 않으면
	// 현재의 입자 목록을 담은 정점 버퍼를 사용한다.
	if( mFirstRun )
		dc->IASetVertexBuffers(0, 1, &mInitVB, &stride, &offset);
	else
		dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	//
	// 현재 입자 목록을 스트림 출력 전용 기법으로 그려서 입자들을 갱신한다.
	// 갱신된 입자들은 스트림 출력을 통해서 대상 정점 버퍼에 기록된다.
	//
	dc->SOSetTargets(1, &mStreamOutVB, &offset); // 스트림 출력에 정점 연동

    D3DX11_TECHNIQUE_DESC techDesc;
	mFX->StreamOutTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
        mFX->StreamOutTech->GetPassByIndex( p )->Apply(0, dc);
        
		if( mFirstRun )
		{
			dc->Draw(1, 0);
			mFirstRun = false;
		}
		else
		{
			// 실제 화면에 그려지지는 않고 기하 셰이더에서 스트림 출력에 의해
			// 정의되는 정점을 GPU에 담는다.
			// 이 담은 정점은 다음 그리기 처리에서 실제 렌더링 될 것이다.
			dc->DrawAuto();
		}
    }

	// 스트림 출력이 끝났으니 스트림 정점 버퍼를 떼어낸다.
	ID3D11Buffer* bufferArray[1] = {0};
	dc->SOSetTargets(1, bufferArray, &offset);

	// 정점 버퍼들을 맞바꾼다. (핑퐁)
	std::swap(mDrawVB, mStreamOutVB);

	//
	// 방금 스트림 출력된, 갱신된 입자 시스템을 화면에 그린다.
	//
	dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	mFX->DrawTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
        mFX->DrawTech->GetPassByIndex( p )->Apply(0, dc);
        
		dc->DrawAuto();
    }

	// 최종적으로 이 공정을 정리해보면 SO 단계에 묶인 mStreamOutVB 여기에 기하셰이더에서 변형하여 배출한 정점들이
	// 담기고 이 담긴 녀석을 swap하기 위해서 SO단계에서 뗀다.
	// mStreamOutVB <--> mDrawVB 얘네를 바꿔치기 하는데 기존의 DrawVB에 들어있던 정점은 mStreamOutVB 여기로 이동하고
	// mDrawVB로 mStreamOutVB에 담긴 녀석들이 가고 mDrawVB를 렌더링하면 효과가 화면에 보인다.
	// 바뀐 mStreamOutVB에는 다시 갱신된 효과 정점들이 덮어씌워질 것이고 이게 반복되면서 어떤 효과가 갱신되는 것이다.
}

void ParticleSystem::BuildVB(ID3D11Device* device)
{
	//
	// 입자들을 생산할 시작 정점 생성
	// 용암을 분출하는 분화구 같은 역할을 한다고 볼 수 있다.
	//

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Vertex::Particle) * 1;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	Vertex::Particle p;
	ZeroMemory(&p, sizeof(Vertex::Particle));
	p.Age  = 0.0f;
	p.Type = 0; 
 
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &p;

	HR(device->CreateBuffer(&vbd, &vinitData, &mInitVB)); // 입자 방출 정점 생성
	
	//
	// 핑퐁을 진행할 정점 버퍼 2개 생성
	// 핑퐁이라 하면 swap 하면서 서로의 위치를 계속 뒤바꾸는 기법을 말한다.
	// 이 짓을 여기서 왜 하냐면 하나의 정점 버퍼를
	// IA단계와 SO단계에 동시에 묶어둘 수가 없어서 하나가 SO단계에 묶였다면
	// 묶인 녀석을 swap으로 빼와서 다시 IA단계에 묶기 위해서이다.
	//
	vbd.ByteWidth = sizeof(Vertex::Particle) * mMaxParticles;
	// 스트림 출력을 하는 용도의 정점이면 D3D11_BIND_STREAM_OUTPUT 사용
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	// 하위 리소스는 필요없는 것이 기하 셰이더에서 정점을 만들어서
	// 이 정점에 동적으로 채울 것이기에 0으로 놓는다.
    HR(device->CreateBuffer(&vbd, 0, &mDrawVB));
	HR(device->CreateBuffer(&vbd, 0, &mStreamOutVB));
}