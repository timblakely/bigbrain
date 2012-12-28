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

#ifndef TOPOLOGY_CONNECTOR_H_
#define TOPOLOGY_CONNECTOR_H_

#include <vector>

#include "base_types.h"
#include "boost/random/taus88.hpp"
#include "protos/topology.pb.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

class Neuron;
class ParameterCollection;
class Connection_Layout;

class Connector {
 public:
  Connector();
  virtual ~Connector();

  virtual void MakeConnections(Neuron* source,
                               std::vector<Neuron*>& potential_targets) = 0;
  virtual google::protobuf::Message* get_params() = 0;

  void set_layout(const Connection_Layout* layout);
  void set_random_seed(const int& rand_seed);
  void set_allow_self_connections(const bool& allow_self_connections);
  void set_parameter_collection(ParameterCollection* parameter_collection);
 protected:
  void AddLink(Neuron* souce, Neuron* dest);
  bool allow_self_connections_;
  boost::random::taus88 random_;
 private:
  const Connection_Layout* layout_;
  ParameterCollection* parameter_collection_;
  google::protobuf::Message* params_;
  DISALLOW_COPY_AND_ASSIGN(Connector);
};

}  // namespace bigbrain
#endif  // TOPOLOGY_CONNECTOR_H_
