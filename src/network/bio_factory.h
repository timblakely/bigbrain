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

#ifndef NETWORK_BIO_FACTORY_H_
#define NETWORK_BIO_FACTORY_H_

#include "base_types.h"
#include "google/protobuf/message.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

class Arrangement;
class Channel;
class Channel_Layout;
class Connection_Layout;
class Connector;
class NetworkUnit;
class Neuron;
class Neuron_Layout;
class StoredNetworkUnit;
class Synapse;

class BioFactory {
 public:
  BioFactory();
  ~BioFactory();

  static NetworkUnit* InstantiateNetworkUnitFromStoredNetworkUnit(
      const StoredNetworkUnit& stored_network_unit);

  static Neuron* InstantiateNeuronFromLayout(const Neuron_Layout& neuron_layout,
                                             const int32& rand_seed);
  static google::protobuf::Message* InstantiateNeuronParamsFromLayout(
      const Neuron_Layout& neuron_layout);

  static Channel* InstantiateChannelFromLayout(
      const Channel_Layout& channel_layout, const int32& rand_seed);
  static google::protobuf::Message* InstantiateChannelParamsFromLayout(
      const Channel_Layout &channel_layout);

  static Synapse* InstantiateSynapseFromLayout(const Connection_Layout& layout,
                                               const int32& random_seed);
  static google::protobuf::Message* InstantiateSynapseParamsFromLayout(
      const Connection_Layout& synapse_layout);
  static Connector* InstantiateConnectorFromLayout(
      const Connection_Layout& layout, const int32& random_seed);
  static void ResetUUIDs();

  static uint64 GetNewUUID();

 private:
  static uint64 next_uuid_;
};

}  // namespace bigbrain
#endif  // NETWORK_BIO_FACTORY_H_
