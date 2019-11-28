#include "RenderingResources.h"
#include "IVR_Log.h"
namespace dxshow {
RenderingResources::RenderingResources() : isValuable(false)
{
}

RenderingResources::RenderingResources(RenderingResources&& copy) : ref_device(copy.ref_device),
                                                                    m_vertexBuffer(copy.m_vertexBuffer),
                                                                    ref_d3dtex(copy.ref_d3dtex),
                                                                    m_ResourceView(copy.m_ResourceView),
                                                                    m_MVPbuffer(copy.m_MVPbuffer),
                                                                    m_commandBuffer(copy.m_commandBuffer),
                                                                    _MVPmatrix(copy._MVPmatrix),
                                                                    m_vp(copy.m_vp),
                                                                    isValuable(copy.isValuable) /*,
                                                                    updateFlag(false)*/
{
    /* copy.ref_device = 0;
    copy.m_vertexBuffer = 0;
    copy.m_d3dtex = 0;
    copy.m_ResourceView = 0;
    copy.m_MVPbuffer = 0;*/
}

RenderingResources::RenderingResources(ID3D11Device* device,
                                       ID3D11Texture2D* d3dtex,
                                       ResourceViewport vp) : ref_device(device),
                                                              ref_d3dtex(d3dtex),
                                                              m_vp(vp)
{
    using namespace DirectX;
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
    resourceData.pSysMem = vertices;
    HRESULT hr = ref_device->CreateBuffer(&vertexDesc, &resourceData, m_vertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        char charBuf[128];
        sprintf_s(charBuf, 128, "RenderingResources:CreateBuffer(m_vertexBuffer) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 0);
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC vdesc;
    vdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    vdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    vdesc.Texture2D.MostDetailedMip = 0;
    vdesc.Texture2D.MipLevels = 1;

    hr = ref_device->CreateShaderResourceView(ref_d3dtex, &vdesc, m_ResourceView.GetAddressOf());
    if (FAILED(hr)) {
        //     MessageBox(NULL, L"CreateShaderResourceView failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "RenderingResources:CreateShaderResourceView(...) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        return;
    }

    m_commandBuffer._world = XMMatrixTranspose(XMMatrixIdentity());
    D3D11_BUFFER_DESC commandDesc;
    ZeroMemory(&commandDesc, sizeof(commandDesc));
    commandDesc.Usage = D3D11_USAGE_DEFAULT;
    commandDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    commandDesc.ByteWidth = sizeof(CommandBuffer);
    commandDesc.CPUAccessFlags = 0;
    hr = ref_device->CreateBuffer(&commandDesc, NULL, m_MVPbuffer.GetAddressOf());

    if (FAILED(hr)) {
        //    MessageBox(NULL, L"Create Buffer failed!", L"error", MB_OK);
        char charBuf[128];
        sprintf_s(charBuf, 128, "RenderingResources:CreateBuffer(m_MVPbuffer) failed with error %x", hr);
        IvrLog::Inst()->Log(std::string(charBuf), 4);
        return;
    }
    isValuable = true;
}

RenderingResources::~RenderingResources()
{
    SafeRelease(ref_d3dtex);
}

void RenderingResources::UpdateMVPMatrix()
{
    if (!isValuable) return;
    using namespace DirectX;
    ComPtr<ID3D11DeviceContext> ctx = NULL;
    ref_device->GetImmediateContext(ctx.GetAddressOf());

    switch (m_vp) {
    case ResourceViewport::FULL_VIEW:
        m_commandBuffer._world = XMMatrixTranspose(XMMatrixIdentity());
        break;
    case ResourceViewport::LEFT_HALF:
        m_commandBuffer._world = XMMatrixTranspose(XMMatrixScaling(0.5, 1, 1) * XMMatrixTranslation(-0.5, 0.0, 1.0));
        break;
    case ResourceViewport::RIGHT_HALF:
        m_commandBuffer._world = XMMatrixTranspose(XMMatrixScaling(0.5, 1, 1) * XMMatrixTranslation(0.5, 0.0, 0.5));
        break;
    default:
        m_commandBuffer._world = XMMatrixTranspose(XMMatrixIdentity());
        break;
    }

    ctx->UpdateSubresource(m_MVPbuffer.Get(), 0, NULL, &m_commandBuffer, 0, 0);
}

void RenderingResources::ResetToSteropicMatirx(ID3D11DeviceContext* ctx)
{
    if (!isValuable) return;
    using namespace DirectX;

    m_commandBuffer._world = XMMatrixTranspose(XMMatrixIdentity());
    ctx->UpdateSubresource(m_MVPbuffer.Get(), 0, NULL, &m_commandBuffer, 0, 0);
}

void RenderingResources::Render(ID3D11DeviceContext* ctx, UINT index) const
{
    if (!isValuable) return;
    unsigned int stride = sizeof(VertexPos);
    unsigned int offset = 0;

    ctx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->UpdateSubresource(m_MVPbuffer.Get(), 0, NULL, &m_commandBuffer, 0, 0);
    ctx->VSSetConstantBuffers(0, 1, m_MVPbuffer.GetAddressOf());
    ctx->PSSetShaderResources(0, 1, m_ResourceView.GetAddressOf());
    ctx->Draw(index, 0);
}

RenderingResources::ResourceViewport RenderingResources::GetResourceVieportType() const
{
    return m_vp;
}
} // namespace dxshow
