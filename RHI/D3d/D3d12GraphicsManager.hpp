#pragma once
#include <DXGI1_4.h>
#include <d3d12.h>
#include <stdint.h>

#include <map>
#include <vector>

#include "Buffer.hpp"
#include "D3d12PipelineStateManager.hpp"
#include "GraphicsManager.hpp"
#include "Image.hpp"
#include "SceneObject.hpp"

namespace My {
class D3d12GraphicsManager : public GraphicsManager {
   public:
    ~D3d12GraphicsManager() override;

    int Initialize() final;
    void Finalize() final;

    void Draw() final;
    void Present() final;

    void SetPipelineState(const std::shared_ptr<PipelineState>& pipelineState,
                          const Frame& frame) final;

    void DrawBatch(const Frame& frame) final;

    int32_t GenerateCubeShadowMapArray(const uint32_t width,
                                       const uint32_t height,
                                       const uint32_t count) final;
    int32_t GenerateShadowMapArray(const uint32_t width, const uint32_t height,
                                   const uint32_t count) final;
    void BeginShadowMap(const int32_t light_index, const int32_t shadowmap,
                        const uint32_t width, const uint32_t height,
                        const int32_t layer_index, const Frame& frame) final;
    void EndShadowMap(const int32_t shadowmap, const int32_t layer_index) final;
    void SetShadowMaps(const Frame& frame) final;
    void DestroyShadowMap(int32_t& shadowmap) final;

    // skybox
    void DrawSkyBox(const Frame& frame) final;

    // compute shader tasks
    int32_t GenerateAndBindTextureForWrite(const char* id,
                                           const uint32_t slot_index,
                                           const uint32_t width,
                                           const uint32_t height) final;

    void Dispatch(const uint32_t width, const uint32_t height,
                  const uint32_t depth) final;

   private:
    void BeginScene(const Scene& scene) final;
    void EndScene() final;

    void BeginFrame(const Frame& frame) final;
    void EndFrame(const Frame& frame) final;

    void BeginPass(const Frame& frame) final;
    void EndPass(const Frame& frame) final {}

    void initializeGeometries(const Scene& scene) final;
    void initializeSkyBox(const Scene& scene) final;

    void SetPerFrameConstants(const Frame& frame);
    void SetLightInfo(const Frame& frame);

    HRESULT CreateDescriptorHeaps();
    HRESULT CreateRenderTarget();
    HRESULT CreateDepthStencil();
    HRESULT CreateGraphicsResources();

    uint32_t CreateSamplerBuffer();
    int32_t CreateTextureBuffer(SceneObjectTexture& texture);
    uint32_t CreateConstantBuffer();
    size_t CreateIndexBuffer(const void* pData, size_t size, int32_t index_size); 
    size_t CreateIndexBuffer(const SceneObjectIndexArray& index_array);
    size_t CreateVertexBuffer(const void* pData, size_t size, int32_t stride);
    size_t CreateVertexBuffer(const SceneObjectVertexArray& v_property_array);

    HRESULT WaitForPreviousFrame(uint32_t frame_index);
    HRESULT CreatePSO(D3d12PipelineState& pipelineState);
    HRESULT CreateCommandList();
    HRESULT MsaaResolve();

   private:
    ID3D12Device* m_pDev =
        nullptr;  // the pointer to our Direct3D device interface
#if defined(_DEBUG)
    ID3D12Debug* m_pDebugController = nullptr;
    ID3D12DebugDevice* m_pDebugDev = nullptr;
#endif
    D3D12_VIEWPORT m_ViewPort;  // viewport structure
    D3D12_RECT m_ScissorRect;   // scissor rect structure
    IDXGISwapChain3* m_pSwapChain =
        nullptr;  // the pointer to the swap chain interface
    ID3D12Resource*
        m_pRenderTargets[GfxConfiguration::kMaxInFlightFrameCount *
                         2];  // the pointer to rendering buffer. [descriptor]
    ID3D12Resource* m_pDepthStencilBuffer
        [GfxConfiguration::kMaxInFlightFrameCount];  // the pointer to the depth
                                                     // stencil buffer
    ID3D12CommandAllocator* m_pGraphicsCommandAllocator
        [GfxConfiguration::kMaxInFlightFrameCount];  // the pointer to command
                                                     // buffer allocator
    ID3D12CommandAllocator*
        m_pComputeCommandAllocator;                   // the pointer to command
                                                      // buffer allocator
    ID3D12CommandAllocator* m_pCopyCommandAllocator;  // the pointer to command
                                                      // buffer allocator
    ID3D12GraphicsCommandList* m_pGraphicsCommandList
        [GfxConfiguration::kMaxInFlightFrameCount];  // a list to store GPU
                                                     // commands, which will be
                                                     // submitted to GPU to
                                                     // execute when done
    ID3D12GraphicsCommandList* m_pComputeCommandList;
    ID3D12GraphicsCommandList* m_pCopyCommandList;

