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

#include "base_types.h"
#include "network/objects/channels/i_h.h"
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
using google::protobuf::Message;

IhChannel::IhChannel() {
  params_ = NULL;
  state_.set_g(0);
  state_.set_m(0);
  state_.set_output(0);

  type_ = NU_TYPE_CHANNEL;
}

IhChannel::~IhChannel() {
}

string IhChannel::get_description() {
  return get_common_params()->name();
}

bool IhChannel::CalculateDerivatives(const float& time, const Message* state,
                                     Message* output) {
  const IhState* input_state = static_cast<const IhState*>(state);
  IhState* output_state = static_cast<IhState*>(output);

  double v = get_parent()->get_membrane_potential();

  // This provides a 1:1 compatibility with Synthesis.  Synthesis used an
  // approximation of exp for speed considerations back when programs were CPU
  // bound.
#ifdef USE_EXPX
  double tau_m = 1.0 / (expx(-14.59 - 0.086 * v) + expx(-1.87 + 0.0701 * v));
  double m = 1.0 / (1.0 + expx((v - params_->v_threshold()) / 5.5));
#else
  double tau_m = 1.0 / (exp(-14.59 - 0.086 * v) + exp(-1.87 + 0.0701 * v));
  double m = 1.0 / (1.0 + exp((v - params_->v_threshold()) / 5.5));
#endif
  double result = (m - input_state->m()) / tau_m;

  output_state->set_m(result);

  return true;
}

bool IhChannel::UpdateState(const float& time, const float& dt) {
  double g = params_->g_peak() * state_.m();
  state_.set_g(g);

  return true;
}

Message* IhChannel::get_state() {
  return &state_;
}

void IhChannel::set_params(Message* params) {
  params_ = static_cast<IhParams*>(params);
  // Set state values, probably should be in its own file
  state_.set_g(0);
  state_.set_m(0);
}

double IhChannel::get_current() {
  double current = -state_.g()
      * (get_parent()->get_membrane_potential() - params_->e_rev());
  return current;
}

google::protobuf::Message* IhChannel::get_unique_params() {
  return params_;
}

}  // namespace bigbrain
