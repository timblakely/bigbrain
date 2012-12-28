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

// This file is unique, in that it must grow every time we add a new channel,
// synapse, or cell type. This is the unfortunate reality of statically linking:
// we can't simply dynamically link in NetworkUnit plugins, and as such, must
// implement them as a case-by-case type somewhere. A factory is the best way to
// do this
//
// When you create a new class, you must be sure to update the following
//  functions:
//
//    InstantiateFromStoredNetworkUnit
#include "network/bio_factory.h"

#include <bits/basic_string.h>
#include <bits/char_traits.h>
#include <bits/ios_base.h>
#include <google/protobuf/message.h>
#include <stddef.h>
#include <stdio.h>
#include <iosfwd>
#include <iostream>
#include <ostream>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/text_format.h"
#include "network/objects/cells/pulse_integrate_and_fire.h"
#include "network/objects/channel.h"
#include "network/objects/channels/i_h.h"
#include "network/objects/channels/i_k_na.h"
#include "network/objects/channels/i_na_p.h"
#include "network/objects/connections/ampa.h"
#include "network/objects/network_unit.h"
#include "network/objects/neuron.h"
#include "network/objects/synapse.h"
#include "protos/neuron.pb.h"
#include "protos/stored_network_unit.pb.h"
// Topologies
#include "protos/topology.pb.h"
#include "topology/connector.h"
#include "topology/connector/gaussian_box.h"

