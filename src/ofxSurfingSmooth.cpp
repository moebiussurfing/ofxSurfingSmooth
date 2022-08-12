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
	ofLogNotice() << __FUNCTION__;

	path_Global = "ofxSurfingSmooth/";
	path_Settings = path_Global + "ofxSurfingSmooth_Settings.xml";
	ofxSurfingHelpers::CheckFolder(path_Global);

	//-

	setupParams();

	//-

	enablersForParams.clear();// an enabler toggler for each param
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
	ofLogNotice() << __FUNCTION__;

	bDISABLE_CALLBACKS = false;

	//doReset();

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
	colorPlots = (ofColor::green);
#endif
	colorBaseLine = ofColor(255, 48);
	colorSelected = ofColor(255, 150);
	colorBonk = ofColor(ofColor::blue, 255);
	colorTrig = ofColor(ofColor::yellow, 255);

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

		bool b1 = (i % 2 == 0);//1st plot of each var. input
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
	ofColor c0(0, 90);
	//boxPlots.path
	boxPlots.bEditMode.setName("Edit Plots");
	boxPlots.setColorEditingHover(c0);
	boxPlots.setColorEditingMoving(c0);
	boxPlots.enableEdit();
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

	for (int i = 0; i < mParamsGroup.size(); i++) {
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
			auto pc = mParamsGroup_COPY.getFloat(_p.getName() + suffix);

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
			auto pc = mParamsGroup_COPY.getInt(_p.getName() + suffix);

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

		else {
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
		auto type = p.type();
		bool isBool = type == typeid(ofParameter<bool>).name();
		string name = p.getName();
		ofParameter<bool> _bSmooth = p.cast<bool>();
		//if (!_bSmooth) continue;//skip if it's disabled

		// input
		float _input = ofClamp(inputs[i], minInput, maxInput);
		if (bGui_Plots) plots[2 * i]->update(_input);//source

		// output
		if (bGui_Plots) {
			if (bEnableSmooth && _bSmooth) plots[2 * i + 1]->update(outputs[i].getValue());//filtered
			else plots[2 * i + 1]->update(_input);//source
		}

		if (i == index) input = _input;
	}

	//----

	// index of the selected param!

	// toggle
	int i = index;
	auto& _p = params_EditorEnablers[i];// ofAbstractParameter
	//auto type = _p.type();
	//bool isBool = type == typeid(ofParameter<bool>).name();
	//string name = _p.getName();
	bool _bSmooth = _p.cast<bool>().get();
	//ofParameter<bool> _bSmooth = _p.cast<bool>();
	//if (!_bSmooth) continue;//skip if it's disabled

	// output
	if (bEnableSmooth && _bSmooth)
	{
		if (bNormalized) output = outputs[index].getValueN();
		else output = outputs[index].getValue();
	}
	else //bypass
	{
		output = input;
	}

	//----

	//TODO:
	// Log bangs / onSets
	// add individual thresholds
	// add callbacks notifiers
	for (int i = 0; i < NUM_VARS; i++)
	{
		//if (i!=0)continue;

		if (outputs[i].getTrigger()) {
			if (i == index) ofLogVerbose() << "Trigger: " << i;
		}

		if (outputs[i].getBonk()) {
			if (i == index) ofLogVerbose() << "Bonk: " << i;
		}

		// if the direction has changed and
		// if the time of change is greater than 0.5 sec
		// print the time between changes and amount of change
		if (outputs[i].getDirectionTimeDiff() > 0.5f && outputs[i].directionHasChanged())
		{
			if (i == index)
				ofLogVerbose() << "Direction: " << i << " " << outputs[i].getDirectionTimeDiff() << " "
				<< outputs[i].getDirectionValDiff();
		}
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
			//	//ofLogNotice() << __FUNCTION__ << " " << ti.getName() << " : " << ti.get() << " : " << value;
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
	if (!bGui_Global || !bGui) return;

	if (bGui_Plots)
	{
		ofPushStyle();
		if (bFullScreenPlot) drawPlots(ofGetCurrentViewport());
		else drawPlots(boxPlots);

		ofSetColor(ofColor(255, 4));
		boxPlots.draw();
		ofPopStyle();
	}

	if (bGui) draw_ImGui();
}

//--------------------------------------------------------------
void ofxSurfingSmooth::doRandomize() {
	ofLogNotice(__FUNCTION__);

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
//	ofLogVerbose(__FUNCTION__) << index;
//
//	int i = index;
//
//	//for (auto p : enablersForParams)
//	//for (int i = 0; i<enablersForParams.size(); i++)
//	{
//		auto p = enablersForParams[i];
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
	for (int i = 0; i < enablersForParams.size(); i++)
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

		plots[ii]->draw(x, y, ww, h);
		plots[ii + 1]->draw(x, y, ww, h);

		// baseline
		ofSetColor(colorBaseLine);
		ofSetLineWidth(1);
		ofLine(x, y + h, x + ww, y + h);

		// add labels
		
		string s;
		string sp;
		string _spacing = "\t";

		// name
		if (!bSolo)  ofSetColor(colorSelected);
		else {
			if (i == index) ofSetColor(colorSelected);
			else ofSetColor(colorBaseLine);
		}
		s = "#" + ofToString(i);

		// add param name
		// add raw value
		int ip = i;
		if(ip< outputs.size())
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
		s += " " + _spacing;
		_spacing = "";

		// add flags for trig, bonk, redirected
		if (outputs[i].getTrigger()) s += "x" + _spacing; // trigged
		else s += " " + _spacing;
		if (isBonked(i)) s += "o" + _spacing; // bonked
		else s += " " + _spacing;

		if (isRedirectedTo(i) == 0) s += " " + _spacing; // redirected
		else if (isRedirectedTo(i) < 0) s += "-" + _spacing; // redirected
		else if (isRedirectedTo(i) > 0) s += "+" + _spacing; // redirected

		//latched
		//if (isRedirectedTo(i) < 0) bDirectionLast = false; // redirected
		//else if (isRedirectedTo(i) > 0) bDirectionLast = true; // redirected
		//if (bDirectionLast) s += "+"; // redirected
		//else s += "-"; // redirected

		// display text
		ofDrawBitmapString(s, x + 5, y + 11);

		//--

		ofColor _c1 = colorSelected;//monochrome
		ofColor _c2 = colorBonk;
		ofColor _c3 = colorTrig;

		//ofColor _c1 = ofColor(colors[ii]);//colored
		//ofColor _c2 = ofColor(colors[ii]);

		// alpha blink
		int _a0 = (int)ofMap(ofxSurfingHelpers::Bounce(0.5),0,1, 200, 255);// 
		float _a1 = MAX(0.5, ofxSurfingHelpers::Bounce(0.2));// bonked
		float _a2 = MAX(0.5, ofxSurfingHelpers::Bounce(0.2));// trigged

		// threshold
		{
			int l = 3;
			ofColor c;

			if (isRedirected(i)) c.set(ofColor(_c2, 200 * _a1)); // redirected
			else if (isBonked(i)) {
				c.set(ofColor(_c2, 255)); // bonked
				l = 5;
			}
			else if (outputs[i].getTrigger()) c.set(ofColor(_c3, 200 * _a2)); // trigged
			else 
			{
				c.set(ofColor(_c1, _a0)); // standby
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
		//	ofSetColor(colorSelected);
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
	ofLogNotice() << __FUNCTION__;

	string name = "SMOOTH SURF";

	//bool _bNormalized = true;
	float _inputMinRange;
	float _inputMaxRange;
	float _outMinRange;
	float _outMaxRange;

	//if (_bNormalized)//normalizad
	{
		_inputMinRange = 0;
		_inputMaxRange = 1;
		_outMaxRange = 1;
		_outMinRange = 0;
	}
	//else//midi range
	//{
	//    _inputMinRange = 0;
	//    _inputMaxRange = 1;
	//    _outMinRange = 0;
	//    _outMaxRange = 127;
	//}

	//params
	params.setName(name);
	params.add(index.set("index", 0, 0, 0));
	params.add(bPlay.set("Play", false));
	params.add(playSpeed.set("Speed", 0.5, 0, 1));
	params.add(bGui_Plots.set("PLOTS", true));
	params.add(bGui_Inputs.set("INPUTS", true));
	params.add(bGui_Outputs.set("OUTPUTS", true));
	params.add(bFullScreenPlot.set("Full Screen", false));
	params.add(bUseGenerators.set("Generators", false));
	params.add(bEnableSmooth.set("SMOOTH", true));
	params.add(bSolo.set("SOLO", false));
	params.add(minInput.set("minIn", 0, _inputMinRange, _inputMaxRange));
	params.add(maxInput.set("maxIn", 1, _inputMinRange, _inputMaxRange));
	params.add(minOutput.set("minOut", 0, _outMinRange, _outMaxRange));
	params.add(maxOutput.set("maxOut", 1, _outMinRange, _outMaxRange));
	params.add(bNormalized.set("Normalized", false));
	params.add(typeSmooth.set("Type Smooth", 0, 0, 2));
	params.add(typeSmooth_Str.set(" ", ""));
	params.add(typeMean.set("Type Mean", 0, 0, 2));
	params.add(typeMean_Str.set(" ", ""));
	params.add(smoothPower.set("Power", 0.25, 0.0, 1));
	params.add(slideMin.set("SlideIn", 0.2, 0.0, 1));
	params.add(slideMax.set("SlideOut", 0.2, 0.0, 1));
	params.add(onsetGrow.set("OnGrow", 0.f, 0.0, 1));
	params.add(onsetDecay.set("OnDecay", 0.1, 0.0, 1));
	params.add(threshold.set("Thresh", 0.7, 0.0, 1));
	params.add(timeRedirection.set("TimeDir", 0.5, 0.0, 1));
	params.add(bReset.set("Reset", false));
	//params.add(bClamp.set("CLAMP", true));

	params.add(input.set("INPUT", 0, _inputMinRange, _inputMaxRange));
	params.add(output.set("OUTPUT", 0, _outMinRange, _outMaxRange));

	params.add(bGui);

	params.add(guiManager.bHelp);
	params.add(guiManager.bMinimize);
	params.add(guiManager.bKeys);

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

	// exclude
	//bUseGenerators.setSerializable(false);//fails if enabled
	//input.setSerializable(false);
	//output.setSerializable(false);
	//bPlay.setSerializable(false);
	typeSmooth_Str.setSerializable(false);
	typeMean_Str.setSerializable(false);
	bReset.setSerializable(false);

	ofAddListener(params.parameterChangedE(), this, &ofxSurfingSmooth::Changed_Params); // setup()

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
	ofLogNotice(__FUNCTION__);

	//enable = true;
	//bPlay = false;
	//bUseGenerators = false;
	//bGui_Plots = true;
	//bGui_Inputs = true;
	//bGui_Outputs = true;
	//bEnableSmooth = true;
	minInput = 0;
	maxInput = 1;
	minOutput = 0;
	maxOutput = 1;
	slideMin = 0.2;
	slideMax = 0.2;
	onsetGrow = 0.0;
	onsetDecay = 0.1;
	output = 0;
	bNormalized = false;
	smoothPower = 0.75;
	typeSmooth = 1;
	typeMean = 0;
	bClamp = true;
	threshold = 0.7;
	timeRedirection = 0.5;
	playSpeed = 0.5;

	//--

}

// callback for a parameter group
//--------------------------------------------------------------
void ofxSurfingSmooth::Changed_Params(ofAbstractParameter& e)
{
	if (bDISABLE_CALLBACKS) return;

	string name = e.getName();

	if (name != input.getName() && name != output.getName() && name != "")
	{
		ofLogNotice() << __FUNCTION__ << " : " << name << " : " << e;
	}

	if (0) {}

	else if (name == bReset.getName())
	{
		if (bReset)
		{
			bReset = false;
			doReset();
		}
	}

	else if (name == guiManager.bKeys.getName())
	{
		buildHelp();
	}

	else if (name == threshold.getName())
	{
		for (int i = 0; i < NUM_VARS; i++) {
			outputs[i].setThresh(threshold);
		}
	}

	else if (name == minOutput.getName() || name == maxOutput.getName())
	{
		for (int i = 0; i < NUM_VARS; i++) {
			outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));
		}
	}

	else if (name == bNormalized.getName())
	{
		for (int i = 0; i < NUM_VARS; i++) {
			//outputs[i].setOutputRange(ofVec2f(minOutput, maxOutput));

			if (bNormalized) outputs[i].setNormalized(bNormalized, ofVec2f(0, 1));
			else outputs[i].setNormalized(bNormalized, ofVec2f(minOutput, maxOutput));
		}
	}

	else if (name == smoothPower.getName())
	{
		int MAX_ACC_HISTORY = 60;//calibrated to 60fps
		float v = ofMap(smoothPower, 0, 1, 1, MAX_ACC_HISTORY);
		for (int i = 0; i < NUM_VARS; i++) {
			outputs[i].initAccum(v);
		}

		//if (typeSmooth != ofxDataStream::SMOOTHING_ACCUM) typeSmooth = ofxDataStream::SMOOTHING_ACCUM;
	}

	else if (name == bEnableSmooth.getName())
	{
		//if (!typeSmooth) typeSmooth = 1;
	}

	else if (name == slideMin.getName() || name == slideMax.getName())
	{
		for (int i = 0; i < NUM_VARS; i++) {
			const int MIN_SLIDE = 1;
			const int MAX_SLIDE = 50;
			float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
			float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

			outputs[i].initSlide(_slmin, _slmax);

			//if (typeSmooth != ofxDataStream::SMOOTHING_SLIDE) typeSmooth = ofxDataStream::SMOOTHING_SLIDE;
		}
	}

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
	}

	else if (name == typeSmooth.getName())
	{
		typeSmooth = ofClamp(typeSmooth, typeSmooth.getMin(), typeSmooth.getMax());

		switch (typeSmooth)
		{
		case ofxDataStream::SMOOTHING_NONE:
		{
			if (!bEnableSmooth) bEnableSmooth = false;
			typeSmooth_Str = typeSmoothLabels[0];
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
		}
		break;
		}
	}

	//-

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
		}
		break;

		case ofxDataStream::MEAN_GEOM:
		{
			typeMean_Str = typeMeanLabels[1];
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].setMeanType(ofxDataStream::MEAN_GEOM);
			}
		}
		break;

		case ofxDataStream::MEAN_HARM:
		{
			typeMean_Str = typeMeanLabels[2];
			for (int i = 0; i < NUM_VARS; i++) {
				outputs[i].setMeanType(ofxDataStream::MEAN_HARM);
			}
		}
		break;
		}
	}
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setup_ImGui()
{
	guiManager.setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	guiManager.setup();

	guiManager.addWindowSpecial(bGui);
	guiManager.addWindowSpecial(bGui_Inputs);
	guiManager.addWindowSpecial(bGui_Outputs);

	guiManager.startup();

	//--

	guiManager.setHelpInfoApp(helpInfo);
	guiManager.bHelpInternal = false;
}

