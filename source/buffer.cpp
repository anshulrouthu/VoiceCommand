/*
 * buffer.cpp
 *
 *  Created on: Jan 28, 2014
 *      Author: anshul
 */

#include "buffer.h"

Buffer::Buffer() :
    m_data(NULL),
    m_size(0),
    m_tag(TAG_NONE)
{
    m_data = malloc(100 * 1024);
    VC_CHECK(m_data == NULL,, "Error allocating buffer");
}

Buffer::~Buffer()
{
    if (m_data)
        free(m_data);
}

void* Buffer::GetData()
{
    return (m_data);
}

VC_STATUS Buffer::WriteData(void* buf, size_t size)
{
    VC_CHECK(!memcpy(m_data, buf, size),return VC_FAILURE,"Error writing data to buffer");
    m_size = size;
    return (VC_SUCCESS);
}

size_t Buffer::GetSize()
{
    return (m_size);
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
