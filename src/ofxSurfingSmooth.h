#pragma once

/*

	TODO:

	+ fix callback slow when load preset
		at lambda. on add method

	+ check notifier/void from @alptugan
	+ fix hysteresis gate. tempo based. 1 bar i.e.
		+ some bangs are bypassed when checked multiple times
	do the filtering in another "parent" method!
	isBangFiltered(i);

	+ add mini float window with basic controls, vertical and knobs

	+ add bypass detector

	+ plotting int type should be stepped/not continuous.

	+ add clamp and normalization modes.

	+ implement "real" nested sub-groups tree levels.. ?
	+ add colors types, vectors, using templates.
		avoid crash to unsupported types.

*/


#include "ofMain.h"

#include "smoothChannel.h"
#include "ofxDataStream.h"
#include "ofxHistoryPlot.h"
#include "ofxSurfingHelpers.h"
#include "ofxSurfingImGui.h"
#include "ofxSurfingBoxInteractive.h"
#include "circleBeatWidget.h"

//#define COLORS_MONCHROME // Uncomment to draw all plots with the same color.

#define MAX_PRE_AMP_POWER 1.5f

//#define DISABLE_GATE // WIP fixing

// constants
#define MIN_SLIDE 1
#define MAX_SLIDE 120
#define MAX_ACCUM 120 // 1 sec calibrated to 60fps ?

//--

class ofxSurfingSmooth : public ofBaseApp
{

private:

	/*
	// constants
	const int MIN_SLIDE = 1;
	const int MAX_SLIDE = 120;
	const int MAX_ACCUM = 60; // 1 sec calibrated to 60fps ?
	*/
	
	// custom font
	ofTrueTypeFont font;
	int fontSize = 6;

public:

	ofxSurfingSmooth();
	~ofxSurfingSmooth();

	void draw();
	void keyPressed(int key);

private:

	void update(ofEventArgs& args);
	void exit();

	//TODO:
	// Fix workaround to allow multiple bangs..
	//--------------------------------------------------------------
	//void refreshGate(ofEventArgs& args)
	void refreshGate()
	{
		// all the reading has happened,
		// and all has been allowed!
		for (int i = 0; i < gates.size(); i++)
		{
			gates[i].bIsTrig = false;
			//gates[i].bIsTrigState = false;
		}
	};

	//--

private:

	// channels processors
	vector<unique_ptr<SmoothChannel>> smoothChannels;
	ofEventListeners listeners;
	//int iIndex = -1;

public:

	ofxSurfingGui ui;

private:

	ofParameterGroup params_EditorEnablers; // Contents the enabled params to smooth or to bypass.
	vector<ofParameter<bool>> editorEnablers;
	void drawTogglesWidgets();

	void doSetAll(bool b);
	void doDisableAll();
	void doEnableAll();

	//--

	// API INITIALIZERS

public:

	void setup(ofParameterGroup& aparams); // main setup method. to all pass the params with one line.
	void setupCallback(int i);

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
	ofColor getColorPlot(int i) const {
		{
			ofColor c;

#ifdef COLORS_MONCHROME
			c = colorPlots;
#else
			c = colorsPicked[i];
			//c = colors[2 * i];
#endif
			return c;
		}
	}

	//--------------------------------------------------------------
	ofParameterGroup& getParamsSmoothed() {
		return mParamsGroup_Smoothed;
	}

	// We can get the auto generated channel amplifier,
	// to apply that gain to any external signal.
	//--------------------------------------------------------------
	ofParameter<float>& getChannelAmplififer(int ch) {
		if (ch > amountChannels) {
			ofParameter<float> pError{ "error",-1,-1,-1 };
			return pError;
		}
		return smoothChannels[ch]->ampInput;
	}

	//--

	// Hysteresis Gate
	ofParameter<float> bpm{ "BPM", 120, 40, 240 };//we will use that bpm to filter / gate the bangs
	// as an intended to reduce the trigs, 
	// and probably to help the detectors fine tweaking a bit.

private:

