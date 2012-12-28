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

#include <boost/random/taus88.hpp>
#include <iostream>

#include "base_types.h"
#include "boost/random/uniform_01.hpp"
#include "network/objects/neuron.h"
#include "protos/basics.pb.h"
#include "protos/neuron.pb.h"
#include "protos/topology.pb.h"
#include "topology/arrangement.h"
#include "topology/arrangement/flat_rectangular.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

using boost::random::uniform_01;

FlatRectangularArrangement::FlatRectangularArrangement(const int32 &random_seed)
    : Arrangement(random_seed),
      num_neurons_placed_(0) {
}

FlatRectangularArrangement::~FlatRectangularArrangement() {
}

google::protobuf::Message *FlatRectangularArrangement::get_params() {
  return &params_;
}

void FlatRectangularArrangement::SetNeuronPosition(
    Neuron* cell, const int32& num_this_type_neurons) {
  Position* pos = cell->get_common_params()->mutable_pos();

  float neg_one_to_one;
  uniform_01<> range;

  double x, y;

  switch (params_.distribution()) {
    case DIST_RANDOM:
      // X

      neg_one_to_one = 2 * range(random_) - 1;
      pos->set_x(
          neg_one_to_one * params_.dimensions().width() / 2
              + params_.origin().x());
      // Y
      neg_one_to_one = 2 * range(random_) - 1;
      pos->set_y(
          neg_one_to_one * params_.dimensions().height() / 2
              + params_.origin().y());
      // Z
      neg_one_to_one = 2 * range(random_) - 1;
      pos->set_z(
          neg_one_to_one * params_.dimensions().depth() / 2
              + params_.origin().z());
      break;
    case DIST_UNIFORM:
      if (params_.num_cols() == 1) {
        x = params_.origin().x();
      } else {
        x = num_neurons_placed_ % params_.num_cols()
            * params_.dimensions().width() / (params_.num_cols() - 1)
            - params_.dimensions().width() / 2 + params_.origin().x();
      }
      if (params_.num_rows() == 1) {
        y = params_.origin().y();
      } else {
        y = floor(num_neurons_placed_ / params_.num_cols())
            * params_.dimensions().height() / (params_.num_rows() - 1)
            - params_.dimensions().height() / 2 + params_.origin().y();
      }

      pos->set_x(x);
      pos->set_y(y);
      pos->set_z(0);
      break;
  }
  ++num_neurons_placed_;
}

}  // namespace bigbrain
