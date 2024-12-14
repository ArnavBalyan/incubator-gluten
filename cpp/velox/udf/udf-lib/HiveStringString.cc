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

#include <velox/expression/VectorFunction.h>
#include <velox/functions/Macros.h>
#include <velox/functions/Registerer.h>
#include "udf/Udf.h"
#include "UdfCommon.h"
#include "Registry.h"

// This is an example UDF. New UDFs can be created using similar structure.

namespace hivestringstring {

template <typename T>
struct HiveStringStringFunction {
    VELOX_DEFINE_FUNCTION_TYPES(T);

    FOLLY_ALWAYS_INLINE void call(
        out_type<facebook::velox::Varchar>& result,
        const arg_type<facebook::velox::Varchar>& a) const {
        result.append(a.data());
        result.append(" cpp_udf");
    }
};

class HiveStringStringRegisterer final : public gluten::UdfRegisterer {
 public:
    int getNumUdf() override {
        return 1;
    }

    void populateUdfEntries(int& index, gluten::UdfEntry* udfEntries) override {
        udfEntries[index++] = {name_.c_str(), "varchar", 1, argTypes_, false, true};
    }

    void registerSignatures() override {
        facebook::velox::registerFunction<
            HiveStringStringFunction,
            facebook::velox::Varchar,
            facebook::velox::Varchar>(
                {name_});
    }

 private:
    const std::string name_ = "org.apache.spark.sql.hive.execution.UDFStringString";
    const char* argTypes_[1] = {"varchar"};
};

} // namespace hivestringstring

namespace gluten {
void registerHiveStringStringUdf() {
    globalUdfRegisterers().push_back(
        std::make_shared<hivestringstring::HiveStringStringRegisterer>());
}
}
