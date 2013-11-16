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

// Change this to disable logging of various LV2 activities
#ifndef LV2_LOGGING
 #define LV2_LOGGING 0
#endif

#if LV2_LOGGING
 #define JUCE_LV2_LOG(a) Logger::writeToLog(a);
#else
 #define JUCE_LV2_LOG(a)
#endif


#ifndef LVTK_JUCE_PLUGIN_INSTANCE_CLASS
#define LVTK_JUCE_PLUGIN_INSTANCE_CLASS AudioPluginInstance
#endif


//==============================================================================
class LV2PluginInstance     : public LVTK_JUCE_PLUGIN_INSTANCE_CLASS
{
public:

    LV2PluginInstance (LV2World& world, LV2Module* module_)
        : wantsMidiMessages (false),
          initialised (false),
          isPowerOn (false),
          tempBuffer (1, 1),
          module (module_)
    {
        assert (module != nullptr);

        numPorts   = module->getNumPorts();
        midiPort   = module->getMidiPort();
        notifyPort = LV2UI_INVALID_PORT_INDEX;

        // TODO: channel/param mapping should all go in LV2Module
        const LilvPlugin* plugin (module->getPlugin());
        for (uint32 p = 0; p < numPorts; ++p)
        {
            const LilvPort* port (module->getPort (p));
            const bool input = module->isPortInput (p);
            const PortType type = module->getPortType (p);
 
            channels.addPort (type, p, input);
            
            if (input)
            {
                
                if (PortType::Control == type)
                {
                    LV2Parameter* param = new LV2Parameter (plugin, port);
                    params.add (param);
                    
                    float min = 0.0f, max = 1.0f, def = 0.0f;
                    module->getPortRange (p, min, max, def);
                    param->setMinMaxValue (min, max, def);
                }
            }
            else
            {
               
            }
        }

        setPlayConfigDetails (channels.getNumAudioInputs(),
                              channels.getNumAudioOutputs(), 44100.0, 1024);
    }


    ~LV2PluginInstance()
    {
        module = nullptr;
    }

    //=========================================================================
    uint32 getNumPorts() const { return module->getNumPorts(); }
    uint32 getNumPorts (PortType type, bool isInput) const { return module->getNumPorts (type, isInput); }
    PortType getPortType (uint32 port) const { return module->getPortType (port); }
    bool isPortInput (uint32 port)     const { return module->isPortInput (port); }
    bool isPortOutput (uint32 port)    const { return module->isPortOutput (port); }

    //=========================================================================
    void fillInPluginDescription (PluginDescription& desc) const
    {
        desc.name = module->getName();

        desc.descriptiveName = String::empty;
        if (desc.descriptiveName.isEmpty())
            desc.descriptiveName = desc.name;

        desc.fileOrIdentifier = module->getURI();
        desc.uid = desc.fileOrIdentifier.hashCode();

//        desc.lastFileModTime = 0;
        desc.pluginFormatName = "LV2";

        desc.category = module->getClassLabel();
        desc.manufacturerName = module->getAuthorName();
        desc.version = "";

        desc.numInputChannels  = module->getNumPorts (PortType::Audio, true);
        desc.numOutputChannels = module->getNumPorts (PortType::Audio, false);
        desc.isInstrument = false; //XXXX
    }

    void initialise()
    {
        if (initialised)
            return;

       #if JUCE_WINDOWS
        // On Windows it's highly advisable to create your plugins using the message thread,
        // because many plugins need a chance to create HWNDs that will get their
        // messages delivered by the main message thread, and that's not possible from
        // a background thread.
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
       #endif

        wantsMidiMessages = midiPort != LV2UI_INVALID_PORT_INDEX;

        initialised = true;
        setLatencySamples (0);
    }

    double getTailLengthSeconds() const { return 0.0f; }
    void* getPlatformSpecificData()  { return module->getHandle(); }
    const String getName() const     { return module->getName(); }
    bool silenceInProducesSilenceOut() const { return false; }
    bool acceptsMidi()  const        { return wantsMidiMessages; }
    bool producesMidi() const        { return notifyPort != LV2UI_INVALID_PORT_INDEX; }

    //==============================================================================
    void prepareToPlay (double sampleRate, int blockSize)
    {
        setPlayConfigDetails (channels.getNumAudioInputs(), channels.getNumAudioOutputs(),
                              sampleRate, blockSize);
        initialise();

        if (initialised)
        {
            module->setSampleRate (sampleRate);
            tempBuffer.setSize (jmax (1, getNumOutputChannels()), blockSize);
            module->activate();
        }
    }

    void releaseResources()
    {
        if (initialised)
            module->deactivate();

        tempBuffer.setSize (1, 1);
    }


