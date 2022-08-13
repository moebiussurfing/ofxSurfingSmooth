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
void ofxSurfingSmooth::setup() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	path_Global = "ofxSurfingSmooth/";
	path_Settings = path_Global + "ofxSurfingSmooth_Settings.xml";
	ofxSurfingHelpers::CheckFolder(path_Global);

	//-

	setupParams();

	//-

	editorEnablers.clear();// an enabler toggler for each param
	params_EditorEnablers.clear();// an enabler toggler for each param
	params_EditorEnablers.setName("Params");

	//--

	generators.resize(NUM_GENERATORS);

	//--

	setup_ImGui();

	bGui = true;

	//--

	mParamsGroup.setName("ofxSurfingSmooth");
	ofAddListener(mParamsGroup.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Controls_Out);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::startup() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	bDISABLE_CALLBACKS = false;

	doReset();

	//--

	// settings
	ofxSurfingHelpers::loadGroup(params, path_Settings);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setupPlots() {
	NUM_VARS = mParamsGroup.size();
	NUM_PLOTS = 2 * NUM_VARS;

	index.setMax(NUM_VARS - 1);
	plots.resize(NUM_PLOTS);

	// colors

#ifdef COLORS_MONCHROME
	//colorPlots = (ofColor::green);
	colorPlots = (ofColor::yellow);
#endif

	colorBaseLine = ofColor(255, 48);
	colorTextSelected = ofColor(255, 150);
	colorThreshold = ofColor(150, 150);
	colorTrig = ofColor(ofColor::red, 255);
	colorBonk = ofColor(ofColor::blue, 255);
	colorDirect = ofColor(ofColor::green, 255);

	// colors
	colors.clear();
	colors.resize(NUM_PLOTS);

	// alphas
	int a1 = 128;//input
	int a2 = 255;//output
	ofColor c;

#ifdef COLORS_MONCHROME
	for (int i = 0; i < NUM_VARS; i++)
	{
		c = colorPlots;
		colors[2 * i] = ofColor(c, a1);
		colors[2 * i + 1] = ofColor(c, a2);
	}
#endif

#ifndef COLORS_MONCHROME
	int sat = 255;
	int brg = 255;
	int hueStep = 255. / (float)NUM_VARS;
	for (int i = 0; i < NUM_VARS; i++)
	{
		c.setHsb(hueStep * i, sat, brg);
		colors[2 * i] = ofColor(c, a1);
		colors[2 * i + 1] = ofColor(c, a2);
	}
#endif

	for (int i = 0; i < NUM_PLOTS; i++)
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

		plots[i] = new ofxHistoryPlot(NULL, _name, 60 * 4, false);//4 secs at 60fps
		plots[i]->setRange(0, 1);
		plots[i]->setColor(colors[i]);
		plots[i]->setDrawTitle(bTitle);
		plots[i]->setShowNumericalInfo(bInfo);
		plots[i]->setShowSmoothedCurve(false);
		plots[i]->setDrawBackground(bBg);
		plots[i]->setDrawGrid(bGrid);
	}

	// draggable rectangle

	boxPlots.setPathGlobal(path_Global);
	boxPlots.setup();
	boxPlots.bEdit.setName("Edit Plots");
}

