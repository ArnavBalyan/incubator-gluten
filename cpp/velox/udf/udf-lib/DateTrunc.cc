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

#include "UdfFramework.h"
#include <velox/functions/Registerer.h>
#include <velox/type/Timestamp.h>
#include <unordered_map>
#include <string>
#include <chrono>
#include <ctime>

namespace gluten {

class DateTruncUdf : public UdfFramework {
public:
    DateTruncUdf() : name_("date_trunc"), returnType_("timestamp") {}

    const std::string& getName() const override { return name_; }
    const std::string& getReturnType() const override { return returnType_; }
    const std::vector<std::string>& getArgTypes() const override {
        return {"varchar", "timestamp", "varchar", "varchar"};
    }

    void registerWithVelox() override {
        facebook::velox::registerFunction<FieldSource, velox::Timestamp, std::string, velox::Timestamp>({name_});
        facebook::velox::registerFunction<FieldSourceToZone, velox::Timestamp, std::string, velox::Timestamp, std::string>({name_});
        facebook::velox::registerFunction<FieldSourceFromToZone, velox::Timestamp, std::string, velox::Timestamp, std::string, std::string>({name_});
    }

    void populateUdfEntry(UdfEntry& entry) const override {
        entry.name = name_.c_str();
        entry.returnType = returnType_.c_str();
        entry.numArgs = 4;
        entry.argTypes = new const char*[4]{"varchar", "timestamp", "varchar", "varchar"};
    }

private:
    const std::string name_;
    const std::string returnType_;

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
            {"hour", Precision::HOUR},
            {"day", Precision::DAY},
            {"week", Precision::WEEK},
            {"month", Precision::MONTH},
            {"year", Precision::YEAR},
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
            case Precision::WEEK:
                tm.tm_wday = 0;
                tm.tm_hour = 0;
                tm.tm_min = 0;
                tm.tm_sec = 0;
                break;
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
                throw std::invalid_argument("Invalid precision field");
        }
    }

    struct FieldSource {
        VELOX_DEFINE_FUNCTION_TYPES(T);

        FOLLY_ALWAYS_INLINE void call(out_type<velox::Timestamp>& result,
                                      const arg_type<std::string>& field,
                                      const arg_type<velox::Timestamp>& source) {
            truncate(result, field, source, "UTC", "UTC");
        }
    };

    struct FieldSourceToZone {
        VELOX_DEFINE_FUNCTION_TYPES(T);

        FOLLY_ALWAYS_INLINE void call(out_type<velox::Timestamp>& result,
                                      const arg_type<std::string>& field,
                                      const arg_type<velox::Timestamp>& source,
                                      const arg_type<std::string>& toTimeZone) {
            truncate(result, field, source, "UTC", toTimeZone);
        }
    };

    struct FieldSourceFromToZone {
        VELOX_DEFINE_FUNCTION_TYPES(T);

        FOLLY_ALWAYS_INLINE void call(out_type<velox::Timestamp>& result,
                                      const arg_type<std::string>& field,
                                      const arg_type<velox::Timestamp>& source,
                                      const arg_type<std::string>& fromTimeZone,
                                      const arg_type<std::string>& toTimeZone) {
            truncate(result, field, source, fromTimeZone, toTimeZone);
        }
    };

    static void truncate(out_type<velox::Timestamp>& result,
                         const std::string& field,
                         const velox::Timestamp& source,
                         const std::string& fromTimeZone,
                         const std::string& toTimeZone) {
        std::time_t rawTime = source.toMillis() / 1000;
        std::tm tm = *std::gmtime(&rawTime);

        auto precision = getPrecision(field);
        if (precision == Precision::INVALID) {
            throw std::invalid_argument("Invalid field: " + field);
        }

        truncateDate(precision, tm);

        std::time_t truncatedTime = std::mktime(&tm);
        result = velox::Timestamp::fromMillis(truncatedTime * 1000);
    }
};

} // namespace gluten
