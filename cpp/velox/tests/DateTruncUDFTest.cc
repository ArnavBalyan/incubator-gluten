#include <vector>
#include "udf/UdfLoader.h"
#include "velox/expression/SimpleFunctionRegistry.h"
#include "velox/functions/prestosql/tests/utils/FunctionBaseTest.h"
#include "velox/parse/TypeResolver.h"

using namespace facebook::velox::functions::test;
using namespace facebook::velox;

class DateTruncUdfTest : public FunctionBaseTest {
 protected:
  static void SetUpTestCase() {
    // Initialize Velox type resolver
    parse::registerTypeResolver();

    // Create UdfLoader instance
    auto udfLoader = gluten::UdfLoader::getInstance();

    // Load and register UDF library
    const std::string udfLibPath = "../udf/udf-lib/libUdfFramework.so";
    LOG(INFO) << "Loading UDF library: " << udfLibPath;
    udfLoader->loadUdfLibraries(udfLibPath);
    udfLoader->registerUdf();

    // Debugging: List registered UDFs
    auto signatures = udfLoader->getRegisteredUdfSignatures();
    LOG(INFO) << "Registered UDFs:";
    for (const auto& sig : signatures) {
      LOG(INFO) << " - Name: " << sig->name
                << ", ArgTypes: " << sig->argTypes;
    }

    // Initialize memory manager
    memory::MemoryManager::testingSetInstance({});
  }
};

TEST_F(DateTruncUdfTest, dateTruncRegistration) {
  const std::string name = "com.uber.hive.udf.DateTrunc";
  EXPECT_EQ(TypeKind::VARCHAR, exec::simpleFunctions().resolveFunction(name, {VARCHAR(), VARCHAR()})->type()->kind());
}

TEST_F(DateTruncUdfTest, dateTruncFunctionality) {
  // Test the UDF directly using the short name
  auto result1 = evaluateOnce<std::string>(
      "DateTrunc(c0, c1)",
      std::optional<std::string>("second"),
      std::optional<std::string>("2016-03-29T18:18:18.123"));
  EXPECT_EQ(result1, "2016-03-29T18:18:18");

  auto result2 = evaluateOnce<std::string>(
      "DateTrunc(c0, c1)",
      std::optional<std::string>("minute"),
      std::optional<std::string>("2016-03-29T18:18:18"));
  EXPECT_EQ(result2, "2016-03-29T18:18:00");

  auto result3 = evaluateOnce<std::string>(
      "DateTrunc(c0, c1)",
      std::optional<std::string>("hour"),
      std::optional<std::string>("2016-03-29T18:18:18"));
  EXPECT_EQ(result3, "2016-03-29T18:00:00");

  auto result4 = evaluateOnce<std::string>(
      "DateTrunc(c0, c1)",
      std::optional<std::string>("day"),
      std::optional<std::string>("2016-03-29T18:18:18"));
  EXPECT_EQ(result4, "2016-03-29T00:00:00");
}
