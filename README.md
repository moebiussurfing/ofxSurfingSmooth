ofxSurfingSmooth
=============================

# Overview
An **openFrameworks** addon to do different styles of timed **smoothing** to grouped ```ofParameters```.

This add-on is extremely based on:  
https://github.com/turowskipaul/ofxDataStream  
Copyright (C) 2015 [Paul Turowski] (http://paulturowski.com)  
Thanks to @**turowskipaul**!  

## Screenshots
![image](docs/readme_images/Capture.jpg?raw=true "image")
![gif](docs/readme_images/ofxSurfingSmooth.gif?raw=true "gif")

## Features
- Just pass your ofParameterGroup container.
- Another smoothed ofParameterGroup will be created with the same structure.
- Only float and int types yet.
- Different smooth styles.
- Scalable and draggable plots.
- Store/Recall all the settings.

## Usage
 
** ofApp.h **
```.cpp
  #include "ofxSurfingSmooth.h"

  ofxSurfingSmooth surfingSmooth;

  ofParameterGroup params;
  ofParameter<float> lineWidth;
  ofParameter<float> separation;
  ofParameter<float> speed;
```

** ofApp.cpp **
```.cpp
ofApp::setup(){
 	params.setName("paramsGroup");// main container
 	params.add(lineWidth.set("lineWidth", 0.5, 0, 1));
	params.add(separation.set("separation", 50, 1, 100));
	params.add(speed.set("speed", 0.5, 0, 1));

	surfingSmooth.setup(params);
}
```

details>
  <summary>Dependencies</summary>
  <p>

Clone these add-ons and include into the **OF Project Generator** to allow compile your projects or the examples:
* [ofxHistoryPlot](https://github.com/moebiussurfing/ofxHistoryPlot)
* [ofxScaleDragRect](https://github.com/moebiussurfing/ofxScaleDragRect)
* [ofxImGui](https://github.com/moebiussurfing/ofxImGui)  
* [ofxSurfingHelpers](https://github.com/moebiussurfing/ofxSurfingHelpers)  
* [ofxWindowApp](https://github.com/moebiussurfing/ofxWindowApp)  [ Only for one of the examples ]
*Thanks a lot to all these ofxAddons coders.*  
  </p>
</details>

<details>
  <summary>Tested Systems</summary>
  <p>

  - **Windows 10** / **VS 2017** / **OF ~0.11**
  </p>
</details>

<br/>

### TODO
+ Add example/snippets to help access the smoothed parameters on the new created group.
+ Add "Real" nested sub-groups tree levels.
+ Add colors types, vectors, using templates...
+ Add thresholds/bonk independent for each variable/channel: make it functional. Add callbacks...
+ Add param to calibrate max history smooth.

## Author
An addon by **@moebiusSurfing**  
*( ManuMolina ) 2019-2021*  

[Twitter](https://twitter.com/moebiussurfing/)  
[Instagram](https://www.instagram.com/moebiussurfing/)  
[YouTube](https://www.youtube.com/channel/UCzUw96_wjmNxyIoFXf84hQg)  