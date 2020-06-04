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


#include "Env.h"


Env::Env(Buffer const& levels,
         Buffer const& times,
         EnvCurveList const& curves,
         const int releaseNode,
         const int loopNode) throw()
:	levels_(levels),
	times_(times),
	curves_(curves),
	releaseNode_(releaseNode),
	loopNode_(loopNode)
{
}


double Env::duration() const throw()
{
    double sum = 0.0;
    
    for (auto time : times_)
        sum += time;
    
    return sum;
}

Env Env::levelScale(const double scale) const throw()
{
    auto newLevels = levels_;
    
    for (auto& level : newLevels)
        level *= scale;

	return Env(newLevels,
			   times_,
			   curves_,
			   releaseNode_,
			   loopNode_);
}

Env Env::levelBias(const double bias) const throw()
{
    auto newLevels = levels_;
    
    for (auto& level : newLevels)
        level += bias;

    return Env(newLevels,
               times_,
               curves_,
               releaseNode_,
               loopNode_);
}

Env Env::timeScale(const double scale) const throw()
{
	assert(scale > 0.0);
    
    auto newTimes = times_;
    
    for (auto& time : newTimes)
        time *= scale;
	
    return Env(levels_,
               newTimes,
               curves_,
               releaseNode_,
               loopNode_);
}


float Env::lookup(float time) const throw()
{
    const int numTimes = (int) times_.size();
    const int numLevels = (int) levels_.size();
    const int lastLevel = numLevels-1;

    assert(numTimes == lastLevel);

    if(numLevels < 1) return 0.f;
    if(time <= 0.f || numTimes == 0) return levels_[0];

    float lastTime = 0.f;
    float stageTime = 0.f;
    int stageIndex = 0;

    while(stageTime < time && stageIndex < numTimes)
    {
        lastTime = stageTime;
        stageTime += times_[stageIndex];
        stageIndex++;
    }

    if(stageIndex > numTimes) return levels_[lastLevel];

    float level0 = levels_[stageIndex-1];
    float level1 = levels_[stageIndex];

    EnvCurve curve = curves_[stageIndex-1];
    EnvCurve::CurveType type = curve.getType();
    float curveValue = curve.getCurve();

    if((lastTime - stageTime)==0.f)
    {
       return level1;
    }
    else if(type == EnvCurve::Linear)
    {
        return jmap(time, lastTime, stageTime, level0, level1);
    }
    else if(type == EnvCurve::Numerical)
    {
        if(abs(curveValue) <= 0.001)
        {
            return jmap(time, lastTime, stageTime, level0, level1);
        }
        else
        {
            float pos = (time-lastTime) / (stageTime-lastTime);
            float denom = 1.f - std::exp(curveValue);
            float numer = 1.f - std::exp(pos * curveValue);
            return level0 + (level1 - level0) * (numer/denom);
        }
    }
    else if(type == EnvCurve::Sine)
    {
        return jmap(time, lastTime, stageTime, level0, level1);
    }
    else if(type == EnvCurve::Exponential)
    {
        return jmap(time, lastTime, stageTime, level0, level1);
    }
    else if(type == EnvCurve::Welch)
    {
        return jmap(time, lastTime, stageTime, level0, level1);
    }
    else
    {
        // Empty or Step
        return level1;
    }
}

//Env::operator Buffer () const throw()
//{
//    const float duration = getTimes().sum();
//    const int size = UGen::getSampleRate() * duration;
//    Buffer buffer = BufferSpec(size, 1, false);
//    float *bufferSamples = buffer.getData();
//
//    double time = 0.0;
//    for(int i = 0; i < size; i++, time += UGen::getReciprocalSampleRate())
//    {
//        *bufferSamples++ = lookup(time);
//    }
//
//    return buffer;
//}

//void Env::writeToBuffer(Buffer& buffer, const int channel) const throw()
//{
//    const double duration = getTimes().sum();
//    const int size = buffer.size();
//
//    assert(size > 0);
//    assert(buffer.getNumChannels() > 0);
//
//    float *bufferSamples = buffer.getData(channel % buffer.getNumChannels());
//
//    const double timeInc = duration / size;
//    double time = 0.0;
//    for(int i = 0; i < size; i++, time += timeInc)
//    {
//        *bufferSamples++ = lookup(time);
//    }
//}

Env Env::linen(const double attackTime, 
			   const double sustainTime, 
			   const double releaseTime, 
			   const double sustainLevel,
			   EnvCurve const& curve) throw()
{
	assert(attackTime >= 0.0);
	assert(sustainTime >= 0.0);
	assert(releaseTime >= 0.0);
	assert(sustainLevel >= 0.0);
	assert((attackTime + sustainTime + releaseTime) > 0.0);
	
    return Env({ 0.0, sustainLevel, sustainLevel, 0.0 },
               { attackTime, sustainTime, releaseTime },
               { curve });
}

Env Env::triangle(const double duration, 
				  const double level) throw()
{
	assert(duration > 0.0);
	assert(level > 0.0);

	const double durationHalved = duration * 0.5;
    return Env({ 0.0, level, 0.0 },
               { durationHalved, durationHalved });
}

Env Env::sine(const double duration, 
			  const double level) throw()
{
	assert(duration > 0.0);
	assert(level > 0.0);
	
	const double durationHalved = duration * 0.5;
    return Env({ 0.0, level, 0.0 },
               { durationHalved, durationHalved },
               { EnvCurve::Sine });
}

Env Env::perc(const double attackTime, 
			  const double releaseTime, 
			  const double level, 
			  EnvCurve const& curve) throw()
{
	assert(attackTime >= 0.0);
	assert(releaseTime >= 0.0);
	assert(level > 0.0);
	assert((attackTime + releaseTime) > 0.0);
	
    return Env({ 0.0, level, 0.0 },
               { attackTime, releaseTime },
               { curve });
}

Env Env::adsr(const double attackTime, 
			  const double decayTime, 
			  const double sustainLevel, 
			  const double releaseTime,
			  const double level, 
			  EnvCurve const& curve) throw()
{
	assert(attackTime >= 0.0);
	assert(decayTime >= 0.0);
	assert(sustainLevel >= 0.0);
	assert(releaseTime >= 0.0);
	assert(level > 0.0);
	
    return Env({ 0.0, level, (level * sustainLevel), 0.0 },
               { attackTime, decayTime, releaseTime },
               { curve }, 2);
}

Env Env::asr(const double attackTime, 
			 const double sustainLevel, 
			 const double releaseTime, 
			 const double level, 
			 EnvCurve const& curve) throw()
{
	assert(attackTime >= 0.0);
	assert(sustainLevel >= 0.0);
	assert(releaseTime >= 0.0);
	assert(level > 0.0);
	
    return Env({ 0.0, (level * sustainLevel), 0.0 },
               { attackTime, releaseTime },
               { curve }, 1);
}

