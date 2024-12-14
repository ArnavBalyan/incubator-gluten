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
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <unordered_map>

namespace datetrunc {

enum class Precision {
    SECOND,
    MINUTE,
    HOUR,
    DAY,
    WEEK,
    MONTH,
    YEAR,
    INVALID
};

static Precision getPrecision(const std::string& field) {
    static const std::unordered_map<std::string, Precision> precisionMap = {
        {"second", Precision::SECOND},
        {"minute", Precision::MINUTE},
        {"hour",   Precision::HOUR},
        {"day",    Precision::DAY},
        {"week",   Precision::WEEK},
        {"month",  Precision::MONTH},
        {"year",   Precision::YEAR}
    };

    auto it = precisionMap.find(field);
    return (it != precisionMap.end()) ? it->second : Precision::INVALID;
}

static void truncateDate(Precision precision, std::tm& tm) {
    switch (precision) {
        case Precision::SECOND:
            break;
        case Precision::MINUTE:
            tm.tm_sec = 0;
            break;
        case Precision::HOUR:
            tm.tm_min = 0;
            tm.tm_sec = 0;
            break;
        case Precision::DAY:
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            break;
        case Precision::WEEK: {
            int offset = (tm.tm_wday == 0) ? -6 : 1 - tm.tm_wday;
            tm.tm_mday += offset;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            break;
        }
        case Precision::MONTH:
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            break;
        case Precision::YEAR:
            tm.tm_mon = 0;
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
            break;
        case Precision::INVALID:
            throw std::invalid_argument("Invalid precision");
    }
}

static std::string formatTime(const std::tm& tm, bool hasT) {
    std::ostringstream oss;
    oss << std::put_time(&tm, hasT ? "%Y-%m-%dT%H:%M:%S" : "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

template <typename T>
struct DateTruncFunction {
    VELOX_DEFINE_FUNCTION_TYPES(T);

    FOLLY_ALWAYS_INLINE void call(
        out_type<facebook::velox::Varchar>& result,
        const arg_type<facebook::velox::Varchar>& field,
        const arg_type<facebook::velox::Varchar>& source) const {

        std::string fieldStr(field.data(), field.size());
        std::string sourceStr(source.data(), source.size());

        Precision precision = getPrecision(fieldStr);
        if (precision == Precision::INVALID) {
            result = "";
            return;
        }

        std::tm tm = {};
        bool hasT = (sourceStr.find('T') != std::string::npos);

        std::istringstream ss(sourceStr);
        if (hasT) {
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        } else {
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        }

        if (ss.fail()) {
            result = "";
            return;
        }

        truncateDate(precision, tm);

        result = formatTime(tm, hasT);
    }
};

class DateTruncRegisterer final : public gluten::UdfRegisterer {
 public:
    int getNumUdf() override { return 1; }

    void populateUdfEntries(int& index, gluten::UdfEntry* udfEntries) override {
        udfEntries[index++] = {name_.c_str(), "varchar", 2, argTypes_, false, true};
    }

    void registerSignatures() override {
        facebook::velox::registerFunction<
            DateTruncFunction,
            facebook::velox::Varchar,
            facebook::velox::Varchar,
            facebook::velox::Varchar>({name_});
    }

 private:
    const std::string name_ = "com.uber.hive.udf.DateTrunc";
    const char* argTypes_[2] = {"varchar", "varchar"};
};

} // namespace datetrunc

namespace gluten {
void registerDateTruncUdf() {
    globalUdfRegisterers().push_back(std::make_shared<datetrunc::DateTruncRegisterer>());
}
}
