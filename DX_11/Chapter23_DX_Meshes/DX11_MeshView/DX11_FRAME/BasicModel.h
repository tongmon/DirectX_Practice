#ifndef BASICMODEL_H
#define BASICMODEL_H

#include "MeshGeometry.h"
#include "TextureMgr.h"
#include "Vertex.h"

class BasicModel
{
public:
	BasicModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath);
	~BasicModel();

	UINT SubsetCount;

	std::vector<Material> Mat;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

	// CPU에서 메시 자료를 읽을 수 있도록 시스템 메모리에 복사본을 남겨 둔다.
	// 경계입체 계산, 선택, 충돌 검출은 모두 CPU가 수행해서 필요하다.
	std::vector<Vertex::PosNormalTexTan> Vertices;
	std::vector<USHORT> Indices;
	std::vector<MeshGeometry::Subset> Subsets;

	MeshGeometry ModelMesh;
};

struct BasicModelInstance
{
	BasicModel* Model;
	XMFLOAT4X4 World;
};

#endif // SKINNEDMODEL_H