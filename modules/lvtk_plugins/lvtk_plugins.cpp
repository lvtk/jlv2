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

#include "AppConfig.h"
#include "lvtk_plugins.h"

namespace juce {
namespace lvtk {

#include "host/lvtk_LV2Module.cpp"
#include "host/lvtk_LV2PluginFormat.cpp"
#include "host/lvtk_LV2PluginModel.cpp"
#include "host/lvtk_LV2World.cpp"
    
#include "features/lvtk_LV2Log.cpp"
#include "features/lvtk_LV2Worker.cpp"
    
}}
