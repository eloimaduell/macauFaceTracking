#include "testApp.h"

using namespace ofxCv;

int gestureCount = 4;

bool done = false;
ofVec2f fboSize;
int guiWidth = 600;
int guiHeight = 310;

int fboMult=4;

////////////////////////////////////////////////////////////////////////////////////
void testApp::setup() {
    
    setupFinished=false;
    processAll=false;
    manualMode = false;
    
    /// SETUP TRACKER
	tracker.setup();
    tracker.setIterations(25);   // [1-25] 1 is fast and inaccurate, 25 is slow and accurate
    tracker.setRescale(1.0);
    tracker.setClamp(4);        // [0-4] 1 gives a very loose fit, 4 gives a very tight fit
    tracker.setTolerance(1.000);  // [.01-1] match tolerance
    tracker.setAttempts(4);     // [1-4] 1 is fast and may not find faces, 4 is slow but will find faces
    
    
    /// ADD IMAGES
    currentImage = 0;
    ofDirectory dir;
    dir.open("./inputFaces");
    dir.listDir();
    numImages =dir.numFiles();
    for(int i=0;i<numImages;i++)
    {
        ofImage img;
        img.loadImage(dir.getFile(i));
        cout << "Loading Image : " <<i <<" : " <<dir.getFile(i).getFileName() <<endl;
        images.push_back(img);
        imageFilnenames.push_back(dir.getFile(i).getFileName());
        trackingResults.push_back(false);
        // orientation
        system((ofToDataPath("jpegexiforient") + " " +dir.getFile(i).path()  + " > " + ofToDataPath("tmp.txt")).c_str());
        int orientation = ofToInt(ofBufferFromFile("tmp.txt").getFirstLine());
        imageOrientations.push_back(orientation);
        cout <<(ofToDataPath("jpegexiforient") + " " +dir.getFile(i).path()  + " > " + ofToDataPath("tmp.txt")).c_str() <<" == orient : " <<orientation <<endl;
        
        
        if(orientation==8)
        {
            //images[i].rotate90(3);
        }
        else if((orientation==6))//||(orientation==1))
        {
            //images[i].rotate90(1);
        }
        
        // fill default values to manual controls vectors
        ofVec2f offs;
        offs.set(0,0);
        offsetPerImage.push_back(offs);
        
        float scale=1.0;
        scalePerImage.push_back(scale);
        
        int orient = 0;
        orientatPerImage.push_back(orient);
        
        float rot = 0.0;
        rotationPerImage.push_back(rot);
        
    }
    
    //    string jpeg = "/Users/rick/Downloads/image.jpg";
    //    system((ofToDataPath("jpegexiforient") + " " + jpeg + " > " + ofToDataPath("tmp.txt")).c_str());
    //    int orientation = ofToInt(ofBufferFromFile("tmp.txt").getFirstLine());
    //    cout << "orientation: " << orientation << endl;
    
    
    /// FBO & IMAGE
    fboSize.x = 1080;
    fboSize.y = 1080;
    fbo.allocate(fboSize.x,fboSize.y);
    imageOut.allocate(fboSize.x,fboSize.y,OF_IMAGE_COLOR);
    
    /// GUI
    p_orientationManual.addListener(this,&testApp::orientationChanged);
    
    parametersA.add(p_active.set("active",true));
    parametersA.add(p_eyesWidth.set("eyes_width",400,0,fboSize.x));
    parametersA.add(p_eyesHeight.set("eyes_height",400,0,fboSize.y));
    parametersA.add(p_noseHeight.set("nose_height",500,0,fboSize.y));
    parametersA.add(p_mouthHeight.set("mouth_height",600,0,fboSize.y));
    
    parametersA.add(p_orientationManual.set("manual orientation",0,0,3));
    parametersA.add(p_scaleManual.set("manual scale",1.0,-4.0,4.0));
    parametersA.add(p_offsetManual.set("manual offset",ofVec2f(0.0,0.0),ofVec2f(-1.0,-1.0),ofVec2f(1.0,1.0)));
    parametersA.add(p_rotationManual.set("manual rotation",0.0,-45.0,45.0));
    
    gui = new ofxPanel();
    
    gui->setup();
    gui->setName("face_align . v0.1");
    gui->add(parametersA);
    gui->setSize(guiWidth,100);
    gui->setWidthElements(guiWidth);
    gui->setPosition(10,10);
    
    gui->loadFromFile("settings.xml");
    
    
    ofNoFill();
    setupFinished=true;
    
}



