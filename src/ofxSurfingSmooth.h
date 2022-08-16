#pragma once

/*

	TODO:

	+ fix circle beat widget
		check bang behavior!
	+ fix chars to debug plot
	+ add clamp and normalization modes.

	+ add colors types, vectors, using templates.
		avoid crash to unsupported types.
	+ "real" nested sub-groups tree levels.. ?
	+ add param to calibrate max history smooth/speed.
	+ plotting int type should be stepped/not continuous.

*/


#include "ofMain.h"

#include "smoothChannel.h"
#include "ofxDataStream.h"
#include "ofxHistoryPlot.h"
#include "ofxSurfingHelpers.h"
#include "ofxSurfingImGui.h"
#include "ofxSurfingBoxInteractive.h"
#include "circleBeatWidget.h"

#define COLORS_MONCHROME // Un comment to draw all plots with the same color.
#define MAX_AMP_POWER 1

//--

class ofxSurfingSmooth : public ofBaseApp
{

private:

	vector<unique_ptr<SmoothChannel>> smoothChannels;
	ofEventListeners listeners;
	//int iIndex = -1;

	const int MAX_HISTORY = 30;
	const int MIN_SLIDE = 1;
	const int MAX_SLIDE = 50;
	const int MAX_ACC_HISTORY = 60; // calibrated to 60fps

public:

	ofxSurfingSmooth();
	~ofxSurfingSmooth();

	void draw();
	void keyPressed(int key);

private:

	void update(ofEventArgs& args);
	void exit();

	//--

	ofxSurfing_ImGui_Manager guiManager;

private:

	ofParameterGroup params_EditorEnablers; // Contents the enabled params to smooth or to bypass.
	vector<ofParameter<bool>> editorEnablers;
	void drawToggles();

	void doSetAll(bool b);
	void doDisableAll();
	void doEnableAll();

	//--

	// API INITIALIZERS

public:

	void setup(ofParameterGroup& aparams); // main setup method. to all pass the params with one line.

private:

	void add(ofParameterGroup aparams);
	void add(ofParameter<float>& aparam);
	void add(ofParameter<int>& aparam);
	void add(ofParameter<bool>& aparam);
	//TODO: add more types

private:

	void addParam(ofAbstractParameter& aparam);

	//--

	// API GETTERS

public:

	//--------------------------------------------------------------
	ofParameterGroup& getParamsSmoothed() {
		return mParamsGroup_Smoothed;
	}

	//--

	// Bang / Bonk / Triggers under threshold detectors

private:

	// Getting by the index could be probably problematic,
	// bc some param types could be skipped. 
	// we disable to better use getting by name as below.

	//--------------------------------------------------------------
	bool isTriggered(int i) {//flag true when triggered this index param
		if (!bEnableSmoothGlobal) return false;

		if (i > params_EditorEnablers.size() - 1)
		{
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}

		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!smoothChannels[i]->bEnableSmooth || !_bSmooth) return false;

