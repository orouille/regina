
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

#import "TextHelper.h"
#import "TriangulationViewController.h"
#import "TriRecognition.h"
#import "census/ncensus.h"
#import "manifold/nmanifold.h"
#import "snappea/nsnappeatriangulation.h"
#import "subcomplex/nstandardtri.h"
#import "triangulation/ntriangulation.h"

#define PROP_SPHERE 1
#define PROP_BALL 2
#define PROP_SOLIDTORUS 3
#define PROP_ZEROEFF 4
#define PROP_SPLITTING 5
#define PROP_IRREDUCIBLE 6
#define PROP_HAKEN 7
#define PROP_STRICT 8
#define PROP_HYPERBOLIC 9

// Currently the largest isosig in the census tables starts with 'G'.
// This represents 32 tetrahedra.
// We will be more conservative here.
#define MAX_CENSUS_TRIANGULATION_SIZE 50

// TODO: Census lookup: long press for more lines of information.

@interface PropertyCell : UITableViewCell
@property (assign, nonatomic) int property;
@property (weak, nonatomic) IBOutlet UILabel *name;
@property (weak, nonatomic) IBOutlet UILabel *value;
@property (weak, nonatomic) IBOutlet UIButton *calculate;
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *progress;
@end

@implementation PropertyCell
@end

@interface TriRecognition () <UITableViewDataSource> {
    NSMutableArray* propertyList;
    NSString* manifoldName;
    regina::NProperty<bool> isHyp;
}
@property (weak, nonatomic) IBOutlet UILabel *header;
@property (weak, nonatomic) IBOutlet UITableView *properties;
@property (weak, nonatomic) IBOutlet UILabel *manifold;
@property (weak, nonatomic) IBOutlet UILabel *census;

@property (strong, nonatomic) TriangulationViewController* viewer;
@property (assign, nonatomic) regina::NTriangulation* packet;
@end

@implementation TriRecognition

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.viewer = static_cast<TriangulationViewController*>(self.parentViewController);
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    self.packet = self.viewer.packet;

    [self.viewer updateHeader:self.header];

    manifoldName = nil;
    isHyp.clear();

    // Basic tests.
    if (! self.packet->isValid())
        isHyp = false;

    // Combinatorial recognition.
    {
        regina::NTriangulation simp(*self.packet);
        simp.intelligentSimplify();
        regina::NStandardTriangulation* std = regina::NStandardTriangulation::isStandardTriangulation(&simp);
        if (std) {
            regina::NManifold* mfd = std->getManifold();
            if (mfd) {
                isHyp = mfd->isHyperbolic();
                manifoldName = @(mfd->getName().c_str());
                delete mfd;

                // If we have the 3-sphere, 3-ball or solid torus, then
                // automatically run the large recognition routines: these
                // should finish quickly and give results consistent with
                // the combinatorial routines.
                if ([manifoldName isEqualToString:@"S3"]) {
                    self.packet->isThreeSphere();
                } else if ([manifoldName isEqualToString:@"B3"]) {
                    self.packet->isBall();
                } else if ([manifoldName isEqualToString:@"B2 x S1"]) {
                    self.packet->isSolidTorus();
                }
            }
            delete std;
        }
        if (manifoldName)
            self.manifold.text = manifoldName;
        else
            self.manifold.attributedText = [TextHelper dimString:@"Not recognised"];
    }

    // What properties should we display?
    // Precompute these properties now for sufficiently small triangulations.
    propertyList = [[NSMutableArray alloc] init];
    if (self.packet->isClosed()) {
        [propertyList addObject:@PROP_SPHERE];
        if (self.packet->getNumberOfTetrahedra() <= 6)
            self.packet->isThreeSphere();
    } else if (self.packet->getNumberOfBoundaryComponents() > 0) {
        // Real boundary only:
        if (self.packet->hasBoundaryTriangles()) {
            [propertyList addObject:@PROP_BALL];
            if (self.packet->getNumberOfTetrahedra() <= 6)
                self.packet->isBall();
        }
        // Either real or ideal boundary:
        [propertyList addObject:@PROP_SOLIDTORUS];
        if (self.packet->getNumberOfTetrahedra() <= 6)
            self.packet->isSolidTorus();
    }
    if (! dynamic_cast<regina::NSnapPeaTriangulation*>(self.packet)) {
        [propertyList addObject:@PROP_ZEROEFF];
        [propertyList addObject:@PROP_SPLITTING];
        if (self.packet->getNumberOfTetrahedra() <= 6) {
            self.packet->isZeroEfficient();
            self.packet->hasSplittingSurface();
        }
    }
    if (self.packet->isOrientable() && self.packet->isClosed() && self.packet->isValid() && self.packet->isConnected()) {
        [propertyList addObject:@PROP_IRREDUCIBLE];
        [propertyList addObject:@PROP_HAKEN];
        if (self.packet->getNumberOfTetrahedra() <= 6) {
            self.packet->isIrreducible();
            self.packet->isHaken();
        }
    }
    if (self.packet->isIdeal() && ! self.packet->hasBoundaryFaces()) {
        [propertyList addObject:@PROP_STRICT];
        [propertyList addObject:@PROP_HYPERBOLIC];
        if (self.packet->getNumberOfTetrahedra() <= 50)
            self.packet->hasStrictAngleStructure();
    }
    self.properties.dataSource = self;

    // Display the results of a census lookup.
    if (self.packet->getNumberOfTetrahedra() <= MAX_CENSUS_TRIANGULATION_SIZE) {
        regina::NCensusHits* hits = regina::NCensus::lookup(static_cast<regina::NTriangulation*>(self.packet)->isoSig());
        if (hits->count() == 0)
            self.census.attributedText = [TextHelper dimString:@"Not found"];
        else {
            NSMutableString* msg = [[NSMutableString alloc] init];
            const regina::NCensusHit* hit;
            for (hit = hits->first(); hit; hit = hit->next()) {
                if (hit != hits->first())
                    [msg appendString:@"\n"];
                [msg appendString:@(hit->name().c_str())];
            }
            self.census.text = msg;
        }
        delete hits;
    } else
        self.census.attributedText = [TextHelper dimString:@"Not found"];

    [self updateHyperbolic];
}

