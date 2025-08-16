#ifndef VXE_VOXEL_GRID_EVENT_H
#define VXE_VOXEL_GRID_EVENT_H

#include "Event.h"

namespace vxe {
    class GridChangedEvent : public Event {
    public:
        GridChangedEvent() = default;

        EVENT_CLASS_TYPE(GridChanged);
        EVENT_CLASS_CATEGORY(EventCategoryVoxelGrid);
    };
}

#endif