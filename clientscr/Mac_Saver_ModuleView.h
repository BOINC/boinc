// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

@end

void            initBOINCSaver(void);
int             startBOINCSaver(void);
int             getSSMessage(char **theMessage, int* coveredFreq);
void            windowIsCovered();
void            drawPreview(CGContextRef myContext);
void            closeBOINCSaver(void);
double          getGFXDefaultPeriod();
double          getGFXSciencePeriod();
double          getGGFXChangePeriod();
void            setGFXDefaultPeriod(double value);
void            setGFXSciencePeriod(double value);
void            setGGFXChangePeriod(double value);
bool            validateNumericString(CFStringRef s);
extern void     print_to_log_file(const char *format, ...);
