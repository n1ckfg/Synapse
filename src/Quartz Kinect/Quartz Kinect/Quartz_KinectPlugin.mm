//
//  Quartz_KinectPlugIn.m
//  Quartz Kinect
//
//  Created by Ryan Challinor on 6/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
#import <OpenGL/CGLMacro.h>

#import "Quartz_KinectPlugIn.h"

#define	kQCPlugIn_Name				@"Synapse Kinect"
#define	kQCPlugIn_Description		@"Gets Kinect depth buffer from the Synapse application"

static void _BufferReleaseCallback(const void* address, void* info)
{
	free((void*)address);
}

@implementation Quartz_KinectPlugIn

/*
Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
@dynamic inputFoo, outputBar;
*/
@dynamic outputImage;

+ (NSDictionary *)attributes
{
	/*
	Return a dictionary of attributes describing the plug-in (QCPlugInAttributeNameKey, QCPlugInAttributeDescriptionKey...).
	*/
	
	return [NSDictionary dictionaryWithObjectsAndKeys:kQCPlugIn_Name, QCPlugInAttributeNameKey, kQCPlugIn_Description, QCPlugInAttributeDescriptionKey, nil];
}

+ (NSDictionary *)attributesForPropertyPortWithKey:(NSString *)key
{
	/*
	Specify the optional attributes for property based ports (QCPortAttributeNameKey, QCPortAttributeDefaultValueKey...).
	*/
    if([key isEqualToString:@"outputImage"])
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Depth Image", QCPortAttributeNameKey, nil];
	
	return nil;
}

+ (QCPlugInExecutionMode)executionMode
{
	/*
	Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
	*/
	
	return kQCPlugInExecutionModeProcessor;
}

+ (QCPlugInTimeMode)timeMode
{
	/*
	Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
	*/
	
	return kQCPlugInTimeModeIdle;
}

- (id)init
{
	self = [super init];
	if (self) {
		/*
		Allocate any permanent resource required by the plug-in.
		*/
        shmStatus = SHM_COUNT;
        [self allocSharedMemory:true];
        
        // Init header
        addr_header->bpp		= SHM_IMAGE_BPP;		// RGB
        addr_header->width		= SHM_IMAGE_WIDTH;
        addr_header->height		= SHM_IMAGE_HEIGHT;
        addr_header->dataSize	= SHARED_BUF_SIZE;
        addr_header->clientPid	= 0;		// no-one connected
        addr_header->filled		= false;	// empty
	}
	
	return self;
}

- (void)finalize
{
	/*
	Release any non garbage collected resources created in -init.
	*/
	
	[super finalize];
}

- (void)dealloc
{
	/*
	Release any resources created in -init.
	*/
    
    [self freeSharedMemory];
	
	[super dealloc];
}


//
// Get shared memory
-(BOOL)allocSharedMemory:(bool)server
{
	int shmflg = ( server ? IPC_CREAT|0666 : 0666 );
	
	//
	// ALLOC HEADER
	if ((shmid_header = shmget(SHM_KEY_HEADER, sizeof(FakamImageHeader), shmflg)) < 0)
	{
		printf("FAKAM: HEADER: shmget failed!!! errno[%d/%s]\n",errno,strerror(errno));
		shmStatus = SHM_SHMGET_ERROR;
		return false;
	}
    if ((char*)(addr_header = (FakamImageHeader*) shmat(shmid_header, NULL, 0)) == (char *) -1)
	{
		printf("FAKAM: HEADER: shmat failed!!! errno[%d/%s]\n",errno,strerror(errno));
		shmStatus = SHM_SHMAT_ERROR;
		return false;
	}
	//printf("FAKAM: HEADER: addr_header[%d] OK!\n",(int)addr_header);
	
	//
	// ALLOC DATA
	if ((shmid_data = shmget(SHM_KEY_DATA, SHARED_BUF_SIZE, shmflg)) < 0)
	{
		printf("FAKAM: DATA: shmget failed!!! errno[%d/%s]\n",errno,strerror(errno));
		shmStatus = SHM_SHMGET_ERROR;
		return false;
	}
    if ((char*)(addr_data = (unsigned char*) shmat(shmid_data, NULL, 0)) == (char *) -1)
	{
		printf("FAKAM: DATA: shmat failed!!! errno[%d/%s]\n",errno,strerror(errno));
		shmStatus = SHM_SHMAT_ERROR;
		return false;
	}
	//printf("FAKAM: DATA:   addr_data[%d] OK!\n",(int)addr_header);
    
	// Ok!
	[self erase];
	shmStatus = SHM_OK;
	return true;
}