    ID3D12CommandQueue*
        m_pGraphicsCommandQueue;                 // the pointer to command queue
    ID3D12CommandQueue* m_pComputeCommandQueue;  // the pointer to command queue
    ID3D12CommandQueue* m_pCopyCommandQueue;     // the pointer to command queue

    ID3D12DescriptorHeap*
        m_pRtvHeap[GfxConfiguration::kMaxInFlightFrameCount];  // an array of
                                                               // descriptors of
                                                               // GPU objects
    ID3D12DescriptorHeap*
        m_pDsvHeap[GfxConfiguration::kMaxInFlightFrameCount];  // an array of
                                                               // descriptors of
                                                               // GPU objects

    ID3D12DescriptorHeap* m_pCbvSrvUavHeap
        [GfxConfiguration::kMaxInFlightFrameCount];  // an array of descriptors
                                                     // of GPU objects

    ID3D12DescriptorHeap* m_pSamplerHeap
        [GfxConfiguration::kMaxInFlightFrameCount];  // an array of descriptors
                                                     // of GPU objects

    uint32_t m_nRtvDescriptorSize;
    uint32_t m_nCbvSrvUavDescriptorSize;
    uint32_t m_nSamplerDescriptorSize;

    std::vector<ID3D12Resource*>
        m_Buffers;  // the pointer to the GPU buffer other than texture
    std::vector<ID3D12Resource*>
        m_Textures;  // the pointer to the Texture buffer
    std::vector<D3D12_VERTEX_BUFFER_VIEW>
        m_VertexBufferView;  // vertex buffer descriptors
    std::vector<D3D12_INDEX_BUFFER_VIEW>
        m_IndexBufferView;  // index buffer descriptors

    struct D3dDrawBatchContext : public DrawBatchContext {
        uint32_t index_count;
        size_t index_offset;
        uint32_t property_count;
        size_t property_offset;
        ID3D12DescriptorHeap* pCbvSrvUavHeap;

        ~D3dDrawBatchContext();
    };

    D3dDrawBatchContext m_dbcSkyBox;

    uint8_t* m_pPerFrameCbvDataBegin[GfxConfiguration::kMaxInFlightFrameCount];
    ID3D12Resource* m_pPerFrameConstantUploadBuffer
        [GfxConfiguration::kMaxInFlightFrameCount];

    uint8_t* m_pLightInfoBegin[GfxConfiguration::kMaxInFlightFrameCount];
    ID3D12Resource*
        m_pLightInfoUploadBuffer[GfxConfiguration::kMaxInFlightFrameCount];

#ifdef DEBUG
    uint8_t* m_pDebugConstantsBegin[GfxConfiguration::kMaxInFlightFrameCount];
    ID3D12Resource*
        m_pDebugConstantsUploadBuffer[GfxConfiguration::kMaxInFlightFrameCount];
#endif

    uint8_t* m_pShadowConstantsBegin[GfxConfiguration::kMaxInFlightFrameCount];
    ID3D12Resource*
        m_pShadowDataUploadBuffer[GfxConfiguration::kMaxInFlightFrameCount];

    // Synchronization objects
    HANDLE m_hFenceEvent;
    ID3D12Fence* m_pFence[GfxConfiguration::kMaxInFlightFrameCount];
    uint64_t m_nFenceValue[GfxConfiguration::kMaxInFlightFrameCount];
};
}  // namespace My
