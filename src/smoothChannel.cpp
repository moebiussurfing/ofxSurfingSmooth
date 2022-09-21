
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

	params.add(onsetGrow.set("Grow", 0.1f, 0.001, 0.5));
	params.add(onsetDecay.set("Decay", 0.1, 0.001, 0.5));

	params.add(bClamp.set("Clamp", false));
	params.add(minInput.set("minIn", 0, 0, 1));
	params.add(maxInput.set("maxIn", 1, 0, 1));
	params.add(minOutput.set("minOut", 0, 0, 1));
	params.add(maxOutput.set("maxOut", 1, 0, 1));
	params.add(bNormalized.set("Normalized", false));

	params.add(bGateModeEnable);
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
	
	surfingPresets.setPath(pathGlobal);
	surfingPresets.AddGroup(params);
	//W_vReset.makeReferenceTo(surfingPresets.vReset);

}

void SmoothChannel::startup()
{
	ofLogNotice("SmoothChannel") << (__FUNCTION__);

	name_Settings = params.getName() + ".json";
	ofLogNotice("SmoothChannel") << "Load Settings for channel / param: " << name_Settings;
	ofxSurfingHelpers::loadGroup(params, path_Global + name_Settings);

	//bDoReFresh = true;

	//surfingPresets.startup();
}

//void SmoothChannel::update()
//{
//	//return;
//	/*
//	if (bDoReFresh)
//	{
//		cout << ofGetFrameNum() << endl;
//		bDoReFresh = false;
//		doRefresh();
//	}
//	*/
//}

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
	slideMin = 0.f;
	slideMax = 0.4f;
	onsetGrow = 0.015f;
	onsetDecay = 0.25f;
	timeRedirection = 0.1f;

	smoothPower = 0.1f;

	bGateModeEnable = false;
	bGateSlow = true;
	bpmDiv = 1;

	typeSmooth = 1;
	typeMean = 0;

	//bDoReFresh = true;
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

			return;
		}
		break;

		case SMOOTHING_ACCUM:
		{
			if (!bEnableSmooth) bEnableSmooth = true;
			typeSmooth_Str = typeSmoothLabels[1];

			return;
		}
		break;

		case SMOOTHING_SLIDE:
		{
			if (!bEnableSmooth) bEnableSmooth = true;
			typeSmooth_Str = typeSmoothLabels[2];

			return;
		}
		break;

		}

		return;
	}

	//--

	else if (name == typeMean.getName())
	{
		//int typeMean_ = typeMean;
		int _typeMean = ofClamp(typeMean, typeMean.getMin(), typeMean.getMax());
		typeMean.setWithoutEventNotifications(typeMean);

		switch (typeMean)
		{
		case MEAN_ARITH:
		{
			typeMean_Str = typeMeanLabels[0];

			return;
		}
		break;

		case MEAN_GEOM:
		{
			typeMean_Str = typeMeanLabels[1];

			return;
		}
		break;

		case MEAN_HARM:
		{
			typeMean_Str = typeMeanLabels[2];

			return;
		}
		break;
		}

		return;
	}
}

//----

// Fix workaround, for different related params and modes
// reduce by calling once per frame or make some bAttendintCalls flag..

void SmoothChannel::doRefresh()
{
	ofLogWarning("SmoothChannel") << (__FUNCTION__);
	ofLogWarning("SmoothChannel") << ("--------------------------------------------------------------");

	// To trig callbacks

	//--

	doRefreshMean();

	//--

	doRefreshSmooth();

	//--

	doRefreshDetector();

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

void SmoothChannel::doRefreshMean()
{
	ofLogWarning("SmoothChannel") << (__FUNCTION__);
	ofLogWarning("SmoothChannel") << ("--------------------------------------------------------------");

	if (typeMean == 0) {
	}
	else if (typeMean == 1) {
	}
	else if (typeMean == 2) {
	}

	typeMean = typeMean;
}

void SmoothChannel::doRefreshSmooth()
{
	ofLogWarning("SmoothChannel") << (__FUNCTION__);
	ofLogWarning("SmoothChannel") << ("--------------------------------------------------------------");

	if (typeSmooth == 0) {
	}
	else if (typeSmooth == 1) {
		smoothPower = smoothPower;
	}
	else if (typeSmooth == 2) {
		slideMin = slideMin;
		slideMax = slideMax;
	}
}

void SmoothChannel::doRefreshDetector()
{
	ofLogWarning("SmoothChannel") << (__FUNCTION__);
	ofLogWarning("SmoothChannel") << ("--------------------------------------------------------------");

	// state
	if (bangDetectorIndex == 0) { 
		threshold = threshold;
	}
	
	// bonk
	else if (bangDetectorIndex == 1) { 
		onsetGrow = onsetGrow;
		onsetDecay = onsetDecay;
	}

	// re direct
	else if ( 
		bangDetectorIndex == 2 ||
		bangDetectorIndex == 3 ||
		bangDetectorIndex == 4) {

		timeRedirection = timeRedirection;
	}
}