+ (NSString*)propertyName:(int)property
{
    switch (property) {
        case PROP_SPHERE: return @"3-sphere?";
        case PROP_BALL: return @"3-ball?";
        case PROP_SOLIDTORUS: return @"Solid torus?";
        case PROP_ZEROEFF: return @"0-efficient?";
        case PROP_SPLITTING: return @"Splitting surface?";
        case PROP_IRREDUCIBLE: return @"Irreducible?";
        case PROP_HAKEN: return @"Haken?";
        case PROP_STRICT: return @"Strict angle structure?";
        case PROP_HYPERBOLIC: return @"Hyperbolic?";
    }
    return nil;
}

- (void)updateHyperbolic
{
    if (isHyp.known())
        return;

    if (self.packet->isClosed() && self.packet->knowsThreeSphere() && self.packet->isThreeSphere())
        isHyp = false;
    else if (self.packet->hasBoundaryTriangles() && self.packet->knowsBall() && self.packet->isBall())
        isHyp = false;
    else if (self.packet->getNumberOfBoundaryComponents() > 0 && self.packet->knowsSolidTorus() && self.packet->isSolidTorus())
        isHyp = false;
    else if (self.packet->isOrientable() && self.packet->isClosed() && self.packet->isValid() && self.packet->isConnected() && self.packet->knowsIrreducible() && ! self.packet->isIrreducible())
        isHyp = false;
    else if (self.packet->isValid() && self.packet->isStandard() && self.packet->knowsStrictAngleStructure() && self.packet->hasStrictAngleStructure())
        isHyp = true;

    if (isHyp.known())
        [self.properties reloadRowsAtIndexPaths:@[[NSIndexPath indexPathForRow:[propertyList indexOfObject:@PROP_HYPERBOLIC] inSection:0]]
                               withRowAnimation:NO];
}

