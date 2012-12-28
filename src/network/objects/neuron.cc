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

#include "network/objects/neuron.h"

#include <bits/basic_string.h>
#include <bits/basic_string.tcc>
#include <bits/char_traits.h>

#include "network/objects/channel.h"
#include "network/objects/synapse.h"
#include "protos/neuron.pb.h"

namespace bigbrain {

using std::string;

Neuron::Neuron() {
}

Neuron::~Neuron() {
  // Free neurons
  if (channels_.size() > 0) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }

  // Free synapses
  if (synapses_.size() > 0) {
    for (auto synapse : synapses_) {
      delete synapse;
    }
    synapses_.clear();
  }
}

double Neuron::SummedChannelCurrents() {
  double current = 0;
  for (Channel *channel : channels_) {
    current += channel->get_current();
  }
  for (Synapse* synapse : input_synapses_) {
    current += synapse->get_current();
  }
  return current;
}

void Neuron::AddChannel(Channel* channel) {
  channel->set_parent(this);
  channels_.push_back(channel);
}

void Neuron::AddSynapse(Synapse* synapse) {
  synapses_.push_back(synapse);
}

void Neuron::AddInputSynapse(Synapse* synapse) {
  input_synapses_.push_back(synapse);
}

string Neuron::get_description() {
  string description;

  description += get_common_params()->name();
  /*+ StringPrintf("[%"GG_LL_FORMAT"u, pos<%f,%f,%f>]\n",
   get_common_params()->uuid(),
   get_common_params()->pos().x(),
   get_common_params()->pos().y(),
   get_common_params()->pos().z()) + "{\n";*/

  for (auto chan : channels_) {
    description += "  C) " + chan->get_description() + "\n";
  }
  for (auto synapse : synapses_) {
    description += "  S -> " + synapse->get_description() + "\n";
  }

  description += "}";

  return description;
}

const std::vector<Synapse*>& Neuron::get_synapses() {
  return synapses_;
}

const std::vector<Channel*>& Neuron::get_channels() {
  return channels_;
}

}  // namespace bigbrain