//--------------------------------------------------------------
void ofxSurfingSmooth::draw_ImGui()
{
	guiManager.begin();
	{
		if (bGui)
		{
			IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;

			if (guiManager.beginWindowSpecial(bGui))
			{
				guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_ROUNDED);
				if (!guiManager.bMinimize)
				{
				guiManager.AddSpacingSeparated();
					guiManager.AddLabelBig("PANELS");

					guiManager.Add(bGui_Inputs, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
					guiManager.Add(bGui_Outputs, OFX_IM_TOGGLE_ROUNDED_MEDIUM);
					guiManager.AddSpacingSeparated();

					guiManager.Add(bGui_Plots, OFX_IM_TOGGLE_ROUNDED);
					guiManager.Add(bFullScreenPlot, OFX_IM_TOGGLE_ROUNDED_SMALL);
				}

				//----

				// Enable Toggles

				if (!guiManager.bMinimize)
				{
					guiManager.AddSpacingSeparated();

					bool bOpen;
					ImGuiColorEditFlags _flagc;

					bOpen = false;
					_flagc = (bOpen ? ImGuiWindowFlags_NoCollapse : ImGuiWindowFlags_None);
					if (ImGui::CollapsingHeader("ENABLERS", _flagc))
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
					}

					guiManager.AddSpacingSeparated();
				}

				if (!guiManager.bMinimize)
				{
					if (ImGui::CollapsingHeader("TESTER"))
					{
						guiManager.refreshLayout();

						guiManager.Add(bUseGenerators, OFX_IM_TOGGLE);

						//if (!bUseGenerators)
						{
							if (guiManager.AddButton("Randomizer", OFX_IM_BUTTON))
							{
								doRandomize();
							}

							//TODO:
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
						}
					}
				}

				guiManager.AddSpacingSeparated();

				// Main enable
				guiManager.Add(bEnableSmooth, OFX_IM_TOGGLE_BIG_BORDER_BLINK);
				guiManager.AddSpacingSeparated();

				//--

				if (bEnableSmooth)
				{
					if (ImGui::CollapsingHeader("ENGINE"))
					{
						//guiManager.AddLabelBig("ENGINE");
						if (!guiManager.bMinimize)
							if (guiManager.AddButton("> Smooth", OFX_IM_BUTTON))
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
							if (guiManager.AddButton("> Mean", OFX_IM_BUTTON))
							{
								nextTypeMean();
							}

						ImGui::PushID("##CONVO2");
						guiManager.AddCombo(typeMean, typeMeanLabels);
						ImGui::PopID();

						if (!guiManager.bMinimize)
						{
							guiManager.AddSpacingSeparated();
							if (ImGui::TreeNodeEx("Clamp", ImGuiTreeNodeFlags_None))
							{
								guiManager.refreshLayout();

								guiManager.Add(minInput);
								guiManager.Add(maxInput);
								guiManager.AddSpacing();
								guiManager.Add(minOutput);
								guiManager.Add(maxOutput);

								guiManager.Add(bNormalized, OFX_IM_TOGGLE_SMALL);

								ImGui::TreePop();
							}
						}

						//if (!guiManager.bMinimize) 
						{
							guiManager.AddSpacingSeparated();
							guiManager.Add(bReset, OFX_IM_BUTTON_SMALL);
						}
					}
				}

				// monitor
				//if (!guiManager.bMinimize)
				{
					if (bEnableSmooth)
					{
						guiManager.AddSpacingSeparated();

						if (ImGui::CollapsingHeader("MONITOR"))
						{
							guiManager.refreshLayout();

							if (index < enablersForParams.size()) {
								guiManager.AddLabelBig(enablersForParams[index].getName(), false);
							}
							if (guiManager.Add(index, OFX_IM_STEPPER))
							{
								index = ofClamp(index, index.getMin(), index.getMax());
							}

							guiManager.Add(bSolo, OFX_IM_TOGGLE);

							if (!guiManager.bMinimize)
							{
								guiManager.Add(input);
							}
							guiManager.Add(output);
						}
					}
				}

				//--

				if (bEnableSmooth)
				{
					guiManager.AddSpacingSeparated();

					if (ImGui::CollapsingHeader("DETECTORS"))
					{
						guiManager.refreshLayout();

						if (guiManager.bMinimize)
						{
							guiManager.Add(threshold, OFX_IM_HSLIDER_SMALL);
							guiManager.Add(threshold, OFX_IM_STEPPER);
							guiManager.AddSpacing();
							guiManager.Add(timeRedirection, OFX_IM_STEPPER);
							guiManager.Add(onsetGrow);
							guiManager.Add(onsetDecay);
						}
						else
							//if (ImGui::TreeNodeEx("OnSets", ImGuiTreeNodeFlags_DefaultOpen))
							{
								//guiManager.refreshLayout();

								guiManager.AddLabel("Trigger");
								guiManager.Add(threshold, OFX_IM_HSLIDER_SMALL);
								guiManager.Add(threshold, OFX_IM_STEPPER);
								guiManager.AddSpacing();
								guiManager.AddLabel("Direction");
								guiManager.Add(timeRedirection, OFX_IM_STEPPER);
								guiManager.AddLabel("Bonks");
								guiManager.Add(onsetGrow);
								guiManager.Add(onsetDecay);

								//ImGui::TreePop();
							}
					}
				}

				//--

				if (!guiManager.bMinimize)
				{
					guiManager.AddSpacingSeparated();

					if (ImGui::CollapsingHeader("Advanced"))
					{
						guiManager.refreshLayout();

						guiManager.Add(guiManager.bKeys);
						guiManager.Add(guiManager.bHelp);
						guiManager.Add(boxPlots.bEditMode);
					}
				}
			}

			guiManager.endWindowSpecial();
		}

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
				guiManager.AddGroup(mParamsGroup_COPY);

				guiManager.endWindowSpecial();
			}
		}

	}
	guiManager.end();
}

