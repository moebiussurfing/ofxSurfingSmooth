#pragma once

/*

TODO:

+ add bool to enable/disable each param..
+ independent settings for each param..
+ "real" nested sub-groups tree levels..
+ add colors types, vectors, using templates..
+ add thresholds/bonk independent for each variable/channel: make it functional. add callbacks..
+ add param to calibrate max history smooth/speed..
+ fix broke-continuity/state when tweak smooth power "on playing"..
+ plotting int type should be stepped/not continuous..

*/


#include "ofMain.h"

#include "ofxDataStream.h"

#include "ofxHistoryPlot.h"
#include "ofxImGui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ofxSurfingHelpers.h"
#include "ofxSurfing_Timers.h"
#include "ofxSurfing_ImGui.h"
#include "ofxInteractiveRect.h"

#define NUM_GENERATORS 6

#define COLORS_MONCHROME

class ofxSurfingSmooth : public ofBaseApp {

public:
	ofxSurfingSmooth();
	~ofxSurfingSmooth();

	//----

public:
	void update(ofEventArgs & args);
	void draw(ofEventArgs & args);
	void exit();

	void keyPressed(int key);
	//void keyReleased(int key);
	//void mouseMoved(int x, int y);
	//void mouseDragged(int x, int y, int button);
	//void mousePressed(int x, int y, int button);
	//void mouseReleased(int x, int y, int button);
	//void mouseEntered(int x, int y);
	//void mouseExited(int x, int y);
	//void windowResized(int w, int h);
	//void gotMessage(ofMessage msg);
	//void dragEvent(ofDragInfo dragInfo);

	//---

	// api initializers
public:
	void setup(ofParameterGroup& aparams);//main setup method. to all pass the params with one line

	void add(ofParameterGroup aparams);
	void add(ofParameter<float>& aparam);
	void add(ofParameter<bool>& aparam);
	void add(ofParameter<int>& aparam);
	void addParam(ofAbstractParameter& aparam);

	void addGroupSmooth_ImGuiWidgets(ofParameterGroup &group);//monitor preview: to populate the widgets inside an ImGui begin/end

	// api getters
public:
	//--------------------------------------------------------------
	ofParameterGroup& getParamsSmoothed() {
		return mParamsGroup_COPY;
	}

public:
	float get(ofParameter<float> &e);
	int get(ofParameter<int> &e);
	float getParamFloatValue(ofAbstractParameter &e);
	int getParamIntValue(ofAbstractParameter &e);
	ofAbstractParameter& getParamAbstract(ofAbstractParameter &e);
	ofAbstractParameter& getParamAbstract(string name);
	ofParameter<float>& getParamFloat(string name);
	ofParameter<int>& getParamInt(string name);

	void doRandomize();

	//---

private:
	void setup();
	void startup();
	void setupPlots();
	void updateGenerators();
	void updateSmooths();
	void updateEngine();
	void drawPlots(ofRectangle r);

	//----

	//private:
	//	enum ParamType {
	//		PTYPE_FLOAT = 0,
	//		PTYPE_INT,
	//		PTYPE_BOOL,
	//		PTYPE_UNKNOWN
	//	};
	//
	//private:
	//	class MidiParamAssoc {
	//	public:
	//		//int midiId = -1;
	//		int paramIndex = 0;
	//		ParamType ptype = PTYPE_UNKNOWN;
	//		//ofRectangle drawRect;
	//		string displayMidiName = "";
	//		//bool bListening = false;
	//		//bool bNeedsTextPrompt = false;
	//		string xmlParentName = "";
	//		string xmlName = "";
	//	}; 
	//vector< shared_ptr<MidiParamAssoc> > mAssocParams;

private:
	ofParameterGroup mParamsGroup;

	ofParameterGroup mParamsGroup_COPY;//TODO:
	string suffix = "";
	//string suffix = "_COPY";

	void Changed_Controls_Out(ofAbstractParameter &e);

private:
	vector<ofxDataStream> outputs;//the smooth class
	vector<float> inputs;//feed normnalized signals here
	vector<float> generators;//testing signals

	string path_Settings = "DataStreamGroup.xml";

	ofxInteractiveRect rectangle_PlotsBg = { "Rect_Plots" };

	int NUM_PLOTS;
	int NUM_VARS;

	vector<ofxHistoryPlot *> plots;
	vector<ofColor> colors;

#ifdef COLORS_MONCHROME
	ofColor colorPlots;
#endif
	ofColor colorSelected;
	ofColor colorBaseLine;

private:
	void Changed_Params(ofAbstractParameter &e);
	bool bDISABLE_CALLBACKS = true;

	ofParameterGroup params;
	ofParameter<bool> enable;
	ofParameter<bool> bFullScreen;
	ofParameter<bool> bShowPlots;
	ofParameter<bool> bShowInputs;
	ofParameter<bool> bShowOutputs;
	ofParameter<bool> bUseGenerators;
	ofParameter<bool> solo;
	ofParameter<int> index;
	ofParameter<int> typeSmooth;
	ofParameter<string> typeSmooth_Str;
	ofParameter<int> typeMean;
	ofParameter<string> typeMean_Str;
	ofParameter<bool> bClamp;
	ofParameter<float> minInput;
	ofParameter<float> maxInput;
	ofParameter<bool> bNormalized;
	ofParameter<float> minOutput;
	ofParameter<float> maxOutput;
	ofParameter<bool> enableSmooth;
	ofParameter<float> smoothPower;
	ofParameter<float> threshold;
	ofParameter<float> onsetGrow;
	ofParameter<float> onsetDecay;
	ofParameter<float> slideMin;
	ofParameter<float> slideMax;
	ofParameter<float> input;//index selected
	ofParameter<float> output;
	ofParameter<bool> bReset;
	ofParameter<bool> bPlay;
	ofParameter<float> playSpeed;

	int tf;
	float tn;

	void doReset();
	void setupParams();

	bool bTrigManual = false;//flip first
	bool bModeFast = false;//fast generators

	ofColor colorBg;

	void setup_ImGui();
	void draw_ImGui();
	ofxImGui::Gui gui;
	ofxImGui::Settings mainSettings = ofxImGui::Settings();
	ImFont* customFont = nullptr;
	ofParameter<bool> bGui{ "Show ImGui", true };
	ofParameter<bool> auto_resize{ "Auto Resize", true };
	ofParameter<bool> bLockMouseByImGui{ "Mouse Locked", false };
	ofParameter<bool> auto_lockToBorder{ "Lock GUI", false };

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
public:
	std::string getHelpInfo() {
		return helpInfo;
	}

public:
	ofParameter<bool> bShowGui{ "SHOW SMOOTH SURFER", true };// exposed to use in external gui's
};
