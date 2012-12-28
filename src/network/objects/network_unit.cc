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
#include "network/objects/network_unit.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <stddef.h>
#include <stdio.h>
#include <iosfwd>
#include <iostream>
#include <ostream>
#include <string>

#include "protos/basics.pb.h"
#include "protos/neuron.pb.h"

namespace bigbrain {

using std::string;

std::unordered_map<const google::protobuf::Descriptor*,
    google::protobuf::Message**> NetworkUnit::solver_memory_map_;

NetworkUnit::NetworkUnit()
    : rand_num_gen_(0),
      type_(NU_TYPE_UNINITALIZED) {
  common_params_.set_name("UNKNOWN");
  common_params_.set_uuid(0);
  common_params_.set_diff_eq_solver(RUNGEKUTTA);
  common_params_.set_random_seed(0);
}

NetworkUnit::~NetworkUnit() {
}

void NetworkUnit::FreeSolverMemory() {
  printf("[NU-FSM] Freeing solver memory\n");
  for (auto pair : solver_memory_map_) {
    printf("[NU-FSM] Freeing %s\n", pair.first->name().c_str());
    for (int i = 0; i < 5; ++i) {
      delete pair.second[i];
    }
    delete[] pair.second;
  }
  solver_memory_map_.clear();
}

bool NetworkUnit::Update(const float& time, const float& dt) {
  const CommonParams *params = get_common_params();
  if (params == NULL) {
    printf("Common Params is null! Problematic thing:%s\n",
           get_description().c_str());
    return false;
  }

  bool status = true;

  // First, evaluate the differential equations

  switch (params->diff_eq_solver()) {
    case RUNGEKUTTA:
      status = SolveViaRungeKutta(time, dt);
      break;
    case RUNGEKUTTA_LEGACY:
      status = SolveViaLegacyRungeKutta(time, dt);
      break;
    case NONE:
      status = true;
      break;
  }

  if (status == false) {
    printf("Error solving differential equations!\n");
    return status;
  }

  // Next, update the state
  status = UpdateState(time, dt);

  return status;
}

// Should be in Neuron

bool NetworkUnit::SolveViaRungeKutta(const float& time, const float& dt) {
  // This could (should?) be instantiated once during malloc of Neuron. Would
  // increase the memory
  google::protobuf::Message *initial_state = get_state();
  google::protobuf::Message **rungekutta_derivatives = get_solver_memory(
      get_state());
  google::protobuf::Message *temp_state = rungekutta_derivatives[4];

  // Init all 5 state memory locations (including temp_state)
  for (int i = 0; i < 5; i++) {
    rungekutta_derivatives[i]->CopyFrom(*initial_state);
  }

  float half_dt = dt / 2.0f;
  float sixth_dt = dt / 6.0f;

  const google::protobuf::Descriptor *state_description = initial_state
      ->GetDescriptor();
  const google::protobuf::Reflection *state_reflection = initial_state
      ->GetReflection();

  //
  // ---- First RK step ----
  //
  CalculateDerivatives(time, initial_state, (rungekutta_derivatives[0]));

  //
  // ---- Second RK step ----
  //

  // Make sure we get all the values copied first, then change the fields we
  // need to update. This performs a double-copy in some instances, but is
  // necessary to be general enough for reflection to work properly
  temp_state->CopyFrom(*rungekutta_derivatives[0]);

  for (int i = 1; i < state_description->field_count(); i++) {
    const google::protobuf::FieldDescriptor *field = state_description->field(
        i);
    // only operate on doubles
    if (field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE) {
      double deriv = state_reflection->GetDouble(*rungekutta_derivatives[0],
                                                 field);
      double current_state_value = state_reflection->GetDouble(*initial_state,
                                                               field);
      double temp_state_value = current_state_value + half_dt * deriv;

      // Set the field
      state_reflection->SetDouble(temp_state, field, temp_state_value);
    }
  }
  CalculateDerivatives(time + half_dt, temp_state, (rungekutta_derivatives[1]));

  //
  // ---- Third RK step ----
  //
  temp_state->CopyFrom(*rungekutta_derivatives[1]);

  for (int i = 1; i < state_description->field_count(); i++) {
    const google::protobuf::FieldDescriptor *field = state_description->field(
        i);
    // only operate on doubles
    if (field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE) {
      double deriv = state_reflection->GetDouble(*rungekutta_derivatives[1],
                                                 field);
      double current_state_value = state_reflection->GetDouble(*initial_state,
                                                               field);
      double temp_state_value = current_state_value + half_dt * deriv;

      // Set the field
      state_reflection->SetDouble(temp_state, field, temp_state_value);
    }
  }
  CalculateDerivatives(time + half_dt, temp_state, (rungekutta_derivatives[2]));

  //
  // ---- Fourth RK step ----
  //
  temp_state->CopyFrom(*rungekutta_derivatives[2]);

  for (int i = 1; i < state_description->field_count(); i++) {
    const google::protobuf::FieldDescriptor *field = state_description->field(
        i);
    // only operate on doubles
    if (field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE) {
      double deriv = state_reflection->GetDouble(*rungekutta_derivatives[2],
                                                 field);
      double current_state_value = state_reflection->GetDouble(*initial_state,
                                                               field);
      double temp_state_value = current_state_value + dt * deriv;

      // Set the field
      state_reflection->SetDouble(temp_state, field, temp_state_value);
    }
  }
  CalculateDerivatives(time + dt, temp_state, (rungekutta_derivatives[3]));

  //
  // ---- Final calculation ----
  //

  for (int i = 1; i < state_description->field_count(); i++) {
    const google::protobuf::FieldDescriptor *field = state_description->field(
        i);
    // only operate on doubles
    if (field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE) {
      double state_value = state_reflection->GetDouble(*initial_state, field);
      double a = state_reflection->GetDouble(*rungekutta_derivatives[0], field);
      double b = state_reflection->GetDouble(*rungekutta_derivatives[1], field);
      double c = state_reflection->GetDouble(*rungekutta_derivatives[2], field);
      double d = state_reflection->GetDouble(*rungekutta_derivatives[3], field);
      double temp_state_value = state_value
          + sixth_dt * (a + 2 * b + 2 * c + d);

      // Set the field
      state_reflection->SetDouble(temp_state, field, temp_state_value);
    }
  }

  // finally, update the Neuron's internal state

  initial_state->CopyFrom(*temp_state);

  return true;
}

void NetworkUnit::set_id(uint64 id) {
  get_common_params()->set_uuid(id);
}

const uint64 NetworkUnit::get_id() {
  return get_common_params()->uuid();
}

const string& NetworkUnit::get_name() {
  return get_common_params()->name();
}

void NetworkUnit::set_name(const string & name) {
  get_common_params()->set_name(name);
}

void NetworkUnit::set_random_seed(const uint32 &seed) {
  get_common_params()->set_random_seed(seed);
}

const NetworkUnitType &NetworkUnit::get_type() {
  return type_;
}

CommonParams *NetworkUnit::get_common_params() {
  return &common_params_;
}

bool NetworkUnit::SolveViaLegacyRungeKutta(const float& time, const float& dt) {
  // This is a stub function that would implement the approximate RK solver
  // that was used by Synthesis.  It's very close, but not precisely the same

  printf("WARNING: LEGACY NOT IMPLEMENTED YET! SOLVING VIA NEW METHOD\n");
  return SolveViaRungeKutta(time, dt);
}

google::protobuf::Message** NetworkUnit::get_solver_memory(
    const google::protobuf::Message* proto) {
  google::protobuf::Message** memory =
      solver_memory_map_[proto->GetDescriptor()];

  if (memory) {
    return memory;
  }

  // Haven't seen this type before
  printf("[NU-GETSOLVMEM] Haven't seen %s before, allocating and zeroing\n",
         proto->GetDescriptor()->name().c_str());

  memory = new google::protobuf::Message*[5];
  for (int i = 0; i < 5; ++i) {
    memory[i] = proto->New();
  }

  printf("[NU-GETSOLVMEM] Placing memory in map\n");

  solver_memory_map_[proto->GetDescriptor()] = memory;

  return memory;
}

}  // namespace bigbrain
