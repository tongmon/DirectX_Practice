#include "Model.h"

Model::Model(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename)
{
	std::vector<ModelLoader::MLMaterial> mats;
	std::vector<ModelLoader::LoadedVertex> verts;
	std::wstring MapName;

	ModelLoader::ReadModel(modelFilename, verts, Indices, mSubsets, mats, mBoneOffset);

	mVertices.reserve(verts.size());
	for (const auto &v : verts) {
		Vertex::PosNormalTexTanSkinned temp;
		temp.Pos = v.Pos;
		temp.Normal = v.Normal;
		temp.Tex = v.texcoord;
		temp.Weights = { v.BoneWeights.x, v.BoneWeights.y, v.BoneWeights.z };
		temp.TangentU = v.TangentU;
		temp.BoneIndices[0] = v.BoneID.x; temp.BoneIndices[1] = v.BoneID.y;
		temp.BoneIndices[2] = v.BoneID.z; temp.BoneIndices[3] = v.BoneID.w;
		mVertices.push_back(temp);
	}

	mMeshes.SetVertices(device, &mVertices[0], mVertices.size());
	mMeshes.SetIndices(device, &Indices[0], Indices.size());
	mMeshes.SetSubsetTable(mSubsets);

	mSubsetCount = mats.size();

	for (UINT i = 0; i < mSubsetCount; ++i)
	{
		Material temp;
		temp.Ambient = mats[i].Ambient;
		temp.Diffuse = mats[i].Diffuse;
		temp.Reflect = mats[i].Reflect;
		temp.Specular = mats[i].Specular;

		mMat.push_back(temp);

		std::copy(mats[i].DiffuseMapName.begin(), mats[i].DiffuseMapName.end(), std::back_inserter(MapName));
		ID3D11ShaderResourceView* diffuseMapSRV = texMgr.CreateTexture(MapName);
		mDiffuseMapSRV.push_back(diffuseMapSRV);
		MapName.clear();

		std::copy(mats[i].NormalMapName.begin(), mats[i].NormalMapName.end(), std::back_inserter(MapName));
		ID3D11ShaderResourceView* normalMapSRV = texMgr.CreateTexture(MapName);
		mNormalMapSRV.push_back(normalMapSRV);
		MapName.clear();

		std::copy(mats[i].SpecularMapName.begin(), mats[i].SpecularMapName.end(), std::back_inserter(MapName));
		ID3D11ShaderResourceView* specMapSRV = texMgr.CreateTexture(MapName);
		mSpecularMapSRV.push_back(specMapSRV);
		MapName.clear();
	}
}

Model::~Model()
{
	for (auto& Anim : mAnimData) {
		delete Anim.second;
	}
}

void Model::AddAnimation(const std::string& filename, const std::string& animName)
{
	ModelLoader::LoadedAnimation temp;
	ModelLoader::ReadAnimation(filename, temp);
	ModelAnimation* modelAnim = new ModelAnimation(temp);
	mAnimData[animName] = modelAnim;
}

void ModelInstance::Update(float dt)
{
	float ft = mTimePos * 1000;
	mModel->mAnimData[mClipName]->GetFinalTransfrom(ft, mModel->mBoneOffset, mFinalTransforms);
	
	mWalkTime = mState == 1 ? mWalkTime + dt : 0.f;
	mRunTime = mState == 2 ? mRunTime + dt : 0.f;
	mJumpTime = mState == 3 ? mJumpTime + dt : 0.f;
	mTurnTime = mState == 4 ? mTurnTime + dt : 0.f;
	mIdleTime = mState == 5 ? mIdleTime + dt : 0.f;

	if (mState == 1)
		mTimePos = mWalkTime;
	else if (mState == 2)
		mTimePos = mRunTime;
	else if (mState == 3)
		mTimePos = mJumpTime;
	else if (mState == 4)
		mTimePos = mTurnTime;
	else
		mTimePos = mIdleTime;

	if (mTimePos > mModel->mAnimData[mClipName]->GetDuration() / 1000.f) {
		if (mState == 1)
			mWalkTime = 0.f;
		else if (mState == 2)
			mRunTime = 0.f;
		else if (mState == 3)
			mJumpTime = 0.f;
		else if (mState == 4)
			mTurnTime = 0.f;
		else
			mIdleTime = 0.f;
	}

	mModel->mVertices;
}

const float RadSpeed = 0.2;

