#pragma once

#include "ofMain.h"

#include "ofxSurfingSmooth.h"

#include "ofxWindowApp.h"
#include "ofxGui.h"
#include "ofxSurfing_ofxGui.h"

class ofApp : public ofBaseApp
{
	public:

		void setup();
		void update();
		void draw();
		void keyPressed(int key);

		ofxSurfingSmooth data;

		ofParameterGroup params;
		ofParameter<float> lineWidth;
		ofParameter<float> separation;
		ofParameter<float> speed;
		ofParameter<int> shapeType;
		ofParameter<int> amount;
		ofParameter<int> size;
		ofParameter<int> shapeType2;
		ofParameter<int> amount2;
		ofParameter<int> size2;
		ofParameter<float> lineWidth3;
		ofParameter<float> separation3;
		ofParameter<float> speed3;
		ofParameter<int> shapeType3;
		
		ofxWindowApp windowApp;
};
