#include "BasicModel.h"
#include "LoadM3d.h"

BasicModel::BasicModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath)
{
	// TextureMgr
	// 얘는 텍스쳐 중복 적재를 방지하기 위해 정의된 클래스이다.
	// map을 사용하여 이를 수행한다.
	
	std::vector<M3dMaterial> mats;
	M3DLoader m3dLoader;
	m3dLoader.LoadM3d(modelFilename, Vertices, Indices, Subsets, mats);

	ModelMesh.SetVertices(device, &Vertices[0], Vertices.size());
	ModelMesh.SetIndices(device, &Indices[0], Indices.size());
	ModelMesh.SetSubsetTable(Subsets);

	SubsetCount = mats.size();

	for(UINT i = 0; i < SubsetCount; ++i)
	{
		Mat.push_back(mats[i].Mat);

		// texturePath(폴더명) + mats[i].DiffuseMapName(폴더 내의 특정 파일명)
		ID3D11ShaderResourceView* diffuseMapSRV = texMgr.CreateTexture(texturePath + mats[i].DiffuseMapName);
		DiffuseMapSRV.push_back(diffuseMapSRV);

		ID3D11ShaderResourceView* normalMapSRV = texMgr.CreateTexture(texturePath + mats[i].NormalMapName);
		NormalMapSRV.push_back(normalMapSRV);
	}
}

BasicModel::~BasicModel()
{
}
