#pragma once

#include <engine/filesystem.hpp>
#include <engine/resource_map.hpp>
#include <renderer/renderer.hpp>

struct MaterialSetup {
    ResourceMap<PipelineSlot>* m_map{};
    Renderer* m_renderer{};
    Filesystem* m_filesystem{};

    void operator () ( Asset&& );
};
