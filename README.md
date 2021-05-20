# Overview
An **openFrameworks** add-on to do different styles of timed **smoothing** to grouped ```ofParameters```.

## Based on the ofxDataStream Engine  
This add-on is extremely based on:  
https://github.com/turowskipaul/ofxDataStream  
Copyright (C) 2015, Paul Turowski. ( http://paulturowski.com )  

## Screencast
![gif](docs/readme_images/ofxSurfingSmooth.gif?raw=true "gif"){:height="50%" width="50%"}

## Features
- Just pass your ```ofParameterGroup``` container.
- Another smoothed ```ofParameterGroup``` will be created with the same structure.
- 2 Smoothing algorithms: **Accumulator** and **Slide**.
- 3 Mean types: **Arith**, **Geom** and **Harm**.
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
```

**ofApp.cpp**
```.cpp
void ofApp::setup() {
  params.setName("paramsGroup");// main container
  params.add(lineWidth.set("lineWidth", 0.5, 0, 1));
  params.add(separation.set("separation", 50, 1, 100));
  params.add(speed.set("speed", 0.5, 0, 1));

  surfingSmooth.setup(params);
}

void ofApp::update() {
 // we can get the smoothed params doing different approaches:

	// 1. just the values
	int _shapeType = surfingSmooth.getParamIntValue(shapeType);
	int _amount = surfingSmooth.getParamIntValue(amount);

 // 2. the parameter itself
	ofParameter<float> _lineWidth = surfingSmooth.getParamFloat(lineWidth.getName());
	ofParameter<float> _separation = surfingSmooth.getParamFloat(separation.getName());
 
 // 3. the whole group
	auto &group = surfingSmooth.getParamsSmoothed();
 
 // 4. as ofAbstractParameter to be casted after
	auto &ap = surfingSmooth.getParamAbstract(lineWidth);
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
* [ofxWindowApp](https://github.com/moebiussurfing/ofxWindowApp)  [ Only for example-Advanced ]  
* [ofxMidiParams](https://github.com/moebiussurfing/ofxMidiParams)  [ FORK | Only for example-Advanced ]  

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
+ Add "Real" nested sub-groups tree levels. Now the params are recreated on one depth level.
+ Add colors types, vectors, using templates...
+ Add thresholds/onSet/bonk independent for each variable/channel: make it functional. Add callbacks...
+ Add param to calibrate max history smooth/speed.

## Authors
Original Author:  
Paul Turowski. http://paulturowski.com  
Thanks @**turowskipaul** !  

An add-on [updated] by **@moebiusSurfing**  
*( ManuMolina ) 2019-2021*  

[Twitter](https://twitter.com/moebiussurfing/)  
[Instagram](https://www.instagram.com/moebiussurfing/)  
[YouTube](https://www.youtube.com/channel/UCzUw96_wjmNxyIoFXf84hQg)  
