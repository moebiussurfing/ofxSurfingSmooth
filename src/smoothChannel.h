#pragma once

#include "ofMain.h"
#include "ofxSurfingHelpers.h"

class SmoothChannel
{
public:

	SmoothChannel() {};
	~SmoothChannel() {
		ofxSurfingHelpers::CheckFolder(path_Global);
		ofxSurfingHelpers::saveGroup(params, path_Settings);
	};

	void setPathGlobal(string name) {
		path_Global = name;
		ofxSurfingHelpers::CheckFolder(path_Global);
	};

	void setup(string name);

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
	ofParameter<bool> bReset;

	ofParameter<bool> bClamp;
	ofParameter<float> minInput;
	ofParameter<float> maxInput;
	ofParameter<bool> bNormalized;
	ofParameter<float> minOutput;
	ofParameter<float> maxOutput;

	ofParameter<float> ampInput;
	ofParameter<int> bangDetectorIndex;

	ofParameterGroup params;

private:

	vector<string> bangDetectors = { "Trig","Bonk", "Direction", "Above", "Below" };

	string path_Global;
	string path_Settings;

	void doReset() 
	{
		ofLogNotice("SmoothChannel") << (__FUNCTION__)<< name;

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

	float _inputMinRange = 0;
	float _inputMaxRange = 1;
	float _outMinRange = 0;
	float _outMaxRange = 1;

	string name = "SmoothChannel";
};

