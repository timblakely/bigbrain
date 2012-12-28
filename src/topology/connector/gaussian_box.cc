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

#include "topology/connector/gaussian_box.h"

#include <boost/random/taus88.hpp>
#include <stdlib.h>
#include <iostream>

#include "boost/random/uniform_01.hpp"
#include "network/objects/neuron.h"
#include "protos/basics.pb.h"
#include "protos/neuron.pb.h"
#include "protos/topology.pb.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

using std::vector;
using boost::random::uniform_01;

GaussianBoxConnector::GaussianBoxConnector() {
}

GaussianBoxConnector::~GaussianBoxConnector() {
}

void GaussianBoxConnector::MakeConnections(Neuron* source,
                                           vector<Neuron*>& potential_targets) {
  const Position& source_pos = source->get_common_params()->pos();

  double size_x = params_.width() * 2 + 1;
  double size_x2 = size_x / 2;
  double size_y = params_.height() * 2 + 1;
  double size_y2 = size_y / 2;
  double size_z = params_.depth() * 2 + 1;
  double size_z2 = size_y / 2;
  double scale_x = 2 * 3.14159265358 / size_x;
  double scale_y = 2 * 3.14159265358 / size_y;
  double scale_z = 2 * 3.14159265358 / size_z;

  uniform_01<> random;

  double origin[3];

  if (params_.use_origin()) {
    origin[0] = params_.origin().x();
    origin[1] = params_.origin().y();
    origin[2] = params_.origin().z();
  } else {
    origin[0] = source_pos.x();
    origin[1] = source_pos.y();
    origin[2] = source_pos.z();
  }

  // This can be GREATLY sped up with some form of spatial culling
  for (auto target : potential_targets) {
    // Make sure we don't connect to ourself if we don't allow it
    if (source->get_common_params()->uuid()
        == target->get_common_params()->uuid() && !allow_self_connections_) {
      continue;
    }

    const Position& target_pos = target->get_common_params()->pos();
    if (abs(target_pos.x() - origin[0]) <= params_.width()
        && abs(target_pos.y() - origin[1]) <= params_.height()
        && abs(target_pos.z() - origin[2]) <= params_.depth()) {
      // Since we're close enough, let's see if we can connect
      double relative[3];
      relative[0] = (target_pos.x() - origin[0]) * scale_x;
      relative[1] = (target_pos.y() - origin[1]) * scale_y;
      relative[2] = (target_pos.z() - origin[2]) * scale_z;

      double len = sqrt(
          pow(relative[0] * relative[0], 2) + pow(relative[1] * relative[1], 2)
              + pow(relative[2] * relative[2], 2));

      double gauss_val = params_.amplitude() / pow(2 * 3.14159265358, len / 2)
          * exp(
              -1 / 2.75 * relative[0] * relative[0] + relative[1] * relative[1]
                  + relative[2] * relative[2]);

      // The following is an implementation that Synergy used, and is kept here
      // for reference
      /*

       double gauss_val = params_.amplitude() * exp(-((
       pow(relative[0] * scale_x,2) + pow(relative[1] * scale_y,2))
       /params_.std()));

       */

      double randval = random(random_);
      if (fabs(gauss_val) > randval) {
        AddLink(source, target);
      }
    }
  }
}

google::protobuf::Message* GaussianBoxConnector::get_params() {
  return &params_;
}

}  // namespace bigbrain
