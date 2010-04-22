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

#include <mach-o/dyld.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <ApplicationEnhancer/ApplicationEnhancer.h>
#include <ApplicationEnhancer/APETools.h>

#if __LP64__ || TARGET_OS_EMBEDDED || TARGET_OS_IPHONE || TARGET_OS_WIN32 || NS_BUILD_32_LIKE_64
typedef unsigned long NSUInteger;
#else
typedef unsigned int NSUInteger;
#endif

struct objc_method {
  char *method_name;
  char *method_types;
  void *method_imp;
};

void * NSSelectorFromString(CFStringRef string);
void * objc_msgSend(void *target, char *selector, ...);
void objc_msgSend_stret(void *stret, void *target, char *selector, ...);
struct objc_method * class_getInstanceMethod(void *inClass, void *selector);
void * objc_getClass(const char *name);

extern void *NSFontAttributeName;
extern void *NSParagraphStyleAttributeName;
extern void *NSForegroundColorAttributeName;
extern void *NSUnderlineStyleAttributeName;
extern void *NSSuperscriptAttributeName;
extern void *NSBackgroundColorAttributeName;
extern void *NSAttachmentAttributeName;
extern void *NSLigatureAttributeName;
extern void *NSBaselineOffsetAttributeName;
extern void *NSKernAttributeName;
extern void *NSLinkAttributeName;
extern void *NSLayoutManager;


// NSTextView
// - (void)setMarkedText:(id)string selectionRange:(CFRange)selectionRange replacementRange:(CFRange)replacementRange
typedef void (*NSTextView_setMarkedText_selectedRange_replacementRangeProcPtr)(void *inObj, char *inSel, void *aString, CFRange selRange, CFRange replRange);

NSTextView_setMarkedText_selectedRange_replacementRangeProcPtr
  gNSTextView_setMarkedText_selectedRange_replacementRange = NULL;

void
IH_NSTextView_setMarkedText_selectedRange_replacementRange(void *inObj, char *inSel, void *aString, CFRange selRange, CFRange replRange);

// reloads our preferences
static void My_ReloadPrefs();


void
APEBundleMainEarlyLoad(CFBundleRef inBundle, CFStringRef inAPEToolsApplicationID)
{
  void *nsClass;
  struct objc_method* method;
  
  // check if this application is in the exclude list;
  if (APEToolsIsInExcludeList(CFSTR("info.yatsu.InputHiliter"), NULL)) {
     apeprintf("InputHiliter: not loading as this application is excluded.\n");
     return;
  }
  
  // load preferences
  My_ReloadPrefs();
  
  if ((nsClass = objc_getClass("NSTextView"))) {
    if ((method =
         class_getInstanceMethod(nsClass,
                                 NSSelectorFromString(CFSTR("setMarkedText:selectedRange:replacementRange:"))))) {
      gNSTextView_setMarkedText_selectedRange_replacementRange =
        (NSTextView_setMarkedText_selectedRange_replacementRangeProcPtr)
          APEPatchCreate(method->method_imp,
                         &IH_NSTextView_setMarkedText_selectedRange_replacementRange);
    }
  }

  //apeprintf("InputHiliter: APEBundleMainEarlyLoad()...done\n");
}

void
IH_NSTextView_setMarkedText_selectedRange_replacementRange(void *inObj, char *inSel, void *aString, CFRange selRange, CFRange replRange)
{
  CFRange (*markedRangeFunc)(void *id, void *SEL) = (CFRange(*)(void *id, void *SEL)) objc_msgSend;
  CFRange range = CFRangeMake(0, 0);
  CFIndex index = range.location + range.length;
  NSUInteger length = (NSUInteger) objc_msgSend(aString, NSSelectorFromString(CFSTR("length")));
  void *layoutManager = NULL;
  CFDictionaryRef attr;
  CFRange markedRange = {0, 0};
  static void *activeForeground = NULL;
  static void *otherForeground = NULL;
  static void *activeBackground = NULL;
  static void *otherBackground = NULL;
  
  if (!activeForeground) {
    void *nsColor = objc_getClass("NSColor");
    if (nsColor) {
      activeForeground =
        objc_msgSend(nsColor, NSSelectorFromString(CFSTR("selectedMenuItemTextColor")));
    
      otherForeground =
        objc_msgSend(nsColor, NSSelectorFromString(CFSTR("selectedTextColor")));
    
      activeBackground =
        objc_msgSend(nsColor, NSSelectorFromString(CFSTR("selectedMenuItemColor")));
    
      otherBackground =
        objc_msgSend(nsColor, NSSelectorFromString(CFSTR("selectedTextBackgroundColor")));
    }
  }
  
  gNSTextView_setMarkedText_selectedRange_replacementRange(inObj, inSel, aString, selRange, replRange);
  
  //objc_msgSend_stret(&markedRange, inObj, NSSelectorFromString(CFSTR("markedRange"))); // crash on Intel Macs
  markedRange = markedRangeFunc(inObj, NSSelectorFromString(CFSTR("markedRange")));
  
  if (!layoutManager) {
    layoutManager = objc_msgSend(inObj, NSSelectorFromString(CFSTR("layoutManager")));
    if (!layoutManager) return;
  }
  
  while (index < length &&
         (attr = (CFDictionaryRef)
          objc_msgSend(aString, NSSelectorFromString(CFSTR("attributesAtIndex:effectiveRange:")),
                       index, &range))) {
    CFNumberRef underline;
    int val = 0;
    CFRange updateRange = {
      markedRange.location + range.location,
      range.length
    };
    
    underline = CFDictionaryGetValue(attr, NSUnderlineStyleAttributeName);
    if (underline)
      CFNumberGetValue(underline, kCFNumberIntType, &val);
    
    if (val) {
      CFMutableDictionaryRef mAttr = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attr);
      switch (val) {
        case 1:
          if (otherForeground)
            CFDictionaryAddValue(mAttr, NSForegroundColorAttributeName,
                                 otherForeground);
          if (otherBackground)
            CFDictionaryAddValue(mAttr, NSBackgroundColorAttributeName,
                                 otherBackground);
        break;
          
        case 2:
          if (activeForeground)
            CFDictionaryAddValue(mAttr, NSForegroundColorAttributeName,
                                 activeForeground);
          if (activeBackground)
            CFDictionaryAddValue(mAttr, NSBackgroundColorAttributeName,
                                 activeBackground);
        break;
      }
      objc_msgSend(layoutManager,
                   NSSelectorFromString(CFSTR("setTemporaryAttributes:forCharacterRange:")),
                   mAttr, updateRange);
    }
    index = range.location + range.length;
  }
}

OSStatus
APEBundleMessage(CFStringRef message,CFDataRef inData,CFDataRef *outData)
{
  // request to reload prefs from our preference pane
  if (CFStringCompare(message, CFSTR("Refresh"), 0) == kCFCompareEqualTo) {
    My_ReloadPrefs();
  }
    
  return noErr;
}

// Reload our settings from the info.yatsu.InputHiliter.plist
static void
My_ReloadPrefs()
{
  //Boolean keyExists;
  CFPreferencesAppSynchronize(CFSTR("info.yatsu.InputHiliter"));
}
