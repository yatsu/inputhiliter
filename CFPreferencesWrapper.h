#import <Foundation/Foundation.h>

@interface CFPreferencesWrapper_APEBeepMultiplier : NSObject
{
    NSString *identifier;
}

- initWithApplication:(NSString*)appIdentifier;
+ preferencesWithApplication:(NSString*)appIdentifier;

- (NSArray*)arrayForKey:(NSString*)key;
- (BOOL)boolForKey:(NSString*)key;
- (NSData*)dataForKey:(NSString*)key;
- (NSDictionary*)dictionaryForKey:(NSString*)key;
- (float)floatForKey:(NSString*)key;
- (int)integerForKey:(NSString*)key;
- (id)objectForKey:(NSString*)key;
- (NSArray*)stringArrayForKey:(NSString*)key;
- (NSString*)stringForKey:(NSString*)key;

- (BOOL)keyExist:(NSString*)key;

- (void)removeObjectForKey:(NSString*)key;
- (void)setBool:(BOOL)value forKey:(NSString*)key;
- (void)setFloat:(float)value forKey:(NSString*)key;
- (void)setInteger:(int)value forKey:(NSString*)key;
- (void)setObject:(id)value forKey:(NSString*)key;
- (BOOL)synchronize;

@end
