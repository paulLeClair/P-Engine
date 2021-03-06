// This file is part of meshoptimizer library; see meshoptimizer.h for version/license details
#include "../../include/mesh_optimizer/meshoptimizer.h"

void meshopt_setAllocator(void* (*allocate)(size_t), void (*deallocate)(void*))
{
	meshopt_Allocator::Storage::allocate = allocate;
	meshopt_Allocator::Storage::deallocate = deallocate;
}
