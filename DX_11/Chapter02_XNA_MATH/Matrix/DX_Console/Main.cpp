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

ostream& operator<<(ostream& os, FXMVECTOR v)
{
	XMFLOAT4 dest;
	XMStoreFloat4(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ", " << dest.w << ")";
	return os;
}

// XMMATRIX를 매개변수로 전달하려 할 때는 CXMMATRIX를 사용해 전달해야 한다.
// 특수한 경우에 FXMMATRIX를 사용해 넘기기도 하는데 밑과 같은 경우이다.
// 1. 일단은 변수형이 XMMATRIX 이와 같아야 한다.
// 2. XMMATRIX 변수 앞에 XMVECTOR가 3개 이상 존재하지 않을 것
// 3. XMMATRIX 변수 뒤에 2개 이상의 float나 double, XMVECTOR가 존재하지 않을 것
// 생성자의 경우 그냥 CXMMATRIX를 사용한다고 보면 된다.

ostream& operator<<(ostream& os, CXMMATRIX m)
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			os << m(i, j) << "\t";
		}
		os << endl;
	}
	return os;
}

int main()
{
	// XNA MATH 호환성 검사
	if (!XMVerifyCPUSupport())
	{
		cout << "xna math not supported" << endl;
		return 0;
	}
	cout << "XNA MATH 행렬 함수 사용" << endl;
	
	XMMATRIX A(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 4.0f, 0.0f,
		1.0f, 2.0f, 3.0f, 1.0f); // 생성자를 통해 무난히 행렬 생성

	XMMATRIX B = XMMatrixIdentity(); // 단위 행렬 도출

	XMMATRIX C = A * B; // 곱셈은 편하게 정의되어 있다.

	XMMATRIX D = XMMatrixTranspose(A); // 전치 행렬 도출

	// 행렬식 도출, 보통 역행렬 구할 때 많이 사용된다.
	XMVECTOR det = XMMatrixDeterminant(A); // 계산 값은 하나지만 벡터로 반환되어 모든 값이 행렬식의 값인 벡터가 도출된다.
	
	XMMATRIX E = XMMatrixInverse(&det, A); // 역행렬 도출, 크레이머 공식으로 구한다.

	XMMATRIX F = A * E; // 단위행렬이 튀어나와야 정상

	cout << "A = " << endl << A << endl;
	cout << "B = " << endl << B << endl;
	cout << "C = A*B = " << endl << C << endl;
	cout << "D = transpose(A) = " << endl << D << endl;
	cout << "det = determinant(A) = " << det << endl << endl;
	cout << "E = inverse(A) = " << endl << E << endl;
	cout << "F = A*E = " << endl << F << endl;

	return 0;
}