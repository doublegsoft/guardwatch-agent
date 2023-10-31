/*
**                                  _________                   _____        ______
** _______ _____  ________ _______________  /___      ________ ___  /___________  /_
** __  __ `/_  / / /_  __ `/__  ___/_  __  / __ | /| / /_  __ `/_  __/_  ___/__  __ \
** _  /_/ / / /_/ / / /_/ / _  /    / /_/ /  __ |/ |/ / / /_/ / / /_  / /__  _  / / /
** _\__, /  \__,_/  \__,_/  /_/     \__,_/   ____/|__/  \__,_/  \__/  \___/  /_/ /_/
** /____/
**
** Copyright (C) 2020 doublegsoft.net
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

#include <string.h>
#include <gfc.h>

#include "gw_app.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (strong, nonatomic) NSStatusItem *statusItem;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  self.statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];

  NSString* resourcesPath = [[NSBundle mainBundle] resourcePath];
  NSString* pngIcon = [resourcesPath stringByAppendingString:@"/guardwatch-status.png"];

  NSImage* statusImage = [[NSImage alloc]initWithContentsOfFile:pngIcon];
  // black image
  [statusImage setTemplate:YES];
//  [self.statusItem setTitle:@"Hello"];
  [self.statusItem setImage:statusImage];
//  [self.statusItem setHighlightMode:YES];

  NSMenu *menu = [[NSMenu alloc] initWithTitle:@""];
  [menu addItemWithTitle:@"退出" action:@selector(terminate:) keyEquivalent:@"q"];
  [self.statusItem setMenu:menu];
//  [self.statusItem setAction:@selector(statusItemClicked:)];
}

- (void)statusItemClicked:(id)sender {
  NSLog(@"Status item clicked!");
}

@end

int _gw_snmp_daemon_running = 1;
int _gw_snmp_trap_running = 1;
int _gw_http_web_running = 1;
int _gw_os_collect_running = 1;

int main(int argc, const char* argv[]) {
  @autoreleasepool {
    NSString* homeDir = NSHomeDirectory();
    const char* home = [homeDir UTF8String];

    NSString* resourcesPath = [[NSBundle mainBundle] resourcePath];
    const char* resources = [resourcesPath UTF8String];

    char workdir[1024] = {'\0'};
    char logdir[1024] = {'\0'};

    strcpy(workdir, home);
    strcat(workdir, "/.guardwatch-agent");

    strcpy(logdir, workdir);
    strcat(logdir, "/log");

    gfc_gc_init();
    gfc_log_init(logdir, "guardwatch-agent");
    gw_app_start(workdir, resources);

    NSApplication* app = [NSApplication sharedApplication];
    AppDelegate* delegate = [[AppDelegate alloc] init];
    [app setDelegate:delegate];
    [app run];
  }
  return 0;
}