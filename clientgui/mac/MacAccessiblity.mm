// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

//  MacAccessiblity.mm

#include "MacGUI.pch"
#include <Cocoa/Cocoa.h>
#include <objc/runtime.h>

#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "BOINCListCtrl.h"
#include "DlgEventLogListCtrl.h"
#include "ViewStatistics.h"
#include "wxPieCtrl.h"

#import <AppKit/NSAccessibility.h>

// I have not been able to get it to work when this is FALSE
#define RETAIN_KIDS_OF_ROWS TRUE

#if !wxOSX_USE_NATIVE_FLIPPED
#error: must convert to Quartz coordinates in accessibilityHitTest and NSAccessibilityPositionAttribute
#endif

#pragma mark === CBOINCListCtrl & CDlgEventLogListCtrl Accessibility Shared Code ===

#if USE_NATIVE_LISTCONTROL
#error: This code assumes wxGenericListCtrl
#endif

#if (DLG_LISTCTRL_BASE != wxGenericListCtrl)
#error: This code assumes wxGenericListCtrl
#endif

// Allowable column numbers are 0 to 15, but 15 is reserved to indicate a full row.
#define MAX_LIST_COL 16

#define isHeaderFlag (1<<0)
#define isEventLogFlag (1<<1)
#define isRowFlag (1<<2)

static UInt32 makeElementIdentifier(SInt32 row, SInt32 col) {
    UInt32 id = (((row + 1) * MAX_LIST_COL) + (col + 1)) * 2;
return id;
}

static void wxRectToNSRect(wxRect &wxr, NSRect &nsr) {
    nsr.origin.x = wxr.x;
    nsr.origin.y = wxr.y;
    nsr.size.width = wxr.width;
    nsr.size.height = wxr.height;
}


@interface  EventLogCellUIElement : NSObject {
    NSInteger row, col;
    NSInteger listFlags;
    BOOL isHeader;
    BOOL isEventLog;
    BOOL isRow;     // is this needed?
    wxGenericListCtrl *pList;
    id parent;
    NSView *listControlView;
    NSInteger headerHeight;
    CBOINCBaseView *BOINCView;


}
- (id)initWithRow:(NSInteger)aRow column:(NSInteger)acol listFlags:(NSInteger)flags listCtrl:(wxGenericListCtrl *)aListCtrl parent:(id)aParent BOINCView:(CBOINCBaseView *)aBOINCView;
+ (EventLogCellUIElement *)elementWithRow:(NSInteger)aRow column:(NSInteger)acol listFlags:(NSInteger)flags listCtrl:(wxGenericListCtrl *)aListCtrl parent:(id)aParent BOINCView:(CBOINCBaseView *)aBOINCView;
- (NSInteger)row;
- (NSInteger)col;

@end


@implementation EventLogCellUIElement

- (id)initWithRow:(NSInteger)aRow column:(NSInteger)acol listFlags:(NSInteger)flags listCtrl:(wxGenericListCtrl *)aListCtrl parent:(id)aParent BOINCView:(CBOINCBaseView *)aBOINCView {
    if (self = [super init]) {
        row = aRow;
        col = acol;
        listFlags = flags;
        isHeader = (flags & isHeaderFlag) ? YES : NO;
        isEventLog = (flags & isEventLogFlag) ? YES : NO;
        isRow = (flags & isRowFlag) ? YES : NO; // is this needed?
        pList = aListCtrl;
        parent = aParent;
        listControlView = pList->GetHandle();
        headerHeight = ((wxWindow *)(pList->m_headerWin))->GetSize().y;
        BOINCView = aBOINCView;
    }
    return self;
}

+ (EventLogCellUIElement *)elementWithRow:(NSInteger)aRow column:(NSInteger)acol listFlags:(NSInteger)flags listCtrl:(wxGenericListCtrl *)aListCtrl parent:(id)aParent BOINCView:(CBOINCBaseView *)aBOINCView {
#if RETAIN_KIDS_OF_ROWS
    return [[[EventLogCellUIElement alloc] initWithRow:aRow column:acol listFlags:flags listCtrl:aListCtrl parent:aParent BOINCView:aBOINCView] autorelease];
#else
    return [[[self alloc] initWithRow:aRow column:acol listFlags:flags listCtrl:aListCtrl parent:aParent BOINCView:aBOINCView] autorelease];
#endif
}

- (BOOL)accessibilityIsIgnored {
    return NO;
}

- (BOOL)isEqual:(id)object {
    if ([object isKindOfClass:[EventLogCellUIElement self]]) {
        EventLogCellUIElement *other = object;
        return (row == other->row) && (col == other->col) && (listFlags == other->listFlags) && [super isEqual:object];
    } else {
        return NO;
    }
}

//TODO: Is this ever actually called?
- (id)accessibilityHitTest:(NSPoint)point {
    NSPoint windowPoint;
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 1070
    //convertRectFromScreen is not available before OS 10.7
    if (! [[listControlView window] respondsToSelector: @selector(convertRectFromScreen:)]) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        windowPoint = [[listControlView window] convertScreenToBase:point];
#pragma clang diagnostic pop
    } else
#endif
    {
        NSRect r1 = NSMakeRect(point.x, point.y, 1, 1);
        NSRect r2 = [[listControlView window] convertRectFromScreen:r1];
        windowPoint = r2.origin;
    }

    NSPoint localPoint = [listControlView convertPoint:windowPoint fromView:nil];

    int i, x = 0, yoff;
    // First get the position relative to the ListCtrl
    pList->CalcScrolledPosition(0, 0, &x, &yoff);
    for (i=0; i<col; i++) {
        x += pList->GetColumnWidth(i);
    }
    NSRect r;
    r = [listControlView bounds];
    r.origin.x = x;
    r.size.width = pList->GetColumnWidth(col);
    if (isHeader) {
        r.size.height = headerHeight;
    } else {
        wxRect wxr;
        pList->GetItemRect(row, wxr);
        r.origin.y = wxr.y;
//        r.origin.y = r.size.height - wxr.y; // Convert to Quartz coordinates
        r.size.height = wxr.height;
    }
    if (NSPointInRect(localPoint, r)){
        return self;
    }
	return nil;
}

- (NSUInteger)hash {
    // Equal objects must hash the same.
    return [super hash] + makeElementIdentifier(isHeader ? row : row + 1, col);
}

- (NSInteger)row {
    return row;
}

- (NSInteger)col {
    return col;
}

- (void)dealloc {
    [super dealloc];
}

//
// accessibility protocol
//

// attributes

- (NSArray *)accessibilityAttributeNames {
    static NSArray *attributes = nil;
    if (attributes == nil) {
        attributes = [[NSArray alloc] initWithObjects:
                    NSAccessibilityWindowAttribute,
                    NSAccessibilityTopLevelUIElementAttribute,
                    NSAccessibilityDescriptionAttribute,
                    NSAccessibilitySizeAttribute,
                    NSAccessibilityPositionAttribute,
                    NSAccessibilityTitleAttribute,
                    NSAccessibilityEnabledAttribute,
                    NSAccessibilityFocusedAttribute,
                    NSAccessibilityRoleAttribute,
                    NSAccessibilityRoleDescriptionAttribute,
                    NSAccessibilityParentAttribute,
                    nil];
    }
    if (isHeader && !isEventLog) {
        NSMutableArray *temp = [attributes mutableCopy];
        [temp addObject:NSAccessibilitySubroleAttribute];
        [temp addObject:NSAccessibilitySortDirectionAttribute];
        return [temp copy];
    }
    return attributes;
}