    void processBlock (AudioSampleBuffer& audio, MidiBuffer& midi)
    {
        const int32 numSamples = audio.getNumSamples();

        if (! initialised)
        {
            for (int i = 0; i < getNumOutputChannels(); ++i)
                audio.clear (i, 0, numSamples);

            return;
        }

        if (AudioPlayHead* const playHead = getPlayHead ())
        {
            AudioPlayHead::CurrentPositionInfo position;
            playHead->getCurrentPosition (position);
            
            if (position.isLooping)
            { }
            else
            { }
        }
        
        for (int32 i = getNumInputChannels(); --i >= 0;)
            module->connectPort (channels.getAudioOutputPort (i), audio.getSampleData (i));

        for (int32 i = getNumOutputChannels(); --i >= 0;)
            module->connectPort (channels.getAudioOutputPort (i), tempBuffer.getSampleData (i));

        module->run ((uint32) numSamples);

        for (int i = getNumOutputChannels(); --i >= 0;)
            audio.copyFrom (i, 0, tempBuffer.getSampleData (i), numSamples);
    }

    //==============================================================================
    bool hasEditor() const { return false; }
    AudioProcessorEditor* createEditor() { return nullptr; }

    
    //==============================================================================
    const String
    getInputChannelName (int index) const
    {
        const ChannelMapping& inputs (channels.getInputs());
        
        String name = String ("Audio In ") + String (index + 1);
        if (! isPositiveAndBelow (index, inputs.getNumChannels (PortType::Audio)))
            return name;
        
        if (const LilvPort* port = module->getPort (inputs.getAudioPort (index)))
        {
            LilvNode* node = lilv_port_get_name (module->getPlugin(), port);
            name = CharPointer_UTF8 (lilv_node_as_string (node));
            lilv_node_free (node);
        }
        
        return name;
    }

    bool isInputChannelStereoPair (int index) const { return false; }

    const String
    getOutputChannelName (int index) const
    {
        String name = String ("Audio Out ") + String (index + 1);
        if (! isPositiveAndBelow (index, channels.getNumAudioOutputs()))
            return name;
        
        if (const LilvPort* port = module->getPort (channels.getAudioPort (index, false)))
        {
            LilvNode* node = lilv_port_get_name (module->getPlugin(), port);
            name = CharPointer_UTF8 (lilv_node_as_string (node));
            lilv_node_free (node);
        }
        
        return name;
    }

    bool isOutputChannelStereoPair (int index) const { return false; }

    
    //==============================================================================
    int getNumParameters() { return params.size(); }

    const String
    getParameterName (int index)
    {
        if (isPositiveAndBelow (index, params.size()))
            return params.getUnchecked(index)->getName();
        return String::empty;
    }

    
    float
    getParameter (int index)
    {
        if (isPositiveAndBelow (index, params.size()))
            return static_cast<float> (params.getUnchecked(index)->getNormalValue());

        return 0.0f;
    }

    const String
    getParameterText (int index)
    {
        if (! isPositiveAndBelow (index, params.size()))
            return String (getParameter (index));
        
        LV2Parameter* const param = params.getUnchecked (index);
        return String (static_cast<float> (param->getValue()));
    }
    
    
    void
    setParameter (int index, float newValue)
    {
        if (isPositiveAndBelow (index, params.size()))
        {
            LV2Parameter* const param = params.getUnchecked (index);
            param->setNormalValue (newValue);
            module->setControlValue (param->getPortIndex(), param->getValue());
        }
    }


    int getParameterNumSteps (int index)
    {
        if (isPositiveAndBelow(index, params.size()))
            return params.getUnchecked(index)->getNumSteps();
        return AudioProcessor::getParameterNumSteps (index);
    }
    

    float
    getParameterDefaultValue (int index)
    {
        if (LV2Parameter* const param = params [index])
        {
            float min, max, def;
            module->getPortRange (param->getPortIndex(), min, max, def);
            return def;
        }
        
        return AudioProcessor::getParameterDefaultValue (index);
    }

    String getParameterLabel (int index) const
    {
        return String::empty;
    }

    bool isParameterAutomatable (int index) const { return true; }

    //==============================================================================
    int getNumPrograms()          { return 0; }
    int getCurrentProgram()       { return 0; }
    void setCurrentProgram (int /*index*/) { }
    const String getProgramName (int /*index*/)  { return String::empty; }
    void changeProgramName (int /*index*/, const String& /*name*/) { }

    //==============================================================================
    void getStateInformation (MemoryBlock& mb)                  { ; }
    void getCurrentProgramStateInformation (MemoryBlock& mb)    { ; }
    void setStateInformation (const void* data, int size)               { ; }
    void setCurrentProgramStateInformation (const void* data, int size) { ; }

