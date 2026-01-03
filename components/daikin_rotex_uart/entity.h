#pragma once

#include "esphome/core/entity_base.h"
#include <functional>
#include <stdint.h>
#include <variant>
#include <list>

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

    uint8_t getRegistryID() const {
        return m_config.registryID;
    }

    uint8_t getOffset() const {
        return m_config.offset;
    }

    uint8_t getDataSize() const {
        return m_config.data_size;
    }

    std::string convert(uint8_t* data);

    void set_entity(TEntityArguments&& arg) {
        m_config = std::move(arg);
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

inline std::list<std::string> const& TEntity::get_update_entities() {
    return m_config.update_entities;
}

}
}
