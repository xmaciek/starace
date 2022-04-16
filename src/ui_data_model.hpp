#pragma once

#include <cstdint>
#include <functional>
#include <memory_resource>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ui {

struct DataModel {
    using size_type = uint16_t;

    virtual ~DataModel() noexcept = default;
    DataModel() noexcept = default;

    virtual size_type size() const = 0;
    virtual std::pmr::u32string at( size_type ) const = 0;
    virtual void activate( size_type );
    virtual void select( size_type );
};

struct StringListModel : public DataModel {
    std::pmr::vector<std::pmr::u32string> m_data{};

    ~StringListModel() noexcept = default;
    StringListModel() noexcept = default;

    virtual size_type size() const override;
    virtual std::pmr::u32string at( size_type ) const override;
};

struct GenericDataModel : public DataModel {
    std::function<size_type()> m_size{};
    std::function<std::pmr::u32string(size_type)> m_at{};
    std::function<void(size_type)> m_activate{};
    std::function<void(size_type)> m_select{};

    ~GenericDataModel() noexcept = default;
    GenericDataModel() noexcept = default;

    virtual size_type size() const override;
    virtual std::pmr::u32string at( size_type ) const override;
    virtual void activate( size_type ) override;
    virtual void select( size_type ) override;
};

}

[[maybe_unused]]
inline std::unordered_map<std::string_view, ui::DataModel*> g_gameUiDataModels{};
