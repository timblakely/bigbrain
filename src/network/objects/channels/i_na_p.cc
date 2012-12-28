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

#include "network/objects/channels/i_na_p.h"

#include <stddef.h>

#include "network/objects/neuron.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

using std::string;
using google::protobuf::Message;

INaPChannel::INaPChannel() {
  // TODO(blakely): Auto-generated constructor stub
  params_ = NULL;
  state_.set_g(0);
  state_.set_output(0);

  type_ = NU_TYPE_CHANNEL;
}

INaPChannel::~INaPChannel() {
}

string INaPChannel::get_description() {
  return get_common_params()->name();
}

bool INaPChannel::CalculateDerivatives(const float& time, const Message* state,
                                       Message* output) {
  const INaPState* input_state = static_cast<const INaPState*>(state);
  INaPState* output_state = static_cast<INaPState*>(output);

  // There wasn't anything in the original Synthesis implementation, so this
  // function is empty

  return true;
}

bool INaPChannel::UpdateState(const float& time, const float& dt) {
  double v = get_parent()->get_membrane_potential();
  double g = params_->g_peak()
      * (1 / (1 + exp(-(v - params_->threshold()) / params_->slope())));

  state_.set_g(g);

  return true;
}

Message* INaPChannel::get_state() {
  return &state_;
}

void INaPChannel::set_params(Message* params) {
  params_ = static_cast<INaPParams*>(params);

  // Set state values, probably should be in its own file
  state_.set_g(0);
}

double INaPChannel::get_current() {
  double asdf = get_parent()->get_membrane_potential();
  double current = -state_.g()
      * (get_parent()->get_membrane_potential() - params_->e_rev());
  return current;
}

google::protobuf::Message* INaPChannel::get_unique_params() {
  return params_;
}

}  // namespace bigbrain