- (NSAttributedString*)value:(int)property
{
    switch (property) {
        case PROP_SPHERE:
            if (self.packet->knowsThreeSphere()) {
                if (self.packet->isThreeSphere() && ! manifoldName)
                    self.manifold.text = manifoldName = @"S3";
                return [TextHelper yesNoString:self.packet->isThreeSphere() yes:@"Yes" no:@"No"];
            }
            return nil;
        case PROP_BALL:
            if (self.packet->knowsBall()) {
                if (self.packet->isBall() && ! manifoldName)
                    self.manifold.text = manifoldName = @"B3";
                return [TextHelper yesNoString:self.packet->isBall() yes:@"Yes" no:@"No"];
            }
            return nil;
        case PROP_SOLIDTORUS:
            if (self.packet->knowsSolidTorus()) {
                if (self.packet->isSolidTorus() && ! manifoldName)
                    self.manifold.text = manifoldName = @"B2 x S1";
                return [TextHelper yesNoString:self.packet->isSolidTorus() yes:@"Yes" no:@"No"];
            }
            return nil;
        case PROP_ZEROEFF:
            if (self.packet->knowsZeroEfficient())
                return [TextHelper yesNoString:self.packet->isZeroEfficient() yes:@"Yes" no:@"No"];
            return nil;
        case PROP_SPLITTING:
            if (self.packet->knowsSplittingSurface() || self.packet->getNumberOfTetrahedra() <= 6)
                return [TextHelper yesNoString:self.packet->hasSplittingSurface() yes:@"Yes" no:@"No"];
            return nil;
        case PROP_IRREDUCIBLE:
            if (self.packet->knowsIrreducible())
                return [TextHelper yesNoString:self.packet->isIrreducible() yes:@"Yes" no:@"No"];
            return nil;
        case PROP_HAKEN:
            if (self.packet->knowsIrreducible() && ! self.packet->isIrreducible())
                return [TextHelper markedString:@"N/A"];
            else if (self.packet->knowsHaken())
                return [TextHelper yesNoString:self.packet->isHaken() yes:@"Yes" no:@"No"];
            return nil;
        case PROP_STRICT:
            if (self.packet->knowsStrictAngleStructure())
                return [TextHelper yesNoString:self.packet->hasStrictAngleStructure() yes:@"Yes" no:@"No"];
            return nil;
        case PROP_HYPERBOLIC:
            if (isHyp.known())
                return [TextHelper yesNoString:isHyp.value() yes:@"Yes" no:@"No"];
            return nil;
        default:
            return nil;
    }
}

- (void)calculate:(id)sender
{
    UIView *cell = [sender superview];
    while (! [cell isKindOfClass:[PropertyCell class]])
        cell = cell.superview;

    static_cast<PropertyCell*>(cell).calculate.hidden = YES;

    // TODO: Progress HUD.
    switch (static_cast<PropertyCell*>(cell).property) {
        case PROP_SPHERE:
            self.packet->isThreeSphere(); break;
        case PROP_BALL:
            self.packet->isBall(); break;
        case PROP_SOLIDTORUS:
            self.packet->isSolidTorus(); break;
        case PROP_ZEROEFF:
            self.packet->isZeroEfficient(); break;
        case PROP_SPLITTING:
            self.packet->hasSplittingSurface(); break;
        case PROP_IRREDUCIBLE:
            self.packet->isIrreducible(); break;
        case PROP_HAKEN:
            self.packet->isHaken(); break;
        case PROP_STRICT:
            self.packet->hasStrictAngleStructure(); break;
    }
    [self updateHyperbolic];
    [self.properties reloadData];
}

#pragma mark - Table view

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return NO;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return propertyList.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    int prop = [propertyList[indexPath.row] intValue];

    PropertyCell* cell = [tableView dequeueReusableCellWithIdentifier:@"Property" forIndexPath:indexPath];
    cell.property = prop;
    cell.name.text = [TriRecognition propertyName:prop];

    NSAttributedString* value = [self value:prop];

    if (value)
        cell.value.attributedText = value;
    else
        cell.value.text = @"Unknown";

    if (value || prop == PROP_HYPERBOLIC)
        cell.calculate.hidden = YES;
    else
        [cell.calculate addTarget:self action:@selector(calculate:) forControlEvents:UIControlEventTouchUpInside];
    cell.calculate.hidden = (value || prop == PROP_HYPERBOLIC);
    cell.progress.hidden = YES;
    return cell;
}

@end