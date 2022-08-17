#pragma once

#include "ofMain.h"
#include "ofxImGui.h"

class circleBeatWidget
{
public:


public:

	circleBeatWidget() {
		dt = 1.0f / 60.f;
	};

	~circleBeatWidget() {
	};

	void draw() {
		update();
		draw_ImGui_CircleBeatWidget();
	}

	void update() {
		animRunning = (animCounter <= 1.0f);//goes from 0 to 1 (finished)

		if (animRunning)
		{
			animCounter += speedRatio * speed * dt;
		}
	};

	void bang()
	{
		animCounter = 0.0f;//anim from 0.0 to 1.0

		bState = false;
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

		//stop
		if (!bState) animCounter = 1.0f;
	}

	void reset() {
		setState(false);
	}

	bool isSate() {
		return bState;
	}

	// 0=TrigState, 1=Bonk, 2=Direction, 3=DirUp, 4=DirDown
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
	float speedRatio = 6.0f;
	float speed = 0.5f;

	// 0=TrigState, 1=Bonk, 2=Direction, 3=DirUp, 4=DirDown
	ofColor color0 = ofColor::red;
	ofColor color1 = ofColor::blue;
	ofColor color2 = ofColor::green;
	ofColor color3 = ofColor::turquoise;
	ofColor color4 = ofColor::brown;

	int mode = 0;
	ofColor color = color0;

public:

	void draw_ImGui_CircleBeatWidget()
	{
		float radius = 30;

		// Big circle segments outperforms..
		const int nsegm = 24;

		ofColor colorBg = ofColor(16, 200);

		ofColor colorBeat = this->getColor();

		//---

		float pad = 10;
		float __w100 = ImGui::GetContentRegionAvail().x - 2 * pad;

		const char* label = " ";

		float radius_inner = radius * this->getValue() - 2; // 2 px inner

		float radius_outer = radius;

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		ImVec2 pos = ImGui::GetCursorScreenPos(); // get top left of current widget

		float xx = pos.x + pad;
		//float yy = pos.y + pad / 2;
		float yy = pos.y ;

		//spacing in units
		const int spcUnits = 3;
		//ImVec4 widgetRec = ImVec4(xx, yy, radius * 2.0f, radius * 2.0f + spcUnits * pad);

		ImVec4 widgetRec = ImVec4(xx, yy, radius * 2.0f, radius * 2.0f );

		//ImVec2 labelLength = ImGui::CalcTextSize(label);

		//ImVec2 center = ImVec2(
		//	pos.x + space_width + radius,
		//	pos.y + space_height * 2 + line_height + radius);

		//ImVec2 center = ImVec2(
		//	xx + radius,
		//	yy + radius);

		ImVec2 center = ImVec2(xx + __w100 / 2, yy + radius /*+ pad*/);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImGui::InvisibleButton(label, ImVec2(widgetRec.z, widgetRec.w));

		//-

		// Draw label

		//float texPos = pos.x + ((widgetRec.z - labelLength.x) * 0.5f);
		//draw_list->AddText(ImVec2(texPos, pos.y + space_height), ImGui::GetColorU32(ImGuiCol_Text), label);

		//-

		// Outer Circle / Bg

		draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImVec4(colorBg)), nsegm);

		//-

		// Inner Circle

		/*
		// highlight 1st beat
		ofColor c;
		if (Beat_current == 1) c = colorBeat;
		else c = colorTick;
		*/

		ofColor c = colorBeat;

		if (mode == 0) // blink
		{
			int a = ofMap(ofxSurfingHelpers::getFadeBlink(0.05), 0, 1, 110, 200);
			c = ofColor(colorBeat, a);
		}

		draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(ImVec4(c)), nsegm);
	}
};