	// TODO: could be moved inside the smoothChannel class
	struct GateStruct
	{
		//ofParameter<int> bpmDiv{ "Div", 1, 1, 4 };//divide the bar duration in quarter
		//ofParameter<bool> bGateModeEnable{ "Gate", false };
		ofParameter<bool> bGateStateClosed{ "State", false };
		int tDurationGate;//duration in ms
		int timerGate = 0;//measure in ms
		int lastTimerGate = 0;//last happened in ms

		bool bIsTrig = false;
		//bool bIsTrigState = false;
	};

public:

	vector<GateStruct> gates;

	//--

	//--------------------------------------------------------------
	bool doBangGated(int i, bool bTrig) {

		// Gate
		// Skip and bypass any bang when gate is enabled
		// bGateStateClosed true = locked = gate is active
		if (smoothChannels[i]->bGateModeEnable && gates[i].bGateStateClosed)
		{
			ofLogVerbose("ofxSurfingSmooth") << "Bang Ignored bc gate active for index " << i;
			return false;
		}

		// Gates
		// Set timer
		// a bang happened, but check gate mode / state before accept it!
		if (bTrig)
		{
			// bGateStateClosed = false, gate opened / passing
			// must be opened to listen to it!
			if (smoothChannels[i]->bGateModeEnable && !gates[i].bGateStateClosed)
			{
				gates[i].bGateStateClosed = true; // close. gate activated
				gates[i].lastTimerGate = ofGetElapsedTimeMillis(); // store when happened.

				//gates[i].bIsTrigState = true;
			}
		}

		if (bTrig) gates[i].bIsTrig = true;
		return bTrig;

		//return gates[i].bIsTrigState;
	}

	//void doRefresh();

	//--

	// Bang / Bonk / Triggers under threshold detectors

private:

	// Getting by the index could be probably problematic,
	// bc some param types could be skipped. 
	// we disable to better use getting by name as below.

	//--------------------------------------------------------------
	bool isTriggered(int i) {//flag true when triggered this index param
		if (!bEnableSmoothGlobal) return false;

		// check enabler
		if (i > params_EditorEnablers.size() - 1)
		{
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!smoothChannels[i]->bEnableSmooth || !_bSmooth) return false;

		// Gate

		bool b = outputs[i].getTrigger();

		//b = doBangGated(i, b);

		return b;



		//if (outputs[i].getTrigger()) 
		//{
		//	ofLogVerbose("ofxSurfingSmooth") << "Triggered: " << i;
		//	return true;
		//}
		//return false;
	}

	//--------------------------------------------------------------
	bool isBonked(int i) {//flag true when triggered this index param
		if (!bEnableSmoothGlobal) return false;

		// check enabler
		if (i > params_EditorEnablers.size() - 1)
		{
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!smoothChannels[i]->bEnableSmooth || !_bSmooth) return false;


		// Gate

		bool b = outputs[i].getBonk();

		//b = doBangGated(i, b);

		return b;



		//if (outputs[i].getBonk()) {
		//	ofLogVerbose("ofxSurfingSmooth") << "Bonked: " << i;
		//	return true;
		//}
		//return false;
	}

	//--------------------------------------------------------------
	bool isRedirectedUp(int i) {
		return (isRedirectedTo(i) > 0);
	}
	//--------------------------------------------------------------
	bool isRedirectedDown(int i) {
		return (isRedirectedTo(i) < 0);
	}
	//--------------------------------------------------------------
	bool isRedirected(int i) {//flag true when signal direction changed
		if (!bEnableSmoothGlobal) return false;

		// check enabler
		if (i > params_EditorEnablers.size() - 1)
		{
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!smoothChannels[i]->bEnableSmooth || !_bSmooth) return false;


		// Gate

		bool b = outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
			outputs[i].directionHasChanged();

		//b = doBangGated(i, b);

		return b;



		//// if the direction has changed and
		//// if the time of change is greater than (0.5) timeRedirection sec
		//// print the time between changes and amount of change
		//if (outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
		//	outputs[i].directionHasChanged())
		//{
		//	ofLogVerbose("ofxSurfingSmooth") <<
		//		"Redirected: " << i << " " <<
		//		outputs[i].getDirectionTimeDiff() << ", " <<
		//		outputs[i].getDirectionValDiff();

		//	return true;
		//}
		//return false;
	}

