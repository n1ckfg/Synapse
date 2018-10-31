#pragma once

#include "ofMain.h"
#include "ofxOpenNI.h"
#include "UDPMessenger.h"
#include "ActiveSkeleton.h"
#include "ofxFakam.h"

class JointHitDetector;

class testApp : public ofBaseApp, public IMessageReceiver
{
public:
	void setup();
	void update();
	void draw();
   void exit();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
   
   void OnMessage(const ofxOscMessage& msg);
   
private:
   void DisplayString(const char* string);
   
	//ofxONI oni;
	ofxOpenNIContext context;
	ofxDepthGenerator depth;
	ofxUserGenerator user;
   UDPMessenger mMessenger;
   std::vector<JointHitDetector*> mHitDetector;
   JointHitDetector* mClosestHand;
   ActiveSkeleton mActiveSkeleton;
   ofxFakam mFakeCamera;
   
   enum DepthMode
   {
      kAllDepth,
      kUserOnly,
      kAllUsers,
      kNumDepthModes
   }mDepthMode;
   
   char mDisplayString[100];
   float mDisplayStringTime;
};
