/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#pragma once

#include <atomic>
#include <array>
#include <memory>

#include "PipelineResourceSignature.h"
#include "ObjectBase.hpp"
#include "STDAllocator.hpp"
#include "Serializer.hpp"
#include "DeviceObjectArchiveBase.hpp"
#include "SerializationDeviceImpl.hpp"

namespace Diligent
{

class SerializableResourceSignatureImpl final : public ObjectBase<IPipelineResourceSignature>
{
public:
    using TBase = ObjectBase<IPipelineResourceSignature>;

    using DeviceType                    = DeviceObjectArchiveBase::DeviceType;
    static constexpr Uint32 DeviceCount = static_cast<Uint32>(DeviceType::Count);

    SerializableResourceSignatureImpl(IReferenceCounters*                  pRefCounters,
                                      SerializationDeviceImpl*             pDevice,
                                      const PipelineResourceSignatureDesc& Desc,
                                      ARCHIVE_DEVICE_DATA_FLAGS            DeviceFlags,
                                      SHADER_TYPE                          ShaderStages = SHADER_TYPE_UNKNOWN);

    SerializableResourceSignatureImpl(IReferenceCounters* pRefCounters, const char* Name) noexcept;

    ~SerializableResourceSignatureImpl() override;

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(IID_PipelineResourceSignature, TBase)

    virtual const PipelineResourceSignatureDesc& DILIGENT_CALL_TYPE GetDesc() const override final;

    virtual void DILIGENT_CALL_TYPE CreateShaderResourceBinding(IShaderResourceBinding** ppShaderResourceBinding,
                                                                bool                     InitStaticResources) override final {}

    virtual void DILIGENT_CALL_TYPE BindStaticResources(SHADER_TYPE                 ShaderStages,
                                                        IResourceMapping*           pResourceMapping,
                                                        BIND_SHADER_RESOURCES_FLAGS Flags) override final {}

    virtual IShaderResourceVariable* DILIGENT_CALL_TYPE GetStaticVariableByName(SHADER_TYPE ShaderType,
                                                                                const Char* Name) override final { return nullptr; }

    virtual IShaderResourceVariable* DILIGENT_CALL_TYPE GetStaticVariableByIndex(SHADER_TYPE ShaderType,
                                                                                 Uint32      Index) override final { return nullptr; }

    virtual Uint32 DILIGENT_CALL_TYPE GetStaticVariableCount(SHADER_TYPE ShaderType) const override final { return 0; }

    virtual void DILIGENT_CALL_TYPE InitializeStaticSRBResources(IShaderResourceBinding* pShaderResourceBinding) const override final {}

    virtual bool DILIGENT_CALL_TYPE IsCompatibleWith(const IPipelineResourceSignature* pPRS) const override final { return false; }

    virtual Int32 DILIGENT_CALL_TYPE GetUniqueID() const override final { return 0; }

    virtual void DILIGENT_CALL_TYPE SetUserData(IObject* pUserData) override final {}

    virtual IObject* DILIGENT_CALL_TYPE GetUserData() const override final { return nullptr; }

    bool   IsCompatible(const SerializableResourceSignatureImpl& Rhs, ARCHIVE_DEVICE_DATA_FLAGS DeviceFlags) const;
    bool   operator==(const SerializableResourceSignatureImpl& Rhs) const;
    size_t CalcHash() const;

    const SerializedData& GetCommonData() const { return m_CommonData; }

    const SerializedData* GetDeviceData(DeviceType Type) const
    {
        VERIFY_EXPR(static_cast<Uint32>(Type) < DeviceCount);
        auto& Wrpr = m_pDeviceSignatures[static_cast<size_t>(Type)];
        return Wrpr ? &Wrpr->Data : nullptr;
    }

    template <typename SignatureType>
    struct SignatureTraits;