//
// Get shared memory
-(void)freeSharedMemory
{
	// Release shared memory
	if (addr_header > 0)
	{
		if (shmdt(addr_header) < 0)
		{
			printf("FAKAM: addr_header shmdt failed!!!\n");
			shmStatus = SHM_SHMDT_ERROR;
			return;
		}
	}
	// Release shared memory
	if (addr_data > 0)
	{
		if (shmdt(addr_data) < 0)
		{
			printf("FAKAM: addr_data shmdt failed!!!\n");
			shmStatus = SHM_SHMDT_ERROR;
			return;
		}
	}
	// Ok!
	shmStatus = SHM_OK;
}

-(void)erase
{
	memset(addr_data, 0, addr_header->dataSize);
}

@end

@implementation Quartz_KinectPlugIn (Execution)

- (BOOL)startExecution:(id <QCPlugInContext>)context
{
	/*
	Called by Quartz Composer when rendering of the composition starts: perform any required setup for the plug-in.
	Return NO in case of fatal failure (this will prevent rendering of the composition to start).
	*/
	
	return YES;
}

- (void)enableExecution:(id <QCPlugInContext>)context
{
	/*
	Called by Quartz Composer when the plug-in instance starts being used by Quartz Composer.
	*/
}

- (BOOL)execute:(id <QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary *)arguments
{
	/*
	Called by Quartz Composer whenever the plug-in instance needs to execute.
	Only read from the plug-in inputs and produce a result (by writing to the plug-in outputs or rendering to the destination OpenGL context) within that method and nowhere else.
	Return NO in case of failure during the execution (this will prevent rendering of the current frame to complete).
	
	The OpenGL context for rendering can be accessed and defined for CGL macros using:
	CGLContextObj cgl_ctx = [context CGLContextObj];
	*/
    
	NSString*						format = QCPlugInPixelFormatI8;
	id								provider = nil;
	NSUInteger						pixelsWide, pixelsHigh;
	void*							pixelBuffer;
	NSUInteger						rowBytes;
	CGColorSpaceRef					colorSpace = CGColorSpaceCreateDeviceGray();//[context colorSpace];
    
   /* Create memory buffer */
   pixelsWide = SHM_IMAGE_WIDTH;
   pixelsHigh = SHM_IMAGE_HEIGHT;
   rowBytes = pixelsWide*SHM_IMAGE_BPP;
   pixelBuffer = valloc(pixelsHigh*rowBytes);
   if(pixelBuffer == NULL)
      return NO;
   
   memcpy(pixelBuffer, addr_data, addr_header->dataSize);
   addr_header->filled = false;
    
    /* Create simple provider from memory buffer */
    provider = [context outputImageProviderFromBufferWithPixelFormat:format pixelsWide:pixelsWide pixelsHigh:pixelsHigh baseAddress:pixelBuffer bytesPerRow:rowBytes releaseCallback:_BufferReleaseCallback releaseContext:NULL colorSpace:colorSpace shouldColorMatch:YES];
    if(provider == nil) {
        free(pixelBuffer);
        return NO;
    }
	
	/* Update the result image */
	self.outputImage = provider;
	
	return YES;
}

- (void)disableExecution:(id <QCPlugInContext>)context
{
	/*
	Called by Quartz Composer when the plug-in instance stops being used by Quartz Composer.
	*/
}

- (void)stopExecution:(id <QCPlugInContext>)context
{
	/*
	Called by Quartz Composer when rendering of the composition stops: perform any required cleanup for the plug-in.
	*/
}

@end
