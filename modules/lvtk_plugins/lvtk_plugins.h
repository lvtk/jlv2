/*
    This file is part of the lvtk_plugins JUCE module
    Copyright (C) 2013  Michael Fisher <mfisher31@gmail.com>

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

#ifndef LVTK_JUCE_PLUGINS_H_INCLUDED
#define LVTK_JUCE_PLUGINS_H_INCLUDED

#ifndef JUCE_MODULE_AVAILABLE_lvtk_plugins
#ifdef _MSC_VER
#pragma message ("Have you included your AppConfig.h file before including the JUCE headers?")
#else
#warning "Have you included your AppConfig.h file before including the JUCE headers?"
#endif
#endif

#include "juce_audio_processors/juce_audio_processors.h"
#include "../lvtk_core/lvtk_core.h"

#include <lilv/lilvmm.hpp>
#include <lilv/lilv.h>
#include <suil/suil.h>

namespace juce {
namespace lvtk {

#include "features/lvtk_LV2Features.h"
#include "features/lvtk_SymbolMap.h"
#include "host/lvtk_LV2World.h"
#include "host/lvtk_LV2Module.h"
#include "host/lvtk_LV2PluginFormat.h"
    
}}

#endif
