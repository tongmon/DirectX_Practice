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

	float GetDuration() { return mDuration; }
	std::pair<int, float> GetTimeFrac(std::vector<float> times, float& ft);
	void GetFinalTransfrom(float& ft, std::vector<XMFLOAT4X4>& boneOffset, std::vector<XMFLOAT4X4>& fin);
};

