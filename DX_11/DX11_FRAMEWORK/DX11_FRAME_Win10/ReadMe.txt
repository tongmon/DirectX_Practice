Directx11 프레임 워크다.
x86, x64에서 모두 돌아가고 sln 설정의 상태는 밑과 같다.

포함 디렉터리
$(SolutionDir)\DX11_FRAMEWORK\DXTK
$(SolutionDir)\DX11_FRAMEWORK\Common

쉐이더 버전 및 종류
버전 : fx_5_0
종류 : 효과(.fx)

미리 컴파일된 헤더 : 사용 안함

근데 이 프레임 워크는 Legacy SDK를 일체 사용하지 않고 Windows Kit에 내장되어 있는
라이브러리, 헤더만을 가지고 작동하게 된다.
그래서 효과 파일을 사용할 때 d3dx11effect, d3dxGlobal 등의 헤더가 필요해서 내장시켰고
오류 탐지에 필요한 dxerr 헤더가 없어서 dxerr 헤더도 따로 포함시켰다.
이미 해당 프레임워크에 세팅이 되어있기에 sln 파일을 열어 빌드하면 사용이 가능하다.
위에서 말한 헤더들은 Common 파일에 있으며 DirectX tool Kit도 사용가능하도록
VS 설정에서 경로를 지정해주었다.
근데 sln파일을 실행해서 빌드가 그냥 되지는 않는데 
사전에 Effect11.lib, Effects11d.lib 이 두가지 라이브러리 파일을
C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um 이 경로에
4가지 폴더(x64, x86, arm...)가 있을 텐데 각 폴더에 넣어주어야 한다.
그니까 예를 들어 C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64 에
x64용 Effect11.lib, Effects11d.lib 얘네를 만들어서 넣어야 한다는 것이다.
그러면 어디서 Effect11.lib, Effects11d.lib를 구하냐면
https://github.com/microsoft/FX11 여기에 올린 파일을 다운로드하고
자신이 원하는 버전의 sln파일을 선택해서 열고 빌드하면 된다.
빌드할 때 x86(Win32), x64, arm, 디버그, 릴리즈 각 경우를 다 해야 4개 폴더(x64, x86, arm, arm64)에 알맞은 라이브러리를 모두 획득 가능하다.
arm64, arm은 자신의 cpu를 말하는 것 같으니 자신의 cpu가 amd라면 arm용 빌드는 선택이 아닌 필수가 될 것이다.
이렇게 Effect11.lib, Effects11d.lib 파일을 알맞게 넣었다면 #pragma comment (lib,"Effects11d.lib") 얘네가 잘 작동하면서
빌드가 정상적으로 잘 될 것이다.

또한 DXTK를 사용하기 위한 라이브러리도 필요한데 https://github.com/microsoft/DirectXTK 여기에 가서
Effects11.lib 만들던 것과 똑같이 원하는 버전 sln 켜서 각 경우마다 빌드한 lib를 디버그면 d붙이고
아니면 그냥 .lib를 Effects11.lib의 경로와 똑같이 넣어주면 된다.

um 폴더에 넣는게 싫으면 이런 라이브러리 빌드한 후에 그냥 lib 폴더 따로 만들어서 다 쳐박아둬도 좋다.

또한 DXTK에서 Audio.h 즉 XAudio를 사용하고 싶다면 nuget도 설치해줘야 한다.
VS에서 위 프로젝트 탭을 누르면 Nuget 패키지 관리 항목이 있는데 그걸 누른다.
그리고 XAudio2를 검색하면 나오는 XAudio2.Redist 얘가 나오는데 이걸 설치하면 된다.

이 프레임 워크는 추가적으로 https://github.com/walbourn/directx-vs-templates
깃 허브에서 배포하는 VS2019 템플릿을 조금 수정하고 몇 가지 추가해서 만든 것이다.
배포하는 템플릿을 매우 유용하니 필요하면 받아서 VSIX 폴더로 가서 .vsix 로 설치하면
VS2019에서 Direct3D 템플릿들이 생겨난다.