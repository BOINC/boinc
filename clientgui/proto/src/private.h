/////////////////////////////////////////////////////////////////////////////
// Name:        private.h extensions
// Purpose:     Platform specifiy extensions
// Maintainer:  Wyo
// Created:     2003-05-07
// RCS-ID:      $Id$
// Copyright:   (c) wxGuide
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef _MY_PRIVAT_H_
#define _MY_PRIVAT_H_

//============================================================================
// declarations
//============================================================================


// ----------------------------------------------------------------------------
// myIsKeyDown
// ----------------------------------------------------------------------------

// get the current state (pressed down) of keys
inline bool myIsKeyDown (int nVirtKey) {
#if defined(__WXMSW__)
    if (nVirtKey >= WXK_START) {
        switch (nVirtKey) {
            case WXK_MENU: return ::GetAsyncKeyState (VK_MENU) < 0; // cmd
            case WXK_SHIFT: return ::GetAsyncKeyState (VK_SHIFT) < 0; // shift
            case WXK_ALT: return ::GetAsyncKeyState (VK_MENU) < 0; // otpion
            case WXK_CONTROL: return ::GetAsyncKeyState (VK_CONTROL) < 0; // ctrl
            default: return false;
        }
    }else{
        return ::GetAsyncKeyState (nVirtKey) < 0;
    }
#elif defined(__WXMOTIF__)
    return false;
#elif defined(__WXGTK__)
    return false;
#elif defined(__WXX11__)
    return false;
#elif defined(__WXMGL__)
    return false;
#elif defined(__WXMAC__)
    switch (nVirtKey) {
        case WXK_MENU: return !(GetCurrentKeyModifiers() & 0x100); // cmd
        case WXK_SHIFT: return !(GetCurrentKeyModifiers() & 0x200); // shift
        case WXK_ALT: return !!GetCurrentKeyModifiers() & 0x800); // otpion
        case WXK_CONTROL: return !(GetCurrentKeyModifiers() & 0x1000); // ctrl
        default: return false;
    }
#elif defined(__WXPM__)
    return false;
#elif defined(__WXSTUBS__)
    return false;
#else // any other platform return always false
    return false;
#endif
}

#endif // _MY_PRIVAT_H_

