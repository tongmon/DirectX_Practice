#include "ModelMesh.h"

ModelMesh::ModelMesh()
{
    mVB = NULL;
    mIB = NULL;
    mIndexBufferFormat = DXGI_FORMAT_R32_UINT;
    mVertexStride = 0;
}

ModelMesh::~ModelMesh()
{
    mVB->Release(); mVB = 0;
    mIB->Release(); mIB = 0;
}

void ModelMesh::SetIndices(ID3D11Device* device, const UINT* indices, UINT count)
{
    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * count;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = indices;

    HR(device->CreateBuffer(&ibd, &iinitData, &mIB));
}

void ModelMesh::SetSubsetTable(std::vector<ModelLoader::Subset>& subsetTable)
{
    mSubsetTable = subsetTable;
}

void ModelMesh::Draw(ID3D11DeviceContext* dc, UINT subsetId)
{
    UINT offset = 0;

    dc->IASetVertexBuffers(0, 1, &mVB, &mVertexStride, &offset);
    dc->IASetIndexBuffer(mIB, mIndexBufferFormat, 0);

    dc->DrawIndexed(
        mSubsetTable[subsetId].FaceCount * 3,
        mSubsetTable[subsetId].FaceStart * 3,
        0);
}