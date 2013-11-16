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

#ifndef LVTK_JUCE_LV2WORLD_H
#define LVTK_JUCE_LV2WORLD_H

    class  LV2Module;

    /** Slim wrapper around LilvWorld.  Publishes commonly used LilvNodes */
    class LV2World
    {
    public:

        LV2World();
        ~LV2World();

        const LilvNode*   lv2_InputPort;
        const LilvNode*   lv2_OutputPort;
        const LilvNode*   lv2_AudioPort;
        const LilvNode*   lv2_AtomPort;
        const LilvNode*   lv2_ControlPort;
        const LilvNode*   lv2_EventPort;
        const LilvNode*   lv2_CVPort;
        const LilvNode*   midi_MidiEvent;
        const LilvNode*   work_schedule;
        const LilvNode*   work_interface;

        /** Create an LV2Module for a uri string */
        LV2Module* createModule (const String& uri);

        /** Get an LilvPlugin for a uri string */
        const LilvPlugin* getPlugin (const String& uri) const;
        
        /** Get all Available Plugins */
        const LilvPlugins* getAllPlugins() const;

        /** Returns true if a feature is supported */
        bool isFeatureSupported (const String& featureURI);
        
        /** Returns true if the plugin uri is availble */
        bool isPluginAvailable (const String& uri);
        
        /** Returns true if the plugin is supported on this system */
        bool isPluginSupported (const String& uri);

        /** Return the underlying LilvWorld* pointer */
        inline LilvWorld* getLilvWorld() const { return world; }

        /** Add a supported feature */
        inline void addFeature (LV2Feature* feat, bool rebuild = true) { features.add (feat, rebuild); }
        
        /** Get supported features */
        inline LV2FeatureArray& getFeatures() { return features; }
        
    private:

        LilvWorld* world;
        LV2FeatureArray features;

    };

#endif /* LVTK_JUCE_LV2WORLD_H */
