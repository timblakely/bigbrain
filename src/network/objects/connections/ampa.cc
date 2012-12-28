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

#include <boost/random/taus88.hpp>
#include <stddef.h>

#include "base_types.h"
#include "boost/random/uniform_01.hpp"
#include "network/objects/connections/ampa.h"
#include "network/objects/neuron.h"
#include "protos/basics.pb.h"
#include "protos/neuron.pb.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

using boost::random::uniform_01;
using std::string;

SynapseAmpa::SynapseAmpa()
    : params_(NULL) {
}

SynapseAmpa::~SynapseAmpa() {
}

bool SynapseAmpa::CalculateDerivatives(
    const float& time, const google::protobuf::Message* input_state,
    google::protobuf::Message* output_state) {
  return false;
}

bool SynapseAmpa::UpdateState(const float& time, const float& dt) {
  double decay;
  double newdGdt;
  double noiseScalar = 1.0;

  AccumulateDelayedInput(time);

  // note: ignoring Synthesis' noise cutofftable here

  uniform_01<> random;

  if (params_->noise_rate() > 0) {
    if (params_->noise_mode() == "Poisson") {
      if (random(rand_num_gen_) <= params_->noise_rate() * dt) {
        double input = state_.input()
            + noiseScalar * params_->noise_amplitude();
        state_.set_input(input);
      }
    }
  }

  decay = expx(-dt / params_->time_constant_1());

  newdGdt = state_.dg_dt() * decay
      + state_.input() / dt * params_->time_constant_1() * (1.0 - decay);
  decay = expx(-dt / params_->time_constant_2());
  double new_g = state_.g() * decay
      + state_.dg_dt() * params_->time_constant_2() * (1.0 - decay);
  double new_g_lower = state_.g() / params_->peak_response()
      * params_->g_peak();

  state_.set_g(new_g);
  state_.set_dg_dt(newdGdt);
  state_.set_g_lower(new_g_lower);
  state_.set_input(0);

  return true;
}

void SynapseAmpa::add_delayed_input(const double& value) {
  state_.set_input(state_.input() + value);
}

bool SynapseAmpa::UpdateConnection(const float& time, const float& dt) {
  // The following functions from Synthesis are not implemented:
  // > Short term plasticity
  // > Probabilistic modeling
  // > Connection strength (hardcoded to "5" to be in line with Synthesis)
  // > Mini spiking

  double delay = 0;

  uniform_01<> random;

  if (random(rand_num_gen_) <= params_->probability()) {
    DelayInput(time, state_.input() + 5);
  }

  return true;
}

double SynapseAmpa::get_current() {
  // The following functions from Synthesis are not implemented:
  // > Conductance tables
  double mp = get_parent()->get_membrane_potential();
  state_.set_output(
      (params_->reverse_potential() - get_parent()->get_membrane_potential())
          * state_.g_lower());
  return state_.output();
}

void SynapseAmpa::set_params(google::protobuf::Message* params) {
  // The following functions from Synthesis are not implemented:
  // > Conductance tables

  // Need to downcast. Kludgy, but extendable
  params_ = static_cast<AmpaParams*>(params);

  if (params_->time_constant_1() == params_->time_constant_2()) {
    params_->set_peak_response(params_->time_constant_1() / 2.718281828);
  } else {
    double tPeak = params_->time_constant_1() * params_->time_constant_2()
        / (params_->time_constant_1() - params_->time_constant_2())
        * log(params_->time_constant_1() / params_->time_constant_2());
    double peak_response = params_->time_constant_1()
        * params_->time_constant_2()
        / (params_->time_constant_1() - params_->time_constant_2())
        * (expx(-tPeak / params_->time_constant_1())
            - expx(-tPeak / params_->time_constant_2()));
    params_->set_peak_response(peak_response);
  }

  get_common_params()->set_diff_eq_solver(NONE);
}

google::protobuf::Message* SynapseAmpa::get_state() {
  return &state_;
}

string SynapseAmpa::get_description() {
  string description = "AMPA connection";
  return description;
}

google::protobuf::Message* SynapseAmpa::get_unique_params() {
  return params_;
}

}  // namespace bigbrain
