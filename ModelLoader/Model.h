#pragma once

#include <goh.h>

#include <Vertex.h>
#include <ModelMesh.h>
#include <TextureMgr.h>
#include <ModelAnimation.h>
#include <Camera.h>

class Model
{
public:
	Model(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename);
	~Model();

	void AddAnimation(const std::string& filename, const std::string& animname);

	UINT mSubsetCount;

	// 머터리얼 관련 변수들
	std::vector<Material> mMat;
	std::vector<ID3D11ShaderResourceView*> mDiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> mNormalMapSRV;
	std::vector<ID3D11ShaderResourceView*> mSpecularMapSRV;

	// CPU 계산 처리 때문에 정점 같은 정보 남겨둠
	std::vector<Vertex::PosNormalTexTanSkinned> mVertices;
	std::vector<UINT> Indices;
	std::vector<ModelLoader::Subset> mSubsets;

	// 메시 정보, 정점, 인덱스, 서브셋이 담김
	ModelMesh mMeshes;

	// 뼈 오프셋 행렬
	std::vector<XMFLOAT4X4> mBoneOffset;

	// 애니메이션 관련 정보
	std::unordered_map<std::string, ModelAnimation*> mAnimData;
};

struct ModelInstance
{
	Model* mModel = nullptr;
	float mTimePos = 0.0f;
	std::string mClipName = "";
	XMFLOAT4X4 mWorld;
	std::vector<XMFLOAT4X4> mFinalTransforms = {};

	float mScale;
	float mAngle;
	float mX, mY, mZ;
	short mState = 5;

	float mSpeed = 7.f;

	float mWalkTime, mTurnTime, mRunTime, mJumpTime, mIdleTime = 0.f;

	void Update(float dt);
};

struct ModelState
{
	XMFLOAT3 mDir = { 1,0,0 };
	std::string mClipName = "";
};

struct ModelInst
{
	Model* mModel = nullptr;
	float mTimePos = 0.0f;
	std::string mClipName = "";
	XMFLOAT4X4 mWorld;
	std::vector<XMFLOAT4X4> mFinalTransforms = {};

	float mSpeed = 7.0f;
	float mScale;
	XMFLOAT3 mDir;
	XMFLOAT3 mPos;

	Camera* mCam;
	float mBackTimePos = 0.0f;
	std::queue<ModelState> mState;

	void Update(float dt);
};