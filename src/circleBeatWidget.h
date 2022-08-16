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
		//update();
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
		}
	}

	void setColor(int i, ofColor color) {
		if (i > 4) return;
		switch (i)
		{
		case 0: color0=color; break;
		case 1: color1=color; break;
		case 2: color2=color; break;
		case 3: color3=color; break;
		case 4: color4=color; break;
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
		//TODO:
		// Convert to a new ImGui widget
		// Circle widget

		float radius = 30;



		// Big circle segments outperforms..
		//const int nsegm = 4;
		const int nsegm = 24;

		ofColor colorBeat = this->getColor();

		//ofColor colorBeat = ofColor(ofColor::red, 200);
		//ofColor colorTick = ofColor(128, 200);
		ofColor colorBg= ofColor(16, 200);
		//ofColor colorBallTap = ofColor(16, 200);
		//ofColor colorBallTap2 = ofColor(96);

		//---

		float pad = 10;
		float __w100 = ImGui::GetContentRegionAvail().x - 2 * pad;

		//float radius = __w100 / 2; // *this->getRadius();

		const char* label = " ";


		//TODO:
		//float radius_inner = radius * 1;
		float radius_inner = radius * this->getValue() - 2;
		//float radius_inner = radius * ofxSurfingHelpers::getFadeBlink();


		float radius_outer = radius;
		//float spcx = radius * 0.1;

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		ImVec2 pos = ImGui::GetCursorScreenPos(); // get top left of current widget

		//float line_height = ImGui::GetTextLineHeight();
		//float space_height = radius * 0.1; // to add between top, text, knob, value and bottom
		//float space_width = radius * 0.1; // to add on left and right to diameter of knob

		float xx = pos.x + pad;
		float yy = pos.y + pad / 2;

		//ImVec4 widgetRec = ImVec4(
		//	pos.x,
		//	pos.y,
		//	radius * 2.0f + space_width * 2.0f,
		//	space_height * 4.0f + radius * 2.0f + line_height * 2.0f);

		const int spcUnits = 3;

		ImVec4 widgetRec = ImVec4(
			xx,
			yy,
			radius * 2.0f,
			radius * 2.0f + spcUnits * pad);

		//ImVec2 labelLength = ImGui::CalcTextSize(label);

		//ImVec2 center = ImVec2(
		//	pos.x + space_width + radius,
		//	pos.y + space_height * 2 + line_height + radius);

		//ImVec2 center = ImVec2(
		//	xx + radius,
		//	yy + radius);

		ImVec2 center = ImVec2(
			xx + __w100 / 2,
			yy + radius + pad);

		//yy + __w100 / 2);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImGui::InvisibleButton(label, ImVec2(widgetRec.z, widgetRec.w));

		//bool value_changed = false;
		//bool is_active = ImGui::IsItemActive();
		//bool is_hovered = ImGui::IsItemActive();
		//if (is_active && io.MouseDelta.x != 0.0f)
		//{
		//	value_changed = true;
		//}

		//-

		//// Draw label

		//float texPos = pos.x + ((widgetRec.z - labelLength.x) * 0.5f);
		//draw_list->AddText(ImVec2(texPos, pos.y + space_height), ImGui::GetColorU32(ImGuiCol_Text), label);

		//-

		//ofColor cbg;
		////cbg = this->getColor();
		//cbg = colorBallTap;

		/*
		// Background black ball
		if (!bpmTapTempo.isRunning())
		{
			// ball background when not tapping
			cbg = colorBallTap;
		}
		else
		{
			// white alpha fade when measuring tapping
			float t = (ofGetElapsedTimeMillis() % 1000);
			float fade = sin(ofMap(t, 0, 1000, 0, 2 * PI));
			ofLogVerbose(__FUNCTION__) << "fade: " << fade << endl;
			int alpha = (int)ofMap(fade, -1.0f, 1.0f, 0, 50) + 205;
			cbg = ofColor(colorBallTap2.r, colorBallTap2.g, colorBallTap2.b, alpha);
		}
		*/

		//-

		// Outer Circle

		draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImVec4(colorBg)), nsegm);
		//draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : ImGuiCol_FrameBg), nsegm);
		//draw_list->AddCircleFilled(center, radius_outer * 0.8, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : ImGuiCol_FrameBg), nsegm);

		//-

		// Inner Circle

		/*
		// highlight 1st beat
		ofColor c;
		if (Beat_current == 1) c = colorBeat;
		else c = colorTick;
		*/

		ofColor c = colorBeat;

		draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(ImVec4(c)), nsegm);

		//draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_ButtonActive : is_hovered ? ImGuiCol_ButtonHovered : ImGuiCol_SliderGrab), nsegm);
		//draw_list->AddCircleFilled(center, radius_inner * 0.8, ImGui::GetColorU32(is_active ? ImGuiCol_ButtonActive : is_hovered ? ImGuiCol_ButtonHovered : ImGuiCol_SliderGrab), nsegm);

		//// draw value
		//char temp_buf[64];
		//sprintf(temp_buf, "%.2f", *p_value);
		//labelLength = ImGui::CalcTextSize(temp_buf);
		//texPos = pos.x + ((widgetRec.z - labelLength.x) * 0.5f);
		//draw_list->AddText(ImVec2(texPos, pos.y + space_height * 3 + line_height + radius * 2), ImGui::GetColorU32(ImGuiCol_Text), temp_buf);

		//-

		// Border Arc Progress

		//TODO:
		/*
		float control = 0;

		if (bMode_Internal_Clock) control = ofMap(Beat_current, 0, 4, 0, 1);
		else if (bMode_External_MIDI_Clock) control = ofMap(Beat_current, 0, 4, 0, 1);
#ifdef USE_ofxAbletonLink
		else if (bMODE_AbletonLinkSync) control = ofMap(LINK_Phase.get(), LINK_Phase.getMin(), LINK_Phase.getMax(), 0.0f, 1.0f);
#endif
		const int padc = 0;
		static float _radius = radius_outer + padc;
		static int num_segments = 35;
		static float startf = 0.0f;
		static float endf = IM_PI * 2.0f;
		static float offsetf = -IM_PI / 2.0f;
		static float _thickness = 3.0f;
		ofColor cb = ofColor(c.r, c.g, c.b, c.a * 0.6f);
		draw_list->PathArcTo(center, _radius, startf + offsetf, control * endf + offsetf, num_segments);
		draw_list->PathStroke(ImGui::ColorConvertFloat4ToU32(cb), ImDrawFlags_None, _thickness);
		*/
	}
};