////////////////////////////////////////////////////////////////////////////////////
void testApp::update() {
    
    if(!done)
    {
        printf("tracking face %d : %s",currentImage,imageFilnenames[currentImage].c_str());
        tracker.update(toCv(images[currentImage]));
        done = true;
        if(tracker.getFound())
        {
            trackingResults[currentImage] = true;
            //imageOrientations.push_back(-1);
        }
        else
        {
            trackingResults[currentImage] = false;
            //imageOrientations.push_back(0);
            
        }
        printf("... done! \n");
        
    }
    
    //update offset manual to each vector p
    offsetPerImage[currentImage] = p_offsetManual;
    scalePerImage[currentImage] = p_scaleManual;
    orientatPerImage[currentImage] = p_orientationManual;
    rotationPerImage[currentImage] = p_rotationManual;
}

////////////////////////////////////////////////////////////////////////////////////
void testApp::draw() {
    
    
    ofBackground(32);
    
    ofSetColor(255);
    int finalWidthPreview = 600;
    float angle,scale;
    mat.makeIdentityMatrix();
    
    /// DRAW RECT FOR GUI AND PREVIEW
    ofSetColor(16);
    ofFill();
    ofRect(0,0,finalWidthPreview+10+10,ofGetHeight());
    
    /// DEALING WITH TRACKER TO GET EYES POSITIONS AND ROTATION
    ofPushMatrix();
    {
        
        /// CALCULATE LEFT EYE MEDIAN POSITION
        ofPolyline p = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE);
        leftEye = ofPoint(0,0);
        for(int i=0;i<p.size();i++)
        {
            leftEye = leftEye + p.getVertices()[i];
        }
        leftEye = leftEye/p.size();
        
        /// CALCULATE RIGHT EYE MEDIAN POSITION
        p.clear();
        p = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE);
        rightEye = ofPoint(0,0);
        
        
        for(int i=0;i<p.size();i++)
        {
            rightEye = rightEye + p.getVertices()[i];
        }
        rightEye = rightEye/p.size();
        
        /// CALCULATE INTER EYES POSITION AND VECTOR AND IT's ANGLE
        ofVec2f vecInterEyes = rightEye-leftEye;
        centerEyes = leftEye + vecInterEyes/2;
        ofVec2f vecXAxis = ofVec2f(1,0);
        angle = acos(vecXAxis.dot(vecInterEyes.normalize()));
        if(rightEye.y > leftEye.y) angle = -angle;
    }
    ofPopMatrix();
    
    
    /// DRAWING INTO FINAL FBO ...
    
    fbo.begin();
    {
        ofPushMatrix();
        
        // CLEAR FBO
        ofClear(0,0,0, 0);
        ofSetColor(0);
        ofRect(0,0,fboSize.x,fboSize.y);
        ofSetColor(255);
        
        if(trackingResults[currentImage])
        {
            /// IF WE HAVE FOUND A FACE, ALIGN it !!
            float distanceBetweenEyes = rightEye.x-leftEye.x;
            scale = p_eyesWidth/distanceBetweenEyes;
            
            // ROTATE & SCALE & TRANLATE
            ofTranslate(fboSize.x/2,p_eyesHeight);
            ofRotate(ofRadToDeg(angle),0,0,1);
            ofScale(scale,scale);
            ofTranslate(-centerEyes.x,-centerEyes.y);
            
            // APPLY TO MATRIX (INVERSE ORDER!!)
            mat.translate(fboSize.x/2,p_eyesHeight,0);
            mat.scale(scale,scale,1);
            mat.rotate(ofRadToDeg(angle), 0, 0, 1);
            mat.translate(ofGetMouseX(),ofGetMouseY(),0);
            
            // MULT BY MAT
            transformedLeftEye = leftEye * mat;
            transformedRightEye = rightEye * mat;
            transformedCenterEyes = centerEyes * mat;
            
            images[currentImage].draw(0,0);
            
            ofSetColor(0,255,0);
            ofPolyline p = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE);
            p.draw();
        }
        else
        {
            /// IF NO FACE IS FOUND ... MANUAL
            ofSetColor(255);
            // rotate manually
            ofPushMatrix();
            ofTranslate(images[currentImage].getWidth()/2,images[currentImage].getHeight()/2);
            switch(orientatPerImage[currentImage])
            {
                case 0 :
                    //ofRotateZ(-90);
                    break;
                case 1 :
                    ofRotateZ(90);
                    break;
                case 2 :
                    ofRotateZ(-90);
                    break;
            }
            ofTranslate(-images[currentImage].getWidth()/2,-images[currentImage].getHeight()/2);
            
            // MOVE AND SCALE (AUTO + MANUALLY) THE PICTURE TO FBO to have a first image to manually modify
            ofVec2f scalePicture;
            float imgAspectRatio = images[currentImage].getWidth() / images[currentImage].getHeight();
            
            ofTranslate(fboSize.x/2,fboSize.y/2);
            ofScale(scalePerImage[currentImage],scalePerImage[currentImage]);
            ofRotateZ(rotationPerImage[currentImage]);
            ofTranslate(-fboSize.x/2,-fboSize.y/2);
            
            ofTranslate(fboSize.x*offsetPerImage[currentImage].x,fboSize.y*offsetPerImage[currentImage].y);
            
            if (images[currentImage].getWidth()>images[currentImage].getHeight())
            {
                // LANDSCAPE
                scalePicture.y = fboSize.y / images[currentImage].getHeight();
                
                images[currentImage].draw(0,0,(scalePicture.y * images[currentImage].getHeight())*imgAspectRatio,scalePicture.y * images[currentImage].getHeight());
                
            }
            else
            {
                // PORTRAIT
                scalePicture.x = fboSize.x / images[currentImage].getWidth();
                images[currentImage].draw(0,0,scalePicture.x * images[currentImage].getWidth(),(scalePicture.x * images[currentImage].getWidth())/imgAspectRatio);
            }
            
            ofPopMatrix();
            
        }
        
        ofPopMatrix();
        
        /// draw CONTROL lines
        
        if(!processAll)
        {
            /// eyes width line
            ofPushMatrix();
            ofSetColor(0,128,255);
            ofLine(fboSize.x/2 - p_eyesWidth/2,0,fboSize.x/2 - p_eyesWidth/2,fboSize.y);
            ofLine(fboSize.x/2 + p_eyesWidth/2,0,fboSize.x/2 + p_eyesWidth/2,fboSize.y);
            /// eyes height line
            ofSetColor(0,255,255);
            ofLine(0,p_eyesHeight,fboSize.x,p_eyesHeight);
            
            /// mouth reference
            ofSetColor(255,128,0);
            ofLine(0,p_mouthHeight,fboSize.x,p_mouthHeight);
            /// nose reference
            ofSetColor(255,128,0);
            ofLine(0,p_noseHeight,fboSize.x,p_noseHeight);
            
            //            ofSetColor(0,255,0);
            //            ofCircle(transformedLeftEye,15);
            //            ofSetColor(0,255,0);
            //            ofCircle(transformedRightEye,15);
            
        }
        
        ofPopMatrix();
    }
    fbo.end();
    
    
    /// DRAW FBO TO SCREEN AND BORDER
    ofPushStyle();
    ofPushMatrix();
    {
        if(trackingResults[currentImage])
        {
            ofSetColor(255);
        }
        else ofSetColor(255,0,0);
        ofRect(guiWidth+10+10,0,fboSize.x+10+10,fboSize.y+10+10);
        ofSetColor(255);
        
        
        fbo.draw(guiWidth+10+10+10,10);
        ofSetColor(255,255,255);
        //        ofNoFill();
        //        ofRect(finalWidthPreview,0,fboSize.x,fboSize.y);
    }
    ofPopMatrix();
    ofPopStyle();
    
    
    /// DRAW PREVIEW PHOTO
    ofSetColor(255);
    images[currentImage].draw(10,guiHeight,finalWidthPreview,finalWidthPreview/(float(images[currentImage].getWidth())/float(images[currentImage].getHeight())));
    ofSetColor(255);
    ofDrawBitmapString( ofToString(currentImage) +"/" +ofToString(numImages) +" : "+ ofToString(imageFilnenames[currentImage]) +"\n" +ofToString(images[currentImage].getWidth()) + " x " +ofToString(images[currentImage].getHeight()) + " orient: " + ofToString(imageOrientations[currentImage]),10,285);
    
    /// DRAW GUI
    ofSetColor(255);
    ofPushMatrix();
    gui->draw();
    ofPopMatrix();
    
    
    // SAVE AUTOMATIC IMAGES
    if(processAll)
    {
        //if(trackingResults[currentImage]==true)
        if(true)
        {
            imageOut.grabScreen(guiWidth+10+10+10,0,fboSize.x,fboSize.y);
            imageOut.saveImage("./outputOk/"+ofToString(currentImage)+".png");
            
        }
        /// DRAW PROGRESS BAR
        ofSetColor(255);
        ofFill();
        ofRect(0,ofGetHeight()-30,(float(currentImage)/float(images.size()))*ofGetWidth(),30);
        ofSetColor(0);
        ofDrawBitmapString(ofToString(int(100.0*(float(currentImage)/float(images.size())))) + "%  " + ofToString(currentImage) +"/" +ofToString(numImages),((float(currentImage)/float(images.size()))*ofGetWidth())-85,ofGetHeight()-10);
        
        currentImage=currentImage+1;
        
        if(currentImage>images.size()-1)
        {
            processAll=false;
            currentImage=0;
        }
        done=false;
        
    }
    
    
}

