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

	SmoothChannel()
	{
		path_Global = "ofxSurfingSmooth/";
		name_Settings = "fileSettings";

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

		exit();
	};

	void setup(string name);
	void startup();
	void doReset();
	
	// fix workaround, for different related params and modes
	// reduce by calling once per frame or make some bAttendintCalls flag..
	void update();
	void doRefresh();
	bool bDoReFresh = false;

public:

	//TODO:
	ofParameter<int> index{ "index", -1, 0, 0 };
	// A workaround to help on lambda callback

	//--

	ofParameterGroup params;

	ofParameter<bool> bEnableSmooth;//TODO: enables all the engine(smoother + detector), not only the smooth
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

	//TODO: WIP:
	ofParameter<bool> bNormalized;
	ofParameter<bool> bClamp;
	ofParameter<float> minInput;
	ofParameter<float> maxInput;
	ofParameter<float> minOutput;
	ofParameter<float> maxOutput;
	
	ofParameter<bool> bGateMode{ "GATE", false };
	ofParameter<bool> bGateSlow{ "Slow", false };
	ofParameter<int> bpmDiv{ "Div", 1, 1, 4 };//divide the bar duration in quarter

	ofParameter<bool> bReset;

	// 0 = TrigState, 1 = Bonk, 2 = Direction, 3 = DirUp, 4 = DirDown
	ofParameter<int> bangDetectorIndex;	


	//--

	vector<string> bangDetectors = { "State","Bonk", "Direct", "DirUp", "DirDown" };

private:

	string name = "SmoothChannel";

	string path_Global;
	string name_Settings;

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

private:

	void exit();
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