	//--------------------------------------------------------------
	int isRedirectedTo(int i) {
		if (!bEnableSmoothGlobal) return 0;

		// return 0 when no direction changed. -1 or 1 depending direction.
		// above or below threshold
		int rdTo = 0;

		// check enabler
		if (i > params_EditorEnablers.size() - 1) {
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return rdTo;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!smoothChannels[i]->bEnableSmooth || !_bSmooth) return rdTo;


		// Gate

		bool b = outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
			outputs[i].directionHasChanged();

		//b = doBangGated(i, b);

		if (b)
		{
			if (outputs[i].getDirectionValDiff() < 0)
				rdTo = 1;
			else
				rdTo = -1;
		}
		return rdTo;



		//// if the direction has changed and
		//// if the time of change is greater than 0.5 sec
		//// print the time between changes and amount of change
		//if (outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
		//	outputs[i].directionHasChanged())
		//{
		//	ofLogVerbose("ofxSurfingSmooth") << "Redirected: " << i << " " <<
		//		outputs[i].getDirectionTimeDiff() << ", " <<
		//		outputs[i].getDirectionValDiff();
		//	if (outputs[i].getDirectionValDiff() < 0)
		//		rdTo = 1;
		//	else
		//		rdTo = -1;
		//	return rdTo;
		//}
		//return rdTo; // 0=nothing useful
	}

	//--

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
	bool isBang(ofAbstractParameter& e) {
		int i = getIndex(e);
		if (i != -1) return isBang(i);
		else return false;
	}

	//--------------------------------------------------------------
	bool isBang(int i)
	{
		// Returns true if i / passed trigger is happening.
		// then we can pick easily which detector to use 
		// 0 = TrigState, 1 = Bonk, 2 = Direction, 3 = DirUp, 4 = DirDown

		if (i > smoothChannels.size() - 1)
		{
			ofLogError("ofxSurfingSmooth") << "Out of range. Unknown channel/param index: " << i;
			return false;
		}

		//--

		////TODO:
		//// Gates
		//// Skip and bypass any bang when gate is enabled
		//if (smoothChannels[i]->bGateModeEnable && gates[i].bGateStateClosed)
		//{
		//	ofLogNotice("ofxSurfingSmooth") << "Bang Ignored bc gate active for index " << i;
		//	return false;
		//}

		//--

		// Get bangs
		bool bReturn = false;
		switch (smoothChannels[i]->bangDetectorIndex)
		{
				
		case 0: bReturn = isTriggered(i); break; // trigState
		case 1: bReturn = isBonked(i); break; // bonked
		case 2: bReturn = isRedirected(i); break; // redirect
		case 3: bReturn = isRedirectedUp(i); break; // up
		case 4: bReturn = isRedirectedDown(i); break; // down 

		default:
		{
			ofLogError("ofxSurfingSmooth") << "Out of range. Unknown detector index: " << i;
			return false;
			break;
		}

		}

		//--

		// Gates
		// Set timer
		/*
		// a bang happened, but check gate mode / state before accept it!
		if (bReturn)
		{
			// must be opened to listen to it!
			if (smoothChannels[i]->bGateModeEnable && !gates[i].bGateStateClosed)
			{
				gates[i].bGateStateClosed = true;
				gates[i].lastTimerGate = ofGetElapsedTimeMillis();
			}
		}
		*/

		//doBangGated(i, bReturn);

		//--

		return bReturn;
	}

