/*
 *  Copyright 2019-2022 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
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

#include "ResourceMapping.h"

int TestObjectCInterface(struct IObject* pObject);

int TestResourceMappingCInterface(struct IResourceMapping* pResourceMapping)
{
    IObject*                  pUnknown = NULL;
    ReferenceCounterValueType RefCnt1 = 0, RefCnt2 = 0;

    ResourceMappingDesc SCDesc;

    memset(&SCDesc, 0, sizeof(SCDesc));

    int num_errors = TestObjectCInterface((struct IObject*)pResourceMapping);

    IObject_QueryInterface(pResourceMapping, &IID_Unknown, &pUnknown);
    if (pUnknown != NULL)
        IObject_Release(pUnknown);
    else
        ++num_errors;

    RefCnt1 = IObject_AddRef(pResourceMapping);
    if (RefCnt1 <= 1)
        ++num_errors;
    RefCnt2 = IObject_Release(pResourceMapping);
    if (RefCnt2 <= 0)
        ++num_errors;
    if (RefCnt2 != RefCnt1 - 1)
        ++num_errors;

    return num_errors;
}

void TestResourceMappingC_API(struct IResourceMapping* pResourceMapping)
{
    IDeviceObject* pObject = NULL;

    Uint32 ArraySize  = 4;
    Uint32 ArrayIndex = 6;
    size_t Size       = 0;

    IResourceMapping_AddResource(pResourceMapping, "Resource Name", pObject, true);
    IResourceMapping_AddResourceArray(pResourceMapping, "Resource Array Name", 0, &pObject, ArraySize, true);
    IResourceMapping_RemoveResourceByName(pResourceMapping, "Resource Name", ArrayIndex);
    pObject = IResourceMapping_GetResource(pResourceMapping, "Resource Name", ArrayIndex);
    Size    = IResourceMapping_GetSize(pResourceMapping);
}