- (id)accessibilityAttributeValue:(NSString *)attribute {
    if ([attribute isEqualToString:NSAccessibilityParentAttribute]) {
        return NSAccessibilityUnignoredAncestor(parent);

    } else if ([attribute isEqualToString:NSAccessibilityWindowAttribute]) {
        return [listControlView window];

    } else if ([attribute isEqualToString:NSAccessibilityTopLevelUIElementAttribute]) {
        return [listControlView window];

    } else if ([attribute isEqualToString:NSAccessibilityDescriptionAttribute]) {
        NSString *desc;
        wxString s;
        BOOL isCurrentSortCol = false;

        if (isHeader) {
            int numCols = pList->GetColumnCount();
            if ((!isEventLog) && (BOINCView != nil)) {
               if (col == BOINCView->m_iColumnIDToColumnIndex[BOINCView->m_iSortColumnID]) {
                    isCurrentSortCol = YES;
                }
            }
            if (isCurrentSortCol) {
                if (BOINCView->m_bReverseSort) {
                    s.Printf(_("(current sort column %d of %d; descending order)"), (int)col+1, (int)numCols);
                } else {
                    s.Printf(_("(current sort column %d of %d; ascending order)"), (int)col+1, (int)numCols);
                }
            } else {
                s.Printf(_("(column %d of %d)"), (int)col+1, (int)numCols);
            }
        } else {
            if (pList->GetItemState(row, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) {
                if (col == 0) {
                    s.Printf(_("(selected row %d of %d)"), (int)row+1, (int)(pList->GetItemCount()));
                } else {
                    s.Printf(_("(selected row %d)"), (int)row+1);
                }
            } else {
                if (col == 0) { // Row is not selected
                    s.Printf(_("(row %d of %d)"), (int)row+1, (int)(pList->GetItemCount()));
                } else {
                    s.Printf(_("(row %d)"), (int)row+1);
                }
            }
        }
        desc = [NSString stringWithUTF8String:(char *)(s.utf8_str().data())];
        return desc;

    } else if ([attribute isEqualToString:NSAccessibilitySizeAttribute]) {
        NSSize sz;
        sz.width = pList->GetColumnWidth(col);
        if (isHeader) {
            sz.height = headerHeight;
        } else {
            wxRect r;
            pList->GetItemRect(row, r);
            sz.height = r.height;
        }
        return [NSValue valueWithSize:sz];

    } else if ([attribute isEqualToString:NSAccessibilityPositionAttribute]) {
        int i, xoff, yoff;
        NSPoint pt;
        pt.x = 0;
        pt.y = 0;
        // First get the position relative to the ListCtrl
        for (i=0; i<col; i++) {
            pt.x += pList->GetColumnWidth(i);
        }
        if (!isHeader) {
            wxRect r;
            pList->GetItemRect(row, r);
            pt.y = r.y;
       }
        pList->CalcScrolledPosition(0, 0, &xoff, &yoff);
        pt.x += xoff;
        pt.y += headerHeight;
//        pt.y = [listControlView bounds].size.height - headerHeight - pt.y; // Convert to Quartz coordinates

		//Convert the point to global (screen) coordinates
		NSPoint windowPoint = [listControlView convertPoint:pt toView: nil];
		pt = [[listControlView window] convertBaseToScreen:windowPoint];

        return [NSValue valueWithPoint:pt];

    } else if ([attribute isEqualToString:NSAccessibilityTitleAttribute]) {
        wxString str;
        if (isHeader) {
            wxListItem headerItem;
            pList->GetColumn(col, headerItem);
            str = headerItem.GetText();
        } else {
            str = pList->GetItemText(row, col);
            if (str.IsEmpty()) {
                str = _("blank");
            }
        }
        char *s = (char *)(str.utf8_str().data());
        NSString *text = [[NSString alloc] initWithUTF8String:s];
        return text;

    } else if ([attribute isEqualToString:NSAccessibilityEnabledAttribute]) {
        return [NSNumber numberWithBool:YES];

    } else if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        return [NSNumber numberWithBool:NO];

    } else if ([attribute isEqualToString:NSAccessibilityRoleAttribute]) {
        return NSAccessibilityStaticTextRole;

    } else if ([attribute isEqualToString:NSAccessibilityRoleDescriptionAttribute]) {
        return NSAccessibilityRoleDescription(NSAccessibilityStaticTextRole,
                    (isHeader && !isEventLog) ? NSAccessibilitySortButtonSubrole : nil);

    } else if ([attribute isEqualToString:NSAccessibilitySubroleAttribute]) {
        return NSAccessibilitySortButtonSubrole;

    } else if ([attribute isEqualToString:NSAccessibilitySortDirectionAttribute]) {
        if (col == BOINCView->m_iColumnIDToColumnIndex[BOINCView->m_iSortColumnID]) {
            return BOINCView->m_bReverseSort ?
                NSAccessibilityDescendingSortDirectionValue : NSAccessibilityAscendingSortDirectionValue;
        } else {
            return NSAccessibilityUnknownSortDirectionValue;
        }

    } else {
        return nil;
    }
}

- (BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
 // return [super accessibilityIsAttributeSettable:attribute];
 	return NO;
}

- (NSArray *)accessibilityActionNames {
    // All cells except EventLog Header accept press action
    if (!isHeader || !isEventLog) {
        return [NSArray arrayWithObject:NSAccessibilityPressAction];
    }
    return [NSArray array];
}

- (NSString *)accessibilityActionDescription:(NSString *)action {
    return NSAccessibilityActionDescription(action);
}

