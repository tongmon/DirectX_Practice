#pragma once

#include <goh.h>

#include "ModelLoader.h"

using namespace DirectX;

class ModelAnimation
{
private:
	XMFLOAT4X4 mGlobalInvTrans;
	int mRootBoneIndex;
	float mDuration;
	
	std::vector<int> mParent;
	std::vector<int> mBoneIndex;
	std::vector<int> mKeyFrameIndex;
	std::vector<XMFLOAT4X4> mNodeMat;

	// 키 프레임 데이터
	std::vector<ModelLoader::LoadedKeyFrame> mKeyFrameData;

	// 여태까지 읽어온 행렬곱 저장소
	std::vector<XMFLOAT4X4> mParentTrans;

public:
	ModelAnimation(ModelLoader::LoadedAnimation &Anim);
	~ModelAnimation();

	std::vector<ModelLoader::LoadedKeyFrame>& GetKeyFrame() { return mKeyFrameData; }
	float GetDuration() { return mDuration; }
	XMFLOAT4X4 &GetGInvTrans() { return mGlobalInvTrans; }
	std::vector<XMFLOAT4X4>& GetNodeMat() { return mNodeMat; }
	std::vector<int>& GetKeyFrameIndex() { return mKeyFrameIndex; }
	std::pair<int, float> GetTimeFrac(std::vector<float> times, float& ft);
	void GetFinalTransfrom(float& ft, std::vector<XMFLOAT4X4>& boneOffset, std::vector<XMFLOAT4X4>& fin);
	void InterMotion(float ft, float preTime, float dur, std::vector<XMFLOAT4X4>& boneOffset, ModelAnimation* preAnim, std::vector<XMFLOAT4X4>& fin);
};

