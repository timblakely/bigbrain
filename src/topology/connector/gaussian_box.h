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

#ifndef TOPOLOGY_CONNECTOR_GAUSSIAN_BOX_H_
#define TOPOLOGY_CONNECTOR_GAUSSIAN_BOX_H_

#include <vector>

#include "base_types.h"
#include "protos/topology.pb.h"
#include "topology/connector.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

class Neuron;

class GaussianBoxConnector : public Connector {
 public:
  GaussianBoxConnector();
  virtual ~GaussianBoxConnector();

  virtual void MakeConnections(Neuron* source,
                               std::vector<Neuron*>& potential_targets);
  virtual google::protobuf::Message* get_params();

 private:
  GaussianBoxConnectorParams params_;
  DISALLOW_COPY_AND_ASSIGN(GaussianBoxConnector);
};

}  // namespace bigbrain
#endif  // TOPOLOGY_CONNECTOR_GAUSSIAN_BOX_H_
