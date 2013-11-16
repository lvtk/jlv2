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

#ifndef LVTK_JUCE_LV2FEATURES_H
#define LVTK_JUCE_LV2FEATURES_H


/** A simple interface for implenting LV2 Features */
class LV2Feature
{
public:

    LV2Feature() { }
    virtual ~LV2Feature() { }
    
    /** Get the LV2_Feature c-type.
        The returned object should be owned by the subclass.  It is the 
        subclass' responsibility to free any associated feature data. */
    virtual const LV2_Feature* getFeature() const = 0;
    
    /** Get the features URI as a string */
    virtual const String& getURI() const = 0;
};


/** An array of lv2 features */
class LV2FeatureArray
{
public:

    LV2FeatureArray() : needsBuilt (true) { }
    ~LV2FeatureArray() { }

    /** Add a new feature to the array.  The passed LV2Feature will be owned
        by this class */
    inline void add (LV2Feature* feature, bool rebuildArray = true)
    {
        ScopedPointer<LV2Feature> f (feature);
        if (f && ! contains (f->getURI()))
        {
            features.add (f.release());
            if (rebuildArray)
                buildArray();
        }
    }

    /** Returns true if a feature has been added to the array */
    inline bool contains (const String& featureURI) const
    {
        for (int i = features.size(); --i >= 0; )
            if (features[i]->getURI() == featureURI)
                return true;
        return false;
    }

    /** Get a C-Style array of feautres */
    inline const LV2_Feature* const*
    getFeatures() const
    {
        jassert (needsBuilt == false);
        return array.getData();
    }

    inline int size() const { return features.size(); }
    inline const LV2_Feature* begin() const { return getFeatures() [0]; }
    inline const LV2_Feature* end()   const { return getFeatures() [features.size()]; }
    inline operator const LV2_Feature* const*() const { return getFeatures(); }

private:
    
    OwnedArray<LV2Feature>  features;
    HeapBlock<LV2_Feature*> array;
    bool needsBuilt;
    
    inline void buildArray()
    {
        needsBuilt = false;
        
        array.calloc (features.size() + 1);
        for (int i = 0; i < features.size(); ++i)
            array[i] = const_cast<LV2_Feature*> (features[i]->getFeature());
        
        array [features.size()] = nullptr;
    }

};

#endif /* LVTK_JUCE_LV2FEATURES_H */
