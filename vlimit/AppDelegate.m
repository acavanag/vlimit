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
@property (nonatomic, strong) NSMenuItem *currentVolumeItem;
@property (nonatomic, strong) NSMenuItem *maxVolumeItem;
@end

@implementation AppDelegate
{
  struct vlimit_helper *_helper;
}

- (void)dealloc
{
  if (_helper != NULL) {
    free(_helper);
    _helper = NULL;
  }
}

static inline void update_volume(struct vlimit_helper *_h, NSMenuItem *current, NSMenuItem *limit) {
  dispatch_async(dispatch_get_main_queue(), ^{
    Float32 vol = _h->vlimit_get_volume(_h);
    Float32 max = _h->max_volume;
    current.title = [NSString stringWithFormat:@"Current Volume: %@", vol == -1 ? @"Error" : @(vol)];
    limit.title = [NSString stringWithFormat:@"Volume Limit: %@", vol == -1 ? @"Error" : @(max)];
  });
}

void volume_did_update(void *ctx) {
  AppDelegate *_self = (__bridge AppDelegate *)ctx;
  update_volume(_self->_helper, _self->_currentVolumeItem, _self->_maxVolumeItem);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  _statusItem = [NSStatusBar.systemStatusBar
                 statusItemWithLength:NSSquareStatusItemLength];

  NSMenu *menu = [NSMenu new];

  NSSlider *volumeSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(0, 0, 160, 16)];
  [volumeSlider setMinValue:0];
  [volumeSlider setMaxValue:1];
  [volumeSlider setTarget:self];
  [volumeSlider sendActionOn:NSEventMaskLeftMouseUp];
  [volumeSlider setAction:@selector(maxVolumeSliderDidChange:)];

  NSMenuItem *title = [[NSMenuItem alloc] initWithTitle:@"Volume Limit:"
                                                 action:nil
                                          keyEquivalent:@""];

  NSMenuItem *quit = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                action:@selector(terminate:)
                                         keyEquivalent:@""];
  [quit setTarget:[NSApplication sharedApplication]];

  NSMenuItem *currentVolume = [[NSMenuItem alloc] initWithTitle:@"Current Volume:"
                                                         action:nil
                                                  keyEquivalent:@""];

  NSMenuItem *volumeLimit = [[NSMenuItem alloc] initWithTitle:@"Volume Limit:"
                                                       action:nil
                                                keyEquivalent:@""];


  NSMenuItem *sliderItem = [NSMenuItem new];
  NSStackView *stackView = [NSStackView stackViewWithViews:@[volumeSlider]];
  [stackView setEdgeInsets:NSEdgeInsetsMake(0, 16, 0, 16)];
  sliderItem.view = stackView;

  [menu addItem:title];
  [menu addItem:[[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""]];
  [menu addItem:sliderItem];
  [menu addItem:[[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""]];
  [menu addItem:volumeLimit];
  [menu addItem:currentVolume];
  [menu addItem:[[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""]];
  [menu addItem:quit];

  _statusItem.button.image = [NSImage imageNamed:@"hearing"];
  _statusItem.menu = menu;

  NSNumber *maxVolumeObj = [[NSUserDefaults standardUserDefaults] objectForKey:MAX_VOLUME_KEY];
  Float32 maxVolume = maxVolumeObj ? [maxVolumeObj floatValue] : 100;
  [volumeSlider setFloatValue:maxVolume];

  _currentVolumeItem = currentVolume;
  _maxVolumeItem = volumeLimit;

  vlimit_start_service(&_helper, (__bridge void *)(self), volume_did_update);
  _helper->vlimit_set_max_volume(_helper, maxVolume);
  update_volume(_helper, _currentVolumeItem, _maxVolumeItem);
}

- (void)maxVolumeSliderDidChange:(NSSlider *)slider
{
  float value = [slider floatValue];

  _helper->vlimit_set_max_volume(_helper, value);
  [[NSUserDefaults standardUserDefaults] setFloat:value forKey:MAX_VOLUME_KEY];
  update_volume(_helper, _currentVolumeItem, _maxVolumeItem);
}

@end
