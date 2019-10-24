/*
    Copyright (C) 2014-2019  Michael Fisher <mfisher@kushview.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#if defined (JLV2_HOST_H_INCLUDED) && ! JUCE_AMALGAMATED_INCLUDE
/* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "jlv2_host/jlv2_host.h"

#if JLV2_PLUGINHOST_LV2

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/extensions/units/units.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/data-access/data-access.h>
#include <lv2/lv2plug.in/ns/ext/event/event.h>
#include <lv2/lv2plug.in/ns/ext/instance-access/instance-access.h>
#include <lv2/lv2plug.in/ns/ext/log/log.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/uri-map/uri-map.h>
#include <lv2/lv2plug.in/ns/ext/worker/worker.h>

#include <lilv/lilv.h>
#include <suil/suil.h>

namespace jlv2 {
class Module;
class ModuleUI;
}

#include "host/PortType.h"
#include "host/PortBuffer.h"
#include "host/PortEvent.h"
#include "host/LV2Features.h"
#include "host/SymbolMap.h"
#include "host/RingBuffer.h"
#include "host/WorkThread.h"
#include "host/LogFeature.h"
#include "host/WorkerFeature.h"
#include "host/World.h"
#include "host/Module.h"

#include "host/LogFeature.cpp"
#include "host/LV2PluginFormat.cpp"
#include "host/Module.cpp"
#include "host/PortBuffer.cpp"
#include "host/RingBuffer.cpp"
#include "host/WorkerFeature.cpp"
#include "host/WorkThread.cpp"
#include "host/World.cpp"

#endif
