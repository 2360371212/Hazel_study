#pragma once

#include "AssetHandle.h"
#include "AssetType.h"

namespace Hazel
{
    class AssetManager;

    class Asset
    {
    public:
        virtual ~Asset() = default;

        AssetHandle GetHandle() const {
            return m_Handle;
        }

        virtual AssetType GetAssetType() const = 0;

    protected:
        void SetHandle(AssetHandle handle)
        {
            m_Handle = handle;
        }

    private:
        AssetHandle m_Handle = InvalidAssetHandle;
        friend class AssetManager;
    };
}