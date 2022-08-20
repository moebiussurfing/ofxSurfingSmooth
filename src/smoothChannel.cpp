
#include "smoothChannel.h"

void SmoothChannel::setup(string _name)
{
	name = _name + "_SmoothChannel";

	params.setName(name);

	params.add(index);//TODO:

	params.add(bEnableSmooth.set("ENABLE", true));
	params.add(ampInput.set("Amp", 0, -1, 1));

	params.add(typeSmooth.set("Type Smooth", 0, 0, 2));
	params.add(typeSmooth_Str.set(" ", ""));
	params.add(typeMean.set("Type Mean", 0, 0, 2));
	params.add(typeMean_Str.set(" ", ""));

	params.add(smoothPower.set("Smooth Power", 0.2f, 0.0f, 1.f));
	params.add(threshold.set("Threshold", 0.5f, 0.0, 1));

	params.add(timeRedirection.set("TimeDir", 0.5f, 0.01, 1));
	params.add(slideMin.set("SlideIn", 0.2f, 0.0, 1));
	params.add(slideMax.set("SlideOut", 0.2f, 0.0, 1));
	params.add(onsetGrow.set("Grow", 0.1f, 0.01, 1));
	params.add(onsetDecay.set("Decay", 0.1, 0.01, 1));

	params.add(bClamp.set("Clamp", false));
	params.add(minInput.set("minIn", 0, 0, 1));
	params.add(maxInput.set("maxIn", 1, 0, 1));
	params.add(minOutput.set("minOut", 0, 0, 1));
	params.add(maxOutput.set("maxOut", 1, 0, 1));
	params.add(bNormalized.set("Normalized", false));

	params.add(bGateMode);
	params.add(bGateSlow);
	params.add(bpmDiv);

	params.add(bReset.set("Reset", false));

	params.add(bangDetectorIndex.set("Detector", 0, 0, 4));

	//--

	doReset();

	//--

	typeMean_Str.setSerializable(false);
	typeSmooth_Str.setSerializable(false);
	bReset.setSerializable(false);

	//--

	/*
	soundEngine.setListener(&controller);
	*/
}

void SmoothChannel::startup()
{
	ofLogNotice("SmoothChannel") << (__FUNCTION__);

	name_Settings = params.getName();
	ofLogNotice("SmoothChannel") << "Load Settings for channel / param: "<< name_Settings;
	ofxSurfingHelpers::loadGroup(params, path_Global + name_Settings);

	doRefresh();
}

void SmoothChannel::exit()
{
	ofLogNotice("SmoothChannel") << (__FUNCTION__);

	ofxSurfingHelpers::CheckFolder(path_Global);
	ofxSurfingHelpers::saveGroup(params, path_Global + name_Settings);
}

void SmoothChannel::doReset()
{
	ofLogNotice("SmoothChannel") << (__FUNCTION__) << name;

	ampInput = 0.f;

	bangDetectorIndex = 0;

	bClamp = false;
	minInput = 0.f;
	maxInput = 1.f;
	minOutput = 0.f;
	maxOutput = 1.f;
	bNormalized = false;

	threshold = 0.75f;
	slideMin = 0.2f;
	slideMax = 0.2f;
	onsetGrow = 0.05f;
	onsetDecay = 0.05f;
	timeRedirection = 0.1f;

	smoothPower = 0.f;

	bGateMode = false;
	bGateSlow = true;
	bpmDiv = 1;

	typeSmooth = 1;
	typeMean = 0;

	doRefresh();
}

void SmoothChannel::Changed(ofAbstractParameter& e)
{
	std::string name = e.getName();

	ofLogNotice("SmoothChannel") << name << " : " << e;

	if (0) {}

	//--

	else if (name == bReset.getName())
	{
		if (bReset)
		{
			bReset = false;
			doReset();
		}
	}

	//--

	else if (name == typeSmooth.getName())
	{
		typeSmooth = ofClamp(typeSmooth, typeSmooth.getMin(), typeSmooth.getMax());

		switch (typeSmooth)
		{

		case SMOOTHING_NONE:
		{
			if (!bEnableSmooth) bEnableSmooth = false;
			typeSmooth_Str = typeSmoothLabels[0];
			doRefresh();
			return;
		}
		break;

		case SMOOTHING_ACCUM:
		{
			if (!bEnableSmooth) bEnableSmooth = true;
			typeSmooth_Str = typeSmoothLabels[1];
			doRefresh();
			return;
		}
		break;

		case SMOOTHING_SLIDE:
		{
			if (!bEnableSmooth) bEnableSmooth = true;
			typeSmooth_Str = typeSmoothLabels[2];
			doRefresh();
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
		case MEAN_ARITH:
		{
			typeMean_Str = typeMeanLabels[0];
			doRefresh();
			return;
		}
		break;

		case MEAN_GEOM:
		{
			typeMean_Str = typeMeanLabels[1];
			doRefresh();
			return;
		}
		break;

		case MEAN_HARM:
		{
			typeMean_Str = typeMeanLabels[2];
			doRefresh();
			return;
		}
		break;
		}

		return;
	}
}

// fix workaround, for different related params and modes
// reduce by calling once per frame or make some bAttendintCalls flag..

void SmoothChannel::doRefresh()
{
	ofLogWarning("SmoothChannel") << (__FUNCTION__);

	// to trig callbacks

	//--

	if (typeMean == 0) {
	}
	else if (typeMean == 1) {
	}
	else if (typeMean == 2) {
	}

	//--

	if (bangDetectorIndex == 0) {//state
		threshold = threshold;
	}
	else if (bangDetectorIndex == 1) {//bonk
		onsetGrow = onsetGrow;
		onsetDecay = onsetDecay;
	}
	else if (bangDetectorIndex == 2||bangDetectorIndex == 3||bangDetectorIndex == 4) {//re direct
		timeRedirection = timeRedirection;
	}

	//--

	if (typeSmooth == 0) {
	}
	else if (typeSmooth == 1) {
		smoothPower = smoothPower;
	}
	else if (typeSmooth == 2) {
		slideMin = slideMin;
		slideMax = slideMax;
	}

	//--

	//int typeSmooth_ = typeSmooth;
	//if (typeSmooth == 0) typeSmooth = 1;
	//else if (typeSmooth == 1) typeSmooth = 2;
	//else if (typeSmooth == 2) typeSmooth = 1;
	//typeSmooth = typeSmooth_;

	//float smoothPower_ = smoothPower;
	//smoothPower = smoothPower.getMax();
	//smoothPower = smoothPower_;
}