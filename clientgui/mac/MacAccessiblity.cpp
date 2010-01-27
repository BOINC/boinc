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
#include "DlgEventLog.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCListCtrl.h"
#include "DlgEventLogListCtrl.h"
#include "ProjectListCtrl.h"
#include "ViewStatistics.h"
#include "wxPieCtrl.h"
#include "sg_BoincSimpleGUI.h"
#include "Events.h"
#include "macAccessiblity.h"

#define MAX_LIST_COL 100

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


OSStatus BOINCListAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData, Boolean isHeader);

pascal OSStatus BOINCListHeaderAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData) {
    return BOINCListAccessibilityEventHandler(inHandlerCallRef, inEvent, pData, true);
}

pascal OSStatus BOINCListBodyAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData) {
    return BOINCListAccessibilityEventHandler(inHandlerCallRef, inEvent, pData, false);
}

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



void CBOINCListCtrl::SetupMacAccessibilitySupport() {
#if !USE_NATIVE_LISTCONTROL
    HIViewRef                       listControlView;
    HIViewRef                       headerView;
    HIViewRef                       bodyView;
    OSErr                           err;

    listControlView = (HIViewRef)GetHandle();
    headerView = HIViewGetFirstSubview(listControlView);
    bodyView = HIViewGetNextView(headerView);
    err = HIViewSetEnabled(headerView, true);
    
    accessibilityHandlerData.pList = (wxGenericListCtrl*)this;
    accessibilityHandlerData.pView = m_pParentView;
    accessibilityHandlerData.headerView = headerView;
    accessibilityHandlerData.bodyView = bodyView;
    accessibilityHandlerData.pEventLog = NULL;
    
    err = InstallHIObjectEventHandler((HIObjectRef)headerView, NewEventHandlerUPP(BOINCListHeaderAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        &accessibilityHandlerData, &m_pHeaderAccessibilityEventHandlerRef);

    err = InstallHIObjectEventHandler((HIObjectRef)bodyView, NewEventHandlerUPP(BOINCListBodyAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        &accessibilityHandlerData, &m_pBodyAccessibilityEventHandlerRef); 
#endif
}


void CBOINCListCtrl::RemoveMacAccessibilitySupport() {
#if !USE_NATIVE_LISTCONTROL
    ::RemoveEventHandler(m_pHeaderAccessibilityEventHandlerRef);
    ::RemoveEventHandler(m_pBodyAccessibilityEventHandlerRef);
#endif
}


void CDlgEventLogListCtrl::SetupMacAccessibilitySupport() {
#if !USE_NATIVE_LISTCONTROL
    HIViewRef                       listControlView;
    HIViewRef                       headerView;
    HIViewRef                       bodyView;
    OSErr                           err;

    listControlView = (HIViewRef)GetHandle();
    headerView = HIViewGetFirstSubview(listControlView);
    bodyView = HIViewGetNextView(headerView);
    err = HIViewSetEnabled(headerView, true);
    
    accessibilityHandlerData.pList = (wxGenericListCtrl*)this;
    accessibilityHandlerData.pView = NULL;
    accessibilityHandlerData.headerView = headerView;
    accessibilityHandlerData.bodyView = bodyView;
    accessibilityHandlerData.pEventLog = m_pParentView;
    
    err = InstallHIObjectEventHandler((HIObjectRef)headerView, NewEventHandlerUPP(BOINCListHeaderAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        &accessibilityHandlerData, &m_pHeaderAccessibilityEventHandlerRef);

    err = InstallHIObjectEventHandler((HIObjectRef)bodyView, NewEventHandlerUPP(BOINCListBodyAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        &accessibilityHandlerData, &m_pBodyAccessibilityEventHandlerRef); 
#endif
}


void CDlgEventLogListCtrl::RemoveMacAccessibilitySupport() {
#if !USE_NATIVE_LISTCONTROL
    ::RemoveEventHandler(m_pHeaderAccessibilityEventHandlerRef);
    ::RemoveEventHandler(m_pBodyAccessibilityEventHandlerRef);
#endif
}


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


OSStatus BOINCListAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData, Boolean isHeader) {
    const UInt32        eventClass = GetEventClass(inEvent);
    const UInt32        eventKind = GetEventKind(inEvent);
    OSStatus            err;
    wxGenericListCtrl*  pList = ((struct ListAccessData*)pData)->pList;
    CBOINCBaseView*     pView = ((struct ListAccessData*)pData)->pView;
    HIViewRef           headerView = ((struct ListAccessData*)pData)->headerView;
    HIViewRef           bodyView = ((struct ListAccessData*)pData)->bodyView;
    CDlgEventLog*       pEventLog = ((struct ListAccessData*)pData)->pEventLog;

    if (eventClass != kEventClassAccessibility) {
        return eventNotHandledErr;
    }

    AXUIElementRef		element;
    UInt64				inIdentifier = 0;
    UInt64              outIdentifier = 0;
    SInt32              row = 0;
    SInt32              col = 0;
    HIObjectRef         obj = NULL;
    
    err = GetEventParameter (inEvent, kEventParamAccessibleObject, 
                typeCFTypeRef, NULL, sizeof(typeCFTypeRef), NULL, &element);
    if (err) return err;
    
    AXUIElementGetIdentifier( element, &inIdentifier );
    obj = AXUIElementGetHIObject(element);
     
    if (inIdentifier) {
        if (! isHeader) {
            row = (inIdentifier / MAX_LIST_COL) - 1 + pList->GetTopItem();
        }
        col = (inIdentifier % MAX_LIST_COL) - 1;
    }

    switch (eventKind) {
        case kEventAccessibleGetChildAtPoint:
        {
            CFTypeRef	child = NULL;
            HIPoint		where;
            long        theRow = wxNOT_FOUND;
            long        ignored;
            int         hitflags;
            
            // Only the whole view can be tested since the parts don't have sub-parts.
            if (inIdentifier != 0) {
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
                if ((p.y > 0) || (p.y < -pList->m_headerHeight)) {
                    return noErr;
                }
                outIdentifier = col+1;
            } else {
                theRow = pList->HitTest(p, hitflags, &ignored);
                if (theRow == wxNOT_FOUND) {
                    return noErr;
                }
                outIdentifier = ((theRow + 1 - pList->GetTopItem()) * MAX_LIST_COL) + col + 1;
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

        case kEventAccessibleGetFocusedChild:
            return noErr;
        break;

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
                CFArrayAppendValue( namesArray, kAXSizeAttribute );
                CFArrayAppendValue( namesArray, kAXPositionAttribute );
                if (isHeader) {
                    CFArrayAppendValue( namesArray, kAXTitleAttribute );
                }
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
            
        case kEventAccessibleGetAllParameterizedAttributeNames:
        {
            CFMutableArrayRef	namesArray;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeNames, 
                    typeCFMutableArrayRef, NULL, sizeof(typeCFMutableArrayRef), NULL, &namesArray);
            if (err) return err;

            return noErr;
        }
        break;
            
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
            }

            if ( inIdentifier == 0 ) {
                // String compare the incoming attribute name and return the appropriate accessibility
                // information as an event parameter.
            
                if ( CFStringCompare( attribute, kAXChildrenAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Create and return an array of AXUIElements describing the children of this view.
                    CFMutableArrayRef	children;
                    AXUIElementRef		child;
                    int                 c, n = pList->GetColumnCount();

                    if (isHeader) {
                        children = CFArrayCreateMutable( kCFAllocatorDefault, n, &kCFTypeArrayCallBacks );

                        for ( c = 0; c < n; c++ ) {
                            // Header item for each column
                            outIdentifier = c+1;
                            child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                            CFArrayAppendValue( children, child );
                            CFRelease( child );
                        }
                    } else {        // ! isHeader
                        int             r, m = pList->GetCountPerPage();
                        
                         children = CFArrayCreateMutable( kCFAllocatorDefault, (m + 1) * n, &kCFTypeArrayCallBacks );

                        for ( r = 0; r < m; r++ ) {
                            // For each row
                            for ( c = 0; c < n; c++ ) {
                                // For each column
                                outIdentifier = ((r + 1) * MAX_LIST_COL) + c + 1;
                                child = AXUIElementCreateWithHIObjectAndIdentifier( obj, outIdentifier );
                                CFArrayAppendValue( children, child );
                                CFRelease( child );
                            }
                        }
                    }
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( children ), &children );
                    CFRelease( children );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this view. Using the table role doesn't work.
                    CFStringRef		role = isHeader? kAXColumnRole : kAXListRole;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this part.

                    CFStringRef		roleDesc = CFSTR("");
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                    CFRelease( roleDesc );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this part.
                    wxString        str;

                    if (isHeader) {
                        str = _("blank");
                    } else {
                        str = _("list of ");
                        if (pEventLog) {
                            str += _("events");
                        } else {
                            if (pView) {
                                str += pView->GetViewDisplayName();
                            }
                        }
                        if (pList->GetItemCount() <= 0) {
                            str += _(" is empty"); 
                        }
                    }
                    
                    CFStringRef		description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( description ), &description );
                    CFRelease( description );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		parent;
                    HIViewRef           parentView;

                    parentView = HIViewGetSuperview(isHeader ? headerView : bodyView);
                    parent = AXUIElementCreateWithHIObjectAndIdentifier((HIObjectRef)parentView, 0);
                    if (parent == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( parent ), &parent );
                    CFRelease( parent );
                    return noErr;
                
                } else if ( CFStringCompare( attribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo ) {
                    HIRect          r;
                    
                    err = HIViewGetBounds(isHeader ? headerView : bodyView, &r);
                    HISize          size = r.size;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeHISize, sizeof( HISize ), &size );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo ) {
                    HIRect          r;
                    HIPoint         pt;
                    
                    err = HIViewGetBounds(isHeader ? headerView : bodyView, &r);
                     int             x = r.origin.x, y = r.origin.y;
                    
                    // Now convert to global coordinates
                    pList->ClientToScreen(&x, &y);
                    pt.x = x;
                    if (isHeader) {
                        pt.y = y - pList->m_headerHeight;
                    } else {
                        pt.y = y;
                    }

                    SetEventParameter(inEvent, kEventParamAccessibleAttributeValue, typeHIPoint, sizeof(HIPoint), &pt);
                    return noErr;

                } else {
                    return CallNextEventHandler( inHandlerCallRef, inEvent );

                }
                
            } else {        // End if ( inIdentifier == 0 )
            
                if ( CFStringCompare( attribute, kAXDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    wxString        str, buf;
                    int             rowCount;
                    Boolean         isCurrentSortCol = false;

                    if (isHeader) {
                        wxListItem      headerItem;

                        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                        if (!pFrame) {
                            return eventNotHandledErr;
                        }
                        
                        pList->GetColumn(col, headerItem);
                        buf.Printf(_("%d of %d; "), col+1, pList->GetColumnCount());
                        if (pView) {
                            if (col == pView->m_iSortColumn) {
                                isCurrentSortCol = true;
                            }
                        }
                        if (isCurrentSortCol) {
                            str = _("current sort column ");
                            str += buf;
                            str += (pView->m_bReverseSort ? _(" descending order ") : _(" ascending order "));
                        } else {
                            str = _("column ");
                            str += buf;
                        }
                        str += headerItem.GetText();
                    } else {    // ! isHeader
                        rowCount = pList->GetItemCount();
                        if (rowCount <= 0) {
                            str = _("list is empty");                         
                        } else {
                            if (pList->GetItemState(row, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) {
                               str = _("selected "); 
                            } else {
                               str = wxEmptyString; 
                            }

                            buf.Printf(_("row %d "), row+1);
                            str += buf;
                            if (col == 0) {
                                buf.Printf(_("of %d; "), rowCount);
                                str += buf;
                           }
                            buf.Printf(_("column %d; "), col+1);
                            str += buf;
                            if (pEventLog) {
                                buf = pEventLog->OnListGetItemText(row, col);
                            } else {
                                buf = pView->FireOnListGetItemText(row, col);
                            }
                            if (buf.IsEmpty()) {
                                buf = _("blank");
                            }
                            str += buf;
                        }
                    }
                    CFStringRef		description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( description ), &description );
                    CFRelease( description );
                    return noErr;
                    
                } else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		parent;
                    HIViewRef           parentView;

                    parentView = isHeader ? headerView : bodyView;
                    parent = AXUIElementCreateWithHIObjectAndIdentifier((HIObjectRef)parentView, 0);
                    if (parent == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( parent ), &parent );
                    CFRelease( parent );
                    return noErr;
                
                } else if ( CFStringCompare( attribute, kAXSubroleAttribute, 0 ) == kCFCompareEqualTo ) {
                    CFStringRef		subRole;
                    int             currentTabView;
                    
                    if (! isHeader) {
                        return eventNotHandledErr;
                    }
                    
                    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                    if (!pFrame) {
                        return eventNotHandledErr;
                    }
                    currentTabView = pFrame->GetCurrentViewPage();
                    
                    if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                        subRole = kAXSortButtonSubrole;
                    } else {
                        return eventNotHandledErr;
                    }
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( subRole ), &subRole );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string indicating the role of this part. The parts of the view behave like
                    // buttons, so use that system role.

                    CFStringRef		role;
                    int             currentTabView;

                    if (isHeader) {
                        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                        if (!pFrame) {
                            return eventNotHandledErr;
                        }
                        currentTabView = pFrame->GetCurrentViewPage();
                        
                        if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                            role = kAXButtonRole;
                        } else {
                            role = kAXColumnRole;
                        }
                    } else {    // ! isHeader
                        role = kAXStaticTextRole;
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( role ), &role );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXRoleDescriptionAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return a string describing the role of this part. Use the system description.
                    CFStringRef		roleDesc;
                    int             currentTabView;
                    
                    if (isHeader) {
                        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                        if (!pFrame) {
                            return eventNotHandledErr;
                        }
                        currentTabView = pFrame->GetCurrentViewPage();
                        
                        if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                            roleDesc = HICopyAccessibilityRoleDescription( kAXButtonRole, kAXSortButtonSubrole );
                        } else {
                            roleDesc = CFStringCreateCopy(NULL, CFSTR(""));
                        }
                    } else {    // ! isHeader
                        roleDesc = HICopyAccessibilityRoleDescription( kAXStaticTextRole, NULL );
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                    CFRelease( roleDesc );
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
                    }

                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeHISize, sizeof( HISize ), &size );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo ) {
                    HIPoint         pt;
                    wxRect          r;
                    int             i, x = 0, y = 0;
                    
                    // Return the position of this part as an HIPoint.
                    // First get the position relative to the ListCtrl
                    for (i=0; i< col; i++) {
                        x += pList->GetColumnWidth(i);
                    }
                    
                    if (!isHeader) {
                        pList->GetItemRect(row, r);
                        y = r.y - 1;
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
                
#if 0
                } else if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return whether or not this part is focused.
//TODO: Add kAXFocusedAttribute support?
                    Boolean				focused = false;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeBoolean, sizeof( focused ), &focused );
                    return noErr;
