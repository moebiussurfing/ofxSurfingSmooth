#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	// Parameters to smooth 
	// or to measure with detectors

	params.setName("params"); // Group container
	params.add(speed.set("speed", 0.5, 0, 1));
	params.add(lineWidth.set("lineWidth", 0.5, 0, 1));
	params.add(separation.set("separation", 50.f, 1, 100));
	params.add(shapeType.set("shapeType", 0, -50, 50));
	params.add(size.set("size", 100, 0, 100));
	params.add(amount.set("amount", 10, 0, 25));
	//params.add(shapeType2.set("shapeType2", 0, -50, 50));
	//params.add(size2.set("size2", 100, 0, 100));
	//params.add(amount2.set("amount2", 10, 0, 25));
	//params.add(lineWidth3.set("lineWidth3", 0.5, 0, 1));
	//params.add(separation3.set("separation3", 50, 1, 100));
	//params.add(speed3.set("speed3", 0.5, 0, 1));

	// Setup
	data.setup(params);

	//-

	// Receiver for the bang detector
	float r = 150;
	circleBeat.setColor(ofColor::yellow);
	circleBeat.setRadius(r);
	circleBeat.setPosition(ofGetWidth() - 2 * r - 5, 5 + r);
	circleBeat.setName(speed.getName());
}

//--------------------------------------------------------------
void ofApp::update() {

	/*

		Here we learn how to access the smoothed parameters.
		Notice that we can't overwrite that smoothing
		into the source parameters!

		We can get the smoothed params/variables instead,
		by doing this approach:

	*/

	// Simple Getters

	if (0)
		if (ofGetFrameNum() % 30 == 0) // Slowdown the log a bit
		{
			// Get the smoothed params
			float _lineWidth = data.get(lineWidth);
			int _shapeType = data.get(shapeType);
			int _size = data.get(size);
			int _amount = data.get(amount);

			// Log 
			string sp = " \t ";
			string str = ">" + sp;

			str += lineWidth.getName()
				+ ":" + ofToString(_lineWidth, 2) + sp;
			str += shapeType.getName()
				+ ":" + ofToString(_shapeType) + sp;
			str += size.getName()
				+ ":" + ofToString(_size) + sp;
			str += amount.getName()
				+ ":" + ofToString(_amount) + sp;

			ofLogNotice(__FUNCTION__) << str;
		}

	/*

	We can also get the copied Group
	and process the content using other approaches...
	Go to look some snippets included
	at the bottom of ofxSurfingSmooth.h

	*/
}

//--------------------------------------------------------------
void ofApp::draw() {

	// Gui
	data.draw();

	//--

	// Log Detector for a param
	string str = " DEBUG    ";
	str += speed.getName() + "   ";
	str += ofToString((data.isBang(speed)) ? "+" : "-");

	ofDrawBitmapStringHighlight(str, 4, 15);

	//--
	
	// Display detector for a param
	if (data.isBang(speed)) circleBeat.bang();
	circleBeat.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	data.keyPressed(key);
}