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
	//tiny workaround required when using more than one addon with an ImGui instance.
	//bc each addon has the autodraw enabled and must be only one of them enabled.
	//maybe we should check ofxImGui/example-sharedcontext..
	//ofAppGui.setSharedMode(true); // Force shared context
	//smoother.setImGuiSharedMode(true);
	smoother.setImGuiAutodraw(false);
	smoother.setup(params);

	// randomizer
	//randomizer.setImGuiAutodraw(false);//tiny workaround
	randomizer.setup(params);

	//--

	// gui
	ofxSurfingHelpers::setThemeDark_ofxGui();

	// panels
	gui.setup("Panels");
	gui.add(bIn);
	gui.add(bOut);
	gui.add(randomizer.bGui);
	gui.add(smoother.bGui);
	gui.setPosition(100 + 210 * 1, 10);

	// input
	guiInput.setup("Input"); // source
	guiInput.add(params);
	guiInput.setPosition(100 + 210 * 2, 10);

	// output
	guiOutput.setup("Output"); // smoothed
	guiOutput.add(smoother.getParamsSmoothed());
	guiOutput.setPosition(100 + 210 * 3, 10);
}

//--------------------------------------------------------------
void ofApp::update() {

	/*
	// NOTE:
	// learn how to access the smoothed parameters.
	// notice that we can't overwrite the smoothing on the source parameters!
	// we can get the smoothed params/variables doing these different approaches:
	*/

	// slowdown a bit
	if (bLog && ofGetFrameNum() % 6 == 0)
	{
		float _lineWidth = smoother.get(lineWidth);
		int _shapeType = smoother.get(shapeType);
		int _size = smoother.get(size);
		int _amount = smoother.get(amount);

		//log
		string sp = "   \t\t       ";
		string str = "SMOOTHED >" + sp;
		str += lineWidth.getName() + ":" + ofToString(_lineWidth, 2); str += sp;
		str += shapeType.getName() + ":" + ofToString(_shapeType); str += sp;
		str += size.getName() + ":" + ofToString(_size); str += sp;
		str += amount.getName() + ":" + ofToString(_amount); str += sp;
		ofLogNotice(__FUNCTION__) << str;
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	smoother.draw();
	
	if (bGui)
	{		
		gui.draw();
		if (bIn) guiInput.draw();
		if (bOut) guiOutput.draw();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	if (key == 'h' || key == 'H') bGui = !bGui;
	if (key == 'l' || key == 'L') bLog = !bLog;

	// randomizer
	//if (randomizer.bGui) randomizer.keyPressed(key);

	// smoothers
	else if (smoother.bGui) smoother.keyPressed(key);
}