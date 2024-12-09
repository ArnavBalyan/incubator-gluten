#ifndef REGISTRY_H
#define REGISTRY_H

#include <vector>
#include <memory>
#include "UdfCommon.h"

namespace gluten {

// Declare the global registry
std::vector<std::shared_ptr<UdfRegisterer>>& globalUdfRegisterers();

// Declare the initialization function
void initializeUdfRegisterers();

} // namespace gluten

#endif // REGISTRY_H
