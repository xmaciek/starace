#pragma once

#include <engine/filesystem.hpp>
#include <renderer/renderer.hpp>
#include <shared/resource_map.hpp>

struct MaterialSetup {
    ResourceMap<PipelineSlot>* m_map{};
    Renderer* m_renderer{};
    Filesystem* m_filesystem{};

    void operator () ( Asset&& );
};
