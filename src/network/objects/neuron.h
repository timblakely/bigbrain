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

#ifndef NETWORK_OBJECTS_NEURON_H_
#define NETWORK_OBJECTS_NEURON_H_

#include <string>
#include <vector>

#include "network/objects/network_unit.h"

namespace bigbrain {

class Channel;
class Synapse;

class Neuron : public NetworkUnit {
 public:
  Neuron();
  virtual ~Neuron();

  // Add and *take ownership* of the channel. MUST FREE MEMORY in deconstructor!
  void AddChannel(Channel* channel);
  void AddSynapse(Synapse* synapse);
  void AddInputSynapse(Synapse* synapse);
  double SummedChannelCurrents();

  virtual bool RequiresUpdatingSynapses() = 0;
  virtual double get_membrane_potential() = 0;
  virtual std::string get_description();

  const std::vector<Synapse*>& get_synapses();
  const std::vector<Channel*>& get_channels();

 protected:
  std::vector<Channel*> channels_;
  std::vector<Synapse*> synapses_;
  std::vector<Synapse*> input_synapses_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Neuron);
};

}  // namespace bigbrain
#endif  // NETWORK_OBJECTS_NEURON_H_
