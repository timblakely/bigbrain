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

#ifndef NETWORK_OBJECTS_CELLS_PULSE_INTEGRATE_AND_FIRE_H_
#define NETWORK_OBJECTS_CELLS_PULSE_INTEGRATE_AND_FIRE_H_

#include <string>

#include "network/objects/neuron.h"
#include "protos/neuron.pb.h"

namespace bigbrain {

// This is an implementation of a NetworkUnit as a Pulse Integrate-and-Fire
// neuron. Eventually it may extend from its own Neuron class, but as of right
// now it just extends from a base NetworkUnit
//
// Note that it has its own parameter and state description:
// PulseIntegrateAndFireDesc

class PulseIntegrateAndFire : public Neuron {
 public:
  PulseIntegrateAndFire();
  virtual ~PulseIntegrateAndFire();

  virtual bool CalculateDerivatives(const float& time,
                                    const google::protobuf::Message* input,
                                    google::protobuf::Message* output);
  virtual bool UpdateState(const float& time, const float& dt);
  virtual google::protobuf::Message* get_state();
  virtual void set_params(google::protobuf::Message* params);
  virtual double get_membrane_potential();

  virtual bool RequiresUpdatingSynapses();

  virtual google::protobuf::Message* get_unique_params();

 private:
  enum FiringState {
    FS_FIRING = 1,
    FS_PLASTIC = 2,
    FS_RESTING = 4,
    FS_SPIKING = 8
  };

  PulseIntegrateAndFireState state_;
  PulseIntegrateAndFireParams* params_;

  DISALLOW_COPY_AND_ASSIGN(PulseIntegrateAndFire);
};

}  // namespace bigbrain
#endif  // NETWORK_OBJECTS_CELLS_PULSE_INTEGRATE_AND_FIRE_H_
