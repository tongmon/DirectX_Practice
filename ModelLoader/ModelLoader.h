#pragma once

#include <goh.h>
#include <ScreenGrab.h>
#include <wincodec.h>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

using namespace DirectX;

class ModelLoader
{
public:
	struct Subset
	{
		UINT Id = -1;
		UINT VertexStart = 0;
		UINT VertexCount = 0;
		UINT FaceStart = 0;
		UINT FaceCount = 0;
	};

	struct MLMaterial
	{
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular; // w = SpecPower
		XMFLOAT4 Reflect;
		bool AlphaClip;
		std::string EffectTypeName;
		std::string DiffuseMapName;
		std::string NormalMapName;
		std::string SpecularMapName;
		std::string GlossinessMapName;
	};

	struct LoadedVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 texcoord;
		XMFLOAT4 TangentU;
		XMFLOAT4 BoneWeights;
		XMUINT4 BoneID;
	};

	struct LoadedKeyFrame {
		std::vector<float> PosTime = {};
		std::vector<float> QuatTime = {};
		std::vector<float> ScaleTime = {};

		std::vector<XMFLOAT3> Pos = {};
		std::vector<XMFLOAT4> Quat = {};
		std::vector<XMFLOAT3> Scale = {};
	};

	struct LoadedAnimation {
		XMMATRIX GlobalInvTrans;
		std::string RootBoneName = "";
		int RootBoneIndex = -1;
		double AnimDuration = 0;
		double TicksPerSecond = 0;
		std::vector<int> ParentIndex = {};
		std::vector<XMMATRIX> nodeMatrix = {};
		std::vector<LoadedKeyFrame> KeyFrames = {};
		std::vector<int> KeyFrameIndex = {};
		std::vector<int> BoneIndex = {};
	};

	struct LoadedMesh {
		std::vector<LoadedVertex> vertices_ = {};
		std::vector<UINT> indices_ = {};
		std::vector<std::string> textures_ = {};
		std::vector<std::string> textypes_ = {};

		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular; // w = SpecPower
		XMFLOAT4 Reflect;
	};

public:
	ModelLoader(const ModelLoader&) = delete;

	static ModelLoader& Get() {
		static ModelLoader inst;
		return inst;
	}

	static bool LoadModel(HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* devcon, std::string filename,
		std::string saveFolder, std::string texFolder);
	static bool LoadAnimation(std::string filename);

	static bool ReadModel(const std::string& filename,
		std::vector<LoadedVertex>& vertices,
		std::vector<UINT>& indices,
		std::vector<Subset>& subsets,
		std::vector<MLMaterial>& mats,
		std::vector<XMFLOAT4X4>& boneOffsets);

	static bool ReadAnimation(const std::string& filename, LoadedAnimation& Anim);

private:
	ModelLoader();
	~ModelLoader();
	static void processNode(aiNode* node, const aiScene* scene);
	static void processMesh(aiMesh* mesh, const aiScene* scene, LoadedMesh &Meshes);
	static void processAnim(aiNode* node, LoadedAnimation& Anim, std::unordered_map<std::string, int> &KF_Info, int Parent);
	static std::string WriteTexture(const aiScene* scene, std::string& file);
	static void WriteModel(std::string fileName);
	static void WriteAnimation(std::string fileName);
	static std::string GetFileNameFromPath(std::string& filePath);
	static void LoadInit();

private:
	HWND mHwnd;
	ID3D11Device* mDev;
	ID3D11DeviceContext* mDevcon;
	
	bool mFindRootBone;
	UINT mBoneIndex;
	std::unordered_map<std::string, std::pair<int, XMMATRIX>> mBoneInfo;

	std::vector<LoadedMesh> mMeshes;

	std::string mTexFolder;
	std::string mSaveFolder;
	std::string mOriginFolder;

	// 저장될 파일명과 해당 애니메이션에 대한 정보가 해쉬맵으로 담김
	std::unordered_map<std::string, LoadedAnimation> mAnimations;

	// 애니메이션 처리법, 트리를 선형으로 바꾸어 처리하기 때문에
	// 배열에 부모 인덱스, 참조할 키 프레임의 인덱스(없으면 -1), 참조할 뼈의 번호(없으면 -1)
};

