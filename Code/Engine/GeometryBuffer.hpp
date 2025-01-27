#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Renderer.hpp"

namespace x {
    class GeometryBuffer {
        ComPtr<ID3D11Buffer> _vertexBuffer;
        ComPtr<ID3D11Buffer> _indexBuffer;
        u32 _stride     = 0;
        u32 _offset     = 0;
        u32 _indexCount = 0;

    public:
        GeometryBuffer() = default;

        void Create(const Renderer& renderer,
                    const void* vertexData,
                    size_t vertexStride,
                    size_t vertexCount,
                    const u32* indexData,
                    size_t indexCount) {
            _stride = vertexStride;

            D3D11_BUFFER_DESC vbd{};
            vbd.Usage     = D3D11_USAGE_IMMUTABLE;
            vbd.ByteWidth = vertexCount * vertexStride;
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA vd{};
            vd.pSysMem = vertexData;

            auto* device = renderer.GetDevice();
            PANIC_IF_FAILED(device->CreateBuffer(&vbd, &vd, &_vertexBuffer), "Failed to create vertex buffer.")

            D3D11_BUFFER_DESC ibd{};
            ibd.Usage     = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = indexCount * sizeof(u32);
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA id{};
            id.pSysMem = indexData;

            PANIC_IF_FAILED(device->CreateBuffer(&ibd, &id, &_indexBuffer), "Failed to create index buffer.")

            _indexCount = indexCount;
        }

        void Bind(const Renderer& renderer) {
            auto* context = renderer.GetContext();
            context->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &_stride, &_offset);
            context->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }

        [[nodiscard]] ID3D11Buffer* GetVertexBuffer() {
            return _vertexBuffer.Get();
        }

        [[nodiscard]] ID3D11Buffer* GetIndexBuffer() {
            return _indexBuffer.Get();
        }

        [[nodiscard]] u32 GetIndexCount() const {
            return _indexCount;
        }
    };
}