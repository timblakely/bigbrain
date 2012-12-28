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

#include <Python.h>

#include <boost/program_options.hpp>
#include <stdio.h>
#include <unistd.h>
#include <iosfwd>
#include <iostream>
#include <ostream>

#include "controller/bigbrain_api.h"
#include "controller/simulator.h"

#define BIGBRAIN_VERSION "0.0.2"

namespace po = boost::program_options;

using bigbrain::Simulator;
using bigbrain::BigBrainAPI;
using std::string;
using std::cout;

int main(int argc, char** argv) {
  po::options_description generic_opts("Generic");
  generic_opts.add_options()
    ("help", "Prints this help message")
    ("version,v", "Version");

  po::options_description server_opts("Server options");
  server_opts.add_options()
    ("instance_name,n", po::value<string>()->required(),
        "Name of this instance, for Agent identification")
    ("rpc_port,p", po::value<string>()->default_value("32123"),
        "Port number to listen on for HTTP RPCs");
  po::options_description agent_opts("Agent options");
  agent_opts.add_options()
    ("front_end_host,F", po::value<string>()->required(), "Front-end hostname")
    ("front_end_port,P", po::value<string>()->required(),
        "Front-end port number");

  po::options_description all_opts("Options");
  all_opts.add(generic_opts).add(server_opts).add(agent_opts);

  po::variables_map opts;
  po::store(po::parse_command_line(argc, argv, all_opts), opts);

  if (opts.count("help")) {
    cout << all_opts << "\n";
    return 1;
  }
  if (opts.count("version")) {
    printf("BigBrain simulator, version: %s\n", BIGBRAIN_VERSION);
    return 1;
  }
  try {
    po::notify(opts);
  }
  catch(po::required_option &e) {
    cout << e.what() << "\n";
    return 1;
  }

  string input_name = opts["instance_name"].as<string>();
  string name = input_name;
  if (input_name == "localhost") {
    // Specifically for localhost, we're going to modify the name to be
    // localhost_[PID]
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "localhost_%i", getpid());
    name = buffer;
  }

  string port = opts["rpc_port"].as<string>();

  printf("Starting instance %s\n", name.c_str());

  Simulator sim;

  BigBrainAPI<Simulator> bigbrain(&sim);

  if (input_name == "localhost") {
    bigbrain.Initialize(name, port, "localhost", "12345");
  } else {
    bigbrain.Initialize();
  }

  bigbrain.Run(false);

  while (true) {
    usleep(1000000);
    if (!bigbrain.Running()) {
      break;
    }
  }

  bigbrain.TearDown();

  return 0;
}