	//--------------------------------------------------------------
	bool isBangGated(ofAbstractParameter& e) {
		int i = getIndex(e);
		if (i != -1) return isBangGated(i);
		else return false;
	}

	//--------------------------------------------------------------
	bool isBangGated(int i)
	{
#ifdef DISABLE_GATE
		isBang(i);
#endif
		//--
		
		//bool b = isBang(i);

		////return doBangGated(i, b);
		//doBangGated(i, b);

		//return gates[i].bIsTrig;


		bool b = isBang(i);
		return doBangGated(i, b);


		/*
		//TODO:
		// Gates
		// Skip and bypass any bang when gate is enabled
		if (smoothChannels[i]->bGateModeEnable && gates[i].bGateStateClosed)
		{
			ofLogNotice("ofxSurfingSmooth") << "Bang Ignored bc gate active for index " << i;
			return false;
		}

		return isBang(i);
		*/
	}

	//----

	// API

public:

	// Some helpers to 
	// Get the smoothed params
	// by different approaches.

	int getIndex(ofAbstractParameter& e) /*const*/;

	float get(ofParameter<float>& e);
	int get(ofParameter<int>& e);

	float getParamFloatValue(ofAbstractParameter& e);
	int getParamIntValue(ofAbstractParameter& e);

	ofAbstractParameter& getParamAbstract(ofAbstractParameter& e);
	ofAbstractParameter& getParamAbstract(string name);
	ofParameter<float>& getParamFloat(ofParameter<float>& p);
	ofParameter<int>& getParamInt(ofParameter<int>& p);
	ofParameter<float>& getParamFloat(string name);
	ofParameter<int>& getParamInt(string name);

	//----

public:

	void doRandomize(); // do and set random in min/max range for all params
	//void doRandomize(int index, bool bForce); // do random in min/max range for a param. bForce ignores enabler

	//---

	void startup();
	void doRefresh();

private:

	void setup();

	void updateGenerators();
	void updatePlayer();
	void updateDetectors();
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

	vector<float> inputs; // feed normalized signals here.
	vector<ofxDataStream> outputs; // the smooth/detect class to get the output from.

	// Signal Generator
	ofxSurfingHelpers::SurfGenerators surfGenerator;

	//--

	string path_Global;
	string path_Settings;

public:

	ofxSurfingBoxInteractive boxPlots;

	ofParameter<float> alphaPlotInput;

private:

	int amountPlots;
	int amountChannels;

	vector<ofxHistoryPlot*> plots;
	vector<ofColor> colors;

	ofColor colorPlots;

#ifndef COLORS_MONCHROME
	vector<ofColor> colorsPicked;
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

	ofParameter<bool> bEnableSmoothGlobal; // global enable

	ofParameter<bool> bPlay;
	ofParameter<float> playSpeed;

	ofParameter<bool> bGenerators;//signal generators for testing

public:

	void disableGenerators() { bGenerators = false; }

	void doReset();

private:

	// Tester timers
	int tf;
	float tn;

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

		if (!ui.bKeys)
		{
			s += "Keys toggle is disabled! \n";
			s += "Enable Keys toggle \non Advanced sub menu. \n";
		}
		else {
			s += "H           HELP \n";
			s += "G           GUI \n";
			s += "\n";

			s += "            TYPE \n";
			s += "TAB         SMOOTH \n";
			s += "SHIFT       MEAN \n";
			s += "\n";

			s += "+|-         THRESHOLD \n";
			s += "\n";

			s += "S           SOLO PLOT \n";
			s += "Up|Down     BROWSE \n";
			s += "\n";

			s += "TESTER \n";
			s += "SPACE       RANDOM \n";
			s += "RETURN      PLAY \n";
		}
		helpInfo = s;

		ui.setHelpInfoApp(helpInfo);
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

	ofParameter<void> bReset;

	CircleBeatWidget circleBeat_Widget;
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