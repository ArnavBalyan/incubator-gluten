#ifndef REGISTRY_H
#define REGISTRY_H

#include <vector>
#include <memory>
#include "UdfCommon.h"

namespace gluten {

std::vector<std::shared_ptr<UdfRegisterer>>& globalUdfRegisterers();

void initializeUdfRegisterers();

} // namespace gluten

#endif // REGISTRY_H
