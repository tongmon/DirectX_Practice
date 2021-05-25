#include "ModelAnimation.h"

ModelAnimation::ModelAnimation(ModelLoader::LoadedAnimation& Anim)
{
	XMStoreFloat4x4(&mGlobalInvTrans, Anim.GlobalInvTrans);
	mDuration = Anim.AnimDuration;
	mRootBoneIndex = Anim.RootBoneIndex;
	mParent = Anim.ParentIndex;
	mBoneIndex = Anim.BoneIndex;
	mNodeMat.resize(Anim.nodeMatrix.size());
	int i = 0;
	for (auto& M : Anim.nodeMatrix)
		XMStoreFloat4x4(&mNodeMat[i++], M);
	mKeyFrameIndex = Anim.KeyFrameIndex;	
	mKeyFrameData = Anim.KeyFrames;
	mParentTrans.resize(mNodeMat.size());
}

ModelAnimation::~ModelAnimation()
{

}

std::pair<int, float> ModelAnimation::GetTimeFrac(std::vector<float> times, float& ft)
{
	if (ft <= times[0])
		return { 0, -1 };
	if (times.back() <= ft)
		return { times.size() - 1, -1 };
	int Ind = std::lower_bound(times.begin(), times.end(), ft) - times.begin();
	float frac = (ft - times[Ind - 1]) / (times[Ind] - times[Ind - 1]);
	return { Ind, frac };
}

void ModelAnimation::GetFinalTransfrom(float& ft, std::vector<XMFLOAT4X4> &boneOffset, std::vector<XMFLOAT4X4> &fin)
{
	XMMATRIX nodeMat, globalTrans, globalInvTrans, parentTrans, bOffset;
	globalInvTrans = XMLoadFloat4x4(&mGlobalInvTrans);
	XMVECTOR pos, pos_1, pos_2, scale, scale_1, scale_2, rotation, rotation_1, rotation_2, zero;
	zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	std::pair<int, float> indAndfrac;

	for (int i = 0; i < mNodeMat.size(); i++)
	{
		if (mParent[i] == -1)
			parentTrans = XMMatrixIdentity();
		else
			parentTrans = XMLoadFloat4x4(&mParentTrans[mParent[i]]);
		nodeMat = XMLoadFloat4x4(&mNodeMat[i]);
		if (mKeyFrameIndex[i] != -1) {
			ModelLoader::LoadedKeyFrame& KF = mKeyFrameData[mKeyFrameIndex[i]];
			
			indAndfrac = GetTimeFrac(KF.PosTime, ft);
			if (indAndfrac.second == -1)
				pos = XMLoadFloat3(&KF.Pos[indAndfrac.first]);
			else {
				pos_1 = XMLoadFloat3(&KF.Pos[indAndfrac.first - 1]);
				pos_2 = XMLoadFloat3(&KF.Pos[indAndfrac.first]);
				pos = XMVectorLerp(pos_1, pos_2, indAndfrac.second);
			}

			indAndfrac = GetTimeFrac(KF.ScaleTime, ft);
			if (indAndfrac.second == -1)
				scale = XMLoadFloat3(&KF.Scale[indAndfrac.first]);
			else {
				scale_1 = XMLoadFloat3(&KF.Scale[indAndfrac.first - 1]);
				scale_2 = XMLoadFloat3(&KF.Scale[indAndfrac.first]);
				scale = XMVectorLerp(scale_1, scale_2, indAndfrac.second);
			}

			indAndfrac = GetTimeFrac(KF.QuatTime, ft);
			if (indAndfrac.second == -1)
				rotation = XMLoadFloat4(&KF.Quat[indAndfrac.first]);
			else {
				rotation_1 = XMLoadFloat4(&KF.Quat[indAndfrac.first - 1]);
				rotation_2 = XMLoadFloat4(&KF.Quat[indAndfrac.first]);
				rotation = XMQuaternionSlerp(rotation_1, rotation_2, indAndfrac.second);
			}

			// 루트 모션 할거면 사용
			if (i == mRootBoneIndex) {
				XMFLOAT3 t;
				XMStoreFloat3(&t, pos);
				t.x = t.z = 0;
				pos = XMLoadFloat3(&t);
			}

			nodeMat = XMMatrixAffineTransformation(scale, zero, rotation, pos);
		}

		globalTrans = nodeMat * parentTrans;
		XMStoreFloat4x4(&mParentTrans[i], globalTrans);

		if (mBoneIndex[i] != -1) {
			bOffset = XMLoadFloat4x4(&boneOffset[mBoneIndex[i]]);
			XMStoreFloat4x4(&fin[mBoneIndex[i]], bOffset * globalTrans * globalInvTrans);
		}
	}
}