//----

//--------------------------------------------------------------
void ofxSurfingSmooth::addParam(ofAbstractParameter& aparam) {

	string _name = aparam.getName();
	ofLogNotice() << __FUNCTION__ << " [ ofAbstractParameter ] \t " << _name;

	//--

	//TODO:
	//nested groups doing recursive

	// https://forum.openframeworks.cc/t/ofxparametercollection-manage-multiple-ofparameters/34888/3
	auto type = aparam.type();

	bool isGroup = type == typeid(ofParameterGroup).name();
	bool isFloat = type == typeid(ofParameter<float>).name();
	bool isInt = type == typeid(ofParameter<int>).name();
	bool isBool = type == typeid(ofParameter<bool>).name();

	ofLogNotice() << __FUNCTION__ << " " << _name << " \t [ " << type << " ]";

	if (isGroup)
	{
		auto& g = aparam.castGroup();

		////TODO:
		//group COPY 
		//nested groups
		//string n = g.getName();
		//ofParameterGroup gc{ n + suffix };
		//mParamsGroup_COPY.add(gc);

		for (int i = 0; i < g.size(); i++) {
			addParam(g.get(i));
		}
	}

	// add/queue each param
	// exclude groups to remove from plots
	if (!isGroup) mParamsGroup.add(aparam);

	//--

	// create a copy group
	// will be the output or target to be use params

	if (isFloat) {
		ofParameter<float> p = aparam.cast<float>();
		ofParameter<float> _p{ _name + suffix, p.get(), p.getMin(), p.getMax() };
		mParamsGroup_COPY.add(_p);

		//-

		ofParameter<bool> b0{ _name, true };
		enablersForParams.push_back(b0);
		params_EditorEnablers.add(b0);
	}
	else if (isInt) {
		ofParameter<int> p = aparam.cast<int>();
		ofParameter<int> _p{ _name + suffix, p.get(), p.getMin(), p.getMax() };
		mParamsGroup_COPY.add(_p);

		//-

		ofParameter<bool> b0{ _name, true };
		enablersForParams.push_back(b0);
		params_EditorEnablers.add(b0);
	}
	else if (isBool) {
		ofParameter<bool> p = aparam.cast<bool>();
		ofParameter<bool> _p{ _name + suffix, p.get() };
		mParamsGroup_COPY.add(_p);

		//-

		ofParameter<bool> b0{ _name, true };
		enablersForParams.push_back(b0);
		params_EditorEnablers.add(b0);
	}
	else {
	}

	//-

	params.add(params_EditorEnablers);


	//auto mac = make_shared<ofxSurfingSmooth::MidiParamAssoc>();
	//mac->paramIndex = mParamsGroup.size();
	////ofLogWarning() << __FUNCTION__ << " ";
	//if (aparam.type() == typeid(ofParameter<int>).name()) {
	//	mac->ptype = PTYPE_INT;
	//	ofParameter<int> ti = aparam.cast<int>();
	//	ofParameterGroup pgroup = ti.getFirstParent();
	//	if (pgroup) {
	//		mac->xmlParentName = pgroup.getEscapedName();
	//	}
	//}
	//else if (aparam.type() == typeid(ofParameter<float>).name()) {
	//	mac->ptype = PTYPE_FLOAT;
	//	ofParameter<float> fi = aparam.cast<float>();
	//	ofParameterGroup pgroup = fi.getFirstParent();
	//	if (pgroup) {
	//		mac->xmlParentName = pgroup.getEscapedName();
	//	}
	//}
	//else if (aparam.type() == typeid(ofParameter<bool>).name()) {
	//	mac->ptype = PTYPE_BOOL;
	//	ofParameter<bool> bi = aparam.cast<bool>();
	//	ofParameterGroup pgroup = bi.getFirstParent();
	//	if (pgroup) {
	//		mac->xmlParentName = pgroup.getEscapedName();
	//	}
	//}
	//if (mac->ptype == PTYPE_UNKNOWN) {
	//	//ofLogNotice("ofxMidiParams :: addParam : unsupported param type");
	//	return;
	//}
	//mac->xmlName = aparam.getEscapedName();
	//mParamsGroup.add(aparam);
	//mAssocParams.push_back(mac);
}

