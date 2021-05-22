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
	data.setup(params);

	//--

	// gui
	ofxSurfingHelpers::setThemeDark_ofxGui();

	// input
	gui.setup("Input"); // source
	gui.add(params);
	gui.setPosition(10, 10);

	// output
	guiSmooth.setup("Output"); // smoothed
	guiSmooth.add(data.getParamsSmoothed());
	guiSmooth.setPosition(220, 10);
}

//--------------------------------------------------------------
void ofApp::update() {

	/*
	// NOTE:
	// learn how to access the smoothed parameters.
	// notice that we can't overwrite the smoothing on the source parameters!
	// we can get the smoothed params/variables doing these different approaches:

	// 0. simplified getters
	float _lineWidth = data.get(lineWidth);
	int _shapeType = data.get(shapeType);
	int _size = data.get(size);
	int _amount = data.get(amount);
	string str;
	str += lineWidth.getName() + ":" + ofToString(_lineWidth); str += " \t ";
	str += shapeType.getName() + ":" + ofToString(_shapeType); str += " \t ";
	str += size.getName() + ":" + ofToString(_size); str += " \t ";
	str += amount.getName() + ":" + ofToString(_amount); str += " \t ";
	ofLogNotice(__FUNCTION__) << str;

	// 1. just the values
	int _shapeType = data.getParamIntValue(shapeType);
	int _size = data.getParamIntValue(size);
	int _amount = data.getParamIntValue(amount);

	// 2. the parameters itself
	ofParameter<float> _lineWidth = data.getParamFloat(lineWidth.getName());
	ofParameter<float> _separation = data.getParamFloat(separation.getName());
	ofParameter<float> _separation3 = data.getParamFloat(separation3.getName());
	ofParameter<float> _speed3 = data.getParamFloat(speed3.getName());

	// 3. the whole group
	auto &group = data.getParamsSmoothed();
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
	auto &ap = data.getParamAbstract(lineWidth);
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

		ofDrawBitmapStringHighlight(data.getHelpInfo(), ofGetWidth() - 280, 25);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	data.keyPressed(key);

	if (key == 'h' || key == 'H') bGui = !bGui;
}