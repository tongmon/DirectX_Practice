#pragma once

#include <goh.h>

#include <Vertex.h>
#include <ModelMesh.h>
#include <TextureMgr.h>
#include <ModelAnimation.h>

class Model
{
public:
	Model(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename);
	~Model();

	void AddAnimation(const std::string& filename, const std::string& animname);

	UINT mSubsetCount;

	// ���͸��� ���� ������
	std::vector<Material> mMat;
	std::vector<ID3D11ShaderResourceView*> mDiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> mNormalMapSRV;
	std::vector<ID3D11ShaderResourceView*> mSpecularMapSRV;

	// CPU ��� ó�� ������ ���� ���� ���� ���ܵ�
	std::vector<Vertex::PosNormalTexTanSkinned> mVertices;
	std::vector<UINT> Indices;
	std::vector<ModelLoader::Subset> mSubsets;

	// �޽� ����, ����, �ε���, ������� ���
	ModelMesh mMeshes;

	// �� ������ ���
	std::vector<XMFLOAT4X4> mBoneOffset;

	// �ִϸ��̼� ���� ����
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
