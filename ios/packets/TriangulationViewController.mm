
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

#import "TriangulationViewController.h"
#import "TextHelper.h"
#import "triangulation/ntriangulation.h"

@implementation TriangulationViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setSelectedImages:@[@"Tab-Gluings-Bold",
                              @"Tab-Skeleton-Bold",
                              [NSNull null],
                              @"Tab-Composition-Bold",
                              @"Tab-Recognition-Bold",
                              @"Tab-SnapPea-Bold"]];
    [self registerDefaultKey:@"ViewTriangulationTab"];
}

- (NSString *)packetActionIcon
{
    return @"NavBar-Triangulation";
}

- (void)packetActionPressed
{
    NSLog(@"TODO: Triangulation action pressed!");
}

- (void)updateHeader:(UILabel *)header
{
    if (self.packet->isEmpty()) {
        header.text = @"Empty";
        return;
    }

    if (! self.packet->isValid()) {
        header.attributedText = [TextHelper badString:@"Invalid triangulation"];
        return;
    }

    NSMutableString* msg;

    if (self.packet->isClosed())
        msg = [NSMutableString stringWithString:@"Closed, "];
    else {
        if (self.packet->isIdeal() && self.packet->hasBoundaryTriangles())
            msg = [NSMutableString stringWithString:@"Ideal & real boundary, "];
        else if (self.packet->isIdeal())
            msg = [NSMutableString stringWithString:@"Ideal boundary, "];
        else if (self.packet->hasBoundaryTriangles())
            msg = [NSMutableString stringWithString:@"Real boundary, "];
    }

    if (self.packet->isOrientable()) {
        if (self.packet->isOriented())
            [msg appendString:@"orientable and oriented, "];
        else
            [msg appendString:@"orientable but not oriented, "];
    } else
        [msg appendString:@"non-orientable, "];

    if (self.packet->isConnected())
        [msg appendString:@"connected"];
    else
        [msg appendString:@"disconnected"];

    header.text = msg;
}

@end