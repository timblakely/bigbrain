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

#include "topology/connector.h"

#include <bits/stl_algobase.h>
#include <boost/random/detail/seed.hpp>
#include <boost/random/taus88.hpp>
#include <stddef.h>

#include "boost/random/uniform_int_distribution.hpp"
#include "network/bio_factory.h"
#include "network/objects/neuron.h"
#include "network/objects/synapse.h"
#include "network/parameter_collection.h"

namespace bigbrain {

class Connection_Layout;

using boost::random::uniform_int_distribution;

Connector::Connector()
    : allow_self_connections_(false),
      layout_(NULL),
      params_(NULL),
      parameter_collection_(NULL) {
  random_.seed(0);
}

Connector::~Connector() {
}

void Connector::set_layout(const Connection_Layout* layout) {
  layout_ = layout;
  params_ = parameter_collection_->Insert(
      BioFactory::InstantiateSynapseParamsFromLayout(*layout_));
}

void Connector::set_random_seed(const int& rand_seed) {
  random_.seed(rand_seed);
}

void Connector::set_parameter_collection(
    ParameterCollection* parameter_collection) {
  parameter_collection_ = parameter_collection;
}

void Connector::AddLink(Neuron* source, Neuron* dest) {
  // We're sure we want to create a link between these neurons

  uniform_int_distribution<> range;
  Synapse* synapse = BioFactory::InstantiateSynapseFromLayout(*layout_,
                                                              range(random_));

  synapse->set_params(params_);
  synapse->SetLinkedNeurons(source, dest);

  source->AddSynapse(synapse);
}

void Connector::set_allow_self_connections(const bool& allow_self_connections) {
  allow_self_connections_ = allow_self_connections;
}

}  // namespace bigbrain
