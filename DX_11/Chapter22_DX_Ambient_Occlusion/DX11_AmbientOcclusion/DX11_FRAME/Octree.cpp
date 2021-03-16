//***************************************************************************************
// Octree.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Octree.h"


Octree::Octree()
	: mRoot(0)
{
}

Octree::~Octree()
{
	SafeDelete(mRoot);
}

void Octree::Build(const std::vector<XMFLOAT3>& vertices, const std::vector<UINT>& indices)
{
	// Cache a copy of the vertices.
	mVertices = vertices;

	// Build AABB to contain the scene mesh.
	XNA::AxisAlignedBox sceneBounds = BuildAABB();
	
	// Allocate the root node and set its AABB to contain the scene mesh.
	mRoot = new OctreeNode();
	mRoot->Bounds = sceneBounds;

	BuildOctree(mRoot, indices);
}

bool Octree::RayOctreeIntersect(FXMVECTOR rayPos, FXMVECTOR rayDir)
{
	return RayOctreeIntersect(mRoot, rayPos, rayDir);
}

XNA::AxisAlignedBox Octree::BuildAABB()
{
	XMVECTOR vmin = XMVectorReplicate(+MathHelper::Infinity);
	XMVECTOR vmax = XMVectorReplicate(-MathHelper::Infinity);
	for(size_t i = 0; i < mVertices.size(); ++i)
	{
		XMVECTOR P = XMLoadFloat3(&mVertices[i]);

		vmin = XMVectorMin(vmin, P);
		vmax = XMVectorMax(vmax, P);
	}

	XNA::AxisAlignedBox bounds;
	XMVECTOR C = 0.5f*(vmin + vmax);
	XMVECTOR E = 0.5f*(vmax - vmin); 

	XMStoreFloat3(&bounds.Center, C); 
	XMStoreFloat3(&bounds.Extents, E); 

	return bounds;
}

void Octree::BuildOctree(OctreeNode* parent, const std::vector<UINT>& indices)
{
	size_t triCount = indices.size() / 3;

	if (triCount < 60) // 삼각형 개수가 60개 미만이면 재귀 종료
	{
		parent->IsLeaf = true;
		parent->Indices = indices;
	}
	else
	{
		parent->IsLeaf = false;

		// 하나의 박스를 8개 구역으로 나눔 (충돌 박스가 8개로 쪼개짐)
		XNA::AxisAlignedBox subbox[8];
		parent->Subdivide(subbox);

		for (int i = 0; i < 8; ++i)
		{
			// 팔진 트리의 자식 노드 할당
			parent->Children[i] = new OctreeNode();
			parent->Children[i]->Bounds = subbox[i];

			// 8개로 쪼개어진 박스 중에 선택된 하나의 박스와 충돌이 되는 삼각형을 찾아서
			// intersectedTriangleIndices 여기에 인덱스로 담아둔다.
			std::vector<UINT> intersectedTriangleIndices;
			for (size_t j = 0; j < triCount; ++j)
			{
				UINT i0 = indices[j * 3 + 0];
				UINT i1 = indices[j * 3 + 1];
				UINT i2 = indices[j * 3 + 2];

				XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
				XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
				XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

				if (XNA::IntersectTriangleAxisAlignedBox(v0, v1, v2, &subbox[i]))
				{
					intersectedTriangleIndices.push_back(i0);
					intersectedTriangleIndices.push_back(i1);
					intersectedTriangleIndices.push_back(i2);
				}
			}

			// 재귀적으로 계속 충돌 박스를 쪼개어 나간다.
			// 근데 삼각형 개수가 60미만이면 그에 따른 조건으로 들어가 재귀가 종료된다.
			BuildOctree(parent->Children[i], intersectedTriangleIndices);
		}
	}
}

bool Octree::RayOctreeIntersect(OctreeNode* parent, FXMVECTOR rayPos, FXMVECTOR rayDir)
{
	// 모든 삼각형 인덱스는 잎 노드에 위치하니 잎노드에 도달하기 전까지는 계속 재귀적으로 탐색
	if (!parent->IsLeaf)
	{
		for (int i = 0; i < 8; ++i)
		{
			// 8개로 쪼개진 박스에 쏜 레이져가 맞았다면 그 쪽 트리 방향으로 재귀적으로 탐색한다.
			float t;
			if (XNA::IntersectRayAxisAlignedBox(rayPos, rayDir, &parent->Children[i]->Bounds, &t))
			{
				// 8개의 트리 자식 중에 하나라도 레이져에 맞았으면 재귀적으로 탐색
				// 그리고 돌아올 때 true를 반환하면서 종료
				if (RayOctreeIntersect(parent->Children[i], rayPos, rayDir))
					return true;
			}
		}

		// 아무것도 치지 못했다면 false 반환
		return false;
	}
	else
	{
		size_t triCount = parent->Indices.size() / 3;

		for (size_t i = 0; i < triCount; ++i)
		{
			UINT i0 = parent->Indices[i * 3 + 0];
			UINT i1 = parent->Indices[i * 3 + 1];
			UINT i2 = parent->Indices[i * 3 + 2];

			XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
			XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
			XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

			float t;
			if (XNA::IntersectRayTriangle(rayPos, rayDir, v0, v1, v2, &t))
				return true;
		}

		return false;
	}
}