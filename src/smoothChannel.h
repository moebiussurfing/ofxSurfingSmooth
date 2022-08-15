#pragma once

#include "ofMain.h"
#include "ofxSurfingHelpers.h"


//--

// Learn callbacks
/*
class ControllerClass {
public:
	/// TheEventType can be an int, void or whatever type but it is important that it is the same you use in the callback function, either if it is a class method or a lambda function
	ofEvent<void> event;

	void notify() {
		//ofNotifyEvent(event, eventArgs, this);
		ofNotifyEvent(event, this); // if TheEventType is void you should use this form
	}

	//TheEventType eventArgs; // whatever data you might want to pass with the event. dont use if you declare ofEvent<void>

};

class SoundEngineClass {
public:
	void setListener(ControllerClass* controller) {
		eventListener = controller->event.newListener(this, &SoundEngineClass::eventCallback);
	}

	// in case you used a void event just make a funtion without arguments
	//void eventCallback(TheEventType&) {
	void eventCallback() {
		// this gets triggered by the event!

		ofLogNotice("SmoothChannel") << (__FUNCTION__) << " " << aparams.getName();

	}

	ofEventListener eventListener;

};
*/

//--

class SmoothChannel
{
public:

	//--

	ofParameterGroup params;

	ofParameter<bool> bEnableSmooth;
	ofParameter<float> ampInput;

	ofParameter<int> typeSmooth;
	ofParameter<int> typeMean;
	ofParameter<string> typeSmooth_Str;
	ofParameter<string> typeMean_Str;

	ofParameter<float> smoothPower;
	ofParameter<float> threshold;
	ofParameter<float> timeRedirection;
	ofParameter<float> slideMin;
	ofParameter<float> slideMax;
	ofParameter<float> onsetGrow;
	ofParameter<float> onsetDecay;

	ofParameter<bool> bNormalized;
	ofParameter<bool> bClamp;
	ofParameter<float> minInput;
	ofParameter<float> maxInput;
	ofParameter<float> minOutput;
	ofParameter<float> maxOutput;

	ofParameter<bool> bReset;

	ofParameter<int> bangDetectorIndex;

	//--

	vector<string> bangDetectors = { "Trig","Bonk", "Direction", "Above", "Below" };

private:

	string name = "SmoothChannel";

	string path_Global;
	string path_Settings;

	//--

private:

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

	std::vector<std::string> typeSmoothLabels;
	std::vector<std::string> typeMeanLabels;

public:

	SmoothChannel() 
	{
		path_Global = "ofxSurfingSmooth/";

		typeSmoothLabels.clear();
		typeSmoothLabels.push_back("None");
		typeSmoothLabels.push_back("Accumulator");
		typeSmoothLabels.push_back("Slide");

		typeMeanLabels.clear();
		typeMeanLabels.push_back("Arith");
		typeMeanLabels.push_back("Geom");
		typeMeanLabels.push_back("Harm");

		//--

		ofAddListener(params.parameterChangedE(), this, &SmoothChannel::Changed);
	};

	~SmoothChannel() 
	{
		ofRemoveListener(params.parameterChangedE(), this, &SmoothChannel::Changed);
		ofxSurfingHelpers::CheckFolder(path_Global);
		ofxSurfingHelpers::saveGroup(params, path_Settings);
	};

	void setup(string name);

private:

	void doReset()
	{
		ofLogNotice("SmoothChannel") << (__FUNCTION__) << name;

		ampInput = 0;
		bangDetectorIndex = 0;

		bClamp = false;
		minInput = 0;
		maxInput = 1;
		minOutput = 0;
		maxOutput = 1;
		bNormalized = false;

		typeSmooth = 1;
		typeMean = 0;
		smoothPower = 0.2;
		threshold = 0.5;
		timeRedirection = 0.5;
		slideMin = 0.2;
		slideMax = 0.2;
		onsetGrow = 0.1;
		onsetDecay = 0.1;
	}

	void Changed(ofAbstractParameter& e);

public:

	void setPathGlobal(string name) {
		path_Global = name;
		ofxSurfingHelpers::CheckFolder(path_Global);
	};

	//--

	/*
public:

	SoundEngineClass soundEngine;
	ControllerClass controller;
	*/
};

