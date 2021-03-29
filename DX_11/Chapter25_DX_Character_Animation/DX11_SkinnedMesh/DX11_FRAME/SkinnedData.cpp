

#include "SkinnedData.h"
#include "MeshGeometry.h"

Keyframe::Keyframe()
	: TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Keyframe::~Keyframe()
{
}
 
float BoneAnimation::GetStartTime()const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	float f = Keyframes.back().TimePos;

	return f;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
{
	if( t <= Keyframes.front().TimePos )
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if( t >= Keyframes.back().TimePos )
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for(UINT i = 0; i < Keyframes.size()-1; ++i)
		{
			if( t >= Keyframes[i].TimePos && t <= Keyframes[i+1].TimePos )
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i+1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i+1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i+1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i+1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime()const
{
	// Find smallest start time over all bones in this clip.
	float t = MathHelper::Infinity;
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	// Find largest end time over all bones in this clip.
	float t = 0.0f;
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}

void AnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms)const
{
	for(UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

float SkinnedData::GetClipStartTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipEndTime();
}

UINT SkinnedData::BoneCount()const
{
	return mBoneHierarchy.size();
}

void SkinnedData::Set(std::vector<int>& boneHierarchy, 
		              std::vector<XMFLOAT4X4>& boneOffsets,
		              std::map<std::string, AnimationClip>& animations)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets   = boneOffsets;
	mAnimations    = animations;
}
 
void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos,  std::vector<XMFLOAT4X4>& finalTransforms)const
{
	UINT numBones = mBoneOffsets.size();

	std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	// 이 클립의 모든 뼈대를 주어진 시간(순간)에 맞게 보간한다.
	auto clip = mAnimations.find(clipName);
	clip->second.Interpolate(timePos, toParentTransforms);

	//
	// 골격 계통구조를 훑으면서 모든 뼈대를 뿌리 공간으로 변환한다.
	//

	std::vector<XMFLOAT4X4> toRootTransforms(numBones);

	// 뿌리 뼈대의 색인은 0이다. 뿌리 뼈대에는 부모가 없으므로
	// 뿌리 변환은 그냥 뼈대 국소 변환이다.
	toRootTransforms[0] = toParentTransforms[0];

	// 이제 자식 뼈대들의 뿌리 변환을 구한다.
	// 일단 각 배열에는 부모 인덱스, 해당 좌표계를 부모 좌표계로 바꾸어주는 행렬
	// 이렇게 두가지 정보가 담겨있다.
	// 루트는 당연히 국소 -> 세계를 해주는 변환 행렬이 들어있을 것이고 부모 인덱스는 -1이거나 따로 없을 것이다.
	// 루트 밑의 노드들이 어떻게 진행되는 지를 보면 일단 자신의 좌표계를 부모 국소 좌표계로 바꾸는 행렬을 얻어야
	// 한다. 근데 이것은 배열에 해당 인덱스로 가면 존재한다.
	// 그러면 부모 국소 좌표계로 이동을 했으면 이 좌표계를 세계 좌표계까지 끌고 가는 행렬이 있어야 한다.
	// 이 부모 국소 좌표계 -> 세계 좌표계를 해주는 행렬은 밑의 층으로 갈 수록 생성되어 저장된다.
	// 이게 무슨 말이냐면 루트 바로 밑의 자식 노드를 보면 자신의 국소 좌표계에서 루트 노드 국소 좌표계로 바꾸는 행렬을 얻고
	// 자신의 부모인 루트 노드가 가진 RootTransfrom 행렬 즉, 국소 -> 세계 행렬인 변환 행렬을 얻어
	// 곱하게되는데 이를 곱한 녀석을 다시 해당 인덱스의 RootTransfrom 에 저장하여 지금 현재 노드 밑의 자식 노드가
	// 현재 노드 국소 좌표계 -> 세계 좌표계 로 가는 행렬을 얻을 수 있게 마련한다.
	// 현재 노드 밑의 자식 노드는 부모 국소 좌표계로 이동을 했고 이후에 세계 좌표계로까지 이동을 해야하기 때문이다.
	// 이러한 방식으로 뼈대 개수만큼의 루프 순회로 계산이 끝난다.
	// 물론 이게 동작하려면 무조건 부모가 앞에서 미리 계산 되어야하기에 정렬이 된 상태여야 한다.
	for(UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	// 뿌리 뼈에서 얼마나 떨어져 있는지 정보가 담긴 
	// 뼈대 오프셋 변환을 앞에서 곱해서 최종 변환을 구한다.
	for(UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot));
	}
}