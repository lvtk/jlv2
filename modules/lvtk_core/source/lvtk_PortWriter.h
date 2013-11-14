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

#ifndef LVTK_JUCE_PORT_WRITER_H
#define LVTK_JUCE_PORT_WRITER_H

/** Function type for writing to a port buffer
    Params are in order port, size, protocol, buffer
    @note This is compatible with lvtk's port write ui method */
typedef std::function<void(uint32_t, uint32_t, uint32_t, void const*)> PortWriteFunction;

struct PortEvent
{
    uint32_t index;
    uint32_t protocol;
    union {
        double decimal;
        int64_t frames;
    } time;
    uint32_t size;
};


#endif  /* LVTK_JUCE_PORT_WRITER_H */