//--------------------------------------------------------------
void ofxSurfingSmooth::setup(ofParameterGroup& aparams) {

	ofLogNotice() << __FUNCTION__ << " " << aparams.getName();

	setup();

	//--

	string n = aparams.getName();
	mParamsGroup.setName(n);//name

	//TODO:
	//group COPY
	mParamsGroup_COPY.setName(n + "_SMOOTH");//name
	//mParamsGroup_COPY.setName(n + suffix);//name

	for (int i = 0; i < aparams.size(); i++) {
		addParam(aparams.get(i));
	}

	//--

	// already added all params content
	// build the smoothers
	// build the plots

	setupPlots();//NUM_VARS will be counted here..

	outputs.resize(NUM_VARS);
	inputs.resize(NUM_VARS);

	// default init
	for (int i = 0; i < NUM_VARS; i++)
	{
		outputs[i].initAccum(100);
		outputs[i].directionChangeCalculated = true;
		//outputs[i].setBonk(0.1, 0.0);
		outputs[i].setBonk(onsetGrow, onsetDecay);
	}

	////--

	////TODO:
	//mParamsGroup_COPY.setName(aparams.getName() + "_COPY");//name
	//mParamsGroup_COPY = mParamsGroup;//this kind of copy links param per param. but we want to clone the "structure" only
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

	ofLogVerbose(__FUNCTION__) << name << " : " << e;
}

