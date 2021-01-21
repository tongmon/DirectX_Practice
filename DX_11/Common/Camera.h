//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Simple first person style camera class that lets the viewer explore the 3D scene.
//   -It keeps track of the camera coordinate system relative to the world space
//    so that the view matrix can be constructed.  
//   -It keeps track of the viewing frustum of the camera so that the projection
//    matrix can be obtained.
//***************************************************************************************

#ifndef CAMERA_H
#define CAMERA_H

#include "d3dUtil.h"

class Camera
{
public:
	Camera();
	~Camera();

	// 세계 공간 카메라 위치를 조회 또는 설정
	XMVECTOR GetPositionXM()const;
	XMFLOAT3 GetPosition()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& v);
	
	// 카메라 기저벡터들을 돌려준다.
	XMVECTOR GetRightXM()const;
	XMFLOAT3 GetRight()const;
	XMVECTOR GetUpXM()const;
	XMFLOAT3 GetUp()const;
	XMVECTOR GetLookXM()const;
	XMFLOAT3 GetLook()const;

	// 절두체 속성들을 돌려준다.
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// 시야 공간 기준의 가까운/먼 평면 크기를 돌려준다.
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;
	
	// 절두체를 형성한다.
	void SetLens(float fovY, float aspect, float zn, float zf);

	// 카메라 위치와 시선 및 위쪽 방향으로 카메라 공간을 설정한다.
	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	// 시야 행렬과 투영행렬을 돌려준다.
	XMMATRIX View()const;
	XMMATRIX Proj()const;
	XMMATRIX ViewProj()const;

	// 카메라를 거리 d만큼 횡이동, 축이동한다.
	void Strafe(float d);
	void Walk(float d);

	// 카메라를 회전한다.
	void Pitch(float angle);
	void RotateY(float angle);
	void Roll(float angle);

	// 매 프레임마다, 카메라 위치나 방향을 수정한 후
	// 이 메서드를 호출해서 시야 행렬을 재구축한다.
	void UpdateViewMatrix();

private:

	// 세계 공간 기준의 카메라 좌표계
	XMFLOAT3 mPosition; // 시야 공간 원점
	XMFLOAT3 mRight; // 시야 공간 x 축
	XMFLOAT3 mUp; // 시야 공간 y 축
	XMFLOAT3 mLook; // 시야 공간 z 축

	// 절두체 속성들
	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;

	// 시야 행렬과 투영 행렬
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
};

#endif // CAMERA_H