//--------------------------------------------------------------
void ofxSurfingSmooth::update(ofEventArgs& args) {
	if (ofGetFrameNum() == 0) { startup(); }

	// tester
	// play timed randoms
	static const int _secs = 2;
	if (bPlay) {
		//int max = 60 * _secs;
		int max = ofMap(playSpeed, 0, 1, 60, 5) * _secs;
		tf = ofGetFrameNum() % max;
		tn = ofMap(tf, 0, max, 0, 1);
		if (tf == 0)
		{
			doRandomize();
		}
	}

	// engine
	if (bUseGenerators) updateGenerators();

	updateEngine();

	updateSmooths();
	//if (!bUseGenerators) updateSmooths();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::updateSmooths() {
	//getting from the params not from the generators!

	for (int i = 0; i < mParamsGroup.size(); i++)
	{
		ofAbstractParameter& p = mParamsGroup[i];

		//toggle
		auto& _p = params_EditorEnablers[i];// ofAbstractParameter
		auto type = _p.type();
		bool isBool = type == typeid(ofParameter<bool>).name();
		string name = _p.getName();
		ofParameter<bool> _bSmooth = _p.cast<bool>();

		//-

		//string str = "";
		//string name = aparam.getName();
		float vn = 0;//normalized params

		if (p.type() == typeid(ofParameter<float>).name()) {
			ofParameter<float> _p = p.cast<float>();
			vn = ofMap(_p, _p.getMin(), _p.getMax(), 0, 1);

			//smooth group
			auto pc = mParamsGroup_Smoothed.getFloat(_p.getName() + suffix);

			if (bEnableSmooth && _bSmooth) {
				float v = ofMap(outputs[i].getValue(), 0, 1, _p.getMin(), _p.getMax(), true);
				pc.set(v);
			}
			else {
				pc.set(_p.get());
			}
		}

		else if (p.type() == typeid(ofParameter<int>).name()) {
			ofParameter<int> _p = p.cast<int>();
			vn = ofMap(_p, _p.getMin(), _p.getMax(), 0, 1);

			//smooth group
			auto pc = mParamsGroup_Smoothed.getInt(_p.getName() + suffix);

			if (bEnableSmooth && _bSmooth) {
				int v = ofMap(outputs[i].getValue(), 0, 1, _p.getMin(), _p.getMax() + 1, true);//TODO: round fix..
				//int v = ofMap(outputs[i].getValue(), 0, 1, _p.getMin(), _p.getMax());
				pc.set(v);
			}
			else {
				pc.set(_p.get());
			}
		}

		//else if (p.type() == typeid(ofParameter<bool>).name()) {
		//	ofParameter<bool> ti = p.cast<bool>();
		//}

		else
		{
			continue;
		}

		//-

		inputs[i] = vn; // prepare and feed the input with the normalized parameter

		outputs[i].update(inputs[i]); // raw value, index (optional)
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::updateEngine() {

	//TODO: crash when added other types than int/float
	//for (int i = 0; i < NUM_VARS && i < params_EditorEnablers.size(); i++)
	for (int i = 0; i < NUM_VARS; i++)
	{
		// toggle
		auto& p = params_EditorEnablers[i];// ofAbstractParameter
		bool isBool = (p.type() == typeid(ofParameter<bool>).name());
		if (!isBool) {
			ofLogError("ofxSurfingSmooth") << (__FUNCTION__) << "Skip to avoid crash!";
			continue;
		}

		bool _bSmooth = p.cast<bool>().get();

		// get input as source or clamped
		float _input;
		if (bClamp) _input = ofClamp(inputs[i], minInput, maxInput);
		else _input = inputs[i];

		// plot
		if (bGui_Plots) plots[2 * i]->update(_input);//source

		// output
		if (bGui_Plots)
		{
			if (bEnableSmooth && _bSmooth) plots[2 * i + 1]->update(outputs[i].getValue());//use filtered
			else plots[2 * i + 1]->update(_input);//use source
		}

		//TODO: ?
		if (i == index) input = _input;
	}

	//----

	// index of the selected param!

	// toggle
	int i = index;
	auto& _p = params_EditorEnablers[i];// ofAbstractParameter
	bool _bSmooth = _p.cast<bool>().get();

	// output. get the output as it is or normalized
	if (bEnableSmooth && _bSmooth)
	{
		if (bNormalized) output = outputs[index].getValueN();
		else output = outputs[index].getValue();
	}
	else // bypass
	{
		output = input;
	}

	//----

	//TODO:
	// Log bangs / onSets but for only the selected/index channel/param !
	// add individual thresholds
	// add callbacks notifiers
	for (int i = 0; i < NUM_VARS; i++)
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
}

//--------------------------------------------------------------
void ofxSurfingSmooth::updateGenerators() {
	if (!bUseGenerators) return;

	// run generators

	if (ofGetFrameNum() % 20 == 0) {
		if (ofRandom(0, 1) > 0.5) bTrigManual = !bTrigManual;
	}

	// NOTICe that fails when ignoring original params ranges!
	for (int g = 0; g < NUM_GENERATORS; g++) {
		switch (g) {
		case 0: generators[g] = (bTrigManual ? 0.7 : 0.1); break;
		case 1: generators[g] = ofxSurfingHelpers::Tick((bModeFast ? 0.2 : 1)); break;
		case 2: generators[g] = ofxSurfingHelpers::Noise(ofPoint((!bModeFast ? 1 : 0.001), (!bModeFast ? 1.3 : 2.3))); break;
		case 3: generators[g] = ofClamp(ofxSurfingHelpers::NextGaussian(0.5, (bModeFast ? 1 : 0.1)), 0.2, 0.8); break;
		case 4: generators[g] = ofxSurfingHelpers::NextReal(0, (bModeFast ? 1 : 0.1)); break;
		case 5: generators[g] = ofxSurfingHelpers::Noise(ofPoint((!bModeFast ? 1 : 0.00001), (!bModeFast ? 0.3 : 0.03))); break;
		}
	}
	//outputs[i].update(inputs[i]); // raw value, index (optional)

	//----

	// feed generators to parameters

	for (int i = 0; i < mParamsGroup.size(); i++)
	{
		//if (i > generators.size() - 1) continue;//skip

		ofAbstractParameter& aparam = mParamsGroup[i];

		//string str = "";
		//string name = aparam.getName();

		if (i < generators.size())
		{
			float value = 0;

			if (aparam.type() == typeid(ofParameter<int>).name()) {
				ofParameter<int> ti = aparam.cast<int>();
				value = ofMap(generators[i], 0, 1, ti.getMin(), ti.getMax());
				ti.set((int)value);
				//ti = (int)value;
			}
			else if (aparam.type() == typeid(ofParameter<float>).name()) {
				ofParameter<float> ti = aparam.cast<float>();
				value = ofMap(generators[i], 0, 1, ti.getMin(), ti.getMax());
				ti.set(value);
				//ti = value;
			}
			//else if (aparam.type() == typeid(ofParameter<bool>).name()) {
			//	ofParameter<bool> ti = aparam.cast<bool>();
			//	value = ofMap(ti, ti.getMin(), ti.getMax(), 0, 1);
			//	//ofLogNotice("ofxSurfingSmooth") <<(__FUNCTION__) << " " << ti.getName() << " : " << ti.get() << " : " << value;
			//}

			else {//skip
				continue;
			}

			inputs[i] = value; // prepare and feed input
		}
		//else continue;//by pass the other params

		//inputs[i] = value; // prepare and feed input
		outputs[i].update(inputs[i]); // raw value, index (optional)
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw() {
	if (!bGui_Global) return;
	//if (!bGui) return;

	if (bGui_Plots)
	{
		if (bPlotFullScreen) drawPlots(ofGetCurrentViewport());
		else
		{
			//fix
			if (0) {
				ofColor c;
				if (!bPlotIn) c = plots[0]->getBackgroundColor();
				ofPushStyle();
				ofFill();
				ofSetColor(c);
				ofDrawRectangle(boxPlots.getRectangle());
				ofPopStyle();
			}

			drawPlots(boxPlots.getRectangle());
		}

		boxPlots.draw();

		if (boxPlots.isEditing()) boxPlots.drawBorderBlinking(ofColor::yellow);
	}

	if (bGui) draw_ImGui();
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
void ofxSurfingSmooth::drawToggles() {
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

	ofPushStyle();

	int hh = r.getHeight();
	int ww = r.getWidth();
	int x = r.getX();
	int y = r.getY();

	//ofPushMatrix();
	//ofTranslate(x,y)

	int h;
	if (!bSolo)
	{
		h = hh / NUM_VARS; // multiplot height
	}
	else // bSolo
	{
		h = hh; // full height on bSolo
	}

	for (int i = 0; i < NUM_VARS; i++)
	{
		if (bSolo) if (i != index) continue;

		int ii = 2 * i;

		//// grid
		//int hg = h / 2;
		//plot[ii]->setGridUnit(hg);
		//plot[ii + 1]->setGridUnit(hg);

		if (bPlotIn) plots[ii]->draw(x, y, ww, h);
		if (bPlotOut) plots[ii + 1]->draw(x, y, ww, h);

		// baseline
		ofSetColor(colorBaseLine);
		ofSetLineWidth(1);
		ofLine(x, y + h, x + ww, y + h);

		// add labels

		string s;
		string sp;
		string _spacing = "\t";

		// # number

		if (!bSolo)  ofSetColor(colorTextSelected);
		else
		{
			if (i == index) ofSetColor(colorTextSelected);
			else ofSetColor(colorBaseLine);
		}

		s = "#" + ofToString(i);

		// add param name and value

		if (0)
		{
			int ip = i;
			if (ip < outputs.size())
			{
				ofAbstractParameter& p = mParamsGroup[ip];
				if (p.type() == typeid(ofParameter<int>).name()) {
					ofParameter<int> tp = p.cast<int>();
					int tmpRef = ofMap(outputs[ip].getValue(), 0, 1, tp.getMin(), tp.getMax());
					sp = tp.getName() + _spacing;
					sp += ofToString(tmpRef);
				}
				else if (p.type() == typeid(ofParameter<float>).name()) {
					ofParameter<float> tp = p.cast<float>();
					float tmpRef = ofMap(outputs[ip].getValue(), 0, 1, tp.getMin(), tp.getMax());
					sp = tp.getName() + _spacing;
					sp += ofToString(tmpRef, 2);
				}
			}
			s += " " + _spacing + sp;
		}

		// space
		s += " " + _spacing;
		_spacing = "";

		// add flags for trig, bonk, redirected
		if (outputs[i].getTrigger()) s += "+" + _spacing; // trigged
		else s += "-" + _spacing;
		if (isBonked(i)) s += "+" + _spacing; // bonked
		else s += "-" + _spacing;

		// redirected
		if (isRedirected(i)) s += "+" + _spacing;
		else s += "-" + _spacing;

		//if (isRedirectedTo(i) == 0) s += " " + _spacing; // redirected
		//else if (isRedirectedTo(i) < 0) s += "-" + _spacing; // redirected
		//else if (isRedirectedTo(i) > 0) s += "+" + _spacing; // redirected

		// latched
		static bool bDirectionLast = false;
		if (isRedirectedTo(i) < 0) bDirectionLast = false; // redirected
		else if (isRedirectedTo(i) > 0) bDirectionLast = true; // redirected
		if (bDirectionLast) s += ">"; // redirected
		else s += "<"; // redirected

		// display text
		ofDrawBitmapString(s, x + 5, y + 11);

		//--

		//ofColor _c1 = ofColor(colors[ii]);//colored
		//ofColor _c2 = ofColor(colors[ii]);

		ofColor _c0 = colorThreshold;
		ofColor _c1 = colorTrig;
		ofColor _c2 = colorBonk;
		ofColor _c3 = colorDirect;

		// alpha blink
		int _a0 = (int)ofMap(ofxSurfingHelpers::Bounce(0.5), 0, 1, 128, 200); // standby
		int _a1 = (int)ofMap(ofxSurfingHelpers::Bounce(0.5), 0, 1, 128, 200); // trigged
		int _a2 = (int)ofMap(ofxSurfingHelpers::Bounce(0.5), 0, 1, 128, 200); // bonked
		int _a3 = (int)ofMap(ofxSurfingHelpers::Bounce(0.5), 0, 1, 128, 200); // direct

		// threshold overlayed
		{
			int l = 3;
			ofColor c;

			if (isRedirected(i)) c.set(ofColor(_c3, _a3)); // redirected
			else if (isBonked(i)) {
				c.set(ofColor(_c2, _a2)); // bonked
				l = 6;
			}
			else if (outputs[i].getTrigger()) c.set(ofColor(_c1, _a1)); // trigged
			else
			{
				c.set(ofColor(_c0, _a0)); // standby
				l = 1;
			}

			// draw line
			ofSetLineWidth(l);
			ofSetColor(c);
			float yth = y + (1 - threshold) * h;
			ofLine(x, yth, x + ww, yth);
		}

		////mark selected left line
		//if (i == index && !bSolo)
		//{
		//	ofSetLineWidth(1);
		//	ofSetColor(colorTextSelected);
		//	ofLine(x, y, x, y + h);
		//}

		if (!bSolo) y += h;
	}

	//ofPopMatrix();
	ofPopStyle();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::keyPressed(int key)
{
	if (!guiManager.bKeys) return;

	if (key == 'g') bGui = !bGui;

	if (key == OF_KEY_RETURN) bPlay = !bPlay;
	if (key == ' ') doRandomize();

	if (key == 's') bSolo = !bSolo;

	if (key == OF_KEY_UP) {
		index--;
		index = ofClamp(index, index.getMin(), index.getMax());
	}
	if (key == OF_KEY_DOWN) {
		index++;
		index = ofClamp(index, index.getMin(), index.getMax());
	}

	//if (key == OF_KEY_RETURN) bTrigManual = !bTrigManual;
	//if (key == OF_KEY_RETURN) bModeNoise = !bModeNoise;

	//threshold
	if (key == '-') {
		threshold = threshold.get() - 0.05f;
		threshold = ofClamp(threshold, threshold.getMin(), threshold.getMax());
	}
	if (key == '+') {
		threshold = threshold.get() + 0.05f;
		threshold = ofClamp(threshold, threshold.getMin(), threshold.getMax());
	}

	//types
	if (key == OF_KEY_TAB) {
		nextTypeSmooth();
	}

	if (key == OF_KEY_LEFT_SHIFT) {
		nextTypeMean();
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setupParams() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	string name = "SMOOTH SURF";

	float _inputMinRange = 0;
	float _inputMaxRange = 1;
	float _outMinRange = 0;
	float _outMaxRange = 1;

	//--

	// params
	params.setName(name);
	params.add(index.set("index", 0, 0, 0));
	params.add(bSolo.set("SOLO", false));

	params.add(bGui);
	params.add(bGui_Inputs.set("INPUTS", true));
	params.add(bGui_Outputs.set("OUTPUTS", true));
	params.add(bGui_Extra);

	params.add(bGui_Plots.set("PLOTS", true));
	params.add(bPlotFullScreen.set("Full Screen", false));
	params.add(bPlotIn.set("Plot In", true));
	params.add(bPlotOut.set("Plot Out", true));

	params.add(bUseGenerators.set("Generators", false));
	params.add(bPlay.set("Play", false));
	params.add(playSpeed.set("Speed", 0.5, 0, 1));

	params.add(bEnableSmooth.set("SMOOTH", true));

	//--

	params.add(typeSmooth.set("Type Smooth", 0, 0, 2));
	params.add(typeMean.set("Type Mean", 0, 0, 2));
	params.add(smoothPower.set("Power", 0.2, 0.0, 1));
	params.add(threshold.set("Thresh", 0.5, 0.0, 1));
	params.add(timeRedirection.set("TimeDir", 0.5, 0.0, 1));
	params.add(slideMin.set("SlideIn", 0.2, 0.0, 1));
	params.add(slideMax.set("SlideOut", 0.2, 0.0, 1));
	params.add(onsetGrow.set("Grow", 0.1f, 0.0, 1));
	params.add(onsetDecay.set("Decay", 0.1, 0.0, 1));
	params.add(bReset.set("Reset", false));

	params.add(typeMean_Str.set(" ", ""));
	params.add(typeSmooth_Str.set(" ", ""));

	params.add(bClamp.set("Clamp", false));
	params.add(minInput.set("minIn", 0, _inputMinRange, _inputMaxRange));
	params.add(maxInput.set("maxIn", 1, _inputMinRange, _inputMaxRange));
	params.add(minOutput.set("minOut", 0, _outMinRange, _outMaxRange));
	params.add(maxOutput.set("maxOut", 1, _outMinRange, _outMaxRange));
	params.add(bNormalized.set("Normalized", false));

	//--

	params.add(input.set("INPUT", 0, _inputMinRange, _inputMaxRange));
	params.add(output.set("OUTPUT", 0, _outMinRange, _outMaxRange));

	//why recuired?
	params.add(guiManager.bHelp);
	params.add(guiManager.bMinimize);
	params.add(guiManager.bKeys);
	params.add(guiManager.bGameMode);

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
	typeSmooth_Str.setSerializable(false);
	typeMean_Str.setSerializable(false);
	bReset.setSerializable(false);
	input.setSerializable(false);
	output.setSerializable(false);
	//bPlay.setSerializable(false);

	ofAddListener(params.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Params); // setup()

	//--

	buildHelp();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::exit() {
	ofRemoveListener(params.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Params); // exit()
	ofRemoveListener(mParamsGroup.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Controls_Out);

	ofxSurfingHelpers::CheckFolder(path_Global);
	ofxSurfingHelpers::saveGroup(params, path_Settings);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::doReset() {
	ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__);

	bClamp = false;
	minInput = 0;
	maxInput = 1;
	minOutput = 0;
	maxOutput = 1;
	bNormalized = false;

	typeSmooth = 1;
	typeMean = 0;
	smoothPower = 0.2;
	threshold = 0.5;
	timeRedirection = 0.5;
	slideMin = 0.2;
	slideMax = 0.2;
	onsetGrow = 0.1;
	onsetDecay = 0.1;

	//--

	output = 0;
	playSpeed = 0.5;
}

//--------------------------------------------------------------
void ofxSurfingSmooth::Changed_Params(ofAbstractParameter& e)
{
	if (bDISABLE_CALLBACKS) return;

	string name = e.getName();

	if (name != input.getName() && name != output.getName() && name != "")
	{
		ofLogNotice("ofxSurfingSmooth") << (__FUNCTION__) << " : " << name << " : " << e;
	}

	if (0) {}

	else if (name == guiManager.bKeys.getName())
	{
		buildHelp();
		return;
	}

	else if (name == bEnableSmooth.getName())
	{
		return;
	}

	//--

	else if (name == bReset.getName())
	{
		if (bReset)
		{
			bReset = false;
			doReset();
		}
		return;
	}

	//--

	else if (name == threshold.getName())
	{
		for (int i = 0; i < NUM_VARS; i++) {
			outputs[i].setThresh(threshold);
		}
		return;
	}

	//--

	else if (name == smoothPower.getName())
	{
		int MAX_ACC_HISTORY = 60;//calibrated to 60fps
		float v = ofMap(smoothPower, 0, 1, 1, MAX_ACC_HISTORY);
		for (int i = 0; i < NUM_VARS; i++) {
			outputs[i].initAccum(v);
		}
		return;
	}

	//--

	else if (name == slideMin.getName() || name == slideMax.getName())
	{
		for (int i = 0; i < NUM_VARS; i++) {
			const int MIN_SLIDE = 1;
			const int MAX_SLIDE = 50;
			float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
			float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

			outputs[i].initSlide(_slmin, _slmax);
		}
		return;
	}

	//--

	else if (name == minOutput.getName() || name == maxOutput.getName())
	{
		for (int i = 0; i < NUM_VARS; i++) {
			outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));
		}
		return;
	}

	//--

	else if (name == bNormalized.getName())
	{
		for (int i = 0; i < NUM_VARS; i++)
		{
			//outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));

			if (bNormalized) outputs[i].setNormalized(bNormalized, ofVec2f(0, 1));
			else outputs[i].setNormalized(bNormalized, ofVec2f(minOutput, maxOutput));
		}
		return;
	}

	//--

	//TODO:
	//detect "bonks" (onsets):
	//amp.setBonk(0.1, 0.1);  // min growth for onset, min decay
	//set growth/decay:
	//amp.setDecayGrow(true, 0.99); // a framerate-dependent steady decay/growth
	else if (name == onsetGrow.getName() || name == onsetDecay.getName())
	{
		for (int i = 0; i < NUM_VARS; i++) {
			outputs[i].setBonk(onsetGrow, onsetDecay);
			//specAmps[i].setDecayGrow(true, 0.99);

			outputs[i].directionChangeCalculated = true;
			//outputs[i].setBonk(0.1, 0.0);
		}
		return;
	}

	//--

	else if (name == typeSmooth.getName())
	{
		typeSmooth = ofClamp(typeSmooth, typeSmooth.getMin(), typeSmooth.getMax());

		switch (typeSmooth)
		{
		case ofxDataStream::SMOOTHING_NONE:
		{
			if (!bEnableSmooth) bEnableSmooth = false;
			typeSmooth_Str = typeSmoothLabels[0];
			return;
		}
		break;

		case ofxDataStream::SMOOTHING_ACCUM:
		{
			if (!bEnableSmooth) bEnableSmooth = true;
			typeSmooth_Str = typeSmoothLabels[1];
			int MAX_HISTORY = 30;
			float v = ofMap(smoothPower, 0, 1, 1, MAX_HISTORY);
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].initAccum(v);
			}
			return;
		}
		break;

		case ofxDataStream::SMOOTHING_SLIDE:
		{
			if (!bEnableSmooth) bEnableSmooth = true;
			typeSmooth_Str = typeSmoothLabels[2];
			for (int i = 0; i < NUM_VARS; i++) {
				const int MIN_SLIDE = 1;
				const int MAX_SLIDE = 50;
				float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
				float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

				outputs[i].initSlide(_slmin, _slmax);
			}
			return;
		}
		break;
		}

		return;
	}

	//--

	else if (name == typeMean.getName())
	{
		typeMean = ofClamp(typeMean, typeMean.getMin(), typeMean.getMax());

		switch (typeMean)
		{
		case ofxDataStream::MEAN_ARITH:
		{
			typeMean_Str = typeMeanLabels[0];
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].setMeanType(ofxDataStream::MEAN_ARITH);
			}
			return;
		}
		break;

		case ofxDataStream::MEAN_GEOM:
		{
			typeMean_Str = typeMeanLabels[1];
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].setMeanType(ofxDataStream::MEAN_GEOM);
			}
			return;
		}
		break;

		case ofxDataStream::MEAN_HARM:
		{
			typeMean_Str = typeMeanLabels[2];
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].setMeanType(ofxDataStream::MEAN_HARM);
			}
			return;
		}
		break;
		}

		return;
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setup_ImGui()
{
	guiManager.setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	guiManager.setup();

	guiManager.addWindowSpecial(bGui);
	guiManager.addWindowSpecial(bGui_Extra);
	guiManager.addWindowSpecial(bGui_Inputs);
	guiManager.addWindowSpecial(bGui_Outputs);

	guiManager.startup();

	//--

	guiManager.setHelpInfoApp(helpInfo);
	guiManager.bHelpInternal = false;
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

			if (!guiManager.bMinimize)
			{
				if (guiManager.beginTree("PLOTS"))
				{
					guiManager.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
					if (bGui_Plots)
					{
						guiManager.Indent();
						guiManager.Add(bPlotIn, OFX_IM_TOGGLE_ROUNDED_SMALL);
						guiManager.Add(bPlotOut, OFX_IM_TOGGLE_ROUNDED_SMALL);
						guiManager.Indent();
						guiManager.Add(bPlotFullScreen, OFX_IM_TOGGLE_ROUNDED_MINI);
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
					guiManager.Add(bUseGenerators, OFX_IM_TOGGLE);

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
	if (guiManager.beginWindow(guiManager.bGameMode))
	{
		int i = index.get();

		guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
		guiManager.AddSpacingSeparated();

		//--

		guiManager.AddLabelHuge("SURF SMOOTH");
		guiManager.AddSpacingBigSeparated();

		//--

		guiManager.AddLabelBig("Signal");
		ofxImGuiSurfing::AddMatrixClicker(index);
		guiManager.AddSpacing();
		guiManager.Add(bSolo, OFX_IM_TOGGLE);
		guiManager.AddSpacing();

		guiManager.Add(smoothChannels[i]->ampInput, OFX_IM_KNOB, 2, true);
		guiManager.Add(smoothChannels[i]->threshold, OFX_IM_VSLIDER_NO_NUMBER);
		guiManager.AddSpacingBigSeparated();

		// main controls
		guiManager.AddTooltip(ofToString(smoothChannels[i]->threshold.get(), 2));
		guiManager.Add(smoothChannels[i]->smoothPower, OFX_IM_HSLIDER_NO_NUMBER);
		guiManager.AddTooltip(ofToString(smoothChannels[i]->smoothPower.get(), 2));
		guiManager.AddSpacingBigSeparated();

		//--

		guiManager.AddLabelBig("DETECTOR");

		guiManager.AddCombo(smoothChannels[i]->bangDetectorIndex, smoothChannels[i]->bangDetectors);
		if (smoothChannels[i]->bangDetectorIndex == 2 ||
			smoothChannels[i]->bangDetectorIndex == 3 ||
			smoothChannels[i]->bangDetectorIndex == 4)\
		{
			guiManager.Add(smoothChannels[i]->timeRedirection, OFX_IM_HSLIDER_SMALL);
		}
		if (smoothChannels[i]->bangDetectorIndex == 1)
		{
			guiManager.Add(smoothChannels[i]->onsetGrow, OFX_IM_STEPPER);
			guiManager.Add(smoothChannels[i]->onsetDecay, OFX_IM_STEPPER);
		}

		//--

		guiManager.AddSpacingBigSeparated();

		if (!guiManager.bMinimize)
		{

			if (guiManager.beginTree("PLOTS", false))
			{
				guiManager.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED);
				if (bGui_Plots) {
					guiManager.Indent();
					guiManager.Add(bPlotIn, OFX_IM_CHECKBOX);
					guiManager.Add(bPlotOut, OFX_IM_CHECKBOX);
					guiManager.Add(boxPlots.bEdit, OFX_IM_TOGGLE_ROUNDED_MINI);
					guiManager.Unindent();
				}
				guiManager.endTree();
			}
		}
		else
		{
			guiManager.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED);
		}

		//--

		guiManager.AddSpacingBigSeparated();

		if (guiManager.Add(smoothChannels[i]->bReset, OFX_IM_BUTTON_SMALL)) {

		};

		guiManager.endWindow();
	}
}

/*
// original mode
//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGuiMain()
{
	if (bGui)
	{
		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;

		if (guiManager.beginWindowSpecial(bGui))
		{
			guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_ROUNDED);
			guiManager.AddSpacingSeparated();

			// Main enable

			guiManager.Add(bEnableSmooth, OFX_IM_TOGGLE_BIG_BORDER_BLINK);

			// Monitor

			if (bEnableSmooth)
			{
				guiManager.AddSpacingSeparated();

				if (ImGui::CollapsingHeader("MONITOR"))
				{
					guiManager.refreshLayout();

					if (index < editorEnablers.size()) {
						guiManager.AddLabelBig(editorEnablers[index].getName(), false);
					}

					if (!guiManager.bMinimize)
					{
						if (index.getMax() < 5) {
							ofxImGuiSurfing::AddMatrixClicker(index);
						}
						else {
							if (guiManager.Add(index, OFX_IM_STEPPER))
							{
								index = ofClamp(index, index.getMin(), index.getMax());
							}
						}

						guiManager.Add(bSolo, OFX_IM_TOGGLE);

						//if (!guiManager.bMinimize)
						{
							guiManager.Add(input, OFX_IM_HSLIDER_MINI);
							guiManager.Add(output, OFX_IM_HSLIDER_MINI);
						}
					}
					else
					{
						if (index.getMax() < 5) {
							ofxImGuiSurfing::AddMatrixClicker(index);
						}
						else {
							if (guiManager.Add(index, OFX_IM_STEPPER))
							{
								index = ofClamp(index, index.getMin(), index.getMax());
							}
						}

						guiManager.Add(bSolo, OFX_IM_TOGGLE);
						guiManager.AddSpacingSeparated();

						guiManager.Add(input, OFX_IM_HSLIDER_MINI_NO_LABELS);
						guiManager.Add(output, OFX_IM_HSLIDER_MINI_NO_LABELS);
					}
				}

				guiManager.refreshLayout();
			}

			//--

			if (bEnableSmooth)
			{
				guiManager.AddSpacingSeparated();

				if (ImGui::CollapsingHeader("ENGINE"))
				{
					guiManager.AddSpacing();

					if (!guiManager.bMinimize)
						if (guiManager.AddButton("SMOOTH >", OFX_IM_BUTTON))
						{
							nextTypeSmooth();
						}

					ImGui::PushID("##CONVO1");
					guiManager.AddCombo(typeSmooth, typeSmoothLabels);
					ImGui::PopID();

					if (typeSmooth == ofxDataStream::SMOOTHING_ACCUM)
					{
						guiManager.Add(smoothPower, OFX_IM_HSLIDER_SMALL);
					}

					if (typeSmooth == ofxDataStream::SMOOTHING_SLIDE)
					{
						guiManager.Add(slideMin, OFX_IM_HSLIDER_MINI);
						guiManager.Add(slideMax, OFX_IM_HSLIDER_MINI);
					}

					guiManager.AddSpacingSeparated();

					//--

					if (!guiManager.bMinimize)
						if (guiManager.AddButton("MEAN >", OFX_IM_BUTTON))
						{
							nextTypeMean();
						}

					ImGui::PushID("##CONVO2");
					guiManager.AddCombo(typeMean, typeMeanLabels);
					ImGui::PopID();

					if (!guiManager.bMinimize)
					{
						guiManager.AddSpacingSeparated();
						guiManager.Add(bClamp, OFX_IM_TOGGLE_SMALL, 2, true);
						guiManager.Add(bNormalized, OFX_IM_TOGGLE_SMALL, 2);
						if (bClamp) {
							guiManager.Add(minInput);
							guiManager.Add(maxInput);
							guiManager.AddSpacing();
							guiManager.Add(minOutput);
							guiManager.Add(maxOutput);
							guiManager.AddSpacing();
						}
					}

					//if (!guiManager.bMinimize)
					{
						guiManager.AddSpacingSeparated();

						guiManager.Add(bReset, OFX_IM_BUTTON_SMALL);
					}
				}
			}

			//--

			if (bEnableSmooth)
			{
				guiManager.AddSpacingSeparated();

				if (ImGui::CollapsingHeader("DETECTOR"))
				{
					guiManager.refreshLayout();

					//guiManager.AddCombo(bangDetectorIndex, bangDetectors);

					if (!guiManager.bMinimize)
					{
						guiManager.Add(threshold, OFX_IM_HSLIDER_SMALL);
						guiManager.Add(threshold, OFX_IM_STEPPER);
						guiManager.AddSpacing();
						guiManager.AddLabel("Direction");
						guiManager.Add(timeRedirection, OFX_IM_STEPPER);
						guiManager.AddLabel("Bonks");
						guiManager.Add(onsetGrow, OFX_IM_STEPPER);
						guiManager.Add(onsetDecay, OFX_IM_STEPPER);
					}
					else
					{
						//if (bangDetectorIndex == 2 || bangDetectorIndex == 3 || bangDetectorIndex == 4)
						{
							guiManager.Add(timeRedirection, OFX_IM_HSLIDER_SMALL);
						}

						//if (bangDetectorIndex == 1)
						{
							guiManager.Add(onsetGrow, OFX_IM_STEPPER);
							guiManager.Add(onsetDecay, OFX_IM_STEPPER);
						}
					}

					guiManager.AddSpacingBigSeparated();

					if (guiManager.Add(bReset, OFX_IM_BUTTON_SMALL)) {
						//signal_0_PreAmp = 0;
						//signal_1_PreAmp = 0;
					};

					//if (guiManager.bMinimize)
					//{
					//	guiManager.Add(threshold, OFX_IM_HSLIDER_SMALL);
					//	guiManager.AddSpacing();
					//	guiManager.Add(timeRedirection, OFX_IM_STEPPER);
					//	guiManager.Add(onsetGrow, OFX_IM_STEPPER);
					//	guiManager.Add(onsetDecay, OFX_IM_STEPPER);
					//}
					//else
					//{
					//	guiManager.Add(threshold, OFX_IM_HSLIDER_SMALL);
					//	guiManager.Add(threshold, OFX_IM_STEPPER);
					//	guiManager.AddSpacing();
					//	guiManager.AddLabel("Direction");
					//	guiManager.Add(timeRedirection, OFX_IM_STEPPER);
					//	guiManager.AddLabel("Bonks");
					//	guiManager.Add(onsetGrow, OFX_IM_STEPPER);
					//	guiManager.Add(onsetDecay, OFX_IM_STEPPER);
					//}
				}
				guiManager.refreshLayout();
			}

			if (!guiManager.bMinimize)
			{
				guiManager.AddSpacingSeparated();
				guiManager.Add(bGui_Extra, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
			}
		}

		guiManager.endWindowSpecial();
	}
}
*/

// new mode
//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGuiMain()
{
	if (bGui)
	{
		int i = index.get();

		IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;

		if (guiManager.beginWindowSpecial(bGui))
		{
			guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_ROUNDED);
			guiManager.AddSpacingSeparated();

			guiManager.Add(guiManager.bGameMode, OFX_IM_TOGGLE_ROUNDED_BIG);
			guiManager.AddSpacing();

			// Main enable
			guiManager.Add(bEnableSmooth, OFX_IM_TOGGLE_BIG_BORDER_BLINK);

			// Monitor
			if (bEnableSmooth)
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
						if (index.getMax() < 5) {
							ofxImGuiSurfing::AddMatrixClicker(index);
						}
						else {
							if (guiManager.Add(index, OFX_IM_STEPPER))
							{
								index = ofClamp(index, index.getMin(), index.getMax());
							}
						}

						guiManager.Add(bSolo, OFX_IM_TOGGLE);
						guiManager.Add(input, OFX_IM_HSLIDER_MINI);
						guiManager.Add(output, OFX_IM_HSLIDER_MINI);
					}
					else
					{
						if (index.getMax() < 5) {
							ofxImGuiSurfing::AddMatrixClicker(index);
						}
						else {
							if (guiManager.Add(index, OFX_IM_STEPPER))
							{
								index = ofClamp(index, index.getMin(), index.getMax());
							}
						}

						guiManager.Add(bSolo, OFX_IM_TOGGLE);
						guiManager.AddSpacingSeparated();

						guiManager.Add(input, OFX_IM_HSLIDER_MINI_NO_LABELS);
						guiManager.Add(output, OFX_IM_HSLIDER_MINI_NO_LABELS);
					}
				}

				guiManager.refreshLayout();
			}

			//--

			if (bEnableSmooth)
			{
				guiManager.AddSpacingSeparated();

				if (ImGui::CollapsingHeader("ENGINE"))
				{
					guiManager.AddSpacing();

					if (!guiManager.bMinimize)
						if (guiManager.AddButton("SMOOTH >", OFX_IM_BUTTON))
						{
							nextTypeSmooth();
						}

					guiManager.AddCombo(smoothChannels[i]->typeSmooth, typeSmoothLabels);

					if (typeSmooth == ofxDataStream::SMOOTHING_ACCUM)
					{
						guiManager.Add(smoothChannels[i]->smoothPower, OFX_IM_HSLIDER_SMALL);
					}

					if (typeSmooth == ofxDataStream::SMOOTHING_SLIDE)
					{
						guiManager.Add(smoothChannels[i]->slideMin, OFX_IM_HSLIDER_MINI);
						guiManager.Add(smoothChannels[i]->slideMax, OFX_IM_HSLIDER_MINI);
					}

					guiManager.AddSpacingSeparated();

					//--

					if (!guiManager.bMinimize)
						if (guiManager.AddButton("MEAN >", OFX_IM_BUTTON))
						{
							nextTypeMean();
						}

					guiManager.AddCombo(smoothChannels[i]->typeMean, typeMeanLabels);

					if (!guiManager.bMinimize)
					{
						guiManager.AddSpacingSeparated();
						guiManager.Add(smoothChannels[i]->bClamp, OFX_IM_TOGGLE_SMALL, 2, true);
						guiManager.Add(smoothChannels[i]->bNormalized, OFX_IM_TOGGLE_SMALL, 2);
						if (smoothChannels[i]->bClamp)
						{
							guiManager.Add(smoothChannels[i]->minInput);
							guiManager.Add(smoothChannels[i]->maxInput);
							guiManager.AddSpacing();
							guiManager.Add(smoothChannels[i]->minOutput);
							guiManager.Add(smoothChannels[i]->maxOutput);
							guiManager.AddSpacing();
						}
					}

					guiManager.AddSpacingSeparated();
					guiManager.Add(smoothChannels[i]->bReset, OFX_IM_BUTTON_SMALL);
				}
			}

			//--

			if (bEnableSmooth)
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
	guiManager.begin();
	{
		if (guiManager.bGameMode) draw_ImGuiGameMode();

		//--

		if (bGui) draw_ImGuiMain();

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
		//nested groups

		for (int i = 0; i < g.size(); i++)
		{
			addParam(g.get(i));
		}
	}

	// add/queue each param
	// exclude groups to remove from plots
	if (!isGroup) mParamsGroup.add(aparam);

	//--

	// create a copy group
	// will be the output 
	// or what, once smoothed, we will use target to be use params

	if (isFloat)
	{
		ofParameter<float> p = aparam.cast<float>();
		ofParameter<float> _p{ _name + suffix, p.get(), p.getMin(), p.getMax() };
		mParamsGroup_Smoothed.add(_p);

		//-

		ofParameter<bool> _b{ _name, true };
		editorEnablers.push_back(_b);
		params_EditorEnablers.add(_b);

		//-

		int i = params_EditorEnablers.size() - 1;
		smoothChannels.push_back(make_unique<SmoothChannel>());
		smoothChannels[i]->setup("Ch" + ofToString(i));
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

		//-

		int i = params_EditorEnablers.size() - 1;
		smoothChannels.push_back(make_unique<SmoothChannel>());
		smoothChannels[i]->setup("Ch" + ofToString(i));
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

		//-

		int i = params_EditorEnablers.size() - 1;
		smoothChannels.push_back(make_unique<SmoothChannel>());
		smoothChannels[i]->setup("Ch" + ofToString(i));
	}
	else {
	}

	//-

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
	//group COPY
	mParamsGroup_Smoothed.setName(n + "_SMOOTH");//name
	//mParamsGroup_Smoothed.setName(n + suffix);//name

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
	//NUM_VARS will be counted here..

	outputs.resize(NUM_VARS);
	inputs.resize(NUM_VARS);

	// default init

	for (int i = 0; i < NUM_VARS; i++)
	{
		outputs[i].initAccum(100);
		outputs[i].directionChangeCalculated = true;
		outputs[i].setBonk(onsetGrow, onsetDecay);
	}

	////--

	////TODO:
	//mParamsGroup_Smoothed.setName(aparams.getName() + "_COPY");//name
	//mParamsGroup_Smoothed = mParamsGroup;//this kind of copy links param per param. but we want to clone the "structure" only
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