//------------

// API getters

// to get the smoothed parameters indiviauly and externaly

//simplified getters
//--------------------------------------------------------------
float ofxSurfingSmooth::get(ofParameter<float>& e) {
	string name = e.getName();
	auto& p = mParamsGroup_COPY.get(name);
	if (p.type() == typeid(ofParameter<float>).name())
	{
		return p.cast<float>().get();
	}
	else
	{
		ofLogError(__FUNCTION__) << "Not expected type: " << name;
		return -1;
	}
}
//--------------------------------------------------------------
int ofxSurfingSmooth::get(ofParameter<int>& e) {
	string name = e.getName();
	auto& p = mParamsGroup_COPY.get(name);
	if (p.type() == typeid(ofParameter<int>).name())
	{
		return p.cast<int>().get();
	}
	else
	{
		ofLogError(__FUNCTION__) << "Not expected type: " << name;
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
	//ofLogNotice(__FUNCTION__) << aparam.getName() << " : " << e;

	return p;
}

int ofxSurfingSmooth::getIndex(ofAbstractParameter& e) /*const*/
{
	string name = e.getName();

	auto& p = mParamsGroup.get(name);
	int i = mParamsGroup.getPosition(name);

	return i;

	//TODO: should verify types? e.g to skip bools?

	//auto& p = mParamsGroup_COPY.get(name);
	//auto i = mParamsGroup_COPY.getPosition(name);
	//if (p.type() == typeid(ofParameter<float>).name()) {
	//	ofParameter<float> pf = p.cast<float>();
	//	return pf;
	//}
	//else
	//{
	//	ofParameter<float> pf{ "empty", -1 };
	//	ofLogError(__FUNCTION__) << "Not expected type: " << name;
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

	auto& p = mParamsGroup_COPY.get(name);
	auto i = mParamsGroup_COPY.getPosition(name);
	if (p.type() == typeid(ofParameter<float>).name()) {
		ofParameter<float> pf = p.cast<float>();
		return pf;
	}
	else
	{
		ofParameter<float> pf{ "empty", -1 };
		ofLogError(__FUNCTION__) << "Not expected type: " << name;
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

	auto& p = mParamsGroup_COPY.get(name);
	auto i = mParamsGroup_COPY.getPosition(name);
	if (p.type() == typeid(ofParameter<float>).name()) {
		ofParameter<float> pf = p.cast<float>();
		return pf.get();
	}
	else
	{
		ofLogError(__FUNCTION__) << "Not expected type: " << name;
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

	auto& p = mParamsGroup_COPY.get(name);
	auto i = mParamsGroup_COPY.getPosition(name);
	if (p.type() == typeid(ofParameter<int>).name()) {
		ofParameter<int> pf = p.cast<int>();
		return pf.get();
	}
	else
	{
		ofLogError(__FUNCTION__) << "Not expected type: " << name;
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

	auto& p = mParamsGroup_COPY.get(name);
	auto i = mParamsGroup_COPY.getPosition(name);
	if (p.type() == typeid(ofParameter<int>).name()) {
		ofParameter<int> pf = p.cast<int>();
		return pf;
	}
	else
	{
		ofParameter<int> pf{ "empty", -1 };
		ofLogError(__FUNCTION__) << "Not expected type: " << name;
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
	ofLogNotice(__FUNCTION__) << b;

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