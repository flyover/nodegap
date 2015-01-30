/**
 * Copyright (c) Flyover Games, LLC
 */

#import <Cordova/CDV.h>

@interface NodePlugin : CDVPlugin

- (void) loop: (CDVInvokedUrlCommand*) command;
- (void) done: (CDVInvokedUrlCommand*) command;
- (void) exec: (CDVInvokedUrlCommand*) command;

@end