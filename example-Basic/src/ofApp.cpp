#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	// parameters to smooth
	params.setName("paramsGroup");// main container
	params2.setName("paramsGroup2");// nested
	params3.setName("paramsGroup3");// nested
	params.add(lineWidth.set("lineWidth", 0.5, 0, 1));
	params.add(separation.set("separation", 50, 1, 100));
	params.add(speed.set("speed", 0.5, 0, 1));
	params.add(shapeType.set("shapeType", 0, -50, 50));
	params.add(size.set("size", 100, 0, 100));
	params.add(amount.set("amount", 10, 0, 25));
	params2.add(shapeType2.set("shapeType2", 0, -50, 50));
	params2.add(size2.set("size2", 100, 0, 100));
	params2.add(amount2.set("amount2", 10, 0, 25));
	params3.add(lineWidth3.set("lineWidth3", 0.5, 0, 1));
	params3.add(separation3.set("separation3", 50, 1, 100));
	params3.add(speed3.set("speed3", 0.5, 0, 1));
	params2.add(params3);
	params.add(params2);

	// smoother
	surfingSmooth.setup(params);

	//--

	// gui
	ofxSurfingHelpers::setThemeDark_ofxGui();

	// input
	gui.setup("Input");
	gui.add(params);
	gui.setPosition(10, 10);

	// output
	guiSmooth.setup("Output");
	guiSmooth.add(surfingSmooth.getParamsSmoothed());
	guiSmooth.setPosition(220, 10);
}

//--------------------------------------------------------------
void ofApp::update() {
}

//--------------------------------------------------------------
void ofApp::draw() {

	if (bGui) {
		gui.draw();
		guiSmooth.draw();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	surfingSmooth.keyPressed(key);

	if (key == 'g') bGui = !bGui;
}