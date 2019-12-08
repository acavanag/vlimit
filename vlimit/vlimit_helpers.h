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
  AudioObjectID audioID;
  Float32 max_volume;

  OSStatus (*vlimit_set_max_volume)(struct vlimit_helper *self, Float32 max_volume);
};

extern struct vlimit_helper vlimit_start_service(void);

#endif /* vlimit_helpers_h */