void ModelInst::Update(float dt)
{
	ModelState& Front = mState.front();

	float ft = mTimePos * 1000;
	mModel->mAnimData[mClipName]->GetFinalTransfrom(ft, mModel->mBoneOffset, mFinalTransforms);

	XMVECTOR model_dir = XMLoadFloat3(&mDir);
	XMFLOAT3 cL = Front.mDir; cL.y = 0;
	XMVECTOR look = XMLoadFloat3(&cL);
	XMVECTOR Angle = XMVector3AngleBetweenVectors(model_dir, look);
	XMVECTOR Cross = XMVector3Cross(look, model_dir);
	XMVECTOR Dot = XMVector3Dot(XMVectorSet(0, 1, 0, 0), Cross);
	if (XMVectorGetX(Dot) > 0)
		Angle *= -1;

	float Radian = XMVectorGetX(Angle);
	if (Radian < 0 && Radian + RadSpeed < 0) {
		Radian = -RadSpeed;
	}
	else if (Radian > 0 && Radian - RadSpeed >= 0) {
		Radian = RadSpeed;
	}
	
	XMVECTOR DirVec = XMLoadFloat3(&mDir);
	DirVec = XMVector3Rotate(DirVec, XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), Radian));
	DirVec = XMVector3Normalize(DirVec);
	XMStoreFloat3(&mDir, DirVec);

	mPos.x += mDir.x * mSpeed * dt;
	mPos.y += mDir.y * mSpeed * dt;
	mPos.z += mDir.z * mSpeed * dt;

	XMFLOAT3 S = { mScale,mScale,mScale };

	Angle = XMVector3AngleBetweenVectors(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), DirVec);
	Cross = XMVector3Cross(DirVec, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
	Dot = XMVector3Dot(XMVectorSet(0, 1, 0, 0), Cross);
	if (XMVectorGetX(Dot) > 0)
		Angle *= -1;

	XMMATRIX Result = XMMatrixAffineTransformation(XMLoadFloat3(&S), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorGetX(Angle)), XMLoadFloat3(&mPos));
	XMStoreFloat4x4(&mWorld, Result);

	mBackTimePos = mTimePos;

	if (mClipName == Front.mClipName && mTimePos <= mModel->mAnimData[mClipName]->GetDuration() / 1000.f) {
		mTimePos += dt;
	}
	else {
		mClipName = Front.mClipName;
		mTimePos = 0.f;
	}

	mState.pop();
}

/*
void ModelInst::Update(float dt)
{
	ModelState& Front = mState.front();

	float ft = mTimePos * 1000;
	mModel->mAnimData[mClipName]->GetFinalTransfrom(ft, mModel->mBoneOffset, mFinalTransforms);

	XMVECTOR model_dir = XMLoadFloat3(&mDir);
	XMFLOAT3 cL = Front.mDir; cL.y = 0;
	XMVECTOR look = XMLoadFloat3(&cL);
	XMVECTOR Angle = XMVector3AngleBetweenVectors(model_dir, look);
	XMVECTOR Cross = XMVector3Cross(look, model_dir);
	XMVECTOR Dot = XMVector3Dot(XMVectorSet(0, 1, 0, 0), Cross);
	if (XMVectorGetX(Dot) > 0)
		Angle *= -1;

	float Radian = XMVectorGetX(Angle);

	XMVECTOR DirVec = XMLoadFloat3(&mDir);
	DirVec = XMVector3Rotate(DirVec, XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), Radian));
	DirVec = XMVector3Normalize(DirVec);
	XMStoreFloat3(&mDir, DirVec);

	mPos.x += mDir.x * mSpeed * dt;
	mPos.y += mDir.y * mSpeed * dt;
	mPos.z += mDir.z * mSpeed * dt;

	XMFLOAT3 S = { mScale,mScale,mScale };

	Angle = XMVector3AngleBetweenVectors(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), DirVec);
	Cross = XMVector3Cross(DirVec, XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
	Dot = XMVector3Dot(XMVectorSet(0, 1, 0, 0), Cross);
	if (XMVectorGetX(Dot) > 0)
		Angle *= -1;

	XMMATRIX Result = XMMatrixAffineTransformation(XMLoadFloat3(&S), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
		XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorGetX(Angle)), XMLoadFloat3(&mPos));
	XMStoreFloat4x4(&mWorld, Result);

	mBackTimePos = mTimePos;

	if (mClipName == Front.mClipName && mTimePos <= mModel->mAnimData[mClipName]->GetDuration() / 1000.f) {
		mTimePos += dt;
	}
	else {
		mClipName = Front.mClipName;
		mTimePos = 0.f;
	}

	mState.pop();
}
*/