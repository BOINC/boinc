// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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

//  macAccessiblity.cpp

#include <Carbon/Carbon.h>

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "BOINCBaseView.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCListCtrl.h"
#include "ProjectListCtrl.h"
#include "ViewStatistics.h"
#include "wxPieCtrl.h"
#include "sg_BoincSimpleGUI.h"
#include "Events.h"
#include "macAccessiblity.h"

#define MAX_LIST_COL 100

UInt64 makeElementIdentifier(SInt32 row, SInt32 col, Boolean isHeader) {
    UInt64 id = (((row + 1) * MAX_LIST_COL) + (col + 1)) * 2;
    if (isHeader) id |= 1;
return id;
}

void parseElementIdentifier(UInt64 inIdentifier, SInt32& row, SInt32&col, Boolean& isHeader) {
    isHeader = inIdentifier & 1;
    row = (inIdentifier / (MAX_LIST_COL * 2)) - 1;
    col = ((inIdentifier / 2) % MAX_LIST_COL) - 1;
}

void AccessibilityIgnoreAllChildren(HIViewRef parent, int recursionLevel) {
    HIViewRef       child;
    OSStatus        err;
    
    if (recursionLevel > 100) {
        fprintf(stderr, "Error: AccessibilityIgnoreAllChildren recursion level > 100\n");
        return;
    }

    child = HIViewGetFirstSubview(parent);
    while (child) {
        err = HIObjectSetAccessibilityIgnored((HIObjectRef)child, true);
        AccessibilityIgnoreAllChildren(child, recursionLevel + 1);
        child = HIViewGetNextView(child);
    }
}


pascal OSStatus BOINCListAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData);

pascal OSStatus AttachListAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData);


    static EventTypeSpec myAccessibilityEvents[] = {
                                    { kEventClassAccessibility, kEventAccessibleGetChildAtPoint },
                                    { kEventClassAccessibility, kEventAccessibleGetFocusedChild },
                                    { kEventClassAccessibility, kEventAccessibleGetAllAttributeNames },
                                    { kEventClassAccessibility, kEventAccessibleGetAllParameterizedAttributeNames },
                                    { kEventClassAccessibility, kEventAccessibleIsNamedAttributeSettable },
                                    { kEventClassAccessibility, kEventAccessibleSetNamedAttribute },
                                    { kEventClassAccessibility, kEventAccessibleGetNamedAttribute },
                                    { kEventClassAccessibility, kEventAccessibleGetAllActionNames },			
                                    { kEventClassAccessibility, kEventAccessibleGetNamedActionDescription },
                                    { kEventClassAccessibility, kEventAccessiblePerformNamedAction }			
                                };


    static EventTypeSpec Simple_AccessibilityEvents[] = {
                                    { kEventClassAccessibility, kEventAccessibleGetNamedAttribute }			
                                };

pascal OSStatus SimpleAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData);

pascal OSStatus PieCtrlAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData);



