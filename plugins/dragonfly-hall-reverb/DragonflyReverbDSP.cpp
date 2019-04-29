/*
 * Dragonfly Reverb, a hall-style reverb plugin
 * Copyright (c) 2018 Michael Willis, Rob van den Berg
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE file.
 */

#include "DistrhoPlugin.hpp"
#include "DistrhoPluginInfo.h"
#include "DragonflyReverbDSP.hpp"

DragonflyReverbDSP::DragonflyReverbDSP(double sampleRate) {
  early.setSampleRate(sampleRate);
  early.setMuteOnChange(false);
  early.setdryr(0); // mute dry signal
  early.setwet(0); // 0dB
  early_send = 0.20;

  late.setSampleRate(sampleRate);
  late.setMuteOnChange(false);
  late.setdryr(0); // mute dry signal
  late.setwet(0); // 0dB

  for (uint32_t param = 0; param < paramCount; param++) {
    newParams[param] = banks[DEFAULT_BANK].presets[DEFAULT_PRESET].params[param];
    oldParams[param] = FP_NAN;
  }
}

float DragonflyReverbDSP::getParameterValue(uint32_t index) const {
  if (index < paramCount) {
    return newParams[index];
  }

  return 0.0;
}

void DragonflyReverbDSP::setParameterValue(uint32_t index, float value) {
  if (index < paramCount) {
    newParams[index] = value;
  }
}

void DragonflyReverbDSP::run(const float** inputs, float** outputs, uint32_t frames) {
  for (uint32_t index = 0; index < paramCount; index++) {
    if (d_isNotEqual(oldParams[index], newParams[index])) {
      oldParams[index] = newParams[index];
      float value = newParams[index];

      switch(index) {
        case     paramDry_level: dry_level        = (value / 100.0); break;
        case   paramEarly_level: early_level      = (value / 100.0); break;
        case    paramLate_level: late_level       = (value / 100.0); break;
        case          paramSize: early.setRSFactor  (value / 10.0);
                                 late.setRSFactor   (value / 80.0);  break;
        case         paramWidth: early.setwidth     (value / 100.0);
                                 late.setwidth      (value / 100.0); break;
        case      paramPredelay: late.setPreDelay   (value);         break;
        case       paramDiffuse: late.setidiffusion1(value / 140.0);
                                 late.setapfeedback (value / 140.0); break;
        case       paramLowCut:  early.setoutputhpf (value);
                                 late.setoutputhpf  (value);         break;
        case     paramLowXover:  late.setxover_low  (value);         break;
        case      paramLowMult:  late.setrt60_factor_low(value);     break;
        case      paramHighCut:  early.setoutputlpf (value);
                                 late.setoutputlpf  (value);         break;
        case    paramHighXover:  late.setxover_high (value);         break;
        case     paramHighMult:  late.setrt60_factor_high(value);    break;
        case          paramSpin: late.setspin       (value);         break;
        case        paramWander: late.setwander     (value);         break;
        case         paramDecay: late.setrt60       (value);         break;
      }
    }
  }


  for (uint32_t offset = 0; offset < frames; offset += BUFFER_SIZE) {
    long int buffer_frames = frames - offset < BUFFER_SIZE ? frames - offset : BUFFER_SIZE;

    early.processreplace(
        const_cast<float *>(inputs[0] + offset),
        const_cast<float *>(inputs[1] + offset),
        early_out_buffer[0],
        early_out_buffer[1],
        buffer_frames
    );

    for (uint32_t i = 0; i < buffer_frames; i++) {
      late_in_buffer[0][i] = early_send * early_out_buffer[0][i] + inputs[0][offset + i];
      late_in_buffer[1][i] = early_send * early_out_buffer[1][i] + inputs[1][offset + i];
    }

    late.processreplace(
      const_cast<float *>(late_in_buffer[0]),
      const_cast<float *>(late_in_buffer[1]),
      late_out_buffer[0],
      late_out_buffer[1],
      buffer_frames
    );

    for (uint32_t i = 0; i < buffer_frames; i++) {
      outputs[0][offset + i] =
        dry_level   * inputs[0][offset + i]  +
        early_level * early_out_buffer[0][i] +
        late_level  * late_out_buffer[0][i];

      outputs[1][offset + i] =
        dry_level   * inputs[1][offset + i]  +
        early_level * early_out_buffer[1][i] +
        late_level  * late_out_buffer[1][i];
    }
  }
}

void DragonflyReverbDSP::sampleRateChanged(double newSampleRate) {
    early.setSampleRate(newSampleRate);
    late.setSampleRate(newSampleRate);
}

void DragonflyReverbDSP::mute() {
  early.mute();
  late.mute();
}