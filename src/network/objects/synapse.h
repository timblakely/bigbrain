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

#ifndef NETWORK_OBJECTS_SYNAPSE_H_
#define NETWORK_OBJECTS_SYNAPSE_H_

#include "boost/circular_buffer.hpp"
#include "boost/circular_buffer/base.hpp"
#include "boost/circular_buffer/space_optimized.hpp"
#include "network/objects/network_unit.h"

namespace bigbrain {

class Neuron;

class Synapse : public NetworkUnit {
 public:
  Synapse();
  virtual ~Synapse();

  void SetLinkedNeurons(Neuron* parent, Neuron* target);
  virtual bool UpdateConnection(const float& time, const float& dt) = 0;
  virtual double get_current() = 0;

  // This is used by the delayed input.  This should be refactored when the move
  // away from protocol buffers occurs
  virtual void add_delayed_input(const double& value) = 0;

  bool AccumulateDelayedInput(const double& time_now);
  bool DelayInput(const double& t, const double& value);

  Neuron* get_parent();
  Neuron* get_target();

 protected:
  Neuron* parent_;
  Neuron* target_;

 private:
  // Note: this is something unique to Synapses.  Must be serialized directly!
  // This is a quick implementation of a growing ring buffer
  struct Delayed {
    double t_;
    double value_;
  };
  boost::circular_buffer_space_optimized<Delayed> delayed_inputs_;
  DISALLOW_COPY_AND_ASSIGN(Synapse);
};

}  // namespace bigbrain
#endif  // NETWORK_OBJECTS_SYNAPSE_H_