#endif

                } else if ( CFStringCompare( attribute, kAXTitleAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return the item's text
                    wxString        str;
                    wxListItem      headerItem;

                    if (!isHeader) {
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

            } // End if ( inIdentifier != 0 )
        break;
        }       // End case kEventAccessibleGetNamedAttribute:

        
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

        case kEventAccessibleSetNamedAttribute:
        {
            return eventNotHandledErr;
        }
        break;

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
    
        case kEventAccessiblePerformNamedAction:
        {
            CFStringRef				action;
            wxWindowID              id = pList->GetId();
                
            if (inIdentifier == 0) {
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
    
//TODO: Check with Rom why this starts at 0 not 1
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

        case kEventAccessibleGetFocusedChild:
            return noErr;
        break;

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
            
        case kEventAccessibleGetAllParameterizedAttributeNames:
        {
            CFMutableArrayRef	namesArray;

            err = GetEventParameter (inEvent, kEventParamAccessibleAttributeNames, 
                    typeCFMutableArrayRef, NULL, sizeof(typeCFMutableArrayRef), NULL, &namesArray);
            if (err) return err;

            return noErr;
        }
        break;
            
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
//                    str += pView->GetViewDisplayName();
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
                        str = _("selected ");
                    } else {
                        str = wxEmptyString;
                    }
                    buf.Printf(_("row %d of %d; "), row, n);
                    str += buf;

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
//                    CFStringRef		roleDesc = HICopyAccessibilityRoleDescription( kAXStaticTextRole, NULL );

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
                
#if 0
                } else if ( CFStringCompare( attribute, kAXFocusedAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return whether or not this part is focused.
//TODO: Add kAXFocusedAttribute support?
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

        case kEventAccessibleSetNamedAttribute:
        {
            return eventNotHandledErr;
        }
        break;

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

