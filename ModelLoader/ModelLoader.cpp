#include "ModelLoader.h"

ModelLoader::ModelLoader()
{
	mDev = NULL;
	mDevcon = NULL;
	mHwnd = NULL;
	mOriginFolder = "";
	mMeshes = {};
	mFindRootBone = false;
	mBoneIndex = 0;
}

ModelLoader::~ModelLoader() {
	// empty
}

bool ModelLoader::LoadModel(HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* devcon, std::string filename,
	std::string saveFolder, std::string texFolder)
{
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_ConvertToLeftHanded | 
		aiProcess_Triangulate |
		aiProcess_GenNormals | 
		aiProcess_CalcTangentSpace);

	if (pScene == nullptr)
		return false;

	Get().LoadInit();

	Get().mOriginFolder = filename.substr(0, filename.find_last_of("/\\"));
	Get().mSaveFolder = saveFolder;
	Get().mTexFolder = texFolder;

	Get().mDev = dev;
	Get().mDevcon = devcon;
	Get().mHwnd = hwnd;

	std::wstring wsaveFolder(saveFolder.begin(), saveFolder.end());
	if (_waccess(wsaveFolder.c_str(), 0) == -1) {
		CreateDirectory(wsaveFolder.c_str(), NULL);
	}

	std::wstring wtexFolder(texFolder.begin(), texFolder.end());
	if (_waccess(wtexFolder.c_str(), 0) == -1) {
		CreateDirectory(wtexFolder.c_str(), NULL);
	}

	Get().processNode(pScene->mRootNode, pScene);

	Get().WriteModel(Get().GetFileNameFromPath(filename));

	return true;
}

bool ModelLoader::LoadAnimation(std::string filename)
{
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	const aiScene* pScene;
	pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	if (pScene == nullptr)
		return false;

	Get().mAnimations.clear();

	XMMATRIX GlobalInvMat(pScene->mRootNode->mTransformation[0]);
	GlobalInvMat = XMMatrixTranspose(GlobalInvMat);
	GlobalInvMat = XMMatrixInverse(nullptr, GlobalInvMat);

	std::string AnimName = Get().GetFileNameFromPath(filename);
	for (int i = 0; i < pScene->mNumAnimations; i++)
	{
		Get().mFindRootBone = false;	
		aiAnimation* Anim = pScene->mAnimations[i];
		LoadedAnimation tempAnim;
		tempAnim.GlobalInvTrans = GlobalInvMat;
		tempAnim.TicksPerSecond = Anim->mTicksPerSecond;
		tempAnim.AnimDuration = Anim->mDuration;

		std::unordered_map<std::string, int> KF_NameWithIndex;
		for (int j = 0; j < Anim->mNumChannels; j++)
		{
			aiNodeAnim* Channel = Anim->mChannels[j];
			KF_NameWithIndex[Channel->mNodeName.data] = j;
			LoadedKeyFrame temp;
			for (int k = 0; k < Channel->mNumPositionKeys; k++) {
				temp.PosTime.push_back((float)Channel->mPositionKeys[k].mTime);
				temp.Pos.push_back({ Channel->mPositionKeys[k].mValue.x, 
					Channel->mPositionKeys[k].mValue.y, 
					Channel->mPositionKeys[k].mValue.z });
			}
			for (int k = 0; k < Channel->mNumRotationKeys; k++) {
				temp.QuatTime.push_back((float)Channel->mRotationKeys[k].mTime);
				temp.Quat.push_back({ Channel->mRotationKeys[k].mValue.x,
					Channel->mRotationKeys[k].mValue.y,
					Channel->mRotationKeys[k].mValue.z,
					Channel->mRotationKeys[k].mValue.w });
			}
			for (int k = 0; k < Channel->mNumScalingKeys; k++) {
				temp.ScaleTime.push_back((float)Channel->mScalingKeys[k].mTime);
				temp.Scale.push_back({ Channel->mScalingKeys[k].mValue.x,
					Channel->mScalingKeys[k].mValue.y,
					Channel->mScalingKeys[k].mValue.z });
			}
			tempAnim.KeyFrames.push_back(temp);
		}

		processAnim(pScene->mRootNode, tempAnim, KF_NameWithIndex, -1);
		Get().mAnimations[AnimName + '_' + std::to_string(i + 1)] = tempAnim;
	}

	for (auto& A : Get().mAnimations) {
		WriteAnimation(A.first);
	}

	return true;
}

