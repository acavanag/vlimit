//
//  AppDelegate.m
//  vlimit
//
//  Created by Andrew Cavanagh on 12/7/19.
//  Copyright © 2019 andrewjmc. All rights reserved.
//

#import "AppDelegate.h"

#import "vlimit_helpers.h"

#define MAX_VOLUME_KEY @"maxVolume"

@interface AppDelegate ()
@property (nonatomic, strong) NSStatusItem *statusItem;
@property (nonatomic, strong) NSSlider *maxVolumeSlider;
@end

@implementation AppDelegate
{
  struct vlimit_helper _helper;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  _statusItem = [NSStatusBar.systemStatusBar
                 statusItemWithLength:NSSquareStatusItemLength];

  NSMenu *menu = [NSMenu new];

  _maxVolumeSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(0, 0, 160, 16)];
  [_maxVolumeSlider setMinValue:0];
  [_maxVolumeSlider setMaxValue:1];
  [_maxVolumeSlider setTarget:self];
  [_maxVolumeSlider sendActionOn:NSEventMaskLeftMouseUp];
  [_maxVolumeSlider setAction:@selector(maxVolumeSliderDidChange:)];

  NSMenuItem *title = [[NSMenuItem alloc] initWithTitle:@"Maximum Volume:"
                                                 action:nil
                                          keyEquivalent:@""];

  NSMenuItem *quit = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                action:@selector(terminate:)
                                         keyEquivalent:@""];
  [quit setTarget:[NSApplication sharedApplication]];


  NSMenuItem *sliderItem = [NSMenuItem new];
  NSStackView *stackView = [NSStackView stackViewWithViews:@[_maxVolumeSlider]];
  [stackView setEdgeInsets:NSEdgeInsetsMake(0, 16, 0, 16)];
  sliderItem.view = stackView;

  [menu addItem:title];
  [menu addItem:[[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""]];
  [menu addItem:sliderItem];
  [menu addItem:[[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""]];
  [menu addItem:quit];

  _statusItem.button.image = [NSImage imageNamed:@"hearing"];
  _statusItem.menu = menu;

  NSNumber *maxVolumeObj = [[NSUserDefaults standardUserDefaults] objectForKey:MAX_VOLUME_KEY];
  Float32 maxVolume = maxVolumeObj ? [maxVolumeObj floatValue] : 100;
  [_maxVolumeSlider setFloatValue:maxVolume];

  _helper = get_vlimit_helper();
  _helper.vlimit_set_max_volume(&_helper, maxVolume);
}

- (void)maxVolumeSliderDidChange:(NSSlider *)slider
{
  float value = [slider floatValue];

  _helper.vlimit_set_max_volume(&_helper, value);
  [[NSUserDefaults standardUserDefaults] setFloat:value forKey:MAX_VOLUME_KEY];
}

@end
