/*********************************************************************
VoiceCommand ( A Voice command utility using Google API )

Copyright (C) 2014 Anshul Routhu <anshul.m67@gmail.com>

All rights reserved.

This file buffer.cpp is part of VoiceCommand project

VoiceCommand is a free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

/*
 * buffer.cpp
 *
 *  Created on: Jan 28, 2014
 *      Author: anshul
 */

#include "buffer.h"

Buffer::Buffer(size_t size) :
    m_size(0),
    m_tag(TAG_NONE),
    m_samples(0)
{
    m_data = malloc(size);
    VC_CHECK(m_data == NULL,, "Error allocating buffer");
}

Buffer::~Buffer()
{
    if (m_data)
    {
        free(m_data);
        m_data = NULL;
    }
}

void* Buffer::GetData()
{
    return (m_data);
}

VC_STATUS Buffer::WriteData(void* buf, size_t size)
{
    VC_CHECK(!memcpy(m_data, buf, size), return VC_FAILURE, "Error writing data to buffer");
    m_size = size;
    return (VC_SUCCESS);
}

size_t Buffer::GetSize()
{
    return (m_size);
}

size_t Buffer::GetMaxSize()
{
    return (VC_BUFFER_MAXSIZE);
}

int Buffer::GetSamples()
{
    return (m_samples);
}

VC_STATUS Buffer::SetSamples(int samples)
{
    m_samples = samples;
    return (VC_SUCCESS);
}

VC_STATUS Buffer::SetSize(size_t size)
{
    m_size = size;
    return (VC_SUCCESS);
}

VC_STATUS Buffer::Reset()
{
    m_size = 0;
    m_samples = 0;
    m_tag = TAG_NONE;
    return (VC_SUCCESS);
}

BUF_TAG Buffer::GetTag()
{
    return (m_tag);
}

VC_STATUS Buffer::SetTag(BUF_TAG tag)
{
    m_tag = tag;
    return (VC_SUCCESS);
}
