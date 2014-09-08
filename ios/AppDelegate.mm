
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  iOS User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2013, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  As an exception, when this program is distributed through (i) the     *
 *  App Store by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or     *
 *  (iii) Google Play by Google Inc., then that store may impose any      *
 *  digital rights management, device limits and/or redistribution        *
 *  restrictions that are required by its terms of service.               *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,       *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************/

#import "AppDelegate.h"
#import "MasterViewController.h"
#import "file/nglobaldirs.h"

// TODO: Support state preservation and restoration.
// See: https://developer.apple.com/library/ios/documentation/iPhone/Conceptual/iPhoneOSProgrammingGuide/StatePreservation/StatePreservation.html

@interface AppDelegate() {
    UIBackgroundTaskIdentifier bgTask;
    MasterViewController* master;
}
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.
    //
    // From the iOS App Programming Guide:
    // - Assume portrait orientation here.  If the device is in landscape
    //   then the system will rotate the views later, but before displaying them.
    // - This routine must be fast.  Long tasks should be run on a secondary thread.
    //
    UISplitViewController *splitViewController = (UISplitViewController *)self.window.rootViewController;
    UINavigationController *navigationController;
    
    navigationController = [splitViewController.viewControllers firstObject];
    master = (id)navigationController.topViewController;
    
    navigationController = [splitViewController.viewControllers lastObject];
    splitViewController.delegate = (id)navigationController.topViewController;
    
    // Make sure that Regina knows where to find its internal data files.
    NSBundle* mainBundle = [NSBundle mainBundle];
    if (! mainBundle) {
        NSLog(@"Could not access main bundle.");
    } else {
        NSString* path = [mainBundle resourcePath];
        if (! path) {
            NSLog(@"Could not access resource path.");
        } else {
            const char* home = [path UTF8String];
            regina::NGlobalDirs::setDirs(home, "", home);
        }
        return NO;
    }

    return YES;
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    return [master openURL:url];
}
							
- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // We need to save data, and also remember what we were doing so that
    // we can restore state later.
    // This routine *must* return quickly; otherwise the app may simply be killed.
    bgTask = [application beginBackgroundTaskWithExpirationHandler:^{
        // We are still running and about to run out of time.
        // If there is a way to gracefully abort the save, here is the
        // place to do it.
        [application endBackgroundTask:bgTask];
        bgTask = UIBackgroundTaskInvalid;
    }];

    // TODO: Save state.
    
    // Start the save in a different thread and return immediately.
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        // TODO: Save data.
        [application endBackgroundTask:bgTask];
        bgTask = UIBackgroundTaskInvalid;
    });
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // We need to save data and state here.
    // This *must* be fast: it has a strict time limit of ~5s, and there is
    // no way to request more time.

    // TODO: Save state.
    // TODO: Save data.
}

@end