#include "ofxSurfingSmooth.h"

//--------------------------------------------------------------
ofxSurfingSmooth::ofxSurfingSmooth()
{
	ofAddListener(ofEvents().update, this, &ofxSurfingSmooth::update);
}

//--------------------------------------------------------------
ofxSurfingSmooth::~ofxSurfingSmooth()
{
	ofRemoveListener(ofEvents().update, this, &ofxSurfingSmooth::update);

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
	bGenerators = true;
	guiManager.bHelp = true;
	guiManager.bKeys = true;

	//--

	mParamsGroup.setName("ofxSurfingSmooth");
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
	//guiManager.bHelp = true;
	//guiManager.bKeys = true;

	//--

	// settings
	ofxSurfingHelpers::loadGroup(params, path_Settings);

	// Load file settings
	for (int i = 0; i < smoothChannels.size(); i++)
	{
		//fix
		//outputs[i].initAccum(0);

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
#ifdef COLORS_MONCHROME
	colorPlots = (ofColor::yellow);
	//colorPlots = (ofColor::green);
#endif

	colorBaseLine = ofColor(255, 100);
	colorTextSelected = ofColor(255, 150);

	colorThreshold = ofColor(150);

	//// style 1
	//colorTrig = ofColor(ofColor::red);
	//colorBonk = ofColor(ofColor::blue);
	//colorDirect = ofColor(ofColor::green);
	//colorDirectUp = ofColor(ofColor::green);
	//colorDirectDown = ofColor(ofColor::green);

	// style 2. all the same colors 
	colorTrig = colorPlots;
	colorBonk = colorPlots;
	colorDirect = colorPlots;
	colorDirectUp = colorPlots;
	colorDirectDown = colorPlots; \

		//--

	// circle beat widget
	circleBeatWidget.setColor(0, colorTrig);
	circleBeatWidget.setColor(1, colorBonk);
	circleBeatWidget.setColor(2, colorDirect);
	circleBeatWidget.setColor(3, colorDirectUp);
	circleBeatWidget.setColor(4, colorDirectDown);

	// colors
	colors.clear();
	colors.resize(amountPlots);

	// alphas
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
		c.setHsb(hueStep * i, sat, brg);
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
		plots[i]->setRange(0, 1);
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
			//float g = ofMap((float)MAX_AMP_POWER * smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
			float g = ofMap(smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
			g *= (float)MAX_AMP_POWER;

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
			//float g = ofMap((float)MAX_AMP_POWER * smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
			float g = ofMap(smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
			g *= (float)MAX_AMP_POWER;

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
			_input = ofClamp(inputs[i],
				smoothChannels[i]->minInput, smoothChannels[i]->maxInput);
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
		if (smoothChannels[i]->bGateMode)
		{
			// calculate duration depending on mode slow/fast
			if (!smoothChannels[i]->bGateSlow) // fast 
				gates[i].tDurationGate = (60000 / bpm) / (smoothChannels[i]->bpmDiv);
			else // slow
				gates[i].tDurationGate = (60000 / bpm) * (2 * smoothChannels[i]->bpmDiv);

			if (gates[i].bGateState) // true is for gate closed 
			{
				// calculate time to open the gate if timer passed the target duration
				gates[i].timerGate = (t - gates[i].lastTimerGate);
				if (gates[i].timerGate > gates[i].tDurationGate)
				{
					gates[i].bGateState = false; // re open / release the gate!
				}
			}
		}

		//isBang(i);
	}


	////--

	//TODO: it seems that calling isBang closes the gate!
	// so, can on be readed once per frame! 
	// and followed isBang are igored, returning always false!
	// should use a notifier/callback approach.

	// Gui Detector bang
	// showing the selected channel by the gui index
	// 0=TrigState, 1=Bonk, 2=Direction, 3=DirUp, 4=DirDown
	int i = index.get();
	if (this->isBang(i))
	{
		switch (smoothChannels[i]->bangDetectorIndex)
		{
			//case 0:
			//{
			//	circleBeatWidget.setMode(0);
			//	if (isTriggered(i)) circleBeatWidget.bang();
			//	else circleBeatWidget.reset();
			//	break;
			//}
		case 0: circleBeatWidget.bang(0); break;
		case 1: circleBeatWidget.bang(1); break;
		case 2: circleBeatWidget.bang(2); break;
		case 3: circleBeatWidget.bang(3); break;
		case 4: circleBeatWidget.bang(4); break;
		}
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
				//float g = ofMap((float)MAX_AMP_POWER * smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
				float g = ofMap(smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
				g *= (float)MAX_AMP_POWER;

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
				//float g = ofMap((float)MAX_AMP_POWER * smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
				float g = ofMap(smoothChannels[i]->ampInput, -1, 1, 0, 2, true);
				g *= (float)MAX_AMP_POWER;

				value = ofClamp(g * value, 0, 1);

				// feed param ranged
				ti.set((int)ofMap(value, 0, 1, ti.getMin(), ti.getMax()));

				// feed input normalized
				inputs[i] = value;
			}

			//--

			// Bool

			//--

			// Skip / by pass the other params bc unknown types
			else
			{
				// i--; // to don't discard the generator..
				//inputs[i] = value; 
				continue;
			}
		}

		//--

		//TODO:
		//outputs[i].update(inputs[i]); 
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw()
{
	if (!bGui_Global) return;

	//if (!bGui) return;

	if (bGui_Plots)
	{
		if (bGui_PlotFullScreen) drawPlots(ofGetCurrentViewport());
		else
		{
			//fix draw bg
			if (!bGui_PlotIn && !bGui_PlotOut)
			{
				ofColor c = plots[0]->getBackgroundColor();
				ofPushStyle();
				ofFill();
				ofSetColor(c);
				ofDrawRectangle(boxPlots.getRectangle());
				ofPopStyle();
			}
			drawPlots(boxPlots.getRectangle());
		}

		boxPlots.draw();

		if (boxPlots.isEditing()) boxPlots.drawBorderBlinking();
	}

	//if (bGui) draw_ImGui();
	draw_ImGui();
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
		ofDrawBitmapString(s, x + 5, y + 11);

		//--

		// Draw threshold line overlayed
		{
			/// each channel colored differently
			//ofColor _c1 = ofColor(colors[ii]);
			//ofColor _c2 = ofColor(colors[ii]);

			ofColor _c = colorThreshold;
			ofColor _c0 = colorTrig;
			ofColor _c1 = colorBonk;
			ofColor _c2 = colorDirect;
			ofColor _c2up = colorDirectUp;
			ofColor _c2down = colorDirectDown;

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

		// Mark selected channel with a vertical line on the left border 
		if (i == index && !bSolo)
		{
			int l = 4;
			int lh = 2;

			ofSetLineWidth(l);
			ofSetColor(colorPlots, ofMap(ofxSurfingHelpers::Bounce(), 0, 1, 64, 150));

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
	if (!guiManager.bKeys) return;

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
	params.add(alphaPlotInput.set("AlphaIn", 0.5, 0, 1));

	params.add(bEnableSmoothGlobal.set("ENABLE GLOBAL", true));

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

	params.add(bReset.set("ResetAll", false));

	//--

	//params.add(input.set("INPUT", 0, _inputMinRange, _inputMaxRange));
	//params.add(output.set("OUTPUT", 0, _outMinRange, _outMaxRange));

	guiManager.bGui_GameMode.setName("SMOOTH");

	//why required?
	params.add(guiManager.bHelp);
	params.add(guiManager.bMinimize);
	params.add(guiManager.bKeys);
	params.add(guiManager.bGui_GameMode);

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
	//typeSmooth_Str.setSerializable(false);
	//typeMean_Str.setSerializable(false);
	//input.setSerializable(false);
	//output.setSerializable(false);
	//bPlay.setSerializable(false);

	ofAddListener(params.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Params); // setup()

	//--

	buildHelp();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::exit() {
	ofRemoveListener(params.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Params); // exit()

	ofxSurfingHelpers::CheckFolder(path_Global);
	ofxSurfingHelpers::saveGroup(params, path_Settings);
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

	else if (name == guiManager.bKeys.getName())
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

	// Enable all
	else if (name == bGui_Main.getName())
	{
		////workflow
		//if (!bGui_Main) {
		//	bGui_Extra = false;
		//}
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
		if (bReset)
		{
			bReset = false;
			doReset();
		}
		return;
	}

	//----

	// remove below..

	//else if (name == threshold.getName())
	//{
	//	for (int i = 0; i < amountChannels; i++) {
	//		outputs[i].setThresh(threshold);
	//	}
	//	return;
	//}

	////--

	//else if (name == smoothPower.getName())
	//{
	//	int MAX_ACC_HISTORY = 60;//calibrated to 60fps
	//	float v = ofMap(smoothPower, 0, 1, 1, MAX_ACC_HISTORY);
	//	for (int i = 0; i < amountChannels; i++) {
	//		outputs[i].initAccum(v);
	//	}
	//	return;
	//}

	////--

	//else if (name == slideMin.getName() || name == slideMax.getName())
	//{
	//	for (int i = 0; i < amountChannels; i++) {
	//		const int MIN_SLIDE = 1;
	//		const int MAX_SLIDE = 50;
	//		float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
	//		float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

	//		outputs[i].initSlide(_slmin, _slmax);
	//	}
	//	return;
	//}

	////--

	//else if (name == minOutput.getName() || name == maxOutput.getName())
	//{
	//	for (int i = 0; i < amountChannels; i++) {
	//		outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));
	//	}
	//	return;
	//}

	////--

	//else if (name == bNormalized.getName())
	//{
	//	for (int i = 0; i < amountChannels; i++)
	//	{
	//		//outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));

	//		if (bNormalized) outputs[i].setNormalized(bNormalized, ofVec2f(0, 1));
	//		else outputs[i].setNormalized(bNormalized, ofVec2f(minOutput, maxOutput));
	//	}
	//	return;
	//}

	////--

	////TODO:
	////detect "bonks" (onsets):
	////amp.setBonk(0.1, 0.1);  // min growth for onset, min decay
	////set growth/decay:
	////amp.setDecayGrow(true, 0.99); // a framerate-dependent steady decay/growth
	//else if (name == onsetGrow.getName() || name == onsetDecay.getName())
	//{
	//	for (int i = 0; i < amountChannels; i++) {
	//		outputs[i].setBonk(onsetGrow, onsetDecay);
	//		//specAmps[i].setDecayGrow(true, 0.99);

	//		outputs[i].directionChangeCalculated = true;
	//		//outputs[i].setBonk(0.1, 0.0);
	//	}
	//	return;
	//}

	////--

	//else if (name == typeSmooth.getName())
	//{
	//	typeSmooth = ofClamp(typeSmooth, typeSmooth.getMin(), typeSmooth.getMax());

	//	switch (typeSmooth)
	//	{
		//case ofxDataStream::SMOOTHING_NONE:
	//	{
	//		if (!bEnableSmooth) bEnableSmooth = false;
	//		typeSmooth_Str = typeSmoothLabels[0];
	//		return;
	//	}
	//	break;

	//	case ofxDataStream::SMOOTHING_ACCUM:
	//	{
	//		if (!bEnableSmooth) bEnableSmooth = true;
	//		typeSmooth_Str = typeSmoothLabels[1];
	//		int MAX_HISTORY = 30;
	//		float v = ofMap(smoothPower, 0, 1, 1, MAX_HISTORY);
	//		for (int i = 0; i < amountChannels; i++) {
	//			outputs[i].initAccum(v);
	//		}
	//		return;
	//	}
	//	break;

	//	case ofxDataStream::SMOOTHING_SLIDE:
	//	{
	//		if (!bEnableSmooth) bEnableSmooth = true;
	//		typeSmooth_Str = typeSmoothLabels[2];
	//		for (int i = 0; i < amountChannels; i++) {
	//			const int MIN_SLIDE = 1;
	//			const int MAX_SLIDE = 50;
	//			float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
	//			float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

	//			outputs[i].initSlide(_slmin, _slmax);
	//		}
	//		return;
	//	}
	//	break;
	//	}

	//	return;
	//}

	////--

	//else if (name == typeMean.getName())
	//{
	//	typeMean = ofClamp(typeMean, typeMean.getMin(), typeMean.getMax());

	//	switch (typeMean)
	//	{
	//	case ofxDataStream::MEAN_ARITH:
	//	{
	//		typeMean_Str = typeMeanLabels[0];
	//		for (int i = 0; i < amountChannels; i++) {
	//			outputs[i].setMeanType(ofxDataStream::MEAN_ARITH);
	//		}
	//		return;
	//	}
	//	break;

	//	case ofxDataStream::MEAN_GEOM:
	//	{
	//		typeMean_Str = typeMeanLabels[1];
	//		for (int i = 0; i < amountChannels; i++) {
	//			outputs[i].setMeanType(ofxDataStream::MEAN_GEOM);
	//		}
	//		return;
	//	}
	//	break;

	//	case ofxDataStream::MEAN_HARM:
	//	{
	//		typeMean_Str = typeMeanLabels[2];
	//		for (int i = 0; i < amountChannels; i++) {
	//			outputs[i].setMeanType(ofxDataStream::MEAN_HARM);
	//		}
	//		return;
	//	}
	//	break;
	//	}

	//	return;
	//}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setup_ImGui()
{
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	guiManager.setName("SmoothSurf");
	guiManager.setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	guiManager.setup();

	guiManager.addWindowSpecial(bGui_Main);
	guiManager.addWindowSpecial(guiManager.bGui_GameMode);
	guiManager.addWindowSpecial(bGui_Extra);
	guiManager.addWindowSpecial(bGui_Inputs);
	guiManager.addWindowSpecial(bGui_Outputs);

	guiManager.startup();

	//--

	// custom
	guiManager.setHelpInfoApp(helpInfo);
	guiManager.bHelpInternal = false;
	bGui_GameMode.makeReferenceTo(guiManager.bGui_GameMode);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGuiExtra()
{
	if (bGui_Extra)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;

		if (guiManager.beginWindowSpecial(bGui_Extra))
		{
			// In / Out

			if (!guiManager.bMinimize)
			{
				if (guiManager.beginTree("IN / OUT"))
				{
					guiManager.Add(bGui_Inputs, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
					guiManager.Add(bGui_Outputs, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
					guiManager.endTree();
				}
				guiManager.AddSpacingSeparated();
			}

			// Plots
			if (!bGui_GameMode)
				if (!guiManager.bMinimize)
				{
					if (guiManager.beginTree("PLOTS"))
					{
						guiManager.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
						if (bGui_Plots)
						{
							guiManager.Indent();
							guiManager.Add(bGui_PlotIn, OFX_IM_TOGGLE_ROUNDED);
							guiManager.Add(bGui_PlotOut, OFX_IM_TOGGLE_ROUNDED);
							guiManager.Indent();
							guiManager.Add(bGui_PlotFullScreen, OFX_IM_TOGGLE_ROUNDED_MINI);
							guiManager.Add(boxPlots.bEdit, OFX_IM_TOGGLE_ROUNDED_MINI);
							if (guiManager.Add(bGui_PlotsLink, OFX_IM_TOGGLE_ROUNDED_MINI)) {
							};
							guiManager.Unindent();
							guiManager.Unindent();
						}
						guiManager.endTree();
					}
					guiManager.AddSpacingSeparated();
				}

			//----

			// Enable Toggles

			if (!guiManager.bMinimize)
			{
				if (guiManager.beginTree("ENABLERS"))
				{
					if (guiManager.AddButton("NONE", OFX_IM_BUTTON_SMALL, 2))
					{
						doDisableAll();
					}
					guiManager.SameLine();
					if (guiManager.AddButton("ALL", OFX_IM_BUTTON_SMALL, 2))
					{
						doEnableAll();
					}

					guiManager.AddSpacing();

					for (int i = 0; i < params_EditorEnablers.size(); i++)
					{
						auto& p = params_EditorEnablers[i];// ofAbstractParameter
						auto type = p.type();
						bool isBool = type == typeid(ofParameter<bool>).name();
						string name = p.getName();

						if (isBool) // just in case... 
						{
							ofParameter<bool> pb = p.cast<bool>();
							guiManager.Add(pb, OFX_IM_CHECKBOX);
						}
					}

					guiManager.endTree();
				}

				guiManager.AddSpacingSeparated();
			}

			if (!guiManager.bMinimize)
			{
				if (guiManager.beginTree("TESTER"))
				{
					guiManager.Add(bGenerators, OFX_IM_TOGGLE);

					if (guiManager.AddButton("Randomizer", OFX_IM_BUTTON))
					{
						doRandomize();
					}

					// blink by timer
					{
						bool b = bPlay;
						float a;
						if (b) a = 1 - tn;
						if (b) ImGui::PushStyleColor(ImGuiCol_Border,
							(ImVec4)ImColor::HSV(0.5f, 0.0f, 1.0f, 0.5 * a));
						guiManager.Add(bPlay, OFX_IM_TOGGLE);
						if (b) ImGui::PopStyleColor();
					}
					if (bPlay)
					{
						guiManager.Add(playSpeed);
					}

					guiManager.endTree();
				}
			}

			//--

			if (!guiManager.bMinimize)
			{
				guiManager.AddSpacingSeparated();

				if (ImGui::CollapsingHeader("Advanced"))
				{
					guiManager.refreshLayout();

					guiManager.Add(guiManager.bKeys, OFX_IM_TOGGLE_BUTTON_ROUNDED_MEDIUM);
					guiManager.Add(guiManager.bHelp, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
					guiManager.Add(boxPlots.bEdit);
				}
			}

			guiManager.endWindowSpecial();
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGuiGameMode()
{
	if (guiManager.bGui_GameMode) ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

	if (guiManager.beginWindowSpecial(guiManager.bGui_GameMode))
	{
		int i = index.get();

		//--

		guiManager.AddLabelHuge("SMOOTH & BANG");
		//guiManager.AddLabelHuge(guiManager.bGui_GameMode.getName());
		guiManager.AddSpacingBigSeparated();

		////TODO:
		//ImGui::Columns(2, "arrows", true);
		//ImVec2 sz(20, 40);
		//guiManager.AddButton("»", sz);
		//guiManager.SameLine();
		//ImGui::NextColumn();
		//guiManager.AddButton("»", sz);
		//ImGui::Columns(1);

		guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_ROUNDED_MEDIUM);

		if (!guiManager.bMinimize) {
			//guiManager.AddSpacingSeparated();
			guiManager.Add(bGui_Main, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
			//guiManager.Add(bGui_Main);
		}

		guiManager.AddSpacingSeparated();

		//--

		if (guiManager.beginTree("CHANNEL", true, true))
		{
			//if (!guiManager.bMinimize)
			//{
			//	guiManager.AddLabelBig("Signal");
			//	//guiManager.AddLabelBig("CHANNEL");
			//}

			// Channel name
			if (i > editorEnablers.size() - 1)
			{
				ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Index out of range! #" << i;
			}
			else
			{
				//ImGui::Columns(2, "t1", false);
				string n = "#" + ofToString(i);
				guiManager.AddLabel(n, false, true);
				//ImGui::NextColumn();
				guiManager.SameLine();
				n = editorEnablers[i].getName();
				guiManager.AddLabelBig(n, false, true);
				//ImGui::Columns(1);
			}

			// Selector
			if (!guiManager.bMinimize) {
				bool bResponsive = true;
				int amountBtRow = 3;
				const bool bDrawBorder = true;
				float __h = 1.5f * ofxImGuiSurfing::getWidgetsHeightUnit();
				string toolTip = "Select Signal channel";
				ofxImGuiSurfing::AddMatrixClicker(index, bResponsive, amountBtRow, bDrawBorder, __h, toolTip);
			}
			else
			{
				guiManager.AddComboArrows(index);
			}

			// Enable, Solo, Amp
			if (!guiManager.bMinimize)
			{
				//guiManager.AddSpacing();
				guiManager.Add(smoothChannels[i]->bEnableSmooth, OFX_IM_TOGGLE_SMALL, 2);
				//guiManager.AddTooltip("Activate Smoother or bypass");
				guiManager.SameLine();
				guiManager.Add(bSolo, OFX_IM_TOGGLE_SMALL_BORDER_BLINK, 2);

				//guiManager.Add(smoothChannels[i]->ampInput);
				guiManager.Add(smoothChannels[i]->ampInput, OFX_IM_HSLIDER_MINI);
				guiManager.AddTooltip(ofToString(smoothChannels[i]->ampInput.get(), 2));
			}

			guiManager.endTree();
		}

		guiManager.AddSpacingSeparated();

		//--

		if (smoothChannels[i]->bEnableSmooth) {

			if (guiManager.beginTree("SMOOTH", true, true))
			{
				guiManager.AddSpacing();

				//--

				guiManager.AddCombo(smoothChannels[i]->typeSmooth, typeSmoothLabels);
				guiManager.AddTooltip("Type Smooth");

				if (smoothChannels[i]->typeSmooth == ofxDataStream::SMOOTHING_ACCUM)
				{
					guiManager.Add(smoothChannels[i]->smoothPower, OFX_IM_HSLIDER_SMALL_NO_NUMBER);
					guiManager.AddTooltip(ofToString(smoothChannels[i]->smoothPower.get(), 2));
				}
				if (smoothChannels[i]->typeSmooth == ofxDataStream::SMOOTHING_SLIDE)
				{
					guiManager.Add(smoothChannels[i]->slideMin, OFX_IM_HSLIDER_MINI);
					guiManager.AddTooltip(ofToString(smoothChannels[i]->slideMin.get(), 2));
					guiManager.Add(smoothChannels[i]->slideMax, OFX_IM_HSLIDER_MINI);
					guiManager.AddTooltip(ofToString(smoothChannels[i]->slideMax.get(), 2));
				}

				guiManager.AddCombo(smoothChannels[i]->typeMean, typeMeanLabels);
				guiManager.AddTooltip("Type Mean");

				guiManager.endTree();
			}

			guiManager.AddSpacingSeparated();

			//--

				// Bang Detector / Trigger selector

			if (guiManager.beginTree("DETECTOR", true, true))
			{
				guiManager.AddSpacing();

				guiManager.AddLabel("Mode");

				guiManager.AddComboButtonDualLefted(smoothChannels[i]->bangDetectorIndex, smoothChannels[i]->bangDetectors);

				//--

				//if (!guiManager.bMinimize)
				{
					if (smoothChannels[i]->bangDetectorIndex == 0) // state
					{
						guiManager.Add(smoothChannels[i]->threshold, OFX_IM_HSLIDER_SMALL);
						//guiManager.AddTooltip(ofToString(smoothChannels[i]->threshold.get(), 2));
					}
					else if (smoothChannels[i]->bangDetectorIndex == 1) // bonk
					{
						guiManager.Add(smoothChannels[i]->onsetGrow, OFX_IM_HSLIDER_SMALL);
						//guiManager.AddTooltip(ofToString(smoothChannels[i]->onsetGrow.get(), 2));
						guiManager.Add(smoothChannels[i]->onsetDecay, OFX_IM_HSLIDER_SMALL);
						//guiManager.AddTooltip(ofToString(smoothChannels[i]->onsetDecay.get(), 2));
					}
					else if (smoothChannels[i]->bangDetectorIndex == 2 || // direction
						smoothChannels[i]->bangDetectorIndex == 3 || // up
						smoothChannels[i]->bangDetectorIndex == 4) // down
					{
						guiManager.Add(smoothChannels[i]->timeRedirection, OFX_IM_HSLIDER_SMALL);
						//guiManager.AddTooltip(ofToString(smoothChannels[i]->timeRedirection.get(), 2));
					}
				}

				guiManager.AddSpacing();

				// Circle Widget

				string n = editorEnablers[i].getName();
				guiManager.AddLabel(n, false, true);
				guiManager.AddLabelBig("BANG!", true, true);
				guiManager.AddSpacing();

				circleBeatWidget.draw();

				//--

				//guiManager.AddSpacingSeparated();
				guiManager.AddSpacing();

				guiManager.Add(smoothChannels[i]->bGateMode, OFX_IM_TOGGLE_SMALL);
				if (smoothChannels[i]->bGateMode)
				{
					// Timer progress
					float v = ofMap(gates[i].timerGate, 0, gates[i].tDurationGate, 0, 1, true);
					ofxImGuiSurfing::ProgressBar2(v, 1.f, true);
					ofxImGuiSurfing::AddToggleNamed(smoothChannels[i]->bGateSlow, "Slow", "Fast");
					guiManager.Add(smoothChannels[i]->bpmDiv, OFX_IM_STEPPER);
				}

				//--

				guiManager.endTree();
			}

			guiManager.AddSpacingSeparated();
		}

		//--

		if (guiManager.bMinimize)
		{
			guiManager.Add(guiManager.bKeys, OFX_IM_TOGGLE_ROUNDED);
		}

		//--

		if (!guiManager.bMinimize)
			if (guiManager.beginTree("EXTRA"))
			{
				//if (!guiManager.bMinimize)
				{
					if (guiManager.beginTree("PLOTS", false))
					{
						guiManager.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
						if (bGui_Plots)
						{
							guiManager.Indent();
							guiManager.Add(bGui_PlotIn, OFX_IM_TOGGLE_ROUNDED);
							guiManager.Add(bGui_PlotOut, OFX_IM_TOGGLE_ROUNDED);
							guiManager.Indent();
							guiManager.Add(bGui_PlotFullScreen, OFX_IM_TOGGLE_ROUNDED_MINI);
							//guiManager.Add(boxPlots.bEdit, OFX_IM_TOGGLE_ROUNDED_MINI);
							//guiManager.Add(bGui_PlotsLink, OFX_IM_TOGGLE_ROUNDED_MINI);
							guiManager.Unindent();
							guiManager.Unindent();
						}

						guiManager.endTree();
					}

					//guiManager.AddSpacing();
					guiManager.AddSpacingSeparated();

					guiManager.Add(guiManager.bKeys, OFX_IM_TOGGLE_ROUNDED);
					guiManager.Add(guiManager.bHelp, OFX_IM_TOGGLE_ROUNDED);
					//guiManager.Add(bEnableSmoothGlobal, OFX_IM_TOGGLE_ROUNDED_MINI);
					guiManager.Add(bGui_Extra, OFX_IM_TOGGLE_ROUNDED_MINI);
					guiManager.Add(bGenerators, OFX_IM_TOGGLE_SMALL);
				}
				//else
				//{
				//	guiManager.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED);
				//	if (bGui_Plots)
				//	{
				//		guiManager.Indent();
				//		guiManager.Add(bGui_PlotIn, OFX_IM_TOGGLE_ROUNDED_MINI);
				//		guiManager.Unindent();
				//	}
				//	////workflow
				//	//if (bGui_Plots)
				//	//	if (!bGui_PlotIn && !!bGui_PlotOut) {
				//	//		bGui_PlotIn = bGui_PlotOut = true;
				//	//	}

				//	guiManager.Add(guiManager.bKeys, OFX_IM_TOGGLE_ROUNDED);
				//	guiManager.Add(guiManager.bHelp, OFX_IM_TOGGLE_ROUNDED);

				//	//guiManager.Indent();
				//	//guiManager.Add(bGui_PlotIn, OFX_IM_TOGGLE_ROUNDED);
				//	//if (guiManager.Add(bGui_PlotsLink, OFX_IM_TOGGLE_ROUNDED_MINI)) {
				//	//};
				//	//guiManager.Unindent();

				//}

				guiManager.endTree();
			}

		//--

		guiManager.AddSpacingSeparated();

		if (!guiManager.bMinimize)
		{
			if (guiManager.Add(smoothChannels[i]->bReset, OFX_IM_BUTTON_SMALL, 2)) {

			};
			guiManager.SameLine();
			if (guiManager.Add(bReset, OFX_IM_BUTTON_SMALL, 2)) {
				doReset();
			};
		}
		else {
			guiManager.Add(smoothChannels[i]->bReset, OFX_IM_BUTTON_SMALL);
		}

		guiManager.endWindowSpecial();
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGuiMain()
{
	if (bGui_Main)
	{
		int i = index.get();

		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;

		if (guiManager.beginWindowSpecial(bGui_Main))
		{
			guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_ROUNDED);
			guiManager.AddSpacingSeparated();

			guiManager.Add(guiManager.bGui_GameMode, OFX_IM_TOGGLE_ROUNDED_BIG);
			guiManager.AddSpacingSeparated();

			//--

			// Enable Global
			guiManager.Add(bEnableSmoothGlobal, OFX_IM_TOGGLE_BIG_BORDER_BLINK);

			if (!guiManager.bMinimize)
			{
				guiManager.AddSpacingSeparated();

				guiManager.Add(smoothChannels[i]->bClamp, OFX_IM_TOGGLE_SMALL, 2, true);
				guiManager.Add(smoothChannels[i]->bNormalized, OFX_IM_TOGGLE_SMALL, 2);

				if (smoothChannels[i]->bClamp)
				{
					guiManager.AddSpacing();
					guiManager.Add(smoothChannels[i]->minInput);
					guiManager.Add(smoothChannels[i]->maxInput);
					guiManager.AddSpacing();
					guiManager.Add(smoothChannels[i]->minOutput);
					guiManager.Add(smoothChannels[i]->maxOutput);
					//guiManager.AddSpacing();
				}
			}

			//guiManager.AddSpacingSeparated();

			// Monitor
			if (!bGui_GameMode) {

				if (bEnableSmoothGlobal)
				{
					guiManager.AddSpacingSeparated();

					if (ImGui::CollapsingHeader("MONITOR"))
					{
						guiManager.refreshLayout();

						if (index < editorEnablers.size())
						{
							guiManager.AddLabelBig(editorEnablers[index].getName(), false);
						}

						if (!guiManager.bMinimize)
						{
							if (index.getMax() < 5)
							{
								ofxImGuiSurfing::AddMatrixClicker(index);
							}
							else {
								if (guiManager.Add(index, OFX_IM_STEPPER))
								{
									index = ofClamp(index, index.getMin(), index.getMax());
								}
							}
							guiManager.AddSpacingSeparated();
							guiManager.Add(smoothChannels[i]->bEnableSmooth, OFX_IM_TOGGLE);
							guiManager.Add(bSolo, OFX_IM_TOGGLE);

							//guiManager.Add(input, OFX_IM_HSLIDER_MINI);
							//guiManager.Add(output, OFX_IM_HSLIDER_MINI);
						}
						else
						{
							if (index.getMax() < 5)
							{
								ofxImGuiSurfing::AddMatrixClicker(index);
							}
							else {
								if (guiManager.Add(index, OFX_IM_STEPPER))
								{
									index = ofClamp(index, index.getMin(), index.getMax());
								}
							}

							guiManager.Add(bSolo, OFX_IM_TOGGLE);

							//guiManager.AddSpacingSeparated();
							//guiManager.Add(input, OFX_IM_HSLIDER_MINI_NO_LABELS);
							//guiManager.Add(output, OFX_IM_HSLIDER_MINI_NO_LABELS);
						}
					}

					guiManager.refreshLayout();
				}
			}

			//--

			if (bEnableSmoothGlobal)
			{
				guiManager.AddSpacingSeparated();

				if (ImGui::CollapsingHeader("ENGINE"))
				{
					guiManager.AddSpacing();

					if (!guiManager.bMinimize)
						if (guiManager.AddButton("SMOOTH +", OFX_IM_BUTTON))
						{
							nextTypeSmooth(index.get());
						}

					guiManager.AddCombo(smoothChannels[i]->typeSmooth, typeSmoothLabels);

					if (smoothChannels[i]->typeSmooth == ofxDataStream::SMOOTHING_ACCUM)
					{
						guiManager.Add(smoothChannels[i]->smoothPower, OFX_IM_HSLIDER_SMALL);
					}

					if (smoothChannels[i]->typeSmooth == ofxDataStream::SMOOTHING_SLIDE)
					{
						guiManager.Add(smoothChannels[i]->slideMin, OFX_IM_HSLIDER_MINI);
						guiManager.Add(smoothChannels[i]->slideMax, OFX_IM_HSLIDER_MINI);
					}

					guiManager.AddSpacingSeparated();

					//--

					if (!guiManager.bMinimize)
						if (guiManager.AddButton("MEAN +", OFX_IM_BUTTON))
						{
							nextTypeMean(index.get());
						}

					guiManager.AddCombo(smoothChannels[i]->typeMean, typeMeanLabels);

					//guiManager.AddSpacingSeparated();
					//guiManager.Add(smoothChannels[i]->bReset, OFX_IM_BUTTON_SMALL);
				}
			}

			//--

			if (bEnableSmoothGlobal)
			{
				guiManager.AddSpacingSeparated();

				if (ImGui::CollapsingHeader("DETECTOR"))
				{
					guiManager.refreshLayout();

					guiManager.AddCombo(smoothChannels[i]->bangDetectorIndex, smoothChannels[i]->bangDetectors);

					if (!guiManager.bMinimize)
					{
						guiManager.Add(smoothChannels[i]->threshold, OFX_IM_HSLIDER_SMALL);
						guiManager.Add(smoothChannels[i]->threshold, OFX_IM_STEPPER);
						guiManager.AddSpacing();

						guiManager.AddLabel("Direction");
						guiManager.Add(smoothChannels[i]->timeRedirection, OFX_IM_STEPPER);

						guiManager.AddLabel("Bonks");
						guiManager.Add(smoothChannels[i]->onsetGrow, OFX_IM_STEPPER);
						guiManager.Add(smoothChannels[i]->onsetDecay, OFX_IM_STEPPER);
					}
					else
					{
						if (smoothChannels[i]->bangDetectorIndex == 2 ||
							smoothChannels[i]->bangDetectorIndex == 3 ||
							smoothChannels[i]->bangDetectorIndex == 4)
						{
							guiManager.Add(smoothChannels[i]->timeRedirection, OFX_IM_HSLIDER_SMALL);
						}

						//if (bangDetectorIndex == 1)
						{
							guiManager.Add(smoothChannels[i]->onsetGrow, OFX_IM_STEPPER);
							guiManager.Add(smoothChannels[i]->onsetDecay, OFX_IM_STEPPER);
						}
					}

					guiManager.AddSpacingBigSeparated();

					if (guiManager.Add(smoothChannels[i]->bReset, OFX_IM_BUTTON_SMALL))
					{
					};
				}
				guiManager.refreshLayout();
			}

			//--

			if (!guiManager.bMinimize)
			{
				guiManager.AddSpacingSeparated();
				guiManager.Add(bGui_Extra, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
			}
		}

		guiManager.endWindowSpecial();
	}
}
//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGui()
{
	if (!bGui_Global) return;

	guiManager.begin();
	{
		//TODO:
		if (bGui_PlotsLink)
		{
			// Get last window
			//int pad = guiManager.getWindowSpecialPadSize();
			int padmin = 8;
			int pad = MAX(guiManager.getWindowSpecialPadSize(), padmin);//add a bit more space
			glm::vec2 p = guiManager.getWindowSpecialLastTopRight();
			p = p + glm::vec2(pad, 0);

			// Current window
			//ofRectangle r = guiManager.getWindowShape();
			//int pad = MAX(guiManager.getWindowSpecialPadSize(), 2);

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

		if (guiManager.bGui_GameMode) draw_ImGuiGameMode();

		//--

		if (bGui_Main) draw_ImGuiMain();
		//else return;

		//--

		if (bGui_Inputs)
		{
			if (guiManager.beginWindowSpecial(bGui_Inputs))
			{
				guiManager.AddGroup(mParamsGroup);
				guiManager.endWindowSpecial();
			}
		}

		//--

		if (bGui_Outputs)
		{
			if (guiManager.beginWindowSpecial(bGui_Outputs))
			{
				guiManager.AddGroup(mParamsGroup_Smoothed);
				guiManager.endWindowSpecial();
			}
		}

		//--

		if (bGui_Extra)
		{
			draw_ImGuiExtra();
		}
	}
	guiManager.end();
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

			ofLogNotice("ofxSurfingSmooth") << "--";
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

			ofLogNotice("ofxSurfingSmooth") << "Ch: " << i;

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
				case ofxDataStream::SMOOTHING_ACCUM:
				{
					float v = ofMap(smoothChannels[i]->smoothPower, 0, 1, 1, MAX_HISTORY);
					{
						outputs[i].initAccum(v);
					}
					smoothChannels[i]->doRefresh();
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
					smoothChannels[i]->doRefresh();
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
					smoothChannels[i]->doRefresh();
					return;
				}
				break;

				case ofxDataStream::MEAN_GEOM:
				{
					outputs[i].setMeanType(ofxDataStream::MEAN_GEOM);
					smoothChannels[i]->doRefresh();
					return;
				}
				break;

				case ofxDataStream::MEAN_HARM:
				{
					outputs[i].setMeanType(ofxDataStream::MEAN_HARM);
					smoothChannels[i]->doRefresh();
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
				smoothChannels[i]->doRefresh();
				return;
			}

			//--

			else if (name == smoothChannels[i]->smoothPower.getName())
			{
				//float v = ofMap(smoothChannels[i]->smoothPower, 0, 1, 1, MAX_ACC_HISTORY);
				float v = ofMap(smoothChannels[i]->smoothPower, 0, 1, 1, MAX_HISTORY);
				outputs[i].initAccum(v);
				smoothChannels[i]->doRefresh();
				return;
			}

			//--

			else if (name == smoothChannels[i]->slideMin.getName() ||
				name == smoothChannels[i]->slideMax.getName())
			{
				float _slmin = ofMap(smoothChannels[i]->slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
				float _slmax = ofMap(smoothChannels[i]->slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
				outputs[i].initSlide(_slmin, _slmax);
				smoothChannels[i]->doRefresh();
				return;
			}

			//--

			//TODO:
			else if (name == smoothChannels[i]->minOutput.getName() || name == smoothChannels[i]->maxOutput.getName())
			{
				outputs[i].setOutputRange(ofVec2f(
					smoothChannels[i]->minOutput,
					smoothChannels[i]->maxOutput));
				smoothChannels[i]->doRefresh();
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
				smoothChannels[i]->doRefresh();
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
				smoothChannels[i]->doRefresh();
				return;
			}

			//--

			// Do not requires to update the engine!

			else if (name == smoothChannels[i]->bangDetectorIndex.getName())
			{
				//	circleBeatWidget.reset();
				//	circleBeatWidget.setMode(smoothChannels[i]->bangDetectorIndex.get());
				smoothChannels[i]->doRefresh();
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
		outputs[i].initAccum(100);
		outputs[i].directionChangeCalculated = true;
		outputs[i].setBonk(smoothChannels[i]->onsetGrow, smoothChannels[i]->onsetDecay);

		// prepare gate engine
		GateStruct gate;
		gate.bGateState.setSerializable(false);
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