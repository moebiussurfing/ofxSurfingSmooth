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
- Just pass your ```ofParameterGroup``` parameters container.
- Another smoothed ```ofParameterGroup``` will be created with the same parameters structure.
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

ofxSurfingSmooth data;

ofParameterGroup params; // main container
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
	params.setName("paramsGroup");
	params.add(lineWidth.set("lineWidth", 0.5, 0.0, 1.0));
	params.add(separation.set("separation", 50.0, 1.0, 100.0));
	params.add(speed.set("speed", 0.5, 0.0, 1.0));
	params.add(amount.set("amount", 1, 1, 10));
	params.add(speed.set("shapeType", 0, 0, 3));

	data.setup(params);
}

void ofApp::update() 
{
  // we can get the smoothed params using different approaches.
  // this is the simpler approach:

  float _lineWidth = data.get(lineWidth);
  int _shapeType = data.get(shapeType);
  int _size = data.get(size);
  int _amount = data.get(amount);

  // Look on the example-Basic for more helping snippets to access the smoothed parameters on the newly re-created smoothed group. 
}
```

 


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
+ Simplify API getters.
+ Add more types: 2D/3D vectors and colors. Using templates [?] ...  
  [ _**ANY HELP/PULL ON THIS IS REALLY APPRECIATED!**_ ]
+ Add "real" nested sub-groups with tree levels. Now the params are recreated on one depth level only. This could help when duplicated names or to indent sub-groups on a GUI too.
+ Add independent thresholds/onSet for each parameter/channel and make it functional. Add callbacks to trig other events...
+ Add a global param to calibrate max history/speed.

#### ALTERNATIVE
There's another more powerful but complex filtering add-on that you can check too:  
https://github.com/bensnell/ofxFilter

## Authors
Original **ofxDataStream** engine author:  
Paul Turowski. http://paulturowski.com  
Thanks @**turowskipaul** !  

An add-on by **@moebiusSurfing**  
*( ManuMolina ) 2021*  

[Twitter](https://twitter.com/moebiussurfing/)  
[YouTube](https://www.youtube.com/channel/UCzUw96_wjmNxyIoFXf84hQg)  
[Instagram](https://www.instagram.com/moebiussurfing/)  
