// $Id$
// $HeadURL$

/*
 ==============================================================================
 
 This file is part of the UGEN++ library
 Copyright 2008-11 The University of the West of England.
 by Martin Robinson
 
 ------------------------------------------------------------------------------
 
 UGEN++ can be redistributed and/or modified under the terms of the
 GNU General Public License, as published by the Free Software Foundation;
 either version 2 of the License, or (at your option) any later version.
 
 UGEN++ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with UGEN++; if not, visit www.gnu.org/licenses or write to the
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA
 
 The idea for this project and code in the UGen implementations is
 derived from SuperCollider which is also released under the 
 GNU General Public License:
 
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 

 Modified to work with JUCE 5
 
 ==============================================================================
 */

#pragma once

#include "JuceHeader.h"
#include "Env.h"

#define HANDLESIZE 7
#define FINETUNE 0.001

class EnvelopeComponent;
class EnvelopeHandleComponent;
class EnvelopeLegendComponent;

class EnvelopeHandleComponentConstrainer :    public ComponentBoundsConstrainer
{
public:
    EnvelopeHandleComponentConstrainer(EnvelopeHandleComponent* handle);
    
    void checkBounds (Rectangle<int>& bounds,
                      const Rectangle<int>& old, const Rectangle<int>& limits,
                      bool isStretchingTop, bool isStretchingLeft,
                      bool isStretchingBottom, bool isStretchingRight);
    
    void setAdjacentHandleLimits(int setLeftLimit, int setRightLimit);
    
private:
    int leftLimit, rightLimit;
    EnvelopeHandleComponent* handle;
};

class EnvelopeHandleComponent :    public Component
{
public:    
    EnvelopeHandleComponent();
    EnvelopeComponent* getParentComponent() const;
    
    void updateTimeAndValue();
    
    void updateLegend();
    void paint(Graphics& g);
    void moved();
        
    void mouseMove         (const MouseEvent& e);
    void mouseEnter        (const MouseEvent& e);
    void mouseExit         (const MouseEvent& e);
    void mouseDown         (const MouseEvent& e);
    void mouseDrag         (const MouseEvent& e);
    void mouseUp           (const MouseEvent& e);
        
    EnvelopeHandleComponent* getPreviousHandle() const;
    EnvelopeHandleComponent* getNextHandle() const;
    void removeThisHandle();
    
    void setMousePositionToThisHandle();
    
    void resetOffsets() { offsetX = offsetY = 0; }
    double getTime() const    { return time;    }
    double getValue() const    { return value; }
    EnvCurve getCurve() const    { return curve; }
    int getHandleIndex() const;
        
    void setTime(double timeToSet);
    void setValue(double valueToSet);
    void setCurve(EnvCurve curveToSet);
    void setTimeAndValue(double timeToSet, double valueToSet, double quantise = 0.0);
    void offsetTimeAndValue(double offsetTime, double offsetValue, double quantise = 0.0);
    double constrainDomain(double domainToConstrain) const;
    double constrainValue(double valueToConstrain) const;
    
    void lockTime(double time);
    void lockValue(double value);
    void unlockTime();
    void unlockValue();
    
    friend class EnvelopeComponent;
    
private:
    bool dontUpdateTimeAndValue;
    void recalculatePosition();
    
    ComponentDragger dragger;
    int lastX, lastY;
    int offsetX, offsetY;
    EnvelopeHandleComponentConstrainer resizeLimits;
    
    double time, value;
    bool shouldLockTime, shouldLockValue;
    EnvCurve curve;
    bool ignoreDrag;
};


class EnvelopeComponentListener
{
public:
    EnvelopeComponentListener() throw() {}
    virtual ~EnvelopeComponentListener() {}
    virtual void envelopeChanged(EnvelopeComponent* changedEnvelope) = 0;
    virtual void envelopeStartDrag(EnvelopeComponent*) { }
    virtual void envelopeEndDrag(EnvelopeComponent*) { }
};

/** For displaying and editing a breakpoint envelope. 
 @ingoup EnvUGens
 @see Env */
class EnvelopeComponent : public Component
{
public:
    EnvelopeComponent();
    ~EnvelopeComponent();
    
    void setDomainRange(const double min, const double max);
    void setDomainRange(const double max) { setDomainRange(0.0, max); }
    void getDomainRange(double& min, double& max) const;
    void setValueRange(const double min, const double max);
    void setValueRange(const double max) { setValueRange(0.0, max); }
    void getValueRange(double& min, double& max) const;
    
