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

#ifndef CONTROLLER_SIMULATOR_H_
#define CONTROLLER_SIMULATOR_H_

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <map>

#include "base_types.h"
#include "network/network.h"

#include "google/protobuf/message.h"

namespace bigbrain {

// This is where the job thread management should go

class Simulator {
  friend class JSONSimulatorAPI;
  friend class MsgpackSimulatorAPI;
 public:
  Simulator();
  virtual ~Simulator();

  void Beep(int test);
  void GenerateNetwork(std::string description);
  void RunTimestep(const double& t, const double& dt);
  void RunUntil(const double& t, const double& dt);

  std::vector<uint64> GetNeuronUUIDs();
  std::vector<double> GetMembranePotentials();
  std::vector<double> GetCellPosition(const uint64& uuid);
  std::vector<uint64> GetCellConnections(const uint64& uuid);
  std::vector<std::string> GetCellParamNames(const uint64& uuid);
  std::vector<std::string> GetCellParamValues(const uint64& uuid);
  std::vector<std::string> GetCellStateNames(const uint64& uuid);
  std::vector<std::string> GetCellStateValues(const uint64& uuid);

 protected:
  std::vector<std::string> GetProtoFieldNames(google::protobuf::Message*);
  std::vector<std::string> GetProtoFieldValues(google::protobuf::Message*);

  std::string description_;
  Network network_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Simulator);
};

}  // namespace bigbrain
#endif  // CONTROLLER_SIMULATOR_H_
