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
#include <algorithm>

// Our custom namespace
namespace datetrunc {

static const int TRUNCATE_BY_WEEK = -7;

enum class Precision {
    SECOND = 0,
    MINUTE,
    HOUR,
    DAY,
    WEEK,
    MONTH,
    YEAR,
    INVALID
};

Precision getPrecision(const std::string& field) {
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

// Attempt parse with a single format. If success, modifies tmOut, returns true
bool tryParse(const std::string& text, const char* format, std::tm& tmOut) {
    std::istringstream iss(text);
    iss >> std::get_time(&tmOut, format);
    return !iss.fail();
}

// Attempt "withT" -> "withoutT" -> "dateOnly" in sequence
// Returns "withT" or "withSpace" or "dateOnly" if success, else empty
std::string parseDateTime(const std::string& text, std::tm& tm) {
    // Zero out struct
    std::memset(&tm, 0, sizeof(std::tm));

    // 1) withT = "yyyy-MM-dd'T'HH:mm:ss"
    if (tryParse(text, "%Y-%m-%dT%H:%M:%S", tm)) {
        return "withT";
    }
    // 2) withoutT = "yyyy-MM-dd HH:mm:ss"
    if (tryParse(text, "%Y-%m-%d %H:%M:%S", tm)) {
        return "withSpace";
    }
    // 3) dateOnly = "yyyy-MM-dd"
    if (tryParse(text, "%Y-%m-%d", tm)) {
        return "dateOnly";
    }

    return "";  // all attempts failed
}

// Java's "week" logic. Sets day_of_week to Monday, if we overshoot, subtract 7, then zero hour/min/sec
void replicateJavaWeekTrunc(std::tm& tm) {
    std::time_t original = std::mktime(&tm);
    int wday = tm.tm_wday; // 0=Sun,1=Mon,...6=Sat
    int offset = 1 - wday; // shift to Monday
    tm.tm_mday += offset;
    std::time_t truncated = std::mktime(&tm);
    if (truncated > original) {
        tm.tm_mday -= 7;
        std::mktime(&tm); // recalc
    }
    // Then zero out hour/min/sec
    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    std::mktime(&tm);
}

void truncateDate(Precision prec, std::tm& tm) {
    switch (prec) {
        case Precision::SECOND:
            break; // do nothing
        case Precision::MINUTE:
            tm.tm_sec = 0;
            break;
        case Precision::HOUR:
            tm.tm_min = 0;
            tm.tm_sec = 0;
            break;
        case Precision::DAY:
            tm.tm_hour = 0;
            tm.tm_min  = 0;
            tm.tm_sec  = 0;
            break;
        case Precision::WEEK:
            replicateJavaWeekTrunc(tm);
            break;
        case Precision::MONTH:
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min  = 0;
            tm.tm_sec  = 0;
            break;
        case Precision::YEAR:
            tm.tm_mon  = 0;
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min  = 0;
            tm.tm_sec  = 0;
            break;
        default:
            break;
    }
}

// Format the result in the same style that succeeded in parseDateTime()
std::string formatTime(const std::tm& tm, const std::string& style) {
    std::ostringstream oss;
    if (style == "withT") {
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    } else if (style == "withSpace") {
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    } else if (style == "dateOnly") {
        oss << std::put_time(&tm, "%Y-%m-%d");
    } else {
        // fallback
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    }
    return oss.str();
}

// The actual Velox function:
template <typename T>
struct DateTruncFunction {
    VELOX_DEFINE_FUNCTION_TYPES(T);

    FOLLY_ALWAYS_INLINE void call(
        out_type<facebook::velox::Varchar>& result,
        const arg_type<facebook::velox::Varchar>& field,
        const arg_type<facebook::velox::Varchar>& source) const {

        // 1. Convert field to lower-case & get precision
        std::string fieldStr(field.data(), field.size());
        std::transform(fieldStr.begin(), fieldStr.end(), fieldStr.begin(), ::tolower);
        auto prec = getPrecision(fieldStr);
        if (prec == Precision::INVALID) {
            result = ""; // return empty if invalid field
            return;
        }

        std::string sourceStr(source.data(), source.size());
        std::tm tm{};
        std::string style = parseDateTime(sourceStr, tm);
        if (style.empty()) {
            result = "";
            return;
        }

        // 3. Truncate
        truncateDate(prec, tm);

        // 4. Format in same style
        std::string out = formatTime(tm, style);
        result = out;
    }
};

// Register with Velox under the same name as Java
class DateTruncRegisterer final : public gluten::UdfRegisterer {
 public:
    int getNumUdf() override { return 1; }

    void populateUdfEntries(int& index, gluten::UdfEntry* udfEntries) override {
        udfEntries[index++] = {
            name_.c_str(),
            /*returnType=*/"varchar",
            /*arity=*/2,
            argTypes_,
            /*needContext=*/false,
            /*needReader=*/true
        };
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
} // namespace gluten
