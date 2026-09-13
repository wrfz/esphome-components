#pragma once

#include "esphome/core/entity_base.h"
#include "esphome/components/daikin_rotex_uart/protocol.h"
#include <functional>
#include <stdint.h>
#include <variant>
#include <list>
#include <memory>

namespace esphome {
namespace daikin_rotex_uart {

class TRequest;

class TEntity {
public:
    enum class Endian {
        Little,
        Big
    };

    using THandleFunc = std::function<uint16_t(uint8_t*)>;
    using TVariant = std::variant<double, bool, std::string>;
    using TPostHandleLabda = std::function<void(TEntity*, TEntity::TVariant const&, TEntity::TVariant const&)>;

    // Signal description of an entity. The I-config (set_entity) always exists; the
    // S-config (set_entity_s) is only valid when an "s:" sub-config was defined.
    // valid==true marks a fully initialized (non-default) configuration.
    struct TEntityArguments {
        EntityBase* pEntity;
        std::string id;
        uint8_t registryID;
        uint8_t offset;
        bool isSigned;
        uint8_t data_size;
        Endian endian;
        double divider;
        uint8_t accuracy_decimals;
        std::list<std::string> update_entities;
        THandleFunc handle_lambda;
        bool handle_lambda_set;
        bool valid;

        TEntityArguments()
        : pEntity(nullptr)
        , id("")
        , registryID(0x0)
        , offset(0)
        , isSigned(false)
        , data_size(1)
        , endian(Endian::Little)
        , divider(1.0)
        , accuracy_decimals(1)
        , update_entities({})
        , handle_lambda([](uint8_t*){ return 0; })
        , handle_lambda_set(false)
        , valid(false)
        {}

        TEntityArguments(
            EntityBase* _pEntity,
            std::string const& _id,
            uint8_t _registryID,
            uint8_t _offset,
            bool _isSigned,
            uint8_t _data_size,
            Endian _endian,
            float _divider,
            uint8_t _accuracy_decimals,
            std::list<std::string> const& _update_entities,
            THandleFunc _handle_lambda,
            bool _handle_lambda_set
        )
        : pEntity(_pEntity)
        , id(_id)
        , registryID(_registryID)
        , offset(_offset)
        , isSigned(_isSigned)
        , data_size(_data_size)
        , endian(_endian)
        , divider(_divider)
        , accuracy_decimals(_accuracy_decimals)
        , update_entities(_update_entities)
        , handle_lambda(_handle_lambda)
        , handle_lambda_set(_handle_lambda_set)
        , valid(true)
        {}
    };

public:
    TEntity();

    void setRequest(std::shared_ptr<TRequest> pRequest) {
        m_pRequest = pRequest;
    }

    std::shared_ptr<TRequest> getRequest() const {
        return m_pRequest;
    }

    std::string getName() const {
        return m_config.pEntity != nullptr ? m_config.pEntity->get_name().str() : "<INVALID>";
    }

    std::string get_id() const;
    void set_id(std::string const& id);

    uint8_t getRegistryID() const;
    uint8_t getOffset() const;
    uint8_t getDataSize() const;
    bool isSigned() const;
    double getDivider() const;
    Endian getEndian() const;
    uint8_t getAccuracyDecimals() const;
    bool hasHandleLambda() const;
    uint16_t callHandleLambda(uint8_t* data) const;

    bool is_active() const;
    bool is_s_active() const;
    bool is_i_active() const;

    std::string convert(uint8_t* data);

    void set_entity(TEntityArguments&& arg) {
        m_config = std::move(arg);
    }

    void set_entity_s(TEntityArguments&& arg) {
        m_config_s = std::move(arg);
    }

    std::list<std::string> const& get_update_entities();

    void set_post_handle(TPostHandleLabda&& func) {
        m_post_handle_lambda = std::move(func);
    }

    virtual void update(uint32_t millis) {}

protected:
    virtual bool handleValue(uint16_t value, TVariant& current, TVariant& previous) = 0;

protected:
    TEntityArguments m_config;
    TEntityArguments m_config_s;

private:
    std::shared_ptr<TRequest> m_pRequest;
    TPostHandleLabda m_post_handle_lambda;
};

inline std::string TEntity::get_id() const {
    return m_config.id;
}

inline void TEntity::set_id(std::string const& id) {
    m_config.id = id;
}

inline bool TEntity::is_s_active() const {
    return TProtocolManager::getInstance().is_s_active() && m_config_s.valid;
}

inline bool TEntity::is_i_active() const {
    return TProtocolManager::getInstance().is_i_active();
}

inline bool TEntity::is_active() const {
    return is_s_active() || is_i_active();
}

inline uint8_t TEntity::getRegistryID() const {
    return is_s_active() ? m_config_s.registryID : m_config.registryID;
}

inline uint8_t TEntity::getOffset() const {
    return is_s_active() ? m_config_s.offset : m_config.offset;
}

inline uint8_t TEntity::getDataSize() const {
    return is_s_active() ? m_config_s.data_size : m_config.data_size;
}

inline bool TEntity::isSigned() const {
    return is_s_active() ? m_config_s.isSigned : m_config.isSigned;
}

inline double TEntity::getDivider() const {
    return is_s_active() ? m_config_s.divider : m_config.divider;
}

inline TEntity::Endian TEntity::getEndian() const {
    return is_s_active() ? m_config_s.endian : m_config.endian;
}

inline uint8_t TEntity::getAccuracyDecimals() const {
    return is_s_active() ? m_config_s.accuracy_decimals : m_config.accuracy_decimals;
}

inline bool TEntity::hasHandleLambda() const {
    return is_s_active() ? m_config_s.handle_lambda_set : m_config.handle_lambda_set;
}

inline uint16_t TEntity::callHandleLambda(uint8_t* data) const {
    return is_s_active() ? m_config_s.handle_lambda(data) : m_config.handle_lambda(data);
}

inline std::list<std::string> const& TEntity::get_update_entities() {
    return is_s_active() ? m_config_s.update_entities : m_config.update_entities;
}

}
}