#pragma once
#include "ofMain.h"

#define USE_MIDI // can be disabled 

// this example it's the same than example-Basic plus:
// + midi control stuff 
// + windows management

#include "ofxSurfingSmooth.h"
#include "ofxGui.h"
#include "ofxSurfing_ofxGui.h"
#include "ofxWindowApp.h"
#ifdef USE_MIDI
#include "ofxMidiParams.h"
#endif

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed(int key);

		ofxSurfingSmooth dataStreamGroup;

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
		
#ifdef USE_MIDI
		ofxMidiParams mMidiParams;
#endif
		ofxWindowApp windowApp;

		ofParameterGroup paramsApp{ "ofApp" };
		ofxPanel gui;
};
