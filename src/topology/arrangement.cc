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

#include <boost/random/detail/seed.hpp>
#include <boost/random/taus88.hpp>
#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <ostream>

#include "base_types.h"
#include "boost/random/uniform_int_distribution.hpp"
#include "network/bio_factory.h"
#include "network/objects/channel.h"
#include "network/objects/neuron.h"
#include "network/parameter_collection.h"
#include "protos/neuron.pb.h"
#include "protos/topology.pb.h"
#include "topology/arrangement.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

using boost::random::uniform_int_distribution;
using std::string;
using std::unordered_map;
using std::vector;

Arrangement::Arrangement(const int32 &random_seed) {
  random_.seed(random_seed);
}

void Arrangement::set_name(const string& name) {
  name_ = name;
}

void Arrangement::AddNeuronType(const Neuron_Layout& neuron_layout) {
  printf("[ARRANGEMENT] Informed of neuron layout \"%s\"\n",
         neuron_layout.name().c_str());
  neuron_types_.push_back(&neuron_layout);
}

bool Arrangement::AllocateNeurons(
    ParameterCollection* param_collection,
    unordered_map<Neuron*, vector<const Connection_Layout*>>& neuron_connection_map) {
  printf("[ARRANGEMENT] Allocating neurons for arrangement %s\n",
         name_.c_str());

  Neuron* neuron = NULL;

  for (auto neuron_layout : neuron_types_) {
    if (neuron_group_map_[neuron_layout->name()].size() != 0) {
      printf("ERROR: Duplicate neuron layout \"%s\", skipping\n",
             neuron_layout->name().c_str());
      continue;
    }

    printf("INFO: Getting parameter set for neuron %s\n",
           neuron_layout->name().c_str());

    google::protobuf::Message* neuron_param_set = param_collection->Insert(
        BioFactory::InstantiateNeuronParamsFromLayout(*neuron_layout));

    printf("INFO: Allocating %i neurons\n", neuron_layout->num_neurons());

    uniform_int_distribution<> rand_int;

    for (int i = 0; i < neuron_layout->num_neurons(); ++i) {
      neuron = BioFactory::InstantiateNeuronFromLayout(*neuron_layout,
                                                       rand_int(random_));

      neuron->set_params(neuron_param_set);

      // If the common params don't have a name specified, use the one from the
      // arrangement
      if (!neuron->get_common_params()->has_name()) {
        neuron->get_common_params()->set_name(neuron_layout->name());
      }

      // Place it in the world
      SetNeuronPosition(neuron, neuron_layout->num_neurons());

      all_neurons_.push_back(neuron);
      neuron_group_map_[neuron_layout->name()].push_back(neuron);

      // Allocate the channels

      Channel *chan = NULL;
      for (int j = 0; j < neuron_layout->channel_size(); ++j) {
        google::protobuf::Message* chan_param_set = param_collection->Insert(
            BioFactory::InstantiateChannelParamsFromLayout(
                neuron_layout->channel(j)));

        chan = BioFactory::InstantiateChannelFromLayout(
            neuron_layout->channel(j), rand_int(random_));

        chan->set_params(chan_param_set);
        neuron->AddChannel(chan);
      }

      for (int j = 0; j < neuron_layout->connection_size(); ++j) {
        const Connection_Layout* l = &(neuron_layout->connection(j));
        neuron_connection_map[neuron].push_back(l);
      }
    }
  }
  return true;
}

string Arrangement::get_name() {
  return name_;
}

const vector<Neuron*> Arrangement::get_instantiated_neurons() {
  return all_neurons_;
}

}  // namespace bigbrain