- (void)accessibilityPerformAction:(NSString *)action {
    if (isHeader) {
        wxWindowID id = pList->GetId();
        wxListEvent event(wxEVT_COMMAND_LIST_COL_CLICK, id);
        event.m_col = col;
        pList->GetEventHandler()->AddPendingEvent(event);
    } else {
        int i = -1; // Step through all selected items and deselect each one
        while (1) {
            i = pList->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (i < 0) break;
            pList->SetItemState(i, 0, wxLIST_STATE_SELECTED);
        }
        // Select the one row the user "clicked" on
        pList->SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
}

@end


@interface FauxListRow : NSObject {
    NSInteger row;
    BOOL isEventLog;
    wxGenericListCtrl *pList;
#if RETAIN_KIDS_OF_ROWS
    NSMutableArray *kids;
    NSInteger numkids;
#endif
    id parent;
    NSView *listControlView;
}

+ (FauxListRow *)listRowWithRow:(NSInteger)aRow listFlags:(NSInteger)flags listCtrl:(wxGenericListCtrl *)aListCtrl parent:(id)aParent;
- (id)initWithRow:(NSInteger)aRow listFlags:(NSInteger)flags listCtrl:(wxGenericListCtrl *)aListCtrl parent:(id)aParent;
#if RETAIN_KIDS_OF_ROWS
- (void) adjustKidsIfNeeded;
#endif
@end


@implementation FauxListRow

- (id)initWithRow:(NSInteger)aRow listFlags:(NSInteger)flags listCtrl:(wxGenericListCtrl *)aListCtrl parent:(id)aParent {
    if (self = [super init]) {
        row = aRow;
        pList = aListCtrl;
        isEventLog = (flags & isEventLogFlag) ? YES : NO;
#if RETAIN_KIDS_OF_ROWS
        numkids = 0;
        kids = nil;
#endif
        parent = aParent;
        listControlView = pList->GetHandle();
    }
	return self;
}

+ (FauxListRow *)listRowWithRow:(NSInteger)aRow listFlags:(NSInteger)flags listCtrl:(wxGenericListCtrl *)aListCtrl parent:(id)aParent {
#if RETAIN_KIDS_OF_ROWS
    return [[[FauxListRow alloc]initWithRow:aRow listFlags:flags listCtrl:aListCtrl parent:aParent] autorelease];
#else
    return [[[self alloc]initWithRow:aRow listFlags:flags listCtrl:aListCtrl parent:aParent] autorelease];
#endif
}

- (BOOL)accessibilityIsIgnored {
    return NO;
}

- (BOOL)isEqual:(id)object {
    if ([object isKindOfClass:[FauxListRow self]]) {
        FauxListRow *other = object;
        return (row == other->row) && (isEventLog == other->isEventLog) && [super isEqual:object];
    } else {
        return NO;
    }
}

- (id)accessibilityHitTest:(NSPoint)point {
    NSPoint windowPoint;
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 1070
    //convertRectFromScreen is not available before OS 10.7
    if (! [[listControlView window] respondsToSelector: @selector(convertRectFromScreen:)]) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        windowPoint = [[listControlView window] convertScreenToBase:point];
#pragma clang diagnostic pop
    } else
#endif
    {
        NSRect r1 = NSMakeRect(point.x, point.y, 1, 1);
        NSRect r2 = [[listControlView window] convertRectFromScreen:r1];
        windowPoint = r2.origin;
    }

    NSPoint localPoint = [listControlView convertPoint:windowPoint fromView:nil];

//TODO: should we just generate temporary EventLogCellUIElement objects as needed?
#if RETAIN_KIDS_OF_ROWS
    [self adjustKidsIfNeeded];
#endif

    NSRect r;
    wxRect wxcr = pList->GetClientRect();
    wxRect wxr;
    pList->GetItemRect(row, wxr);

    int col, numCols, x = 0, yoff;
    pList->CalcScrolledPosition(0, 0, &x, &yoff);

    numCols = pList->GetColumnCount();
    for (col=0; col<numCols; col++) {
        // First get the position relative to the ListCtrl
        r = [listControlView bounds];
        r.origin.x = x;
        r.size.width = pList->GetColumnWidth(col);
        r.origin.y = wxr.y;
//        r.origin.y = r.size.height - wxr.y; // Convert to Quartz coordinates
        r.size.height = wxr.height;
        if (NSPointInRect(localPoint, r)){
#if RETAIN_KIDS_OF_ROWS
            return (EventLogCellUIElement *)[kids objectAtIndex:col];
#else
        NSInteger myFlags = isRowFlag | (isEventLog ? isEventLogFlag : 0);
        EventLogCellUIElement * cell = [EventLogCellUIElement elementWithRow:row column:col listFlags:myFlags listCtrl:pList parent:self BOINCView:nil];
        return cell;
#endif
        }

        x += r.size.width;
    }

    r = [listControlView bounds];
    r.origin.x = 0;
    r.size.width = wxcr.width;
    r.size.height = wxcr.height;
    r.origin.y = wxr.y;
//    r.origin.y = r.size.height - wxr.y; // Convert to Quartz coordinates
    r.size.height = wxr.height;
    if (NSPointInRect(localPoint, r)){
        return self;
    }
//    return [super accessibilityHitTest:point];
	return nil;
}


- (NSUInteger)hash {
    // Equal objects must hash the same.
    return [super hash] + makeElementIdentifier(row, MAX_LIST_COL-1);
}

#if RETAIN_KIDS_OF_ROWS
- (void)adjustKidsIfNeeded {
    if (kids == nil) {
        numkids = pList->GetColumnCount();
        kids = [[NSMutableArray arrayWithCapacity:numkids] retain];
        // The number of columns never changes
        NSInteger myFlags = isRowFlag | (isEventLog ? isEventLogFlag : 0);
        for (NSInteger i = 0; i < numkids; ++i) {
            EventLogCellUIElement * cell = [EventLogCellUIElement elementWithRow:row column:i listFlags:myFlags listCtrl:pList parent:self BOINCView:nil];
            [cell retain];
            [kids addObject:cell];
        }
    }
}
#endif

//
// accessibility protocol
//

// attributes

- (NSArray *)accessibilityAttributeNames {
    static NSArray *attributes = nil;
    if (attributes == nil) {
        attributes = [[NSArray alloc] initWithObjects:
                    NSAccessibilityRoleAttribute,
                    NSAccessibilityRoleDescriptionAttribute,
                    NSAccessibilitySubroleAttribute,
//                    NSAccessibilityDescriptionAttribute,
                    NSAccessibilityTitleAttribute,
                    NSAccessibilityEnabledAttribute,
                    NSAccessibilityFocusedAttribute,
                    NSAccessibilityParentAttribute,
                    NSAccessibilityWindowAttribute,
                    NSAccessibilityTopLevelUIElementAttribute,
                    NSAccessibilityPositionAttribute,
                    NSAccessibilitySizeAttribute,
                    NSAccessibilityChildrenAttribute,
                    NSAccessibilityVisibleChildrenAttribute,
                    NSAccessibilityWindowAttribute,
                    NSAccessibilityIndexAttribute,
                    NSAccessibilitySelectedAttribute,
                    nil];
    }
    return attributes;
}

