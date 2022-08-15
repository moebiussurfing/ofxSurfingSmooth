#pragma once

#include "ofMain.h"
#include "ofxSurfingHelpers.h"

//--

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

		ofLogNotice("SmoothChannel") << (__FUNCTION__) /*<< " " << aparams.getName()*/;

	}

	ofEventListener eventListener;

};

//--

class SmoothChannel
{

public:

	SoundEngineClass soundEngine;
	ControllerClass controller;

	//--

public:

	SmoothChannel() {
		path_Global = "ofxSurfingSmooth/";

		ofAddListener(params.parameterChangedE(), this, &SmoothChannel::Changed);
	};

	~SmoothChannel() {
		ofRemoveListener(params.parameterChangedE(), this, &SmoothChannel::Changed);
		ofxSurfingHelpers::CheckFolder(path_Global);
		ofxSurfingHelpers::saveGroup(params, path_Settings);
	};

	void setup(string name);

	//--

	ofParameterGroup params;

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

	ofParameter<bool> bClamp;
	ofParameter<float> minInput;
	ofParameter<float> maxInput;
	ofParameter<bool> bNormalized;
	ofParameter<float> minOutput;
	ofParameter<float> maxOutput;

	ofParameter<int> bangDetectorIndex;

	ofParameter<bool> bReset;
	
	//--

	vector<string> bangDetectors = { "Trig","Bonk", "Direction", "Above", "Below" };

private:

	string name = "SmoothChannel";

	string path_Global;
	string path_Settings;

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

	void SmoothChannel::Changed(ofAbstractParameter& e)
	{
		std::string name = e.getName();

		ofLogVerbose("SmoothChannel") << name << " : " << e;

		if (0) {}

		else if (name == bReset.getName())
		{
			if (bReset)
			{
				bReset = false;
				doReset();
			}
		}
	}

	void setPathGlobal(string name) {
		path_Global = name;
		ofxSurfingHelpers::CheckFolder(path_Global);
	};
};