void ModelLoader::WriteAnimation(std::string fileName)
{
	int numNodes = Get().mAnimations[fileName].nodeMatrix.size();
	int numKeyFrames = Get().mAnimations[fileName].KeyFrames.size();
	int RootBoneIndex = Get().mAnimations[fileName].RootBoneIndex;
	float Duration = Get().mAnimations[fileName].AnimDuration;
	float TickPerSec = Get().mAnimations[fileName].TicksPerSecond;
	std::string RootBoneName = Get().mAnimations[fileName].RootBoneName;
	XMFLOAT4X4 temp;
	XMStoreFloat4x4(&temp, Get().mAnimations[fileName].GlobalInvTrans);

	std::ofstream wfile;
	wfile.open(Get().mSaveFolder + '\\' + fileName + ".anim");
	wfile << "***************Animation_File_Header***************\n";
	wfile << "#Nodes: " << numNodes << "\n";
	wfile << "#KeyFrames: " << numKeyFrames << "\n";
	wfile << "#Duration: " << Duration << "\n";
	wfile << "#TicksPerSecond: " << TickPerSec << "\n";
	wfile << "#GlobalInverseTransform: " << 
		temp._11 << " " << temp._12 << " " << temp._13 << " " << temp._14 << " " <<
		temp._21 << " " << temp._22 << " " << temp._23 << " " << temp._24 << " " <<
		temp._31 << " " << temp._32 << " " << temp._33 << " " << temp._34 << " " <<
		temp._41 << " " << temp._42 << " " << temp._43 << " " << temp._44 << " " << "\n";
	wfile << "#RootBoneName: " << RootBoneName << "\n";
	wfile << "#RootBoneIndex: " << RootBoneIndex << "\n";
	wfile << "\n";

	wfile << "***********************Nodes***********************\n";
	int k = 0;
	for (auto& Mat : Get().mAnimations[fileName].nodeMatrix) {
		XMStoreFloat4x4(&temp, Mat);
		wfile << "#" << k++ << "_Matrix: " << 
			temp._11 << " " << temp._12 << " " << temp._13 << " " << temp._14 << " " <<
			temp._21 << " " << temp._22 << " " << temp._23 << " " << temp._24 << " " <<
			temp._31 << " " << temp._32 << " " << temp._33 << " " << temp._34 << " " <<
			temp._41 << " " << temp._42 << " " << temp._43 << " " << temp._44 << " " << "\n";
	}
	wfile << "\n";

	wfile << "**********************Parents**********************\n";
	k = 0;
	for (auto& P : Get().mAnimations[fileName].ParentIndex) {
		wfile << "#" << k++ << "_Parents: " << P << "\n";
	}
	wfile << "\n";

	wfile << "********************BoneIndices********************\n";
	k = 0;
	for (auto& P : Get().mAnimations[fileName].BoneIndex) {
		wfile << "#" << k++ << "_BoneIndex: " << P << "\n";
	}
	wfile << "\n";

	wfile << "******************KeyFrameIndices******************\n";
	k = 0;
	for (auto& P : Get().mAnimations[fileName].KeyFrameIndex) {
		wfile << "#" << k++ << "_KeyFrameIndex: " << P << "\n";
	}
	wfile << "\n";

	wfile << "*********************KeyFrames*********************\n";
	k = 0;
	for (auto& KF : Get().mAnimations[fileName].KeyFrames) {
		wfile << "#" << k++ << "_KeyFrame\n";
		wfile << "Scale_Size: " << KF.Scale.size() << "\n";
		wfile << "Rotate_Size: " << KF.Quat.size() << "\n";
		wfile << "Pos_Size: " << KF.Pos.size() << "\n";
		for (int i = 0; i < KF.Scale.size(); i++) {
			wfile << "Scale_Time: " << KF.ScaleTime[i] << " Scale: " << KF.Scale[i].x << " " << KF.Scale[i].y << " " << KF.Scale[i].z << "\n";
		}
		for (int i = 0; i < KF.Quat.size(); i++) {
			wfile << "Rotate_Time: " << KF.QuatTime[i] << " Rotation: " << KF.Quat[i].x << " " << KF.Quat[i].y << " " << KF.Quat[i].z << " " << KF.Quat[i].w << "\n";
		}
		for (int i = 0; i < KF.Pos.size(); i++) {
			wfile << "Pos_Time: " << KF.PosTime[i] << " Pos: " << KF.Pos[i].x << " " << KF.Pos[i].y << " " << KF.Pos[i].z << "\n";
		}
		wfile << "\n";
	}

	wfile.close();
}

