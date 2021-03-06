/*
** Copyright (c) 2020 LunarG, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "decode/vulkan_feature_util.h"
#include "util/logging.h"

#include <cassert>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)
GFXRECON_BEGIN_NAMESPACE(feature_util)

VkResult GetInstanceExtensions(PFN_vkEnumerateInstanceExtensionProperties instance_extension_proc,
                               std::vector<VkExtensionProperties>*        properties)
{
    assert(properties != nullptr);

    VkResult result = VK_ERROR_INITIALIZATION_FAILED;

    if (instance_extension_proc != nullptr)
    {
        uint32_t count = 0;
        result         = instance_extension_proc(nullptr, &count, nullptr);

        if ((result == VK_SUCCESS) && (count > 0))
        {
            properties->resize(count);
            result = instance_extension_proc(nullptr, &count, properties->data());
        }
    }

    return result;
}

VkResult GetDeviceExtensions(VkPhysicalDevice                         physical_device,
                             PFN_vkEnumerateDeviceExtensionProperties device_extension_proc,
                             std::vector<VkExtensionProperties>*      properties)
{
    assert(properties != nullptr);

    VkResult result = VK_ERROR_INITIALIZATION_FAILED;

    if ((physical_device != VK_NULL_HANDLE) && (device_extension_proc != nullptr))
    {
        uint32_t count = 0;
        result         = device_extension_proc(physical_device, nullptr, &count, nullptr);

        if ((result == VK_SUCCESS) && (count > 0))
        {
            properties->resize(count);
            result = device_extension_proc(physical_device, nullptr, &count, properties->data());
        }
    }

    return result;
}

void RemoveUnsupportedExtensions(const std::vector<VkExtensionProperties>& properties,
                                 std::vector<const char*>*                 extensions)
{
    assert(extensions != nullptr);

    auto extensionIter = extensions->begin();
    while (extensionIter != extensions->end())
    {
        bool found = false;
        for (const auto& property : properties)
        {
            if (strcmp(property.extensionName, *extensionIter) == 0)
            {
                found = true;
                break;
            }
        }

        if (found == false)
        {
            GFXRECON_LOG_WARNING("Extension %s, which is not supported by the replay device, will not be enabled",
                                 *extensionIter);
            extensionIter = extensions->erase(extensionIter);
        }
        else
        {
            ++extensionIter;
        }
    }
}

GFXRECON_END_NAMESPACE(feature_util)
GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)
