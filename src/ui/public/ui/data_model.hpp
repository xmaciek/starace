#pragma once

#include <renderer/texture.hpp>
#include <shared/hash.hpp>

#include <cstdint>
#include <functional>
#include <memory_resource>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ui {

class DataModel {
public:
    using size_type = uint16_t;
    virtual ~DataModel() noexcept = default;
    DataModel() noexcept = default;

    virtual size_type revision() const;
    virtual size_type current() const;
    virtual size_type size() const;

    // TODO unionize
    virtual std::pmr::u32string at( size_type ) const;
    virtual Texture texture( size_type ) const;
    virtual float atF( size_type ) const;
    virtual Hash::value_type sprite( size_type ) const;

    virtual void activate( size_type );
    virtual void select( size_type );
    virtual void refresh( size_type i = 1 );
};

class GenericDataModel : public DataModel {
public:
    std::function<size_type()> m_size{};
    std::function<std::pmr::u32string(size_type)> m_at{};
    std::function<void(size_type)> m_activate{};
    std::function<void(size_type)> m_select{};
    std::function<void(size_type)> m_refresh{};
    std::function<size_type()> m_current{};
    std::function<size_type()> m_revision{};
    std::function<Texture(size_type)> m_texture{};

    ~GenericDataModel() noexcept = default;
    GenericDataModel() noexcept = default;

    virtual size_type current() const override;
    virtual size_type revision() const override;
    virtual size_type size() const override;
    virtual std::pmr::u32string at( size_type ) const override;
    virtual Texture texture( size_type ) const override;

    virtual void activate( size_type ) override;
    virtual void select( size_type ) override;
    virtual void refresh( size_type i = 1 ) override;
};

}

[[maybe_unused]]
inline std::unordered_map<std::string_view, ui::DataModel*> g_gameUiDataModels{};
