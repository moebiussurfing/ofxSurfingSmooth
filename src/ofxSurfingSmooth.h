#pragma once

/*

	TODO:

	+ plot smoothed only

	+ independent settings for each param.. ?
		threshold, power, pre amp
		make new class

	+ folder for panels

	+ move enablers, testers to advanced. rename to extra

	+ add colors types, vectors, using templates..
		+ avoid crash to unsuported types

	+ add clamp

	+ "real" nested sub-groups tree levels.. ?
	+ add thresholds/onSet independent for each variable/channel: make it functional. add callbacks..
	+ add param to calibrate max history smooth/speed..
	+ fix broke-continuity/state when tweak smooth power "on playing"..
	+ plotting int type should be stepped/not continuous..

*/


#include "ofMain.h"

#include "ofxDataStream.h"

#include "ofxHistoryPlot.h"
#include "ofxSurfingHelpers.h"
#include "ofxSurfing_Timers.h"
#include "ofxSurfingImGui.h"
#include "ofxSurfingBoxInteractive.h"
#include "smoothChannel.h"

#define NUM_GENERATORS 6

#define COLORS_MONCHROME // all plots same color. green

class ofxSurfingSmooth : public ofBaseApp {

private:

	vector<unique_ptr<SmoothChannel>> smoothChannels;

public:

	ofxSurfingSmooth();
	~ofxSurfingSmooth();

	void draw();
	void keyPressed(int key);

private:

	void update(ofEventArgs& args);
	void exit();

	//--

private:

	ofxSurfing_ImGui_Manager guiManager;

	ofParameterGroup params_EditorEnablers;//the enabled params to randomize
	vector<ofParameter<bool>> editorEnablers;
	void drawToggles();

	void doSetAll(bool b);
	void doDisableAll();
	void doEnableAll();

	//--
	//
	// API INITIALIZERS

public:

	void setup(ofParameterGroup& aparams);//main setup method. to all pass the params with one line

private:

	void add(ofParameterGroup aparams);
	void add(ofParameter<float>& aparam);
	void add(ofParameter<bool>& aparam);
	void add(ofParameter<int>& aparam);
	//TODO: add more types

private:

	void addParam(ofAbstractParameter& aparam);

	//--
	// 
	// API GETTERS

public:

	//--------------------------------------------------------------
	ofParameterGroup& getParamsSmoothed() {
		return mParamsGroup_Smoothed;
	}

	//--
	// 
	// Bang/Bonk/Triggers under threshold detectors

//private:
 
