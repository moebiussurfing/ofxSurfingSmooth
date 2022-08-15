

#include "smoothChannel.h"


void SmoothChannel::setup(string _name)
{
	name = _name;
	string tag = name + "_";

	params.setName(name);

	params.add(typeSmooth.set(tag+"Type Smooth", 0, 0, 2));
	params.add(typeSmooth_Str.set(" ", ""));
	params.add(nameStr.set(name, ""));
	params.add(typeMean.set("Type Mean", 0, 0, 2));
	params.add(typeMean_Str.set(" ", ""));

	params.add(ampInput.set(tag + "Amp", 0, -10, 10));
	params.add(bangDetectorIndex.set("Detector", 0, 0, 4));

	params.add(smoothPower.set(tag + "Power", 0.2, 0.0, 1));
	params.add(threshold.set(tag + "Thresh", 0.5, 0.0, 1));
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

	params.add(bReset.set("Reset", false));

	doReset();

	//--

	typeMean_Str.setSerializable(false);
	typeSmooth_Str.setSerializable(false);
	bReset.setSerializable(false);

	path_Global = "ofxSurfingSmooth/";
	path_Settings = path_Global + "Ch_" + name + ".xml";

	ofxSurfingHelpers::loadGroup(params, path_Settings);

	//--

	soundEngine.setListener(&controller);
}
