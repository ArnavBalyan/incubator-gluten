/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "udf/Udaf.h"
#include "udf/Udf.h"

namespace gluten {

class UdfRegisterer {
 public:
  ~UdfRegisterer() = default;

  virtual int getNumUdf() = 0;

  virtual void populateUdfEntries(int& index, gluten::UdfEntry* udfEntries) = 0;

  virtual void registerSignatures() = 0;
};

class UdafRegisterer {
 public:
  ~UdafRegisterer() = default;

  virtual int getNumUdaf() = 0;

  virtual void populateUdafEntries(int& index, gluten::UdafEntry* udafEntries) = 0;

  virtual void registerSignatures() = 0;
};

} // namespace gluten