    //==============================================================================
    void timerCallback() { }

    void handleAsyncUpdate()
    {
        // indicates that something about the plugin has changed..
        // updateHostDisplay();
    }

private:

    CriticalSection lock, midiInLock;
    bool wantsMidiMessages, initialised, isPowerOn;
    mutable StringArray programNames;

    AudioSampleBuffer tempBuffer;
    ScopedPointer<LV2Module> module;
    OwnedArray<LV2Parameter> params;

    ChannelConfig channels;
    uint32 numPorts;
    uint32 midiPort;
    uint32 notifyPort;

    const char* getCategory() const { return module->getClassLabel().toUTF8(); }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LV2PluginInstance)
};


class LV2PluginFormat::Private
{
public:

    Private ()
    {
        world.setOwned (new LV2World ());
        init();
    }
    
    Private (LV2World& w)
    {
        world.setNonOwned (&w);
        init();
    }

    LV2Module* createModule (const String& uri)
    {
        return world->createModule (uri);
    }

    OptionalScopedPointer<LV2World> world;
    SymbolMap       symbols;
    
private:
    
    void init()
    {
        createProvidedFeatures();
        
    }
    
    void createProvidedFeatures()
    {
        world->addFeature (symbols.createMapFeature(), false);
        world->addFeature (symbols.createUnmapFeature(), true);
    }
    
};

LV2PluginFormat::LV2PluginFormat () : priv (new LV2PluginFormat::Private()) { }    
LV2PluginFormat::LV2PluginFormat (LV2World& w) : priv (new LV2PluginFormat::Private (w)) { }
LV2PluginFormat::~LV2PluginFormat() { priv = nullptr; }

SymbolMap& LV2PluginFormat::getSymbolMap() { return priv->symbols; }

void
LV2PluginFormat::findAllTypesForFile (OwnedArray <PluginDescription>& results,
                                      const String& fileOrIdentifier)
{
    if (! fileMightContainThisPluginType (fileOrIdentifier))
        return;

    ScopedPointer<PluginDescription> desc (new PluginDescription());
    desc->fileOrIdentifier = fileOrIdentifier;
    desc->pluginFormatName = String ("LV2");
    desc->uid = 0;

    
    try
    {
        ScopedPointer<AudioPluginInstance> instance (createInstanceFromDescription (*desc.get(), 44100.0, 1024));
        if (LV2PluginInstance* const p = dynamic_cast <LV2PluginInstance*> (instance.get()))
        {
            p->fillInPluginDescription (*desc.get());
            results.add (desc.release());
        }
    }
    catch (...)
    {
        // crashed while loading...
    }
}

AudioPluginInstance*
LV2PluginFormat::createInstanceFromDescription (const PluginDescription& desc, double sampleRate, int buffersize)
{
    if (desc.pluginFormatName != String ("LV2"))
        return nullptr;

    if (LV2Module* module = priv->createModule (desc.fileOrIdentifier))
    {
        module->instantiate (sampleRate, priv->world->getFeatures());
        module->activate();
        return new LV2PluginInstance (*priv->world, module);
    }

    JUCE_LV2_LOG ("Failed creating LV2 plugin instance");
    return nullptr;
}

bool
LV2PluginFormat::fileMightContainThisPluginType (const String& fileOrIdentifier)
{
    bool maybe = fileOrIdentifier.contains ("http:") ||
                 fileOrIdentifier.contains ("https:") ||
                 fileOrIdentifier.contains ("urn:");

    if (! maybe && File::isAbsolutePath (fileOrIdentifier))
    {
        const File file (fileOrIdentifier);
        maybe = file.getChildFile("manifest.ttl").existsAsFile();
    }

    return maybe;
}

String
LV2PluginFormat::getNameOfPluginFromIdentifier (const String& fileOrIdentifier)
{
    return fileOrIdentifier;
}

StringArray
LV2PluginFormat::searchPathsForPlugins (const FileSearchPath&, bool)
{
    StringArray list;
    Lilv::Plugins plugins (priv->world->getAllPlugins());

    LILV_FOREACH (plugins, iter, plugins)
    {
        Lilv::Plugin plugin (plugins.get (iter));
        String uri (plugin.get_uri().as_string());
        if (priv->world->isPluginSupported (uri))
            list.add (uri);
    }

    return list;
}

FileSearchPath LV2PluginFormat::getDefaultLocationsToSearch() { return FileSearchPath(); }

bool
LV2PluginFormat::doesPluginStillExist (const PluginDescription& desc)
{
    FileSearchPath placeholder;
    StringArray plugins (searchPathsForPlugins (placeholder, true));
    return plugins.contains (desc.fileOrIdentifier);
}
