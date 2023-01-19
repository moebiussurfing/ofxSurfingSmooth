﻿#include "ofxSurfingSmooth.h"

//--------------------------------------------------------------
ofxSurfingSmooth::ofxSurfingSmooth()
{
	ofAddListener(ofEvents().update, this, &ofxSurfingSmooth::update);
	//ofAddListener(ofEvents().draw, this, &ofxSurfingSmooth::refreshGate, OF_EVENT_ORDER_APP);
}

//--------------------------------------------------------------
ofxSurfingSmooth::~ofxSurfingSmooth()
{
	ofRemoveListener(ofEvents().update, this, &ofxSurfingSmooth::update);
	//ofRemoveListener(ofEvents().draw, this, &ofxSurfingSmooth::refreshGate);

	exit();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setup()
{
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	path_Global = "ofxSurfingSmooth/";
	path_Settings = path_Global + "ofxSurfingSmooth_Settings.xml";
	ofxSurfingHelpers::CheckFolder(path_Global);

	//--

	setupParams();

	//--

	editorEnablers.clear();// an enabler toggler for each param
	params_EditorEnablers.clear();// an enabler toggler for each param
	params_EditorEnablers.setName("Params");

	//--

	setup_ImGui();

	// Init state
	// to help user
	// force
	//bGui_Main = true;
	bGui_Main = false;
	bGui_GameMode = true;
	bGenerators = false;
	bPlay = false;
	ui.bHelp = true;
	ui.bKeys = true;

	// font
	std::string _path = "assets/fonts/"; // assets folder
	string f = "JetBrainsMono-Bold.ttf";
	_path += f;
	bool b = font.load(_path, fontSize);
	if (!b) font.load(OF_TTF_MONO, fontSize);

	//--

	mParamsGroup.setName("ofxSurfingSmooth");
}

//--------------------------------------------------------------
void ofxSurfingSmooth::doRefresh() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	for (int i = 0; i < smoothChannels.size(); i++) {
		smoothChannels[i]->doRefresh();
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::startup() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	bDISABLE_CALLBACKS = false;

	doReset();

	//// Init state
	//// to help user
	//// force
	//bGui_Main = false;
	//bGui_GameMode = true;
	//bGenerators = true;
	//ui.bHelp = true;
	//ui.bKeys = true;

	//--

	// settings
	ofxSurfingHelpers::loadGroup(params, path_Settings);

	// Load file settings
	for (int i = 0; i < smoothChannels.size(); i++)
	{
		//fix
		//outputs[i].initAccum(0);
		smoothChannels[i]->surfingPresets.setUiPtr(&ui);
		smoothChannels[i]->setPathGlobal(path_Global + "_" + ofToString(i));
		smoothChannels[i]->startup();
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setupPlots()
{
	//TODO: should count only if added successfully, 
	// not filtered by supported styles or not
	amountChannels = mParamsGroup.size();

	amountPlots = 2 * amountChannels;

	index.setMax(amountChannels - 1);
	plots.resize(amountPlots);


	// strings to debug detectors
	strDetectorsState.assign(4, " ");

	//--

	// Plot colors

	// One color for all channels
	// pick a color
#ifdef COLORS_MONCHROME
	colorPlots = (ofColor::yellow);
	//colorPlots = (ofColor::green);
#else
	// Pick some colors
	colorsPicked.push_back(ofColor::turquoise);
	colorsPicked.push_back(ofColor::orange);
	colorsPicked.push_back(ofColor::pink);
	colorsPicked.push_back(ofColor::green);
#endif

	colorBaseLine = ofColor(255, 100);
	colorTextSelected = ofColor(255, 150);

	colorThreshold = ofColor(150);

	//// Style 1
	//colorTrig = ofColor(ofColor::red);
	//colorBonk = ofColor(ofColor::blue);
	//colorDirect = ofColor(ofColor::green);
	//colorDirectUp = ofColor(ofColor::green);
	//colorDirectDown = ofColor(ofColor::green);

	// Style 2. All the same colors 
#ifdef COLORS_MONCHROME
	colorTrig = colorPlots;
	colorBonk = colorPlots;
	colorDirect = colorPlots;
	colorDirectUp = colorPlots;
	colorDirectDown = colorPlots;
#else
	colorPlots = (ofColor::yellow);
	colorTrig = colorPlots;
	colorBonk = colorPlots;
	colorDirect = colorPlots;
	colorDirectUp = colorPlots;
	colorDirectDown = colorPlots;
#endif

	//--

	// Circle beat widget
	circleBeat_Widget.setName("SmoothWidget");
#ifdef COLORS_MONCHROME
	circleBeat_Widget.setColor(0, colorTrig);
	circleBeat_Widget.setColor(1, colorBonk);
	circleBeat_Widget.setColor(2, colorDirect);
	circleBeat_Widget.setColor(3, colorDirectUp);
	circleBeat_Widget.setColor(4, colorDirectDown);
#endif

	// Colors
	colors.clear();
	colors.resize(amountPlots);

	// Alphas
	int a1 = 128;//input
	int a2 = 255;//output
	ofColor c;

#ifdef COLORS_MONCHROME
	for (int i = 0; i < amountChannels; i++)
	{
		c = colorPlots;
		colors[2 * i] = ofColor(c, a1);//in
		colors[2 * i + 1] = ofColor(c, a2);//out
	}
#else
	int sat = 255;
	int brg = 255;
	int hueStep = 255. / (float)amountChannels;
	for (int i = 0; i < amountChannels; i++)
	{
		if (i >= 0 && i < colorsPicked.size()) c = colorsPicked[i];
		else c.setHsb(hueStep * i, sat, brg);

		colors[2 * i] = ofColor(c, a1);
		colors[2 * i + 1] = ofColor(c, a2);
	}
#endif

	for (int i = 0; i < amountPlots; i++)
	{
		string _name;
		string _name2;

		_name2 = ofToString(mParamsGroup[i / 2].getName());//param name
		//_name2 = ofToString(i / 2);//index as name

		//TODO: fix when disable out plot..
		// 1st plot of each var. input
		// has bg and name labels
		bool b1 = (i % 2 == 0);
		_name = _name2;

		//if (b1) _name = "Input " + _name2;
		//else _name = "Output " + _name2;

		bool bTitle = !b1;
		bool bInfo = false;
		bool bBg = b1;
		bool bGrid = false && b1;

		const int _durationPlotInSecs = 4;
		// plot buffer 4 secs at 60fps

		plots[i] = new ofxHistoryPlot(NULL, _name, 60 * _durationPlotInSecs, false);
		plots[i]->setRange(0, 1);//TODO: normalized
		plots[i]->setColor(colors[i]);
		plots[i]->setDrawTitle(bTitle);
		plots[i]->setShowNumericalInfo(bInfo);
		plots[i]->setShowSmoothedCurve(false);
		plots[i]->setDrawBackground(bBg);
		plots[i]->setDrawGrid(bGrid);
	}

	// draggable rectangle

	//// default
	// //do not actuates
	//boxPlots.setWidth(800);
	//boxPlots.setHeight(400);
	//boxPlots.setEdit(true);

	boxPlots.setBorderColor(ofColor::yellow);
	boxPlots.setPathGlobal(path_Global);
	boxPlots.setup();
	boxPlots.bEdit.setName("Edit Plots");
}

//--------------------------------------------------------------
void ofxSurfingSmooth::update(ofEventArgs& args)
{
	//TODO:
	// fix gate workflow
	//refreshGate();

	// Force input params to test detectors!
	{
		// Generators
		if (bGenerators) updateGenerators();

		// Tester
		updatePlayer();
	}

	//--

	updateDetectors();

	//--

	// Engine
	updateEngine();

	// Smooths
	updateSmooths(); // always
	//if (!bGenerators) updateSmooths(); // if generator disabled
}

//--------------------------------------------------------------
void ofxSurfingSmooth::updateSmooths()
{
	// Getting the source values 
	// from the params input params,
	// not from the generators!

	// 1. Prepare input. Get source param. Normalize the value.
	// 2. Feed the smoother. (made by the ofxDataStream objects!) to start calculate. 
	// 3. Get smoothed output.

	//TODO: do not use the group to iterate! 
	// bc could not correlate elements, 
	// if some types are skipped!

	for (int i = 0; i < mParamsGroup.size(); i++)
	{
		ofAbstractParameter& p = mParamsGroup[i];

		// Enabler toggle
		auto& _p = params_EditorEnablers[i]; // ofAbstractParameter
		bool isBool = (_p.type() == typeid(ofParameter<bool>).name());
		string name = _p.getName();
		bool _bEnabler = _p.cast<bool>().get();

		//--

		// The value to be feeded into input.
		// Input content is always normalized! (0 to 1)

		float _valueToInput = 0;

		//--

		// Float

		if (p.type() == typeid(ofParameter<float>).name())
		{
			//--

			// 1. Prepare input. Get source param. Normalize the value.

			ofParameter<float> _p = p.cast<float>();

			//float g = 1;
			//float g = ofMap((float)MAX_PRE_AMP_POWER * smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
			float g = ofMap(smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
			g *= (float)MAX_PRE_AMP_POWER;

			_valueToInput = ofMap(
				_p * g,
				_p.getMin(), _p.getMax(), 0, 1, true);

			//--

			// 3. Get calculated output

			// to the smooth group
			auto pc = mParamsGroup_Smoothed.getFloat(_p.getName() + suffix);

			// A. get smoothed
			if (smoothChannels[i]->bEnableSmooth && _bEnabler && bEnableSmoothGlobal)
			{
				float v = ofMap(
					outputs[i].getValue(), 0, 1,
					_p.getMin(), _p.getMax(), true);

				pc.set(v);
			}

			// B. get from source
			else
			{
				pc.set(_p.get());
			}
		}

		//--

		// Int

		else if (p.type() == typeid(ofParameter<int>).name())
		{
			// 1. Prepare input. normalize param

			ofParameter<int> _p = p.cast<int>();

			//float g = 1;
			//float g = ofMap((float)MAX_PRE_AMP_POWER * smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
			float g = ofMap(smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
			g *= (float)MAX_PRE_AMP_POWER;

			_valueToInput = ofMap(
				_p * g,
				_p.getMin(), _p.getMax(), 0, 1, true);

			//--

			// 3. Get calculated output

			// to the smooth group
			auto pc = mParamsGroup_Smoothed.getInt(_p.getName() + suffix);

			// A. get smoothed
			if (smoothChannels[i]->bEnableSmooth && _bEnabler && bEnableSmoothGlobal)
			{
				int v = ofMap(
					outputs[i].getValue(), 0, 1,
					_p.getMin(), _p.getMax() + 1, true);//TODO: int round fix..

				pc.set(v);
			}

			// B. get from source
			else
			{
				pc.set(_p.get());
			}
		}

		//--

		// Bool
		// ignored
		//else if (p.type() == typeid(ofParameter<bool>).name()) {
		//	ofParameter<bool> ti = p.cast<bool>();
		//}

		//TOOD: could add other types like multi-dim glm etc
		// ignored
		else
		{
			continue;
		}

		//--

		// 1. Feed input
		// prepared and feed the input with the normalized parameter
		inputs[i] = _valueToInput;

		//-

		// 2. Feed the smoother 
		// (made by the ofxDataStream objects!)
		// to start calculate, 
		// smooth and process detectors!
		outputs[i].update(inputs[i]);
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::updateEngine()
{
	//if (!bEnableSmoothGlobal) return;

	for (int i = 0; i < amountChannels; i++)
	{
		//smoothChannels[i]->update();

		//--

		// Enabler toggle

		auto& p = params_EditorEnablers[i]; // ofAbstractParameter
		bool isBool = (p.type() == typeid(ofParameter<bool>).name());
		if (!isBool || i > params_EditorEnablers.size() - 1)
		{
			ofLogError("ofxSurfingSmooth") << (__FUNCTION__) <<
				"Out o range. Skip param #" << i;

			continue;
		}
		bool _bEnabler = p.cast<bool>().get();

		//--

		// 1. Get from input

		// get input as source raw or clamped
		float _input;

		if (smoothChannels[i]->bClamp)
			_input = ofClamp(inputs[i], smoothChannels[i]->minInput, smoothChannels[i]->maxInput);
		else
			_input = inputs[i];

		//--

		// 2. Feed Plots

		if (bGui_Plots)
		{
			// Input

			// feed the source signal to the input Plot 
			plots[2 * i]->update(_input);

			//--

			// Output

			// use the filtered signal if enabled
			if (smoothChannels[i]->bEnableSmooth && _bEnabler)
				plots[2 * i + 1]->update(outputs[i].getValue());

			// use the raw source signal
			else
				plots[2 * i + 1]->update(_input);
		}
	}

	//----

	/*
	// index of the selected param!

	// toggle
	int i = index;
	auto& _p = params_EditorEnablers[i];// ofAbstractParameter
	bool _bEnabler = _p.cast<bool>().get();

	// output. get the output as it is or normalized
	if (bEnableSmooth && _bEnabler)
	{
		if (bNormalized) output = outputs[index].getValueN();
		else output = outputs[index].getValue();
	}
	else // bypass
	{
		output = input;
	}
	*/

	//----

	/*
	//TODO:
	// Log bangs / onSets but for only the selected/index channel/param !
	// add individual thresholds
	// add callbacks notifiers

	for (int i = 0; i < amountChannels; i++)
	{
		if (outputs[i].getTrigger())
		{
			if (i == index) ofLogVerbose("ofxSurfingSmooth") << (__FUNCTION__) << "Trigger: " << i;
		}

		if (outputs[i].getBonk())
		{
			if (i == index) ofLogVerbose("ofxSurfingSmooth") << (__FUNCTION__) << "Bonk: " << i;
		}

		// if the direction has changed and
		// if the time of change is greater than 0.5 sec
		// print the time between changes and amount of change
		if (outputs[i].getDirectionTimeDiff() > 0.5f && outputs[i].directionHasChanged())
		{
			if (i == index)
			{
				ofLogVerbose("ofxSurfingSmooth") << (__FUNCTION__) << "Direction: " << i << " " <<
					outputs[i].getDirectionTimeDiff() << " " <<
					outputs[i].getDirectionValDiff();
			}
		}

		if (i == index) break;//already done! bc we are monitoring the selected channel!
	}
	*/
}

//--------------------------------------------------------------
void ofxSurfingSmooth::updateDetectors()
{
	// Gate engine
	// put back gates to false (disable) when passed timer duration!
	uint32_t t = ofGetElapsedTimeMillis();
	for (size_t i = 0; i < gates.size(); i++)
	{
		if (smoothChannels[i]->bGateModeEnable)
		{
			// Calculate duration depending on mode slow/fast

			// Fast 
			if (!smoothChannels[i]->bGateSlow)
				gates[i].tDurationGate = (60000 / bpm) / (smoothChannels[i]->bpmDiv);

			// Slow
			else
				gates[i].tDurationGate = (60000 / bpm) * (2 * smoothChannels[i]->bpmDiv);

			// True is for when gate is closed 
			if (gates[i].bGateStateClosed)
			{
				// calculate time to open the gate
				// if timer passed the target duration
				gates[i].timerGate = (t - gates[i].lastTimerGate);
				if (gates[i].timerGate > gates[i].tDurationGate)
				{
					gates[i].bGateStateClosed = false; // re open / release the gate!
				}
			}
		}

		//isBang(i);
	}


	////--

	//TODO: it seems that calling isBang closes the gate!
	// so, can on be readed once per frame! 
	// and followed isBang are ignored, returning always false!
	// should use a notifier/callback approach.

	// Gui Detector bang
	// showing the selected channel by the gui index
	// 0 = TrigState, 1 = Bonk, 2 = Direction, 3 = DirUp, 4 = DirDown
	int i = index.get();

	// Customize
	// Circle beat widget
#ifndef COLORS_MONCHROME
	ofColor c = colors[2 * i];
	circleBeat_Widget.setColor(0, c);
	circleBeat_Widget.setColor(1, ofColor(ofColor::black, 64));
	//circleBeat_Widget.setColor(0, c);
	//circleBeat_Widget.setColor(1, c);
	//circleBeat_Widget.setColor(2, c);
	//circleBeat_Widget.setColor(3, c);
	//circleBeat_Widget.setColor(4, c);
#endif

	//if (this->isBangGated(i))
	if (this->isBang(i))
	{
		bool b = false;//pass hard when true
		if (!smoothChannels[i]->bGateModeEnable) b = true;
		else if (smoothChannels[i]->bGateModeEnable && !gates[i].bGateStateClosed) b = true;
		else if (smoothChannels[i]->bGateModeEnable && gates[i].bGateStateClosed) b = false;

		//if (b) circleBeat_Widget.bang(0);//hard

		if (b) circleBeat_Widget.bang(0);//hard
		else circleBeat_Widget.bang(1);//weak/black



		/*
		switch (smoothChannels[i]->bangDetectorIndex)
		{
		case 0: circleBeat_Widget.bang(0); break;
		case 1: circleBeat_Widget.bang(1); break;
		case 2: circleBeat_Widget.bang(2); break;
		case 3: circleBeat_Widget.bang(3); break;
		case 4: circleBeat_Widget.bang(4); break;
		default:  circleBeat_Widget.bang(0); break;
		}
		*/
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::updatePlayer() {
	// Play timed randoms for each channel/params
	{
		static const int _secs = 2;//wait max 
		if (bPlay)
		{
			int max = ofMap(playSpeed, 0, 1, 60, 5) * _secs;
			tf = ofGetFrameNum() % max;
			tn = ofMap(tf, 0, max, 0, 1);
			if (tf == 0)
			{
				doRandomize();
			}
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::updateGenerators() {
	//if (!bGenerators) return;

	// Run signal generators
	// for testing smooth engine 
	// and detectors

	surfGenerator.update();

	//----

	//outputs[i].update(inputs[i]); // raw value, index (optional)

	//----

	// Feed generators to parameters

	for (int i = 0; i < mParamsGroup.size(); i++)
	{
		ofAbstractParameter& aparam = mParamsGroup[i];

		// Prepare and feed generator into the input,
		// overwritten the input/source params them self!

		if (i < surfGenerator.size())
		{
			float value = 0;

			//--

			// Float

			if (aparam.type() == typeid(ofParameter<float>).name())
			{
				ofParameter<float> ti = aparam.cast<float>();
				value = surfGenerator.get(i);

				//float g = 1;
				//float g = ofMap((float)MAX_PRE_AMP_POWER * smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
				float g = ofMap(smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
				g *= MAX_PRE_AMP_POWER;

				value = ofClamp(g * value, 0, 1);

				// feed param ranged
				ti.set(ofMap(value, 0, 1, ti.getMin(), ti.getMax()));

				// feed input normalized
				inputs[i] = value;
			}

			// Int

			else if (aparam.type() == typeid(ofParameter<int>).name())
			{
				ofParameter<int> ti = aparam.cast<int>();
				value = surfGenerator.get(i);

				//float g = 1;
				//float g = ofMap((float)MAX_PRE_AMP_POWER * smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
				float g = ofMap(smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
				g *= MAX_PRE_AMP_POWER;

				value = ofClamp(g * value, 0, 1);

				// feed param ranged
				ti.set((int)ofMap(value, 0, 1, ti.getMin(), ti.getMax()));

				// feed input normalized
				inputs[i] = value;
			}

			//--

			// Bool

			//--
			/*
			// Skip / by pass the other params bc unknown types
			else
			{
				// i--; // to don't discard the generator..
				//inputs[i] = value;
				continue;
			}
			*/
		}

		//--

		//TODO:
		//outputs[i].update(inputs[i]); 
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw()
{
	//if (bGui) draw_ImGui();
	draw_ImGui();

	//--

	if (!bGui_Global) return;
	if (!bGui_PlotIn && !bGui_PlotOut) return;

	//if (!bGui) return;

	if (bGui_Plots)
	{
		if (bGui_PlotFullScreen) drawPlots(ofGetCurrentViewport());
		else
		{
			//fix draw bg
			if (boxPlots.isVisible())
				if (!bGui_PlotIn && !bGui_PlotOut)
				{
					ofColor c = plots[0]->getBackgroundColor();
					ofPushStyle();
					ofFill();
					ofSetColor(c);
					ofDrawRectangle(boxPlots.getRectangle());
					ofPopStyle();
				}

			if (boxPlots.isVisible()) drawPlots(boxPlots.getRectangle());
		}

		if (!bGui_PlotFullScreen)
		{
			boxPlots.draw();

			if (boxPlots.isVisible())
				if (boxPlots.isEditing()) boxPlots.drawBorderBlinking();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::doRandomize() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	for (int i = 0; i < mParamsGroup.size(); i++) {
		auto& p = mParamsGroup[i];

		if (p.type() == typeid(ofParameter<float>).name()) {
			ofParameter<float> pr = p.cast<float>();
			pr = ofRandom(pr.getMin(), pr.getMax());
		}
		else if (p.type() == typeid(ofParameter<int>).name()) {
			ofParameter<int> pr = p.cast<int>();
			pr = ofRandom(pr.getMin(), pr.getMax());
		}
	}
}

////--------------------------------------------------------------
//void ofxSurfingSmooth::doRandomize(int index, bool bForce) {
//	ofLogVerbose("ofxSurfingSmooth")<<(__FUNCTION__) << index;
//
//	int i = index;
//
//	//for (auto p : editorEnablers)
//	//for (int i = 0; i<editorEnablers.size(); i++)
//	{
//		auto p = editorEnablers[i];
//
//		if (!bForce)
//			if (!p.get()) return;//only reset this param if it's enabled
//
//		//-
//
//		string name = p.getName();//name
//		auto &g = params_EditorGroups.getGroup(name);//ofParameterGroup
//		auto &e = g.get(name);//ofAbstractParameter
//
//		auto type = e.type();
//		bool isFloat = type == typeid(ofParameter<float>).name();
//		bool isInt = type == typeid(ofParameter<int>).name();
//		bool isBool = type == typeid(ofParameter<bool>).name();
//
//		if (isFloat)
//		{
//			auto pmin = g.getFloat("Min").get();
//			auto pmax = g.getFloat("Max").get();
//			ofParameter<float> p0 = e.cast<float>();
//			p0.set((float)ofRandom(pmin, pmax));//random
//		}
//
//		else if (isInt)
//		{
//			auto pmin = g.getInt("Min").get();
//			auto pmax = g.getInt("Max").get();
//			ofParameter<int> p0 = e.cast<int>();
//			p0.set((int)ofRandom(pmin, pmax + 1));//random
//		}
//
//		else if (isBool)
//		{
//			bool b = (ofRandom(0, 2) >= 1);
//			ofParameter<bool> p0 = e.cast<bool>();
//			p0.set(b);
//		}
//	}
//}

//--------------------------------------------------------------
void ofxSurfingSmooth::drawTogglesWidgets() {
	for (int i = 0; i < editorEnablers.size(); i++)
	{
		//numerize
		string tag;//to push ids
		string n = "#" + ofToString(i < 10 ? "0" : "") + ofToString(i);
		//ImGui::Dummy(ImVec2(0,10));
		ImGui::Text(n.c_str());
		ImGui::SameLine();
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::drawPlots(ofRectangle r) {

	const int dur = 300; // maintain flags blinking in ms

	ofPushStyle();

	int hh = r.getHeight();
	int ww = r.getWidth();
	int x = r.getX();
	int y = r.getY();
	int h;

	if (!bSolo)
	{
		h = hh / amountChannels; // multiplot height
	}
	else // bSolo
	{
		h = hh; // full height on bSolo
	}

	for (int i = 0; i < amountChannels; i++)
	{
		if (bSolo) if (i != index) continue;//skip

		int ii = 2 * i;

		//// grid
		//int hg = h / 2;
		//plot[ii]->setGridUnit(hg);
		//plot[ii + 1]->setGridUnit(hg);

		if (bGui_PlotIn)
			plots[ii]->draw(x, y, ww, h); // in

		if (bGui_PlotOut)
			plots[ii + 1]->draw(x, y, ww, h); //out

		// add labels
		string s;
		string sp;
		string _spacing = "  ";
		//string _spacing = "\t";

		// # number
		if (!bSolo)
			ofSetColor(colorTextSelected);
		else
		{
			if (i == index)
				ofSetColor(colorTextSelected);
			else
				ofSetColor(colorBaseLine);
		}

		s = "";
		//s = _spacing;
		s += "#" + ofToString(i);

		// Add param name and/or value
		// useful when drawing only output, or to show full range values, not only normalized
		//if (0)
		{
			int ip = i;
			if (ip < outputs.size())
			{
				ofAbstractParameter& p = mParamsGroup[ip];

				if (p.type() == typeid(ofParameter<int>).name())
				{
					ofParameter<int> tp = p.cast<int>();
					int tmpRef = ofMap(outputs[ip].getValue(), 0, 1, tp.getMin(), tp.getMax());
					//sp = tp.getName() + _spacing;//name
					sp += ofToString(tmpRef);//value
				}

				else if (p.type() == typeid(ofParameter<float>).name())
				{
					ofParameter<float> tp = p.cast<float>();
					float tmpRef = ofMap(outputs[ip].getValue(), 0, 1, tp.getMin(), tp.getMax());
					//sp = tp.getName() + _spacing;//name
					sp += ofToString(tmpRef, 2);//value
				}
			}
			s += _spacing + " " + sp;
		}

		// space
		s += " " + _spacing;
		s += " " + _spacing;

		// add flags for trig, bonk, redirected.

		// Trigged
		if (outputs[i].getTrigger())
			strDetectorsState[0] = "+";
		else
			strDetectorsState[0] = "-";
		s += strDetectorsState[0];

		// Bonked
		//if (isBonked(i))
		if (outputs[i].getBonk())
			strDetectorsState[1] = "+";
		else
			strDetectorsState[1] = "-";
		s += strDetectorsState[1];

		// Redirected
		//if (isRedirected(i))
		bool b = outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
			outputs[i].directionHasChanged();
		if (b)
			strDetectorsState[2] = "+";
		else
			strDetectorsState[2] = "-";
		s += strDetectorsState[2];

		//if (isRedirectedTo(i) == 0) s += " " + _spacing; // redirected
		//else if (isRedirectedTo(i) < 0) s += "-" + _spacing; // redirected
		//else if (isRedirectedTo(i) > 0) s += "+" + _spacing; // redirected

		// Time to retain / latched
		uint32_t t = ofGetElapsedTimeMillis();
		static int tlastBonk = 0;
		static int tlastRedirect = 0;
		static int tlastRedirectUp = 0;
		static int tlastRedirectDown = 0;
		static bool bDirectionLastUp = false; // true=up false=down

		if (isRedirectedTo(i) < 0)
		{
			bDirectionLastUp = false; // redirected down
			tlastRedirect = t;
			tlastRedirectDown = t;
		}
		else if (isRedirectedTo(i) > 0)
		{
			bDirectionLastUp = true; // redirected up
			tlastRedirectUp = t;
			tlastRedirect = t;
		}

		if (t - tlastRedirect < dur)
		{
			if (bDirectionLastUp) // redirected up
				strDetectorsState[3] = ">";

			else // redirected down
				strDetectorsState[3] = "<";
		}
		else
		{
			strDetectorsState[3] = "-";
		}
		s += strDetectorsState[3];

		//--

		// Draw display Text
		//ofDrawBitmapString(s, x + 5, y + 11);
		//string text = varName + string(haveData ? (" " + ofToString(cVal, precision)) : "");
		//ofDrawBitmapString(text, x + w - (text.length()) * 8  , y + 10);
		//float _w = font.getStringBoundingBox(text, 0, 0).getWidth() + 5;
		//font.drawString(text, x + w - _w, y + 10);
		font.drawString(s, x + 5, y + 11);

		//--

		// Draw threshold line overlayed
		{
			/// each channel colored differently
			//ofColor _c1 = ofColor(colors[ii]);
			//ofColor _c2 = ofColor(colors[ii]);

			ofColor _c = colorThreshold; // white

			// all detectors as yellow
			ofColor _c0 = colorTrig;
			ofColor _c1 = colorBonk;
			ofColor _c2 = colorDirect;
			ofColor _c2up = colorDirectUp;
			ofColor _c2down = colorDirectDown;

			//// each detector same plot color
			//ofColor _cp = colors[2 * i];
			//ofColor _c0 = _cp;
			//ofColor _c1 = _cp;
			//ofColor _c2 = _cp;
			//ofColor _c2up = _cp;
			//ofColor _c2down = _cp;

			// alpha blink
			float _speedfast = 0.2;
			float _speedslow = 0.4;
			int _amax = 225;
			int _amin = 180;
			int _a = (int)ofMap(ofxSurfingHelpers::Bounce(_speedslow), 0, 1, _amin - 60, _amax - 60, true); // Standby
			int _a0 = (int)ofMap(ofxSurfingHelpers::Bounce(_speedfast), 0, 1, _amin, _amax, true); // TriggedState
			int _a1 = (int)ofMap(ofxSurfingHelpers::Bounce(_speedfast), 0, 1, _amin, _amax, true); // Bonked
			int _a2 = (int)ofMap(ofxSurfingHelpers::Bounce(_speedfast), 0, 1, _amin, _amax, true); // Redirect

			//line width
			int lmin = bSolo ? 3 : 2;
			int lmax = bSolo ? 5 : 3;
			int l;


			static int tlastRedirect2 = 0;
			static int tlastRedirectUp2 = 0;
			static int tlastRedirectDown2 = 0;

			ofColor c;

			if (!smoothChannels[i]->bEnableSmooth) // disabled
			{
				c.set(ofColor(_c, _amin / 2));
				l = lmin;
			}
			else { // enabled
				switch (smoothChannels[i]->bangDetectorIndex)
				{
				case 0: // trigger
				{
					if (outputs[i].getTrigger())
					{
						c.set(ofColor(_c0, _a0));
						l = lmax;
					}
					else
					{
						c.set(ofColor(_c, _a));
						l = lmin;
					}
				}
				break;

				case 1: // bonk
				{
					bool b = outputs[i].getBonk();
					//if (outputs[i].getBonk())
					//{
					//	tlastBonk = t;
					//}

					if (b || ((t - tlastBonk) < dur))
					{
						if (b) tlastBonk = t;
						c.set(ofColor(_c1, _a1));
						l = lmax;
					}
					else
					{
						c.set(ofColor(_c, _a));
						l = lmin;
					}
				}
				break;

				case 2: // redirect
				{
					////if (outputs[i].directionHasChanged() || (t - tlastRedirect < dur))
					//if (isRedirected(i) || (t - tlastRedirect2) < dur)
					bool b = outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
						outputs[i].directionHasChanged();
					if (b || (t - tlastRedirect2) < dur)
					{
						if (b) tlastRedirect2 = t;
						//if (isRedirected(i)) tlastRedirect2 = t;
						//tlastRedirectUp = t;
						//tlastRedirectDown = t;

						c.set(ofColor(_c2, _a2));
						l = lmax;
					}
					else
					{
						c.set(ofColor(_c, _a));
						l = lmin;
					}
				}
				break;

				case 3: // up
				{
					////if ((outputs[i].directionHasChanged() && bDirectionLastUp) || (t - tlastRedirectUp < dur))
					//if ((isRedirectedTo(i) > 0) || (t - tlastRedirectUp2) < dur)
					bool b = outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
						outputs[i].directionHasChanged();
					if (b || (t - tlastRedirectUp2) < dur) {
						if (b) tlastRedirectUp2 = t;
						//if (isRedirectedTo(i) > 0) tlastRedirectUp2 = t;
						//tlastRedirect = t;
						//tlastRedirectDown = t;

						c.set(ofColor(_c2up, _a2));
						l = lmax;
					}
					else
					{
						c.set(ofColor(_c, _a));
						l = lmin;
					}
				}
				break;

				case 4: // down
				{
					////if ((outputs[i].directionHasChanged() && !bDirectionLastUp) || (t - tlastRedirectDown < dur))
					//if ((isRedirectedTo(i) < 0) || (t - tlastRedirectDown2) < dur)
					bool b = outputs[i].getDirectionTimeDiff() > smoothChannels[i]->timeRedirection &&
						outputs[i].directionHasChanged();
					if (b || (t - tlastRedirectDown2) < dur) {
						if (b) tlastRedirectDown2 = t;
						//if (isRedirectedTo(i) < 0) tlastRedirectDown2 = t;
						//tlastRedirect = t;
						//tlastRedirectUp = t;

						c.set(ofColor(_c2down, _a2));
						l = lmax;
					}
					else
					{
						c.set(ofColor(_c, _a));
						l = lmin;
					}
				}
				break;

				}
			}

			//--

			// Draw threshold line

			float y_Th;

			// Bonk detector don't uses threshold. 
			// will follow the curve "fast jumps"!
			if (smoothChannels[i]->bangDetectorIndex == 1 ||
				smoothChannels[i]->bangDetectorIndex == 2 ||
				smoothChannels[i]->bangDetectorIndex == 3 ||
				smoothChannels[i]->bangDetectorIndex == 4
				)
			{
				y_Th = y + (1 - outputs[i].getValue()) * h;
			}
			else
			{
				y_Th = y + (1 - smoothChannels[i]->threshold) * h;
			}

			//--

			ofSetLineWidth(l);
			ofSetColor(c);

			ofLine(x, y_Th, x + ww, y_Th);
		}

		//--

		// Vertical line on the left border 
		// Mark selected channel 
		if (i == index && !bSolo)
		{
			int l = 2;
			int lh = 2;
			int a;
			ofSetLineWidth(l);
			if (ui.bKeys) a = ofMap(ofxSurfingHelpers::Bounce(), 0, 1, 64, 150);
			else a = 150;
			ofSetColor(colorPlots, a);

			ofLine(x + lh, y, x + lh, y + h);
		}

		// Baseline bottom horizontal
		ofSetColor(colorBaseLine);
		ofSetLineWidth(1);
		ofLine(x, y + h, x + ww, y + h);

		if (!bSolo) y += h;
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::keyPressed(int key)
{
	if (!ui.bKeys) return;

	if (0) {}

	else if (key == 'g') bGui_Main = !bGui_Main;

	else if (key == OF_KEY_RETURN) bPlay = !bPlay;

	else if (key == ' ') doRandomize();

	else if (key == 's') bSolo = !bSolo;

	else if (key == OF_KEY_UP)
	{
		index--;
		index = ofClamp(index, index.getMin(), index.getMax());
	}
	else if (key == OF_KEY_DOWN)
	{
		index++;
		index = ofClamp(index, index.getMin(), index.getMax());
	}

	// threshold
	else if (key == '-')
	{
		int i = index;
		smoothChannels[i]->threshold =
			smoothChannels[i]->threshold.get() - 0.05f;
		smoothChannels[i]->threshold =
			ofClamp(smoothChannels[i]->threshold,
				smoothChannels[i]->threshold.getMin(), smoothChannels[i]->threshold.getMax());
	}
	else if (key == '+')
	{
		int i = index;
		smoothChannels[i]->threshold =
			smoothChannels[i]->threshold.get() + 0.05f;
		smoothChannels[i]->threshold =
			ofClamp(smoothChannels[i]->threshold,
				smoothChannels[i]->threshold.getMin(), smoothChannels[i]->threshold.getMax());
	}

	// types
	else if (key == OF_KEY_TAB)
	{
		nextTypeSmooth(index.get());
	}
	else if (key == OF_KEY_LEFT_SHIFT)
	{
		nextTypeMean(index.get());
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setupParams() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	string name = "SMOOTH SURF";

	//--

	// params
	params.setName(name);
	params.add(index.set("index", 0, 0, 0));
	params.add(bSolo.set("SOLO", false));

	params.add(bGui_Main);
	params.add(bGui_Inputs.set("INPUTS", true));
	params.add(bGui_Outputs.set("OUTPUTS", true));
	params.add(bGui_Extra);

	params.add(bGui_Plots.set("PLOTS", true));
	params.add(bGui_PlotsLink.set("Link", false));
	params.add(bGui_PlotFullScreen.set("Full Screen", false));
	params.add(bGui_PlotIn.set("Plot In", true));
	params.add(bGui_PlotOut.set("Plot Out", true));

	params.add(bGenerators.set("Generators", false));
	params.add(bPlay.set("Play", false));
	params.add(playSpeed.set("Speed", 0.5, 0, 1));
	params.add(alphaPlotInput.set("AlphaIn", 0.25, 0, 1));

	params.add(bEnableSmoothGlobal.set("ENABLE GLOBAL", true));

	//TODO: link
	boxPlots.bGui.makeReferenceTo(bGui_Plots);

	//--

	//params.add(ampInput.set("Amp", 0, -1, 1));
	//params.add(threshold.set("Threshold", 0.5, 0.0, 1));
	//params.add(smoothPower.set("Smooth Power", 0.2, 0.0, 1));
	//params.add(typeSmooth.set("Type Smooth", 0, 0, 2));
	//params.add(typeMean.set("Type Mean", 0, 0, 2));
	//params.add(timeRedirection.set("TimeDir", 0.5, 0.0, 1));
	//params.add(slideMin.set("SlideIn", 0.2, 0.0, 1));
	//params.add(slideMax.set("SlideOut", 0.2, 0.0, 1));
	//params.add(onsetGrow.set("Grow", 0.1f, 0.0, 1));
	//params.add(onsetDecay.set("Decay", 0.1, 0.0, 1));
	//params.add(bClamp.set("Clamp", false));//TODO:
	//params.add(bNormalized.set("Normalized", false));//TODO:
	//params.add(minInput.set("minIn", 0, 0, 1));//TODO:
	//params.add(maxInput.set("maxIn", 1, 0, 1));
	//params.add(minOutput.set("minOut", 0, 0, 1));
	//params.add(maxOutput.set("maxOut", 1, 0, 1));

	//params.add(typeMean_Str.set(" ", ""));
	//params.add(typeSmooth_Str.set(" ", ""));

	params.add(bReset.set("ResetAll"));

	//--

	//params.add(input.set("INPUT", 0, _inputMinRange, _inputMaxRange));
	//params.add(output.set("OUTPUT", 0, _outMinRange, _outMaxRange));

	ui.bGui_GameMode.setName("SMOOTH");

	//why required?
	params.add(ui.bHelp);
	params.add(ui.bMinimize);
	params.add(ui.bKeys);
	params.add(ui.bGui_GameMode);

	//--

	typeSmoothLabels.clear();
	typeSmoothLabels.push_back("None");
	typeSmoothLabels.push_back("Accumulator");
	typeSmoothLabels.push_back("Slide");

	typeMeanLabels.clear();
	typeMeanLabels.push_back("Arith");
	typeMeanLabels.push_back("Geom");
	typeMeanLabels.push_back("Harm");

	//--

	// Exclude
	bReset.setSerializable(false);
	bPlay.setSerializable(false);
	bGenerators.setSerializable(false);

	ofAddListener(params.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Params); // setup()

	//--

	buildHelp();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::exit() {
	ofRemoveListener(params.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Params); // exit()

	ofxSurfingHelpers::CheckFolder(path_Global);
	ofxSurfingHelpers::saveGroup(params, path_Settings);

	//// Reset All
	//for (int i = 0; i < smoothChannels.size(); i++)
	//	smoothChannels[i]->exit();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::doReset() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	// Reset All
	for (int i = 0; i < smoothChannels.size(); i++)
		smoothChannels[i]->doReset();

	//--

	playSpeed = 0.5;
}

//--------------------------------------------------------------
void ofxSurfingSmooth::Changed_Params(ofAbstractParameter& e)
{
	if (bDISABLE_CALLBACKS) return;

	string name = e.getName();

	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__) << " : " << name << " : " << e;

	//if (name != input.getName() && name != output.getName() && name != "")
	//{
	//	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__) << " : " << name << " : " << e;
	//}

	//--

	if (0) {}

	else if (name == ui.bKeys.getName())
	{
		buildHelp();

		return;
	}

	else if (name == bGui_PlotIn.getName())
	{
		refreshPlots();

		return;
	}

	else if (name == bGui_PlotOut.getName())
	{
		refreshPlots();

		return;
	}

	else if (name == alphaPlotInput.getName())
	{
#ifdef COLORS_MONCHROME
		for (int i = 0; i < amountChannels; i++)
		{
			int ii = 2 * i;
			int a1 = ofMap(alphaPlotInput, 0, 1, 0, 255, true);//input
			colors[ii] = ofColor(colorPlots, a1);//in
			plots[ii]->setColor(colors[ii]);
		}
#endif
		return;
	}

	//--

	else if (name == bGui_Main.getName())
	{
		/*
		// workflow
		//fails
		if (!bGui_Main)
		{
			bGui_Extra = false;
		}
		*/

		return;
	}

	//--

	//// Enable all
	//else if (name == bEnableSmoothGlobal.getName())
	//{
	//	return;
	//}

	//--

	// Reset all
	else if (name == bReset.getName())
	{
		doReset();

		return;
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setup_ImGui()
{
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	ui.setName("SmoothSurf");
	ui.setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	ui.setup();

	ui.addWindowSpecial(bGui_Main);
	ui.addWindowSpecial(ui.bGui_GameMode);
	ui.addWindowSpecial(bGui_Extra);
	ui.addWindowSpecial(bGui_Inputs);
	ui.addWindowSpecial(bGui_Outputs);

	ui.startup();

	//--

	// custom
	ui.setHelpInfoApp(helpInfo);
	ui.bHelpInternal = false;
	bGui_GameMode.makeReferenceTo(ui.bGui_GameMode);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGuiExtra()
{
	if (bGui_Extra)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;

		if (ui.BeginWindowSpecial(bGui_Extra))
		{
			//--

			if (!ui.bMinimize)
			{
				if (ui.BeginTree("PLOTS", false))
				{
					ui.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
					if (bGui_Plots)
					{
						ui.Indent();
						ui.Add(bGui_PlotIn, OFX_IM_TOGGLE_ROUNDED);
						ui.Add(bGui_PlotOut, OFX_IM_TOGGLE_ROUNDED);
						ui.Indent();
						ui.Add(bGui_PlotFullScreen, OFX_IM_TOGGLE_ROUNDED_MINI);
						//ui.Add(boxPlots.bEdit, OFX_IM_TOGGLE_ROUNDED_MINI);
						//ui.Add(bGui_PlotsLink, OFX_IM_TOGGLE_ROUNDED_MINI);
						ui.Unindent();
						ui.Unindent();
					}

					ui.EndTree();
				}

				ui.AddSpacingSeparated();
			}

			//--
			// 
			// In / Out

			if (!ui.bMinimize)
			{
				if (ui.BeginTree("IN / OUT"))
				{
					ui.Add(bGui_Inputs, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
					ui.Add(bGui_Outputs, OFX_IM_TOGGLE_ROUNDED_MEDIUM);

					ui.EndTree();
				}
				ui.AddSpacingSeparated();
			}

			//--

			// Plots

			if (!bGui_GameMode)
				if (!ui.bMinimize)
				{
					if (ui.BeginTree("PLOTS"))
					{
						ui.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
						if (bGui_Plots)
						{
							ui.Indent();
							ui.Add(bGui_PlotIn, OFX_IM_TOGGLE_ROUNDED);
							ui.Add(bGui_PlotOut, OFX_IM_TOGGLE_ROUNDED);
							ui.Indent();
							ui.Add(bGui_PlotFullScreen, OFX_IM_TOGGLE_ROUNDED_MINI);
							ui.Add(boxPlots.bEdit, OFX_IM_TOGGLE_ROUNDED_MINI);
							if (ui.Add(bGui_PlotsLink, OFX_IM_TOGGLE_ROUNDED_MINI)) {
							};
							ui.Unindent();
							ui.Unindent();
						}

						ui.EndTree();
					}
					ui.AddSpacingSeparated();
				}

			//----

			// Enable Toggles

			if (!ui.bMinimize)
			{
				if (ui.BeginTree("ENABLERS"))
				{
					if (ui.AddButton("NONE", OFX_IM_BUTTON_SMALL, 2))
					{
						doDisableAll();
					}
					ui.SameLine();
					if (ui.AddButton("ALL", OFX_IM_BUTTON_SMALL, 2))
					{
						doEnableAll();
					}

					ui.AddSpacing();

					for (int i = 0; i < params_EditorEnablers.size(); i++)
					{
						auto& p = params_EditorEnablers[i]; // ofAbstractParameter
						auto type = p.type();
						bool isBool = type == typeid(ofParameter<bool>).name();
						string name = p.getName();

						if (isBool) // just in case... 
						{
							ofParameter<bool> pb = p.cast<bool>();
							ui.Add(pb, OFX_IM_CHECKBOX);
						}
					}

					ui.EndTree();
				}

				ui.AddSpacingSeparated();
			}

			if (!ui.bMinimize)
			{
				if (ui.BeginTree("TESTER"))
				{
					ui.Add(bGenerators, OFX_IM_TOGGLE);

					if (ui.AddButton("Randomizer", OFX_IM_BUTTON))
					{
						doRandomize();
					}

					// Blink by timer
					{
						bool b = bPlay;
						float a;
						if (b) a = 1 - tn;
						if (b) ImGui::PushStyleColor(ImGuiCol_Border,
							(ImVec4)ImColor::HSV(0.5f, 0.0f, 1.0f, 0.5 * a));
						ui.Add(bPlay, OFX_IM_TOGGLE);
						if (b) ImGui::PopStyleColor();
					}

					if (bPlay)
					{
						ui.Add(playSpeed);
					}

					ui.EndTree();
				}
			}

			//--

			if (!ui.bMinimize)
			{
				ui.AddSpacingSeparated();

				if (ImGui::CollapsingHeader("Advanced"))
				{
					ui.refreshLayout();

					ui.Add(ui.bKeys, OFX_IM_TOGGLE_BUTTON_ROUNDED_MEDIUM);
					ui.Add(ui.bHelp, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
					ui.Add(boxPlots.bEdit);
				}
			}

			//ui.AddSpacingSeparated();

			//ui.AddSpacing();

			/*
			ui.Add(ui.bKeys, OFX_IM_TOGGLE_ROUNDED);
			ui.Add(ui.bHelp, OFX_IM_TOGGLE_ROUNDED);
			//ui.Add(bEnableSmoothGlobal, OFX_IM_TOGGLE_ROUNDED_MINI);
			ui.Add(bGui_Extra, OFX_IM_TOGGLE_ROUNDED_MINI);
			ui.Add(bGenerators, OFX_IM_TOGGLE_SMALL);
			*/

			ui.EndWindowSpecial();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGuiGameMode()
{
	if (ui.bGui_GameMode) ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

	if (ui.BeginWindowSpecial(ui.bGui_GameMode))
	{
		int i = index.get();

		//--

		ui.AddLabelHuge("SMOOTH \n+DETECT");
		//ui.AddLabelHuge(ui.bGui_GameMode.getName());
		ui.AddSpacingBigSeparated();

		////TODO:
		//ImGui::Columns(2, "arrows", true);
		//ImVec2 sz(20, 40);
		//ui.AddButton("»", sz);
		//ui.SameLine();
		//ImGui::NextColumn();
		//ui.AddButton("»", sz);
		//ImGui::Columns(1);

		ui.Add(ui.bMinimize, OFX_IM_TOGGLE_ROUNDED);

		if (!ui.bMinimize) {
			ui.Add(bGui_Main, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
		}

		//--

		ui.AddSpacingSeparated();

		// Presets
		smoothChannels[i]->surfingPresets.drawImGui(false, false, true);

		//--

		ui.AddSpacingSeparated();
		//ui.AddSeparated();

		//--

		if (ui.BeginTree("CHANNEL", true, true))
		{
			//if (!ui.bMinimize)
			//{
			//	ui.AddLabelBig("Signal");
			//	//ui.AddLabelBig("CHANNEL");
			//}

			// Channel name
			if (i > editorEnablers.size() - 1)
			{
				ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Index out of range! #" << i;
			}
			else
			{
				//ImGui::Columns(2, "t1", false);
				string n;
				/*
				if (!ui.bMinimize) {
					n = "#" + ofToString(i);
					ui.AddLabel(n, false, true);
					//ImGui::NextColumn();
					ui.SameLine();
				}
				*/

				n = editorEnablers[i].getName();
				ui.AddLabelBig(n, false, true);

				//ImGui::Columns(1);
			}

			// Selector
			if (!ui.bMinimize) {
				bool bResponsive = true;
				int amountBtRow = 3;
				const bool bDrawBorder = true;
				float __h = 1.5f * ofxImGuiSurfing::getWidgetsHeightUnit();
				string toolTip = "Select Signal channel";
				ofxImGuiSurfing::AddMatrixClicker(index, bResponsive, amountBtRow, bDrawBorder, __h, toolTip);
			}
			else
			{
				ui.AddComboArrows(index);
			}

			// Enable, Solo, Amp
			if (!ui.bMinimize)
			{
				//ui.AddSpacing();
				ui.Add(smoothChannels[i]->bEnableSmooth, OFX_IM_TOGGLE_SMALL, 2);
				//ui.AddTooltip("Activate Smoother or bypass");
				ui.SameLine();
				ui.Add(bSolo, OFX_IM_TOGGLE_SMALL_BORDER_BLINK, 2);
			}

			SurfingGuiTypes st = (!ui.bMinimize) ? OFX_IM_HSLIDER_MINI : OFX_IM_HSLIDER_MINI_NO_NAME;
			ui.Add(smoothChannels[i]->ampInput, st);
			ui.AddTooltip(ofToString(smoothChannels[i]->ampInput.get(), 2));

			ui.EndTree();
		}

		ui.AddSpacingSeparated();

		//--

		if (smoothChannels[i]->bEnableSmooth) {

			if (ui.BeginTree("SMOOTH", true, true))
			{
				ui.AddSpacing();

				//--

				ui.AddCombo(smoothChannels[i]->typeSmooth, typeSmoothLabels);
				ui.AddTooltip("Type Smooth");

				if (smoothChannels[i]->typeSmooth == ofxDataStream::SMOOTHING_ACCUM)
				{
					SurfingGuiTypes st = (!ui.bMinimize) ?
						OFX_IM_HSLIDER_SMALL_NO_NUMBER : OFX_IM_HSLIDER_MINI_NO_LABELS;

					ui.Add(smoothChannels[i]->smoothPower, st);
					ui.AddTooltip(ofToString(smoothChannels[i]->smoothPower.get(), 2));
				}

				if (smoothChannels[i]->typeSmooth == ofxDataStream::SMOOTHING_SLIDE)
				{
					SurfingGuiTypes st = (!ui.bMinimize) ?
						OFX_IM_HSLIDER_SMALL : OFX_IM_HSLIDER_MINI_NO_LABELS;

					ui.Add(smoothChannels[i]->slideMin, st);
					ui.AddTooltip(ofToString(smoothChannels[i]->slideMin.get(), 2));
					ui.Add(smoothChannels[i]->slideMax, st);
					ui.AddTooltip(ofToString(smoothChannels[i]->slideMax.get(), 2));
				}

				ui.AddCombo(smoothChannels[i]->typeMean, typeMeanLabels);
				ui.AddTooltip("Type Mean");

				ui.EndTree();
			}

			ui.AddSpacingSeparated();

			//--

			// Bang Detector / Trigger selector

			if (ui.BeginTree("DETECTOR", true, true))
			{
				ui.AddSpacing();

				bool bx = (!ui.bMinimize);
				if (bx) ui.AddLabel("Mode");

				ui.AddComboButtonDualLefted(smoothChannels[i]->bangDetectorIndex, smoothChannels[i]->bangDetectors);

				//--

				SurfingGuiTypes st = bx ? OFX_IM_HSLIDER_SMALL : OFX_IM_HSLIDER_MINI_NO_LABELS;

				//if (!ui.bMinimize)
				{
					if (smoothChannels[i]->bangDetectorIndex == 0) // state
					{
						ui.Add(smoothChannels[i]->threshold, st);
						//ui.AddTooltip(ofToString(smoothChannels[i]->threshold.get(), 2));
					}

					else if (smoothChannels[i]->bangDetectorIndex == 1) // bonk
					{
						ui.Add(smoothChannels[i]->onsetGrow, st);
						if (!bx) ui.AddTooltip(ofToString(smoothChannels[i]->onsetGrow.get(), 2));
						ui.Add(smoothChannels[i]->onsetDecay, st);
						if (!bx) ui.AddTooltip(ofToString(smoothChannels[i]->onsetDecay.get(), 2));
					}

					else if (smoothChannels[i]->bangDetectorIndex == 2 || // direction
						smoothChannels[i]->bangDetectorIndex == 3 || // up
						smoothChannels[i]->bangDetectorIndex == 4) // down
					{
						ui.Add(smoothChannels[i]->timeRedirection, st);
						if (!bx) ui.AddTooltip(ofToString(smoothChannels[i]->timeRedirection.get(), 2));
					}
				}

				ui.AddSpacing();

				//--

				// Circle Widget

				/*
				if (bx) {
					string n = editorEnablers[i].getName();
					ui.AddLabel(n, false, true);
					ui.AddLabelBig("BANG!", true, true);
				}
				ui.AddSpacing();
				*/

				circleBeat_Widget.draw();

				//--

#ifndef DISABLE_GATE
				ui.AddSpacing();

				ui.Add(smoothChannels[i]->bGateModeEnable, OFX_IM_TOGGLE_SMALL);
				if (smoothChannels[i]->bGateModeEnable)
				{
					// Timer progress
					float v = ofMap(gates[i].timerGate, 0, gates[i].tDurationGate, 0, 1, true);
					if (!gates[i].bGateStateClosed) v = 1;//if open
					ofxImGuiSurfing::ProgressBar2(v, 1.f, true);

					if (!ui.bMinimize)
					{
						ofxImGuiSurfing::AddToggleNamed(smoothChannels[i]->bGateSlow, "Slow", "Fast");
						ui.Add(smoothChannels[i]->bpmDiv, OFX_IM_STEPPER);
					}
				}
#endif
				//--

				ui.EndTree();
			}
		}

		//--

		//if (!ui.bMinimize) ui.AddSpacingSeparated();
		//if (ui.bMinimize)
		//{
		//	ui.Add(ui.bKeys, OFX_IM_TOGGLE_ROUNDED);
		//}

		//--

		ui.AddSpacingSeparated();

		if (!ui.bMinimize)
		{
			if (ui.Add(smoothChannels[i]->bReset, OFX_IM_BUTTON_SMALL, 2)) {

			};
			ui.SameLine();
			if (ui.Add(bReset, OFX_IM_BUTTON_SMALL, 2)) {
				doReset();
			};

			ui.Add(ui.bKeys, OFX_IM_TOGGLE_BUTTON_ROUNDED_MINI);
			//ui.Add(ui.bHelp, OFX_IM_TOGGLE_BUTTON_ROUNDED_MINI);
		}
		else {
			ui.Add(smoothChannels[i]->bReset, OFX_IM_BUTTON_SMALL);
		}

		ui.EndWindowSpecial();
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGuiMain()
{
	if (bGui_Main)
	{
		int i = index.get();

		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_SMALL;

		if (ui.BeginWindowSpecial(bGui_Main))
		{
			ui.Add(ui.bMinimize, OFX_IM_TOGGLE_ROUNDED);
			ui.AddSpacingSeparated();

			//ui.Add(ui.bGui_GameMode, OFX_IM_TOGGLE_ROUNDED_BIG);
			//ui.AddSpacingSeparated();

			//--

			// Enable Global
			ui.Add(bEnableSmoothGlobal, OFX_IM_TOGGLE_BIG_BORDER_BLINK);

			/*
			//TODO: WIP
			if (!ui.bMinimize)
			{
				ui.AddSpacingSeparated();

				ui.Add(smoothChannels[i]->bClamp, OFX_IM_TOGGLE_SMALL, 2, true);
				ui.Add(smoothChannels[i]->bNormalized, OFX_IM_TOGGLE_SMALL, 2);

				if (smoothChannels[i]->bClamp)
				{
					ui.AddSpacing();
					ui.Add(smoothChannels[i]->minInput);
					ui.Add(smoothChannels[i]->maxInput);
					ui.AddSpacing();
					ui.Add(smoothChannels[i]->minOutput);
					ui.Add(smoothChannels[i]->maxOutput);
					//ui.AddSpacing();
				}
			}
			*/

			//ui.AddSpacingSeparated();

			// Monitor
			//if (!bGui_GameMode)
			{
				if (bEnableSmoothGlobal)
				{
					ui.AddSpacingSeparated();

					if (ImGui::CollapsingHeader("MONITOR"))
					{
						ui.refreshLayout();

						if (index < editorEnablers.size())
						{
							ui.AddLabelBig(editorEnablers[index].getName(), false);
						}

						if (!ui.bMinimize)
						{
							if (index.getMax() < 5) {
								ofxImGuiSurfing::AddMatrixClicker(index);
							}
							else {
								if (ui.Add(index, OFX_IM_STEPPER))
								{
									index = ofClamp(index, index.getMin(), index.getMax());
								}
							}
							ui.AddSpacingSeparated();
							ui.Add(smoothChannels[i]->bEnableSmooth, OFX_IM_TOGGLE);
							ui.Add(bSolo, OFX_IM_TOGGLE);

							//ui.Add(input, OFX_IM_HSLIDER_MINI);
							//ui.Add(output, OFX_IM_HSLIDER_MINI);
						}
						else
						{
							if (index.getMax() < 5)
							{
								ofxImGuiSurfing::AddMatrixClicker(index);
							}
							else {
								if (ui.Add(index, OFX_IM_STEPPER))
								{
									index = ofClamp(index, index.getMin(), index.getMax());
								}
							}

							ui.Add(smoothChannels[i]->bEnableSmooth, OFX_IM_TOGGLE, 2, true);
							ui.Add(bSolo, OFX_IM_TOGGLE, 2);

							//ui.AddSpacingSeparated();
							//ui.Add(input, OFX_IM_HSLIDER_MINI_NO_LABELS);
							//ui.Add(output, OFX_IM_HSLIDER_MINI_NO_LABELS);
						}
					}

					ui.refreshLayout();
				}
			}

			//--

			if (!ui.bMinimize)
			{
				ui.AddSpacingSeparated();
				ui.Add(bGui_Extra, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
			}
		}

		ui.EndWindowSpecial();
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGui()
{
	if (!bGui_Global) return;

	ui.Begin();
	{
		//TODO:
		if (bGui_PlotsLink)
		{
			// Get last window
			//int pad = ui.getWindowSpecialPadSize();
			int padmin = 8;
			int pad = MAX(ui.getWindowSpecialPadSize(), padmin);//add a bit more space
			glm::vec2 p = ui.getWindowSpecialLastTopRight();
			p = p + glm::vec2(pad, 0);

			// Current window
			//ofRectangle r = ui.getWindowShape();
			//int pad = MAX(ui.getWindowSpecialPadSize(), 2);

			//// A. scale
			//float xw = r.getWidth() + pad;
			//r.translateX(xw);
			//r.scaleWidth(2);
			//float h = MIN(r.getHeight(), 700);
			//r.setHeight(h);
			//boxPlots.setShape(r);

			//// B position
			//glm::vec2 p = r.getTopRight() + glm::vec2(pad, 0);

			boxPlots.setPosition(p.x, p.y);
		}

		//----

		if (ui.bGui_GameMode) draw_ImGuiGameMode();

		//--

		if (bGui_Main) draw_ImGuiMain();

		//--

		if (bGui_Inputs)
		{
			if (ui.BeginWindowSpecial(bGui_Inputs))
			{
				ui.AddGroup(mParamsGroup);
				ui.EndWindowSpecial();
			}
		}

		//--

		if (bGui_Outputs)
		{
			if (ui.BeginWindowSpecial(bGui_Outputs))
			{
				ui.AddGroup(mParamsGroup_Smoothed);
				ui.EndWindowSpecial();
			}
		}

		//--

		if (bGui_Extra)
		{
			draw_ImGuiExtra();
		}
	}
	ui.End();
}

//----

//--------------------------------------------------------------
void ofxSurfingSmooth::addParam(ofAbstractParameter& aparam) {

	string _name = aparam.getName();
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__) << " ofAbstractParameter | " << _name;

	//--

	//TODO:
	// nested groups doing recursive
	// https://forum.openframeworks.cc/t/ofxparametercollection-manage-multiple-ofparameters/34888/3

	auto type = aparam.type();

	bool isGroup = (type == typeid(ofParameterGroup).name());
	bool isFloat = (type == typeid(ofParameter<float>).name());
	bool isInt = (type == typeid(ofParameter<int>).name());
	bool isBool = (type == typeid(ofParameter<bool>).name());

	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__) << " " << _name << "  [ " << type << " ]";

	if (isGroup)
	{
		auto& g = aparam.castGroup();

		////TODO:
		// nested groups

		for (int i = 0; i < g.size(); i++)
		{
			addParam(g.get(i));
		}
	}

	// Add / Queue each param
	// exclude groups to remove from plots
	if (!isGroup) mParamsGroup.add(aparam);

	//--

	// Create a copied Group
	// who will be the container 
	// for the output params,
	// or what, once smoothed, 
	// we will use target to be use params.
	// We copy each param and queue it there into this group!

	// Filter which param types to handle 
	// or to discard into the engine:

	if (isFloat)
	{
		ofParameter<float> p = aparam.cast<float>();
		ofParameter<float> _p{ _name + suffix, p.get(), p.getMin(), p.getMax() };
		mParamsGroup_Smoothed.add(_p);

		//-

		ofParameter<bool> _b{ _name, true };
		editorEnablers.push_back(_b);
		params_EditorEnablers.add(_b);
	}

	else if (isInt)
	{
		ofParameter<int> p = aparam.cast<int>();
		ofParameter<int> _p{ _name + suffix, p.get(), p.getMin(), p.getMax() };
		mParamsGroup_Smoothed.add(_p);

		//-

		ofParameter<bool> _b{ _name, true };
		editorEnablers.push_back(_b);
		params_EditorEnablers.add(_b);
	}

	else if (isBool)
	{
		ofParameter<bool> p = aparam.cast<bool>();
		ofParameter<bool> _p{ _name + suffix, p.get() };
		mParamsGroup_Smoothed.add(_p);

		//-

		ofParameter<bool> _b{ _name, true };
		editorEnablers.push_back(_b);
		params_EditorEnablers.add(_b);
	}

	//TODO:
	else
	{
		return;//skip other types
	}

	//--

	// The Main Lambda Callback!

	// Create the controls
	// for each channel using our class

	int _i = params_EditorEnablers.size() - 1;
	setupCallback(_i);

	// Load file settings
	//smoothChannels[i]->startup();

	//--

	// Store the enabler for each param / channel
	params.add(params_EditorEnablers);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setup(ofParameterGroup& aparams) {

	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__) << " " << aparams.getName();

	setup();

	//--

	string n = aparams.getName();
	mParamsGroup.setName(n);//name

	//TODO:
	// copied group
	mParamsGroup_Smoothed.setName(n + "_SMOOTH");
	//mParamsGroup_Smoothed.setName(n + suffix);

	for (int i = 0; i < aparams.size(); i++)
	{
		addParam(aparams.get(i));
	}

	//--

	// Plots

	// already added all params content
	// build the smoothers
	// build the plots

	setupPlots();

	//TODO: should count only if added successfully, 
	// not filtered by supported styles or not
	// amountChannels will be counted here.

	outputs.resize(amountChannels);
	inputs.resize(amountChannels);

	// default init
	for (int i = 0; i < amountChannels; i++)
	{
		// prepare stream processor
		outputs[i].initAccum(MAX_ACCUM);
		outputs[i].directionChangeCalculated = true;
		outputs[i].setBonk(smoothChannels[i]->onsetGrow, smoothChannels[i]->onsetDecay);

		// prepare gate engine
		GateStruct gate;
		gate.bGateStateClosed.setSerializable(false);
		gates.push_back(gate);
	}

	//--

	////TODO:
	//mParamsGroup_Smoothed.setName(aparams.getName() + "_COPY");//name
	//mParamsGroup_Smoothed = mParamsGroup;//this kind of copy links param per param. but we want to clone the "structure" only

	//--

	startup();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setupCallback(int _i)
{
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__) << " " << _i;

	smoothChannels.push_back(make_unique<SmoothChannel>());
	smoothChannels[_i]->setup("Ch_" + ofToString(_i));
	//that will define the path for file settings names too!

	smoothChannels[_i]->index.setMin(index.getMin());
	smoothChannels[_i]->index.setMax(index.getMax());
	smoothChannels[_i]->index.set(_i);
	//smoothChannels[_i]->index = _i;
	//TODO: workaround, mark index

	//TODO:
	// how to pass i from here?
	// it's possible?
	//, const int& i
	//iIndex = i;

	// Create the lambda callback for each channel
	//newListener([&, _i](
	listeners.push(smoothChannels[_i]->params.parameterChangedE().newListener([&]
	(const ofAbstractParameter& e)
		{
			string name = e.getName();

	ofLogNotice("ofxSurfingSmooth") << "--------------------------------------------------------------";
	ofLogNotice("ofxSurfingSmooth") << "Lambda | " << name << ": " << e;

	////ofLogNotice("ofxSurfingSmooth") << "Ch " << iIndex;
	//auto &g = e.castGroup();
	//int i = g.getInt("index");
	//ofLogNotice("ofxSurfingSmooth") << "index : " << smoothChannels[i]->index;

	//TODO:
	// using that workaround:
	// each class object knows which index is
	// for this parent scope class!
	int i = params.getInt("index");
	//int i = _i;

	ofLogNotice("ofxSurfingSmooth") << "CH: " << i;

	if (i > amountChannels)
	{
		ofLogError("ofxSurfingSmooth") << "Out of range: " << i << ". Skip this index!";
		return;
	}

	//--

	if (0) {}

	//--

	else if (name == smoothChannels[i]->typeSmooth.getName())
	{
		switch (smoothChannels[i]->typeSmooth)
		{

		case ofxDataStream::SMOOTHING_NONE:
		{
			//smoothChannels[i]->bDoReFresh = true;
			//smoothChannels[i]->doRefresh();

			return;
		}
		break;

		case ofxDataStream::SMOOTHING_ACCUM:
		{
			float v = ofMap(smoothChannels[i]->smoothPower, 0, 1, 1, MAX_ACCUM);
			{
				outputs[i].initAccum(v);
			}

			//smoothChannels[i]->bDoReFresh = true;
			//smoothChannels[i]->doRefresh();

			return;
		}
		break;

		case ofxDataStream::SMOOTHING_SLIDE:
		{
			float _slmin = ofMap(
				smoothChannels[i]->slideMin, 0, 1,
				MIN_SLIDE, MAX_SLIDE, true);

			float _slmax = ofMap(
				smoothChannels[i]->slideMax, 0, 1,
				MIN_SLIDE, MAX_SLIDE, true);

			outputs[i].initSlide(_slmin, _slmax);

			//smoothChannels[i]->bDoReFresh = true;
			//smoothChannels[i]->doRefresh();

			return;
		}
		break;

		}

		return;
	}

	//--

	else if (name == smoothChannels[i]->typeMean.getName())
	{
		switch (smoothChannels[i]->typeMean)
		{
		case ofxDataStream::MEAN_ARITH:
		{
			outputs[i].setMeanType(ofxDataStream::MEAN_ARITH);

			//smoothChannels[i]->doRefreshMean();
			//smoothChannels[i]->bDoReFresh = true;
			//smoothChannels[i]->doRefresh();

			return;
		}
		break;

		case ofxDataStream::MEAN_GEOM:
		{
			outputs[i].setMeanType(ofxDataStream::MEAN_GEOM);

			//smoothChannels[i]->doRefreshMean();
			//smoothChannels[i]->bDoReFresh = true;
			//smoothChannels[i]->doRefresh();

			return;
		}
		break;

		case ofxDataStream::MEAN_HARM:
		{
			outputs[i].setMeanType(ofxDataStream::MEAN_HARM);

			//smoothChannels[i]->doRefreshMean();
			//smoothChannels[i]->bDoReFresh = true;
			//smoothChannels[i]->doRefresh();

			return;
		}
		break;
		}

		return;
	}

	//--

	else if (name == smoothChannels[i]->threshold.getName())
	{
		outputs[i].setThresh(smoothChannels[i]->threshold);

		//smoothChannels[i]->bDoReFresh = true;
		//smoothChannels[i]->doRefresh();

		return;
	}

	//--

	else if (name == smoothChannels[i]->smoothPower.getName())
	{
		float v = ofMap(smoothChannels[i]->smoothPower, 0, 1, 1, MAX_ACCUM);
		outputs[i].initAccum(v);

		//smoothChannels[i]->doRefreshSmooth();
		//smoothChannels[i]->bDoReFresh = true;
		//smoothChannels[i]->doRefresh();

		return;
	}

	//--

	else if (name == smoothChannels[i]->slideMin.getName() ||
		name == smoothChannels[i]->slideMax.getName())
	{
		float _slmin = ofMap(smoothChannels[i]->slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
		float _slmax = ofMap(smoothChannels[i]->slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
		outputs[i].initSlide(_slmin, _slmax);

		//smoothChannels[i]->doRefreshSmooth();
		//smoothChannels[i]->bDoReFresh = true;
		//smoothChannels[i]->doRefresh();

		return;
	}

	//--

	//TODO:
	else if (name == smoothChannels[i]->minOutput.getName() || name == smoothChannels[i]->maxOutput.getName())
	{
		outputs[i].setOutputRange(ofVec2f(
			smoothChannels[i]->minOutput,
			smoothChannels[i]->maxOutput));

		//smoothChannels[i]->doRefresh();
		//smoothChannels[i]->bDoReFresh = true;

		return;
	}

	//--

	//TODO:
	//detect "bonks" (onsets):
	//amp.setBonk(0.1, 0.1);  // min growth for onset, min decay
	//set growth/decay:
	//amp.setDecayGrow(true, 0.99); // a frame rate dependent steady decay/growth

	else if (
		name == smoothChannels[i]->onsetGrow.getName() ||
		name == smoothChannels[i]->onsetDecay.getName())
	{
		outputs[i].setBonk(smoothChannels[i]->onsetGrow, smoothChannels[i]->onsetDecay);
		outputs[i].directionChangeCalculated = true;
		//specAmps[i].setDecayGrow(true, 0.99);
		//outputs[i].setBonk(0.1, 0.0);

		//smoothChannels[i]->bDoReFresh = true;
		//smoothChannels[i]->doRefresh();

		return;
	}

	//--

	// Do not requires to update the engine!

	else if (name == smoothChannels[i]->bangDetectorIndex.getName())
	{
		//	circleBeat_Widget.reset();
		//	circleBeat_Widget.setMode(smoothChannels[i]->bangDetectorIndex.get());

		//smoothChannels[i]->bDoReFresh = true;
		//smoothChannels[i]->doRefresh();
		smoothChannels[i]->doRefreshDetector();

		return;
	}

	//--

	//TODO:
	else if (name == smoothChannels[i]->bNormalized.getName())
	{
		//TODO:
		//outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));

		if (smoothChannels[i]->bNormalized)
			outputs[i].setNormalized(
				smoothChannels[i]->bNormalized,
				ofVec2f(0, 1));

		else
			outputs[i].setNormalized(
				smoothChannels[i]->bNormalized,
				ofVec2f(smoothChannels[i]->minOutput, smoothChannels[i]->maxOutput));

		//smoothChannels[i]->bDoReFresh = true;
		//smoothChannels[i]->doRefresh();

		return;
	}

	//else if (name == smoothChannels[i]->ampInput.getName())
	//{
	//	//..
	//	return;
	//}

	//else if (name == minOutput.getName())
	//{
	//	//..
	//	return;
	//}

	//else if (name == minOutput.getName())
	//{
	//	//..
	//	return;
	//}

		}));
}

//--------------------------------------------------------------
void ofxSurfingSmooth::add(ofParameterGroup aparams) {
	for (int i = 0; i < aparams.size(); i++) {
		addParam(aparams.get(i));
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::add(ofParameter<float>& aparam) {
	addParam(aparam);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::add(ofParameter<bool>& aparam) {
	addParam(aparam);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::add(ofParameter<int>& aparam) {
	addParam(aparam);
}

//------------

// API Getters

// to get the smoothed parameters individually and externally
// simplified getters

//--------------------------------------------------------------
float ofxSurfingSmooth::get(ofParameter<float>& e) {
	string name = e.getName();
	auto& p = mParamsGroup_Smoothed.get(name);
	if (p.type() == typeid(ofParameter<float>).name())
	{
		return p.cast<float>().get();
	}
	else
	{
		ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Not expected type: " << name;
		return -1;
	}
}
//--------------------------------------------------------------
int ofxSurfingSmooth::get(ofParameter<int>& e) {
	string name = e.getName();
	auto& p = mParamsGroup_Smoothed.get(name);
	if (p.type() == typeid(ofParameter<int>).name())
	{
		return p.cast<int>().get();
	}
	else
	{
		ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Not expected type: " << name;
		return -1;
	}
}

//--------------------------------------------------------------
ofAbstractParameter& ofxSurfingSmooth::getParamAbstract(ofAbstractParameter& e)
{
	string name = e.getName();
	auto& p = mParamsGroup.get(name);


	//TODO:
	auto i = mParamsGroup.getPosition(name);
	float value = outputs[i].getValue();

	//ofAbstractParameter& aparam = mParamsGroup[i];
	//float value = 0;
	//if (aparam.type() == typeid(ofParameter<int>).name()) {
	//	ofParameter<int> ti = aparam.cast<int>();
	//	value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
	//}
	//else if (aparam.type() == typeid(ofParameter<float>).name()) {
	//	ofParameter<float> ti = aparam.cast<float>();
	//	value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
	//}
	//ofLogNotice("ofxSurfingSmooth")<< (__FUNCTION__) << aparam.getName() << " : " << e;

	return p;
}

int ofxSurfingSmooth::getIndex(ofAbstractParameter& e) /*const*/
{
	string name = e.getName();

	auto& p = mParamsGroup.get(name);
	int i = mParamsGroup.getPosition(name);

	return i;

	//TODO: should verify types? e.g to skip bools?

	//auto& p = mParamsGroup_Smoothed.get(name);
	//auto i = mParamsGroup_Smoothed.getPosition(name);
	//if (p.type() == typeid(ofParameter<float>).name()) {
	//	ofParameter<float> pf = p.cast<float>();
	//	return pf;
	//}
	//else
	//{
	//	ofParameter<float> pf{ "empty", -1 };
	//	ofLogError("ofxSurfingSmooth")<<(__FUNCTION__) << "Not expected type: " << name;
	//	return pf;
	//}
}

//--------------------------------------------------------------
ofAbstractParameter& ofxSurfingSmooth::getParamAbstract(string name)
{
	auto& p = mParamsGroup.get(name);


	//TODO:
	auto i = mParamsGroup.getPosition(name);
	float value = outputs[i].getValue();

	return p;
}

//--------------------------------------------------------------
ofParameter<float>& ofxSurfingSmooth::getParamFloat(ofParameter<float>& p)
{
	string n = p.getName();
	return getParamFloat(n);
}
//--------------------------------------------------------------
ofParameter<int>& ofxSurfingSmooth::getParamInt(ofParameter<int>& p)
{
	string n = p.getName();
	return getParamInt(n);
}

//--------------------------------------------------------------
ofParameter<float>& ofxSurfingSmooth::getParamFloat(string name)
{
	//auto &p = mParamsGroup.get(name);
	//auto i = mParamsGroup.getPosition(name);
	//if (p.type() == typeid(ofParameter<float>).name()) {
	//	ofParameter<float> pf = p.cast<float>();
	//	ofParameter<float> pf_Out = pf;//set min/max
	//	//ofParameter<float> pf_Out{ pf.getName(), 0, pf.getMin(), pf.getMax() };//set min/max
	//	float value = ofMap(outputs[i].getValue(), 0, 1, pf.getMin(), pf.getMax());
	//	pf_Out.set(value);
	//	//pf.set(outputs[i].getValue());
	//	return pf_Out;
	//	//return pf;
	//}
	//else
	//{
	//	ofParameter<float> pf{ "empty", -1 };
	//	return pf;
	//}

	auto& p = mParamsGroup_Smoothed.get(name);
	auto i = mParamsGroup_Smoothed.getPosition(name);
	if (p.type() == typeid(ofParameter<float>).name()) {
		ofParameter<float> pf = p.cast<float>();
		return pf;
	}
	else
	{
		ofParameter<float> pf{ "empty", -1 };
		ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Not expected type: " << name;
		return pf;
	}

	//--

	ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Param: " << name;
	ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "from not expected type!. Would expect exception!";
}

//--------------------------------------------------------------
float ofxSurfingSmooth::getParamFloatValue(ofAbstractParameter& e)
{
	string name = e.getName();

	//auto &p = mParamsGroup.get(name);
	//auto i = mParamsGroup.getPosition(name);
	//if (p.type() == typeid(ofParameter<float>).name()) {
	//	ofParameter<float> pf = p.cast<float>();
	//	return ofMap(outputs[i].getValue(), 0, 1, pf.getMin(), pf.getMax());
	//}
	//else
	//{
	//	return -1;
	//}

	auto& p = mParamsGroup_Smoothed.get(name);
	auto i = mParamsGroup_Smoothed.getPosition(name);
	if (p.type() == typeid(ofParameter<float>).name()) {
		ofParameter<float> pf = p.cast<float>();
		return pf.get();
	}
	else
	{
		ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Not expected type: " << name;
		return -1;
	}
}

//--------------------------------------------------------------
int ofxSurfingSmooth::getParamIntValue(ofAbstractParameter& e)
{
	string name = e.getName();

	//auto &p = mParamsGroup.get(name);
	//auto i = mParamsGroup.getPosition(name);
	//if (p.type() == typeid(ofParameter<int>).name()) {
	//	ofParameter<int> pf = p.cast<int>();
	//	return ofMap(outputs[i].getValue(), 0, 1, pf.getMin(), pf.getMax());
	//}
	//else
	//{
	//	return -1;
	//}

	auto& p = mParamsGroup_Smoothed.get(name);
	auto i = mParamsGroup_Smoothed.getPosition(name);
	if (p.type() == typeid(ofParameter<int>).name()) {
		ofParameter<int> pf = p.cast<int>();
		return pf.get();
	}
	else
	{
		ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Not expected type: " << name;
		return -1;
	}
}

//--------------------------------------------------------------
ofParameter<int>& ofxSurfingSmooth::getParamInt(string name)
{
	//auto &p = mParamsGroup.get(name);
	//auto i = mParamsGroup.getPosition(name);
	//if (p.type() == typeid(ofParameter<int>).name()) {
	//	ofParameter<int> pi = p.cast<int>();
	//	ofParameter<int> pi_Out = pi;//set min/max
	//	pi.set(outputs[i].getValue());
	//	return pi_Out;
	//	//return pi;
	//}
	//else
	//{
	//	ofParameter<int> pi{ "empty", -1 };
	//	return pi;
	//}

	auto& p = mParamsGroup_Smoothed.get(name);
	auto i = mParamsGroup_Smoothed.getPosition(name);
	if (p.type() == typeid(ofParameter<int>).name()) {
		ofParameter<int> pf = p.cast<int>();
		return pf;
	}
	else
	{
		ofParameter<int> pf{ "empty", -1 };
		ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Not expected type: " << name;
		return pf;
	}
}

//-----
//
////TODO: not using..
//// will populate widgets params, to monitor the smoothed outputs, not the raw parameters (inputs)
//// not using the recreated smooth parameters...
////--------------------------------------------------------------
//void ofxSurfingSmooth::addGroupSmooth_ImGuiWidgets(ofParameterGroup &group) {
//	string n = group.getName() + " > Smoothed";
//	if (ImGui::TreeNodeEx(n.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoAutoOpenOnLog))
//	{
//		for (int i = 0; i < group.size(); i++)
//		{
//			auto type = group[i].type();
//			bool isGroup = type == typeid(ofParameterGroup).name();
//			bool isFloat = type == typeid(ofParameter<float>).name();
//			bool isInt = type == typeid(ofParameter<int>).name();
//			bool isBool = type == typeid(ofParameter<bool>).name();
//			string str = group[i].getName();
//
//			if (isFloat)
//			{
//				float v = outputs[i].getValue();
//				float min = group[i].cast<float>().getMin();
//				float max = group[i].cast<float>().getMax();
//				v = ofMap(v, 0, 1, min, max);
//				ImGui::SliderFloat(str.c_str(), &v, min, max);
//			}
//			else if (isInt)
//			{
//				float vf = outputs[i].getValue();
//				int vi;
//				int min = group[i].cast<int>().getMin();
//				int max = group[i].cast<int>().getMax();
//				vi = (int)ofMap(vf, 0, 1, min, max);
//				ImGui::SliderInt(str.c_str(), &vi, min, max);
//			}
//		}
//
//		ImGui::TreePop();
//	}
//}

//--------------------------------------------------------------
void ofxSurfingSmooth::doSetAll(bool b) {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__) << b;

	for (int i = 0; i < params_EditorEnablers.size(); i++)
	{
		auto& p = params_EditorEnablers[i];//ofAbstractParameter
		auto type = p.type();
		bool isBool = type == typeid(ofParameter<bool>).name();
		string name = p.getName();

		if (isBool) {
			ofParameter<bool> pb = p.cast<bool>();
			pb.set(b);
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::doDisableAll() {
	doSetAll(false);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::doEnableAll() {
	doSetAll(true);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::refreshPlots()
{
	if (bGui_PlotIn && bGui_PlotOut)
	{
		for (int i = 0; i < amountPlots; i++)
		{
			bool b1 = (i % 2 == 0);
			bool bTitle = !b1;
			bool bInfo = false;
			bool bBg = b1;
			plots[i]->setColor(colors[i]);
			plots[i]->setDrawTitle(bTitle);
			plots[i]->setShowNumericalInfo(bInfo);
			plots[i]->setShowSmoothedCurve(false);
			plots[i]->setDrawBackground(bBg);
		}
	}
	else if (!bGui_PlotIn && bGui_PlotOut)
	{
		for (int i = 0; i < amountPlots; i++)
		{
			bool b1 = (i % 2 == 0);
			bool bTitle = !b1;
			bool bInfo = false;
			bool bBg = true;
			plots[i]->setColor(colors[i]);
			plots[i]->setDrawTitle(bTitle);
			plots[i]->setShowNumericalInfo(bInfo);
			plots[i]->setShowSmoothedCurve(false);
			plots[i]->setDrawBackground(bBg);
		}
	}
	else if (bGui_PlotIn && !bGui_PlotOut)
	{
		for (int i = 0; i < amountPlots; i++)
		{
			bool b1 = (i % 2 == 0);
			bool bTitle = true;
			bool bInfo = false;
			bool bBg = b1;
			plots[i]->setColor(colors[i]);
			plots[i]->setDrawTitle(bTitle);
			plots[i]->setShowNumericalInfo(bInfo);
			plots[i]->setShowSmoothedCurve(false);
			plots[i]->setDrawBackground(bBg);
		}
	}
}



//// fix workaround, for different related params and modes
//void ofxSurfingSmooth::doRefresh()
//{
//	ofLogWarning("SmoothChannel") << (__FUNCTION__);
//	// to trig callbacks
//
//	threshold = threshold;
//
//	onsetGrow = onsetGrow;
//	onsetDecay = onsetDecay;
//	timeRedirection = timeRedirection;
//
//	//int typeSmooth_ = typeSmooth;
//	//if (typeSmooth == 0) typeSmooth = 1;
//	//else if (typeSmooth == 1) typeSmooth = 2;
//	//else if (typeSmooth == 2) typeSmooth = 1;
//	//typeSmooth = typeSmooth_;
//
//	//float smoothPower_ = smoothPower;
//	//smoothPower = smoothPower.getMax();
//	//smoothPower = smoothPower_;
//
//	if (typeSmooth == 0) {}
//	else if (typeSmooth == 1) smoothPower = smoothPower;
//	else if (typeSmooth == 2) {
//		slideMin = slideMin;
//		slideMax = slideMax;
//	}
//
//	if (typeMean == 0) smoothPower = smoothPower;
//
//
//
//	// do the same with detectors 
//}