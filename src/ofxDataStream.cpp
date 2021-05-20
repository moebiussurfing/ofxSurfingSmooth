//------------------------------------------
// ofxDataStream
// Copyright (C) 2015 Paul Turowski
//
// for storing, processing streams of data
// with OpenFrameworks
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//------------------------------------------

#include "ofxDataStream.h"

ofxDataStream::ofxDataStream() {
    // ofxDataStream(1);
    // note: delegating constructors only works in c++ 11
    // which isn't implemented in OF 0.8.4
    init(1);
}

ofxDataStream::ofxDataStream(int _size){
    init(_size);
}

void ofxDataStream::init(int _size) {
    if (_size < 1) {
        ofLogError("ofxDataStream") << "ofxDataStream(): size must be at least 1";
        return;
    }
    // clear all vectors in case they have been already set
    prevVals.clear();
    vals.clear();
    valsN.clear();
    deltaVals.clear();
    triggers.clear();
    directions.clear();
    timeStamps.clear();
    valStamps.clear();
    directionChangeTimes.clear();
    directionChangeVals.clear();
    
    // allocate and initialize vectors
    for (int v=0; v<_size; v++) {
        prevVals.push_back(0);
        vals.push_back(0);
        valsN.push_back(0);
        deltaVals.push_back(0);
        triggers.push_back(false);
        // bonk vars are allocated in setBonk()
        
        // derivative data
        directions.push_back(STATIC);
        timeStamps.push_back(ofGetElapsedTimef());
        valStamps.push_back(0);
        directionChangeTimes.push_back(0);
        directionChangeVals.push_back(0);
    }

    streamSize = vals.size();
    isThreshed = false;
    thresh = 0.0;
    decayGrowRatio = 1.0;
    isNormalized = false;
    valRange = ofVec2f(0,1);
    isClamped = false;
    isBonked = false;
    smoothingType = SMOOTHING_NONE;
    slideUp = 1;
    slideDown = 1;
    
    directionChangeCalculated = false;
    newDirection = false;
}
//-------------------------------------------------------------------------
void ofxDataStream::initAccum(int _depth){
    smoothingType = SMOOTHING_ACCUM;
    meanType = MEAN_ARITH;

    // reset if previously initiated
    if (smoothHistos.size() > 0) {
        for (int a=0; a<smoothHistos.size(); a++) {
            smoothHistos.erase(smoothHistos.begin());
        }
    }
    // create new history
    vector<float> tempVector;
    for (int d=0; d<_depth; d++) {
        tempVector.push_back(0);
    }

    histoSize = tempVector.size();

    // store a history for each stream member
    for (int i=0; i<streamSize; i++) {
        smoothHistos.push_back(tempVector);
    }
}

void ofxDataStream::initSlide(float _sU, float _sD){
    if (_sU == 0 || _sD == 0) {
        ofLogError("ofxDataStream") << "slide values must be non-zero";
        return;
    }
    smoothingType = SMOOTHING_SLIDE;
    slideUp = _sU;
    slideDown = _sD;
}
//-------------------------------------------------------------------------
void ofxDataStream::incrUpdate(float _val, int _idx) {
    // if index is -1 (default), update all
    if (_idx == -1) {
        // reset max values
        maxValue = 0.0;
        maxValueN = 0.0;
        
        for (int i=0; i<streamSize; i++) {
            update(vals[i] + _val, i);
            if (vals[i] > maxValue) {
                maxValue = vals[i];
                maxValueN = valsN[i];
                maxIdx = i;
            }
        }
    }
    else if (_idx < -1 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "update(): index " << _idx << " doesn't exist";
        return;
    }
    else {
        update(vals[_idx] + _val, _idx);
    }
}

void ofxDataStream::update(const vector<float>& _vals) {
    if (_vals.size() != streamSize) {
        ofLogError("ofxDataStream") << "update(): vector size mismatch";
        return;
    }

    // reset max values
    maxValue = 0.0;
    maxValueN = 0.0;
    
    for (int i=0; i<streamSize; i++) {
        update(_vals[i], i);
        if (vals[i] > maxValue) {
            maxValue = vals[i];
            maxValueN = valsN[i];
            maxIdx = i;
        }
    }
}

