#include "testApp.h"
#include "JointHitDetector.h"

//--------------------------------------------------------------
void testApp::setup(){
	context.setupUsingXMLFile(ofToDataPath("openni/config/ofxopenni_config.xml", true));
	depth.setup(&context);
	user.setup(&context, &depth);
   mActiveSkeleton.SetUserGenerator(&user);
   mActiveSkeleton.SetDepthGenerator(&depth);
   
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_RIGHT_HAND, XN_SKEL_TORSO, "/righthand"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_LEFT_HAND, XN_SKEL_TORSO, "/lefthand"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_RIGHT_ELBOW, XN_SKEL_TORSO, "/rightelbow"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_LEFT_ELBOW, XN_SKEL_TORSO, "/leftelbow"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_RIGHT_KNEE, XN_SKEL_TORSO, "/rightknee"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_LEFT_KNEE, XN_SKEL_TORSO, "/leftknee"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_RIGHT_FOOT, XN_SKEL_TORSO, "/rightfoot"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_LEFT_FOOT, XN_SKEL_TORSO, "/leftfoot"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_HEAD, XN_SKEL_RIGHT_HIP, "/head"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_TORSO, XN_SKEL_TORSO, "/torso", 75));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_NECK, XN_SKEL_TORSO, "/neck"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO, "/leftshoulder"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO, "/rightshoulder"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_LEFT_HIP, XN_SKEL_TORSO, "/lefthip"));
   mHitDetector.push_back(new JointHitDetector(XN_SKEL_RIGHT_HIP, XN_SKEL_TORSO, "/righthip"));
   mClosestHand = new JointHitDetector(XN_SKEL_RIGHT_HAND, XN_SKEL_TORSO, "/closesthand");
   mHitDetector.push_back(mClosestHand);
   
   TheMessenger->AddListener(this, "/depth_mode");
   
 	ofSetFrameRate(60);
   
   mDepthMode = kAllDepth;
   mDisplayStringTime = 0.0f;
   
   DisplayString("Press space to change depth modes");
}

void testApp::exit()
{
   for (int i=0; i<mHitDetector.size(); ++i)
      delete mHitDetector[i];
}

//--------------------------------------------------------------
void testApp::update(){
	context.update();
	user.update();
   
   TheMessenger->Poll();
   
   float dt = .01666f;

   ofxTrackedUser* pTracked = NULL;
   for (int i=0; i<15; ++i)
   {
      ofxTrackedUser* pUser = user.getTrackedUser(i);
      if (pUser != NULL && pUser->IsTracked())
      {
         pTracked = pUser;
         break;
      }
   }
   
   mActiveSkeleton.SetActiveUser(pTracked);
   
   static bool sTracking = false;
	if (mActiveSkeleton.IsTracked())
   {
      float dummy;
      if (TheActiveSkeleton->GetRealWorldPos(XN_SKEL_RIGHT_HAND, dummy).z <
          TheActiveSkeleton->GetRealWorldPos(XN_SKEL_LEFT_HAND, dummy).z)
         mClosestHand->SetJoint(XN_SKEL_RIGHT_HAND);
      else
         mClosestHand->SetJoint(XN_SKEL_LEFT_HAND);
      
      for (int i=0; i<mHitDetector.size(); ++i)
      {
         mHitDetector[i]->Poll(dt);
      }
      
      if (!sTracking)
      {
         sTracking = true;
         TheMessenger->SendIntMessage("/tracking_skeleton", 1);
      }
   }
   else
   {
      if (sTracking)
      {
         sTracking = false;
         TheMessenger->SendIntMessage("/tracking_skeleton", 0);
      }
   }
   
   if (mDisplayStringTime > 0.0f)
      mDisplayStringTime -= dt;
}

//--------------------------------------------------------------
void testApp::draw(){
   switch (mDepthMode)
   {
      case kAllDepth:
         depth.draw(0,0,640,480);
         break;
      case kUserOnly:
         depth.draw(0,0,640,480, user.getUserMask(), TheActiveSkeleton->GetUserIndex());
         break;
      case kAllUsers:
         depth.draw(0,0,640,480, user.getUserMask());
         break;
      default:
         break;
   }
	user.draw();
   
   for (int i=0; i<mHitDetector.size(); ++i)
      mHitDetector[i]->Draw();
   
   mFakeCamera.write(depth.getGrayPixels());
   
   if (mDisplayStringTime > 0)
   {
      ofSetColor(255, 255, 255);
      ofDrawBitmapString(mDisplayString, 20, 460);
   }
}

//--------------------------------------------------------------
void testApp::DisplayString(const char *string)
{
   mDisplayStringTime = 2.0f;
   strncpy(mDisplayString, string, 100);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
   switch (key)
   {
      case ' ':
         switch (mDepthMode)
         {
            case kAllDepth:
               DisplayString("Depth display mode: Tracked user only");
               mDepthMode = kUserOnly;
               break;
            case kUserOnly:
               DisplayString("Depth display mode: All users");
               mDepthMode = kAllUsers;
               break;
            case kAllUsers:
               DisplayString("Depth display mode: Full depth buffer");
               mDepthMode = kAllDepth;
               break;
         }
         break;
      default:
         break;
   }
}

void testApp::OnMessage(const ofxOscMessage& msg)
{
   if (msg.getAddress() == "/depth_mode")
   {
      assert(msg.getNumArgs() == 1);
      assert(msg.getArgType(0) == OFXOSC_TYPE_INT32);
      int mode = msg.getArgAsInt32(0);
      if (mode >= 0 && mode < kNumDepthModes)
         mDepthMode = (DepthMode)mode;
   }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}