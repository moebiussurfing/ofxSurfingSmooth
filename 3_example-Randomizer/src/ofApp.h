#pragma once
#include "ofMain.h"

#include "ofxSurfingSmooth.h"
#include "ofxGui.h"
#include "ofxSurfing_ofxGui.h"
#include "ofxSurfingRandomizer.h"
#include "ofxWindowApp.h"

/*

This examples adds the ofxSurfingRandomizer addon to add some randomizer tools
https://github.com/moebiussurfing/ofxSurfingRandomizer

*/

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed(int key);

		ofxSurfingSmooth smoother;

		ofxSurfingRandomizer randomizer;

		ofParameterGroup params;
		ofParameter<float> lineWidth;
		ofParameter<float> separation;
		ofParameter<float> speed;
		ofParameter<int> shapeType;
		ofParameter<int> amount;
		ofParameter<int> size;
		ofParameterGroup params2;
		ofParameter<int> shapeType2;
		ofParameter<int> amount2;
		ofParameter<int> size2;
		ofParameterGroup params3;
		ofParameter<float> lineWidth3;
		ofParameter<float> separation3;
		ofParameter<float> speed3;
		ofParameter<int> shapeType3;
		
		ofxPanel gui;
		ofxPanel guiInput;
		ofxPanel guiOutput;
		ofParameter<bool> bIn{ "Input", false };
		ofParameter<bool> bOut{ "Output", false };
		bool bGui = true;
		bool bLog = false;

		ofxWindowApp windowApp;
};