    template <typename SignatureType>
    SignatureType* GetDeviceSignature(DeviceType Type) const
    {
        constexpr auto TraitsType = SignatureTraits<SignatureType>::Type;
        VERIFY_EXPR(Type == TraitsType || (Type == DeviceType::Metal_iOS && TraitsType == DeviceType::Metal_MacOS));

        return ClassPtrCast<SignatureType>(GetDeviceSignature(Type));
    }

    const char* GetName() const { return m_Name.c_str(); }

    template <typename SignatureImplType>
    void CreateDeviceSignature(DeviceType                           Type,
                               const PipelineResourceSignatureDesc& Desc,
                               SHADER_TYPE                          ShaderStages);

private:
    IPipelineResourceSignature* GetDeviceSignature(DeviceType Type) const
    {
        VERIFY_EXPR(static_cast<Uint32>(Type) < DeviceCount);
        const auto& Wrpr = m_pDeviceSignatures[static_cast<size_t>(Type)];
        return Wrpr ? Wrpr->GetPRS() : nullptr;
    }

    void InitCommonData(const PipelineResourceSignatureDesc& Desc);

private:
    const std::string m_Name;

    const PipelineResourceSignatureDesc* m_pDesc = nullptr;

    SerializedData m_CommonData;

    struct PRSWapperBase
    {
        virtual ~PRSWapperBase() {}
        virtual IPipelineResourceSignature* GetPRS() = 0;

        SerializedData Data;
    };

    template <typename ImplType> struct TPRS;

    std::array<std::unique_ptr<PRSWapperBase>, DeviceCount> m_pDeviceSignatures;

    mutable std::atomic<size_t> m_Hash{0};
};

#define INSTANTIATE_GET_DEVICE_SIGNATURE(PRSImplType) template PRSImplType* SerializableResourceSignatureImpl::GetDeviceSignature<PRSImplType>(DeviceType Type) const
#define DECLARE_GET_DEVICE_SIGNATURE(PRSImplType)     extern INSTANTIATE_GET_DEVICE_SIGNATURE(PRSImplType)

#define INSTANTIATE_CREATE_DEVICE_SIGNATURE(PRSImplType)                                 \
    template void SerializableResourceSignatureImpl::CreateDeviceSignature<PRSImplType>( \
        DeviceType                           Type,                                       \
        const PipelineResourceSignatureDesc& Desc,                                       \
        SHADER_TYPE                          ShaderStages)
#define DECLARE_CREATE_DEVICE_SIGNATURE(PRSImplType) extern INSTANTIATE_CREATE_DEVICE_SIGNATURE(PRSImplType)

#define DECLARE_DEVICE_SIGNATURE_METHODS(PRSImplType) \
    class PRSImplType;                                \
    DECLARE_GET_DEVICE_SIGNATURE(PRSImplType);        \
    DECLARE_CREATE_DEVICE_SIGNATURE(PRSImplType);

#define INSTANTIATE_DEVICE_SIGNATURE_METHODS(PRSImplType) \
    INSTANTIATE_GET_DEVICE_SIGNATURE(PRSImplType);        \
    INSTANTIATE_CREATE_DEVICE_SIGNATURE(PRSImplType);

#if D3D11_SUPPORTED
DECLARE_DEVICE_SIGNATURE_METHODS(PipelineResourceSignatureD3D11Impl)
#endif

#if D3D12_SUPPORTED
DECLARE_DEVICE_SIGNATURE_METHODS(PipelineResourceSignatureD3D12Impl)
#endif

#if GL_SUPPORTED || GLES_SUPPORTED
DECLARE_DEVICE_SIGNATURE_METHODS(PipelineResourceSignatureGLImpl)
#endif

#if VULKAN_SUPPORTED
DECLARE_DEVICE_SIGNATURE_METHODS(PipelineResourceSignatureVkImpl)
#endif

#if METAL_SUPPORTED
DECLARE_DEVICE_SIGNATURE_METHODS(PipelineResourceSignatureMtlImpl)
#endif

} // namespace Diligent
