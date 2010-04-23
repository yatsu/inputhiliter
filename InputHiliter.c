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

void *NSSelectorFromString(CFStringRef string);
void *objc_msgSend(void *target, char *selector, ...);
void objc_msgSend_stret(void *stret, void *target, char *selector, ...);
struct objc_method *class_getInstanceMethod(void *inClass, void *selector);
void *objc_getClass(const char *name);

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

static void *colors[] = { NULL, NULL, NULL, NULL };

#define kActiveForeground 0
#define kActiveBackground 1
#define kOtherForeground  2
#define kOtherBackground  3

CFStringRef colorNames[] = {
	CFSTR("activeForeground"),
	CFSTR("activeBackground"),
	CFSTR("otherForeground"),
	CFSTR("otherBackground")
};

CFStringRef defaultColors[] = {
	CFSTR("1.000000 1.000000 1.000000 1.000000"),
	CFSTR("0.000000 0.501961 1.000000 1.000000"),
	CFSTR("0.298039 0.298039 0.298039 1.000000"),
	CFSTR("0.648860 0.792326 1.000000 1.000000")
};

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
	struct objc_method *method;


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

	// apeprintf("InputHiliter: APEBundleMainEarlyLoad()...done\n");
}

void IH_NSTextView_setMarkedText_selectedRange_replacementRange(void *inObj, char *inSel, void *aString, CFRange selRange, CFRange replRange)
{
	CFRange (*markedRangeFunc)(void *id, void *SEL) = (CFRange (*)(void *id, void *SEL))objc_msgSend;
	CFRange range = CFRangeMake(0, 0);
	CFIndex index = range.location + range.length;
	NSUInteger length = (NSUInteger)objc_msgSend(aString, NSSelectorFromString(CFSTR("length")));
	void *layoutManager = NULL;
	CFDictionaryRef attr;
	CFRange markedRange = { 0, 0 };

	gNSTextView_setMarkedText_selectedRange_replacementRange(inObj, inSel, aString, selRange, replRange);

	markedRange = markedRangeFunc(inObj, NSSelectorFromString(CFSTR("markedRange")));

	layoutManager = objc_msgSend(inObj, NSSelectorFromString(CFSTR("layoutManager")));
	if (!layoutManager) return;

	while (index < length && (attr = (CFDictionaryRef)objc_msgSend(aString, NSSelectorFromString(CFSTR("attributesAtIndex:effectiveRange:")), index, &range))) {
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
										 colors[kOtherForeground]);
					CFDictionaryAddValue(mAttr, NSBackgroundColorAttributeName,
										 colors[kOtherBackground]);
					break;

				case 2:
					CFDictionaryAddValue(mAttr, NSForegroundColorAttributeName,
										 colors[kActiveForeground]);
					CFDictionaryAddValue(mAttr, NSBackgroundColorAttributeName,
										 colors[kActiveBackground]);
					break;
			}
			objc_msgSend(layoutManager,
						 NSSelectorFromString(CFSTR("setTemporaryAttributes:forCharacterRange:")),
						 mAttr, updateRange);
		}

		index = range.location + range.length;
	}
}

OSStatus APEBundleMessage(CFStringRef message, CFDataRef inData, CFDataRef *outData)
{
	// request to reload prefs from our preference pane
	if (CFStringCompare(message, CFSTR("Refresh"), 0) == kCFCompareEqualTo) {
		My_ReloadPrefs();
	}

	return noErr;
}

void loadColor(int index) {
	CFStringRef colorStr, str = NULL;
	double red, green, blue, alpha;
	void *nsColor = objc_getClass("NSColor");
	void *nsColorSpace = objc_getClass("NSColorSpace");
	CGFloat compos[4];
	void *colorSpace;
	void *rgb;

	if (colors[index])
		objc_msgSend(colors[index], NSSelectorFromString(CFSTR("release")));
	
	str = (CFStringRef)CFPreferencesCopyAppValue(colorNames[index], kApp);
	colorStr = str ? str : defaultColors[index];
	sscanf(CFStringGetCStringPtr(colorStr, kCFStringEncodingUTF8), "%lf %lf %lf %lf", &red, &green, &blue, &alpha);
	compos[0] = red;
	compos[1] = green;
	compos[2] = blue;
	compos[3] = alpha;
	colorSpace = objc_msgSend(nsColorSpace, NSSelectorFromString(CFSTR("sRGBColorSpace")));
	rgb = objc_msgSend(nsColor, NSSelectorFromString(CFSTR("colorWithColorSpace:components:count:")), colorSpace, compos, 4);
	colors[index] = objc_msgSend(rgb, NSSelectorFromString(CFSTR("colorUsingColorSpaceName:")),
								 CFSTR("NSCalibratedRGBColorSpace"));
	objc_msgSend(colors[index], NSSelectorFromString(CFSTR("retain")));
	if (str) CFRelease(str);
}

// Reload our settings from the info.yatsu.InputHiliter.plist
static void My_ReloadPrefs()
{
	int i;

	CFPreferencesAppSynchronize(kApp);

	for (i = 0; i < 4; i++) {
		loadColor(i);
	}
}
