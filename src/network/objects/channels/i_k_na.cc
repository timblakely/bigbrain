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

#include "network/objects/channels/i_k_na.h"
#include "network/objects/network_unit.h"
#include "network/objects/neuron.h"
#include "protos/neuron.pb.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

using std::string;

// Initializes all the parameters of an IKNaDesc

IKNaChannel::IKNaChannel() {
  params_ = NULL;

  state_.set_g(0);
  state_.set_na(0);
  state_.set_output(0);

  type_ = NU_TYPE_CHANNEL;
}

// Returns the channel's not-necessarily-unique name
string IKNaChannel::get_description() {
  return get_common_params()->name();
}

google::protobuf::Message* IKNaChannel::get_unique_params() {
  return params_;
}

bool IKNaChannel::CalculateDerivatives(const float& time,
                                       const google::protobuf::Message* state,
                                       google::protobuf::Message* output) {
  const IKNaState* input_state = static_cast<const IKNaState*>(state);
  IKNaState* output_state = static_cast<IKNaState*>(output);

  double Na_i = input_state->na();
  double Vm = get_parent()->get_membrane_potential();

  double sodiumInflux = 1.0
      / (1
          + exp(
              -(Vm - params_->sodium_threshold())
                  / params_->sodium_threshold_slope()));

  double na = params_->sodium_influx_peak() * sodiumInflux
      - (Na_i * (1 - params_->sodium_equilibrium()))
          / params_->sodium_time_constant();

  output_state->set_na(na);
  return true;
}

bool IKNaChannel::UpdateState(const float& time, const float& dt) {
  double Na_i = state_.na();
  double w_inf = 1.0 / (1.0 + pow(0.25 / Na_i, 3.5));

  state_.set_g(-params_->g_peak() * w_inf);
  return true;
}

google::protobuf::Message* IKNaChannel::get_state() {
  return &state_;
}

void IKNaChannel::set_params(google::protobuf::Message* params) {
  // Need to downcast.  Kludgy, but extendable
  params_ = static_cast<IKNaParams*>(params);
  state_.set_g(0);
  state_.set_na(params_->sodium_equilibrium());
}

double IKNaChannel::get_current() {
  double current = state_.g()
      * (get_parent()->get_membrane_potential() - params_->e_rev());
  return current;
}

}  // namespace bigbrain
