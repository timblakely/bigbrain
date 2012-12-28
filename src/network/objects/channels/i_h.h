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

#ifndef NETWORK_OBJECTS_CHANNELS_I_H_H_
#define NETWORK_OBJECTS_CHANNELS_I_H_H_

#include "base_types.h"

#include "network/objects/channel.h"

namespace bigbrain {

class IhChannel : public Channel {
 public:
  IhChannel();
  virtual ~IhChannel();

  // NetworkUnit overrides
  virtual bool CalculateDerivatives(
      const float& time, const google::protobuf::Message* input_state,
      google::protobuf::Message* output_state);
  virtual bool UpdateState(const float& time, const float& dt);
  virtual std::string get_description();
  virtual google::protobuf::Message* get_state();
  virtual void set_params(google::protobuf::Message*);
  virtual google::protobuf::Message* get_unique_params();

  // Channel overrides
  virtual double get_current();

 private:
  IhState state_;
  IhParams* params_;
  DISALLOW_COPY_AND_ASSIGN(IhChannel);
};

}  // namespace bigbrain
#endif  // NETWORK_OBJECTS_CHANNELS_I_H_H_
