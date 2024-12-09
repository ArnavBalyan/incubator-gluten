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

#include "udf/Udf.h"
#include "UdfCommon.h"
#include <vector>
#include <memory>
#include "Registry.h"

namespace gluten {

std::vector<std::shared_ptr<UdfRegisterer>>& globalUdfRegisterers() {
    static std::vector<std::shared_ptr<UdfRegisterer>> registerers;
    return registerers;
}

void initializeUdfRegisterers() {
    static bool initialized = false;
    if (initialized) {
        return;
    }

    // Register UDFs
    extern void registerHiveStringStringUdf();
    registerHiveStringStringUdf();

    initialized = true;
}

} // namespace gluten
