/*
 *  ActiveSkeleton.h
 *  emptyExample
 *
 *  Created by Ryan Challinor on 5/18/11.
 *  Copyright 2011 aww bees. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxOpenNI.h"

class ActiveSkeleton
{
public:
   ActiveSkeleton();
   ~ActiveSkeleton();
   
   bool IsTracked() { return mUser != NULL; }
   
   void SetUserGenerator(ofxUserGenerator* pUserGenerator) { mUserGenerator = pUserGenerator; }
   void SetDepthGenerator(ofxDepthGenerator* pDepthGenerator) { mDepthGenerator = pDepthGenerator; }
   void SetActiveUser(ofxTrackedUser* pUser) { mUser = pUser; }
      
   ofVec3f GetRealWorldPos(XnSkeletonJoint joint, float& confidence) const;
   ofVec3f GetProjectivePos(XnSkeletonJoint joint) const;
   unsigned short GetUserIndex() { return mUser ? mUser->id : 255; }
private:
   ofxUserGenerator* mUserGenerator;
   ofxDepthGenerator* mDepthGenerator;
   ofxTrackedUser* mUser;
};

extern ActiveSkeleton* TheActiveSkeleton;

