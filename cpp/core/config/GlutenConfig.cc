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

#include <jni.h>
#include <optional>
#include <regex>
#include "compute/ProtobufUtils.h"
#include "config.pb.h"
#include "jni/JniError.h"

namespace {

const std::string REGEX_REDACT_KEY = "spark.gluten.redaction.regex";
std::optional<std::regex> getRedactionRegex(const std::unordered_map<std::string, std::string>& conf) {
  auto it = conf.find(REGEX_REDACT_KEY);
  if (it != conf.end()) {
    return std::regex(it->second);
  }
  return std::nullopt;
}
} // namespace

namespace gluten {

const std::string REDACTED_VALUE = "*********(redacted)";

std::unordered_map<std::string, std::string>
parseConfMap(JNIEnv* env, const uint8_t* planData, const int32_t planDataLength) {
  std::unordered_map<std::string, std::string> sparkConfs;
  ConfigMap pConfigMap;
  gluten::parseProtobuf(planData, planDataLength, &pConfigMap);
  for (const auto& pair : pConfigMap.configs()) {
    sparkConfs.emplace(pair.first, pair.second);
  }

  return sparkConfs;
}

std::string printConfig(const std::unordered_map<std::string, std::string>& conf) {
  std::ostringstream oss;
  oss << std::endl;

  auto redactionRegex = getRedactionRegex(conf);

  for (const auto& [k, v] : conf) {
    if (redactionRegex && std::regex_match(k, *redactionRegex)) {
      oss << " [" << k << ", " << REDACTED_VALUE << "]\n";
    } else {
      oss << " [" << k << ", " << v << "]\n";
    }
  }
  return oss.str();
}

} // namespace gluten
