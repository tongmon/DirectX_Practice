#ifndef SKINNEDDATA_H
#define SKINNEDDATA_H

#include "d3dUtil.h"
#include <map>

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
	Keyframe();
	~Keyframe();

    float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.  For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the time.  
///
/// We assume an animation always has two keyframes.
/// 뼈대 한 개에 대한 애니메이션
///</summary>
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

    void Interpolate(float t, XMFLOAT4X4& M)const;

	std::vector<Keyframe> Keyframes; 	

};

///<summary>
/// 걷기, 뛰기, 공격, 방어 같은 개별 애니메이션 클립을 대표한다.
/// 하나의 AnimationClip은 애니메이션 클립을 구성하는
/// 뼈대당 하나씩의 BoneAnimation 인스턴스를 담는다. 
///</summary>
struct AnimationClip
{
	// 클립의 모든 뼈대 중 가장 이른 시작 시간을 돌려준다.
	float GetClipStartTime()const;
	// 클립의 모든 뼈대 중 가장 늦은 종료 시간을 돌려준다.
	float GetClipEndTime()const;
	// 각 클립 시각 사이를 보간으로 획득하기 위한 함수
    void Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms)const;

	// 뼈대별 애니메이션들.
    std::vector<BoneAnimation> BoneAnimations; 	
};

class SkinnedData
{
public:

	UINT BoneCount()const;

	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;

	void Set(
		std::vector<int>& boneHierarchy, 
		std::vector<XMFLOAT4X4>& boneOffsets,
		std::map<std::string, AnimationClip>& animations);

	// 실제 프로젝트의 경우, 만일 같은 clipName과 timePos로
	// 이 메서드를 여러 번 호출할 가능성이 크다면 일종의 캐싱
	// 시스템을 도입하는 것이 바람직 할 것이다.
    void GetFinalTransforms(const std::string& clipName, float timePos, 
		 std::vector<XMFLOAT4X4>& finalTransforms)const;

private:
	// 일차원 배열로 트리 구조 구축
    // i번 뼈대의 부모의 색인을 담는다. i번 뼈대는
	// 애니메이션 클립의 i번째 BoneAnimation 인스턴스에 대응된다.
	std::vector<int> mBoneHierarchy;
	// i번째 뼈대 오프셋 변환
	std::vector<XMFLOAT4X4> mBoneOffsets;
   
	std::map<std::string, AnimationClip> mAnimations;
};
 
#endif // SKINNEDDATA_H