void ofxDataStream::update(float _val, int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "update(): index " << _idx << " doesn't exist";
        return;
    }

    // store the new value
    vals[_idx] = _val;

    // get the delta value
    // (used in the smooth method)
    deltaVals[_idx] = vals[_idx] - prevVals[_idx];

    // smooth vals
    if (smoothingType == SMOOTHING_ACCUM ||
        smoothingType == SMOOTHING_SLIDE) {
        vals[_idx] = smooth(_idx, vals[_idx]);
    }
    
    if (decayGrowRatio != 1.0) {
        float valDiff = vals[_idx] - (vals[_idx] * decayGrowRatio);
        vals[_idx] -= valDiff * ofGetLastFrameTime();
    }
    
    // update bonks
    if (isBonked) {
        // recalculate delta for smoothing/clamping
        float smoothedDelta = vals[_idx] - prevVals[_idx];
        
        if (smoothedDelta >= bonkHi) {
            bonkVals[_idx] = true;
        }
        else if (smoothedDelta <= bonkLo) {
            bonkVals[_idx] = false;
        }
        
        if (bonkVals[_idx] && !bonkPrevVals[_idx]) {
            bonks[_idx] = true;
        }
        else bonks[_idx] = false;
        
        bonkPrevVals[_idx] = bonkVals[_idx];
    }
    
    // calculate direction changes
    if (directionChangeCalculated) {
        if (vals[_idx] == prevVals[_idx] && directions[_idx] != STATIC) {
            directions[_idx] = STATIC;
            timeStamps[_idx] = ofGetElapsedTimef();
        }
        else {
            if ((vals[_idx] > prevVals[_idx] && directions[_idx] != INCREASING) ||
                (vals[_idx] < prevVals[_idx] && directions[_idx] != DECREASING)) {
                newDirection = true;
                
                // calculate the time between direction changes
                directionChangeTimes[_idx] = ofGetElapsedTimef() - timeStamps[_idx];
                timeStamps[_idx] = ofGetElapsedTimef();
                
                // calculate the depth of the change (uses smoothed/clamped vals)
                directionChangeVals[_idx] = vals[_idx] - valStamps[_idx];
                if (directionChangeVals[_idx] < 0) {
                    directions[_idx] = DECREASING;
                }
                else {
                    directions[_idx] = INCREASING;
                }
                valStamps[_idx] = vals[_idx];
            }
            else {
                newDirection = false;
            }
        }
    }

    // store the new value in previous value
    prevVals[_idx] = vals[_idx];

    // set trigger
    if (isThreshed) triggers[_idx] = vals[_idx] > thresh;
    
    // clamp the output
    if (isClamped) {
        vals[_idx] = ofClamp(vals[_idx], valRange.x, valRange.y);
    }

    // normalize the value
    if (isNormalized) {
        valsN[_idx] = (vals[_idx] - valRange.x) / (valRange.y - valRange.x);
    }
}

float ofxDataStream::smooth(int _idx, float _val) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "smooth(): index doesn't exist";
        return -1;
    }

    float smoothedValue = 0.0;

    if (smoothingType == SMOOTHING_ACCUM) {
        smoothHistos[_idx].erase(smoothHistos[_idx].begin());
        smoothHistos[_idx].push_back(_val);

        // get averages
        if (meanType == MEAN_ARITH) {
            for (int i=0; i<histoSize; i++) {
                smoothedValue += smoothHistos[_idx][i];
            }
            smoothedValue /= histoSize;
        }
        else if (meanType == MEAN_GEOM) {
            for (int i=0; i<histoSize; i++) {
                if (i==0) smoothedValue = smoothHistos[_idx][i];
                smoothedValue *= smoothHistos[_idx][i];
            }
            smoothedValue = powf(smoothedValue, 1.0/histoSize);
        }
        else if (meanType == MEAN_HARM) {
            for (int i=0; i<histoSize; i++) {
                if (i==0) smoothedValue = smoothHistos[_idx][i];
                smoothedValue += 1/smoothHistos[_idx][i];
            }
            smoothedValue = histoSize / smoothedValue;
        }
    }
    else if (smoothingType == SMOOTHING_SLIDE) {
        if (deltaVals[_idx] >= 0) smoothedValue = prevVals[_idx] + (deltaVals[_idx]/slideUp);
        else smoothedValue = prevVals[_idx] + (deltaVals[_idx]/slideDown);
    }
    else {ofLogError("ofxDataStream") << "smooth(): not valid smoothing type";}

    return smoothedValue;
}
//-------------------------------------------------------------------------
void ofxDataStream::setThresh(float _t) {
    isThreshed = true;
    thresh = _t;
}

void ofxDataStream::setThreshN(float _tN) {
    isThreshed = true;
    thresh = valRange.x + (_tN * (valRange.y - valRange.x));
}

float ofxDataStream::getThresh() {return thresh;}

float ofxDataStream::getThreshN() {return (thresh - valRange.x) / (valRange.y - valRange.x);}

void ofxDataStream::setDecayGrow(float _ratio) {
    decayGrowRatio = _ratio;
}

void ofxDataStream::setNormalized(bool _n, ofVec2f _range, bool _isClamped) {
    if (_range.y - _range.x == 0) {
        ofLogError("ofxDataStream") << "setNormalized(): value range cannot be zero";
        return;
    }
    isNormalized = _n;
    valRange = _range;
    isClamped = _isClamped;
}

