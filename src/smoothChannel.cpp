
#include "smoothChannel.h"

void SmoothChannel::setup(string _name)
{
	name = _name;

	params.setName(name);

	params.add(ampInput.set("Amp", 0, -10, 10));

	params.add(typeSmooth.set("Type Smooth", 0, 0, 2));
	params.add(typeSmooth_Str.set(" ", ""));
	params.add(typeMean.set("Type Mean", 0, 0, 2));
	params.add(typeMean_Str.set(" ", ""));

	params.add(smoothPower.set("Smooth Power", 0.2, 0.0, 1));
	params.add(threshold.set("Threshold", 0.5, 0.0, 1));

	params.add(timeRedirection.set("TimeDir", 0.5, 0.0, 1));
	params.add(slideMin.set("SlideIn", 0.2, 0.0, 1));
	params.add(slideMax.set("SlideOut", 0.2, 0.0, 1));
	params.add(onsetGrow.set("Grow", 0.1f, 0.0, 1));
	params.add(onsetDecay.set("Decay", 0.1, 0.0, 1));

	params.add(bClamp.set("Clamp", false));
	params.add(minInput.set("minIn", 0, 0, 1));
	params.add(maxInput.set("maxIn", 1, 0, 1));
	params.add(minOutput.set("minOut", 0, 0, 1));
	params.add(maxOutput.set("maxOut", 1, 0, 1));
	params.add(bNormalized.set("Normalized", false));

	params.add(bangDetectorIndex.set("Detector", 0, 0, 4));

	params.add(bReset.set("Reset", false));

	params.add(bEnableSmooth.set("ENABLE", true));

	//--

	doReset();

	//--

	typeMean_Str.setSerializable(false);
	typeSmooth_Str.setSerializable(false);
	bReset.setSerializable(false);

	path_Settings = path_Global + "Ch_" + name + ".xml";

	ofxSurfingHelpers::loadGroup(params, path_Settings);

	//--

	/*
	soundEngine.setListener(&controller);
	*/
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
			//int MAX_HISTORY = 30;
			//float v = ofMap(smoothPower, 0, 1, 1, MAX_HISTORY);
			//for (int i = 0; i < amountChannels; i++) {
			//	outputs[i].initAccum(v);
			//}
			return;
		}
		break;

		case SMOOTHING_SLIDE:
		{
			if (!bEnableSmooth) bEnableSmooth = true;
			typeSmooth_Str = typeSmoothLabels[2];
			//for (int i = 0; i < amountChannels; i++) {
			//	const int MIN_SLIDE = 1;
			//	const int MAX_SLIDE = 50;
			//	float _slmin = ofMap(slideMin, 0, 1, MIN_SLIDE, MAX_SLIDE, true);
			//	float _slmax = ofMap(slideMax, 0, 1, MIN_SLIDE, MAX_SLIDE, true);

			//	outputs[i].initSlide(_slmin, _slmax);
			//}
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
			//for (int i = 0; i < amountChannels; i++) {
			//	outputs[i].setMeanType(MEAN_ARITH);
			//}
			return;
		}
		break;

		case MEAN_GEOM:
		{
			typeMean_Str = typeMeanLabels[1];
			//for (int i = 0; i < amountChannels; i++) {
			//	outputs[i].setMeanType(MEAN_GEOM);
			//}
			return;
		}
		break;

		case MEAN_HARM:
		{
			typeMean_Str = typeMeanLabels[2];
			//for (int i = 0; i < amountChannels; i++) 
			//{
			//	outputs[i].setMeanType(MEAN_HARM);
			//}
			return;
		}
		break;
		}

		return;
	}
}
