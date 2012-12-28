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

#include <Python.h>  // Out of order, but necessary to remove warnings

#include <google/protobuf/descriptor.h>
#include <stddef.h>
#include <stdio.h>

#include "controller/bigbrain_api.h"
#include "controller/simulator.h"
#include "google/protobuf/message.h"
#include "network/network.h"
#include "network/objects/network_unit.h"
#include "network/objects/neuron.h"
#include "network/objects/synapse.h"
#include "network/parameter_collection.h"
#include "protos/basics.pb.h"
#include "protos/neuron.pb.h"

namespace bigbrain {

using std::string;
using std::vector;

Simulator::Simulator() {
  description_ = "";
}

Simulator::~Simulator() {
}

void Simulator::GenerateNetwork(std::string description) {
  description_ = description;
  network_.GenerateFromString(description_);
}

void Simulator::Beep(int test) {
  printf("BOOP\n");
}

void Simulator::RunTimestep(const double& t, const double& dt) {
  network_.UpdateTimestep(dt);
  BigBrain::Update();
}

void Simulator::RunUntil(const double& t, const double& dt) {
  for (float i = 0; i < t; i += dt) {
    network_.UpdateTimestep(dt);
    BigBrain::Update();
  }
}

vector<uint64> Simulator::GetNeuronUUIDs() {
  vector<uint64> uuids;
  for (auto neuron : network_.GetNeurons()) {
    uuids.push_back(neuron->get_id());
  }
  return uuids;
}

vector<double> Simulator::GetMembranePotentials() {
  vector<double> v;
  for (auto neuron : network_.GetNeurons()) {
    v.push_back(neuron->get_membrane_potential());
  }
  return v;
}

vector<double> Simulator::GetCellPosition(const uint64& uuid) {
  vector<double> pos;
  Neuron* neuron = network_.get_neuron(uuid);
  if (neuron == NULL) {
    return pos;
  }
  Position p = neuron->get_common_params()->pos();
  pos.push_back(p.x());
  pos.push_back(p.y());
  pos.push_back(p.z());
  return pos;
}

vector<uint64> Simulator::GetCellConnections(const uint64& uuid) {
  vector<uint64> uuids;

  Neuron* neuron = network_.get_neuron(uuid);
  if (neuron == NULL) {
    return uuids;
  }

  for (auto synapse : neuron->get_synapses()) {
    uuids.push_back(synapse->get_target()->get_id());
  }

  return uuids;
}

vector<string> Simulator::GetCellParamNames(const uint64& uuid) {
  Neuron* neuron = network_.get_neuron(uuid);
  if (neuron == NULL) {
    return vector<string>();
  }
  vector<string> params = GetProtoFieldNames(neuron->get_common_params());
  vector<string> temp = GetProtoFieldNames(neuron->get_unique_params());
  params.insert(params.end(), temp.begin(), temp.end());
  return params;
}

vector<string> Simulator::GetCellParamValues(const uint64& uuid) {
  Neuron* neuron = network_.get_neuron(uuid);
  if (neuron == NULL) {
    return vector<string>();
  }
  vector<string> values = GetProtoFieldValues(neuron->get_common_params());
  vector<string> temp = GetProtoFieldValues(neuron->get_unique_params());
  values.insert(values.end(), temp.begin(), temp.end());
  return values;
}

vector<string> Simulator::GetCellStateNames(const uint64& uuid) {
  Neuron* neuron = network_.get_neuron(uuid);
  if (neuron == NULL) {
    return vector<string>();
  }
  return GetProtoFieldNames(neuron->get_state());
}

vector<string> Simulator::GetCellStateValues(const uint64& uuid) {
  Neuron* neuron = network_.get_neuron(uuid);
  if (neuron == NULL) {
    return vector<string>();
  }
  return GetProtoFieldValues(neuron->get_state());
}

// Helper functions

std::vector<std::string> Simulator::GetProtoFieldNames(
    google::protobuf::Message* msg) {

  vector<string> result;

  namespace pb = google::protobuf;

  char buf[2048];

  const pb::Descriptor* desc = msg->GetDescriptor();
  const pb::Reflection* ref = msg->GetReflection();

  for (int i = 0; i < desc->field_count(); i++) {
    const google::protobuf::FieldDescriptor* field = desc->field(i);
    result.push_back(field->name());
  }
  return result;
}

std::vector<std::string> Simulator::GetProtoFieldValues(
    google::protobuf::Message* msg) {

  vector<string> result;

  namespace pb = google::protobuf;

  char buf[2048];

  const pb::Descriptor* desc = msg->GetDescriptor();
  const pb::Reflection* ref = msg->GetReflection();

  for (int i = 0; i < desc->field_count(); i++) {
    const google::protobuf::FieldDescriptor* field = desc->field(i);
    switch (field->type()) {
      case pb::FieldDescriptor::TYPE_DOUBLE:
        snprintf(buf, sizeof(buf), "%f", ref->GetDouble(*msg, field));
        break;
      case pb::FieldDescriptor::TYPE_BOOL:
        snprintf(buf, sizeof(buf), "%s",
                 ref->GetBool(*msg, field) ? "TRUE" : "FALSE");
        break;
      case pb::FieldDescriptor::TYPE_STRING:
        snprintf(buf, sizeof(buf), "%s", ref->GetString(*msg, field).c_str());
        break;
      case pb::FieldDescriptor::TYPE_UINT64:
        snprintf(buf, sizeof(buf), "%lu", ref->GetUInt64(*msg, field));
        break;
      case pb::FieldDescriptor::TYPE_INT32:
        snprintf(buf, sizeof(buf), "%i", ref->GetInt32(*msg, field));
        break;
      case pb::FieldDescriptor::TYPE_UINT32:
        snprintf(buf, sizeof(buf), "%u", ref->GetUInt32(*msg, field));
        break;
      case pb::FieldDescriptor::TYPE_MESSAGE:
        snprintf(buf, sizeof(buf), "[Protobuf]");
        break;
      case pb::FieldDescriptor::TYPE_ENUM:
        const pb::EnumValueDescriptor* enum_desc = ref->GetEnum(*msg, field);
        snprintf(buf, sizeof(buf), "%s", enum_desc->full_name().c_str());
        break;
    }
    result.push_back(buf);
  }
  return result;
}

}  // namespace bigbrain