void ofxDataStream::setOutputRange(ofVec2f _range) {
    isClamped = true;
    valRange = _range;
}
void ofxDataStream::stampRangeLo(int _idx) {
    valRange.x = vals[_idx];
}
void ofxDataStream::stampRangeHi(int _idx) {
    valRange.y = vals[_idx];
}
ofVec2f ofxDataStream::getRange() {return valRange;}
//-------------------------------------------------------------------------
void ofxDataStream::setBonk(float _hiThresh, float _loThresh) {
    isBonked = true;
    bonkHi = _hiThresh;
    bonkLo = _loThresh;
    
    for (int v=0; v<streamSize; v++) {
        bonkVals.push_back(false);
        bonkPrevVals.push_back(false);
        bonks.push_back(false);
    }
}

//-------------------------------------------------------------------------

float ofxDataStream::getValue(int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getValue(): index doesn't exist";
        return 0;
    }
    return vals[_idx];
}

float ofxDataStream::getValueN(int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getValue(): index doesn't exist";
        return 0;
    }
    return valsN[_idx];
}

float ofxDataStream::getValueAboveThreshN(int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getValueAboveThreshN(): index doesn't exist";
        return 0;
    }
    
    if (getThreshN() == valRange.y) {
        ofLogError("ofxDataStream") << "getValueAboveThreshN(): threshold == max value";
        return 0;
    }
    
    float valOverThresh = valsN[_idx] - getThreshN();
    valOverThresh = ofClamp(valOverThresh, 0, 1);
    float valNOverThresh = valOverThresh / (1.0 - getThreshN());
    
    return valNOverThresh;
}

float ofxDataStream::getDeltaValue(int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getDeltaValue(): index doesn't exist";
        return 0;
    }
    return deltaVals[_idx];
}

bool ofxDataStream::getTrigger(int _idx) {
    bool returnedTrig = false;

    if (_idx >= 0 || _idx < streamSize) {
        returnedTrig = triggers[_idx];
    }
    else ofLogError("ofxDataStream") << "getTrigger(): index doesn't exist";

    return returnedTrig;
}

bool ofxDataStream::getBonk(int _idx) {
    bool returnedBonk = false;
    
    if (!isBonked) {
        ofLogError("ofxDataStream") << "getBonk(): need to call setBonk first";
    }
    else if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getBonk(): index doesn't exist";
    }
    else returnedBonk = bonks[_idx];
    
    return returnedBonk;
}

float ofxDataStream::getMaxVal() {return maxValue;}

float ofxDataStream::getMaxValN() {return maxValueN;}

int ofxDataStream::getMaxIdx() {return maxIdx;}

void ofxDataStream::setMeanType(ofxDataStream::Mean_t _type) {meanType = _type;}
//-------------------------------------------------------------------------
float ofxDataStream::getDirectionTimeDiff(int _idx) {
    float returnedDiff = false;
    
    if (!directionChangeCalculated) {
        ofLogError("ofxDataStream") << "getDirectionTimeDiff(): directionChangeCalculated needs to be enabled";
    }
    else if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getDirectionTimeDiff(): index doesn't exist";
    }
    else returnedDiff = directionChangeTimes[_idx];
    
    return returnedDiff;
}

float ofxDataStream::getDirectionValDiff(int _idx) {
    float returnedDiff = false;
    
    if (!directionChangeCalculated) {
        ofLogError("ofxDataStream") << "getDirectionValDiff(): directionChangeCalculated needs to be enabled";
    }
    else if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getDirectionValDiff(): index doesn't exist";
    }
    else returnedDiff = directionChangeVals[_idx];
    
    return returnedDiff;
}

bool ofxDataStream::directionHasChanged() {return newDirection;}

//-------------------------------------------------------------------------
const vector<float>& ofxDataStream::getStream() {return vals;}
const vector<float>& ofxDataStream::getStreamN() {return valsN;}
const vector<bool>& ofxDataStream::getTriggers() {return triggers;}
const vector<float>& ofxDataStream::getDeltas() {return deltaVals;}
const vector<bool>& ofxDataStream::getBonks() {return bonks;}

void ofxDataStream::reset(int _idx) {
    // reset the value of a particular index
    // if value is out of range, reset all indices
    int tempStart = 0;
    int tempRange = streamSize;
    
    if (_idx >= 0 && _idx < streamSize) {
        tempStart = _idx;
        tempRange = _idx+1;
    }
    
    for (int i=tempStart; i<tempRange; i++) {
        prevVals[i] = 0;
        vals[i] = 0;
        valsN[i] = 0;
        deltaVals[i] = 0;
        triggers[i] = 0;
        directions[i] = STATIC;
        timeStamps[i] = 0;
        valStamps[i] = 0;
        directionChangeTimes[i] = 0;
        directionChangeVals[i] = 0;
    }
}
