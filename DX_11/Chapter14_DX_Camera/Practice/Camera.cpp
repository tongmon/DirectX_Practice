//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Camera.h"

Camera::Camera()
	: mPosition(0.0f, 0.0f, 0.0f), 
	  mRight(1.0f, 0.0f, 0.0f),
	  mUp(0.0f, 1.0f, 0.0f),
	  mLook(0.0f, 0.0f, 1.0f)
{
	SetLens(0.25f*MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
}

Camera::~Camera()
{
}

XMVECTOR Camera::GetPositionXM()const // 변환해서 값 반환
{
	return XMLoadFloat3(&mPosition);
}

XMFLOAT3 Camera::GetPosition()const // 변환해서 값 반환
{
	return mPosition;
}

void Camera::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
}

void Camera::SetPosition(const XMFLOAT3& v)
{
	mPosition = v;
}

XMVECTOR Camera::GetRightXM()const
{
	return XMLoadFloat3(&mRight);
}

XMFLOAT3 Camera::GetRight()const
{
	return mRight;
}

XMVECTOR Camera::GetUpXM()const
{
	return XMLoadFloat3(&mUp);
}

XMFLOAT3 Camera::GetUp()const
{
	return mUp;
}

XMVECTOR Camera::GetLookXM()const
{
	return XMLoadFloat3(&mLook);
}

XMFLOAT3 Camera::GetLook()const
{
	return mLook;
}

float Camera::GetNearZ()const
{
	return mNearZ;
}

float Camera::GetFarZ()const
{
	return mFarZ;
}

float Camera::GetAspect()const
{
	return mAspect;
}

float Camera::GetFovY()const
{
	return mFovY;
}

float Camera::GetFovX()const
{
	float halfWidth = 0.5f*GetNearWindowWidth(); // 가로의 절반을 이용해 삼각함수로
	return 2.0f*atan(halfWidth / mNearZ); // 가로 시야각을 구한다.
}

float Camera::GetNearWindowWidth()const
{
	return mAspect * mNearWindowHeight; // 구해진 높이로 종횡비를 따져서 가로도 구한다.
}

float Camera::GetNearWindowHeight()const
{
	return mNearWindowHeight;
}

float Camera::GetFarWindowWidth()const
{
	return mAspect * mFarWindowHeight;
}

float Camera::GetFarWindowHeight()const
{
	return mFarWindowHeight;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// 속성들을 설정만 해둔다.
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	// 수학적으로 그림 그려서 삼각함수로 유도하면 밑의 식이 나온다.
	mNearWindowHeight = 2.0f * mNearZ * tanf( 0.5f*mFovY ); // 가까운 절두체 크기, 상이 그리지는 곳
	mFarWindowHeight  = 2.0f * mFarZ * tanf( 0.5f*mFovY ); // 먼 곳, 즉 무대 끝의 절두체 크기

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&mPosition, pos);
	XMStoreFloat3(&mLook, L);
	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
}

void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

XMMATRIX Camera::View()const
{
	return XMLoadFloat4x4(&mView);
}

XMMATRIX Camera::Proj()const
{
	return XMLoadFloat4x4(&mProj);
}

XMMATRIX Camera::ViewProj()const
{
	return XMMatrixMultiply(View(), Proj());
}

void Camera::Strafe(float d) // 횡이동, 좌우
{
	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d); // (d,d,d) 벡터 반환
	XMVECTOR r = XMLoadFloat3(&mRight); // 좌,우로 이동하는 방향의 척도 벡터
	XMVECTOR p = XMLoadFloat3(&mPosition);
	// XMVectorMultiplyAdd 함수는 밑의 인자 기준으로 p + s * r을 반환한다.
	// 즉 현 위치에서 좌,우 뱡향으로 거리 d 만큼을 이동하는 행렬을 획득
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));
}

