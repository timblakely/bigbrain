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

#include "network/parameter_collection.h"

#include <google/protobuf/descriptor.h>
#include <iosfwd>
#include <iostream>
#include <ostream>

#include "google/protobuf/message.h"

namespace bigbrain {

ParameterCollection::ParameterCollection() {
}

ParameterCollection::~ParameterCollection() {
  for (auto paramset : param_list_) {
    delete paramset;
  }
}

google::protobuf::Message* ParameterCollection::Insert(
    google::protobuf::Message* input_param_set) {
  std::string input;
  std::string saved;

  std::string input_description =
      input_param_set->GetDescriptor()->DebugString();

  for (auto paramset : param_list_) {
    std::string paramset_description = paramset->GetDescriptor()->DebugString();

    if (paramset_description != input_description) {
      continue;
    }
    if (input_param_set->SerializeAsString() == paramset->SerializeAsString()) {
      delete input_param_set;
      return paramset;
    }
  }
  printf("[PC] Adding new parameter set\n");
  param_list_.push_back(input_param_set);
  return input_param_set;
}

void ParameterCollection::Collapse() {
  for (auto p : param_list_) {
    delete p;
  }
  param_list_.clear();
}

}  // namespace bigbrain
