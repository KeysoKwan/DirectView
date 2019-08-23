#include "RenderingResources.h"

RenderingResources::RenderingResources()
{
}

RenderingResources::RenderingResources(ID3D11Device* device, ID3D11Texture2D* d3dtex, ResourceViewport vp) : m_device(device), m_d3dtex(d3dtex), m_vp(vp)
{
    //m_device = device;
    //m_d3dtex = d3dtex;
    D3D11_BUFFER_DESC vertexDesc;
    ZeroMemory(&vertexDesc, sizeof(vertexDesc));
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.ByteWidth = sizeof(VertexPos) * 6;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));
    VertexPos vertices[] =
        {
            {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
            {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)},
            {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},

            {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
            {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},
            {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
        };
    switch (m_vp) {
    case FULL_VIEW:
        break;
    case LEFT_HALF:
        vertices[0].tex0.x = 0.5f;
        vertices[1].tex0.x = 0.5f;
        vertices[5].tex0.x = 0.5f;
        /*VertexPos vertices[] =
            {
                {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.5f, 1.0f)},
                {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.5f, 0.0f)},
                {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},

                {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
                {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},
                {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.5f, 1.0f)},
            };*/
        break;
    case RIGHT_HALF:
        vertices[2].tex0.x = 0.5f;
        vertices[3].tex0.x = 0.5f;
        vertices[4].tex0.x = 0.5f;
        /*VertexPos vertices[] =
            {
                {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
                {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)},
                {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.5f, 0.0f)},

                {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.5f, 0.0f)},
                {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.5f, 1.0f)},
                {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
            };*/
        break;
    default:
        break;
    }
    resourceData.pSysMem = vertices;
    HRESULT hr = m_device->CreateBuffer(&vertexDesc, &resourceData, &m_vertexBuffer);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Create Buffer failed!", L"error", MB_OK);
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC vdesc;
    vdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    vdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    vdesc.Texture2D.MostDetailedMip = 0;
    vdesc.Texture2D.MipLevels = 1;

    hr = m_device->CreateShaderResourceView(m_d3dtex, &vdesc, &m_ResourceView);
    if (FAILED(hr)) {
        MessageBox(NULL, L"CreateShaderResourceView failed!", L"error", MB_OK);
    }

    m_commandBuffer._world = XMMatrixTranspose(XMMatrixIdentity());
    D3D11_BUFFER_DESC commandDesc;
    ZeroMemory(&commandDesc, sizeof(commandDesc));
    commandDesc.Usage = D3D11_USAGE_DEFAULT;
    commandDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    commandDesc.ByteWidth = sizeof(CommandBuffer);
    commandDesc.CPUAccessFlags = 0;
    hr = m_device->CreateBuffer(&commandDesc, NULL, &m_MVPbuffer);

    if (FAILED(hr)) {
        MessageBox(NULL, L"Create Buffer failed!", L"error", MB_OK);
        return;
    }
}

RenderingResources::~RenderingResources()
{
    SafeRelease(m_d3dtex);
    SafeRelease(m_ResourceView);
    SafeRelease(m_MVPbuffer);
    SafeRelease(m_vertexBuffer);
}

void RenderingResources::UpdateMVPMatrix()
{
    ID3D11DeviceContext* ctx = NULL;
    m_device->GetImmediateContext(&ctx);

    switch (m_vp) {
    case FULL_VIEW:
        m_commandBuffer._world = XMMatrixTranspose(XMMatrixIdentity());
        break;
    case LEFT_HALF:
        m_commandBuffer._world = XMMatrixTranspose(XMMatrixScaling(0.5, 1, 1) * XMMatrixTranslation(-0.5, 0.0, 0.0));
        break;
    case RIGHT_HALF:
        m_commandBuffer._world = XMMatrixTranspose(XMMatrixScaling(0.5, 1, 1) * XMMatrixTranslation(0.5, 0.0, 1.0)) /* * XMMatrixScaling(2.0, 1, 1)*/;
        break;
    default:
        m_commandBuffer._world = XMMatrixTranspose(XMMatrixIdentity());
        break;
    }

    ctx->UpdateSubresource(m_MVPbuffer, 0, NULL, &m_commandBuffer, 0, 0);
    ctx->Release();
}

void RenderingResources::Render(ID3D11DeviceContext* ctx, UINT index) const
{
    unsigned int stride = sizeof(VertexPos);
    unsigned int offset = 0;

    ctx->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->UpdateSubresource(m_MVPbuffer, 0, NULL, &m_commandBuffer, 0, 0);
    ctx->VSSetConstantBuffers(0, 1, &m_MVPbuffer);
    ctx->PSSetShaderResources(0, 1, &m_ResourceView);
    ctx->Draw(index, 0);
}

RenderingResources::ResourceViewport RenderingResources::GetResourceVieportType() const
{
    return m_vp;
}