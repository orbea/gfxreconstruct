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

#include "decode/resource_util.h"

#include "util/platform.h"

#include <algorithm>

GFXRECON_BEGIN_NAMESPACE(gfxrecon)
GFXRECON_BEGIN_NAMESPACE(decode)
GFXRECON_BEGIN_NAMESPACE(resource)

void CopyImageSubresourceMemory(uint8_t*       dst,
                                const uint8_t* src,
                                size_t         offset,
                                size_t         size,
                                size_t         dst_row_pitch,
                                size_t         src_row_pitch,
                                uint32_t       height)
{
    if (src_row_pitch == dst_row_pitch)
    {
        // Determine the aligned size of the destination subresource as row_pitch * height to ensure that we don't write
        // past the end of the resource in the case that the capture and replay resources had different slice pitches,
        // and the data size matches the size of a capture resource with a larger slice pitch.
        size_t subresource_size = height * dst_row_pitch;
        size_t copy_size        = std::min(size, (subresource_size - offset));

        // Copy entire range without adjustment.
        util::platform::MemoryCopy(dst + offset, copy_size, src, copy_size);
    }
    else
    {
        size_t copy_row_pitch = std::min(dst_row_pitch, src_row_pitch);

        size_t current_row = offset / src_row_pitch;
        size_t row_offset  = offset % src_row_pitch;

        if (row_offset >= copy_row_pitch)
        {
            // When the dst row pitch is smaller than the src row pitch, and the offset points to
            // padding at the end of the row, which is outside the bounds of the dst row pitch, we
            // advance to the start of the next row.  If the write was only to the padding, we set
            // row_offset to zero and advance to the next row without copying anything.
            size -= std::min(src_row_pitch - row_offset, size);
            row_offset = 0;
            ++current_row;
        }

        const uint8_t* copy_src = src;
        uint8_t*       copy_dst = reinterpret_cast<uint8_t*>(dst) + (current_row * dst_row_pitch) + row_offset;

        // Process first partial row.
        if (row_offset > 0)
        {
            // Handle row with both partial begin and end positions.
            size_t copy_size = std::min(copy_row_pitch - row_offset, size);
            util::platform::MemoryCopy(copy_dst, copy_size, copy_src, copy_size);

            copy_src += src_row_pitch - row_offset;
            copy_dst += dst_row_pitch - row_offset;

            size -= std::min(src_row_pitch - row_offset, size);

            ++current_row;
        }

        // Process remaining rows.
        if (size > 0)
        {
            size_t total_rows    = size / src_row_pitch;
            size_t row_remainder = size % src_row_pitch;

            // Ensure that we don't write past the end of the resource memory for aligned sizes that produce a
            // total_rows value that is greater than the subresource height.
            size_t subresource_rows = height - current_row;
            if (total_rows >= subresource_rows)
            {
                total_rows    = subresource_rows;
                row_remainder = 0;
            }

            // First process the complete rows.
            for (size_t i = 0; i < total_rows; ++i)
            {
                size_t copy_size = copy_row_pitch;
                util::platform::MemoryCopy(copy_dst, copy_size, copy_src, copy_size);

                copy_src += src_row_pitch;
                copy_dst += dst_row_pitch;
            }

            // Process a partial end row.
            if (row_remainder != 0)
            {
                size_t copy_size = std::min(copy_row_pitch, row_remainder);
                util::platform::MemoryCopy(copy_dst, copy_size, copy_src, copy_size);
            }
        }
    }
}

GFXRECON_END_NAMESPACE(resource)
GFXRECON_END_NAMESPACE(decode)
GFXRECON_END_NAMESPACE(gfxrecon)