void ModelLoader::processAnim(aiNode* node, LoadedAnimation &Anim, std::unordered_map<std::string, int> &KF_Info, int Parent)
{
	std::string Name = node->mName.data;
	
	XMMATRIX temp(node->mTransformation[0]);
	temp = XMMatrixTranspose(temp);
	
	Anim.ParentIndex.push_back(Parent);
	Anim.nodeMatrix.push_back(temp);

	int Bone_Index = -1, KF_Index = -1, P_Index = Anim.ParentIndex.size() - 1;
	if (Get().mBoneInfo.find(Name) != Get().mBoneInfo.end()) {
		Bone_Index = Get().mBoneInfo[Name].first;
		if (!Get().mFindRootBone) {
			Anim.RootBoneName = Name;
			Anim.RootBoneIndex = P_Index; 
			Get().mFindRootBone = true;
		}
	}
	if (KF_Info.find(Name) != KF_Info.end()) {
		KF_Index = KF_Info[Name];
	}
	Anim.BoneIndex.push_back(Bone_Index);
	Anim.KeyFrameIndex.push_back(KF_Index);

	for (int i = 0; i < node->mNumChildren; i++) {
		processAnim(node->mChildren[i], Anim, KF_Info, P_Index);
	}
}

void ModelLoader::WriteModel(std::string fileName)
{	
	int NumMaterials = Get().mMeshes.size();
	int NumVertices = 0, NumTriangles = 0;
	std::vector<std::pair<int, int>> VertexSubset;
	std::vector<std::pair<int, int>> FaceSubset;
	for (auto& M : Get().mMeshes) {
		VertexSubset.push_back({ NumVertices, M.vertices_.size() });
		FaceSubset.push_back({ NumTriangles, M.indices_.size() / 3 });
		NumVertices += M.vertices_.size();
		NumTriangles += M.indices_.size() / 3;
	}
	int NumBones = Get().mBoneIndex;

	// 통합 정점, 인덱스 설정
	std::vector<LoadedVertex> totalVertex;
	totalVertex.reserve(NumVertices);
	std::vector<UINT> totalIndices;
	totalIndices.reserve(NumTriangles * 3);
	for (int i = 0; i < VertexSubset.size(); i++) {
		for (auto& Ind : Get().mMeshes[i].indices_) {
			totalIndices.push_back(Ind + VertexSubset[i].first);
		}
		for (auto& V : Get().mMeshes[i].vertices_) {
			totalVertex.push_back(V);
		}
	}

	std::ofstream wfile;
	wfile.open(Get().mSaveFolder + '\\' + fileName + ".model");
	wfile << "***************Model_File_Header***************\n";
	wfile << "#Materials: " << NumMaterials << "\n";
	wfile << "#Vertices: " << NumVertices << "\n";
	wfile << "#Triangles: " << NumTriangles << "\n";
	wfile << "#Bones: " << NumBones << "\n";
	// wfile << "#AnimationClips: " << NumVertices << "\n";
	wfile << "\n";

	std::vector<std::string> types;
	types.push_back("diffuse"); types.push_back("normals");
	types.push_back("specular"); types.push_back("shininess");

	wfile << "*******************Materials*******************\n";
	for (auto& M : Get().mMeshes) {
		wfile << "#Ambient: " << M.Ambient.x << " " << M.Ambient.y << " " << M.Ambient.z << "\n";
		wfile << "#Diffuse: " << M.Diffuse.x << " " << M.Diffuse.y << " " << M.Diffuse.z << "\n";
		wfile << "#Specular: " << M.Specular.x << " " << M.Specular.y << " " << M.Specular.z << "\n";
		wfile << "#SpecPower: " << M.Specular.w << "\n";
		wfile << "#Reflectivity: " << M.Reflect.x << " " << M.Reflect.y << " " << M.Reflect.z << "\n";
		wfile << "#AlphaClip: " << 0 << "\n";
		wfile << "#Effect: " << "Basic" << "\n";
		for (int i = 0; i < types.size(); i++) {
			for (int j = 0; j < M.textures_.size(); j++) {
				if (M.textypes_[j] == types[i]) {
					wfile << "#" << types[i] << "Map: ";
					if (M.textures_[j].empty())
						wfile << "None\n";
					else
						wfile << M.textures_[j] << "\n";
					break;
				}
			}
		}
		wfile << "\n";
	}

	wfile << "******************SubsetTable******************\n";
	for (int i = 0; i < VertexSubset.size(); i++) {
		wfile << "#SubsetID: " << i << " VertexStart: " << VertexSubset[i].first
			<< " VertexCount: " << VertexSubset[i].second << " FaceStart: " << FaceSubset[i].first
			<< " FaceCount: " << FaceSubset[i].second << "\n";
	}
	wfile << "\n";

	wfile << "*******************Vertices*******************\n";
	for (auto& V : totalVertex) {
		wfile << "#Position: " << V.Pos.x << " " << V.Pos.y << " " << V.Pos.z << "\n";
		wfile << "#Tangent: " << V.TangentU.x << " " << V.TangentU.y << " " << V.TangentU.z << " " << V.TangentU.w << "\n";
		wfile << "#Normal: " << V.Normal.x << " " << V.Normal.y << " " << V.Normal.z << "\n";
		wfile << "#Tex-Coords: " << V.texcoord.x << " " << V.texcoord.y << "\n";
		wfile << "#BlendWeights: " << V.BoneWeights.x << " " << V.BoneWeights.y << " " << V.BoneWeights.z << " " << V.BoneWeights.w << "\n";
		wfile << "#BlendIndices: " << V.BoneID.x << " " << V.BoneID.y << " " << V.BoneID.z << " " << V.BoneID.w << "\n";
		wfile << "\n";
	}

	wfile << "******************Triangles******************\n";
	for (int i = 0; i < totalIndices.size(); i += 3) {
		wfile << totalIndices[i] << " " << totalIndices[i + 1] << " " << totalIndices[i + 2] << "\n";
	}
	wfile << "\n";

	// 뼈 이름, 뼈가 어디 인덱스에 저장되어야 하는지, 뼈 오프셋 행렬
	wfile << "*****************BoneOffsets*****************\n";
	std::vector<std::pair<std::string, XMFLOAT4X4>> bone_Mat(Get().mBoneInfo.size());
	XMFLOAT4X4 temp;
	for (auto& B : Get().mBoneInfo) {
		XMStoreFloat4x4(&temp, B.second.second);
		bone_Mat[B.second.first] = { B.first, temp };
	}
	int x = 0;
	for (auto& A : bone_Mat) {
		std::string& bone_Name = A.first;
		XMFLOAT4X4& Mat = A.second;
		wfile << x++ << " " << bone_Name << " " <<
			Mat._11 << " " << Mat._12 << " " << Mat._13 << " " << Mat._14 << " " <<
			Mat._21 << " " << Mat._22 << " " << Mat._23 << " " << Mat._24 << " " <<
			Mat._31 << " " << Mat._32 << " " << Mat._33 << " " << Mat._34 << " " <<
			Mat._41 << " " << Mat._42 << " " << Mat._43 << " " << Mat._44 << " " << "\n";
	}

	// wfile << "****************BoneHierarchy****************\n"; // 뼈에 대해선 굳이 필요없음, 애니메이션 노드에 대해서는 부모 자식관계가 필요

	wfile.close();
}

