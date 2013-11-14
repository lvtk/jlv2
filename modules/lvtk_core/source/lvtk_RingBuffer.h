/*
    This file is part of the lvtk_core module for the JUCE Library
    Copyright (c) 2013 - Michael Fisher <mfisher31@gmail.com>.

    Permission to use, copy, modify, and/or distribute this software for any purpose with
    or without fee is hereby granted, provided that the above copyright notice and this
    permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
    TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
    NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
    IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
    CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef LVTK_RINGBUFFER_H
#define LVTK_RINGBUFFER_H

class RingBuffer
{

public:
    
    RingBuffer (uint32 capacity);
    ~RingBuffer();

    void setCapacity (uint32 newCapacity);

    inline bool canRead  (uint32 bytes) const  { return bytes <= (uint32) fifo.getNumReady() && bytes != 0; }
    inline uint32 readSpace() const { return fifo.getNumReady(); }

    inline bool canWrite (uint32 bytes) const { return bytes <= (uint32) fifo.getFreeSpace() && bytes != 0; }
    inline uint32 writeSpace() const { return (uint32) fifo.getFreeSpace(); }

    inline uint32 size() const { return (uint32) fifo.getTotalSize(); }

    inline uint32
    write (const char* src, uint32 bytes)
    {
        buffer = block.getData();
                   
        fifo.prepareToWrite (bytes, vec1.index, vec1.size, vec2.index, vec2.size);

        if (vec1.size > 0)
            memcpy (buffer + vec1.index, src, vec1.size);

        if (vec2.size > 0)
            memcpy (buffer + vec2.index, src + vec1.size, vec2.size);

        fifo.finishedWrite (vec1.size + vec2.size);

        return vec1.size + vec2.size;
    }

    inline uint32 write (const void* src, uint32 bytes)
    {
       return write ((const char*) src, bytes);
    }

    template <typename T>
    uint32 writeType (const T& src)
    {
        write ((char*) &src, sizeof (T));
    }

    inline uint32
    read (char* dest, uint32 size, bool advance = true)
    {
        buffer = block.getData();
        
        fifo.prepareToRead (size, vec1.index, vec1.size, vec2.index, vec2.size);

        if (vec1.size > 0)
            memcpy (dest, buffer + vec1.index, vec1.size);

        if (vec2.size > 0)
            memcpy (dest + vec1.size, buffer + vec2.index, vec2.size);

        if (advance)
            fifo.finishedRead (vec1.size + vec2.size);

        return vec1.size + vec2.size;
    }

    inline uint32
    read (void* dest, uint32 size, bool advance = true)
    {
       return read ((char*) dest, size, advance);
    }

    struct Vector {
        uint32_t size;
        void*    buffer;
    };
#if 0
    void getReadVector (Vector* vec)
    {
        int i1, b1, i2, b2;
        fifo.getReadVector (i1, b1, i2, b2);

        char* buf = (char*) block.getData();

        vec[0].size   = (uint32_t) b1;
        vec[0].buffer = (void*) (buf + i1);
        vec[1].size   = (uint32_t) b2;
        vec[1].buffer = (void*) (buf + i2);
    }

    void getWriteVector (Vector* vec)
    {
        int i1, b1, i2, b2;
        fifo.getWriteVector (i1, b1, i2, b2);

        char* buf = (char*) block.getData();

        vec[0].size   = (uint32_t) b1;
        vec[0].buffer = (void*) (buf + i1);
        vec[1].size   = (uint32_t) b2;
        vec[1].buffer = (void*) (buf + i2);
    }

    void readAdvance (uint32_t bytes)  { fifo.finishedRead ((int) bytes); }
    void writeAdvance (uint32_t bytes) { fifo.finishedWrite ((int) bytes); }
#endif
    juce::CriticalSection& getReadLock() { return readLock; }
    juce::CriticalSection& getWriteLock() { return writeLock; }

private:

    
    
    struct Vec
    {
        int size;
        int index;
    };

    Vec vec1, vec2;
    juce::AbstractFifo fifo;
    juce::HeapBlock<char> block;
    char* buffer;
    juce::CriticalSection readLock, writeLock;
    
};

#endif /* LVTK_RINGBUFFER_H */