#if !USE_NATIVE_LISTCONTROL
void CBOINCListCtrl::SetupMacAccessibilitySupport() {
    HIViewRef                       listControlView;
    HIViewRef                       headerView;
    HIViewRef                       bodyView;
    SInt32                          response;
    Boolean                         snowLeopard;
    OSErr                           err;

    err = Gestalt(gestaltSystemVersion, &response);
    snowLeopard = (err == noErr) && (response >= 0x1060);
    
    listControlView = (HIViewRef)GetHandle();
    headerView = HIViewGetFirstSubview(listControlView);
    bodyView = HIViewGetNextView(headerView);
    err = HIViewSetEnabled(headerView, true);

    accessibilityHandlerData.pList = (wxGenericListCtrl*)this;
    accessibilityHandlerData.pView = m_pParentView;
    accessibilityHandlerData.bodyView = bodyView;
    accessibilityHandlerData.headerView = headerView;
    accessibilityHandlerData.snowLeopard = snowLeopard;
    
    err = InstallHIObjectEventHandler((HIObjectRef)bodyView, NewEventHandlerUPP(BOINCListAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        &accessibilityHandlerData, &m_pBodyAccessibilityEventHandlerRef); 

    err = InstallHIObjectEventHandler((HIObjectRef)headerView, NewEventHandlerUPP(BOINCListAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        &accessibilityHandlerData, &m_pHeaderAccessibilityEventHandlerRef); 
}


void CBOINCListCtrl::RemoveMacAccessibilitySupport() {
    ::RemoveEventHandler(m_pBodyAccessibilityEventHandlerRef);
    ::RemoveEventHandler(m_pHeaderAccessibilityEventHandlerRef);
}
#endif


void CProjectListCtrlAccessible::SetupMacAccessibilitySupport() {
    OSErr       err;

    CProjectListCtrl* pCtrl = wxDynamicCast(mp_win, CProjectListCtrl);
    wxASSERT(pCtrl);
    
    if (pCtrl)
    {
        m_listView = (HIViewRef)pCtrl->GetHandle();
        err = HIViewSetEnabled(m_listView, true);
    
        err = InstallHIObjectEventHandler((HIObjectRef)m_listView, NewEventHandlerUPP(AttachListAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        this, &m_plistAccessibilityEventHandlerRef);
    } else {
        m_plistAccessibilityEventHandlerRef =  NULL;
    }
}


void CProjectListCtrlAccessible::RemoveMacAccessibilitySupport() {
    if (m_plistAccessibilityEventHandlerRef) {
        ::RemoveEventHandler(m_plistAccessibilityEventHandlerRef);
        m_plistAccessibilityEventHandlerRef = NULL;
   }
}


void CSimplePanel::SetupMacAccessibilitySupport() {
    OSStatus        err;
    wxString        str = _("for accessibility support, please select advanced from the view menu or type command shift a");
    HIViewRef       simple = (HIViewRef)GetHandle();
    CFStringRef     description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                                               
    // Have the screen reader tell user to switch to advanced view.
    HIObjectSetAuxiliaryAccessibilityAttribute((HIObjectRef)simple, 0, kAXDescriptionAttribute, description);
    CFRelease( description );

    err = InstallHIObjectEventHandler((HIObjectRef)simple, NewEventHandlerUPP(SimpleAccessibilityEventHandler), 
                                sizeof(Simple_AccessibilityEvents) / sizeof(EventTypeSpec), Simple_AccessibilityEvents, 
                                                        this, &m_pSGAccessibilityEventHandlerRef);
}


void CSimplePanel::RemoveMacAccessibilitySupport() {
    if (m_pSGAccessibilityEventHandlerRef) {
        ::RemoveEventHandler(m_pSGAccessibilityEventHandlerRef);
        m_pSGAccessibilityEventHandlerRef = NULL;
   }
}


void CTaskItemGroup::SetupMacAccessibilitySupport() {
    OSStatus        err;
    HIViewRef       boxView = (HIViewRef)m_pStaticBox->GetHandle();

    if (m_pTaskGroupAccessibilityEventHandlerRef == NULL) {
        err = InstallHIObjectEventHandler((HIObjectRef)boxView, NewEventHandlerUPP(SimpleAccessibilityEventHandler), 
                                sizeof(Simple_AccessibilityEvents) / sizeof(EventTypeSpec), Simple_AccessibilityEvents, 
                                                        this, &m_pTaskGroupAccessibilityEventHandlerRef);
    }
}


void CTaskItemGroup::RemoveMacAccessibilitySupport() {
    if (m_pTaskGroupAccessibilityEventHandlerRef) {
        ::RemoveEventHandler(m_pTaskGroupAccessibilityEventHandlerRef);
        m_pTaskGroupAccessibilityEventHandlerRef = NULL;
   }
}


void CViewStatistics::SetupMacAccessibilitySupport() {
    OSStatus        err;
    HIViewRef       paintPanelView = (HIViewRef)m_PaintStatistics->GetHandle();
    wxString        str = _("This panel contains graphs showing user totals for projects");
    CFStringRef     description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                                               
    // Have the screen reader tell user to switch to advanced view.
    HIObjectSetAuxiliaryAccessibilityAttribute((HIObjectRef)paintPanelView, 0, kAXDescriptionAttribute, description);
    CFRelease( description );

    err = InstallHIObjectEventHandler((HIObjectRef)paintPanelView, NewEventHandlerUPP(SimpleAccessibilityEventHandler), 
                                sizeof(Simple_AccessibilityEvents) / sizeof(EventTypeSpec), Simple_AccessibilityEvents, 
                                                        this, &m_pStatisticsAccessibilityEventHandlerRef);
}


void CViewStatistics::RemoveMacAccessibilitySupport() {
    ::RemoveEventHandler(m_pStatisticsAccessibilityEventHandlerRef);
}


void wxPieCtrl::SetupMacAccessibilitySupport() {
    OSStatus        err;
    HIViewRef       pieControlView = (HIViewRef)GetHandle();

    HIObjectSetAuxiliaryAccessibilityAttribute((HIObjectRef)pieControlView, 0, kAXDescriptionAttribute, CFSTR(""));

    err = InstallHIObjectEventHandler((HIObjectRef)pieControlView, NewEventHandlerUPP(PieCtrlAccessibilityEventHandler), 
                                sizeof(Simple_AccessibilityEvents) / sizeof(EventTypeSpec), Simple_AccessibilityEvents, 
                                                        this, &m_pPieCtrlAccessibilityEventHandlerRef);
}


void wxPieCtrl::RemoveMacAccessibilitySupport() {
    if (m_pPieCtrlAccessibilityEventHandlerRef) {
        ::RemoveEventHandler(m_pPieCtrlAccessibilityEventHandlerRef);
        m_pPieCtrlAccessibilityEventHandlerRef = NULL;
   }
}


pascal OSStatus BOINCListAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData) {
    const UInt32        eventClass = GetEventClass(inEvent);
    const UInt32        eventKind = GetEventKind(inEvent);
    OSStatus            err;
    wxGenericListCtrl*  pList = ((struct ListAccessData*)pData)->pList;
    CBOINCBaseView*     pView = ((struct ListAccessData*)pData)->pView;
    HIViewRef           headerView = ((struct ListAccessData*)pData)->headerView;
    HIViewRef           bodyView = ((struct ListAccessData*)pData)->bodyView;
    Boolean             snowLeopard = ((struct ListAccessData*)pData)->snowLeopard;

    if (eventClass != kEventClassAccessibility) {
        return eventNotHandledErr;
    }

    AXUIElementRef	element;
    UInt64		inIdentifier = 0;
    UInt64              outIdentifier = 0;
    SInt32              row = 0;
    SInt32              col = 0;
    Boolean             isHeader;
    HIObjectRef         obj = NULL;
    
    err = GetEventParameter (inEvent, kEventParamAccessibleObject, 
                typeCFTypeRef, NULL, sizeof(typeCFTypeRef), NULL, &element);
    if (err) return err;
    
    AXUIElementGetIdentifier( element, &inIdentifier );
    obj = AXUIElementGetHIObject(element);
    
    parseElementIdentifier(inIdentifier, row, col, isHeader);
    if (obj == (HIObjectRef)headerView) {
        isHeader = true;
    }

    switch (eventKind) {
#pragma mark kEventAccessibleGetChildAtPoint
        case kEventAccessibleGetChildAtPoint:
        {
            CFTypeRef	child = NULL;
            HIPoint		where;
            long        theRow = wxNOT_FOUND;
            long        ignored;
            int         hitflags;
            
            // Only the whole view or rows can be tested since the cells don't have sub-parts.
            if (col >= 0) {
                return noErr;
            }

            err = GetEventParameter (inEvent, kEventParamMouseLocation, 
                        typeHIPoint, NULL, sizeof(HIPoint), NULL, &where);
            if (err) return err;

            wxPoint     p((int)where.x, (int)where.y);
            pList->ScreenToClient(&p.x, &p.y);

            // HitTest returns the column only on wxMSW
            int x = 0, n = pList->GetColumnCount();
            for (col=0; col<n; col++) {
                x += pList->GetColumnWidth(col);
                if (p.x < x) break;
            }
            
            if (col >= n) {
                return noErr;
            }
            
            if (isHeader) {
                outIdentifier = makeElementIdentifier(-1, col, true);
            } else {
                theRow = pList->HitTest(p, hitflags, &ignored);
                if (theRow == wxNOT_FOUND) {
                    return noErr;
                }
                // Child of body is a row
                outIdentifier = (theRow + 1) * MAX_LIST_COL;
                outIdentifier = makeElementIdentifier(theRow, -1, false);
                if (row >= 0) {
                    // Child of row is a cell
                    outIdentifier = makeElementIdentifier(theRow, col, false);
                }
           }

            child = AXUIElementCreateWithHIObjectAndIdentifier(obj, outIdentifier );
            if (child == NULL) {
                return eventNotHandledErr;
            }
            
            err = SetEventParameter (inEvent, kEventParamAccessibleChild, typeCFTypeRef, 
                                        sizeof(typeCFTypeRef), &child);
            if (err) {
                return eventNotHandledErr;
            }
            
            return noErr;
        }
        break;

#pragma mark kEventAccessibleGetFocusedChild
        case kEventAccessibleGetFocusedChild:
            return noErr;
        break;

#pragma mark kEventAccessibleGetAllAttributeNames
        case kEventAccessibleGetAllAttributeNames:
        {
            CFMutableArrayRef	namesArray;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeNames, 
                    typeCFMutableArrayRef, NULL, sizeof(typeCFMutableArrayRef), NULL, &namesArray);
            if (err) 
                return err;

            CallNextEventHandler( inHandlerCallRef, inEvent );
            
            if ( (row < 0) && (col < 0) ) { // the whole view
                // col < 0 means an entire row.
                // Let accessibility know that this view has children and can
                // return a list of them.
                CFArrayAppendValue( namesArray, kAXChildrenAttribute );
                if (!isHeader) {
                    if (snowLeopard) {
                        CFArrayAppendValue( namesArray, kAXVisibleChildrenAttribute );
                        CFArrayAppendValue( namesArray, kAXSelectedChildrenAttribute );
                        CFArrayAppendValue( namesArray, kAXOrientationAttribute );
                    } else {
                        CFArrayAppendValue( namesArray, kAXVisibleRowsAttribute );
                        CFArrayAppendValue( namesArray, kAXRowsAttribute );
                        CFArrayAppendValue( namesArray, kAXSelectedRowsAttribute );
                        CFArrayAppendValue( namesArray, kAXColumnsAttribute );
                        CFArrayAppendValue( namesArray, kAXSelectedColumnsAttribute );
                        CFArrayAppendValue( namesArray, kAXHeaderAttribute );
                        CFArrayAppendValue( namesArray, kAXVisibleColumnsAttribute );
                    }
                }
            } else {
                if (isHeader) {
                    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                    if (pFrame) {
                        int currentTabView = pFrame->GetCurrentViewPage();
                        
                        if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                            // Sortable list
                            CFArrayAppendValue( namesArray, kAXSubroleAttribute );
                            CFArrayAppendValue( namesArray, kAXTitleAttribute );
                            if (snowLeopard) {
                                CFArrayAppendValue( namesArray, kAXSortDirectionAttribute );
                            }
                        }else {
                            // Message tab is not sortable
                            CFArrayAppendValue( namesArray, kAXValueAttribute );
                        }
                    }
                } else {
                    if (col < 0) {
                        // Row has children
                        CFArrayAppendValue( namesArray, kAXChildrenAttribute );
                        CFArrayAppendValue( namesArray, kAXSelectedAttribute );
                        CFArrayAppendValue( namesArray, kAXIndexAttribute );
                        CFArrayAppendValue( namesArray, kAXVisibleChildrenAttribute );
                        CFArrayAppendValue( namesArray, kAXSubroleAttribute );
                    } else {
                        CFArrayAppendValue( namesArray, kAXValueAttribute );
                    }
                }
                // Let accessibility know that this view's children can return description,
                // size, position, parent window, top level element and isFocused attributes.
                CFArrayAppendValue( namesArray, kAXWindowAttribute );
                CFArrayAppendValue( namesArray, kAXTopLevelUIElementAttribute );
                CFArrayAppendValue( namesArray, kAXSizeAttribute );
                CFArrayAppendValue( namesArray, kAXPositionAttribute );
                CFArrayAppendValue( namesArray, kAXEnabledAttribute );
            }
            
            CFArrayAppendValue( namesArray, kAXFocusedAttribute );
            CFArrayAppendValue( namesArray, kAXRoleAttribute );
            CFArrayAppendValue( namesArray, kAXRoleDescriptionAttribute );
            CFArrayAppendValue( namesArray, kAXDescriptionAttribute );
            CFArrayAppendValue( namesArray, kAXParentAttribute );

            return noErr;
        }
        break;
            
#pragma mark kEventAccessibleGetAllParameterizedAttributeNames
        case kEventAccessibleGetAllParameterizedAttributeNames:
        {
            CFMutableArrayRef	namesArray;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeNames, 
                    typeCFMutableArrayRef, NULL, sizeof(typeCFMutableArrayRef), NULL, &namesArray);
            if (err) return err;

            return noErr;
        }
        break;
            
#pragma mark kEventAccessibleGetNamedAttribute (Entire list or entire header)
        case kEventAccessibleGetNamedAttribute:
        {
            CFStringRef			attribute;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &attribute);
            if (err) return err;

            if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo ) {
                // Return whether or not this part is focused.
//TODO: Add real kAXFocusedAttribute support?
                Boolean				focused = false;
                
                SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( focused ), &focused );
                return noErr;
            }

            if ( (row < 0) && (col < 0) ) { // Entire list or entire header
                // String compare the incoming attribute name and return the appropriate accessibility
                // information as an event parameter.
            
                if ( CFStringCompare( attribute, kAXChildrenAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Create and return an array of AXUIElements describing the children of this view.
                    CFMutableArrayRef	children;
                    AXUIElementRef		child;

                    if (isHeader) {
                        int             c, n = pList->GetColumnCount();
                        children = CFArrayCreateMutable( kCFAllocatorDefault, n, &kCFTypeArrayCallBacks );

                        for ( c = 0; c < n; c++ ) {
                            // Header item for each column
                            outIdentifier = makeElementIdentifier(-1, c, true);
                            child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                            CFArrayAppendValue( children, child );
                            CFRelease( child );
                        }
                    } else {        // ! isHeader
                        int             r, m = pList->GetItemCount();
                        children = CFArrayCreateMutable( kCFAllocatorDefault, m, &kCFTypeArrayCallBacks );

                        // Data rows are each children of entire list
                        for ( r = 0; r < m; r++ ) {
                        // For each row
                            outIdentifier = makeElementIdentifier(r, -1, false);
                            child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                            CFArrayAppendValue( children, child );
                            CFRelease( child );
                        }
                    }
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXHeaderAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		child;

                    outIdentifier = makeElementIdentifier(-1, -1, true);
                    child = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)headerView, outIdentifier );
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( child ), &child );
                    CFRelease( child );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRowsAttribute, 0 ) == kCFCompareEqualTo ) {
                    CFMutableArrayRef	children;
                    AXUIElementRef		child;

                    int             r, m = pList->GetItemCount();
                    children = CFArrayCreateMutable( kCFAllocatorDefault, m, &kCFTypeArrayCallBacks );

                    // Data rows are each children of entire list
                    for ( r = 0; r < m; r++ ) {
                    // For each row
                        outIdentifier = makeElementIdentifier(r, -1, false);
                        child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                        CFArrayAppendValue( children, child );
                        CFRelease( child );
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr; 

                } else if (( CFStringCompare( attribute, kAXVisibleRowsAttribute, 0 ) == kCFCompareEqualTo ) 
                            || ( CFStringCompare( attribute, kAXVisibleChildrenAttribute, 0 ) == kCFCompareEqualTo )) {
                    CFMutableArrayRef	children;
                    AXUIElementRef		child;
                    int                 numItems, topItem, numVisibleItems, r;

                    children = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );

                    numItems = pList->GetItemCount();
                    if (numItems) {
                        topItem = pList->GetTopItem();     // Doesn't work properly for Mac Native control in wxMac-2.8.7

                        numVisibleItems = pList->GetCountPerPage();
                        ++numVisibleItems;

                        if (numItems <= (topItem + numVisibleItems)) numVisibleItems = numItems - topItem;
                        for ( r = 0; r < numVisibleItems; r++ ) {     // For each visible row
                            outIdentifier = makeElementIdentifier(r, -1, false);
                            child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                            CFArrayAppendValue( children, child );
                            CFRelease( child );
                        }
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr;

                } else if (( CFStringCompare( attribute, kAXSelectedChildrenAttribute, 0 ) == kCFCompareEqualTo ) 
                			|| ( CFStringCompare( attribute, kAXSelectedRowsAttribute, 0 ) == kCFCompareEqualTo )) {
                    CFMutableArrayRef	children;
                    AXUIElementRef		child;
                    int                 r;
                    
                    children = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );

                    r = -1;
                    while (1) {
                        // Step through all selected items
                        r = pList->GetNextItem(r, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                        if (r < 0) break;
                        outIdentifier = makeElementIdentifier(r, -1, false);
                        child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                        CFArrayAppendValue( children, child );
                        CFRelease( child );
                    }
             
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr;

                } else if (( CFStringCompare( attribute, kAXColumnsAttribute, 0 ) == kCFCompareEqualTo )
                            || ( CFStringCompare( attribute, kAXSelectedColumnsAttribute, 0 ) == kCFCompareEqualTo )
                            || ( CFStringCompare( attribute, kAXVisibleColumnsAttribute, 0 ) == kCFCompareEqualTo )
                            ) {
                    CFMutableArrayRef	children;
                    
                    // Tell system we don't have any columns
                    children = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXOrientationAttribute, 0 ) == kCFCompareEqualTo ) {
                    CFStringRef		orientation = kAXVerticalOrientationValue;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( orientation ), &orientation );
                    return noErr;


                } else if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this view. Using the table role doesn't work.
                    CFStringRef		role = kAXGroupRole;
                    if (!isHeader) {
                    	role = snowLeopard ? kAXListRole : kAXOutlineRole;
                    }
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this part.

                    CFStringRef		role = kAXGroupRole;
                    if (!isHeader) {
                    	role = snowLeopard ? kAXListRole : kAXOutlineRole;
                    }
                    CFStringRef roleDesc = HICopyAccessibilityRoleDescription( role, NULL );
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue , typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                    CFRelease( roleDesc );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this part.
                    wxString        str;
                    int             n = pList->GetItemCount();

                    if (isHeader) {
                        str = _("list headers");
                    } else {
                        if (pView) {
                            if (n) {
                                str.Printf(_("list of %s"), pView->GetViewDisplayName().c_str());
                            } else {
                                str.Printf(_("list of %s is empty"), pView->GetViewDisplayName().c_str());
                            }
                        }
                    }
                    
                    CFStringRef		description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( description ), &description );
                    CFRelease( description );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		parent;
                    HIViewRef           parentView;
                    
                    parentView = (HIViewGetSuperview(isHeader ? headerView : bodyView));
                    parent = AXUIElementCreateWithHIObjectAndIdentifier((HIObjectRef)parentView, 0);
                    if (parent == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( parent ), &parent );
                    CFRelease( parent );
                    return noErr;
                
                } else if ( CFStringCompare( attribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo ) {
                    HIRect          r;
                    HISize          size;
                    
                    err = HIViewGetBounds(isHeader ? headerView : bodyView, &r);                    
                    size = r.size;
                    if (!isHeader) {
                        size.height += pList->m_headerHeight;
                    }
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeHISize, sizeof( HISize ), &size );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo ) {
                    HIRect          r;
                    HIPoint         pt;
                    int             x, y;

                    err = HIViewGetBounds(isHeader ? headerView : bodyView, &r);
                    x = r.origin.x;
                    y = r.origin.y;
                    
                    // Now convert to global coordinates
                    pList->ClientToScreen(&x, &y);
                    pt.x = x;
                    pt.y = y - pList->m_headerHeight;

                    SetEventParameter(inEvent, kEventParamAccessibleAttributeValue, typeHIPoint, sizeof(HIPoint), &pt);
                    return noErr;

                } else {
                    return CallNextEventHandler( inHandlerCallRef, inEvent );

                }
                
