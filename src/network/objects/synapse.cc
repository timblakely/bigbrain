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

#include "network/objects/synapse.h"

#include <bits/stl_algobase.h>
#include <boost/circular_buffer/details.hpp>
#include <stddef.h>

#include "boost/circular_buffer/base.hpp"
#include "boost/circular_buffer/space_optimized.hpp"
#include "network/objects/neuron.h"

namespace bigbrain {

using boost::circular_buffer;
using boost::circular_buffer_space_optimized;

Synapse::Synapse()
    : parent_(NULL),
      target_(NULL),
      delayed_inputs_(1) {
}

Synapse::~Synapse() {
}

bool Synapse::AccumulateDelayedInput(const double& time_now) {
  double accumulation = 0;
  int num_accumulated = 0;
  for (boost::circular_buffer_space_optimized<Delayed>::iterator delayed =
      delayed_inputs_.begin(); delayed != delayed_inputs_.end(); ++delayed) {
    if (delayed->t_ <= time_now) {
      accumulation += delayed->value_;
    }
    ++num_accumulated;
  }

  // Need to do this so we don't invalidate iterators
  for (int i = 0; i < num_accumulated; ++i) {
    delayed_inputs_.pop_front();
  }
  add_delayed_input(accumulation);
  return true;
}

bool Synapse::DelayInput(const double& t, const double& value) {
  Delayed d;
  d.t_ = t;
  d.value_ = value;
  delayed_inputs_.push_back(d);
  return true;
}

void Synapse::SetLinkedNeurons(Neuron* parent, Neuron* target) {
  parent_ = parent;
  target_ = target;

  target_->AddInputSynapse(this);
}

Neuron* Synapse::get_parent() {
  return parent_;
}

Neuron* Synapse::get_target() {
  return target_;
}

}  // namespace bigbrain
