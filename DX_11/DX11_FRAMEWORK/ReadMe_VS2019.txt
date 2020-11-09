Windows 10으로 업데이트가 되면서 기존의 DirectX의 헤더, 라이브러리 파일들은
Program Files(x86) / Windows Kit 에 소속되게 되었다.
따라서 아래와 같은 순서로 포함 디렉터리와 라이브러리 디렉터리를 설정해준다.

Header Setting
C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.18362.0\shared 
C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.18362.0\ucrt
C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.18362.0\um
C:\Program Files %28x86%29\Windows Kits\10\Include\10.0.18362.0\winrt
C:\Program Files %28x86%29\Microsoft DirectX SDK %28June 2010%29\Include

Lib Setting
C:\Program Files %28x86%29\Windows Kits\10\Lib\10.0.18362.0\um\x86
C:\Program Files %28x86%29\Microsoft DirectX SDK %28June 2010%29\Lib\x86

위의 설정은 기본적으로 깔고 들어가야하며 이 책 예제들을 실행하기 위해
Common 폴더를 위의 설정에서 추가로 포함해야 한다.

위의 설정을 기본으로 밑의 과정을 추가적으로 거친다.

1. 사전 준비

* 2010년 6월자 DirectX SDK(이하 ‘구 DXSDK’) 설치: https://www.microsoft.com/en-us/downloa ... px?id=6812

설치 과정 마지막에 오류가 났는데, http://www.gpgstudy.com/forum/viewtopic.php?t=25426 에 링크된 글을 참고해서 Visual C++ 런타임들을 삭제한 후 다시 설치했습니다.

* Visual Studio Community 2015 설치: https://www.visualstudio.com/ko-kr/visu ... ge-vs.aspx

* http://www.d3dcoder.net/d3d11.htm 에서 책 예제 코드(이하 ‘웹 부록’)를 내려받아서 적당한 곳에 압축 해제. 일단은 Set I만 받아도 됩니다.

2. 일단 서문대로 진행
p.xxxiii ~ xxxviii 까지 따라 한 후 빌드하면 fxc.exe 관련 오류("error X3501: 'main': entrypoint not found")가 발생합니다. 이 오류는 프로젝트 속성의 ‘구성 속성 > HLSL 컴파일러’에서 ‘셰이더 형식’을 ‘효과(/FX)’로 설정하고, 추가로 '셰이더 모델'을 Shader Model 5.0(/5_0) 로 설정하면 사라집니다.

빌드 진행하면 링크 오류가 발생합니다. Effects11.lib 때문인데요. http://www.gpgstudy.com/forum/viewtopic ... 697#127697 에 이 문제가 정리되어 있습니다. 해결책은 Effect11 라이브러리를 직접 빌드해서 VS 2015와 구 DXSDK 조합에 맞는 Effects11.lib을 만들어 내는 것입니다.

3. Effects11 라이브러리 빌드 및 적용
https://github.com/Microsoft/FX11/releases 에서 라이브러리 소스 코드를 내려 받습니다. 저는 https://github.com/Microsoft/FX11/archive/nov2015.zip 을 받았습니다. 압축 해제 후 Effects11_2015.sln를 엽니다.

실행 가능 디렉터리, 포함 디렉터리, 라이브러리 디렉터리에 서문 따라할 때처럼 구 DXSDK의 디렉터리를 제일 앞에 추가합니다. Common 디렉터리는 추가할 필요가 없습니다. 여기서 핵심은 빌드 시 Windows Kits의 DX 11.1이나 DX 12 헤더들을 참조하지 못하게 하는 것입니다. 반드시 구 DXSDK의 디렉터리들이 제일 앞에 와야 합니다.

이제 빌드 하면 아무 오류 없이 성공할 것입니다. 릴리스 모드와 디버그 모드 둘 다 빌드한 후, 디버그 모드 빌드의 결과물은 Effects11d.lib 로 이름을 바꿉니다.

빌드한 Effects11.lib 과 Effects11d.lib, 그리고 Effects11 디렉터리의 inc 디렉터리에 있는 .h 파일들을 웹 부록의 Common 디렉터리에 복사합니다.

