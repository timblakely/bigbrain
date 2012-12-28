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

#ifndef NETWORK_OBJECTS_NETWORK_UNIT_H_
#define NETWORK_OBJECTS_NETWORK_UNIT_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#include <unordered_map>
#include <stdint.h>
#include <string>

typedef uint32_t uint32;
typedef uint64_t uint64;

#include "boost/random/taus88.hpp"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "protos/neuron.pb.h"

namespace google {
namespace protobuf {
class Descriptor;
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

enum NetworkUnitType {
  NU_TYPE_UNINITALIZED,
  NU_TYPE_CELL,
  NU_TYPE_CHANNEL,
  NU_TYPE_SYNAPSE
};

class StoredNetworkUnit;

// This is the base class for any object that is contained in the network. These
// shouldn't be generated directly, rather by scripts loaded by the simulator
// that describe the layout and type of neurons, cells, and channels contained
// within.
//
// Note that every NetworkUnit has its own
//
// Important: This class essentially only contains the mathematics and a single
// member variable: the protobuf that contains all variables, states, etc. This
// is by design, both so that the user interface can access the states via RPC
// calls, and so that the entire state itself can be serialized to/from files
// easily
class NetworkUnit {
 public:
  NetworkUnit();
  virtual ~NetworkUnit();

  // Currently a dummy function that will evaluate the derivatives using a
  // differential equation solver (i.e. euler or RungeKutta4). This is called by
  // the network's evaluate step
  virtual bool CalculateDerivatives(
      const float& time, const google::protobuf::Message * input_state,
      google::protobuf::Message * output_state) = 0;

  // Called by the system after the current timestep's derivatives have been
  // solved
  virtual bool UpdateState(const float& time, const float&dt) = 0;

  // Returns the specific name of the object, which can be a duplicate of other
  // neurons or a unique identifier
  virtual std::string get_description() = 0;
  virtual google::protobuf::Message* get_state() = 0;
  virtual google::protobuf::Message* get_unique_params() = 0;

  // Common to all NetworkUnits
  bool Update(const float& time, const float &dt);
  const std::string& get_name();
  void set_name(const std::string& name);
  const uint64 get_id();
  void set_id(const uint64 id);
  void set_random_seed(const uint32& seed);
  CommonParams * get_common_params();

  virtual void set_params(google::protobuf::Message* params) = 0;
  const NetworkUnitType& get_type();

  // Free any memory associated with the differential equation solver
  static void FreeSolverMemory();

 protected:
  // ACMRandom is slightly less random, but it's a LOT smaller than MT random.
  // This is important, since we could be instantiating hundreds of millions of
  // network units, and need to keep memory usage to a minimum
  boost::random::taus88 rand_num_gen_;
  NetworkUnitType type_;

 private:
  // Implementation of the RungeKutta solver
  bool SolveViaRungeKutta(const float& time, const float& dt);

  // Legacy implementation of the RungeKutta solver, as implemented by Sean
  // Hill. This doesn't follow the actual equation and hence is slightly
  // different than a true RK solver, but may have speed benefits, which could
  // be why Sean implemented them this way
  bool SolveViaLegacyRungeKutta(const float& time, const float& dt);

  CommonParams common_params_;

  // This is the memory that is used for the solvers. One set of five *State per
  // NetworkUnit type. Currently not multithreaded safe!! Could use a mutex
  // around it. In fact, consider it a TODO(blakely)
  static google::protobuf::Message** get_solver_memory(
      const google::protobuf::Message* proto);
  static std::unordered_map<const google::protobuf::Descriptor*,
      google::protobuf::Message**> solver_memory_map_;

  DISALLOW_COPY_AND_ASSIGN(NetworkUnit);
};

}  // namespace bigbrain

#endif  // NETWORK_OBJECTS_NETWORK_UNIT_H_
