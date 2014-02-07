/*
 * buffer.h
 *
 *  Created on: Jan 28, 2014
 *      Author: anshul
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include "utils.h"

class Buffer
{
public:
    Buffer();
    ~Buffer();
    void* GetData();
    size_t GetSize();
    int GetSamples();
    VC_STATUS SetSamples(int samples);
    VC_STATUS Reset();
    VC_STATUS WriteData(void* buf, size_t size);
    BUF_TAG GetTag();
    VC_STATUS SetTag(BUF_TAG tag);

private:
    const char*  c_str()
    {
        return ("Buffer");
    }
    void* m_data;
    size_t m_size;
    int m_samples;
    BUF_TAG m_tag;

};


#endif /* BUFFER_H_ */
