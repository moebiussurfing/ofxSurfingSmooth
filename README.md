# ofxSurfingSmooth

## Overview
An **openFrameworks** add-on to do different styles of timed **smoothing** to grouped ```ofParameters```.

## ofxDataStream engine based  
This add-on is extremely based on:  
https://github.com/turowskipaul/ofxDataStream  
Copyright (C) 2015, Paul Turowski. (http://paulturowski.com)  

**ofxSurfingSmooth** is just a kind of helper with the **ofxDataStream** engine, ```ofParameters``` bridge, plottings, easy integration workflow, GUI, and settings management.  

## Screencast 
<img src="docs/readme_images/ofxSurfingSmooth.gif" width="80%" height="80%">

## Features
- Just pass your ```ofParameterGroup``` container.
- Another smoothed ```ofParameterGroup``` will be created with the same structure.
- 2 Smoothing algorithms: **Accumulator** and **Slide**.
- 3 Mean types: **Arithmetic**, **Geometric** and **Harmonic**.
- Only ```float``` and ```int``` types yet.
- Scalable and draggable plots.
- Auto Store/Recall all the settings.
- **ImGui** based GUI ready to integrate.

## Usage
 
**ofApp.h**
```.cpp
#include "ofxSurfingSmooth.h"

ofxSurfingSmooth surfingSmooth;

ofParameterGroup params;
ofParameter<float> lineWidth;
ofParameter<float> separation;
ofParameter<float> speed;
ofParameter<int> amount;
ofParameter<int> shapeType;
```

**ofApp.cpp**
```.cpp
void ofApp::setup() 
{
	params.setName("paramsGroup");// main container
	params.add(lineWidth.set("lineWidth", 0.5, 0.0, 1.0));
	params.add(separation.set("separation", 50.0, 1.0, 100.0));
	params.add(speed.set("speed", 0.5, 0.0, 1.0));
	params.add(amount.set("amount", 1, 1, 10));
	params.add(speed.set("shapeType", 0, 0, 3));

	surfingSmooth.setup(params);
}

void ofApp::update() 
{
	// we can get the smoothed params using different approaches:

	// 1. just the param values
	int _shapeType = surfingSmooth.getParamIntValue(shapeType);
	int _amount = surfingSmooth.getParamIntValue(amount);
	float _speed = surfingSmooth.getParamFloatValue(speed);
	
	// 2. the parameter itself
	ofParameter<int> _amount = surfingSmooth.getParamInt(amount.getName());
	ofParameter<float> _lineWidth = surfingSmooth.getParamFloat(lineWidth.getName());
	ofParameter<float> _separation = surfingSmooth.getParamFloat(separation.getName());

	// 3. the ofAbstractParameter
	ofAbstractParameter& _lineWidth = surfingSmooth.getParamAbstract(lineWidth);
	// to be casted to his correct type after
	
	// 4. the whole ofParameterGroup
	ofParameterGroup& _paramsSmoothed = surfingSmooth.getParamsSmoothed();
	// requires more work after, like iterate the group content, get a param by name...etc.
}
```

*Look on the **example-Basic** for more helping snippets to access the smoothed parameters on the newly re-created smoothed group.*  


<details>
  <summary>Dependencies</summary>
  <p>

Clone these add-ons and include into the **OF PROJECT GENERATOR** to allow compile your projects or the examples:
* [ofxHistoryPlot](https://github.com/moebiussurfing/ofxHistoryPlot)  [ FORK ]
* [ofxScaleDragRect](https://github.com/moebiussurfing/ofxScaleDragRect)  [ FORK ]
* [ofxImGui](https://github.com/moebiussurfing/ofxImGui)  [ FORK ]  
* [ofxSurfingHelpers](https://github.com/moebiussurfing/ofxSurfingHelpers)  
* [ofxWindowApp](https://github.com/moebiussurfing/ofxWindowApp)  [ Only for **example-Advanced** ]  
* [ofxMidiParams](https://github.com/moebiussurfing/ofxMidiParams)  [ FORK | **Only for example-Advanced** ]  

*Thanks a lot to all these ofxAddons coders.*  
  </p>
</details>

<details>
  <summary>Tested Systems</summary>
  <p>

  - **Windows 10** / **VS 2017** / **OF ~0.11**
  </p>
</details>

### TODO
+ Add more types: 2D/3D/4D vectors, colors. Using templates [?] ...  
  [ _**ANY HELP/PULL ON THIS IS REALLY APPRECIATED!**_ ]
+ Add "Real" nested sub-groups tree levels. Now the params are recreated on one depth level only. This could help when duplicated names too.
+ Add independent thresholds/onSet for each parameter/channel: make it functional. Add callbacks...
+ Add param to calibrate max history smooth/speed.

#### ALTERNATIVE
There's another more powerful filtering add-on that you can check too:  
https://github.com/bensnell/ofxFilter

## Authors
Original Author:  
Paul Turowski. http://paulturowski.com  
Thanks @**turowskipaul** !  

An add-on [updated] by **@moebiusSurfing**  
*( ManuMolina ) 2019-2021*  

[Twitter](https://twitter.com/moebiussurfing/)  
[Instagram](https://www.instagram.com/moebiussurfing/)  
[YouTube](https://www.youtube.com/channel/UCzUw96_wjmNxyIoFXf84hQg)  
