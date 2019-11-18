#ifndef RENDERINGRESOURCES_H
#define RENDERINGRESOURCES_H

#include <d3d11.h>
#include <DirectXMath.h>

namespace dxshow {
//渲染纹理资源类
class RenderingResources
{
  public:
    enum class ResourceViewport
    {
        FULL_VIEW = 0,
        LEFT_HALF = 1,
        RIGHT_HALF = 2
    };
    RenderingResources();
    //支持右值拷贝
    RenderingResources(RenderingResources&& copy);
    RenderingResources(ID3D11Device* device, ID3D11Texture2D* d3dtex, ResourceViewport vp = ResourceViewport::FULL_VIEW);
    ~RenderingResources();
    void UpdateMVPMatrix();
    void ResetToSteropicMatirx(ID3D11DeviceContext* ctx);
    void Render(ID3D11DeviceContext* ctx, UINT index) const;
    ResourceViewport GetResourceVieportType() const;

    bool isValuable;

  private:
    struct VertexPos
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 tex0;
    };
    struct CommandBuffer
    {
        DirectX::XMMATRIX _world;
    };
    // bool updateFlag;
    ID3D11Device* m_device;
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Texture2D* m_d3dtex;
    ID3D11ShaderResourceView* m_ResourceView;
    ID3D11Buffer* m_MVPbuffer;
    CommandBuffer m_commandBuffer;
    DirectX::XMMATRIX _MVPmatrix;
    ResourceViewport m_vp = ResourceViewport::FULL_VIEW;
    template <typename Res>
    inline void SafeRelease(Res* ptr)
    {
        if (ptr != nullptr) {
            ptr->Release();
            ptr = nullptr;
        }
    }
};
} // namespace dxshow
#endif