- (id)accessibilityAttributeValue:(NSString *)attribute {
    if ([attribute isEqualToString:NSAccessibilityRoleAttribute]) {
        return NSAccessibilityRowRole;

    } else if ([attribute isEqualToString:NSAccessibilityRoleDescriptionAttribute]) {
        return NSAccessibilityRoleDescription(NSAccessibilityRowRole, nil);

    } else if ([attribute isEqualToString:NSAccessibilitySubroleAttribute]) {
        return NSAccessibilityTableRowSubrole;

    } else if ([attribute isEqualToString:NSAccessibilityTopLevelUIElementAttribute]) {
        return [parent window];

    } else if ([attribute isEqualToString:NSAccessibilityDescriptionAttribute]) {
        wxString s;
        s.Printf(_("(row %d)"), (int)row+1);
        NSString *desc = [NSString stringWithUTF8String:(char *)(s.utf8_str().data())];
        return desc;

    } else if ([attribute isEqualToString:NSAccessibilitySizeAttribute]) {
        NSSize sz;
        wxRect r;
        pList->GetItemRect(row, r);
        sz.height = r.height;
        sz.width = r.width;
        return [NSValue valueWithSize:sz];

    } else if ([attribute isEqualToString:NSAccessibilityPositionAttribute]) {
        int xoff, yoff;
        NSPoint pt;
        // First get the position relative to the ListCtrl
        wxRect r;
        pList->GetItemRect(row, r);
        pList->CalcScrolledPosition(0, 0, &xoff, &yoff);
        pt.x = xoff;
        pt.y = r.y;
//        pt.y = [parent bounds].size.height - pt.y; // Convert to Quartz coordinates

        //Convert the point to global (screen) coordinates
        NSPoint windowPoint = [parent convertPoint:pt toView: nil];
        pt = [[parent window] convertBaseToScreen:windowPoint];

        return [NSValue valueWithPoint:pt];

    } else if ([attribute isEqualToString:NSAccessibilityTitleAttribute]) {
        wxString s;
        s.Printf(_("row %d"), (int)row+1);
        NSString *title = [NSString stringWithUTF8String:(char *)(s.utf8_str().data())];
        return title;

    } else if ([attribute isEqualToString:NSAccessibilityEnabledAttribute]) {
        return [NSNumber numberWithBool:YES];

    } else if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        return [NSNumber numberWithBool:NO];

    } else if ([attribute isEqualToString:NSAccessibilityParentAttribute]) {
        return NSAccessibilityUnignoredAncestor(parent);

    } else if ([attribute isEqualToString:NSAccessibilityChildrenAttribute]) {
#if RETAIN_KIDS_OF_ROWS
        [self adjustKidsIfNeeded];
#else
        NSInteger i;
        NSInteger numkids = pList->GetColumnCount();

        NSMutableArray *kids = [NSMutableArray arrayWithCapacity:numkids];  // autorelease ??
        NSInteger myFlags = isRowFlag | (isEventLog ? isEventLogFlag : 0);
        for (i = 0; i < numkids; ++i) {
            EventLogCellUIElement * cell = [EventLogCellUIElement elementWithRow:row column:i listFlags:myFlags listCtrl:pList parent:self BOINCView:nil];   // Retain???
            [kids addObject:cell];
        }
#endif
        return kids;
//        return NSAccessibilityUnignoredChildren(kids);    // No children are ignored

    } else if ([attribute isEqualToString:NSAccessibilityVisibleChildrenAttribute]) {
        int w, h, xoff, yoff;
        pList->GetClientSize(&w, &h);
        pList->CalcUnscrolledPosition(0, 0, &xoff, &yoff);
        int leftVisiblePos = xoff;      // Horizontal scrolled offset in pixels
        int rightVisiblePos = leftVisiblePos + w;

        NSInteger i, cellLeftEdge = 0, cellRightEdge;
        NSMutableArray *visibleChildren = [NSMutableArray array];

#if RETAIN_KIDS_OF_ROWS
        [self adjustKidsIfNeeded];
#else
        NSInteger numkids = pList->GetColumnCount();
#endif
        for (i = 0; i < numkids; ++i) {
            BOOL isVisible = NO;
            cellRightEdge = cellLeftEdge + pList->GetColumnWidth(i);
            if ((leftVisiblePos <= cellLeftEdge) && rightVisiblePos >= cellLeftEdge) {
                isVisible = YES;        // Left edge of cell is in visible area of row
            } else if ((leftVisiblePos <= cellRightEdge) && rightVisiblePos >= cellRightEdge) {
                isVisible = YES;        // Right edge of cell is in visible area of row
            } else if ((leftVisiblePos > cellLeftEdge) && (rightVisiblePos < cellRightEdge)) {
                isVisible = YES;        // Visible area of row is totally within cell
            }

            if (isVisible) {
#if RETAIN_KIDS_OF_ROWS
                [visibleChildren addObject:[kids objectAtIndex:i] ];
#else
                NSInteger myFlags = isRowFlag | (isEventLog ? isEventLogFlag : 0);
                EventLogCellUIElement * cell = [EventLogCellUIElement elementWithRow:row column:i listFlags:myFlags listCtrl:pList parent:self BOINCView:nil];   // Retain???
                [visibleChildren addObject:cell];
#endif
            }

            cellLeftEdge = cellRightEdge;
        }

        return visibleChildren;

    } else if ([attribute isEqualToString:NSAccessibilityWindowAttribute]) {
        return [parent window];

    } else if ([attribute isEqualToString:NSAccessibilityIndexAttribute]) {
        return [NSNumber numberWithInt:row];

    } else if ([attribute isEqualToString:NSAccessibilitySelectedAttribute]) {
        BOOL isSelected = pList->GetItemState(row, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED;
        return [NSNumber numberWithBool:isSelected];

    } else {
        return nil;
//        return [super accessibilityAttributeValue:attribute];
    }
}

- (BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
 //   return [super accessibilityIsAttributeSettable:attribute];
 	return NO;
}

- (NSArray *)accessibilityActionNames {
    return [NSArray array];
}

- (NSString *)accessibilityActionDescription:(NSString *)action {
    return nil;
}

- (void)accessibilityPerformAction:(NSString *)action {
}

- (void)dealloc {
#if RETAIN_KIDS_OF_ROWS
    for (NSInteger i = 0; i < numkids; ++i) {
        [(EventLogCellUIElement*)[kids objectAtIndex:i] release];
    }
//TODO: Do we need to delete or free up kids array?
    if (kids) [kids release];
#endif
    [super dealloc];
}

@end



@interface FauxListBodyView : NSView {
    BOOL isHeader;
    BOOL isEventLog;
    wxGenericListCtrl *pList;
    NSMutableArray *kids;
    NSInteger numkids;
    id parent;
    CBOINCBaseView *BOINCView;
}

- (id)initWithFrame:(NSRect)frame listCtrl:(wxGenericListCtrl *)aListCtrl listFlags:(NSInteger)flags parent:aParent BOINCView:(CBOINCBaseView *)aBOINCView;
- (void) adjustKidsIfNeeded;

@end


@implementation FauxListBodyView

- (id)initWithFrame:(NSRect)frame listCtrl:(wxGenericListCtrl *)aListCtrl listFlags:(NSInteger)flags parent:aParent BOINCView:(CBOINCBaseView *)aBOINCView {
	if ((self = [super initWithFrame:frame]) != nil) {
        pList = aListCtrl;
        isEventLog = (flags & isEventLogFlag) ? YES : NO;
        numkids = 0;
        kids = nil;
        parent = aParent;
        BOINCView = aBOINCView;
    }
	return self;
}

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)accessibilityIsIgnored {
    return NO;
}

- (id)accessibilityHitTest:(NSPoint)point {
    NSInteger rowNumber;
    id item;

    [self adjustKidsIfNeeded];

    NSPoint windowPoint;
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 1070
    //convertRectFromScreen is not available before OS 10.7
    if (! [[parent window] respondsToSelector: @selector(convertRectFromScreen:)]) {  // parent == listControlView
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        windowPoint = [[parent window] convertScreenToBase:point];
#pragma clang diagnostic pop
    } else
#endif
    {
        NSRect r1 = NSMakeRect(point.x, point.y, 1, 1);
        NSRect r2 = [[parent window] convertRectFromScreen:r1];
        windowPoint = r2.origin;
    }

    NSPoint localPoint = [parent convertPoint:windowPoint fromView:nil];

    // The scroll bars are among the list control's subviews.
    // If we are outside the list control's client area, determine
    // which scroll bar we are over and return the info for it.
    wxRect wxcr = pList->GetClientRect();
    NSRect cr;
    wxRectToNSRect(wxcr, cr);
    if (!NSPointInRect(localPoint, cr)) {
        NSArray *mySubViews = [parent subviews];
        NSInteger nSubViews = [mySubViews count];
        for (NSInteger i=0; i<nSubViews; i++) {
            NSView *aView = [mySubViews objectAtIndex:i];
            if ([aView isKindOfClass:[NSScroller class]]) {
                if (NSPointInRect(localPoint, [aView frame ])) {
                    return [[mySubViews objectAtIndex:i] accessibilityHitTest:point];
                }
            }
        }
    }

    if (kids) {
        // Check only rows that are visible
        NSInteger firstVisibleRow = pList->GetTopItem();
        NSInteger numVisibleRows = pList->GetCountPerPage();

        if (numkids <= (firstVisibleRow + numVisibleRows)) {
            numVisibleRows = numkids - firstVisibleRow;
        }
        // This allows for an additional partially visible row
        NSInteger lastVisibleRow = firstVisibleRow + numVisibleRows;
        if (lastVisibleRow > (numkids - 1)) {
            lastVisibleRow = (numkids - 1);
        }

        for (rowNumber = firstVisibleRow; rowNumber <= lastVisibleRow; ++rowNumber) {
            NSRect r;
            r = [parent bounds];
            r.origin.x = 0;
            r.size.width = wxcr.width;
            r.size.height = wxcr.height;

            wxRect wxr;
            pList->GetItemRect(rowNumber, wxr);
            r.origin.y = wxr.y;
        //    r.origin.y = r.size.height - wxr.y; // Convert to Quartz coordinates
            r.size.height = wxr.height;
            if (!NSPointInRect(localPoint, r)) continue;

//TODO: should we just generate a temporary FauxListRow object as needed?
            FauxListRow *row = (FauxListRow *)[kids objectAtIndex:rowNumber];
            item = [row accessibilityHitTest:point];
            if (item != nil) {
                return item;
            }
        }
    }

    return self;
//	return [super accessibilityHitTest:point];
}