#pragma mark kEventAccessibleGetNamedAttribute (row or item)
            } else {        // End if ( (row < 0) && (col < 0) )
            
                if ( CFStringCompare( attribute, kAXChildrenAttribute, 0 ) == kCFCompareEqualTo ) {
                    if (col >= 0) {
                        return eventNotHandledErr;
                    }
                    // Create and return an array of AXUIElements describing the children of this view.
                    CFMutableArrayRef	children;
                    AXUIElementRef		child;
                    int                 c, n = pList->GetColumnCount();
                    
                    children = CFArrayCreateMutable( kCFAllocatorDefault, n, &kCFTypeArrayCallBacks );

                    for ( c = 0; c < n; c++ ) {
                        // For each column
                        outIdentifier = makeElementIdentifier(row, c, isHeader);
                        child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                        CFArrayAppendValue( children, child );
                        CFRelease( child );
                    }
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    wxString        str, buf;
                    int             rowCount;
                    Boolean         isCurrentSortCol = false;

                    if (isHeader) {
                        wxListItem      headerItem;
                        int             numCols = pList->GetColumnCount();

                        pList->GetColumn(col, headerItem);
                        if (pView) {
                            if (col == pView->m_iSortColumn) {
                                isCurrentSortCol = true;
                            }
                        }
                        if (isCurrentSortCol) {
                            if (pView->m_bReverseSort) {
                                buf.Printf(_("; current sort column %d of %d; descending order; "), col+1, numCols);
                            } else {
                                buf.Printf(_("; current sort column %d of %d; ascending order; "), col+1, numCols);
                            }
                        } else {
                            buf.Printf(_("; column %d of %d; "), col+1, numCols);
                        }
                        str = headerItem.GetText();
                        str += buf;
                    } else {    // ! isHeader
                        rowCount = pList->GetItemCount();
                        if (rowCount <= 0) {
                            str = _("list is empty");                         
                        } else {
                            if (col < 0) {
                                str.Printf(_("; row %d; "), row+1);
                            } else {
                                if (pList->GetItemState(row, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) {
                                    if (col == 0) {
                                        buf.Printf(_("; selected row %d of %d; "), row+1, rowCount);
                                    } else {
                                        buf.Printf(_("; selected row %d ; "), row+1);
                                    }
                                } else {        // Row is not selected
                                    if (col == 0) {
                                        buf.Printf(_("; row %d of %d; "), row+1, rowCount);
                                    } else {
                                        buf.Printf(_("; row %d; "), row+1);
                                    }
                                }

                                str = pView->FireOnListGetItemText(row, col);
                                if (str.IsEmpty()) {
                                    str = _("blank");
                                }
                                
                                str += buf;
                           }
                        }
                    }
                    
                    CFStringRef		description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( description ), &description );
                    CFRelease( description );
                    return noErr;
                    
                } else if ( CFStringCompare( attribute, kAXValueAttribute, 0 ) == kCFCompareEqualTo ) {
                   CFStringRef		value = CFSTR("");

                   SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( value ), &value );
                    CFRelease( value );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		parent;
                    HIViewRef           parentView;

                    parentView = isHeader ? headerView : bodyView;
                    if (isHeader) {
                        outIdentifier = 0;      // Parent is entire list
                    } else {            // ! isHeader
                        if (col < 0) {  // Data row
                            outIdentifier = 0;      // Parent is entire list
                        } else {
                            outIdentifier = makeElementIdentifier(row, -1, isHeader);   // Parent of cell is data row
                        }
                    }
                    parent = AXUIElementCreateWithHIObjectAndIdentifier((HIObjectRef)parentView, outIdentifier);
                    if (parent == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( parent ), &parent );
                    CFRelease( parent );
                    return noErr;
                
                } else if ( CFStringCompare( attribute, kAXSubroleAttribute, 0 ) == kCFCompareEqualTo ) {
                    CFStringRef		subRole;
                    
                    if (isHeader) {
                        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                        if (!pFrame) {
                            return eventNotHandledErr;
                        }
                        int currentTabView = pFrame->GetCurrentViewPage();
                        
                        if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                            subRole = kAXSortButtonSubrole;     // SHould this be kAXTableRowSubrole ?
                        } else {
                            return eventNotHandledErr;
                        }
                    } else {    // ! isHeader
                        if (col < 0) {
                            subRole = kAXOutlineRowSubrole;     
                        } else {
                            return eventNotHandledErr;
                        }
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( subRole ), &subRole );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this part. The parts of the view behave like
                    // buttons, so use that system role.

                    CFStringRef		role;

                    if (isHeader) {
                        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                        if (!pFrame) {
                            return eventNotHandledErr;
                        }
                        int currentTabView = pFrame->GetCurrentViewPage();
                        
                        if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                            role = kAXButtonRole;
                        } else {
                            role = kAXStaticTextRole;
                        }
                    } else if (col < 0) {
                        role = kAXRowRole;
                    } else {
                        role = kAXStaticTextRole;
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string describing the role of this part. Use the system description.
                    CFStringRef		roleDesc;
                    
                    if (isHeader) {
                        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                        if (!pFrame) {
                            return eventNotHandledErr;
                        }
                        int currentTabView = pFrame->GetCurrentViewPage();
                        
                        if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                            roleDesc = HICopyAccessibilityRoleDescription( kAXButtonRole, kAXSortButtonSubrole );
                        } else {
                            roleDesc = HICopyAccessibilityRoleDescription( kAXStaticTextRole, NULL );
                        }
                    } else {    // ! isHeader
                        CFStringRef		role = kAXStaticTextRole;
                        if (col < 0) {
                            if (snowLeopard) {
                                role = kAXRowRole;
                            } else {
                                role = kAXOutlineRowSubrole;
                            }
                        }
                        roleDesc = HICopyAccessibilityRoleDescription( role, NULL );
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                    CFRelease( roleDesc );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXSortDirectionAttribute, 0 ) == kCFCompareEqualTo ) {
                    CFStringRef		sortDirection;

                    if (col == pView->m_iSortColumn) {
                        sortDirection = pView->m_bReverseSort ? kAXDescendingSortDirectionValue : kAXAscendingSortDirectionValue;
                    } else {
                        sortDirection = kAXUnknownSortDirectionValue;
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( sortDirection ), &sortDirection );
                    CFRelease( sortDirection );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo ) {
                    HISize          size;
                    wxRect          r;
                    
                    // Return the size of this part as an HISize.
                    size.width = pList->GetColumnWidth(col);
                    if (isHeader) {
                        size.height = pList->m_headerHeight;
                    } else {    // ! isHeader
                        pList->GetItemRect(row, r);
                        size.height = r.height;
                        if (col < 0) {
                            size.width = r.width;
                        }
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeHISize, sizeof( HISize ), &size );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo ) {
                    HIPoint         pt;
                    wxRect          r;
                    int             i, x = 0, y = 0;
                    
                    // Return the position of this part as an HIPoint.
                    // First get the position relative to the ListCtrl
                    for (i=0; i<col; i++) {
                        x += pList->GetColumnWidth(i);
                    }
                    
                    if (!isHeader) {
                        pList->GetItemRect(row, r);
                        y = r.y - 1;
                        if (col < 0) {
                            x = r.x;
                        }
                    }
                    // Now convert to global coordinates
                    pList->ClientToScreen(&x, &y);
                    pt.x = x;
                    pt.y = y - pList->m_headerHeight;

                    SetEventParameter(inEvent, kEventParamAccessibleAttributeValue, typeHIPoint, sizeof(HIPoint), &pt);
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXWindowAttribute, 0 ) == kCFCompareEqualTo
                        || CFStringCompare( attribute, kAXTopLevelUIElementAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return the window or top level ui element for this part. They are both the same so re-use the code.
                    AXUIElementRef		windOrTopUI;

                    WindowRef win = GetControlOwner(bodyView);
 
                    if (win == NULL) {
                        return eventNotHandledErr;
                    }
                    
                    windOrTopUI = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)win, 0 );
                    if (windOrTopUI == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( windOrTopUI ), &windOrTopUI );
                    CFRelease( windOrTopUI );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXEnabledAttribute, 0 ) == kCFCompareEqualTo ) {
                    Boolean				enabled = true;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( enabled ), &enabled );
                    return noErr;
                
