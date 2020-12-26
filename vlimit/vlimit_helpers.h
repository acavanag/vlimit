//
//  vlimit_helpers.h
//  vlimit
//
//  Created by Andrew Cavanagh on 12/7/19.
//  Copyright © 2019 andrewjmc. All rights reserved.
//

#ifndef vlimit_helpers_h
#define vlimit_helpers_h

#include <stdio.h>

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>

struct vlimit_helper {
// Private API
  AudioObjectID audioID;
  Float32 max_volume;
  void *ctx;
  void (*volume_did_update)(void *ctx);

// Public API
  OSStatus (*vlimit_set_max_volume)(struct vlimit_helper *self, Float32 max_volume);
  Float32 (*vlimit_get_volume)(struct vlimit_helper *self);
};

// Constructor
extern OSStatus vlimit_start_service(struct vlimit_helper **helper,
                                     void *ctx,
                                     void (*volume_did_update)(void *ctx));

#endif /* vlimit_helpers_h */
