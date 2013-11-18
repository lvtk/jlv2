
#ifndef LVTK_JUCE_LV2LOG_H
#define LVTK_JUCE_LV2LOG_H

class LV2Log : public LV2Feature
{
public:
    
    LV2Log();
    ~LV2Log();
    
    inline const String& getURI() const { return uri; }
    inline const LV2_Feature* getFeature() const { return &feat; }
    
private:
    
    String uri;
    LV2_Feature feat;
    LV2_Log_Log log;
    
};


#endif /* LVTK_JUCE_LV2LOG_H */