//TODO: Add kAXFocusedAttribute support?
#if 0
                } else if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return whether or not this part is focused.
                    Boolean				focused = false;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( focused ), &focused );
                    return noErr;
#endif

                } else if ( CFStringCompare( attribute, kAXSelectedAttribute, 0 ) == kCFCompareEqualTo ) {
                    if (col >= 0) {
                        return eventNotHandledErr;
                    }
                    Boolean isSelected = pList->GetItemState(row, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED;
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( isSelected ), &isSelected );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXIndexAttribute, 0 ) == kCFCompareEqualTo ) {
                    if (col >= 0) {
                        return eventNotHandledErr;
                    }
                    int theRow = row + 1;
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeInteger, sizeof( theRow ), &theRow );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXVisibleChildrenAttribute, 0 ) == kCFCompareEqualTo ) {
//TODO: Actually determine which columns are visible
                    // Create and return an array of AXUIElements describing the children of this view.
                    CFMutableArrayRef	children;
                    AXUIElementRef	child;
                    int                 c, n = pList->GetColumnCount();
                    
                    children = CFArrayCreateMutable( kCFAllocatorDefault, n, &kCFTypeArrayCallBacks );

                    if (col >= 0) {
                        return eventNotHandledErr;
                    }

                    for ( c = 0; c < n; c++ ) {
                        // For each column
                        outIdentifier = makeElementIdentifier(row, c, isHeader);
                        child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                        CFArrayAppendValue( children, child );
                        CFRelease( child );
                    }
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr;
                
                } else if ( CFStringCompare( attribute, kAXTitleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return the item's text
                    wxString        str;
                    wxListItem      headerItem;

                    if (!isHeader) {
                        return eventNotHandledErr;
                    }
                    
                    if (col < 0) {
                        return eventNotHandledErr;
                    }
                    
                    pList->GetColumn(col, headerItem);
                    str = headerItem.GetText();
                    
                    CFStringRef		title = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( title ), &title );
                    CFRelease( title );
                    return noErr;

                } else {
                    return eventNotHandledErr;
                }

            } // End // End if (!( (row < 0) && (col < 0) ))
        break;
        }       // End case kEventAccessibleGetNamedAttribute:

