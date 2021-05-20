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

#pragma once

#include "ofMain.h"

using namespace std;

class ofxDataStream {
    // STORAGE
    int streamSize;
    vector<float> prevVals;
    vector<float> vals;
    vector<float> valsN;
    vector<float> deltaVals;
    vector<bool> triggers;
    vector<bool> bonkVals;
    vector<bool> bonkPrevVals;
    vector<bool> bonks;
    ofVec2f valRange;
    bool isThreshed;
    float thresh;
    float decayGrowRatio;
    bool isNormalized;
    bool isClamped;
    bool isBonked;
    float bonkLo, bonkHi;
    float maxValue;
    float maxValueN;
    int maxIdx;

    // SMOOTHING
    vector<vector<float> > smoothHistos;
    int histoSize;
    float slideUp;
    float slideDown;

    float smooth(int _idx, float _val);
    
    // DIRECTION
    enum Direction_t {
        STATIC,
        DECREASING,
        INCREASING
    };
    vector<Direction_t> directions;
    vector<float> timeStamps;
    vector<float> valStamps;
    vector<float> directionChangeTimes;
    vector<float> directionChangeVals;
    bool newDirection;

public:
    enum Smoothing_t {
        SMOOTHING_NONE,
        SMOOTHING_ACCUM,
        SMOOTHING_SLIDE
    } smoothingType;

    enum Mean_t {
        MEAN_ARITH,
        MEAN_GEOM,
        MEAN_HARM,
    } meanType;
    
    bool directionChangeCalculated;

    ofxDataStream();
    ofxDataStream(int _size);
    void init(int _size);

    void initAccum(int _depth);
    void initSlide(float _sU, float _sD);
    
    void incrUpdate(float _val, int _idx=-1);
    void update(const vector<float>& _vals);
    void update(float _val, int _idx=0);

    void setThresh(float _t);
    void setThreshN(float _tN);
    float getThresh();
    float getThreshN();
    void setDecayGrow(float _ratio=1.0);
    void setNormalized(bool _n=true, ofVec2f _range = ofVec2f(0,1), bool _isClamped=true);
    void setOutputRange(ofVec2f _range=ofVec2f(0,1));
    void stampRangeLo(int _idx=0);
    void stampRangeHi(int _idx=0);
    ofVec2f getRange();

    void setBonk(float _hi=0.1, float _lo=0);
    
    float getValue(int _idx=0);
    float getValueN(int _idx=0);
    float getValueAboveThreshN(int _idx=0);
    float getDeltaValue(int _idx=0);
    bool getTrigger(int _idx=0);
    bool getBonk(int _idx=0);
    float getMaxVal();
    float getMaxValN();
    int getMaxIdx();
    void setMeanType(Mean_t _type);
    
    float getDirectionTimeDiff(int _idx=0);
    float getDirectionValDiff(int _idx=0);
    bool directionHasChanged();

    const vector<float>& getStream();
    const vector<float>& getStreamN();
    const vector<bool>& getTriggers();
    const vector<float>& getDeltas();
    const vector<bool>& getBonks();
    
    void reset(int _idx=-1);
};