//--------------------------------------------------------------
void ofxSurfingSmooth::Changed_Controls_Out(ofAbstractParameter& e)
{
	if (bDISABLE_CALLBACKS) return;

	std::string name = e.getName();

	ofLogVerbose("ofxSurfingSmooth") << (__FUNCTION__) << name << " : " << e;
}

//------------

// API getters

// to get the smoothed parameters indiviauly and externaly

//simplified getters
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
ofAbstractParameter& ofxSurfingSmooth::getParamAbstract(ofAbstractParameter& e) {
	string name = e.getName();
	auto& p = mParamsGroup.get(name);
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
ofAbstractParameter& ofxSurfingSmooth::getParamAbstract(string name) {
	auto& p = mParamsGroup.get(name);

	auto i = mParamsGroup.getPosition(name);
	float value = outputs[i].getValue();

	return p;
}

//--------------------------------------------------------------
ofParameter<float>& ofxSurfingSmooth::getParamFloat(string name) {
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
}

//--------------------------------------------------------------
float ofxSurfingSmooth::getParamFloatValue(ofAbstractParameter& e) {
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
int ofxSurfingSmooth::getParamIntValue(ofAbstractParameter& e) {
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
ofParameter<int>& ofxSurfingSmooth::getParamInt(string name) {
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