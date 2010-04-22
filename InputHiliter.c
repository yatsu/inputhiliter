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

#define kApp CFSTR("info.yatsu.InputHiliter")

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

static void *activeForeground = NULL;
static void *activeBackground = NULL;
static void *otherForeground = NULL;
static void *otherBackground = NULL;

#define kDefaultActiveForeground CFSTR("1.000000 1.000000 1.000000 1.000000")
#define kDefaultActiveBackground CFSTR("0.000000 0.501961 1.000000 1.000000")
#define kDefaultOtherForeground CFSTR("0.298039 0.298039 0.298039 1.000000")
#define kDefaultOtherBackground CFSTR("0.648860 0.792326 1.000000 1.000000")

// NSTextView
// - (void)setMarkedText:(id)string selectionRange:(CFRange)selectionRange replacementRange:(CFRange)replacementRange
typedef void (*NSTextView_setMarkedText_selectedRange_replacementRangeProcPtr)(void *inObj, char *inSel, void *aString, CFRange selRange, CFRange replRange);

NSTextView_setMarkedText_selectedRange_replacementRangeProcPtr
  gNSTextView_setMarkedText_selectedRange_replacementRange = NULL;

void
IH_NSTextView_setMarkedText_selectedRange_replacementRange(void *inObj, char *inSel, void *aString, CFRange selRange, CFRange replRange);

// reloads our preferences
static void My_ReloadPrefs();


void APEBundleMainEarlyLoad(CFBundleRef inBundle, CFStringRef inAPEToolsApplicationID)
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

void IH_NSTextView_setMarkedText_selectedRange_replacementRange(void *inObj, char *inSel, void *aString, CFRange selRange, CFRange replRange)
{
  CFRange (*markedRangeFunc)(void *id, void *SEL) = (CFRange(*)(void *id, void *SEL)) objc_msgSend;
  CFRange range = CFRangeMake(0, 0);
  CFIndex index = range.location + range.length;
  NSUInteger length = (NSUInteger) objc_msgSend(aString, NSSelectorFromString(CFSTR("length")));
  void *layoutManager = NULL;
  CFDictionaryRef attr;
  CFRange markedRange = {0, 0};

  gNSTextView_setMarkedText_selectedRange_replacementRange(inObj, inSel, aString, selRange, replRange);

  markedRange = markedRangeFunc(inObj, NSSelectorFromString(CFSTR("markedRange")));

  layoutManager = objc_msgSend(inObj, NSSelectorFromString(CFSTR("layoutManager")));
  if (!layoutManager) return;

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
          CFDictionaryAddValue(mAttr, NSForegroundColorAttributeName,
                               otherForeground);
          CFDictionaryAddValue(mAttr, NSBackgroundColorAttributeName,
                               otherBackground);
        break;

        case 2:
          CFDictionaryAddValue(mAttr, NSForegroundColorAttributeName,
                               activeForeground);
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

OSStatus APEBundleMessage(CFStringRef message,CFDataRef inData,CFDataRef *outData)
{
  // request to reload prefs from our preference pane
  if (CFStringCompare(message, CFSTR("Refresh"), 0) == kCFCompareEqualTo) {
    My_ReloadPrefs();
  }

  return noErr;
}

// Reload our settings from the info.yatsu.InputHiliter.plist
static void My_ReloadPrefs()
{
  CFStringRef str = NULL;
  CFStringRef activeFore, activeBack, otherFore, otherBack;
  double red, green, blue, alpha;
  void *nsColor = objc_getClass("NSColor");

  if (activeForeground)
	objc_msgSend(activeForeground, NSSelectorFromString(CFSTR("release")));
  if (activeBackground)
	objc_msgSend(activeBackground, NSSelectorFromString(CFSTR("release")));
  if (otherForeground)
	objc_msgSend(otherForeground, NSSelectorFromString(CFSTR("release")));
  if (otherBackground)
	objc_msgSend(otherBackground, NSSelectorFromString(CFSTR("release")));

  CFPreferencesAppSynchronize(kApp);

  str = (CFStringRef)CFPreferencesCopyAppValue(CFSTR("activeForeground"), kApp);
  activeFore = str ? str : kDefaultActiveForeground;
  sscanf(CFStringGetCStringPtr(activeFore, kCFStringEncodingUTF8), "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
  if (nsColor) {
	activeForeground = objc_msgSend(nsColor, NSSelectorFromString(CFSTR("colorWithCalibratedRed:green:blue:alpha:")),
									red, green, blue, alpha);
	objc_msgSend(activeForeground, NSSelectorFromString(CFSTR("retain")));
  }
  if (str) CFRelease(str);

  str = (CFStringRef)CFPreferencesCopyAppValue(CFSTR("activeBackground"), kApp);
  activeBack = str ? str : kDefaultActiveBackground;
  sscanf(CFStringGetCStringPtr(activeBack, kCFStringEncodingUTF8), "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
  if (nsColor) {
	activeBackground = objc_msgSend(nsColor, NSSelectorFromString(CFSTR("colorWithCalibratedRed:green:blue:alpha:")),
									red, green, blue, alpha);
	objc_msgSend(activeBackground, NSSelectorFromString(CFSTR("retain")));
  }
  if (str) CFRelease(str);

  str = (CFStringRef)CFPreferencesCopyAppValue(CFSTR("otherForeground"), kApp);
  otherFore = str ? str : kDefaultOtherForeground;
  sscanf(CFStringGetCStringPtr(otherFore, kCFStringEncodingUTF8), "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
  if (nsColor) {
	otherForeground = objc_msgSend(nsColor, NSSelectorFromString(CFSTR("colorWithCalibratedRed:green:blue:alpha:")),
									red, green, blue, alpha);
	objc_msgSend(otherForeground, NSSelectorFromString(CFSTR("retain")));
  }
  if (str) CFRelease(str);

  str = (CFStringRef)CFPreferencesCopyAppValue(CFSTR("otherBackground"), kApp);
  otherBack = str ? str : kDefaultOtherBackground;
  sscanf(CFStringGetCStringPtr(otherBack, kCFStringEncodingUTF8), "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
  if (nsColor) {
	otherBackground = objc_msgSend(nsColor, NSSelectorFromString(CFSTR("colorWithCalibratedRed:green:blue:alpha:")),
									red, green, blue, alpha);
	objc_msgSend(otherBackground, NSSelectorFromString(CFSTR("retain")));
  }
  if (str) CFRelease(str);
}
