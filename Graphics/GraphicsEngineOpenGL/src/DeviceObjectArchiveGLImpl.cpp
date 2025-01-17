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

#include "pch.h"
#include "RenderDeviceGLImpl.hpp"
#include "DeviceObjectArchiveGLImpl.hpp"
#include "PipelineResourceSignatureGLImpl.hpp"

namespace Diligent
{

DeviceObjectArchiveGLImpl::DeviceObjectArchiveGLImpl(IReferenceCounters* pRefCounters, IArchive* pSource) :
    DeviceObjectArchiveBase{pRefCounters, pSource, DeviceType::OpenGL}
{
}

DeviceObjectArchiveGLImpl::~DeviceObjectArchiveGLImpl()
{
}

RefCntAutoPtr<IPipelineResourceSignature> DeviceObjectArchiveGLImpl::UnpackResourceSignature(const ResourceSignatureUnpackInfo& DeArchiveInfo, bool IsImplicit)
{
    return DeviceObjectArchiveBase::UnpackResourceSignatureImpl<RenderDeviceGLImpl, PRSSerializerGL<SerializerMode::Read>>(DeArchiveInfo, IsImplicit);
}

RefCntAutoPtr<IShader> DeviceObjectArchiveGLImpl::UnpackShader(Serializer<SerializerMode::Read>& Ser,
                                                               ShaderCreateInfo&                 ShaderCI,
                                                               IRenderDevice*                    pDevice)
{
    Ser(ShaderCI.UseCombinedTextureSamplers, ShaderCI.CombinedSamplerSuffix);

    ShaderCI.Source       = static_cast<const Char*>(Ser.GetCurrentPtr());
    ShaderCI.SourceLength = Ser.GetRemainingSize() - 1;
    VERIFY_EXPR(ShaderCI.SourceLength == strlen(ShaderCI.Source));

    ShaderCI.CompileFlags &= ~SHADER_COMPILE_FLAG_SKIP_REFLECTION; // AZ TODO: remove

    RefCntAutoPtr<IShader> pShader;
    pDevice->CreateShader(ShaderCI, &pShader);
    return pShader;
}

template <SerializerMode Mode>
void PRSSerializerGL<Mode>::SerializeInternalData(
    Serializer<Mode>&                                   Ser,
    ConstQual<PipelineResourceSignatureInternalDataGL>& InternalData,
    DynamicLinearAllocator*                             Allocator)
{
    PRSSerializer<Mode>::SerializeInternalData(Ser, InternalData, Allocator);

    Ser.SerializeArrayRaw(Allocator, InternalData.pResourceAttribs, InternalData.NumResources);

    ASSERT_SIZEOF64(InternalData, 48, "Did you add a new member to PipelineResourceSignatureInternalDataGL? Please add serialization here.");
}

template struct PRSSerializerGL<SerializerMode::Read>;
template struct PRSSerializerGL<SerializerMode::Write>;
template struct PRSSerializerGL<SerializerMode::Measure>;

} // namespace Diligent
