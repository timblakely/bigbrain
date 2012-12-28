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

#ifndef NETWORK_OBJECTS_CHANNEL_H_
#define NETWORK_OBJECTS_CHANNEL_H_

#include "network/objects/network_unit.h"

namespace bigbrain {

class Neuron;

class Channel : public NetworkUnit {
 public:
  Channel();
  virtual ~Channel();

  void set_parent(Neuron* parent_cell);
  Neuron* get_parent();
  virtual double get_current() = 0;

 private:
  Neuron* parent_cell_;
};

}  // namespace bigbrain
#endif  // NETWORK_OBJECTS_CHANNEL_H_