// 매쉬 데이터 읽는 함수
void ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, LoadedMesh& Meshes) 
{
	// 메쉬 정점, 인덱스, 텍스처 경로
	std::vector<LoadedVertex> &vertices = Meshes.vertices_;
	std::vector<UINT> &indices = Meshes.indices_;
	std::vector<std::string> &textures = Meshes.textures_;

	// 정점 정보 획득
	for (UINT i = 0; i < mesh->mNumVertices; i++) {
		LoadedVertex vertex;

		vertex.Pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		if (mesh->HasNormals())
			vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

		if (mesh->HasTangentsAndBitangents()) {
			vertex.TangentU = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, -1.0f };
			XMFLOAT3 Tan = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z }, Dot;
			XMFLOAT3 BiTan = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
			XMVECTOR calcBiTan = XMVector3Cross(XMLoadFloat3(&vertex.Normal), XMLoadFloat3(&Tan));
			XMStoreFloat3(&Dot, XMVector3Dot(XMVector3Normalize(calcBiTan), XMLoadFloat3(&BiTan)));
			if (Dot.x >= 0) // 애초에 bitangent와 일치하면 방향 틀 필요가 없으니 1
				vertex.TangentU.w = 1.0f;
		}

		if (mesh->mTextureCoords[0]) {
			vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertex.BoneID = { 0,0,0,0 };
		vertex.BoneWeights = { 0,0,0,0 };

		vertices.push_back(vertex);
	}

	// 각 정점마다 영향을 끼치는 뼈들이 있는데 그 영향 끼치는 뼈의
	// 가중치를 정렬 시키기 위한 배열, 가중치와 인덱스 순으로 담긴다.
	std::vector<std::vector<std::pair<float, int>>> bone_W_ID;
	bone_W_ID.resize(vertices.size());

	// 뼈대 정보 획득
	for (UINT i = 0; i < mesh->mNumBones; i++) {
		aiBone* bone = mesh->mBones[i];
		XMMATRIX M(bone->mOffsetMatrix[0]);
		M = XMMatrixTranspose(M);

		if (Get().mBoneInfo.find(bone->mName.C_Str()) == Get().mBoneInfo.end())
			Get().mBoneInfo[bone->mName.C_Str()] = { Get().mBoneIndex++, M };

		// 해당 정점에 영향을 미치는 뼈들의 가중치 획득
		for (int j = 0; j < bone->mNumWeights; j++) {
			UINT id = bone->mWeights[j].mVertexId;
			UINT boneID = Get().mBoneInfo[bone->mName.C_Str()].first;
			float weight = bone->mWeights[j].mWeight;

			bone_W_ID[id].push_back({ weight, boneID });
		}
	}

	// 가중치가 4개 보다 많은 경우가 있으므로 가중치 큰 녀석부터 처리
	int p = 0;
	for (auto& W_ID : bone_W_ID) {
		if (W_ID.size() > 4) {
			std::sort(W_ID.begin(), W_ID.end(), std::greater<std::pair<float, int>>());
		}
		if (W_ID.size() > 0) {
			vertices[p].BoneWeights.x = W_ID[0].first;
			vertices[p].BoneID.x = W_ID[0].second;
		}
		if (W_ID.size() > 1) {
			vertices[p].BoneWeights.y = W_ID[1].first;
			vertices[p].BoneID.y = W_ID[1].second;
		}
		if (W_ID.size() > 2) {
			vertices[p].BoneWeights.z = W_ID[2].first;
			vertices[p].BoneID.z = W_ID[2].second;
		}
		if (W_ID.size() > 3) {
			vertices[p].BoneWeights.w = W_ID[3].first;
			vertices[p].BoneID.w = W_ID[3].second;
		}
		p++;
	}

	// 가중치의 합을 1로 만들어준다.
	for (int i = 0; i < vertices.size(); i++) {
		XMFLOAT4& BoneWeight = vertices[i].BoneWeights;
		float totalWeight = BoneWeight.x + BoneWeight.y + BoneWeight.z + BoneWeight.w;
		if (totalWeight > 0.0f) {
			vertices[i].BoneWeights = XMFLOAT4(
				BoneWeight.x / totalWeight,
				BoneWeight.y / totalWeight,
				BoneWeight.z / totalWeight,
				BoneWeight.w / totalWeight);
		}
	}

	// 인덱스 정보 획득
	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// 머터리얼(색상, 텍스쳐, 노멀맵) 정보 획득
	if (mesh->mMaterialIndex >= 0) 
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// 색상, 전역광, 스펙, 반사율 획득
		aiColor3D color;
		
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		Meshes.Diffuse = { color.r, color.g, color.b, 1.0f };
		
		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		Meshes.Ambient = { color.r, color.g, color.b, 1.0f };

		material->Get(AI_MATKEY_COLOR_SPECULAR, color);
		Meshes.Specular = { color.r, color.g, color.b, 1.0f };

		ai_real sExp;
		material->Get(AI_MATKEY_SHININESS_STRENGTH, sExp);
		Meshes.Specular.w = sExp;

		material->Get(AI_MATKEY_COLOR_REFLECTIVE, color);
		Meshes.Reflect = { color.r, color.g, color.b, 1.0f };

		aiString file;
		std::string strFile;

		material->GetTexture(aiTextureType_DIFFUSE, 0, &file);
		strFile = file.C_Str();
		strFile = WriteTexture(scene, strFile);
		Meshes.textures_.push_back(strFile);
		Meshes.textypes_.push_back("diffuse");

		material->GetTexture(aiTextureType_SPECULAR, 0, &file);
		strFile = file.C_Str();
		strFile = WriteTexture(scene, strFile);
		Meshes.textures_.push_back(strFile);
		Meshes.textypes_.push_back("specular");

		material->GetTexture(aiTextureType_NORMALS, 0, &file);
		strFile = file.C_Str();
		strFile = WriteTexture(scene, strFile);
		Meshes.textures_.push_back(strFile);
		Meshes.textypes_.push_back("normals");

		material->GetTexture(aiTextureType_SHININESS, 0, &file);
		strFile = file.C_Str();
		strFile = WriteTexture(scene, strFile);
		Meshes.textures_.push_back(strFile);
		Meshes.textypes_.push_back("shininess");
	}
}

