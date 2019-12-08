//
//  vlimit_helpers.c
//  vlimit
//
//  Created by Andrew Cavanagh on 12/7/19.
//  Copyright © 2019 andrewjmc. All rights reserved.
//

#include "vlimit_helpers.h"

#define CR(ret)\
do {\
OSStatus __s = (ret);\
if (__s != noErr) {\
return (__s);\
}\
} while (0)\

static inline AudioObjectID vlimit_get_audio_device()
{
  AudioObjectID audioId = kAudioObjectUnknown;
  UInt32 audioIdSize = sizeof(audioId);

  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
  addr.mScope = kAudioObjectPropertyScopeGlobal;
  addr.mElement = kAudioObjectPropertyElementMaster;

  if (!AudioObjectHasProperty(kAudioObjectSystemObject, &addr)) {
    return kAudioObjectUnknown;
  }

  if (AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                 &addr,
                                 0,
                                 NULL,
                                 &audioIdSize,
                                 &audioId) != noErr) {
    return kAudioObjectUnknown;
  }

  return audioId;
}

static OSStatus
vlimit_system_volume_changed(AudioObjectID inObjectID,
                             UInt32 inNumberAddresses,
                             const AudioObjectPropertyAddress *inAddresses,
                             void *inClientData)
{
  Float32 volume = 0;
  UInt32 volume_size = sizeof(volume);

  CR(AudioObjectGetPropertyData(inObjectID, inAddresses, 0, NULL, &volume_size, &volume));

  struct vlimit_helper *self = inClientData;

  if (volume > self->max_volume) {
    Boolean settable = 0;
    CR(AudioObjectIsPropertySettable(inObjectID, inAddresses, &settable));
    if (!settable) return 1;
    CR(AudioObjectSetPropertyData(inObjectID, inAddresses, 0, NULL, sizeof(self->max_volume), &self->max_volume));
  }

  return noErr;
}

static OSStatus vlimit_set_max_volume(struct vlimit_helper *self, Float32 maxVolume)
{
  self->max_volume = maxVolume;
  AudioObjectID prevAudioId = self->audioID;

  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioHardwareServiceDeviceProperty_VirtualMasterVolume;
  addr.mScope = kAudioObjectPropertyScopeOutput;
  addr.mElement = kAudioObjectPropertyElementMaster;

  self->audioID = vlimit_get_audio_device();

  if (prevAudioId != self->audioID) {
    CR(AudioObjectRemovePropertyListener(self->audioID,
                                         &addr,
                                         vlimit_system_volume_changed,
                                         self));
  }

  if (self->audioID == kAudioObjectUnknown) {
    return 1;
  }

  if (prevAudioId != self->audioID) {
    return AudioObjectAddPropertyListener(self->audioID,
                                          &addr,
                                          vlimit_system_volume_changed,
                                          self);
  }

  return noErr;
}

struct vlimit_helper get_vlimit_helper()
{
  struct vlimit_helper helper;
  helper.audioID = kAudioObjectUnknown;
  helper.max_volume = 100;
  helper.vlimit_set_max_volume = vlimit_set_max_volume;
  return helper;
}