namespace bigbrain {

uint64 BioFactory::next_uuid_ = 0;  // But first UUID will be 1

BioFactory::BioFactory() {
}

BioFactory::~BioFactory() {
}

NetworkUnit* BioFactory::InstantiateNetworkUnitFromStoredNetworkUnit(
    const StoredNetworkUnit &stored_network_unit) {

  printf("BROKEN AT THE MOMENT!!!!\n");
  return NULL;

  NetworkUnit* new_unit = NULL;

  if (stored_network_unit.has_ikna_state()) {
    //
    // IKNa Channel
    //
    printf("[BF-NETWORKUNIT] Extracting IKNA state\n");
    new_unit = new IKNaChannel();
  } else if (stored_network_unit.has_piaf_state()) {
    //
    // PulseIntegrateAndFire neuron
    //
    new_unit = new PulseIntegrateAndFire();
    printf("[BF-NETWORKUNIT] Extracting PIAF state");
  } else {
    printf("FATAL ERROR: BF-NETWORKUNIT] Unknown type stored in "
        "StoredNetworkUnit: %s\n",
        stored_network_unit.GetDescriptor()->name().c_str());
  }

  new_unit->get_common_params()->set_uuid(GetNewUUID());

  return new_unit;
}

// Sets up neuron's common parameters and state
Neuron* BioFactory::InstantiateNeuronFromLayout(
    const Neuron_Layout& neuron_layout, const int32& rand_seed) {

  Neuron* result = NULL;

  switch (neuron_layout.type()) {
    case N_TYPE_PIAF:
      result = new PulseIntegrateAndFire();
      break;
  }

  if (!::google::protobuf::TextFormat::ParseFromString(
      neuron_layout.common_params(), result->get_common_params())) {
    printf("ERROR: [BF-NEURON] Couldn't parse neuron's common parameters\n");
    delete result;
    return NULL;
  }

  if (!::google::protobuf::TextFormat::ParseFromString(neuron_layout.state(),
                                                       result->get_state())) {
    printf("ERROR: [BF-NEURON] Couldn't parse neuron's state\n");
    delete result;
    return NULL;
  }

  result->get_common_params()->set_random_seed(rand_seed);

  result->get_common_params()->set_uuid(GetNewUUID());

  return result;
}

// Sets up neuron's common parameters and state
Channel* BioFactory::InstantiateChannelFromLayout(
    const Channel_Layout& channel_layout, const int32& rand_seed) {

  Channel* result = NULL;
  switch (channel_layout.type()) {
    case CH_TYPE_IKNA:
      result = new IKNaChannel();
      break;
    case CH_TYPE_INAP:
      result = new INaPChannel();
      break;
      return NULL;
    case CH_TYPE_IH:
      result = new IhChannel();
      break;
      return NULL;
  }

  if (!::google::protobuf::TextFormat::ParseFromString(
      channel_layout.common_params(), result->get_common_params())) {
    printf("ERROR: BF-CHAN] Couldn't parse neuron's common parameters\n");
    delete result;
    return NULL;
  }

  if (!::google::protobuf::TextFormat::ParseFromString(channel_layout.state(),
                                                       result->get_state())) {
    printf("ERROR: BF-CHAN] Couldn't parse neuron's state");
    delete result;
    return NULL;
  }

  result->get_common_params()->set_random_seed(rand_seed);

  result->get_common_params()->set_uuid(GetNewUUID());

  return result;
}

google::protobuf::Message* BioFactory::InstantiateChannelParamsFromLayout(
    const Channel_Layout& channel_layout) {

  google::protobuf::Message* result = NULL;

  switch (channel_layout.type()) {
    case CH_TYPE_IKNA:
      result = new IKNaParams();
      break;
    case CH_TYPE_INAP:
      result = new INaPParams();
      break;
      return NULL;
    case CH_TYPE_IH:
      result = new IhParams();
      break;
      return NULL;
  }

  if (!::google::protobuf::TextFormat::ParseFromString(channel_layout.params(),
                                                       result)) {
    printf("ERROR: [BF-CHANPARAMS] Couldn't parse neuron's parameters\n");
    delete result;
    return NULL;
  }

  return result;
}

google::protobuf::Message* BioFactory::InstantiateNeuronParamsFromLayout(
    const Neuron_Layout& neuron_layout) {

  google::protobuf::Message* result = NULL;

  switch (neuron_layout.type()) {
    case N_TYPE_PIAF:
      result = new PulseIntegrateAndFireParams();
      break;
  }

  if (!::google::protobuf::TextFormat::ParseFromString(neuron_layout.params(),
                                                       result)) {
    printf("ERROR: BF-NEURONPARAMS] Couldn't parse neuron's parameters\n");
    delete result;
    return NULL;
  }

  return result;
}

google::protobuf::Message* BioFactory::InstantiateSynapseParamsFromLayout(
    const Connection_Layout& synapse_layout) {

  google::protobuf::Message* result = NULL;

  switch (synapse_layout.type()) {
    case SYN_TYPE_AMPA:
      result = new AmpaParams();
      break;
  }

  if (!::google::protobuf::TextFormat::ParseFromString(synapse_layout.params(),
                                                       result)) {
    printf("ERROR: BF-SyanpseParams] Couldn't parse neuron's parameters\n");
    delete result;
    return NULL;
  }

  return result;
}

Synapse* BioFactory::InstantiateSynapseFromLayout(
    const Connection_Layout& layout, const int32& random_seed) {
  Synapse *synapse;
  switch (layout.type()) {
    case SYN_TYPE_AMPA:
      synapse = new SynapseAmpa();
      synapse->get_common_params()->set_random_seed(random_seed);
      break;
  }

  synapse->get_common_params()->set_uuid(GetNewUUID());
  return synapse;
}

Connector* BioFactory::InstantiateConnectorFromLayout(
    const Connection_Layout& layout, const int32& random_seed) {
  Connector* connector = NULL;
  switch (layout.target().type()) {
    case TARGET_DIST_TYPE_GAUSSIAN_BOX:
      connector = new GaussianBoxConnector();
      break;
  }

  if (layout.target().has_params()) {
    google::protobuf::TextFormat::ParseFromString(layout.target().params(),
                                                  connector->get_params());
  }

  if (layout.has_allow_self_connection()) {
    connector->set_allow_self_connections(layout.allow_self_connection());
  }

  connector->set_random_seed(random_seed);

  return connector;
}

uint64 BioFactory::GetNewUUID() {
  ++next_uuid_;
  return next_uuid_;
}

void BioFactory::ResetUUIDs() {
  next_uuid_ = 0;
}

}  // namespace bigbrain
