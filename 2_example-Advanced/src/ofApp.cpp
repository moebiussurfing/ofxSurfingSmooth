#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

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

	//--

	// smooth
	dataStreamGroup.setup(params);

	// midi
#ifdef USE_MIDI
	mMidiParams.connect(1, true);
	mMidiParams.add(params);//recursive is not implemented, groups can't be nested..
	mMidiParams.add(params2);
	mMidiParams.add(params3);
#endif

	//--

	// gui
#ifdef USE_MIDI
	paramsApp.add(mMidiParams.bGui);
#endif
	paramsApp.add(dataStreamGroup.bShowGui);

	ofxSurfingHelpers::setThemeDark_ofxGui();
	gui.setup();
	gui.add(paramsApp);
	gui.add(params);
	gui.add(dataStreamGroup.getParamsSmoothed());

	ofxSurfingHelpers::loadGroup(paramsApp);
}

//--------------------------------------------------------------
void ofApp::update() {
}

//--------------------------------------------------------------
void ofApp::draw() {
	smoother.draw();
	
#ifdef USE_MIDI
	mMidiParams.draw();
#endif

	gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
	ofxSurfingHelpers::saveGroup(paramsApp);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	dataStreamGroup.keyPressed(key);

#ifdef USE_MIDI
	if (key == 'm') { mMidiParams.toggleVisible(); }
#endif
}
