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

#ifndef NETWORK_NETWORK_H_
#define NETWORK_NETWORK_H_

#include <unordered_map>

#include <string>
#include <vector>

#include "base_types.h"
#include "boost/random/taus88.hpp"
#include "network/parameter_collection.h"
#include "protos/basics.pb.h"

namespace bigbrain {

class Neuron;
class Topology;

// This is the core class of the simulator. A network is a collection of nodes
// and interconnects between them. These nodes can be neurons, channels, or
// intrinsics, but all are of type NetworkUnit
//
// Generally one network object is instantiated and then updated. Generation of
// a new network wipes out any previous network that was controlled by this
// object. As of 5-16-12, the entire Network object is closed and does not
// expose any of its internal structure and is completely encapsulated, instead
// manipulated from the outside by its member functions

class Network {
 public:
  Network();
  virtual ~Network();

  // Generates a network of neurons
  void GenerateFromFile(const std::string& filename);
  void GenerateFromString(const std::string& description);

  // Accessor to get the number of neurons in the network
  int GetNumberOfCells();
  const std::vector<Neuron*>& GetNeurons();
  Neuron* get_neuron(const uint64& uuid);

  // Run a timestep, incrementing by dt
  bool UpdateIntrinsicChannels(const float& dt);
  bool UpdateSynapticChannels(const float& dt);
  bool UpdateCells(const float& dt);

  bool UpdateTimestep(const float& dt);

 private:
  void CollapseNetwork();

  std::vector<Neuron*> cells_;

  Topology* topology_;

  std::unordered_map<uint64, Neuron*> id_map_;
  ParameterCollection parameter_collection_;

  NetworkInfo network_state_;

  boost::random::taus88 random_;

  DISALLOW_COPY_AND_ASSIGN(Network);
};

}  // namespace bigbrain
#endif  // NETWORK_NETWORK_H_
