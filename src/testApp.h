#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxFaceTracker.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
    void keyPressed(int key);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x,int y,int button);
	void mouseDragged(int x, int y, int button);
    void mouseMoved(int x, int y);
    
	ofVideoPlayer           video;
    vector<ofImage>         images;
    int                     numImages;
    int                     currentImage;
    int                     currentManual;
    bool                    manualMode;
    bool                    setupFinished;
    
	ofxFaceTracker          tracker;
    vector<ofMesh>          trackedFrames;
    vector<vector<float> >  trackedGestures;
    vector<string>          imageFilnenames;
    vector<bool>            trackingResults;
    vector<int>             imageOrientations;
    void                    orientationChanged(int &i);
    
    
    ofxPanel*               gui;
    ofParameterGroup        parametersA;
    
    ofFbo                   fbo;
    ofParameter<bool>       p_active;
    ofParameter<int>        p_eyesWidth;
    ofParameter<int>        p_eyesHeight;
    ofParameter<int>        p_mouthHeight;
    ofParameter<int>        p_noseHeight;
    ofParameter<int>        p_orientationManual;
    ofParameter<float>      p_scaleManual;
    ofParameter<ofVec2f>    p_offsetManual;
    ofParameter<float>      p_rotationManual;
    
    vector<ofVec2f>         offsetPerImage;
    vector<int>             orientatPerImage;
    vector<float>           scalePerImage;
    vector<float>           rotationPerImage;
    
    ofVec2f                 vec;
    ofPoint                 centerEyes;
    ofPoint                 leftEye;
    ofPoint                 rightEye;
    ofPoint                 transformedLeftEye;
    ofPoint                 transformedRightEye;
    ofPoint                 transformedCenterEyes;
    ofPoint                 pressedPoint;
    float                   pressedScale;
    ofVec2f                 pressedOffset;
    float                   pressedRotation;
    
    ofMatrix4x4             mat;
    int                     eyeAlignX;
    int                     eyeAlignY;
    ofImage                 imageOut;
    bool                    processAll;

    
};