void ModelAnimation::InterMotion(float ft, float preTime, float dur, std::vector<XMFLOAT4X4>& boneOffset, ModelAnimation* preAnim, std::vector<XMFLOAT4X4>& fin)
{
	if (ft > dur) return;
	
	XMMATRIX nodeMat, globalTrans, globalInvTrans, parentTrans, bOffset;
	globalInvTrans = XMLoadFloat4x4(&mGlobalInvTrans);
	XMVECTOR pos, pos_1, pos_2, scale, scale_1, scale_2, rotation, rotation_1, rotation_2, zero;
	zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	std::pair<int, float> indAndfrac;

	// 예전 애니메이션
	std::vector<ModelLoader::LoadedKeyFrame> &preKF = preAnim->GetKeyFrame();
	std::vector<int>& preKFInd = preAnim->GetKeyFrameIndex();

	for (int i = 0; i < mNodeMat.size(); i++)
	{
		if (mParent[i] == -1)
			parentTrans = XMMatrixIdentity();
		else
			parentTrans = XMLoadFloat4x4(&mParentTrans[mParent[i]]);
		nodeMat = XMLoadFloat4x4(&mNodeMat[i]);

		if (mKeyFrameIndex[i] != -1 && preKFInd[i] != -1) {
			ModelLoader::LoadedKeyFrame& KF = mKeyFrameData[mKeyFrameIndex[i]];
			ModelLoader::LoadedKeyFrame& pKF = preKF[preKFInd[i]];

			indAndfrac = GetTimeFrac(KF.PosTime, preTime);
			pos_1 = XMLoadFloat3(&pKF.Pos[indAndfrac.first]);
			pos_2 = XMLoadFloat3(&KF.Pos[0]);
			pos = XMVectorLerp(pos_1, pos_2, ft / dur);

			indAndfrac = GetTimeFrac(KF.ScaleTime, preTime);
			scale_1 = XMLoadFloat3(&pKF.Scale[indAndfrac.first]);
			scale_2 = XMLoadFloat3(&KF.Scale[0]);
			scale = XMVectorLerp(scale_1, scale_2, ft / dur);

			indAndfrac = GetTimeFrac(KF.QuatTime, preTime);
			rotation_1 = XMLoadFloat4(&pKF.Quat[indAndfrac.first]);
			rotation_2 = XMLoadFloat4(&KF.Quat[0]);
			rotation = XMVectorLerp(rotation_1, rotation_2, ft / dur);
		}
		else if (mKeyFrameIndex[i] != -1) {
			ModelLoader::LoadedKeyFrame& KF = mKeyFrameData[mKeyFrameIndex[i]];
			XMMatrixDecompose(&scale_1, &rotation_1, &pos_1, nodeMat);

			pos_2 = XMLoadFloat3(&KF.Pos[0]);
			pos = XMVectorLerp(pos_1, pos_2, ft / dur);

			scale_2 = XMLoadFloat3(&KF.Scale[0]);
			scale = XMVectorLerp(scale_1, scale_2, ft / dur);

			rotation_2 = XMLoadFloat4(&KF.Quat[0]);
			rotation = XMQuaternionSlerp(rotation_1, rotation_2, ft / dur);
		}
		else if (preKFInd[i] != -1) {
			ModelLoader::LoadedKeyFrame& KF = preKF[preKFInd[i]];
			XMMatrixDecompose(&scale_2, &rotation_2, &pos_2, nodeMat);

			indAndfrac = GetTimeFrac(KF.PosTime, preTime);
			pos_1 = XMLoadFloat3(&KF.Pos[indAndfrac.first]);
			pos = XMVectorLerp(pos_1, pos_2, ft / dur);

			indAndfrac = GetTimeFrac(KF.ScaleTime, preTime);
			scale_1 = XMLoadFloat3(&KF.Scale[indAndfrac.first]);
			scale = XMVectorLerp(scale_1, scale_2, ft / dur);

			indAndfrac = GetTimeFrac(KF.QuatTime, preTime);
			rotation_1 = XMLoadFloat4(&KF.Quat[indAndfrac.first]);
			rotation = XMQuaternionSlerp(rotation_1, rotation_2, ft / dur);
		}

		if (preKFInd[i] != -1 || mKeyFrameIndex[i] != -1) {
			if (i == mRootBoneIndex) {
				XMFLOAT3 t;
				XMStoreFloat3(&t, pos);
				t.x = t.z = 0;
				pos = XMLoadFloat3(&t);
			}
			nodeMat = XMMatrixAffineTransformation(scale, zero, rotation, pos);
		}

		globalTrans = nodeMat * parentTrans;
		XMStoreFloat4x4(&mParentTrans[i], globalTrans);

		if (mBoneIndex[i] != -1) {
			bOffset = XMLoadFloat4x4(&boneOffset[mBoneIndex[i]]);
			XMStoreFloat4x4(&fin[mBoneIndex[i]], bOffset * globalTrans * globalInvTrans);
		}
	}
}