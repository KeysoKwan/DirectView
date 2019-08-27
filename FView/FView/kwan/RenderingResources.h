#ifndef RENDERINGRESOURCES_H
#define RENDERINGRESOURCES_H

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

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
    RenderingResources(ID3D11Device* device, ID3D11Texture2D* d3dtex, ResourceViewport vp = ResourceViewport::FULL_VIEW);
    ~RenderingResources();
    void UpdateMVPMatrix();
    void Render(ID3D11DeviceContext* ctx, UINT index) const;
    ResourceViewport GetResourceVieportType() const;

  private:
    //Uncopyable
    RenderingResources(RenderingResources& copy) {}
    struct VertexPos
    {
        XMFLOAT3 pos;
        XMFLOAT2 tex0;
    };
    struct CommandBuffer
    {
        XMMATRIX _world;
    };

    ID3D11Device* m_device;
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Texture2D* m_d3dtex;
    ID3D11ShaderResourceView* m_ResourceView;
    ID3D11Buffer* m_MVPbuffer;
    CommandBuffer m_commandBuffer;
    XMMATRIX _MVPmatrix;
    ResourceViewport m_vp = ResourceViewport::FULL_VIEW;
    template <typename Res>
    inline void SafeRelease(Res* ptr)
    {
        if (ptr != 0) ptr->Release();
        ptr = 0;
    }
};

#endif