#pragma mark kEventAccessibleIsNamedAttributeSettable
        case kEventAccessibleIsNamedAttributeSettable:
        {
            CFStringRef			attribute;
            Boolean				isSettable = false;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &attribute);
            if (err) return err;

            // The focused attribute is the only settable attribute for this view,
            // and it can only be set on part (or subelements), not the whole view.
            if ((row >= 0) || (col >= 0))
            {
                if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
                {
                    isSettable = true;
                }
            }
            SetEventParameter( inEvent, kEventParamAccessibleAttributeSettable, typeBoolean, sizeof( Boolean ), &isSettable );
            return noErr;
        }
        break;

#pragma mark kEventAccessibleSetNamedAttribute
        case kEventAccessibleSetNamedAttribute:
        {
            return eventNotHandledErr;
        }
        break;

#pragma mark kEventAccessibleGetAllActionNames
        case kEventAccessibleGetAllActionNames:
        {
            CFMutableArrayRef	array;

            err = GetEventParameter (inEvent, kEventParamAccessibleActionNames, 
                        typeCFMutableArrayRef, NULL, sizeof(typeCFMutableArrayRef), NULL, &array);
            if (err) return err;
            
            if (isHeader && (col >= 0) ) {
                CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                if (pFrame) {
                    int currentTabView = pFrame->GetCurrentViewPage();
                    
                    if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                        // Sortable list
                        CFArrayAppendValue( array, kAXPressAction );
                    }
                }
            }
            return noErr;
        }
        break;
        
