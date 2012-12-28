/*
 Copyright 2012 Google Inc. All Rights Reserved.
 Author: blakely@google.com (Tim Blakely)

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include <stddef.h>

#include "network/objects/cells/pulse_integrate_and_fire.h"
#include "network/objects/network_unit.h"
#include "protos/neuron.pb.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

using std::string;

PulseIntegrateAndFire::PulseIntegrateAndFire() {
  params_ = NULL;

  // Set PIAF-specific information
  state_.set_firing_state(0);
  state_.set_membrane_potential(0);
  state_.set_threshold(0);
  state_.set_last_plastic_time(0);
  state_.set_last_spike_time(0);

  type_ = NU_TYPE_CELL;

  params_ = NULL;
}

PulseIntegrateAndFire::~PulseIntegrateAndFire() {
}

bool PulseIntegrateAndFire::CalculateDerivatives(
    const float& time, const google::protobuf::Message* input,
    google::protobuf::Message* output) {

  // Need to downcast.  Kludgy, but extendable
  const PulseIntegrateAndFireState* input_state =
      static_cast<const PulseIntegrateAndFireState *>(input);
  PulseIntegrateAndFireState* output_derivatives =
      static_cast<PulseIntegrateAndFireState *>(output);

  double currents = SummedChannelCurrents();

  // artifical cap on currents to keep it from NANing

  if (currents > 1000) {
    currents = 1000;
  }

  float firing =
      ((input_state->firing_state() & FS_FIRING)
          || (input_state->firing_state() & FS_SPIKING)) ? 1 : 0;

  // work with the threshold
  double threshold = -(input_state->threshold() - params_->resting_threshold())
      / params_->threshold_time_constant();
  output_derivatives->set_threshold(threshold);

  // this is super wordy... I wonder if the compiler can do compile-time
  // substitution without a performance hit, so we can do something like this:
  //
  // double gKLeak = params_->potassium_leak_conductance();
  // double E_K = params_->potassium_reversal_potential();

  double v_membrane = (-params_->potassium_leak_conductance()
      * (input_state->membrane_potential()
          - params_->potassium_reversal_potential())
      - params_->sodium_leak_conductance()
          * (input_state->membrane_potential()
              - params_->sodium_reversal_potential()) + currents)
      / params_->membrane_time_constant()
      - firing
          * (input_state->membrane_potential()
              - params_->potassium_reversal_potential())
          / params_->spike_time_constant();
  output_derivatives->set_membrane_potential(v_membrane);
  output_derivatives->set_last_plastic_time(0);
  output_derivatives->set_last_spike_time(0);

  return true;
}

bool PulseIntegrateAndFire::UpdateState(const float& time, const float& dt) {
  double time_since_last_spike = time - state_.last_spike_time();

  // If we're in a firing state
  if (state_.firing_state() & FS_FIRING) {
    if (time_since_last_spike > 0) {
      state_.set_firing_state(state_.firing_state() & ~FS_SPIKING);
      if (params_->plasticity()) {
        state_.set_firing_state(state_.firing_state() & !FS_PLASTIC);
      }
    }
    if (time_since_last_spike >= params_->spike_duration()) {
      state_.set_firing_state(state_.firing_state() | FS_RESTING);
      state_.set_firing_state(state_.firing_state() & ~FS_FIRING);
    }
  }
  if ((state_.firing_state() & FS_RESTING)
      && state_.membrane_potential() > state_.threshold()) {
    state_.set_firing_state(state_.firing_state() & ~FS_RESTING);
    state_.set_firing_state(state_.firing_state() | FS_FIRING);
    state_.set_firing_state(state_.firing_state() | FS_SPIKING);
    if (params_->plasticity()) {
      state_.set_firing_state(state_.firing_state() | FS_PLASTIC);
    }
    state_.set_membrane_potential(params_->sodium_reversal_potential());
    state_.set_threshold(params_->sodium_reversal_potential());
    state_.set_last_spike_time(time);
  }

  if (params_->plasticity()) {
    state_.set_last_plastic_time(time);
  }
  return true;
}

bool PulseIntegrateAndFire::RequiresUpdatingSynapses() {
  return state_.firing_state() & FS_SPIKING;
}

google::protobuf::Message* PulseIntegrateAndFire::get_state() {
  return &state_;
}

void PulseIntegrateAndFire::set_params(google::protobuf::Message* params) {
  // Need to downcast.  Kludgy, but extendable
  params_ = static_cast<PulseIntegrateAndFireParams*>(params);
  state_.set_membrane_potential(params_->resting_threshold() - 10);
  state_.set_threshold(params_->resting_threshold());
  state_.set_last_spike_time(-10000);
  state_.set_last_spike_time(-10000);
  state_.set_firing_state(FS_RESTING);
}

double PulseIntegrateAndFire::get_membrane_potential() {
  return state_.membrane_potential();
}

google::protobuf::Message* PulseIntegrateAndFire::get_unique_params() {
  return params_;
}

}  // namespace bigbrain
