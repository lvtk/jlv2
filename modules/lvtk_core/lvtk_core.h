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

#ifndef LVTK_CORE_H_INCLUDED
#define LVTK_CORE_H_INCLUDED

#ifndef JUCE_MODULE_AVAILABLE_lvtk_core
 #ifdef _MSC_VER
  #pragma message ("Have you included your AppConfig.h file before including JUCE headers?")
 #else
  #warning "Have you included your AppConfig.h file before including JUCE headers?"
 #endif
#endif

/** Config: LVTK_USE_CXX11
    Set this if to enable C++ 11 language features.  The default is enabled.
 */
#ifndef LVTK_USE_CXX11
 #define LVTK_USE_CXX11 1
#endif

#include <functional>
#include <map>

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/event/event.h>
#include <lv2/lv2plug.in/ns/ext/log/log.h>
#include <lv2/lv2plug.in/ns/ext/morph/morph.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/uri-map/uri-map.h>
#include <lv2/lv2plug.in/ns/ext/worker/worker.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>

#include "modules/juce_core/juce_core.h"

namespace juce {
namespace lvtk {
    
#include "source/lvtk_URIs.h"
#include "source/lvtk_PortType.h"
#include "source/lvtk_PortBuffer.h"
#include "source/lvtk_Parameter.h"
#include "source/lvtk_PortWriter.h"
#include "source/lvtk_RingBuffer.h"
#include "source/lvtk_Semaphore.h"
#include "source/lvtk_WorkThread.h"

}}

#endif
