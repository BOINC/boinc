// This file is part of BOINC.
// http://boinc.berkeley.edu
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

//  Mac_Saver_ModuleView.h
//  BOINC_Saver_Module
//

#import <ScreenSaver/ScreenSaver.h>
#import <Cocoa/Cocoa.h>


@interface BOINC_Saver_ModuleView : ScreenSaverView
{
    IBOutlet id mConfigureSheet;		// our configuration sheet
    IBOutlet NSButton *mGoToBlankCheckbox;
    IBOutlet NSTextField *mBlankingTimeTextField;
    IBOutlet NSTextField *mDefaultPeriodTextField;
    IBOutlet NSTextField *mSciencePeriodTextField;
    IBOutlet NSTextField *mChangePeriodTextField;

    int mVersion;               // the version of our prefs
    NSString *mBlankingTimeString;
    NSString *mDefaultPeriodString;
    NSString *mSciencePeriodString;
    NSString *mChangePeriodString;
}

- (IBAction)closeSheetSave:(id) sender;
- (IBAction)closeSheetCancel:(id) sender;

- (bool) setUpToUseCGWindowList;
- (void) cleanupSaver:(bool)mayExit;
- (void) doPeriodicTasks;
- (void) advancePosition:(NSTimer*)timer;

@end

@interface SharedGraphicsController : NSObject <NSMachPortDelegate>

@property (NS_NONATOMIC_IOSONLY, readonly) GLuint currentTextureName;
- (void)init:(NSView*)saverView;
- (void)testConnection;
- (void)portDied:(NSNotification *)notification;
- (void)closeServerPort;
- (void)cleanUpOpenGL;

@end

@interface saverOpenGLView : NSOpenGLView

- (GLuint)setupIOSurfaceTexture:(IOSurfaceRef)ioSurfaceBuffer;

@end

// The declarations below must be kept in sync with
// the corresponding ones in Mac_Saver_Module.h
#ifdef _DEBUG
    #define _T(x) x
#endif

#ifdef __cplusplus
extern "C" {
#endif

void            initBOINCSaver(void);
int             startBOINCSaver(void);
int             getSSMessage(char **theMessage, int* coveredFreq);
void            windowIsCovered();
void            drawPreview(CGContextRef myContext);
void            stopAllGFXApps(void);
void            closeBOINCSaver(void);
void            setDefaultDisplayPeriods(void);
bool            getShow_default_ss_first();
double          getGFXDefaultPeriod();
double          getGFXSciencePeriod();
double          getGGFXChangePeriod();
void            incompatibleGfxApp(char * appPath, char * wuName, pid_t pid, int slot);
void            setShow_default_ss_first(bool value);
void            setGFXDefaultPeriod(double value);
void            setGFXSciencePeriod(double value);
void            setGGFXChangePeriod(double value);
double          getDTime();
void            doBoinc_Sleep(double seconds);
void            launchedGfxApp(char * appPath, char * wuName, pid_t thePID, int slot);
int             compareBOINCLibVersionTo(int toMajor, int toMinor, int toRelease);
void            print_to_log_file(const char *format, ...);
void            strip_cr(char *buf);
void            PrintBacktrace(void);

extern bool     gIsCatalina;
extern bool     gIsHighSierra;
extern bool     gIsMojave;
extern bool     gIsSonoma;
extern bool     gMach_bootstrap_unavailable_to_screensavers;
extern mach_port_name_t commsPort;


#ifdef __cplusplus
}    // extern "C"
#endif
