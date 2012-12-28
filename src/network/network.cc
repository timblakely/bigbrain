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

#include "network.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/random/detail/seed.hpp>
#include <boost/random/taus88.hpp>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef int32_t int32;

#include <fstream>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <ostream>

#include "boost/random/uniform_int_distribution.hpp"
#include "network/bio_factory.h"
#include "network/objects/cells/pulse_integrate_and_fire.h"
#include "network/objects/network_unit.h"
#include "network/objects/synapse.h"
#include "objects/channel.h"
#include "objects/neuron.h"
#include "parameter_collection.h"
#include "protos/neuron.pb.h"
#include "topology/topology.h"

namespace bigbrain {

using std::string;
using std::vector;
using std::ifstream;

using boost::random::uniform_int_distribution;

int kNumSynapses = 5;

Network::Network()
    : topology_(NULL) {
  network_state_.set_time(0);
  network_state_.set_next_uuid(0);
  random_.seed();
}

Network::~Network() {
  CollapseNetwork();
}

void Network::GenerateFromFile(const string &filename) {
  printf("Generating from file name: %s\n", filename.c_str());

  ifstream file(filename);

  if (file.fail()) {
    printf("Unable to open file %s\n", filename.c_str());
    return;
  }

  file.seekg(0, std::ios::end);
  string layout_as_string;
  layout_as_string.reserve(file.tellg());
  file.seekg(0, std::ios::beg);

  layout_as_string.assign((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

  printf("Read in file of length %lu\n", layout_as_string.length());

  GenerateFromString(layout_as_string);
}

void Network::GenerateFromString(const string &description) {
  printf("[NETWORK] Generating from string:\n%s\n", description.c_str());

  // If there was a network, collapse it
  if (cells_.size() > 0) {
    printf("[NETWORK] Previous network detected, collapsing...");
    CollapseNetwork();
  }

  string description_sans_whitespace;

  // Clean multi-line whitespace

  description_sans_whitespace = description;
  boost::replace_all(description_sans_whitespace, "\n", " ");

  if (topology_) {
    delete topology_;
    topology_ = NULL;
  }

  topology_ = new Topology(&parameter_collection_);

  // Inform topology of how it should look
  printf("Creating topology structure\n");
  topology_->CreateFromString(description_sans_whitespace);

  // Now that we know what it should look like, allocate the neurons
  printf("Allocating neurons\n");
  topology_->AllocateNeurons();

  // Wire up the connections
  printf("Wiring connections\n");
  topology_->MakeConnections();

  printf("Retrieving neurons from topology\n");
  cells_ = topology_->GetNeurons();

  printf("Collapsing topology generator\n");
  topology_->Collapse();

  printf("Creating ID map\n");
  for (auto cell : cells_) {
    id_map_[cell->get_common_params()->uuid()] = cell;
  }

  delete topology_;
  topology_ = NULL;
}

// Destroys the network, freeing all the associated memory

void Network::CollapseNetwork() {
  if (cells_.size() > 0) {
    for (NetworkUnit * cell : cells_) {
      delete cell;
    }
    cells_.clear();
  }

  id_map_.clear();

  parameter_collection_.Collapse();

  network_state_.set_next_uuid(0);
  network_state_.set_time(0);

  // Need to clean the memory for the solvers
  NetworkUnit::FreeSolverMemory();

  if (topology_) {
    delete topology_;
    topology_ = NULL;
  }

  // Don't forget to reset UUIDs
  BioFactory::ResetUUIDs();

  id_map_.clear();
}

int Network::GetNumberOfCells() {
  return cells_.size();
}

const vector<Neuron*>& Network::GetNeurons() {
  return cells_;
}

Neuron* Network::get_neuron(const uint64& uuid) {
  return id_map_[uuid];
}

bool Network::UpdateTimestep(const float& dt) {
  UpdateIntrinsicChannels(dt);
  UpdateSynapticChannels(dt);
  UpdateCells(dt);

  network_state_.set_time(network_state_.time() + dt);
  return true;
}

bool Network::UpdateSynapticChannels(const float& dt) {
  for (Neuron* object : cells_) {
    for (auto synapse : object->get_synapses()) {
      synapse->Update(network_state_.time(), dt);
    }
  }
  return false;
}

bool Network::UpdateIntrinsicChannels(const float& dt) {
  for (Neuron* object : cells_) {
    for (auto synapse : object->get_channels()) {
      synapse->Update(network_state_.time(), dt);
    }
  }
  return true;
}

bool Network::UpdateCells(const float& dt) {
  for (Neuron* neuron : cells_) {
    neuron->Update(network_state_.time(), dt);
    if (neuron->RequiresUpdatingSynapses()) {
      for (auto synapse : neuron->get_synapses()) {
        synapse->UpdateConnection(network_state_.time(), dt);
      }
    }
  }

  return true;
}

}  // namespace bigbrain