// 내부 텍스쳐면 해당 텍스쳐 추출해서 저장하고 아니면 원래 파일 복사해서 저장하는 함수
// 반환은 저장 경로를 반환함
std::string ModelLoader::WriteTexture(const aiScene* scene, std::string &file)
{
	if (file.empty())
		return "";

	auto tex = scene->GetEmbeddedTexture(file.c_str());
	std::string fileName = Get().GetFileNameFromPath(file);

	if (tex != NULL) {
		fileName = Get().mTexFolder + "\\" + fileName + ".png";

		if (tex->mHeight < 1) {
			std::ofstream writeFile;
			writeFile.open(fileName, std::ostream::binary);
			writeFile.write((const char*)tex->pcData, tex->mWidth);
			writeFile.close();
		}
		else {
			ID3D11Texture2D* dest;
			D3D11_TEXTURE2D_DESC destDesc;
			ZeroMemory(&destDesc, sizeof(D3D11_TEXTURE2D_DESC));
			destDesc.Width = tex->mWidth;
			destDesc.Height = tex->mHeight;
			destDesc.MipLevels = 1;
			destDesc.ArraySize = 1;
			destDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			destDesc.SampleDesc.Count = 1;
			destDesc.SampleDesc.Quality = 0;

			D3D11_SUBRESOURCE_DATA subResource = { 0 };
			subResource.pSysMem = tex->pcData;

			Get().mDev->CreateTexture2D(&destDesc, &subResource, &dest);
			
			std::wstring wfileName(fileName.begin(), fileName.end());
			HRESULT hr = SaveWICTextureToFile(Get().mDevcon, dest, GUID_ContainerFormatPng, wfileName.c_str());
			assert(hr == S_OK);
		}
	}
	else {
		std::string extend;
		int i = file.length() - 1;
		for (; i >= 0; i--) {
			extend.insert(extend.begin(), file[i]);
			if (file[i] == '.')
				break;
		}
		fileName += extend;
		std::wstring wOriginPath(Get().mOriginFolder.begin(), Get().mOriginFolder.end());
		wOriginPath += L"\\";
		for (auto& s : fileName)
			wOriginPath.push_back((wchar_t)s);
		fileName = Get().mTexFolder + "\\" + fileName;
		std::wstring wfileName(fileName.begin(), fileName.end());
		CopyFile(wOriginPath.c_str(), wfileName.c_str(), FALSE);
	}

	return fileName;
}

