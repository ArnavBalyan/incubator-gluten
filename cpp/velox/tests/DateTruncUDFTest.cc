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
    const std::string udfLibPath = "/home/user/gluten/cpp/build/velox/udf/udf-lib/libUdfFramework.so";
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