4. Effects11 라이브러리의 d3dx11effect.h 수정
다시 서문 예제 프로젝트로 돌아가서 프로젝트를 다시 빌드하면 이번에는 “error C4430: 형식 지정자가 없습니다. int로 가정합니다.” 같은 컴파일 오류들이 나옵니다. 이 오류들은 구 DX SDK에는 없는 DX 11.1 관련 헤더를 Effects11 라이브러리의 d3dx11effect.h가 포함시키기 때문에 발생하는 것입니다. 소스 코드를 수정하지 않고 해결할 방법을 모색했지만, 그냥 d3dx11effect.h를 수정하는 것이 가장 간단한 방법이라는 결론을 내렸습니다. Common 디렉터리에 있는 d3dx11effect.h에서 다음 부분을
코드: 모두 선택

#if defined(_XBOX_ONE) && defined(_TITLE)
#include <d3d11_x.h>
#define DCOMMON_H_INCLUDED
#define NO_D3D11_DEBUG_NAME
#elif (_WIN32_WINNT >= 0x0602) || defined(_WIN7_PLATFORM_UPDATE) 
#include <d3d11_1.h>
#include <d3d11shader.h>
#else
#include <d3d11.h>
#include <d3d11shader.h>
#endif
다음으로 대체합니다(마지막 #else에 걸리도록 _WIN32_WINNT 와 _WIN7_PLATFORM_UPDATE를 적절히 설정하는 방법도 있겠지만 시도해 보지는 않았습니다).
코드: 모두 선택

#include <d3d11.h>
#include <d3d11shader.h>
5. dxerr 갱신
이제 다시 빌드하면 이번에는 LNK2019 __vsnwprintf ... 같은 오류가 발생합니다. 이는 BoxDemo.cpp의 310 행에서 호출하는 DXTrace 때문인데요. http://gpgstudy.com/forum/viewtopic.php?p=127766#127766 에 있는 비회원 글을 참고해서 다음과 같이 해결했습니다.

* http://blogs.msdn.com/b/chuckw/archive/ ... r-lib.aspx 의 dxerr_nov2015.zip 를 받아서 dxerr.h와 dxerr.cpp 를 웹 부록의 Common 디렉터리에 복사해 넣습니다.

* Common 의 d3dUtil.h 에서 #include <dxerr.h> 를 #include "dxerr.h" 로 수정합니다. 빌드 시 DX SDK의 dxerr.h가 아니라 Common의 dxerr.h가 포함되게 하기 위한 것입니다.

* 프로젝트 탐색기에서 Common 필터 우클릭 후 ‘추가 - 기존 항목’을 선택해서 Common 디렉터리에 있는 dxerr.h와 dxerr.cpp를 추가합니다.

* 프로젝트 속성의 ‘링크 - 입력’ 설정에서 dxerr.lib을 제거합니다(이제는 Common에 있는 dxerr.cpp를 직접 컴파일해서 링크하므로).

* 이제 빌드하면 또다시 컴파일 오류가 발생하는데, 이는 새 dxerr.h가 신 SDK의 d3d11_1.h와 wincodec.h를 참조하기 때문입니다. Common의 dxerr.h에서 해당 부분을 다음과 같이 주석으로 처리합니다. 그냥 삭제해도 되겠고요.
코드: 모두 선택

#include <d3d10_1.h>
//#include <d3d11_1.h>
#include <d3d11.h>

/*
#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
#include <wincodec.h>
#include <d2derr.h>
#include <dwrite.h>
#endif
*/
6. DXTrace 오류 해결
다시 빌드를 시도하면 이번에는 BoxDemo.cpp에서 DXTraceW와 WCHAR* 관련 컴파일 오류가 발생합니다. 여러 가지 해결책이 있겠지만, 애초에 클라이언트 쪽에서 DXTrace의 첫 인수로 WCHAR*를 넘겨주게 하는 게 제일 간단하다는 결론을 내렸습니다. 그런데 __FILE__의 넓은 문자 버전에 해당하는 표준 매크로가 없으므로 따로 만들어야 합니다. Common의 dxerr.h의 끝에 (#ifdef __cplusplus 전에) 다음을 추가합니다( http://stackoverflow.com/questions/1442 ... nt-of-file 에 나온 코드입니다.)
코드: 모두 선택

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)
다음으로, BoxDemo.cpp에서 DXTrace를 호출할 때 __FILE__ 대신 WFILE을 사용하도록 수정합니다.
코드: 모두 선택

DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
=>
DXTrace(WFILE, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
이제 빌드하면 매크로 재정의 경고와 int 관련 경고가 많이 뜨긴 하지만 실행 파일이 만들어집니다.

이러한 과정을 거쳐 VS2019에서 실행되는 DX11_FRAME을 생성하였다.