// 확장자 없이 파일 이름만 가져오는 함수
std::string ModelLoader::GetFileNameFromPath(std::string &filePath)
{
	int dot = 0;
	std::string fileName;
	for (int i = filePath.length() - 1; i >= 0; i--) {
		if (filePath[i] == '.' && !dot)
			dot = filePath.length() - i;
		if (filePath[i] == '/' || filePath[i] == '\\')
			break;
		fileName += filePath[i];
	}
	std::reverse(fileName.begin(), fileName.end());
	return fileName.substr(0, fileName.length() - dot);
}

void ModelLoader::LoadInit()
{
	Get().mTexFolder = "";
	Get().mSaveFolder = "";
	Get().mOriginFolder = "";

	Get().mMeshes.clear();
	Get().mBoneInfo.clear();
	Get().mAnimations.clear();

	Get().mFindRootBone = false;
	Get().mBoneIndex = 0;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene) {
	for (UINT i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		LoadedMesh lMesh;
		Get().processMesh(mesh, scene, lMesh);
		Get().mMeshes.push_back(lMesh);
	}

	for (UINT i = 0; i < node->mNumChildren; i++) {
		Get().processNode(node->mChildren[i], scene);
	}
}

bool ModelLoader::ReadModel(const std::string& filename,
	std::vector<LoadedVertex>& vertices,
	std::vector<UINT>& indices,
	std::vector<Subset>& subsets,
	std::vector<MLMaterial>& mats,
	std::vector<XMFLOAT4X4>& boneOffsets)
{
	std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numTriangles = 0;
	UINT numBones = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		// 초반 개수 정보 획득
		fin >> ignore;
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;

		// 머터리얼 정보 획득
		mats.reserve(numMaterials);
		fin >> ignore;
		for (int i = 0; i < numMaterials; i++)
		{
			MLMaterial temp;
			fin >> ignore >> temp.Ambient.x >> temp.Ambient.y >> temp.Ambient.z;
			fin >> ignore >> temp.Diffuse.x >> temp.Diffuse.y >> temp.Diffuse.z;
			fin >> ignore >> temp.Specular.x >> temp.Specular.y >> temp.Specular.z;
			fin >> ignore >> temp.Specular.w;
			fin >> ignore >> temp.Reflect.x >> temp.Reflect.y >> temp.Reflect.z;
			fin >> ignore >> temp.AlphaClip;
			fin >> ignore >> temp.EffectTypeName;
			fin >> ignore >> temp.DiffuseMapName;
			fin >> ignore >> temp.NormalMapName;
			fin >> ignore >> temp.SpecularMapName;
			fin >> ignore >> temp.GlossinessMapName;
			mats.push_back(temp);
		}

		// 서브셋 정보 획득
		subsets.reserve(numMaterials);
		fin >> ignore;
		for (int i = 0; i < numMaterials; i++)
		{
			Subset temp;
			fin >> ignore >> temp.Id >> ignore >> temp.VertexStart >> ignore >> temp.VertexCount >> 
				ignore >> temp.FaceStart >> ignore >> temp.FaceCount;
			subsets.push_back(temp);
		}

		// 정점 정보 획득
		vertices.reserve(numVertices);
		fin >> ignore;
		for (int i = 0; i < numVertices; i++)
		{
			LoadedVertex temp;
			fin >> ignore >> temp.Pos.x >> temp.Pos.y >> temp.Pos.z;
			fin >> ignore >> temp.TangentU.x >> temp.TangentU.y >> temp.TangentU.z >> temp.TangentU.w;
			fin >> ignore >> temp.Normal.x >> temp.Normal.y >> temp.Normal.z;
			fin >> ignore >> temp.texcoord.x >> temp.texcoord.y;
			fin >> ignore >> temp.BoneWeights.x >> temp.BoneWeights.y >> temp.BoneWeights.z >> temp.BoneWeights.w;
			fin >> ignore >> temp.BoneID.x >> temp.BoneID.y >> temp.BoneID.z >> temp.BoneID.w;
			vertices.push_back(temp);
		}

		// 인덱스 정보 획득
		indices.resize(numTriangles * 3);
		fin >> ignore;
		for (int i = 0; i < numTriangles; i++)
		{
			fin >> indices[i * 3] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
		}

		// 뼈대 오프셋 획득
		fin >> ignore;
		boneOffsets.reserve(numBones);
		for (int i = 0; i < numBones; i++)
		{
			XMFLOAT4X4 temp;
			fin >> ignore >> ignore >>
				temp._11 >> temp._12 >> temp._13 >> temp._14 >>
				temp._21 >> temp._22 >> temp._23 >> temp._24 >>
				temp._31 >> temp._32 >> temp._33 >> temp._34 >>
				temp._41 >> temp._42 >> temp._43 >> temp._44;
			boneOffsets.push_back(temp);
		}

		fin.close();

		return true;
	}
	return false;
}