		if (outputs[i].getTrigger()) {
			ofLogVerbose("ofxSurfingSmooth") << "Triggered: " << i;
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool isBonked(int i) {//flag true when triggered this index param
		if (!bEnableSmoothGlobal) return false;
		
		if (i > params_EditorEnablers.size() - 1)
		{
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}

		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!smoothChannels[i]->bEnableSmooth || !_bSmooth) return false;

		if (outputs[i].getBonk()) {
			ofLogVerbose("ofxSurfingSmooth") << "Bonked: " << i;
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool isRedirected(int i) {//flag true when signal direction changed
		if (!bEnableSmoothGlobal) return false;
		
		if (i > params_EditorEnablers.size() - 1)
		{
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}

		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!smoothChannels[i]->bEnableSmooth || !_bSmooth) return false;

		// if the direction has changed and
		// if the time of change is greater than (0.5) timeRedirection sec
		// print the time between changes and amount of change
		if (outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
			outputs[i].directionHasChanged())
		{
			ofLogVerbose("ofxSurfingSmooth") <<
				"Redirected: " << i << " " <<
				outputs[i].getDirectionTimeDiff() << ", " <<
				outputs[i].getDirectionValDiff();

			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	int isRedirectedTo(int i) {
		if (!bEnableSmoothGlobal) return 0;
		
		// return 0 when no direction changed. -1 or 1 depending direction.
		// above or below threshold

		int rdTo = 0;

		if (i > params_EditorEnablers.size() - 1) {
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return rdTo;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!smoothChannels[i]->bEnableSmooth || !_bSmooth) return rdTo;

		// if the direction has changed and
		// if the time of change is greater than 0.5 sec
		// print the time between changes and amount of change
		if (outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
			outputs[i].directionHasChanged())
		{
			ofLogVerbose("ofxSurfingSmooth") << "Redirected: " << i << " " <<
				outputs[i].getDirectionTimeDiff() << ", " <<
				outputs[i].getDirectionValDiff();

			if (outputs[i].getDirectionValDiff() < 0) 
				rdTo = 1;
			else 
				rdTo = -1;

			return rdTo;
		}

		return rdTo; // 0=nothing useful
	}

public:

	// Getting detector flags by using the source param names.

	//--------------------------------------------------------------
	bool isTriggered(ofAbstractParameter& e) {
		int i = getIndex(e);
		if (i != -1) return isTriggered(i);
		else return false;
	}
	//--------------------------------------------------------------
	bool isBonked(ofAbstractParameter& e) {
		int i = getIndex(e);
		if (i != -1) return isBonked(i);
		else return false;
	}
	//--------------------------------------------------------------
	bool isRedirected(ofAbstractParameter& e) {
		int i = getIndex(e);
		if (i != -1) return isRedirected(i);
		else return false;
	}
	//--------------------------------------------------------------
	int isRedirectedTo(ofAbstractParameter& e) {
		int i = getIndex(e);
		if (i != -1)/*error*/ return isRedirectedTo(i);
		else return 0;
	}

	//----

public:

	//--------------------------------------------------------------
	bool isBang(int i)
	{
		// Returns true if selected trigger is selected and happening.
		// then we can pick easily which detector
		// to use in out parent scope app!
		// 0=TrigState, 1=Bonk, 2=Direction, 3=DirUp, 4=DirDown

		bool bReturn = false;

		switch (smoothChannels[i]->bangDetectorIndex)
		{
		case 0: bReturn = isTriggered(0); break;
		case 1: bReturn = isBonked(0); break;
		case 2: bReturn = isRedirected(0); break;
		case 3: bReturn = (isRedirectedTo(0) < 0); break;
		case 4: bReturn = (isRedirectedTo(0) > 0); break;

		default: ofLogError("ofxSurfingSmooth") << "Unknown selector index: " << i; return false; break;
		}

		return bReturn;
	}

	//----

public:

	// Some helpers to get the smoothed params
	// by different approaches.

	int getIndex(ofAbstractParameter& e) /*const*/;

	float get(ofParameter<float>& e);
	int get(ofParameter<int>& e);

	float getParamFloatValue(ofAbstractParameter& e);
	int getParamIntValue(ofAbstractParameter& e);

	ofAbstractParameter& getParamAbstract(ofAbstractParameter& e);
	ofAbstractParameter& getParamAbstract(string name);
	ofParameter<float>& getParamFloat(string name);
	ofParameter<int>& getParamInt(string name);

public:

	void doRandomize(); // do and set random in min/max range for all params
	//void doRandomize(int index, bool bForce); // do random in min/max range for a param. bForce ignores enabler

	//---

private:

	void setup();
	void startup();

	void updateGenerators();
	void updateSmooths();
	void updateEngine();

	void setupPlots();
	void refreshPlots();
	void drawPlots(ofRectangle r);
	vector<string> strDetectorsState;

private:

	ofParameterGroup mParamsGroup;

	ofParameterGroup mParamsGroup_Smoothed;

	//TODO:
	string suffix = "";
	//string suffix = "_COPY";

private:

	vector<ofxDataStream> outputs; // the smooth class
	vector<float> inputs; // feed normalized signals here

	// Signal Generator
	ofxSurfingHelpers::SurfGenerators surfGenerator;

	//--

	string path_Global;
	string path_Settings;

public:

	ofxSurfingBoxInteractive boxPlots;

private:

	int amountPlots;
	int amountChannels;

	vector<ofxHistoryPlot*> plots;
	vector<ofColor> colors;

#ifdef COLORS_MONCHROME
	ofColor colorPlots;
#endif

	ofColor colorBaseLine;
	ofColor colorTextSelected;

	ofColor colorThreshold;
	ofColor colorTrig;
	ofColor colorBonk;
	ofColor colorDirect;
	ofColor colorDirectUp;
	ofColor colorDirectDown;

private:

	void Changed_Params(ofAbstractParameter& e);

	bool bDISABLE_CALLBACKS = true;

	ofParameterGroup params;

	ofParameter<bool> bGui_Inputs;
	ofParameter<bool> bGui_Outputs;
	ofParameter<bool> bGenerators;

	ofParameter<bool> bEnableSmoothGlobal; // global enable

	ofParameter<bool> bPlay;
	ofParameter<float> playSpeed;

	// Tester timers
	int tf;
	float tn;

	void doReset();
	void setupParams();

	ofColor colorBg;

	void setup_ImGui();

	void draw_ImGui();
	void draw_ImGuiMain();
	void draw_ImGuiGameMode();
	void draw_ImGuiExtra();

private:

	std::vector<std::string> typeSmoothLabels;
	std::vector<std::string> typeMeanLabels;

	////--------------------------------------------------------------
	//void nextTypeSmooth() {
	//	if (typeSmooth >= typeSmooth.getMax()) typeSmooth = 1;
	//	else typeSmooth++;
	//}
	////--------------------------------------------------------------
	//void nextTypeMean() {
	//	if (typeMean >= typeMean.getMax()) typeMean = 0;
	//	else typeMean++;
	//}

	//--------------------------------------------------------------
	void nextTypeSmooth(int i) {
		if (smoothChannels[i]->typeSmooth >= smoothChannels[i]->typeSmooth.getMax()) smoothChannels[i]->typeSmooth = 1;
		else smoothChannels[i]->typeSmooth++;
	}
	//--------------------------------------------------------------
	void nextTypeMean(int i) {
		if (smoothChannels[i]->typeMean >= smoothChannels[i]->typeMean.getMax()) smoothChannels[i]->typeMean = 0;
		else smoothChannels[i]->typeMean++;
	}

	string helpInfo;

	//--------------------------------------------------------------
	void buildHelp()
	{
		// Help info
		string s = "";
		s += "HELP SMOOTH \n\n";

		if (!guiManager.bKeys)
		{
			s += "Keys toggle is disabled! \n";
			s += "Enable Keys toggle \non Advanced sub menu. \n";
		}
		else {
			s += "H           HELP \n";
			s += "G           GUI \n";
			s += "\n";

			s += "            Type \n";
			s += "TAB         SMOOTH \n";
			s += "SHIFT       MEAN \n";
			s += "\n";

			s += "+|-         THRESHOLD \n";
			s += "\n";

			s += "S           Solo PLOT \n";
			s += "Up|Down     Browse \n";
			s += "\n";

			s += "TESTER \n";
			s += "SPACE       RANDOMIZE \n";
			s += "RETURN      PLAY \n";
		}
		helpInfo = s;

		guiManager.setHelpInfoApp(helpInfo);
	}

	//----

	// Useful for external gui's!

public:

	ofParameter<bool> bGui_Global{ "SMOOTH SURF", true };
	ofParameter<bool> bGui_Main{ "SMOOTH MAIN", true };
	ofParameter<bool> bGui_Plots;
	ofParameter<bool> bGui_PlotsLink;
	ofParameter<bool> bGui_Extra{ "Extra", true };

	ofParameter<bool> bGui_GameMode;

	ofParameter<bool> bGui_PlotFullScreen;
	ofParameter<bool> bGui_PlotIn;
	ofParameter<bool> bGui_PlotOut;

	//--

	ofParameter<int> index; // index of the GUI selected param!
	ofParameter<bool> bSolo; // solo selected index

	//--

	// Channel independent params

	//ofParameter<float> ampInput;
	//ofParameter<float> smoothPower;
	//ofParameter<float> threshold;

	//ofParameter<int> typeSmooth;
	//ofParameter<int> typeMean;
	//ofParameter<string> typeSmooth_Str;
	//ofParameter<string> typeMean_Str;

	//ofParameter<float> timeRedirection;
	//ofParameter<float> slideMin;
	//ofParameter<float> slideMax;
	//ofParameter<float> onsetGrow;
	//ofParameter<float> onsetDecay;

	//ofParameter<bool> bClamp;
	//ofParameter<float> minInput;
	//ofParameter<float> maxInput;
	//ofParameter<bool> bNormalized;
	//ofParameter<float> minOutput;
	//ofParameter<float> maxOutput;

	ofParameter<bool> bReset;

	circleBeatWidget circleBeat;
};

//----


/*

	MORE SNIPPETS FOR INSPIRATION

	Learn how to get
	the processed / smoothed params.

	Somewhere in your ofApp::Update() / Draw()

	//-

	// 1. Just the param values

	int _shapeType = data.getParamIntValue(shapeType);
	int _amount = data.getParamIntValue(amount);
	float _speed = data.getParamFloatValue(speed);

	//-

	// 2. The parameter itself

	ofParameter<int> _amount = data.getParamInt(amount.getName());
	ofParameter<float> _lineWidth = data.getParamFloat(lineWidth.getName());
	ofParameter<float> _separation = data.getParamFloat(separation.getName());

	//-

	// 3. The ofAbstractParameter

	// to be casted to his correct type after
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

	//-

	// 4. The whole group

	// requires more work after, like iterate the group content, get a param by name...etc.
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

	//-

*/