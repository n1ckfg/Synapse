//
//  Quartz_KinectPlugin.h
//  Quartz Kinect
//
//  Created by Ryan Challinor on 6/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>

// Shared memory key
#define SHM_KEY_HEADER				((key_t)5511)
#define SHM_KEY_DATA				((key_t)5522)

// Buffer size, in order
//	1	= FLAG
//	3	= depth (RGB)
//	640	= width
//	480	= height
#define SHM_IMAGE_BPP				1
#define SHM_IMAGE_WIDTH				640
#define SHM_IMAGE_HEIGHT			480
#define SHARED_BUF_SIZE				(SHM_IMAGE_BPP*SHM_IMAGE_WIDTH*SHM_IMAGE_HEIGHT)

// 1st byte Flag
#define SHARED_FLAG_EMPTY			0
#define SHARED_FLAG_FILLED			1
//#define SHARED_FLAG_DISCONNECTED	2

// Client timeout diconnect (millis)
#define SHARED_CLIENT_TIMEOUT		(3000)

//
// HEADER
//
typedef struct _FakamImageHeader {
	short	bpp;			// RGB
	short	width;			// image width
	short	height;			// image height
	int		dataSize;		// data buffer size
	int		clientPid;		// connected client
	bool	filled;			// client pushed some data?
} FakamImageHeader;

// SHM Errors
enum FakamImageStatus {
	SHM_OK = 0,
	SHM_SHMGET_ERROR,
	SHM_SHMAT_ERROR,
	SHM_SHMDT_ERROR,
	SHM_MALLOC_ERROR,
	SHM_COUNT
};
extern char FakamImageStatusDesc[][128];


#import <Quartz/Quartz.h>

@interface Quartz_KinectPlugIn : QCPlugIn {
@private
    // shm
    int					shmid_header;
    int					shmid_data;
	FakamImageHeader	*addr_header;
	unsigned char		*addr_data;
	short				shmStatus;
}

-(BOOL)allocSharedMemory:(bool)server;
-(void)freeSharedMemory;
-(void)erase;

/*
Declare here the properties to be used as input and output ports for the plug-in e.g.
@property double inputFoo;
@property(assign) NSString* outputBar;
You can access their values in the appropriate plug-in methods using self.inputFoo or self.inputBar
*/
@property(assign) id<QCPlugInOutputImageProvider> outputImage;

@end
