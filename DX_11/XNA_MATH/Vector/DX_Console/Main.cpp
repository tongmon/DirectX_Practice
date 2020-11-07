// 이 순서를 지켜야한다... 안그러면 윈10에서 다렉 구동시에 오류난다.
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
using namespace DirectX;

#include <iostream>
using namespace std;

#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"dxguid.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"DxErr.lib")
#pragma comment (lib,"d3dxof.lib")
#pragma comment (lib,"winmm.lib")

// XMVECTOR를 매개변수로 넘길 경우 주의해야 할 점은
// 함수의 처음 세 XMVECTOR 매개변수는 반드시 FXMVECTOR를 사용한다.
// 나머지는 CXMVECTOR를 사용한다.

ostream& operator<<(ostream& os, FXMVECTOR v) // 벡터 출력 연산자 재정의
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ")";
	return os;
}

int main()
{
	cout.setf(ios_base::boolalpha);

	// XNA MATH 호환성 검사
	if (!XMVerifyCPUSupport())
	{
		cout << "xna math not supported" << endl;
		return 0;
	}
	cout << "XNA MATH 벡터 사용" << endl;
	// 벡터 설정
	XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 0.0f);
	XMVECTOR w = XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

	// 벡터 합
	XMVECTOR a = u + v;

	// 벡터 차
	XMVECTOR b = u - v;

	// 벡터와 스칼라 곱도 연산자 오퍼레이더로 정의되어 있다.
	XMVECTOR c = 10.0f * u;

	// 벡터의 크기
	XMVECTOR L = XMVector3Length(u);

	// 단위 벡터로 변환, 뱡향은 같지만 크기가 1
	XMVECTOR d = XMVector3Normalize(u);

	// 내적 연산, 두 벡터 사잇각을 알아낼 경우 유용하다.
	XMVECTOR s = XMVector3Dot(u, v);

	// 외적 연산, 두 벡터와 수직인 법선 벡터를 추출하는데 사용
	XMVECTOR e = XMVector3Cross(u, v);

	// proj_n(w) = 내적을 이용하여 한쪽으로 투영된 벡터 도출
	// perp_n(w) = 법선 벡터 도출
	XMVECTOR projW;
	XMVECTOR perpW;
	XMVector3ComponentsFromNormal(&projW, &perpW, w, n); // 즉 벡터 2개를 던져주면 투영 벡터, 법선 벡터 도출

	// projW + perpW == w 인가? 를 알려준다.
	bool equal = XMVector3Equal(projW + perpW, w) != 0;
	bool notEqual = XMVector3NotEqual(projW + perpW, w) != 0;

	// projW 와 perpW 의 각도를 구해준다. 이 둘의 각은 90도여야 한다.
	XMVECTOR angleVec = XMVector3AngleBetweenVectors(projW, perpW);
	float angleRadians = XMVectorGetX(angleVec);
	float angleDegrees = XMConvertToDegrees(angleRadians);

	cout << "u                   = " << u << endl;
	cout << "v                   = " << v << endl;
	cout << "w                   = " << w << endl;
	cout << "n                   = " << n << endl;
	cout << "a = u + v           = " << a << endl;
	cout << "b = u - v           = " << b << endl;
	cout << "c = 10 * u          = " << c << endl;
	cout << "d = u / ||u||       = " << d << endl;
	cout << "e = u x v           = " << e << endl;
	cout << "L  = ||u||          = " << L << endl;
	cout << "s = u.v             = " << s << endl;
	cout << "projW               = " << projW << endl;
	cout << "perpW               = " << perpW << endl;
	cout << "projW + perpW == w  = " << equal << endl;
	cout << "projW + perpW != w  = " << notEqual << endl;
	cout << "angle               = " << angleDegrees << "\n\n\n";
	
	cout << "부동소수점 오차에 따라 1의 크기가 도출되어야 하는 것이 연산을 거치면서 1이 나오지 않는다." << endl;
	
	XMVECTOR U = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f); // 벡터 설정
	XMVECTOR N = XMVector3Normalize(U); // 단위 벡터로 변환

	float LU = XMVectorGetX(XMVector3Length(N)); // 단위 벡터의 크기를 받는다.

	// 단위 벡터의 크기는 1이다? 하지만 과연 1이 나올까?
	cout << "단위벡터 크기: " << LU << endl; // 여기선 1로 보일 것이다.
	if (LU == 1.0f)
		cout << "Length 1" << endl;
	else
		cout << "Length not 1" << endl; // 콘솔창은 1이라고 보이지만 결과는 1이 아니라고 말한다.

	// 1에 어떤 지수 연산을 해도 1로 동일해야 할 것이다. 하지만 결과는?
	float powLU = powf(LU, 1.0e6f); // 좀 더 정밀하게 따져보자
	cout << "LU^(10^6) = " << powLU << endl; // 1이 아니라는 것이 도출되었다.

	/*
	// 밑과 같은 다양한 벡터 함수들이 마련되어 있으니 입맛대로 골라 쓰면 된다.
	XMVECTOR p = XMVectorSet(2.0f, 2.0f, 1.0f, 0.0f);
	XMVECTOR q = XMVectorSet(2.0f, -0.5f, 0.5f, 0.1f);
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 4.0f, 8.0f);
	XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 2.5f);
	XMVECTOR w = XMVectorSet(0.0f, XM_PIDIV4, XM_PIDIV2, XM_PI);

	cout << "XMVectorAbs(v)                 = " << XMVectorAbs(v) << endl;
	cout << "XMVectorCos(w)                 = " << XMVectorCos(w) << endl;
	cout << "XMVectorLog(u)                 = " << XMVectorLog(u) << endl;
	cout << "XMVectorExp(p)                 = " << XMVectorExp(p) << endl;
	
	cout << "XMVectorPow(u, p)              = " << XMVectorPow(u, p) << endl;
	cout << "XMVectorSqrt(u)                = " << XMVectorSqrt(u) << endl;
	
	cout << "XMVectorSwizzle(u, 2, 2, 1, 3) = " 
		<< XMVectorSwizzle(u, 2, 2, 1, 3) << endl;
	cout << "XMVectorSwizzle(u, 2, 1, 0, 3) = " 
		<< XMVectorSwizzle(u, 2, 1, 0, 3) << endl;

	cout << "XMVectorMultiply(u, v)         = " << XMVectorMultiply(u, v) << endl;
	cout << "XMVectorSaturate(q)            = " << XMVectorSaturate(q) << endl;
	cout << "XMVectorMin(p, v               = " << XMVectorMin(p, v) << endl;
	cout << "XMVectorMax(p, v)              = " << XMVectorMax(p, v) << endl;
	*/

	return 0;
}