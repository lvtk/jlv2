/*
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

/*
    BEGIN_JUCE_MODULE_DECLARATION

    ID:               jlv2_host
    vendor:           LV2 Toolkit
    version:          0.0.1
    name:             LV2 Plugins
    description:      Adds LV2 Plugin Hosting to JUCE
    website:          https://lvtoolkit.org
    license:          GPL

    dependencies:     juce_core
    OSXFrameworks:
    iOSFrameworks:
    linuxLibs:
    mingwLibs:

    END_JUCE_MODULE_DECLARATION
*/

#pragma once
#define JLV2_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

// DLL building settings on Windows
#if JUCE_MSVC
 #ifdef JUCE_DLL_BUILD
  #define JLV2_EXPORT __declspec (dllexport)
  #pragma warning (disable: 4251)
 #elif defined (JUCE_DLL)
  #define JLV2_EXPORT __declspec (dllimport)
  #pragma warning (disable: 4251)
 #endif
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 1125) // (virtual override warning)
 #endif
#else // if defined (JUCE_DLL) || defined (JUCE_DLL_BUILD)
 #define JLV2_EXPORT __attribute__ ((visibility("default")))
#endif

//==============================================================================
#ifndef JLV2_EXPORT
 #define JLV2_EXPORT   /**< This macro is added to all JUCE public class declarations. */
#endif

namespace jlv2 {
using namespace juce;
class World;
class SymbolMap;
}

#include "host/LV2PluginFormat.h"
