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
	gui.setup("Input"); // source
	gui.add(params);
	gui.setPosition(10, 10);

	// output
	guiSmooth.setup("Output"); // smoothed
	guiSmooth.add(surfingSmooth.getParamsSmoothed());
	guiSmooth.setPosition(220, 10);
}

//--------------------------------------------------------------
void ofApp::update() {
	/*
	// NOTE:
	// learn how to access the smoothed parameters.
	// notice that we can't overwrite the smoothing on the source parameters!
	// we can get the smoothed params/variables doing these different approaches:

	// 1. just the values
	int _shapeType = surfingSmooth.getParamIntValue(shapeType);
	int _size = surfingSmooth.getParamIntValue(size);
	int _amount = surfingSmooth.getParamIntValue(amount);

	// 2. the parameters itself
	ofParameter<float> _lineWidth = surfingSmooth.getParamFloat(lineWidth.getName());
	ofParameter<float> _separation = surfingSmooth.getParamFloat(separation.getName());
	ofParameter<float> _separation3 = surfingSmooth.getParamFloat(separation3.getName());
	ofParameter<float> _speed3 = surfingSmooth.getParamFloat(speed3.getName());

	// 3. the whole group
	auto &group = surfingSmooth.getParamsSmoothed();
	for (int i = 0; i < group.size(); i++)
	{
		auto type = group[i].type();
		bool isGroup = type == typeid(ofParameterGroup).name();
		bool isFloat = type == typeid(ofParameter<float>).name();
		bool isInt = type == typeid(ofParameter<int>).name();
		bool isBool = type == typeid(ofParameter<bool>).name();
		string str = group[i].getName();
		if (isFloat)
		{
			ofParameter<float> fp = group[i].cast<float>();
			//do something with this parameter
			//like push_back to your vector or something
		}
		else if (isInt)
		{
			ofParameter<int> ip = group[i].cast<int>();
			//do something with this parameter
			//like push_back to your vector or something
		}
	}

	// 4. as ofAbstractParameter to be casted after
	auto &ap = surfingSmooth.getParamAbstract(lineWidth);
	{
		auto type = ap.type();
		bool isGroup = type == typeid(ofParameterGroup).name();
		bool isFloat = type == typeid(ofParameter<float>).name();
		bool isInt = type == typeid(ofParameter<int>).name();
		bool isBool = type == typeid(ofParameter<bool>).name();
		string str = ap.getName();
		if (isFloat)
		{
			ofParameter<float> fp = ap.cast<float>();
		}
		else if (isInt)
		{
			ofParameter<int> ip = ap.cast<int>();
		}
	}
	*/
}

//--------------------------------------------------------------
void ofApp::draw() {

	if (bGui) {
		gui.draw();
		guiSmooth.draw();

		// help info
		string s = "";
		s += "HELP KEYS";
		s += "\n\n";
		s += "H       :  SHOW HELP"; s += "\n";
		s += "G       :  SHOW GUI"; s += "\n\n";
		s += "TESTER"; s += "\n";
		s += "SPACE   :  Randomize params"; s += "\n";
		s += "RETURN  :  Play Randomizer"; s += "\n";
		s += "\n"; 
		s += "TAB     :  Switch Smooth Type"; s += "\n";
		s += "SHIFT   :  Switch Mean Type"; s += "\n";
		s += "+|-     :  Adjust OnSet Thresholds"; s += "\n"; // WIP
		s += "\n"; 
		s += "S       :  Solo Plot param"; s += "\n";
		s += "Up|Down :  Browse Solo params"; s += "\n";
		ofDrawBitmapStringHighlight(s, ofGetWidth() - 280, 25);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	surfingSmooth.keyPressed(key);

	if (key == 'h' || key == 'H') bGui = !bGui;
}