- (void)adjustKidsIfNeeded {
    NSInteger i, newNumKids;
    newNumKids = pList->GetItemCount(); // Number of rows

    if (kids == nil) {
        kids = [[NSMutableArray arrayWithCapacity:newNumKids] retain];
    }
    if (newNumKids < numkids) {
        for (i = numkids; i > newNumKids; i--) {
            [[kids objectAtIndex:i-1] release];
            [kids removeLastObject];
        }
        numkids = newNumKids;
    } else if (newNumKids > numkids) {
        NSInteger myFlags = isRowFlag | (isEventLog ? isEventLogFlag : 0);
        for (i = numkids; i < newNumKids; ++i) {
            FauxListRow *row = [FauxListRow listRowWithRow:i listFlags:myFlags listCtrl:pList parent:self];
            [row retain];
            [kids addObject:row];
        }
        numkids = newNumKids;
    }
}


//
// accessibility protocol
//

// attributes

- (NSArray *)accessibilityAttributeNames {
    static NSArray *attributes = nil;
    if (attributes == nil) {
        attributes = [[NSArray alloc] initWithObjects:
                    NSAccessibilityRoleAttribute,
                    NSAccessibilityRoleDescriptionAttribute,
                    NSAccessibilityFocusedAttribute,
                    NSAccessibilityParentAttribute,
                    NSAccessibilityWindowAttribute,
                    NSAccessibilityTopLevelUIElementAttribute,
                    NSAccessibilityPositionAttribute,
                    NSAccessibilitySizeAttribute,
                    NSAccessibilityChildrenAttribute,
                    NSAccessibilityVisibleChildrenAttribute,
                    NSAccessibilitySelectedChildrenAttribute,
                    NSAccessibilitySelectedRowsAttribute,
                    NSAccessibilityOrientationAttribute,
// NSAccessibilityHeaderAttribute,
// NSAccessibilityColumnsAttribute,
// NSAccessibilityRowsAttribute,
// NSAccessibilitySelectedColumnsAttribute,
// NSAccessibilityVisibleColumnsAttribute
// NSAccessibilityVisibleRowsAttribute,
                    NSAccessibilityDescriptionAttribute,
                    nil];
    }
    return attributes;
}

- (id)accessibilityAttributeValue:(NSString *)attribute {
    if ([attribute isEqualToString:NSAccessibilityRoleAttribute]) {
        return NSAccessibilityListRole;

    } else if ([attribute isEqualToString:NSAccessibilityRoleDescriptionAttribute]) {
        return NSAccessibilityRoleDescription(NSAccessibilityListRole, nil);

    } else if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        return [NSNumber numberWithBool:NO];

    } else if ([attribute isEqualToString:NSAccessibilityParentAttribute]) {
        return NSAccessibilityUnignoredAncestor(parent);

    } else if ([attribute isEqualToString:NSAccessibilityWindowAttribute]) {
	// We're in the same window as our parent.
        return [parent window];

    } else if ([attribute isEqualToString:NSAccessibilityTopLevelUIElementAttribute]) {
        return [parent window];

    } else if ([attribute isEqualToString:NSAccessibilityDescriptionAttribute]) {
        // To allow localization, we can't just append string
        // " is empty" because that assumes English word order.
        wxString s;
        if (isEventLog) {
            if (numkids) {
                s = _("Event Log");
            } else {
                s = _("Event Log is empty");
            }
        } else {
            wxString viewName = BOINCView->GetViewDisplayName();
            if (numkids) {
                s.Printf(_("%s"), viewName.c_str());
            } else {
                s.Printf(_("You currently have no %s"), viewName.c_str());
            }
        }
        NSString *desc = [NSString stringWithUTF8String:(char *)(s.utf8_str().data())];
        return desc;

    } else if ([attribute isEqualToString:NSAccessibilitySizeAttribute]) {
        return [NSValue valueWithSize:[parent frame].size];

    } else if ([attribute isEqualToString:NSAccessibilityPositionAttribute]) {
        NSPoint pt = [parent bounds].origin;
        pt.y += [parent bounds].size.height;    // We need the bottom left corner
		//Convert the point to global (screen) coordinates
		NSPoint windowPoint = [parent convertPoint:pt toView: nil];
		pt = [[parent window] convertBaseToScreen:windowPoint];

        return [NSValue valueWithPoint:pt];

    } else if ([attribute isEqualToString:NSAccessibilityChildrenAttribute]) {
        [self adjustKidsIfNeeded];
        return kids;
//        return NSAccessibilityUnignoredChildren(kids);    // No children are ignored

    } else if ([attribute isEqualToString:NSAccessibilityVisibleChildrenAttribute]) {
        NSInteger i;
        NSInteger firstVisibleKid = pList->GetTopItem();
        NSInteger numVisibleKids = pList->GetCountPerPage();
        NSMutableArray *visibleChildren = [NSMutableArray array];

        [self adjustKidsIfNeeded];

//TODO: allow for additional partially visible rows at bottom??
        if (numkids <= (firstVisibleKid + numVisibleKids)) {
            numVisibleKids = numkids - firstVisibleKid;
        }
        for (i = 0; i < numVisibleKids; ++i) {
            [visibleChildren addObject:[kids objectAtIndex:i+firstVisibleKid] ];
        }

        return visibleChildren;

    } else if ([attribute isEqualToString:NSAccessibilitySelectedChildrenAttribute] ||
                    [attribute isEqualToString:NSAccessibilitySelectedRowsAttribute]) {
        NSMutableArray *selectedChildren = [NSMutableArray array];

        int i = -1;                  // Step through all selected items
        while (1) {
            i = pList->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (i < 0) break;
            [selectedChildren addObject:[kids objectAtIndex:i] ];
        }
        return selectedChildren;

    } else if ([attribute isEqualToString:NSAccessibilityOrientationAttribute]) {
        return NSAccessibilityVerticalOrientationValue;

    } else {
        return [super accessibilityAttributeValue:attribute];
    }
}

- (NSArray *)accessibilityActionNames {
    return [NSArray arrayWithObject:NSAccessibilityPressAction];
}

- (NSString *)accessibilityActionDescription:(NSString *)action {
    return NSAccessibilityActionDescription(action);
}

// If user does a simulated click below last row, deselect all rows
- (void)accessibilityPerformAction:(NSString *)action {
    if ([self accessibilityHitTest:[NSEvent mouseLocation]] == self) {
        int i = -1; // Step through all selected items and deselect each one
        while (1) {
            i = pList->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (i < 0) break;
            pList->SetItemState(i, 0, wxLIST_STATE_SELECTED);
        }
    }
}

- (void)dealloc {
//TODO: remove this if we don't retain rows in kids
    for (NSInteger i = 0; i < numkids; ++i) {
        [(FauxListRow *)[kids objectAtIndex:i] release];
    }
//TODO: Do we need to delete or free up kids array?
    if (kids) [kids release];
    [super dealloc];
}

@end


@interface FauxListHeaderView : NSView {
    wxGenericListCtrl *pList;
    BOOL isEventLog;
    NSMutableArray *kids;
    NSInteger numkids;
    id parent;
    CBOINCBaseView *BOINCView;
}

