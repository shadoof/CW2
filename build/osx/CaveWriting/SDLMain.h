/*  
	This file should be included in the project for 
	all SDL applications for OS X.  The files SDLMain.h
	and SDLMain.m are included in the SDL 1.2 distribution
	for OS X (see authorship information below), and they're
	included for the sake of convenience in the G3D distribution.

	SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
	
	
*/

#import <Cocoa/Cocoa.h>

int gFinderLaunch;

@interface SDLMain : NSObject
@end
