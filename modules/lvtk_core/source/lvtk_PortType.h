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

#ifndef LVTK_JUCE_PORTTYPE_H
#define LVTK_JUCE_PORTTYPE_H

/** The type of a port. This is intended to be used when working with LV2 plugins but
    could be used to represent ports for non-LV2 'things' */
class PortType {
public:

    enum ID {
        Control = 0,
        Audio   = 1,
        CV      = 2,
        Atom    = 3,
        Midi    = 4,
        Unknown = 5
    };

    explicit PortType (const String& uri)
        : type (Unknown)
    {
        if (uri == typeURI (Audio))
            type = Audio;
        else if (uri == typeURI (Control))
            type = Control;
        else if (uri == typeURI (CV))
            type = CV;
        else if (uri == typeURI (Atom))
            type = Atom;
    }

    PortType (ID id) : type(id) { }

    /** Get a URI string for this port type */
    inline const String& getURI()  const { return typeURI (type); }
    /** Get a human readable name for this port type */
    inline const String& getName() const { return typeName (type); }
    /** Get the port type id. This is useful in switch statements */
    inline ID               id()   const { return type; }

    inline bool operator== (const ID& id) const { return (type == id); }
    inline bool operator!= (const ID& id) const { return (type != id); }
    inline bool operator== (const PortType& t) const { return (type == t.type); }
    inline bool operator!= (const PortType& t) const { return (type != t.type); }
    inline bool operator<  (const PortType& t) const { return (type < t.type); }

    /** Returns true if this is an Audio port */
    inline bool isAudio()   { return type == Audio; }
    /** Returns true if this is a Control port */
    inline bool isControl() { return type == Control; }
    /** Returns true if this is a CV port */
    inline bool isCv()      { return type == CV; }
    /** Returns true if this is an Atom port */
    inline bool isAtom()    { return type == Atom; }
    /** Returns true if this is a MIDI port */
    inline bool isMidi()    { return type == Midi; }

    /** Return true if two port types can connect to one another */
    static inline bool canConnect (const PortType& sourceType, const PortType& destType)
    {
        if (sourceType == PortType::Unknown || destType == PortType::Unknown)
            return false;

        if (sourceType == destType)
            return true;

        if (sourceType == PortType::Audio && destType == PortType::CV)
            return true;

        if (sourceType == PortType::Control && destType == PortType::CV)
            return true;

        return false;
    }

    /** Return true if this port type can connect to another
        @param other The other port type
        @param isOutput Set true if 'this' is the output (source) type */
    inline bool canConnect (const PortType& other, bool isOutput = true) const
    {
        const bool res = isOutput ? canConnect (*this, other) : canConnect (other, *this);
        return res;
    }

    class ChannelMapping {
    public:
        
        ChannelMapping()
        {
            ports.ensureStorageAllocated (PortType::Unknown);
        }
        
    private:
        
        Array<Array<int32> > ports;
    
    };
    
private:

    /** @internal */
    static inline const String& typeURI (unsigned id)
    {
        assert (id <= Atom);
        static const String uris[] = {
            String (LV2_CORE__ControlPort),
            String (LV2_CORE__AudioPort),
            String (LV2_CORE__CVPort),
            String (LV2_ATOM__AtomPort),
            String ("http://lvtoolkit.org/ns#null")
        };
        return uris [id];
    }

    /** @internal */
    static inline const String& typeName (unsigned id)
    {
        assert (id <= Atom);
        static const String uris[] = {
            String ("Control"),
            String ("Audio"),
            String ("CV"),
            String ("Atom"),
            String ("Unknown")
        };
        return uris [id];
    }

    ID type;
};

/** A detailed descption of a port. (not used currently) */
struct PortDescription
{
    PortDescription() : index(0), isInput (false), type (PortType::Unknown) { }
    uint32      index;
    String      symbol;
    bool        isInput;
    PortType    type;
};

#endif /* LVTK_JUCE_PORTTYPE_H */