- (id)initWithFrame:(NSRect)frame listCtrl:(wxGenericListCtrl *)aListCtrl listFlags:(NSInteger)flags parent:aParent BOINCView:(CBOINCBaseView *)aBOINCView;

@end


@implementation FauxListHeaderView

- (id)initWithFrame:(NSRect)frame listCtrl:(wxGenericListCtrl *)aListCtrl listFlags:(NSInteger)flags parent:aParent BOINCView:(CBOINCBaseView *)aBOINCView
{
	if ((self = [super initWithFrame:frame]) != nil) {
        pList = aListCtrl;
        isEventLog = (flags & isEventLogFlag) ? YES : NO;
        numkids = 0;
        kids = nil;
        parent = aParent;
        BOINCView = aBOINCView;
    }
	return self;
}

- (BOOL)isFlipped {
    return YES;
}

// wxWidgetCocoaImpl::mouseEvent() discards NSMouseMoved events unless
// they are for the deepest child in the hierarchy, so the cursor is
// not adjusted over wxListCtrl header column separators unless hitTest
// reports the real wxListHeaderWindow as the deepest (topmost) view.
// But the Accessibility logic doesn work unless hitTest reports our
// FauxListHeaderView as the deepest (topmost) view.
// To work around this, we make the real wxListHeaderWindow a subview
// of our FauxListHeaderView, but FauxListHeaderView reports itself
// as the deepest view when [NSWindowAccessibility accessibilityHitTest]
// is somewhere in the caller chain.
// I wish I could find a more efficient way to do this.
//
static BOOL AccessibilityEnabled = false;

- (NSView *)hitTest:(NSPoint)aPoint {
    // [NSThread callStackSymbols] is not available in OS 10.5, so
    // BOINC does not fully implement accessibility under OS 10.5.
    //
    // Weak linking of objective-C classes and methods is not
    // supported before OS 10.6.8 so to be compatible with
    // OS 10.5 we must test availability at run time.
    //
    static BOOL firstTime = true;
    static BOOL haveMethod = false;

    if (AccessibilityEnabled) {
        if (firstTime) {
            IMP callStackSyms = class_getMethodImplementation(objc_getClass("NSThread"), @selector(callStackSymbols));
            haveMethod = (callStackSyms != nil);
            firstTime = false;
        }

        if (!haveMethod) {
            return [super hitTest:aPoint];
        }

        NSRect r = [parent bounds];
        r.size.height = [self bounds].size.height;
        if (!NSPointInRect(aPoint, r)){
            return [super hitTest:aPoint];  // Point is not within our rect
        }

    //    NSArray *theStack = [NSThread callStackSymbols];
        NSArray *theStack = [ NSThread performSelector:@selector(callStackSymbols) ];

        int limit = [ theStack count ];
        int i = 0;
        do {
            if (limit < (i+1)) break;
            NSString *sourceString = [theStack objectAtIndex:i];
            NSCharacterSet *separatorSet = [NSCharacterSet characterSetWithCharactersInString:@" -[]+?.,"];
            NSMutableArray *array = [NSMutableArray arrayWithArray:[sourceString componentsSeparatedByCharactersInSet:separatorSet]];
            [array removeObject:@""];

            if ([array count] >= 5) {
                NSString *FunctionCaller = [array objectAtIndex:4];
                if ([ FunctionCaller hasPrefix: @"accessibility"]) {
                    return self;
                }

            }
            ++i;
        } while (i < 15);
    }

    return [super hitTest:aPoint];  // Not an accessibility call
}


- (BOOL)accessibilityIsIgnored {
    AccessibilityEnabled = true;
    return NO;
}

- (id)accessibilityHitTest:(NSPoint)point {
    NSPoint windowPoint;
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 1070
    //convertRectFromScreen is not available before OS 10.7
    if (! [[parent window] respondsToSelector: @selector(convertRectFromScreen:)]) {  // parent == listControlView
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        windowPoint = [[parent window] convertScreenToBase:point];
#pragma clang diagnostic pop
    } else
#endif
    {
        NSRect r1 = NSMakeRect(point.x, point.y, 1, 1);
        NSRect r2 = [[parent window] convertRectFromScreen:r1];
        windowPoint = r2.origin;
    }

    NSPoint localPoint = [parent convertPoint:windowPoint fromView:nil];

    // The scroll bars are among the list control's subviews.
    // If we are outside the list control's client area, determine
    // which scroll bar we are over and return the info for it.
    wxRect wxcr = pList->GetClientRect();
    NSRect cr;
    wxRectToNSRect(wxcr, cr);
    if (!NSPointInRect(localPoint, cr)) {
        NSArray *mySubViews = [parent subviews];
        NSInteger nSubViews = [mySubViews count];
        for (NSInteger i=0; i<nSubViews; i++) {
            NSView *aView = [mySubViews objectAtIndex:i];
            if ([aView isKindOfClass:[NSScroller class]]) {
                if (NSPointInRect(localPoint, [aView frame ])) {
                    return [[mySubViews objectAtIndex:i] accessibilityHitTest:point];
                }
            }
        }
    }

    if (kids) {
        int col, numCols, x = 0, yoff;
        pList->CalcScrolledPosition(0, 0, &x, &yoff);

        numCols = pList->GetColumnCount();
        for (col=0; col<numCols; col++) {
            // First get the position relative to the ListCtrl
            NSRect r = [parent bounds];
            r.origin.x = x;
            r.size.width = pList->GetColumnWidth(col);
            r.size.height = [self bounds].size.height;
            if (NSPointInRect(localPoint, r)){
                return (EventLogCellUIElement *)[kids objectAtIndex:col];
            }
            x += r.size.width;
        }
    }
    return self;
//	return [super accessibilityHitTest:point];
}

//
// accessibility protocol
//

// attributes

- (NSArray *)accessibilityAttributeNames {
    static NSArray *attributes = nil;
    if (attributes == nil) {
        attributes = [[NSArray alloc] initWithObjects:
                    NSAccessibilityRoleAttribute,
                    NSAccessibilityRoleDescriptionAttribute,
                    NSAccessibilityFocusedAttribute,
                    NSAccessibilityParentAttribute,
                    NSAccessibilityWindowAttribute,
                    NSAccessibilityTopLevelUIElementAttribute,
                    NSAccessibilityPositionAttribute,
                    NSAccessibilitySizeAttribute,
                    NSAccessibilityChildrenAttribute,
//TODO: Why does NSAccessibilityDescriptionAttribute for header view prevent
//TODO: its NSAccessibilityChildrenAttribute from ever being called ???
//                    NSAccessibilityDescriptionAttribute,
                    nil];
    }
    return attributes;
}