    enum GridMode { 
        GridLeaveUnchanged = -1,
        GridNone = 0,
        GridValue = 1, 
        GridDomain = 2, 
        GridBoth = GridValue | GridDomain 
    };
    void setGrid(const GridMode display, const GridMode quantise, const double domain = 0.0, const double value = 0.0);
    void getGrid(GridMode& display, GridMode& quantise, double& domain, double& value) const;
    
    void paint(Graphics& g);
    void paintBackground(Graphics& g);
    void resized();
        
    void mouseMove         (const MouseEvent& e);
    void mouseEnter        (const MouseEvent& e);
    void mouseDown         (const MouseEvent& e);
    void mouseDrag         (const MouseEvent& e);
    void mouseUp           (const MouseEvent& e);
    
    void addListener (EnvelopeComponentListener* const listener);
    void removeListener (EnvelopeComponentListener* const listener);
    void sendChangeMessage();
    void sendStartDrag();
    void sendEndDrag();
    
    void clear();
    
    EnvelopeLegendComponent* getLegend();
    void setLegendText (const String& legendText);
    void setLegendTextToDefault();
    int getHandleIndex(EnvelopeHandleComponent* handle) const;
    EnvelopeHandleComponent* getHandle(const int index) const;
    int getNumHandles() const { return handles.size(); }
    
    EnvelopeHandleComponent* getPreviousHandle(const EnvelopeHandleComponent* thisHandle) const;
    EnvelopeHandleComponent* getNextHandle(const EnvelopeHandleComponent* thisHandle) const;
    EnvelopeHandleComponent* addHandle(int newX, int newY, EnvCurve curve);
    EnvelopeHandleComponent* addHandle(double newDomain, double newValue, EnvCurve curve);
    void removeHandle(EnvelopeHandleComponent* thisHandle);
    void quantiseHandle(EnvelopeHandleComponent* thisHandle);
    
    bool isReleaseNode(EnvelopeHandleComponent* thisHandle) const;
    bool isLoopNode(EnvelopeHandleComponent* thisHandle) const;
    void setReleaseNode(const int index);
    void setReleaseNode(EnvelopeHandleComponent* thisHandle);
    int getReleaseNode() const;
    void setLoopNode(const int index);
    void setLoopNode(EnvelopeHandleComponent* thisHandle);
    int getLoopNode() const;
    
    void setAllowCurveEditing(const bool flag);
    bool getAllowCurveEditing() const;
    void setAllowNodeEditing(const bool flag);
    bool getAllowNodeEditing() const;
    
    double convertPixelsToDomain(int pixelsX, int pixelsXMax = -1) const;
    double convertPixelsToValue(int pixelsY, int pixelsYMax = -1) const;
    double convertDomainToPixels(double domainValue) const;
    double convertValueToPixels(double value) const;
    
    Env getEnv() const;
    void setEnv(Env const& env);
    float lookup(const float time) const;
    void setMinMaxNumHandles(int min, int max);
    
    double constrainDomain(double domainToConstrain) const;
    double constrainValue(double valueToConstrain) const;
    
//    double quantiseDomain(double value);
//    double quantiseValue(double value);
    
    enum EnvColours { Node, ReleaseNode, LoopNode, Line, LoopLine, Background, GridLine, LegendText, LegendBackground, NumEnvColours };
    void setEnvColour(const EnvColours which, juce::Colour const& colour) throw();
    const juce::Colour& getEnvColour(const EnvColours which) const throw();
    
    enum MoveMode { MoveClip, MoveSlide, NumMoveModes };
    
private:
    void recalculateHandles();
    
    SortedSet <void*> listeners;
    Array<EnvelopeHandleComponent*> handles;
    int minNumHandles, maxNumHandles;
    double domainMin, domainMax;
    double valueMin, valueMax;
    double valueGrid, domainGrid;
    GridMode gridDisplayMode, gridQuantiseMode;
    EnvelopeHandleComponent* draggingHandle;
    int curvePoints;
    int releaseNode, loopNode;
    
    bool allowCurveEditing:1;
    bool allowNodeEditing:1;
    
    juce::Colour colours[NumEnvColours];
};

class EnvelopeLegendComponent : public Component
{
public:
    EnvelopeLegendComponent(String const& defaultText = {});
    virtual ~EnvelopeLegendComponent();
    
    EnvelopeComponent* getEnvelopeComponent() const;
    
    void paint(Graphics& g);
    void resized();
    void setText(String const& legendText);
    void setText();
    
    virtual double mapValue(double value);
    virtual double mapTime(double time);
    virtual String getValueUnits() { return {}; }
    virtual String getTimeUnits() { return {}; }
    
private:
    Label* text;
    String defaultText;
};

/** For displaying and editing a breakpoint envelope with a legend. 
 This provides additional information about the points. 
 @ingoup EnvUGens
 @see Env */
