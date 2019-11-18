/*
    Copyright (c) 2014-2019  Michael Fisher <mfisher@kushview.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

namespace jlv2 {

enum UIQuality {
    UI_NO_SUPPORT     = 0,    // UI not supported
    UI_FULL_SUPPORT   = 1,    // UI directly embeddable (i.e. a juce component)
    UI_NATIVE_EMBED   = 2     // Embeddable Native UI type
};

namespace Callbacks {

inline unsigned uiSupported (const char* hostType, const char* uiType)
{
    if (strcmp (hostType, JLV2__JUCEUI) == 0)
    {
        if (strcmp (uiType, JLV2__JUCEUI) == 0)
            return UI_FULL_SUPPORT;
        else if (strcmp (uiType, JLV2__NativeUI))
            return UI_NATIVE_EMBED;
        return 0;
    }

    return suil_ui_supported (hostType, uiType);
}

}

class Module::Private
{
public:
    Private (Module& module)
        : owner (module) { }

    ~Private() { }

    ModuleUI* createUI (const LilvUI* uiNode,
                             const LilvNode* containerType,
                             const LilvNode* widgetType,
                             const LV2_Feature* const * features)
    {
        const LilvNode* uri = lilv_ui_get_uri (uiNode);
        const LilvPlugin* plugin = owner.getPlugin();

        auto uiptr = std::unique_ptr<ModuleUI> (new ModuleUI (owner.getWorld(), owner));
        uiptr->containerType = lilv_node_as_uri (containerType);
        uiptr->plugin = lilv_node_as_uri (lilv_plugin_get_uri (plugin));
        uiptr->ui = lilv_node_as_uri (uri);
        uiptr->widgetType = lilv_node_as_uri (widgetType);
        uiptr->bundlePath = lilv_uri_to_path (lilv_node_as_uri (lilv_ui_get_bundle_uri (uiNode)));
        uiptr->binaryPath = lilv_uri_to_path (lilv_node_as_uri (lilv_ui_get_binary_uri (uiNode)));
        ui = uiptr.release();

        return ui;
    }

    void sendControlValues()
    {
        if (! ui && ! owner.onPortNotify)
            return;

        for (const auto* port : ports.getPorts())
        {
            if (PortType::Control != port->type)
                continue;

            auto* const buffer = buffers.getUnchecked (port->index);

            if (ui)
                ui->portEvent ((uint32_t)port->index, sizeof(float), 
                               0, buffer->getPortData());
            if (owner.onPortNotify)
                owner.onPortNotify ((uint32_t) port->index, sizeof(float), 
                                    0, buffer->getPortData());
        }
    }

    static const void * getPortValue (const char *port_symbol, void *user_data, uint32_t *size, uint32_t *type)
    {
        Module::Private* priv = static_cast<Module::Private*> (user_data);
        int portIdx = -1;
        for (const auto* port : priv->ports.getPorts())
        {
            if (port->symbol == port_symbol && port->type == PortType::Control) {
                portIdx = port->index;
                break;
            }
        }

        if (portIdx >= 0)
        {
            if (auto* const buffer = priv->buffers [portIdx])
            {
                *size = sizeof (float);
                *type = priv->owner.map (LV2_ATOM__Float);
                return buffer->getPortData();
            }
        }

        *size = 0;
        *type = 0;

        return nullptr;
    }

    static void setPortValue (const char* port_symbol,
                              void*       user_data,
                              const void* value,
                              uint32_t    size,
                              uint32_t    type)
    {
        auto* priv = (Private*) user_data;
        auto& plugin = priv->owner;

        if (type != priv->owner.map (LV2_ATOM__Float))
            return;

        int portIdx = -1;
        const PortDescription* port = nullptr;
        for (const auto* p : priv->ports.getPorts())
        {
            port = p;
            if (port->symbol == port_symbol && port->type == PortType::Control) {
                portIdx = port->index;
                break;
            }
        }

        if (portIdx >= 0 && port)
        {
            if (auto* const buffer = priv->buffers [portIdx])
                buffer->setValue (*((float*) value));
        }
    }
    
private:
    friend class Module;
    Module& owner;
    PortList ports;
    ChannelConfig channels;

    String uri;     ///< plugin URI
    String name;    ///< Plugin name
    String author;  ///< Plugin author name

    ModuleUI::Ptr ui;

    HeapBlock<float> mins, maxes, defaults;    
    OwnedArray<PortBuffer> buffers;

    LV2_Feature instanceFeature { LV2_INSTANCE_ACCESS_URI, nullptr };
};

Module::Module (World& world_, const void* plugin_)
   : instance (nullptr),
     plugin ((const LilvPlugin*) plugin_),
     world (world_),
     active (false),
     currentSampleRate (44100.0),
     numPorts (lilv_plugin_get_num_ports (plugin)),
     events (nullptr)
{
    priv = new Private (*this);
    init();
}

Module::~Module()
{
    freeInstance();
    worker = nullptr;
}

void Module::activatePorts()
{
    // noop
}

void Module::init()
{
    events.reset (new RingBuffer (4096));
    evbufsize = jmax (evbufsize, static_cast<uint32> (256));
    evbuf.realloc (evbufsize);
    evbuf.clear (evbufsize);

    notifications.reset (new RingBuffer (4096));
    ntbufsize = jmax (ntbufsize, static_cast<uint32> (256));
    ntbuf.realloc (ntbufsize);
    ntbuf.clear (ntbufsize);

    // create and set default port values
    priv->mins.allocate (numPorts, true);
    priv->maxes.allocate (numPorts, true);
    priv->defaults.allocate (numPorts, true);

    lilv_plugin_get_port_ranges_float (plugin, priv->mins, priv->maxes, priv->defaults);

    // initialize each port
    for (uint32 p = 0; p < numPorts; ++p)
    {
        const LilvPort* port (lilv_plugin_get_port_by_index (plugin, p));

        // port type
        PortType type = PortType::Unknown;
        if (lilv_port_is_a (plugin, port, world.lv2_AudioPort))
            type = PortType::Audio;
        else if (lilv_port_is_a (plugin, port, world.lv2_AtomPort))
            type = PortType::Atom;
        else if (lilv_port_is_a (plugin, port, world.lv2_ControlPort))
            type = PortType::Control;
        else if (lilv_port_is_a (plugin, port, world.lv2_CVPort))
            type = PortType::CV;
        else if (lilv_port_is_a (plugin, port, world.lv2_EventPort))
            type = PortType::Event;
            
        const bool isInput (lilv_port_is_a (plugin, port, world.lv2_InputPort));
        LilvNode* nameNode = lilv_port_get_name (plugin, port);
        const String name = lilv_node_as_string (nameNode);
        lilv_node_free (nameNode); nameNode = nullptr;
        const String symbol  = lilv_node_as_string (lilv_port_get_symbol (plugin, port));

        priv->ports.add (type, p, priv->ports.size (type, isInput),
                         symbol, name, isInput);
        priv->channels.addPort (type, p, isInput);

        uint32 capacity = sizeof (float);
        uint32 dataType = 0;
        switch ((uint32) type.id()) 
        {
            case PortType::Control:
                capacity = sizeof (float); 
                dataType = map (LV2_ATOM__Float);
                break;
            case PortType::Audio:
                capacity = sizeof (float);
                dataType = map (LV2_ATOM__Float);
                break;
            case PortType::Atom:
                capacity = 4096;
                dataType = map (LV2_ATOM__Sequence);
                break;
            case PortType::Midi:    
                capacity = sizeof (uint32); 
                dataType = map (LV2_MIDI__MidiEvent);
                break;
            case PortType::Event:
                capacity = 4096; 
                dataType = map (LV2_EVENT__Event);
                break;
            case PortType::CV:      
                capacity = sizeof(float);
                dataType = map (LV2_ATOM__Float);
                break;
        }

        PortBuffer* const buf = priv->buffers.add (
            new PortBuffer (isInput, type, dataType, capacity));
        
        if (type == PortType::Control)
            buf->setValue (priv->defaults [p]);
    }

    // load related GUIs
    if (auto* related = lilv_plugin_get_related (plugin, world.ui_UI))
    {
        LILV_FOREACH (nodes, iter, related)
        {
            const auto* res = lilv_nodes_get (related, iter);
            lilv_world_load_resource (world.getWorld(), res);
        }
        lilv_nodes_free (related);
    }

    // plugin URI
    priv->uri = String::fromUTF8 (lilv_node_as_string (lilv_plugin_get_uri (plugin)));

    // plugin name
    if (LilvNode* node = lilv_plugin_get_name (plugin))
    {
        priv->name = String::fromUTF8 (lilv_node_as_string (node));
        lilv_node_free (node);
    }

    // author name
    if (LilvNode* node = lilv_plugin_get_author_name (plugin))
    {
        priv->author = String::fromUTF8 (lilv_node_as_string (node));
        lilv_node_free (node);
    }
}

void Module::loadDefaultState()
{
    if (instance == nullptr)
        return;
    
    auto* const map = (LV2_URID_Map*) world.getFeatures().getFeature (LV2_URID__map)->getFeature()->data;
    if (auto* uriNode = lilv_new_uri (world.getWorld(), priv->uri.toRawUTF8()))
    {
        if (auto* state = lilv_state_new_from_world (world.getWorld(), map, uriNode))
        {
            const LV2_Feature* const features[] = { nullptr };
            lilv_state_restore (state, instance, Private::setPortValue, 
                                priv.get(), LV2_STATE_IS_POD, features);
            lilv_state_free (state);
            priv->sendControlValues();
        }

        lilv_node_free (uriNode);
    }
}

String Module::getStateString() const
{
    if (instance == nullptr)
        return String();
    
    auto* const map = (LV2_URID_Map*) world.getFeatures().getFeature (LV2_URID__map)->getFeature()->data;
    auto* const unmap = (LV2_URID_Unmap*) world.getFeatures().getFeature (LV2_URID__unmap)->getFeature()->data;
    const String descURI = "http://kushview.net/kv/state";

    String result;
    const LV2_Feature* const features[] = { nullptr };
    
    if (auto* state = lilv_state_new_from_instance (plugin, instance, 
        map, 0, 0, 0, 0, 
        Private::getPortValue, priv.get(), 
        LV2_STATE_IS_POD, // flags
        features))
    {
        char* strState = lilv_state_to_string (world.getWorld(), map, unmap, state, descURI.toRawUTF8(), 0);
        result = String::fromUTF8 (strState);
        std::free (strState);

        lilv_state_free (state);
    }

    return result;
}

void Module::setStateString (const String& stateStr)
{
    if (instance == nullptr)
        return;
    auto* const map = (LV2_URID_Map*) world.getFeatures().getFeature (LV2_URID__map)->getFeature()->data;
    auto* const unmap = (LV2_URID_Unmap*) world.getFeatures().getFeature (LV2_URID__unmap)->getFeature()->data;
    if (auto* state = lilv_state_new_from_string (world.getWorld(), map, stateStr.toRawUTF8()))
    {
        const LV2_Feature* const features[] = { nullptr };
        lilv_state_restore (state, instance, Private::setPortValue, 
                            priv.get(), LV2_STATE_IS_POD, features);
        lilv_state_free (state);
        priv->sendControlValues();
    }
}

Result Module::instantiate (double samplerate)
{
    freeInstance();
    jassert(instance == nullptr);
    currentSampleRate = samplerate;

    features.clearQuick();
    world.getFeatures (features);

    // check for a worker interface
    LilvNodes* nodes = lilv_plugin_get_extension_data (plugin);
    LILV_FOREACH (nodes, iter, nodes)
    {
        const LilvNode* node = lilv_nodes_get (nodes, iter);
        if (lilv_node_equals (node, world.work_interface))
        {
            worker = new WorkerFeature (world.getWorkThread(), 1);
            features.add (worker->getFeature());
        }
    }
    lilv_nodes_free (nodes); nodes = nullptr;
    
    features.add (nullptr);
    instance = lilv_plugin_instantiate (plugin, samplerate,
                                        features.getRawDataPointer());
    if (instance == nullptr) {
        features.clearQuick();
        worker = nullptr;
        return Result::fail ("Could not instantiate plugin.");
    }

    if (const void* data = getExtensionData (LV2_WORKER__interface))
    {
        jassert (worker != nullptr);
        worker->setSize (2048);
        worker->setInterface (lilv_instance_get_handle (instance),
                              (LV2_Worker_Interface*) data);
    }
    else if (worker)
    {
        features.removeFirstMatchingValue (worker->getFeature());
        worker = nullptr;
    }

    loadDefaultState();
    startTimerHz (60);
    return Result::ok();
}

void Module::activate()
{
   if (instance && ! active)
   {
       lilv_instance_activate (instance);
       activatePorts();
       active = true;
   }
}

void Module::cleanup()
{
   if (instance != nullptr)
   {
       // TODO: Check if Lilv takes care of calling cleanup
   }
}

void Module::deactivate()
{
   if (instance != nullptr && active)
   {
       lilv_instance_deactivate (instance);
       active = false;
   }
}

bool Module::isActive() const
{
   return instance && active;
}

void Module::freeInstance()
{
    stopTimer();
    if (instance != nullptr)
    {
        deactivate();
        worker = nullptr;
        auto* oldInstance = instance;
        instance = nullptr;
        lilv_instance_free (oldInstance);
    }
}

void Module::setSampleRate (double newSampleRate)
{
    if (newSampleRate == currentSampleRate)
        return;

    if (instance != nullptr)
    {
        const bool wasActive = isActive();
        freeInstance();
        instantiate (newSampleRate);

        jassert (currentSampleRate == newSampleRate);

        if (wasActive)
            activate();
    }
}

void Module::connectChannel (const PortType type, const int32 channel, void* data, const bool isInput)
{
    connectPort (priv->channels.getPort (type, channel, isInput), data);
}

void Module::connectPort (uint32 port, void* data)
{
    lilv_instance_connect_port (instance, port, data);
}

String Module::getURI()         const { return priv->uri; }
String Module::getName()        const { return priv->name; }
String Module::getAuthorName()  const { return priv->author; }

const ChannelConfig& Module::getChannelConfig() const
{
    return priv->channels;
}

String Module::getClassLabel() const
{
   if (const LilvPluginClass* klass = lilv_plugin_get_class (plugin))
       if (const LilvNode* node = lilv_plugin_class_get_label (klass))
           return CharPointer_UTF8 (lilv_node_as_string (node));
   return String();
}

const void* Module::getExtensionData (const String& uri) const
{
    return instance ? lilv_instance_get_extension_data (instance, uri.toUTF8())
                    : nullptr;
}

void* Module::getHandle()
{
    return instance ? (void*) lilv_instance_get_handle (instance)
                    : nullptr;
}

uint32 Module::getNumPorts() const { return numPorts; }

uint32 Module::getNumPorts (PortType type, bool isInput) const
{
    return static_cast<uint32> (priv->ports.size (type, isInput));
}

const LilvPort* Module::getPort (uint32 port) const
{
    return lilv_plugin_get_port_by_index (plugin, port);
}

uint32 Module::getMidiPort() const
{
   for (uint32 i = 0; i < getNumPorts(); ++i)
   {
       const LilvPort* port (getPort (i));
       if ((lilv_port_is_a (plugin, port, world.lv2_AtomPort) || lilv_port_is_a (plugin, port, world.lv2_EventPort)) &&
           lilv_port_is_a (plugin, port, world.lv2_InputPort) &&
           lilv_port_supports_event (plugin, port, world.midi_MidiEvent))
           return i;
   }

   return LV2UI_INVALID_PORT_INDEX;
}

uint32 Module::getNotifyPort() const
{
    for (uint32 i = 0; i < numPorts; ++i)
    {
        const LilvPort* port (getPort (i));
        if (lilv_port_is_a (plugin, port, world.lv2_AtomPort) &&
            lilv_port_is_a (plugin, port, world.lv2_OutputPort) &&
            lilv_port_supports_event (plugin, port, world.midi_MidiEvent))
        {
            return i;
        }
    }

    return LV2UI_INVALID_PORT_INDEX;
}

const LilvPlugin* Module::getPlugin() const { return plugin; }

const String Module::getPortName (uint32 index) const
{
    if (const auto* desc = priv->ports.get (index))
        return desc->name;
    return String();
}

void Module::getPortRange (uint32 port, float& min, float& max, float& def) const
{
    if (port >= numPorts)
        return;

    min = priv->mins [port];
    max = priv->maxes [port];
    def = priv->defaults [port];
}

PortType Module::getPortType (uint32 index) const
{
    if (const auto* desc = priv->ports.get (index))
        return desc->type >= PortType::Control && desc->type <= PortType::Unknown 
            ? desc->type : PortType::Unknown;
    return PortType::Unknown;
}

bool Module::isLoaded() const { return instance != nullptr; }

bool Module::hasEditor() const
{
    if (! supportedUIs.isEmpty())
        return true;

    LilvUIs* uis = lilv_plugin_get_uis (plugin);
    if (nullptr == uis)
        return false;

    auto& suplist = const_cast<OwnedArray<SupportedUI>&> (supportedUIs);
    
    LILV_FOREACH (uis, iter, uis)
    {
        const LilvUI* ui = lilv_uis_get (uis, iter);
        const auto uri = String::fromUTF8 (lilv_node_as_string (lilv_ui_get_uri (ui)));

        // Check JUCE UI
        if (lilv_ui_is_a (ui, world.ui_JUCEUI))
        {
            auto* const supported = suplist.add (new SupportedUI());
            supported->URI = uri;
            supported->container = JLV2__JUCEUI;
            supported->widget = JLV2__JUCEUI;
            continue;
        }

        // check if native UI
        const LilvNode* uitype = nullptr;
        if (lilv_ui_is_supported (ui, suil_ui_supported,
            world.getNativeWidgetType(), &uitype))
        {
            if (uitype != nullptr && lilv_node_is_uri (uitype))
            {
                auto* const supported = suplist.add (new SupportedUI());
                supported->URI = uri;
                supported->container = JLV2__NativeUI;
                supported->widget = String::fromUTF8 (lilv_node_as_uri (uitype));
                continue;
            }
        }

        // Check for Gtk2
        uitype = nullptr;
        if (lilv_ui_is_supported (ui, suil_ui_supported, world.ui_GtkUI, &uitype))
        {
            if (uitype != nullptr && lilv_node_is_uri (uitype))
            {
                auto* const supported = suplist.add (new SupportedUI());
                supported->URI = uri;
                supported->container = LV2_UI__GtkUI;
                supported->widget = String::fromUTF8 (lilv_node_as_uri (uitype));
                continue;
            }
        }
    }

    lilv_uis_free (uis);

    for (const auto* const sui : supportedUIs)
    {
        DBG("[jlv2] supported ui: " << sui->URI);
        DBG("[jlv2]    container: " << sui->container);
        DBG("[jlv2]       widget: " << sui->widget);
    }

    return ! supportedUIs.isEmpty();
}

void Module::clearEditor()
{
    if (priv->ui)
    {
        auto ui = priv->ui;
        priv->ui = nullptr;
        ui->unload();
        ui = nullptr;
    }
}

PortBuffer* Module::getPortBuffer (uint32 port) const
{
    jassert (port < numPorts);
    return priv->buffers.getUnchecked (port);
}

uint32 Module::getPortIndex (const String& symbol) const
{
    for (const auto* port : priv->ports.getPorts())
        if (port->symbol == symbol)
            return static_cast<uint32> (port->index);
    return LV2UI_INVALID_PORT_INDEX;
}

ModuleUI* Module::createEditor()
{
    if (priv->ui)
        return priv->ui.get();
    
    ModuleUI* instance = nullptr;

    if (LilvUIs* uis = lilv_plugin_get_uis (plugin))
    {
        LILV_FOREACH(uis, iter, uis)
        {
            const LilvUI* uiNode = lilv_uis_get (uis, iter);
            const unsigned quality = lilv_ui_is_supported (uiNode, &Callbacks::uiSupported, world.ui_JUCEUI, nullptr);
            const auto uri = String::fromUTF8 (lilv_node_as_string (lilv_ui_get_uri (uiNode)));

            if (quality == 1)
            {
                // JUCEUI 
                instance = priv->createUI (uiNode,
                    world.ui_JUCEUI, 
                    world.ui_JUCEUI,
                    world.getFeatures().getFeatures());
                break;
            }
            else if (quality == 2)
            {
                // Native UI (CocoaUI, WindowsUI, or X11UI)
                instance = priv->createUI (uiNode,
                    world.getNativeWidgetType(),
                    world.getNativeWidgetType(),
                    world.getFeatures().getFeatures());
                break;
            }
        }

        lilv_uis_free (uis);
    }

    if (instance != nullptr)
    {
        jassert (instance == priv->ui.get());
        priv->sendControlValues();
    }

    return instance;
}

void Module::sendPortEvents()
{
    priv->sendControlValues();
}

bool Module::isPortInput (uint32 index) const
{
   return lilv_port_is_a (plugin, getPort (index), world.lv2_InputPort);
}

bool Module::isPortOutput (uint32 index) const
{
   return lilv_port_is_a (plugin, getPort (index), world.lv2_OutputPort);
}

void Module::timerCallback()
{
    PortEvent ev;
    
    static const uint32 pnsize = sizeof (PortEvent);

    for (;;)
    {
        if (! notifications->canRead (pnsize))
            break;

        notifications->read (ev, false);
        if (ev.size > 0 && notifications->canRead (pnsize + ev.size))
        {
            notifications->advance (pnsize, false);
            notifications->read (ntbuf, ev.size, true);

            if (ev.protocol == 0)
            {
                if (auto ui = priv->ui)
                    ui->portEvent (ev.index, ev.size, ev.protocol, ntbuf.getData());
                if (onPortNotify)
                    onPortNotify (ev.index, ev.size, ev.protocol, ntbuf.getData());
            }
        }
    }
}

void Module::referAudioReplacing (AudioSampleBuffer& buffer)
{
    for (int c = 0; c < priv->channels.getNumAudioInputs(); ++c)
        priv->buffers.getUnchecked ((int) priv->channels.getPort (
            PortType::Audio, c, true))->referTo (buffer.getWritePointer (c));

    for (int c = 0; c < priv->channels.getNumAudioOutputs(); ++c)
        priv->buffers.getUnchecked ((int) priv->channels.getPort (
            PortType::Audio, c, false))->referTo (buffer.getWritePointer (c));
}

void Module::run (uint32 nframes)
{
    PortEvent ev;
    
    static const uint32 pesize = sizeof (PortEvent);

    for (;;)
    {
        if (! events->canRead (pesize))
            break;
        events->read (ev, false);
        if (ev.size > 0 && events->canRead (pesize + ev.size))
        {
            events->advance (pesize, false);
            events->read (evbuf, ev.size, true);

            if (ev.protocol == 0)
            {
                auto* buffer = priv->buffers.getUnchecked (ev.index);                
                if (buffer->getValue() != *((float*) evbuf.getData()))
                {
                    buffer->setValue (*((float*) evbuf.getData()));
                    if (notifications->canWrite (pesize + ev.size)) 
                    {
                        notifications->write (ev);
                        notifications->write (evbuf.getData(), ev.size);
                    }
                }
            }
        }
    }

    for (int i = priv->buffers.size(); --i >= 0;)
        connectPort (static_cast<uint32> (i), priv->buffers.getUnchecked(i)->getPortData());
    
    if (worker)
        worker->processWorkResponses();

    lilv_instance_run (instance, nframes);

    if (worker)
        worker->endRun();
}

uint32 Module::map (const String& uri) const
{
    // FIXME: const in SymbolMap::map/unmap 
    return (const_cast<World*> (&world))->map (uri);
}

void Module::write (uint32 port, uint32 size, uint32 protocol, const void* buffer)
{
    PortEvent event;
    zerostruct (event);
    event.index     = port;
    event.size      = size;
    event.protocol  = protocol;

    if (events->canWrite (sizeof (PortEvent) + size))
    {
        events->write (event);
        events->write (buffer, event.size);
    }
    else
    {
        DBG("lv2 plugin write buffer full.");
    }
}

}
