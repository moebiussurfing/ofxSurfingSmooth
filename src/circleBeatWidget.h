#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "surfingTimers.h"

/*

An ImGui widget to visualize trig bangs for an audio analyzer.

*/

//--

class CircleBeatWidget
{
public:

	CircleBeatWidget::CircleBeatWidget()
	{
		dt = 1.0f / 60.f;
	};

	CircleBeatWidget::~CircleBeatWidget()
	{
	};

	void draw()
	{
		update();
		draw_ImGui_CircleBeatWidget();
	};

private:

	float dt_Bpm = 0.f;
	bool bBpmMode = false;

	void update() {
		animRunning = (animCounter <= 1.0f);//goes from 0 to 1 (finished)

		if (animRunning)
		{
			if (!bBpmMode) animCounter += (dt * speedRatio * speed);
			else animCounter += dt_Bpm;
		}
	};

	string name = "";

public:

	ofParameter<float> bpm{ "Bpm", -1, 40.f, 240.f };
	ofParameter<int> div{ "Bpm Div", 2, 1, 4 };

	void setBpm(float _bpm, float _fps = 60)
	{
		bBpmMode = true;
		dt = 1.0f / _fps;

		if (bpm != _bpm)
			bpm = _bpm;

		int barDur = 60000 / bpm;// one bar duration in ms
		dt_Bpm = ((barDur * div) / 1000.f) * dt;

		//animCounter goes from 0 to 1
		//if (animRunning) animCounter += speedRatio * speed * dt;
	}

	void bang()
	{
		animCounter = 0.0f;//anim from 0.0 to 1.0

		bState = false;
	}

	void bang(int mode)
	{
		setMode(mode);
		bang();
	}

	float getValue()
	{
		float f;
		if (bState) return 1.f;
		f = ofClamp(1.0f - animCounter, 0.f, 1.f);
		return f;
	}

	void setToggleState() {
		bState = !bState;
		setState(bState);
	}

	void setState(bool state) {
		bState = state;

		// stop
		if (!bState) animCounter = 1.0f;
	}

	void reset() {
		setState(false);
	}

	bool isSate() {
		return bState;
	}

	// i.e.
	// 0=TrigState, 1=Bonk, 2=Direction, 3=DirUp, 4=DirDown
	// different modes can handle different colors, speeds or release modes.
	void setMode(int i) {
		mode = i;
		switch (mode)
		{
		case 0: color = color0; break;
		case 1: color = color1; break;
		case 2: color = color2; break;
		case 3: color = color3; break;
		case 4: color = color4; break;

		default://out of range.
			color = color0;
			mode = 0;
			break;
		}
	}

	void setName(string _name) {
		name = _name;
	}

	void setColor(ofColor color) {
		setColor(0, color);
		setMode(0);
	}

	void setColor(int i, ofColor color) {
		if (i > 4) return;
		switch (i)
		{
		case 0: color0 = color; break;
		case 1: color1 = color; break;
		case 2: color2 = color; break;
		case 3: color3 = color; break;
		case 4: color4 = color; break;
		}
	}

	ofColor getColor() {
		ofColor c = ofColor(color, ofMap(animCounter, 1, 0, 200, 128));
		return c;
	}

private:

	bool bState = false;

	float dt;
	float animCounter;
	bool animRunning;
	float speedRatio = 3.0f;
	float speed = 0.5f;

	// 0=TrigState, 1=Bonk, 2=Direction, 3=DirUp, 4=DirDown
	ofColor color0 = ofColor::red;
	ofColor color1 = ofColor::blue;
	ofColor color2 = ofColor::green;
	ofColor color3 = ofColor::turquoise;
	ofColor color4 = ofColor::brown;

	int mode = 0;
	ofColor color = color0;

	void draw_ImGui_CircleBeatWidget()
	{
		string t = "##" + name;
		ImGui::PushID(t.c_str());


		float radius = 30;
		ofColor colorCircle = this->getColor();
		
		static ofColor colorBg;

		auto* colors = ImGui::GetStyle().Colors;
		auto base = colors[ImGuiCol_FrameBgActive];
		auto active = colors[ImGuiCol_FrameBg];
		auto hovered = colors[ImGuiCol_FrameBgHovered];

		//---

		float pad = 10;
		float width = ImGui::GetContentRegionAvail().x - 2 * pad;


		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		//TODO:

		//draw_list->AddText(ImVec2(0, 0), ImU32(255), "myLabel", "myLabel");

		////TODO: add label
		////ImGui::Text(name.c_str());
		//auto title_size = ImGui::CalcTextSize(name.c_str(), NULL, false, width);

		//// Center title
		//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - title_size[0]) * 0.5f);
		//
		//ImGui::Text("%s", name.c_str());
		//ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);


		//// Common case
		//const float wrap_width = wrap_enabled ? CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f;
		//const ImVec2 text_size = CalcTextSize(text_begin, text_end, false, wrap_width);

		//ImRect bb(text_pos, text_pos + text_size);
		//ItemSize(text_size, 0.0f);
		//if (!ItemAdd(bb, 0))
		//	//return;

		//// Render (we don't hide text after ## in this end-user function)
		//RenderTextWrapped(ImVec2(0,0), "hola", "tu", 100);


		const char* label = "label";

		float radius_inner = radius * this->getValue() - 2; // 2 px inner
		float radius_outer = radius;


		ImVec2 pos = ImGui::GetCursorScreenPos(); // get top left of current widget


		float xx = pos.x + pad;
		float yy = pos.y;

		ImVec2 center = ImVec2(xx + width / 2.f, yy + radius);

		//TODO: position widget correctly to allow correct hover...
		//ImGui::SetCursorPos(ImVec2(center.x, 0));

		// make the free space
		ImVec4 widgetRec = ImVec4(xx, yy, radius * 2.0f, radius * 2.0f);
		ImGui::InvisibleButton(label, ImVec2(widgetRec.z + radius * 2.0f, widgetRec.w));//fix
		//ImGui::InvisibleButton(label, ImVec2(widgetRec.z, widgetRec.w));

		// assign color
		bool is_active = ImGui::IsItemActive();
		bool is_hovered = ImGui::IsItemHovered();
		colorBg = is_hovered ? hovered : active;

		//-

		// TODO: 
		// NOTICED that big circle that segments outperforms a bit. lower fps..
		const int nsegm = 24;

		// Outer Circle / Bg
		draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImVec4(colorBg)), nsegm);

		// Inner Circle
		ofColor c = colorCircle;

		// blink alpha on that mode
		if (mode == 0)
		{
			int a = ofMap(ofxSurfingHelpers::getFadeBlink(0.05), 0, 1, 160, 190);
			c = ofColor(colorCircle, a);
		}

		draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(ImVec4(c)), nsegm);

		// Add a beauty inner shadow border
		float thickness = 2.f;
		float r = 1 + radius_inner - (thickness / 2.f);
		c = ofColor(ofColor::black, 90);
		draw_list->AddCircle(center, r, ImGui::GetColorU32(ImVec4(c)), nsegm, thickness);


		ImGui::PopID();
	}
};

