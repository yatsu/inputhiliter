#import <ApplicationEnhancer/ApplicationEnhancer.h>
#import "InputHiliterController.h"

// Just a handy macro for localized strings from our bundle; unused in this project.
#define NSLocalizedString_(key, comment) [[self bundle] localizedStringForKey:(key) value:@"" table:nil]

@implementation InputHiliterController

@synthesize window;
@synthesize bundle;
@synthesize activeForegroundWell;
@synthesize activeBackgroundWell;
@synthesize otherForegroundWell;
@synthesize otherBackgroundWell;

- (IBAction)setColor:(id)sender
{
  NSColor *color = [[(NSColorWell *)sender color] colorUsingColorSpaceName:NSCalibratedRGBColorSpace];

  NSString *key;
  if (sender == activeForegroundWell)
	key = @"activeForeground";
  else if (sender == activeBackgroundWell)
	key = @"activeBackground";
  else if (sender == otherForegroundWell)
	key = @"otherForeground";
  else if (sender == otherBackgroundWell)
	key = @"otherBackground";

  NSString *str = [NSString stringWithFormat:@"%lf %lf %lf %lf",
				   [color redComponent], [color greenComponent], [color blueComponent], [color alphaComponent]];
  [prefs setObject:str forKey:key];

  [prefs synchronize];
}

// This method should return an id of a class that implements the protocol
// required by APE Manager.

+ (id)prefPaneHandlerForAPE:(NSBundle*)apeBundle
                  andBundle:(NSBundle*)prefPaneBundle
{
  return [[self alloc] initWithAPE:apeBundle andBundle:prefPaneBundle];
}

// Init the instance of our class
- (id)initWithAPE:(NSBundle*)apeBundle
        andBundle:(NSBundle*)prefPaneBundle
{
  // save our bundle for future use
  [self setBundle:prefPaneBundle];

  // load our nib file; it will call our awakeFromNib method
  [NSBundle loadNibNamed:@"MainMenu" owner:self];

  return self;
}

// This method is called when the preference pane is about to be shown.
- (void)willLoad
{
}

// This method is called when the preference pane will be dismissed.
- (void)willUnload
{
}

// This method should return an NSView to be loaded into APE Manager preference
// area.
- (NSView*)mainView
{
  // we return our window's contentView
  return [window contentView];
}

// This method is called once our nib file is loaded (we load it in the init method)
- (void)awakeFromNib
{
  // load the preferences for our APE module
  prefs = [[CFPreferencesWrapper_InputHiliter preferencesWithApplication:@"info.yatsu.InputHiliter"] retain];
  [prefs synchronize];

  double red, green, blue, alpha;
  NSString *str;

  str = [prefs stringForKey:@"activeForeground"];
  if (str) {
	sscanf([str UTF8String], "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
  }
  else {
	red = green = blue = alpha = 1.0;
	[prefs setObject:[NSString stringWithFormat:@"%lf %lf %lf %lf", red, green, blue, alpha] forKey:@"activeForeground"];
  }
  [activeForegroundWell setColor:[NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha]];

  str = [prefs stringForKey:@"activeBackground"];
  if (str) {
	sscanf([str UTF8String], "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
  }
  else {
	red = 0.0;
	green = 0.501961;
	blue = alpha = 1.0;
	[prefs setObject:[NSString stringWithFormat:@"%lf %lf %lf %lf", red, green, blue, alpha] forKey:@"activeBackground"];
  }
  [activeBackgroundWell setColor:[NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha]];

  str = [prefs stringForKey:@"otherForeground"];
  if (str) {
	sscanf([str UTF8String], "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
  }
  else {
	red = 0.298039;
	green = 0.298039;
	blue = 0.298039;
	alpha = 1.0;
	[prefs setObject:[NSString stringWithFormat:@"%lf %lf %lf %lf", red, green, blue, alpha] forKey:@"otherForeground"];
  }
  [otherForegroundWell setColor:[NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha]];

  str = [prefs stringForKey:@"otherBackground"];
  if (str) {
	sscanf([str UTF8String], "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
  }
  else {
	red = 0.648860;
	green = 0.792326;
	blue = alpha = 1.0;
	[prefs setObject:[NSString stringWithFormat:@"%lf %lf %lf %lf", red, green, blue, alpha] forKey:@"otherBackground"];
  }
  [otherBackgroundWell setColor:[NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha]];

  [prefs synchronize];
}

- (void)dealloc
{
  [window release];
  [bundle release];
  [prefs release];
  [activeForegroundWell release];
  [activeBackgroundWell release];
  [otherForegroundWell release];
  [otherBackgroundWell release];

  [super dealloc];
}

@end
