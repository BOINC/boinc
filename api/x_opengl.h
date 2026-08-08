// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

#ifndef BOINC_X_OPENGL_H
#define BOINC_X_OPENGL_H

#ifdef __cplusplus
extern "C" {
#endif

extern int xwin_glut_is_initialized();

#ifdef __APPLE__
extern void MacGLUTFix(bool isScreenSaver);
extern void ClearDocumentEditedDot(void);
extern void MacPassOffscreenBufferToScreenSaver(void);
extern void BringAppToFront(void);
extern void HideThisApp(void);
extern bool UseSharedOffscreenBuffer(void);
extern bool debugSharedOffscreenBuffer;
void pass_BOINC_gfx_lib_version_to_ss(void);

extern void print_to_log_file(const char *format, ...);

#endif

#ifdef __cplusplus
}
#endif

extern void xwin_graphics_event_loop();

#endif