class EnvelopeContainerComponent : public Component
{
public:
    EnvelopeContainerComponent(const String& defaultText = {});
    ~EnvelopeContainerComponent();
    void resized();
    
    EnvelopeComponent* getEnvelopeComponent() const        { return envelope; }
    EnvelopeLegendComponent* getLegendComponent() const    { return legend;   }
    void setLegendComponent(EnvelopeLegendComponent* newLegend);
    
    
    void addListener (EnvelopeComponentListener* const listener) { envelope->addListener(listener); }
    void removeListener (EnvelopeComponentListener* const listener) { envelope->removeListener(listener); }
    
    Env getEnv() const { return getEnvelopeComponent()->getEnv(); }
    void setEnv(Env const& env) { return getEnvelopeComponent()->setEnv(env); }
    float lookup(const float time) const { return getEnvelopeComponent()->lookup(time); }
    
    void setEnvColour(const EnvelopeComponent::EnvColours which, juce::Colour const& colour) throw()
    {
        envelope->setEnvColour(which, colour);
    }
    
    const juce::Colour& getEnvColour(const EnvelopeComponent::EnvColours which) const throw()
    {
        return envelope->getEnvColour(which);
    }
    
    void setDomainRange(const double min, const double max)        { envelope->setDomainRange(min, max);    }
    void setDomainRange(const double max)                        { setDomainRange(0.0, max);                }
    void getDomainRange(double& min, double& max) const            { envelope->getDomainRange(min, max);    }
    void setValueRange(const double min, const double max)        { envelope->setValueRange(min, max);    } 
    void setValueRange(const double max)                        { setValueRange(0.0, max);                }
    void getValueRange(double& min, double& max) const            { envelope->getValueRange(min, max);    }
    
    void setGrid(const EnvelopeComponent::GridMode display, 
                 const EnvelopeComponent::GridMode quantise, 
                 const double domain = 0.0, 
                 const double value = 0.0)
    {
        envelope->setGrid(display, quantise, domain, value);
    }
    
    void getGrid(EnvelopeComponent::GridMode& display, 
                 EnvelopeComponent::GridMode& quantise,
                 double& domain, 
                 double& value) const
    {
        envelope->getGrid(display, quantise, domain, value);
    }
    
    void setAllowCurveEditing(const bool flag)    { envelope->setAllowCurveEditing(flag);        }
    bool getAllowCurveEditing() const            { return envelope->getAllowCurveEditing();    }
    void setAllowNodeEditing(const bool flag)    { envelope->setAllowNodeEditing(flag);        }
    bool getAllowNodeEditing() const            { return envelope->getAllowNodeEditing();    }

    
private:
    EnvelopeComponent*            envelope;
    EnvelopeLegendComponent*    legend;
};

//class EnvelopeCurvePopup :    public PopupComponent,
//                            public SliderListener,
//                            public ComboBox::Listener
//{
//private:
//    
//    class CurveSlider : public Slider
//    {
//    public:
//        CurveSlider() : Slider("CurveSlider") { }
//        
//#if JUCE_MAJOR_VERSION < 2
//        const
//#endif
//        String getTextFromValue (double value)
//        {
//            value = value * value * value;
//            value = jmap (value, -1.0, 1.0, -50.0, 50.0);
//            return String(value, 6);
//        }
//    };
//    
//    EnvelopeHandleComponent* handle;
//    Slider *slider;
//    ComboBox *combo;
//    
//    bool initialised;
//    
//    static const int idOffset;
//
//    EnvelopeCurvePopup(EnvelopeHandleComponent* handle);
//    
//public:    
//    static void create(EnvelopeHandleComponent* handle, int x, int y);
//    
//    ~EnvelopeCurvePopup();
//    void resized();
//    void sliderValueChanged(Slider* sliderThatChanged);
//    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
//    void expired();
//    
//};
//
//class EnvelopeNodePopup :   public PopupComponent,
//                            public ComboBox::Listener,
//                            public Button::Listener
//{
//private:
//    EnvelopeHandleComponent* handle;
//    ComboBox *combo;
//    TextButton *setLoopButton;
//    TextButton *setReleaseButton;
//    
//    bool initialised;
//    
//    static const int idOffset;
//    
//    EnvelopeNodePopup(EnvelopeHandleComponent* handle);
//    
//public:
//    enum NodeType { Normal, Release, Loop, ReleaseAndLoop };
//    
//    static void create(EnvelopeHandleComponent* handle, int x, int y);
//    
//    ~EnvelopeNodePopup();
//    void resized();
//    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
//    void buttonClicked(Button *button);
//    void expired();
//};
