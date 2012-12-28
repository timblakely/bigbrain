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

#ifndef TOPOLOGY_ARRANGEMENT_H_
#define TOPOLOGY_ARRANGEMENT_H_

#include <unordered_map>
#include <string>
#include <vector>

#include "base_types.h"
#include "protos/topology.pb.h"
#include "google/protobuf/message.h"
#include "boost/random/taus88.hpp"

namespace google {
namespace protobuf {
class Message;
}
}

namespace bigbrain {

class ParameterCollection;
class Neuron;

class Arrangement {
 public:
  explicit Arrangement(const int32 &random_seed);
  virtual ~Arrangement() {
  }

  void AddNeuronType(const Neuron_Layout& neuron_layout);
  bool AllocateNeurons(
      ParameterCollection* param_collection,
      std::unordered_map<Neuron*, std::vector<const Connection_Layout*>>& neuron_connection_map);

  void set_name(const std::string& name);
  std::string get_name();
  virtual google::protobuf::Message* get_params() = 0;
  const std::vector<Neuron*> get_instantiated_neurons();

 protected:
  virtual void SetNeuronPosition(Neuron* cell,
                                 const int32& num_this_type_neurons) = 0;

  boost::random::taus88 random_;
  std::string name_;

 private:
  std::vector<Neuron*> all_neurons_;
  std::unordered_map<std::string, std::vector<Neuron*>> neuron_group_map_;

  std::vector<const Neuron_Layout*> neuron_types_;
  DISALLOW_COPY_AND_ASSIGN(Arrangement);
};

}  // namespace bigbrain

#endif  // TOPOLOGY_ARRANGEMENT_H_