////////////////////////////////////////////////////////////////////////////////////
void testApp::keyPressed(int key)
{
    if(key==' ')
    {
        cout << ofGetMouseX() << " , " << ofGetMouseY() <<endl;
    }
    else if(key==OF_KEY_LEFT)
    {
        currentImage=currentImage-1;
        if(currentImage<0) currentImage=numImages-1;
        // update manual controls
        p_offsetManual = offsetPerImage[currentImage];
        p_scaleManual = scalePerImage[currentImage];
        p_orientationManual = orientatPerImage[currentImage];
        p_rotationManual = rotationPerImage[currentImage];
        done=false;
    }
    else if(key==OF_KEY_RIGHT)
    {
        currentImage=(currentImage+1)%numImages;
        // update manual controls
        p_offsetManual = offsetPerImage[currentImage];
        p_scaleManual = scalePerImage[currentImage];
        p_orientationManual = orientatPerImage[currentImage];
        p_rotationManual = rotationPerImage[currentImage];
        done=false;
    }
    else if (key=='P')
    {
        processAll=true;
    }
    
    else if (key=='v')
    {
        p_orientationManual = (p_orientationManual +1 ) % p_orientationManual.getMax();
    }

    else if (key=='s')
    {
        gui->saveToFile("settings.xml");
    }
}

