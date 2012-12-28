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

#ifndef NETWORK_PARAMETER_COLLECTION_H_
#define NETWORK_PARAMETER_COLLECTION_H_

#include <vector>

class NetworkUnit;
namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace bigbrain {

class ParameterCollection {
 public:
  ParameterCollection();
  virtual ~ParameterCollection();

  // Add param set to parameter collection and TAKE OWNERSHIP! This will take in
  // a pointer to a parameter set and check if it's in the collection already.
  // If it is not, it adds the reference to the list and returns the same
  // pointer. If it is, it frees the current object and returns the
  // already-instantiated parameter set
  google::protobuf::Message* Insert(google::protobuf::Message* input_param_set);
  void Collapse();

 private:
  std::vector<google::protobuf::Message*> param_list_;
};

}  // namespace bigbrain
#endif  // NETWORK_PARAMETER_COLLECTION_H_
