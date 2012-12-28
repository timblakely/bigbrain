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

#include "topology/topology.h"

#include <boost/random/detail/seed.hpp>
#include <boost/random/taus88.hpp>
#include <stddef.h>
#include <stdio.h>
#include <iosfwd>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "boost/random/uniform_int_distribution.hpp"
#include "google/protobuf/text_format.h"
#include "network/bio_factory.h"
#include "network/objects/neuron.h"
#include "protos/neuron.pb.h"
#include "topology/arrangement.h"
#include "topology/arrangement/flat_rectangular.h"
#include "topology/connector.h"

namespace bigbrain {

using boost::random::uniform_int_distribution;
using std::string;
using std::vector;

Topology::Topology(ParameterCollection* param_collection) {
  random_.seed(0);
  param_collection_ = param_collection;
}

Topology::~Topology() {
  Collapse();
}

bool Topology::CreateFromString(const string& description) {
  printf("[TOPO] Creating topology from string\n");
  // Parse the topmost layout into PB
  if (!::google::protobuf::TextFormat::ParseFromString(description, &layout_)) {
    printf("ERROR: Could not parse top level layout. "
           "Please check network description.\n");
    return false;
  }

  printf("Creating topology \"%s\"\n", layout_.name().c_str());

  printf("Topology's random seed: %i\n", layout_.random_seed());

  random_.seed(layout_.random_seed());

  uniform_int_distribution<> dist;
  for (int i = 0; i < layout_.random_seed_cycles(); ++i) {
    dist(random_);
  }

  if (!LoadArrangementLayouts()) {
    printf("ERROR: Could not load arrangement layouts\n");
    return false;
  }

  return true;
}

bool Topology::LoadArrangementLayouts() {
  printf("[TOPO-LAL] Loading arrangements\n");

  // Load each individual arrangement
  for (int i = 0; i < layout_.arrangement_size(); ++i) {
    const Arrangement_Layout& arrangement_layout = layout_.arrangement(i);

    if (arrangement_map_[arrangement_layout.name()] != NULL) {
      printf("ERROR: Duplicate arrangement name: %s, skipping\n",
             layout_.name().c_str());
      continue;
    }

    Arrangement *arrangement = NULL;

    boost::random::uniform_int_distribution<> rand_int;

    switch (arrangement_layout.type()) {
      case ARR_TYPE_FLAT_RECTANGULAR:
        arrangement = new FlatRectangularArrangement(rand_int(random_));
        break;
      case ARR_TYPE_SPHERICAL:
        printf("ERROR: ARR_TYPE_SPHERICAL not yet implemented\n");
        return false;
    }

    arrangement->set_name(arrangement_layout.name());

    AddNeuronLayoutsToArrangement(arrangement, arrangement_layout);

    ::google::protobuf::TextFormat::ParseFromString(arrangement_layout.params(),
                                                    arrangement->get_params());

    // Store for connection linking
    arrangement_map_[arrangement_layout.name()] = arrangement;

    arrangements_.push_back(arrangement);
  }

  return true;
}

bool Topology::AddNeuronLayoutsToArrangement(
    Arrangement* arrangement, const Arrangement_Layout& arrangement_layout) {
  printf("[TOPO-ANLTA] Adding neuron layouts to arrangement\n");
  for (int i = 0; i < arrangement_layout.neuron_size(); ++i) {
    const Neuron_Layout& neuron_layout = arrangement_layout.neuron(i);
    arrangement->AddNeuronType(neuron_layout);
  }

  return true;
}

bool Topology::AllocateNeurons() {
  printf("[TOPO-AN] Allocating neurons\n");
  for (auto arrangement : arrangements_) {
    arrangement->AllocateNeurons(param_collection_, neuron_connection_map_);
    for (auto neuron : arrangement->get_instantiated_neurons()) {
      all_neurons_.push_back(neuron);
    }
  }

  printf("[TOPO-AN] Mapping neurons to name group\n");
  for (auto neuron : all_neurons_) {
    neuron_name_map_[neuron->get_common_params()->name()].push_back(neuron);
  }

  return true;
}

void Topology::Collapse() {
  printf("[TOPO] Collapsing...\n");
  arrangements_.clear();
  arrangement_map_.clear();
}

Connector* Topology::ConnectorForLayout(const Connection_Layout* layout) {
  Connector *connector = connector_map_[layout];
  if (connector) {
    return connector;
  }

  // Don't have a connector set up for this layout yet, so allocate one
  boost::random::uniform_int_distribution<> rand_int;
  connector = BioFactory::InstantiateConnectorFromLayout(*layout,
                                                         rand_int(random_));

  connector->set_parameter_collection(param_collection_);
  connector->set_layout(layout);

  // Add it to the map, and keep track of it so we can deallocate it later
  connector_map_[layout] = connector;
  all_connectors_.push_back(connector);

  return connector;
}

vector<Neuron*> Topology::TargetsForLayout(const Connection_Layout* layout) {
  vector<Neuron*> targets;

  if (!layout->target().has_target_arrangement_name()
      && !layout->target().has_target_neuron_name()) {
    printf("Warning: Potential to target EVERY neuron! Can be VERY slow\n");
    targets = all_neurons_;
    return targets;
  }

  if (!layout->target().has_target_arrangement_name()
      && layout->target().has_target_neuron_name()) {
    // No arrangement, but specific name
    targets = neuron_name_map_[layout->target().target_neuron_name()];
    if (targets.size() == 0) {
      printf("Warning: No neurons found in target %s\n",
             layout->target().target_neuron_name().c_str());
    }
    return targets;
  }

  // Guaranteed that we have arrangement name

  Arrangement *arrangement = arrangement_map_[layout->target()
      .target_arrangement_name()];
  if (arrangement == NULL) {
    printf("ERROR: Unknown arrangement %s\n",
           layout->target().target_arrangement_name().c_str());
    return targets;
  }
  targets = arrangement->get_instantiated_neurons();

  if (!layout->target().has_target_neuron_name()) {
    // Have arrangement and no specific name, so just return what we found in
    // the arrangements
    return targets;
  }

  // We have both arrangements (which we've found) and now need to only get the
  // ones that have the specific name
  vector<Neuron*> pruned_targets;
  for (auto neuron : targets) {
    if (layout->target().target_neuron_name()
        == neuron->get_common_params()->name()) {
      pruned_targets.push_back(neuron);
    }
  }

  return pruned_targets;
}

vector<Neuron*> Topology::GetNeurons() {
  return all_neurons_;
}

bool Topology::MakeConnections() {
  printf("[TOPO-AN] Making connections\n");

  for (auto neuron : all_neurons_) {
    vector<const Connection_Layout*> connection_layouts =
        neuron_connection_map_[neuron];
    for (auto layout : connection_layouts) {
      // Get the connector
      Connector* connector = ConnectorForLayout(layout);
      vector<Neuron*> targets = TargetsForLayout(layout);

      connector->MakeConnections(neuron, targets);
    }
  }

  return true;
}

}  // namespace bigbrain