#pragma mark kEventAccessibleGetNamedActionDescription
        case kEventAccessibleGetNamedActionDescription:
        {
            CFStringRef				action;
            CFMutableStringRef		desc;
            CFStringRef				selfDesc = NULL;
            
            if ( (row < 0) && (col < 0) ) {
                return eventNotHandledErr;
            }
            
            err = GetEventParameter (inEvent, kEventParamAccessibleActionName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &action);
            if (err) return err;

            err = GetEventParameter (inEvent, kEventParamAccessibleActionDescription, 
                        typeCFMutableStringRef, NULL, sizeof(typeCFMutableStringRef), NULL, &desc);
            if (err) return err;

             selfDesc = HICopyAccessibilityActionDescription( action );

            CFStringReplaceAll( desc, selfDesc );
            CFRelease( selfDesc );
            return noErr;
        }
        break;
    
#pragma mark kEventAccessiblePerformNamedAction
        case kEventAccessiblePerformNamedAction:
        {
            CFStringRef				action;
            wxWindowID              id = pList->GetId();
                
            if ( (row < 0) && (col < 0) ) {
                return eventNotHandledErr;
            }
                
            err = GetEventParameter (inEvent, kEventParamAccessibleActionName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &action);
            if (err) return err;
            
            if ( CFStringCompare( action, kAXPressAction, 0 ) != kCFCompareEqualTo ) {
                return eventNotHandledErr;
            }
            if (isHeader) {
                wxListEvent event(wxEVT_COMMAND_LIST_COL_CLICK, id);
                event.m_col = col;
                pList->AddPendingEvent(event);
            } else {
                if (pView) {
                    pView->ClearSelections();
                }
                pList->SetItemState(row,  wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
            return noErr;
        }
        break;
        
        default:
            return eventNotHandledErr;
    }   // End switch(eventKind)
    
    return eventNotHandledErr;
}


