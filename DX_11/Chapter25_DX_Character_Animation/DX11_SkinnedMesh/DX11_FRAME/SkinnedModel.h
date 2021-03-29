#ifndef SKINNEDMODEL_H
#define SKINNEDMODEL_H

#include "SkinnedData.h"
#include "MeshGeometry.h"
#include "TextureMgr.h"
#include "Vertex.h"

class SkinnedModel
{
public:
	SkinnedModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath);
	~SkinnedModel();

	UINT SubsetCount;

	std::vector<Material> Mat;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex::PosNormalTexTanSkinned> Vertices;
	std::vector<USHORT> Indices;
	std::vector<MeshGeometry::Subset> Subsets;

	MeshGeometry ModelMesh;
	SkinnedData SkinnedData;
};

// 애니메이션을 그릴 때 몬스터가 한 종류인데 여러 마리의 애니메이션을 그릴 수가 있다.
// 그런 경우 인스턴싱 기법을 사용한다.
// 모델은 같으니 모델 포인터가 있고 각기 다른 시각의 애니메이션이 필요할 테니 시간이 있고
// 애니메이션 종류, 위치가 다를 것이니 세계 행렬 
// 그리고 각 뼈대마다 다른 최종 변환 행렬에 대한 배열에 관한 정보가 있다.
struct SkinnedModelInstance
{
	SkinnedModel* Model;
	float TimePos;
	std::string ClipName;
	XMFLOAT4X4 World;
	std::vector<XMFLOAT4X4> FinalTransforms;

	void Update(float dt);
};

#endif // SKINNEDMODEL_H