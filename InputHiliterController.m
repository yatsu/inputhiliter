/*
  Copyright (c) 2005-2008 yatsu.info, All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.
*/

#import <ApplicationEnhancer/ApplicationEnhancer.h>
#import "InputHiliterController.h"

// Just a handy macro for localized strings from our bundle; unused in this project.
#define NSLocalizedString_(key, comment) [[self bundle] localizedStringForKey:(key) value:@"" table:nil]

@implementation InputHiliterController

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
  // we do nothing here
}

// This method is called when the preference pane will be dismissed.
- (void)willUnload
{
  // we do nothing here, as well
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
  prefs = [[CFPreferencesWrapper_APEBeepMultiplier preferencesWithApplication:@"info.yatsu.InputHiliter"] retain];

  // do other inits here
}

// Utility getters/setters --

- (void)setBundle:(NSBundle *)aBundle
{
  [aBundle retain];
  [_bundle release];
  _bundle = aBundle;
}

- (NSBundle *)bundle
{
  [_bundle autorelease];
  return [_bundle retain];
}

@end