pascal OSStatus AttachListAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData) {
    const UInt32                eventClass = GetEventClass(inEvent);
    const UInt32                eventKind = GetEventKind(inEvent);
    OSStatus                    err;
    
    CProjectListCtrlAccessible* pAccessible = (CProjectListCtrlAccessible*)pData;
    if (pAccessible == NULL) {
        return eventNotHandledErr;
    }
    
    CProjectListCtrl*           pCtrl = wxDynamicCast(pAccessible->GetWindow(), CProjectListCtrl);
    if (pCtrl == NULL) {
        return eventNotHandledErr;
    }

    if (eventClass != kEventClassAccessibility) {
        return eventNotHandledErr;
    }

    AXUIElementRef		element;
    UInt64				inIdentifier = 0;
    UInt64              outIdentifier = 0;
    SInt32              row = 0;
    HIObjectRef         obj = NULL;
    
    err = GetEventParameter (inEvent, kEventParamAccessibleObject, 
                typeCFTypeRef, NULL, sizeof(typeCFTypeRef), NULL, &element);
    if (err) return err;
    
    AXUIElementGetIdentifier( element, &inIdentifier );
    obj = AXUIElementGetHIObject(element);
     
    row = inIdentifier;

    switch (eventKind) {
#pragma mark kEventAccessibleGetChildAtPoint
        case kEventAccessibleGetChildAtPoint:
        {
            CFTypeRef	child = NULL;
            HIPoint		where;
            int         hitRow;
            
            // Only the whole view can be tested since the parts don't have sub-parts.
            if (inIdentifier != 0) {
                return noErr;
            }

            err = GetEventParameter (inEvent, kEventParamMouseLocation, 
                        typeHIPoint, NULL, sizeof(HIPoint), NULL, &where);
            if (err) return err;

            wxPoint     p((int)where.x, (int)where.y);
            pCtrl->ScreenToClient(&p.x, &p.y);
            
            err = pAccessible->HitTest(p, &hitRow, NULL);
            
            if (err) {
                return eventNotHandledErr;
            }
            
            if (hitRow >= 0) {
                outIdentifier = hitRow + 1;
                child = AXUIElementCreateWithHIObjectAndIdentifier(obj, outIdentifier );
                if (child == NULL) {
                    return eventNotHandledErr;
                }
                
                err = SetEventParameter (inEvent, kEventParamAccessibleChild, typeCFTypeRef, 
                                            sizeof(typeCFTypeRef), &child);
                if (err) {
                    return eventNotHandledErr;
                }
            }
            
            return noErr;
        }
        break;

#pragma mark kEventAccessibleGetFocusedChild
        case kEventAccessibleGetFocusedChild:
            return noErr;
        break;

#pragma mark kEventAccessibleGetAllAttributeNames
        case kEventAccessibleGetAllAttributeNames:
        {
            CFMutableArrayRef	namesArray;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeNames, 
                    typeCFMutableArrayRef, NULL, sizeof(typeCFMutableArrayRef), NULL, &namesArray);
            if (err) 
                return err;

            CallNextEventHandler( inHandlerCallRef, inEvent );
            
            if ( inIdentifier == 0 )
            {
                // Identifier 0 means "the whole view".
                // Let accessibility know that this view has children and can
                // return a list of them.
                CFArrayAppendValue( namesArray, kAXChildrenAttribute );
            } else {
                // Let accessibility know that this view's children can return description,
                // size, position, parent window, top level element and isFocused attributes.
                CFArrayAppendValue( namesArray, kAXWindowAttribute );
                CFArrayAppendValue( namesArray, kAXTopLevelUIElementAttribute );
                CFArrayAppendValue( namesArray, kAXDescriptionAttribute );
                CFArrayAppendValue( namesArray, kAXSizeAttribute );
                CFArrayAppendValue( namesArray, kAXPositionAttribute );
                CFArrayAppendValue( namesArray, kAXTitleAttribute );
                CFArrayAppendValue( namesArray, kAXEnabledAttribute );
            }
            
            CFArrayAppendValue( namesArray, kAXFocusedAttribute );
            CFArrayAppendValue( namesArray, kAXRoleAttribute );
            CFArrayAppendValue( namesArray, kAXRoleDescriptionAttribute );
            CFArrayAppendValue( namesArray, kAXParentAttribute );

            return noErr;
        }
        break;
            
#pragma mark kEventAccessibleGetAllParameterizedAttributeNames
        case kEventAccessibleGetAllParameterizedAttributeNames:
        {
            CFMutableArrayRef	namesArray;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeNames, 
                    typeCFMutableArrayRef, NULL, sizeof(typeCFMutableArrayRef), NULL, &namesArray);
            if (err) return err;

            return noErr;
        }
        break;
            
#pragma mark kEventAccessibleGetNamedAttribute
        case kEventAccessibleGetNamedAttribute:
        {
            CFStringRef			attribute;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &attribute);
            if (err) return err;

            if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo ) {
                // Return whether or not this part is focused.
//TODO: Add kAXFocusedAttribute support?
                Boolean				focused = false;
                
                SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( focused ), &focused );
                return noErr;
                } else if ( CFStringCompare( attribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo ) {
                    wxRect          r;
                    HISize          theSize;
                    
                    err = pAccessible->GetLocation(r, row);
                    if (err) {
                        return eventNotHandledErr;
                    }

                    theSize.width = r.width;
                    theSize.height = r.height;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeHISize, sizeof( HISize ), &theSize );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo ) {
                    wxRect          r;
                    HIPoint         pt;
                    int             x, y;
                    
                    err = pAccessible->GetLocation(r, row);
                    if (err) {
                        return eventNotHandledErr;
                    }
                    
                    x = r.x;
                    y = r.y;
                    
                    // Now convert to global coordinates
                    pCtrl->ClientToScreen(&x, &y);
                    pt.x = x;
                    pt.y = y;

                    SetEventParameter(inEvent, kEventParamAccessibleAttributeValue, typeHIPoint, sizeof(HIPoint), &pt);
                    return noErr;
            }

            if ( inIdentifier == 0 ) {
                // String compare the incoming attribute name and return the appropriate accessibility
                // information as an event parameter.
            
                if ( CFStringCompare( attribute, kAXChildrenAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Create and return an array of AXUIElements describing the children of this view.
                    CFMutableArrayRef	children;
                    AXUIElementRef		child;
                    int                 i, n;

                    err = pAccessible->GetChildCount(&n);
                    children = CFArrayCreateMutable( kCFAllocatorDefault, n, &kCFTypeArrayCallBacks );

                    for ( i = 0; i < n; i++ ) {
                        outIdentifier = i+1;
                        child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                        CFArrayAppendValue( children, child );
                        CFRelease( child );
                    }
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this view. Using the table role doesn't work.
                    CFStringRef		role = kAXListRole;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this part.
//TODO: specify whether projects or account managers
                    wxString        str;

                    str = _("list of projects or account managers");
                    CFStringRef		roleDesc = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                    CFRelease( roleDesc );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		parent;
                    HIViewRef           parentView;

                    parentView = HIViewGetSuperview(pAccessible->m_listView);
                    parent = AXUIElementCreateWithHIObjectAndIdentifier((HIObjectRef)parentView, 0);
                    if (parent == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( parent ), &parent );
                    CFRelease( parent );
                    return noErr;
                
                } else {
                    return CallNextEventHandler( inHandlerCallRef, inEvent );

                }
                
            } else {        // End if ( inIdentifier == 0 )
            
                if ( CFStringCompare( attribute, kAXDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    wxString        str, buf;
                    int             n;

                    err = pAccessible->GetChildCount(&n);
                    if (err) {
                        return eventNotHandledErr;
                    }

                    if (pCtrl->IsSelected(row - 1)) {
                        str.Printf(_("selected row %d of %d; "), row, n);
                    } else {
                        str.Printf(_("row %d of %d; "), row, n);
                    }

                    err = pAccessible->GetDescription(row, &buf);
                    if (err) {
                        return eventNotHandledErr;
                    }
                    str += buf;
                    
                    CFStringRef		description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( description ), &description );
                    CFRelease( description );
                    return noErr;
                    
                } else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		parent;
                    HIViewRef           parentView;

                    parentView = pAccessible->m_listView;
                    parent = AXUIElementCreateWithHIObjectAndIdentifier((HIObjectRef)parentView, 0);
                    if (parent == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( parent ), &parent );
                    CFRelease( parent );
                    return noErr;
                
                } else if ( CFStringCompare( attribute, kAXSubroleAttribute, 0 ) == kCFCompareEqualTo ) {
                    return eventNotHandledErr;

                } else if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this part. The parts of the view behave like
                    // buttons, so use that system role.

                    CFStringRef		role = kAXStaticTextRole;       // kAXRowRole;

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string describing the role of this part. Use the system description.
                    CFStringRef		roleDesc = HICopyAccessibilityRoleDescription( kAXRowRole, NULL );

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                    CFRelease( roleDesc );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXWindowAttribute, 0 ) == kCFCompareEqualTo
                        || CFStringCompare( attribute, kAXTopLevelUIElementAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return the window or top level ui element for this part. They are both the same so re-use the code.
                    AXUIElementRef		windOrTopUI;

                    WindowRef win = GetControlOwner((HIViewRef)obj);
                    if (win == NULL) {
                        return eventNotHandledErr;
                    }
                    
                    windOrTopUI = AXUIElementCreateWithHIObjectAndIdentifier( (HIObjectRef)win, 0 );
                    if (windOrTopUI == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( windOrTopUI ), &windOrTopUI );
                    CFRelease( windOrTopUI );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXEnabledAttribute, 0 ) == kCFCompareEqualTo ) {
                    Boolean				enabled = true;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( enabled ), &enabled );
                    return noErr;
                