////////////////////////////////////////////////////////////////////////////////////
void testApp::mousePressed(int x, int y, int button)
{
    pressedPoint.set(x,y);
    pressedScale = p_scaleManual;
    pressedOffset = p_offsetManual;
    pressedRotation = p_rotationManual;
}
////////////////////////////////////////////////////////////////////////////////////
void testApp::mouseReleased(int x,int y,int button)
{
    
}
////////////////////////////////////////////////////////////////////////////////////
void testApp::mouseDragged(int x, int y, int button)
{
    ofPoint actualDragPoint;
    actualDragPoint.set(x,y);
    
    ofPoint offsetDrag = actualDragPoint - pressedPoint;
    
    
    
    if(ofGetKeyPressed('x'))
    {
        float scale = ((float(offsetDrag.x) / float(ofGetWidth())));
        p_scaleManual = pressedScale + (scale * p_scaleManual.getMax());
    }
    else if(ofGetKeyPressed('z'))
    {
        ofVec2f v;
        
        v.x = ((offsetDrag.x / float(ofGetWidth())));
        v.y = ((offsetDrag.y / float(ofGetHeight())));
        
        p_offsetManual = pressedOffset + v;
        
        
    }
    else if(ofGetKeyPressed('c'))
    {
        float rot = ((float(offsetDrag.x) / float(ofGetWidth())));
        p_rotationManual = pressedRotation + (rot * p_rotationManual.getMax());
    }
    
}
////////////////////////////////////////////////////////////////////////////////////
void testApp::mouseMoved(int x, int y)
{
    
}

////////////////////////////////////////////////////////////////////////////////////
void testApp::orientationChanged(int &i)
{
    if(setupFinished)
    {
//        imageOrientations[currentImage] = i;
    }
}