void Camera::Walk(float d) // 종이동, 앞뒤
{
	// mPosition += d*mLook
	XMVECTOR s = XMVectorReplicate(d); // 이동할 거리
	XMVECTOR l = XMLoadFloat3(&mLook); // 바라보는 방향으로 이동함
	XMVECTOR p = XMLoadFloat3(&mPosition);
	// 횡이동과 같은 원리로 방향만 바라보는 방향이 기준이 되는 것이다.
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));
}

void Camera::Pitch(float angle) // 고개 끄덕이는 경우 카메라 움직임
{
	// 상향 벡터와 시선 벡터를 오른쪽 벡터에 대해 회전한다.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle); // 좌우 방향 벡터가 축이 된다.

	XMStoreFloat3(&mUp,   XMVector3TransformNormal(XMLoadFloat3(&mUp), R)); // 상향 벡터 회전
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R)); // 시점 벡터 회전
}

void Camera::RotateY(float angle) // 고개 양방향 도리도리 하는 경우 카메라 움직임
{
	// 기저벡터들을 Y축에 대해 회전한다.
	// 벡터를 회전하는 것이라 그냥 0,1,0에 축이 붙어있고 회전한다고 보면 된다.
	// 회전시에 반지름에 대한 걱정이 없이 걍 방향만 회전된다.

	XMMATRIX R = XMMatrixRotationY(angle);

	// XMVector3TransformNormal 함수는 뒷 인자로 주어지는 변환 행렬을
	// 앞 인자에 곱하여 행렬 변환해주는 함수이다.
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
}

void Camera::Roll(float angle) // 고개 양방향 도리도리 하는 경우 카메라 움직임
{
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mLook), angle); // 좌우 방향 벡터가 축이 된다.

	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R)); // 상향 벡터 회전
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R)); // 시점 벡터 회전
}

void Camera::UpdateViewMatrix()
{
	XMVECTOR R = XMLoadFloat3(&mRight); // 좌우 벡터
	XMVECTOR U = XMLoadFloat3(&mUp); // 상향 벡터
	XMVECTOR L = XMLoadFloat3(&mLook); // 바라보는 방향 벡터
	XMVECTOR P = XMLoadFloat3(&mPosition); // 카메라 위치 벡터

	// 위의 벡터들을 정규 직교화 하는 작업
	// 정규 직교화 할 뿐만 아니라 새로 계산하여 새로 구한다.
	// 왜냐면 새로 계산 과정이 없이 계속 카메라를 움직여대면
	// 오차가 누적되어 카메라 각 방향 벡터들이 이상해진다.

	// 시선 벡터를 단위벡터로 만든다.
	L = XMVector3Normalize(L); // 정규화
	U = XMVector3Normalize(XMVector3Cross(L, R)); // 시점, 자우 벡터를 외적해서 상향벡터를 새로 만든다.

	// 보정된 좌우 벡터를 새로 계산한다.
	R = XMVector3Cross(U, L); // U, L 이 이미 모두 정규화되어 R은 정규화 할 필요없다.

	// 시야 행렬의 성분들을 채운다.
	// 시야에서 세계로 가는 행렬은 기존의 세계 변환 행렬인 W = RT이고
	// 이것의 역행렬이 세계에서 시야로 가는 시야 행렬이다.
	// 즉 시야행렬은 역 이동행렬 * 역 회전 행렬이 되는 것이다.
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
	XMStoreFloat3(&mLook, L);

	// 첫 열
	mView(0,0) = mRight.x; 
	mView(1,0) = mRight.y; 
	mView(2,0) = mRight.z; 
	mView(3,0) = x;

	// 두번째 열
	mView(0,1) = mUp.x;
	mView(1,1) = mUp.y;
	mView(2,1) = mUp.z;
	mView(3,1) = y;

	// 세번째 열
	mView(0,2) = mLook.x; 
	mView(1,2) = mLook.y; 
	mView(2,2) = mLook.z; 
	mView(3,2) = z;

	// 마지막 열
	mView(0,3) = 0.0f;
	mView(1,3) = 0.0f;
	mView(2,3) = 0.0f;
	mView(3,3) = 1.0f;
}


