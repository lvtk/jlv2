
#ifndef LVTK_JUCE_ATOMBUFFER_H
#define LVTK_JUCE_ATOMBUFFER_H

class AtomBuffer
{
public:
    
    AtomBuffer (uint32 bufferType, uint32 bufferSize);
    ~AtomBuffer();
    
    bool addEvent (int64 frames, uint32 size, uint32 type, const uint8* data);
    
    void clear();
    
	inline uint32 getCapacity() const { return capacity; }
    
    void* getPortData();
    
    inline uint32 getType()     const { return type; }
	inline void setType (uint32 newType) { type = newType; }
    
    
    
	inline bool isAudio()    const { return false; }
	inline bool isControl()  const { return false; }
	inline bool isSequence() const { return true; }
    
private:
    
    uint32 type, capacity;
    HeapBlock<uint8> block;
    LV2_Atom* atom;
    
};

#endif
