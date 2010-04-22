
#import <Carbon/Carbon.h>
#import "CFPreferencesWrapper.h"


@implementation CFPreferencesWrapper_APEBeepMultiplier

- initWithApplication:(NSString*)appIdentifier
{
    self = [super init];
    
    if( !self || !appIdentifier )
    {
        [self release];
        return nil;
    }
    
    identifier = [appIdentifier retain];
    
    return self;
}

- (void)dealloc
{
    [identifier release];
    
    [super dealloc];
}

+ preferencesWithApplication:(NSString*)appIdentifier
{
    return [[[self alloc] initWithApplication:appIdentifier] autorelease];
}


- (NSArray*)arrayForKey:(NSString*)key
{
    id object;
    
    object = (id) CFPreferencesCopyAppValue( (CFStringRef) key, (CFStringRef) identifier);
    
    if( object && (CFGetTypeID( object) == CFArrayGetTypeID()) )
    	return [object autorelease];
    else
    {
        [object release];
        return nil;
    }
}

- (BOOL)boolForKey:(NSString*)key
{
    Boolean dummy;
    
    return CFPreferencesGetAppBooleanValue( (CFStringRef) key, (CFStringRef) identifier, &dummy);
}

- (NSData*)dataForKey:(NSString*)key
{
    id object;
    
    object = (id) CFPreferencesCopyAppValue( (CFStringRef) key, (CFStringRef) identifier);
    
    if( object && (CFGetTypeID( object) == CFDataGetTypeID()) )
    	return [object autorelease];
    else
    {
        [object release];
        return nil;
    }
}

- (NSDictionary*)dictionaryForKey:(NSString*)key
{
    id object;
    
    object = (id) CFPreferencesCopyAppValue( (CFStringRef) key, (CFStringRef) identifier);
    
    if( object && (CFGetTypeID( object) == CFDictionaryGetTypeID()) )
    	return [object autorelease];
    else
    {
        [object release];
        return nil;
    }
}

- (float)floatForKey:(NSString*)key
{
    id object;
    float retVal;
    
    object = (id) CFPreferencesCopyAppValue( (CFStringRef) key, (CFStringRef) identifier);
    
    if( object && (CFGetTypeID( object) == CFNumberGetTypeID()) )
    {
        retVal = [object floatValue];
        [object release];
    	return retVal;
    }
    else
    {
        [object release];
        return 0;
    }
}

- (int)integerForKey:(NSString*)key
{
    Boolean dummy;
    
    return CFPreferencesGetAppIntegerValue( (CFStringRef) key, (CFStringRef) identifier, &dummy);
}

- (id)objectForKey:(NSString*)key
{
    return [(id) CFPreferencesCopyAppValue( (CFStringRef) key, (CFStringRef) identifier) autorelease];
}

- (NSArray*)stringArrayForKey:(NSString*)key
{
    id object;
    
    object = (id) CFPreferencesCopyAppValue( (CFStringRef) key, (CFStringRef) identifier);
    
    if( object && (CFGetTypeID( object) == CFArrayGetTypeID()) )
    {
        NSEnumerator *enumerator = [object objectEnumerator];
        id element;
        
        while( (element = [enumerator nextObject]) )
        {
            if( CFGetTypeID( element) != CFStringGetTypeID() )
            {
                [object release];
                return nil;
            }
        }
        
    	return [object autorelease];
    }
    else
    {
        [object release];
        return nil;
    }
}

- (NSString*)stringForKey:(NSString*)key
{
    id object;
    
    object = (id) CFPreferencesCopyAppValue( (CFStringRef) key, (CFStringRef) identifier);
    
    if( object && (CFGetTypeID( object) == CFStringGetTypeID()) )
    	return [object autorelease];
    else
    {
        [object release];
        return nil;
    }
}


- (void)removeObjectForKey:(NSString*)key
{
    CFPreferencesSetAppValue( (CFStringRef) key, (CFPropertyListRef) NULL, (CFStringRef) identifier);
}

- (void)setBool:(BOOL)value forKey:(NSString*)key
{
    CFPreferencesSetAppValue( (CFStringRef) key, (CFPropertyListRef) (value ? kCFBooleanTrue : kCFBooleanFalse), (CFStringRef) identifier);
}

- (void)setFloat:(float)value forKey:(NSString*)key
{
    CFPreferencesSetAppValue( (CFStringRef) key, (CFPropertyListRef) [NSNumber numberWithFloat:value], (CFStringRef) identifier);
}

- (void)setInteger:(int)value forKey:(NSString*)key
{
    CFPreferencesSetAppValue( (CFStringRef) key, (CFPropertyListRef) [NSNumber numberWithInt:value], (CFStringRef) identifier);
}

- (void)setObject:(id)value forKey:(NSString*)key
{
    CFPreferencesSetAppValue( (CFStringRef) key, (CFPropertyListRef) value, (CFStringRef) identifier);
}

- (BOOL)keyExist:(NSString*)key
{
    BOOL exist = NO;
    
    CFPropertyListRef value = CFPreferencesCopyAppValue((CFStringRef)key, (CFStringRef)identifier);
    if (value)
    {
        exist = YES;
        CFRelease(value);
    }
    
    return exist;
}

- (BOOL)synchronize
{
    return CFPreferencesAppSynchronize( (CFStringRef) identifier);
}

@end