- (id)accessibilityAttributeValue:(NSString *)attribute {
    if ([attribute isEqualToString:NSAccessibilityRoleAttribute]) {
        return NSAccessibilityGroupRole;

    } else if ([attribute isEqualToString:NSAccessibilityRoleDescriptionAttribute]) {
        return NSAccessibilityRoleDescription(NSAccessibilityGroupRole, nil);

    } else if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        return [NSNumber numberWithBool:NO];

    } else if ([attribute isEqualToString:NSAccessibilityParentAttribute]) {
        return NSAccessibilityUnignoredAncestor(parent);

    } else if ([attribute isEqualToString:NSAccessibilityWindowAttribute]) {
	// We're in the same window as our parent.
        return [parent window];

    } else if ([attribute isEqualToString:NSAccessibilityTopLevelUIElementAttribute]) {
        return [parent window];

    } else if ([attribute isEqualToString:NSAccessibilitySizeAttribute]) {
        return [NSValue valueWithSize:[self frame].size];

    } else if ([attribute isEqualToString:NSAccessibilityPositionAttribute]) {
        NSPoint pt = [self bounds].origin;
        pt.y += [self bounds].size.height;    // We need the bottom left corner
		//Convert the point to global (screen) coordinates
//		NSPoint windowPoint = [self convertPoint:pt toView: nil];
		NSPoint windowPoint = [parent convertPoint:pt toView: nil];
		pt = [[parent window] convertBaseToScreen:windowPoint];

        return [NSValue valueWithPoint:pt];

//TODO: Why does NSAccessibilityDescriptionAttribute for header view prevent
//TODO: its NSAccessibilityChildrenAttribute from ever being called ???
    } else if ([attribute isEqualToString:NSAccessibilityDescriptionAttribute]) {
         wxString s;
        if (isEventLog) {
            s = _("Header for Event Log");
        } else {
            wxString viewName = BOINCView->GetViewDisplayName();
            s.Printf(_("Header for %s"), viewName.c_str());
        }
        NSString *desc = [NSString stringWithUTF8String:(char *)(s.utf8_str().data())];
        return desc;

    } else if ([attribute isEqualToString:NSAccessibilityChildrenAttribute]) {
        if (kids == nil) {
            numkids = pList->GetColumnCount();
            kids = [[NSMutableArray arrayWithCapacity:numkids] retain];
            // The number of columns never changes
            NSInteger myFlags = isHeaderFlag | (isEventLog ? isEventLogFlag : 0);
            for (NSInteger i = 0; i < numkids; ++i) {
                EventLogCellUIElement *cell = [EventLogCellUIElement elementWithRow:0 column:i listFlags:myFlags listCtrl:pList parent:self BOINCView:BOINCView];
                [cell retain];
                [kids addObject:cell];
            }
        }

        return kids;
//        return NSAccessibilityUnignoredChildren(kids);    // No children are ignored

    } else {
        return [super accessibilityAttributeValue:attribute];
    }
}

- (void)dealloc {
    for (NSInteger i = 0; i < numkids; ++i) {
        [(EventLogCellUIElement *)[kids objectAtIndex:i] release];
    }
//TODO: Do we need to delete or free up kids array?
    if (kids) [kids release];
    [super dealloc];
}

@end

#pragma mark === CDlgEventLogListCtrl Accessibility Support ===

void CDlgEventLogListCtrl::SetupMacAccessibilitySupport() {
    NSView *listControlView = GetHandle();
    NSRect r = [ listControlView bounds ];
    FauxListHeaderView *fauxHeaderView = [ FauxListHeaderView  alloc ];
    FauxListBodyView *fauxBodyView = [ FauxListBodyView  alloc ];

    NSRect rh = r;
    rh.size.height = ((wxWindow *)m_headerWin)->GetSize().y;

    [fauxHeaderView initWithFrame:rh listCtrl:this listFlags:(isHeaderFlag | isEventLogFlag) parent:listControlView BOINCView:nil];
    [listControlView addSubview:fauxHeaderView ];

    // See comments in [ FauxListHeaderView hitTest:aPoint ]
    NSView *realHeaderView = ((wxWindow *)m_headerWin)->GetHandle();
    [realHeaderView retain];
    [realHeaderView removeFromSuperview];
    [listControlView addSubview:fauxHeaderView ];
    [fauxHeaderView addSubview:realHeaderView ];
    [realHeaderView release];

    NSRect rb = r;
    rb.origin.y += ((wxWindow *)m_headerWin)->GetSize().y;
    rb.size.height -= ((wxWindow *)m_headerWin)->GetSize().y;
    [fauxBodyView initWithFrame:rb listCtrl:this listFlags:(isEventLogFlag) parent:listControlView BOINCView:nil];
    [listControlView addSubview:fauxBodyView ];

    m_fauxHeaderView = fauxHeaderView;
    m_fauxBodyView = fauxBodyView;
}


void CDlgEventLogListCtrl::OnSize(wxSizeEvent& event) {
    NSView *listControlView = GetHandle();
    NSRect r = [ listControlView bounds ];
    FauxListHeaderView *fauxHeaderView = (FauxListHeaderView *)m_fauxHeaderView;
    FauxListBodyView *fauxBodyView = (FauxListBodyView *)m_fauxBodyView;

    if (fauxHeaderView) {
        NSRect rh = r;
        rh.size.height = ((wxWindow *)m_headerWin)->GetSize().y;
        [fauxHeaderView setFrame:rh];
    }

    if (fauxBodyView) {
        NSRect rb = r;
        rb.origin.y += ((wxWindow *)m_headerWin)->GetSize().y;
        rb.size.height -= ((wxWindow *)m_headerWin)->GetSize().y;
        [fauxBodyView setFrame:rb];
    }

    event.Skip();
}


void CDlgEventLogListCtrl::RemoveMacAccessibilitySupport() {
    [(FauxListHeaderView *)m_fauxHeaderView release];
    m_fauxHeaderView = nil;
    [(FauxListBodyView *)m_fauxBodyView release];
    m_fauxBodyView = nil;
}

#pragma mark === CBOINCListCtrl Accessibility Support ===

#if ! USE_NATIVE_LISTCONTROL

void CBOINCListCtrl::SetupMacAccessibilitySupport() {
    NSView *listControlView = GetHandle();
    NSRect r = [ listControlView bounds ];
    FauxListHeaderView *fauxHeaderView = [ FauxListHeaderView  alloc ];
    FauxListBodyView *fauxBodyView = [ FauxListBodyView  alloc ];

    NSRect rh = r;
    rh.size.height = ((wxWindow *)m_headerWin)->GetSize().y;

    [fauxHeaderView initWithFrame:rh listCtrl:this listFlags:isHeaderFlag parent:listControlView BOINCView:m_pParentView ];

    // See comments in [ FauxListHeaderView hitTest:aPoint ]
    NSView *realHeaderView = ((wxWindow *)m_headerWin)->GetHandle();
    [realHeaderView retain];
    [realHeaderView removeFromSuperview];
    [listControlView addSubview:fauxHeaderView ];
    [fauxHeaderView addSubview:realHeaderView ];
    [realHeaderView release];

    NSRect rb = r;
    rb.origin.y += ((wxWindow *)m_headerWin)->GetSize().y;
    rb.size.height -= ((wxWindow *)m_headerWin)->GetSize().y;
    [fauxBodyView initWithFrame:rb listCtrl:this listFlags:0 parent:listControlView BOINCView:m_pParentView];
    [listControlView addSubview:fauxBodyView ];

    m_fauxHeaderView = fauxHeaderView;
    m_fauxBodyView = fauxBodyView;
}


void CBOINCListCtrl::OnSize(wxSizeEvent& event) {
    NSView *listControlView = GetHandle();
    NSRect r = [ listControlView bounds ];
    FauxListHeaderView *fauxHeaderView = (FauxListHeaderView *)m_fauxHeaderView;
    FauxListBodyView *fauxBodyView = (FauxListBodyView *)m_fauxBodyView;

    if (fauxHeaderView) {
        NSRect rh = r;
        rh.size.height = ((wxWindow *)m_headerWin)->GetSize().y;
        [fauxHeaderView setFrame:rh];
    }

    if (fauxBodyView) {
        NSRect rb = r;
        rb.origin.y += ((wxWindow *)m_headerWin)->GetSize().y;
        rb.size.height -= ((wxWindow *)m_headerWin)->GetSize().y;
        [fauxBodyView setFrame:rb];
    }

    event.Skip();
}


void CBOINCListCtrl::RemoveMacAccessibilitySupport() {
    [(FauxListHeaderView *)m_fauxHeaderView release];
    m_fauxHeaderView = nil;
    [(FauxListBodyView *)m_fauxBodyView release];
    m_fauxBodyView = nil;
}