bool ModelLoader::ReadAnimation(const std::string& filename, LoadedAnimation& Anim)
{
	std::ifstream fin(filename);

	XMFLOAT4X4 temp;
	std::string ignore;
	int numNodes;
	int numKeyFrames;

	if (fin) {
		// 초반 애니메이션 정보 획득
		fin >> ignore;
		fin >> ignore >> numNodes;
		fin >> ignore >> numKeyFrames;
		fin >> ignore >> Anim.AnimDuration;
		fin >> ignore >> Anim.TicksPerSecond;
		fin >> ignore >>
			temp._11 >> temp._12 >> temp._13 >> temp._14 >>
			temp._21 >> temp._22 >> temp._23 >> temp._24 >>
			temp._31 >> temp._32 >> temp._33 >> temp._34 >>
			temp._41 >> temp._42 >> temp._43 >> temp._44;
		Anim.GlobalInvTrans = XMLoadFloat4x4(&temp);
		fin >> ignore >> Anim.RootBoneName;
		fin >> ignore >> Anim.RootBoneIndex;

		// 노드 행렬 정보 획득
		fin >> ignore;
		Anim.nodeMatrix.resize(numNodes);
		for (int i = 0; i < numNodes; i++)
		{
			fin >> ignore >>
				temp._11 >> temp._12 >> temp._13 >> temp._14 >>
				temp._21 >> temp._22 >> temp._23 >> temp._24 >>
				temp._31 >> temp._32 >> temp._33 >> temp._34 >>
				temp._41 >> temp._42 >> temp._43 >> temp._44;
			Anim.nodeMatrix[i] = XMLoadFloat4x4(&temp);
		}

		// 부모 인덱스 정보 획득
		fin >> ignore;
		Anim.ParentIndex.resize(numNodes);
		for (int i = 0; i < numNodes; i++)
		{
			fin >> ignore >> Anim.ParentIndex[i];
		}

		// 뼈 인덱스 정보 획득
		fin >> ignore;
		Anim.BoneIndex.resize(numNodes);
		for (int i = 0; i < numNodes; i++)
		{
			fin >> ignore >> Anim.BoneIndex[i];
		}

		// 키 프레임 인덱스 정보 획득
		fin >> ignore;
		Anim.KeyFrameIndex.resize(numNodes);
		for (int i = 0; i < numNodes; i++)
		{
			fin >> ignore >> Anim.KeyFrameIndex[i];
		}

		// 키 프레임 정보 획득
		int numScale, numRotate, numPos;
		Anim.KeyFrames.resize(numKeyFrames);
		fin >> ignore;
		for (int i = 0; i < numKeyFrames; i++)
		{
			LoadedKeyFrame &KF = Anim.KeyFrames[i];
			fin >> ignore;
			fin >> ignore >> numScale;
			fin >> ignore >> numRotate;
			fin >> ignore >> numPos;

			KF.Scale.resize(numScale);
			KF.ScaleTime.resize(numScale);
			for (int j = 0; j < numScale; j++)
			{
				fin >> ignore >> KF.ScaleTime[j] >> ignore >> KF.Scale[j].x >> KF.Scale[j].y >> KF.Scale[j].z;
			}

			KF.Quat.resize(numRotate);
			KF.QuatTime.resize(numRotate);
			for (int j = 0; j < numRotate; j++)
			{
				fin >> ignore >> KF.QuatTime[j] >> ignore >> KF.Quat[j].x >> KF.Quat[j].y >> KF.Quat[j].z >> KF.Quat[j].w;
			}

			KF.Pos.resize(numPos);
			KF.PosTime.resize(numPos);
			for (int j = 0; j < numPos; j++)
			{
				fin >> ignore >> KF.PosTime[j] >> ignore >> KF.Pos[j].x >> KF.Pos[j].y >> KF.Pos[j].z;
			}
		}

		fin.close();

		return true;
	}
	return false;
}