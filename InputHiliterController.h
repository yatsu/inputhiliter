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
	NSColorWell *inactiveForegroundWell;
	NSColorWell *inactiveBackgroundWell;
}

@property (nonatomic, retain) IBOutlet NSWindow *window;
@property (nonatomic, retain) NSBundle *bundle;

@property (nonatomic, retain) IBOutlet NSColorWell *activeForegroundWell;
@property (nonatomic, retain) IBOutlet NSColorWell *activeBackgroundWell;
@property (nonatomic, retain) IBOutlet NSColorWell *inactiveForegroundWell;
@property (nonatomic, retain) IBOutlet NSColorWell *inactiveBackgroundWell;

- (id) initWithAPE:(NSBundle *)apeBundle andBundle:(NSBundle *)prefPaneBundle;

- (IBAction) setColor:(id)sender;

@end
