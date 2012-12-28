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

#ifndef TOPOLOGY_TOPOLOGY_H_
#define TOPOLOGY_TOPOLOGY_H_

#include <unordered_map>
#include <string>
#include <vector>

#include "base_types.h"
#include "boost/random/taus88.hpp"
#include "protos/topology.pb.h"

namespace bigbrain {

class Arrangement;
class Connector;
class Neuron;
class ParameterCollection;

class Topology {
 public:
  // NOTE: param_collection must outlive Topology! Does *not* take ownership of
  // the ParameterCollection!
  explicit Topology(ParameterCollection* param_collection);
  virtual ~Topology();

  bool CreateFromString(const std::string& description);
  bool AllocateNeurons();
  bool MakeConnections();

  void Collapse();

  std::vector<Neuron*> GetNeurons();

 private:
  // Load the arrangements form the layout
  bool LoadArrangementLayouts();
  // Inform arrangement about neuron types
  bool AddNeuronLayoutsToArrangement(
      Arrangement* arrangement, const Arrangement_Layout& arrangement_layout);

  Connector* ConnectorForLayout(const Connection_Layout* layout);
  std::vector<Neuron*> TargetsForLayout(const Connection_Layout* layout);

  // This is a reference we take in from the network. It's function is to make
  // sure we don't allocate more than one type of parameter set if they're the
  // same.  Do not free
  ParameterCollection* param_collection_;

  // Because we have the entire layout allocated, we don't need to worry about
  // any of the *_Layouts being deallocated until this is freed. At which point,
  // they won't be needed anymore
  Layout layout_;

  // We'll be allocating these internally, so we need to free them when we're
  // deallocated
  std::vector<Arrangement*> arrangements_;
  std::vector<Connector*> all_connectors_;

  // Ownership of these will be passed to the Network, so DONT free them
  std::vector<Neuron*> all_neurons_;

  // These are simply helper maps. The memory they point to is stored other
  // places
  std::unordered_map<std::string, Arrangement*> arrangement_map_;
  std::unordered_map<Neuron*, std::vector<const Connection_Layout*>> neuron_connection_map_;
  std::unordered_map<const Connection_Layout*, Connector*> connector_map_;
  std::unordered_map<std::string, std::vector<Neuron*> > neuron_name_map_;

  boost::random::taus88 random_;

  DISALLOW_COPY_AND_ASSIGN(Topology);
};

}  // namespace bigbrain
#endif  // TOPOLOGY_TOPOLOGY_H_
