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

static inline OSStatus
vlimit_set_system_volume(AudioObjectID inObjectID, Float32 volume)
{
  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioHardwareServiceDeviceProperty_VirtualMasterVolume;
  addr.mScope = kAudioObjectPropertyScopeOutput;
  addr.mElement = kAudioObjectPropertyElementMaster;

  Boolean settable = 0;
  CR(AudioObjectIsPropertySettable(inObjectID, &addr, &settable));
  if (!settable) return 1;
  CR(AudioObjectSetPropertyData(inObjectID, &addr, 0, NULL, sizeof(volume), &volume));
  return noErr;
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
    CR(vlimit_set_system_volume(inObjectID, self->max_volume));
  }

  return noErr;
}

static OSStatus
vlimit_system_device_changed(AudioObjectID inObjectID,
                             UInt32 inNumberAddresses,
                             const AudioObjectPropertyAddress *inAddresses,
                             void *inClientData)
{
  struct vlimit_helper *self = inClientData;
  AudioObjectID previousID = self->audioID;

  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioHardwareServiceDeviceProperty_VirtualMasterVolume;
  addr.mScope = kAudioObjectPropertyScopeOutput;
  addr.mElement = kAudioObjectPropertyElementMaster;

  if (previousID != kAudioObjectUnknown) {
    CR(AudioObjectRemovePropertyListener(previousID,
                                         &addr,
                                         vlimit_system_volume_changed,
                                         self));
  }

  if ((self->audioID = vlimit_get_audio_device()) == kAudioObjectUnknown) {
    return 1;
  }

  Float32 volume = 0;
  UInt32 volume_size = sizeof(volume);
  CR(AudioObjectGetPropertyData(inObjectID, inAddresses, 0, NULL, &volume_size, &volume));

  if (volume > self->max_volume) {
    CR(vlimit_set_system_volume(self->audioID, self->max_volume));
  }

  CR(AudioObjectAddPropertyListener(self->audioID,
                                    &addr,
                                    vlimit_system_volume_changed,
                                    self));

  return noErr;
}

static OSStatus vlimit_set_max_volume(struct vlimit_helper *self, Float32 max_volume)
{
  self->max_volume = max_volume;
  return noErr;
}

struct vlimit_helper vlimit_start_service()
{
  struct vlimit_helper helper;
  helper.audioID = vlimit_get_audio_device();
  helper.vlimit_set_max_volume = vlimit_set_max_volume;
  helper.max_volume = 100;

  AudioObjectPropertyAddress addr;
  addr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
  addr.mScope = kAudioObjectPropertyScopeGlobal;
  addr.mElement = kAudioObjectPropertyElementMaster;

  AudioObjectAddPropertyListener(kAudioObjectSystemObject,
                                 &addr,
                                 vlimit_system_device_changed,
                                 &helper);
  return helper;
}