#endif // ! USE_NATIVE_LISTCONTROL

#pragma mark === CPaintStatistics & wxPieCtrl Accessibility Shared Code ===

#define statisticsPage 1
#define resourcesPage 2

@interface FauxGeneralView : NSView {
    id parent;
    NSInteger viewPage;
    void* theClass;
}

- (id)initWithFrame:(NSRect)frame whichViewPage:(NSInteger)aViewPage callingClass:(void*)aClass parent:aParent;
- (NSString*) getValue;
@end


@implementation FauxGeneralView

- (id)initWithFrame:(NSRect)frame whichViewPage:(NSInteger)aViewPage callingClass:(void*)aClass parent:aParent
{
	[super initWithFrame:frame];
    parent = aParent;
    viewPage = aViewPage;
    theClass = aClass;
    return self;
}

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)accessibilityIsIgnored {
    return NO;
}

- (NSString*) getValue {
    wxString s;

    switch (viewPage) {
    case statisticsPage:
        s = _("This panel contains graphs showing user totals for projects");
        break;
    case resourcesPage:
        {
            wxPieCtrl* pPieCtrl = (wxPieCtrl*)theClass;
            s = pPieCtrl->GetLabel();
            unsigned int i;

            for(i=0; i<pPieCtrl->m_Series.Count(); i++) {
                s += wxT("; ");
                s += pPieCtrl->m_Series[i].GetLabel();
            }
        }
        break;
    default:
        s = wxEmptyString;
        break;
    }
    NSString *desc = [NSString stringWithUTF8String:(char *)(s.utf8_str().data())];
    return desc;
}


- (NSArray *)accessibilityAttributeNames {
    static NSArray *attributes = nil;
    if (attributes == nil) {
        attributes = [[NSArray alloc] initWithObjects:
                    NSAccessibilityEnabledAttribute,
                    NSAccessibilityFocusedAttribute,
                    NSAccessibilityNumberOfCharactersAttribute,
                    NSAccessibilityParentAttribute,
                    NSAccessibilityPositionAttribute,
                    NSAccessibilityRoleAttribute,
                    NSAccessibilityRoleDescriptionAttribute,
                    NSAccessibilitySelectedTextAttribute,
                    NSAccessibilitySelectedTextRangeAttribute,
                    NSAccessibilityValueAttribute,
                    NSAccessibilityVisibleCharacterRangeAttribute,
                    NSAccessibilitySizeAttribute,
                    NSAccessibilityTopLevelUIElementAttribute,
                    NSAccessibilityWindowAttribute,
                    nil];
    }
    return attributes;
}

- (id)accessibilityAttributeValue:(NSString *)attribute {
    if ([attribute isEqualToString:NSAccessibilityEnabledAttribute]) {
        return [NSNumber numberWithBool:YES];

    } else if ([attribute isEqualToString:NSAccessibilityFocusedAttribute]) {
        return [NSNumber numberWithBool:NO];

    } else if ([attribute isEqualToString:NSAccessibilityNumberOfCharactersAttribute]) {
        NSString *s = [self getValue];
        NSUInteger n = [s lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        return [NSNumber numberWithUnsignedInt:n];

    } else if ([attribute isEqualToString:NSAccessibilityParentAttribute]) {
        return NSAccessibilityUnignoredAncestor(parent);

    } else if ([attribute isEqualToString:NSAccessibilityPositionAttribute]) {
        NSPoint pt = [self bounds].origin;
        pt.y += [self bounds].size.height;    // We need the bottom left corner
		//Convert the point to global (screen) coordinates
//		NSPoint windowPoint = [self convertPoint:pt toView: nil];
		NSPoint windowPoint = [parent convertPoint:pt toView: nil];
		pt = [[parent window] convertBaseToScreen:windowPoint];

        return [NSValue valueWithPoint:pt];

    } else if ([attribute isEqualToString:NSAccessibilityRoleAttribute]) {
        return NSAccessibilityStaticTextRole;

    } else if ([attribute isEqualToString:NSAccessibilityRoleDescriptionAttribute]) {
        return NSAccessibilityRoleDescription(NSAccessibilityStaticTextRole, nil);

    } else if ([attribute isEqualToString:NSAccessibilitySelectedTextAttribute]) {
        NSString *s = [NSString init];
        return s;

    } else if ([attribute isEqualToString:NSAccessibilitySelectedTextRangeAttribute]) {
        NSRange range = NSMakeRange(0, 0);
        return [NSValue valueWithRange:range];

    } else if ([attribute isEqualToString:NSAccessibilityValueAttribute]) {
        return [self getValue];

    } else if ([attribute isEqualToString:NSAccessibilityVisibleCharacterRangeAttribute]) {
        NSString *s = [self getValue];
        NSRange range = NSMakeRange(0, [s length]);
        return [NSValue valueWithRange:range];

    } else if ([attribute isEqualToString:NSAccessibilitySizeAttribute]) {
        return [NSValue valueWithSize:[self frame].size];

    } else if ([attribute isEqualToString:NSAccessibilityTopLevelUIElementAttribute]) {
        return [parent window];

    } else if ([attribute isEqualToString:NSAccessibilityWindowAttribute]) {
	// We're in the same window as our parent.
        return [parent window];

    } else {
        return [super accessibilityAttributeValue:attribute];
    }
}

- (void)dealloc {
    [super dealloc];
}

@end

#pragma mark === CPaintStatistics Accessibility Support ===

void CPaintStatistics::SetupMacAccessibilitySupport() {
    NSView *statisticsView = GetHandle();
    NSRect r = [ statisticsView bounds ];
    FauxGeneralView *fauxStatisticsView = [FauxGeneralView alloc ];
    [fauxStatisticsView initWithFrame:r whichViewPage:statisticsPage callingClass:this parent:statisticsView];
    [ statisticsView addSubview:fauxStatisticsView ];
    m_fauxStatisticsView = fauxStatisticsView;
}


void CPaintStatistics::ResizeMacAccessibilitySupport() {
    NSView *statisticsView = GetHandle();
    NSRect r = [ statisticsView bounds ];
    FauxGeneralView *fauxStatisticsView = (FauxGeneralView *)m_fauxStatisticsView;
    if (fauxStatisticsView) {
        [ fauxStatisticsView setFrame:r ];
    }
}

void CPaintStatistics::RemoveMacAccessibilitySupport() {
    [(FauxGeneralView *)m_fauxStatisticsView release];
    m_fauxStatisticsView = nil;
}

#pragma mark === wxPieCtrl Accessibility Support ===

void wxPieCtrl::SetupMacAccessibilitySupport() {
    NSView *resourcesView = GetHandle();
    NSRect r = [ resourcesView bounds ];
    FauxGeneralView *fauxResourcesView = [FauxGeneralView alloc ];
    [fauxResourcesView initWithFrame:r whichViewPage:resourcesPage callingClass:this parent:resourcesView];
    [ resourcesView addSubview:fauxResourcesView ];
    m_fauxResourcesView = fauxResourcesView;
}


void wxPieCtrl::ResizeMacAccessibilitySupport() {
    NSView *resourcesView = GetHandle();
    NSRect r = [ resourcesView bounds ];
    FauxGeneralView *fauxResourcesView = (FauxGeneralView *)m_fauxResourcesView;
    if (fauxResourcesView) {
        [ fauxResourcesView setFrame:r ];
    }
}

void wxPieCtrl::RemoveMacAccessibilitySupport() {
    [(FauxGeneralView *)m_fauxResourcesView release];
    m_fauxResourcesView = nil;
}