	//--------------------------------------------------------------
	bool isTriggered(int i) {//flag true when triggered this index param
		if (i > params_EditorEnablers.size() - 1) {
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!bEnableSmooth || !_bSmooth) return false;

		if (outputs[i].getTrigger()) {
			ofLogVerbose("ofxSurfingSmooth") << "Triggered: " << i;
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool isBonked(int i) {//flag true when triggered this index param
		if (i > params_EditorEnablers.size() - 1) {
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!bEnableSmooth || !_bSmooth) return false;

		if (outputs[i].getBonk()) {
			ofLogVerbose("ofxSurfingSmooth") << "Bonked: " << i;
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	bool isRedirected(int i) {//flag true when signal direction changed
		if (i > params_EditorEnablers.size() - 1) {
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return false;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!bEnableSmooth || !_bSmooth) return false;

		// if the direction has changed and
		// if the time of change is greater than (0.5) timeRedirection sec
		// print the time between changes and amount of change
		if (outputs[i].getDirectionTimeDiff() > timeRedirection && 
			outputs[i].directionHasChanged())
		{
				ofLogVerbose("ofxSurfingSmooth") << "Redirected: " << i << " " << 
					outputs[i].getDirectionTimeDiff() << ", " << 
					outputs[i].getDirectionValDiff();

				return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	int isRedirectedTo(int i) {
		// return 0 when no direction changed. -1 or 1 depending direction.
		// above or below threshold

		int rdTo = 0;

		if (i > params_EditorEnablers.size() - 1) {
			ofLogError("ofxSurfingSmooth") << "Index Out of Range: " << i;
			return rdTo;
		}
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		bool _bSmooth = _p.cast<bool>().get();
		if (!bEnableSmooth || !_bSmooth) return rdTo;

		// if the direction has changed and
		// if the time of change is greater than 0.5 sec
		// print the time between changes and amount of change
		if (outputs[i].getDirectionTimeDiff() > timeRedirection && 
			outputs[i].directionHasChanged())
		{
				ofLogVerbose("ofxSurfingSmooth") << "Redirected: " << i << " " << 
					outputs[i].getDirectionTimeDiff() << ", " << 
					outputs[i].getDirectionValDiff();

				if (outputs[i].getDirectionValDiff() < 0) rdTo = 1;
				else rdTo = -1;
				return rdTo;
		}
		return rdTo;//0
	}


public:

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
		if (i != -1) return isRedirectedTo(i);
		else return 0;
	}

	//----

public:

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

	void doRandomize();//do and set random in min/max range for all params
	//void doRandomize(int index, bool bForce);//do random in min/max range for a param. bForce ignores enabler

	//---

private:

	void setup();
	void startup();
	void setupPlots();
	void updateGenerators();
	void updateSmooths();
	void updateEngine();
	void drawPlots(ofRectangle r);

private:

	ofParameterGroup mParamsGroup;

	ofParameterGroup mParamsGroup_Smoothed;//TODO:
	string suffix = "";
	//string suffix = "_COPY";

	void Changed_Controls_Out(ofAbstractParameter& e);

private:

	vector<ofxDataStream> outputs;//the smooth class
	vector<float> inputs;//feed normalized signals here
	vector<float> generators;//testing signals

	string path_Global;
	string path_Settings;

	//ofxInteractiveRect boxPlots = { "Rect_Plots", "/ofxSurfingSmooth/" };

	public:
	ofxSurfingBoxInteractive boxPlots;

	private:
	int NUM_PLOTS;
	int NUM_VARS;

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

private:

	void Changed_Params(ofAbstractParameter& e);

	bool bDISABLE_CALLBACKS = true;

	ofParameterGroup params;

	ofParameter<bool> bGui_Inputs;
	ofParameter<bool> bGui_Outputs;
	ofParameter<bool> bUseGenerators;

	ofParameter<bool> bEnableSmooth;
	
	/// TODO:remove
	ofParameter<float> input;//for index selected
	ofParameter<float> output;

	ofParameter<bool> bPlay;
	ofParameter<float> playSpeed;

	//bool bDirectionLast = false;

	// tester timers
	int tf;
	float tn;

	void doReset();
	void setupParams();

	bool bTrigManual = false; // flip first
	bool bModeFast = false; // fast generators

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
	void nextTypeSmooth() {
		if (typeSmooth >= typeSmooth.getMax()) typeSmooth = 1;
		else typeSmooth++;
	}
	//--------------------------------------------------------------
	void nextTypeMean() {
		if (typeMean >= typeMean.getMax()) typeMean = 0;
		else typeMean++;
	}

	string helpInfo;

	//--------------------------------------------------------------
	void buildHelp() {
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
			s += "TAB         Smooth Type \n";
			s += "SHIFT       Mean Type \n";
			s += "\n";
			s += "+|-         OnSet Thresholds \n";
			s += "\n";
			s += "S           Solo Plot \n";
			s += "Up|Down     Solo Browse \n";
			s += "\n";
			s += "TESTER\n";
			s += "SPACE       Randomize \n";
			s += "RETURN      Play \n";
		}
		helpInfo = s;

		guiManager.setHelpInfoApp(helpInfo);
	}

	//----

	// Useful for external gui's!

public:

	ofParameter<int> index;// index of the selected param!
	ofParameter<bool> bSolo;// solo selected index

	ofParameter<bool> bClamp;
	ofParameter<float> minInput;
	ofParameter<float> maxInput;
	ofParameter<bool> bNormalized;
	ofParameter<float> minOutput;
	ofParameter<float> maxOutput;

	ofParameter<int> typeSmooth;
	ofParameter<int> typeMean;
	ofParameter<string> typeSmooth_Str;
	ofParameter<string> typeMean_Str;

	ofParameter<float> smoothPower;
	ofParameter<float> threshold;
	ofParameter<float> timeRedirection;
	ofParameter<float> slideMin;
	ofParameter<float> slideMax;
	ofParameter<float> onsetGrow;
	ofParameter<float> onsetDecay;

	ofParameter<bool> bReset;

	ofParameter<bool> bGui{ "SMOOTHER", true };
	ofParameter<bool> bGui_Global{ "SMOOTH SURFER", true };// exposed to use in external gui's
	ofParameter<bool> bGui_Plots;
	ofParameter<bool> bGui_Extra{ "Extra", true };
	
	ofParameter<bool> bPlotFullScreen;
	ofParameter<bool> bPlotIn;
	ofParameter<bool> bPlotOut;
};
