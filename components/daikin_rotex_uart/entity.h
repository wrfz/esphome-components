#pragma once

#include "esphome/core/entity_base.h"
#include <functional>
#include <stdint.h>
#include <variant>

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

    struct TEntityArguments {
        EntityBase* pEntity;
        std::string name;
        uint8_t registryID;
        uint8_t offset;
        bool isSigned;
        uint8_t data_size;
        Endian endian;
        double divider;
        uint8_t accuracy_decimals;
        THandleFunc handle_lambda;
        bool handle_lambda_set;

        TEntityArguments()
        : pEntity(nullptr)
        , name("")
        , registryID(0x0)
        , offset(0)
        , isSigned(false)
        , data_size(1)
        , endian(Endian::Little)
        , divider(1.0)
        , accuracy_decimals(1)
        , handle_lambda([](uint8_t*){ return 0; })
        , handle_lambda_set(false)
        {}

        TEntityArguments(
            EntityBase* _pEntity,
            std::string const& _name,
            uint8_t _registryID,
            uint8_t _offset,
            bool _isSigned,
            uint8_t _data_size,
            Endian _endian,
            float _divider,
            uint8_t _accuracy_decimals,
            THandleFunc _handle_lambda,
            bool _handle_lambda_set
        )
        : pEntity(_pEntity)
        , name(_name)
        , registryID(_registryID)
        , offset(_offset)
        , isSigned(_isSigned)
        , data_size(_data_size)
        , endian(_endian)
        , divider(_divider)
        , accuracy_decimals(_accuracy_decimals)
        , handle_lambda(_handle_lambda)
        , handle_lambda_set(_handle_lambda_set)
        {}
    };

public:
    TEntity()
    : m_pRequest(nullptr)
    {
    }

    void setRequest(std::shared_ptr<TRequest> pRequest) {
        m_pRequest = pRequest;
    }

    std::shared_ptr<TRequest> getRequest() const {
        return m_pRequest;
    }

    std::string getName() const {
        return m_config.pEntity != nullptr ? m_config.pEntity->get_name().str() : "<INVALID>";
    }

    EntityBase* get_entity_base() const {
        return m_config.pEntity;
    }

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

protected:
    virtual bool handleValue(uint16_t value, TVariant& current) = 0;

protected:
    TEntityArguments m_config;

private:
    std::shared_ptr<TRequest> m_pRequest;
};

}
}