//TODO: Add kAXFocusedAttribute support?
#if 0
                } else if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return whether or not this part is focused.
                    Boolean				focused = false;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( focused ), &focused );
                    return noErr;
#endif

                } else if ( CFStringCompare( attribute, kAXTitleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return the item's text
                    wxString        str;
                    wxListItem      headerItem;

                    pAccessible->GetName(row, &str);
                    
                    CFStringRef		title = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( title ), &title );
                    CFRelease( title );
                    return noErr;

                } else {
                    return eventNotHandledErr;
                }

            } // End if ( inIdentifier != 0 )
        break;
        }       // End case kEventAccessibleGetNamedAttribute:

#pragma mark kEventAccessibleIsNamedAttributeSettable
        case kEventAccessibleIsNamedAttributeSettable:
        {
            CFStringRef			attribute;
            Boolean				isSettable = false;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &attribute);
            if (err) return err;

            // The focused attribute is the only settable attribute for this view,
            // and it can only be set on part (or subelements), not the whole view.
            if (inIdentifier != 0)
            {
                if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo )
                {
                    isSettable = true;
                }
            }
            SetEventParameter( inEvent, kEventParamAccessibleAttributeSettable, typeBoolean, sizeof( Boolean ), &isSettable );
            return noErr;
        }
        break;

#pragma mark kEventAccessibleSetNamedAttribute
        case kEventAccessibleSetNamedAttribute:
        {
            return eventNotHandledErr;
        }
        break;

#pragma mark kEventAccessibleGetAllActionNames
        case kEventAccessibleGetAllActionNames:
        {
            CFMutableArrayRef	array;

            err = GetEventParameter (inEvent, kEventParamAccessibleActionNames, 
                        typeCFMutableArrayRef, NULL, sizeof(typeCFMutableArrayRef), NULL, &array);
            if (err) return err;
            
            if (inIdentifier != 0) {
                CFArrayAppendValue( array, kAXPressAction );
            }
            return noErr;
        }
        break;
        
#pragma mark kEventAccessibleGetNamedActionDescription
        case kEventAccessibleGetNamedActionDescription:
        {
            CFStringRef				action;
            CFMutableStringRef		desc;
            CFStringRef				selfDesc = NULL;
            
            if (inIdentifier == 0) {
                return eventNotHandledErr;
            }
            
            err = GetEventParameter (inEvent, kEventParamAccessibleActionName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &action);
            if (err) return err;

            err = GetEventParameter (inEvent, kEventParamAccessibleActionDescription, 
                        typeCFMutableStringRef, NULL, sizeof(typeCFMutableStringRef), NULL, &desc);
            if (err) return err;

             selfDesc = HICopyAccessibilityActionDescription( action );

            CFStringReplaceAll( desc, selfDesc );
            CFRelease( selfDesc );
            return noErr;
        }
        break;
    
#pragma mark kEventAccessiblePerformNamedAction
        case kEventAccessiblePerformNamedAction:
        {
            CFStringRef				action;
                
            if (inIdentifier == 0) {
                return eventNotHandledErr;
            }
                
            err = GetEventParameter (inEvent, kEventParamAccessibleActionName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &action);
            if (err) return err;
            
            if ( CFStringCompare( action, kAXPressAction, 0 ) != kCFCompareEqualTo ) {
                return eventNotHandledErr;
            }
            err = pAccessible->DoDefaultAction(inIdentifier);
            if (err) {
                return eventNotHandledErr;
            }
            
            return noErr;
        }
        break;
        
        default:
            return eventNotHandledErr;
    }   // End switch(eventKind)
    
    return eventNotHandledErr;
}

pascal OSStatus SimpleAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData) {
    const UInt32                eventClass = GetEventClass(inEvent);
    const UInt32                eventKind = GetEventKind(inEvent);
    OSStatus                    err;
    
    if (eventClass != kEventClassAccessibility) {
        return eventNotHandledErr;
    }

    switch (eventKind) {

#pragma mark kEventAccessibleGetNamedAttribute
        case kEventAccessibleGetNamedAttribute:
            CFStringRef			attribute;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &attribute);
            if (err) return err;

            if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo ) {
                    CFStringRef		role = kAXStaticTextRole;

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
                    return noErr;
            }

        return CallNextEventHandler( inHandlerCallRef, inEvent );
        return eventNotHandledErr;
    break;

    
    default:
    return CallNextEventHandler( inHandlerCallRef, inEvent );

        return eventNotHandledErr;
    }
    
    return eventNotHandledErr;
}


pascal OSStatus PieCtrlAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData) {
    const UInt32                eventClass = GetEventClass(inEvent);
    const UInt32                eventKind = GetEventKind(inEvent);
    OSStatus                    err;
    wxPieCtrl*                  pPieCtrl = (wxPieCtrl*)pData;

    if (eventClass != kEventClassAccessibility) {
        return eventNotHandledErr;
    }

    switch (eventKind) {

#pragma mark kEventAccessibleGetNamedAttribute
        case kEventAccessibleGetNamedAttribute:
            CFStringRef			attribute;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeName, 
                        typeCFStringRef, NULL, sizeof(typeCFStringRef), NULL, &attribute);
            if (err) return err;

            if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo ) {
                    CFStringRef		role = kAXStaticTextRole;

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
                    return noErr;

            } else if ( CFStringCompare( attribute, kAXDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                // Return a string indicating the role of this part.
                wxString            str;
                CFStringRef         description;
                unsigned int        i;
                
                str = pPieCtrl->GetLabel();
                for(i=0; i<pPieCtrl->m_Series.Count(); i++) {
                    str += wxT("; ");
                    str += pPieCtrl->m_Series[i].GetLabel();
                }
                
                description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( description ), &description );
                CFRelease( description );
                return noErr;
            }

        return CallNextEventHandler( inHandlerCallRef, inEvent );
        return eventNotHandledErr;
    break;

    
    default:
    return CallNextEventHandler( inHandlerCallRef, inEvent );

        return eventNotHandledErr;
    }
    
    return eventNotHandledErr;
}

