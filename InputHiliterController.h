#import <Cocoa/Cocoa.h>
#import <ApplicationEnhancer/APEManagerPrefPane.h>
#import "CFPreferencesWrapper.h"

@interface InputHiliterController : NSObject<APEManagerPrefPane>
{
	NSWindow *window;

	CFPreferencesWrapper_InputHiliter *prefs;
	NSBundle *bundle;

	NSColorWell *activeForegroundWell;
	NSColorWell *activeBackgroundWell;
	NSColorWell *otherForegroundWell;
	NSColorWell *otherBackgroundWell;
}

@property (nonatomic, retain) IBOutlet NSWindow *window;
@property (nonatomic, retain) NSBundle *bundle;

@property (nonatomic, retain) IBOutlet NSColorWell *activeForegroundWell;
@property (nonatomic, retain) IBOutlet NSColorWell *activeBackgroundWell;
@property (nonatomic, retain) IBOutlet NSColorWell *otherForegroundWell;
@property (nonatomic, retain) IBOutlet NSColorWell *otherBackgroundWell;

- (id) initWithAPE:(NSBundle *)apeBundle andBundle:(NSBundle *)prefPaneBundle;

- (IBAction) setColor:(id)sender;

@end
