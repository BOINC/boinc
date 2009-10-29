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

//  mac_accessiblity.cpp

#include <Carbon/Carbon.h>

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "BOINCBaseView.h"
#include "MainDocument.h"
#include "AdvancedFrame.h"
#include "BOINCListCtrl.h"
#include "Events.h"

#define MAX_LIST_COL 100


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


void CBOINCListCtrl::SetupMacListControlAccessibilitySupport(wxWindowID iListWindowID) {
#if !USE_NATIVE_LISTCONTROL
    HIViewRef   listControlView;
    OSErr       err;

    // For accessibility support
    listControlView = (HIViewRef)GetHandle();
    m_headerView = HIViewGetFirstSubview(listControlView);
    m_bodyView = HIViewGetNextView(m_headerView);
  
    err = HIViewSetEnabled(m_headerView, true);
    
    err = InstallHIObjectEventHandler((HIObjectRef)m_headerView, NewEventHandlerUPP(BOINCListHeaderAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        this, &m_pHeaderAccessibilityEventHandlerRef);

    err = InstallHIObjectEventHandler((HIObjectRef)m_bodyView, NewEventHandlerUPP(BOINCListBodyAccessibilityEventHandler), 
                                sizeof(myAccessibilityEvents) / sizeof(EventTypeSpec), myAccessibilityEvents, 
                                                        this, &m_pBodyAccessibilityEventHandlerRef); 
#endif
}


void CBOINCListCtrl::RemoveMacListControlAccessibilitySupport() {
    ::RemoveEventHandler(m_pHeaderAccessibilityEventHandlerRef);
//    ::RemoveEventHandler(m_pBodyAccessibilityEventHandlerRef);
}


OSStatus BOINCListAccessibilityEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData, Boolean isHeader) {
    const UInt32        eventClass = GetEventClass(inEvent);
    const UInt32        eventKind = GetEventKind(inEvent);
    OSStatus            err;
    CBOINCListCtrl*     pList = (CBOINCListCtrl*)pData;

    if (eventClass != kEventClassAccessibility) {
        return eventNotHandledErr;
    }

    AXUIElementRef		element;
    UInt64				inIdentifier = 0;
    UInt64              outIdentifier = 0;
    SInt32              row = 0;
    SInt32              col = 0;
    CBOINCBaseView*     pView = NULL;
    HIObjectRef         obj = NULL;
    
    err = GetEventParameter (inEvent, kEventParamAccessibleObject, 
                typeCFTypeRef, NULL, sizeof(typeCFTypeRef), NULL, &element);
    if (err) return err;
    
    AXUIElementGetIdentifier( element, &inIdentifier );
    obj = AXUIElementGetHIObject(element);

    pView = pList->GetParentView();
     
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
                if ((p.y > 0) || (p.y < -pList->GetHeaderHeight())) {
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
                CFArrayAppendValue( namesArray, kAXDescriptionAttribute );
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
                    wxString        str;

                    str = _("list of ");
                    str += pView->GetViewName();
                    
                    CFStringRef		roleDesc = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                    CFRelease( roleDesc );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		parent;
                    HIViewRef           parentView;

                    parentView = HIViewGetSuperview(isHeader ? pList->m_headerView : pList->m_bodyView);
                    parent = AXUIElementCreateWithHIObjectAndIdentifier((HIObjectRef)parentView, 0);
                    if (parent == NULL) {
                        return eventNotHandledErr;
                    }
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( parent ), &parent );
                    CFRelease( parent );
                    return noErr;
                
                } else if ( CFStringCompare( attribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo ) {
                    HIRect          r;
                    
                    err = HIViewGetBounds(isHeader ? pList->m_headerView : pList->m_bodyView, &r);
                    HISize          size = r.size;
                    
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeHISize, sizeof( HISize ), &size );
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXPositionAttribute, 0 ) == kCFCompareEqualTo ) {
                    HIRect          r;
                    HIPoint         pt;
                    
                    err = HIViewGetBounds(isHeader ? pList->m_headerView : pList->m_bodyView, &r);
                     int             x = r.origin.x, y = r.origin.y;
                    
                    // Now convert to global coordinates
                    pList->ClientToScreen(&x, &y);
                    pt.x = x;
                    if (isHeader) {
                        pt.y = y - pList->GetHeaderHeight();
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

                    if (isHeader) {
                        wxListItem      headerItem;
                        int             currentTabView;

                        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                        if (pFrame) {
                            currentTabView = pFrame->GetCurrentViewPage();
                        }
                        
                        pList->GetColumn(col, headerItem);
                        buf.Printf(_("%d of %d "), col+1, pList->GetColumnCount());
                        if (col == pView->m_iSortColumn) {
                            str = _("current sort column ");
                            str += buf;
                            str += (pView->m_bReverseSort ? _(" descending order ") : _(" ascending order "));
                        } else {
                            str = _("column ");
                            str += buf;
                        }
                        str += headerItem.GetText();
                    } else {    // ! isHeader
                        if (pList->GetItemState(row, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED) {
                           str = _("selected "); 
                        } else {
                           str = wxEmptyString; 
                        }

                        buf.Printf(_("row %d "), row+1);
                        str += buf;
                        if (col == 0) {
                            buf.Printf(_("of %d "), pList->GetItemCount());
                            str += buf;
                       }
                        buf.Printf(_("column %d "), col+1);
                        str += buf;
                        buf = pView->FireOnListGetItemText(row, col);
                        if (buf.IsEmpty()) {
                            buf = _("blank");
                        }
                        str += buf;
                    }
                    
                    CFStringRef		description = CFStringCreateWithCString(NULL, str.char_str(), kCFStringEncodingUTF8);
                    SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( description ), &description );
                    return noErr;
                    
                } else if ( CFStringCompare( attribute, kAXParentAttribute, 0 ) == kCFCompareEqualTo ) {
                    AXUIElementRef		parent;
                    HIViewRef           parentView;

                    parentView = isHeader ? pList->m_headerView : pList->m_bodyView;
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
                    if (pFrame) {
                        currentTabView = pFrame->GetCurrentViewPage();
                    }
                    
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
                        if (pFrame) {
                            currentTabView = pFrame->GetCurrentViewPage();
                        }
                        
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
                        if (pFrame) {
                            currentTabView = pFrame->GetCurrentViewPage();
                        }
                        
                        if (currentTabView & (VW_PROJ | VW_TASK |VW_XFER)) {
                            roleDesc = HICopyAccessibilityRoleDescription( kAXButtonRole, kAXSortButtonSubrole );
                            SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                            CFRelease( roleDesc );
                        } else {
                            roleDesc = CFSTR("");
                            SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                        }
                    } else {    // ! isHeader
                        roleDesc = HICopyAccessibilityRoleDescription( kAXStaticTextRole, NULL );
                            SetEventParameter( inEvent, kEventParamAccessibleAttributeValue, typeCFTypeRef, sizeof( roleDesc ), &roleDesc );
                            CFRelease( roleDesc );
                    }

                    return noErr;

                } else if ( CFStringCompare( attribute, kAXSizeAttribute, 0 ) == kCFCompareEqualTo ) {
                    HISize          size;
                    wxRect          r;
                    
                    // Return the size of this part as an HISize.
                    size.width = pList->GetColumnWidth(col);
                    if (isHeader) {
                        size.height = pList->GetHeaderHeight();
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
                    pt.y = y - pList->GetHeaderHeight();

                    SetEventParameter(inEvent, kEventParamAccessibleAttributeValue, typeHIPoint, sizeof(HIPoint), &pt);
                    return noErr;

                } else if ( CFStringCompare( attribute, kAXWindowAttribute, 0 ) == kCFCompareEqualTo
                        || CFStringCompare( attribute, kAXTopLevelUIElementAttribute, 0 ) == kCFCompareEqualTo ) {
                    // Return the window or top level ui element for this part. They are both the same so re-use the code.
                    AXUIElementRef		windOrTopUI;

                    WindowRef win = GetFrontWindowOfClass(kDocumentWindowClass, true);
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
                pView->ClearSelections();
                pList->SelectRow(row, true);
            }
            return noErr;
        }
        break;
        
        default:
            return eventNotHandledErr;
    }   // End switch(eventKind)
    
    return eventNotHandledErr;
}
