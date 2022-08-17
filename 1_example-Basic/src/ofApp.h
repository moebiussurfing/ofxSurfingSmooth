#pragma once

#include "ofMain.h"

#include "ofxSurfingSmooth.h"
#include "CircleBeat.h"

#include "ofxWindowApp.h" // -> Not required. Just to handle window app position/size.

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
		
		ofxWindowApp w;

		CircleBeat circleBeat;
};
