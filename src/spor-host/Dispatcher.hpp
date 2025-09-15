#pragma once

#include <cstddef>
#include <span>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "spor-common/Messages.hpp"
#include "zpp_bits.h"

template <typename Handler, typename... Args>
class MessageDispatcher {
public:
    explicit MessageDispatcher(Handler &handler) : handler_(handler) {}

    bool DispatchMessage(uint8_t messageTypeIndex, std::span<std::byte> data, Args &&...args) {
        return DispatchMessageImpl(
            messageTypeIndex, data, std::forward<Args>(args)...,
            std::make_index_sequence<std::variant_size_v<Message>>{}
        );
    }

    bool CanDecodeMessage(uint8_t messageTypeIndex, std::span<std::byte> data) {
        return CanDecodeMessageImpl(messageTypeIndex, data, std::make_index_sequence<std::variant_size_v<Message>>{});
    }

private:
    template <std::size_t... Is>
    bool DispatchMessageImpl(
        uint8_t messageTypeIndex, std::span<std::byte> data, Args &&...args, std::index_sequence<Is...>
    ) {
        bool handled = false;
        (void)((messageTypeIndex == Is ? (handled = DecodeAndHandle<Is>(data, std::forward<Args>(args)...), true)
                                       : false) ||
               ...);
        return handled;
    }

    template <std::size_t... Is>
    bool CanDecodeMessageImpl(uint8_t messageTypeIndex, std::span<std::byte> data, std::index_sequence<Is...>) {
        bool canDecode = false;
        (void)((messageTypeIndex == Is ? (canDecode = CanDecode<Is>(data), true) : false) || ...);
        return canDecode;
    }

    template <std::size_t Index>
    bool DecodeAndHandle(std::span<std::byte> data, Args &&...args) {
        using MessageType = std::variant_alternative_t<Index, Message>;

        MessageType message;
        zpp::bits::in in(data, zpp::bits::size_varint{});
        auto result = in(message);

        if (zpp::bits::failure(result)) {
            return false;
        }

        if constexpr (requires { handler_.OnMessage(message, std::forward<Args>(args)...); }) {
            handler_.OnMessage(message, std::forward<Args>(args)...);
        } else if constexpr (requires {
                                 handler_.OnMessage(
                                     std::type_identity<MessageType>{}, message, std::forward<Args>(args)...
                                 );
                             }) {
            handler_.OnMessage(std::type_identity<MessageType>{}, message, std::forward<Args>(args)...);
        } else if constexpr (requires {
                                 (handler_.*
                                  (&Handler::template OnMessage<MessageType>))(message, std::forward<Args>(args)...);
                             }) {
            (handler_.*(&Handler::template OnMessage<MessageType>))(message, std::forward<Args>(args)...);
        } else {
            const char *typeName = typeid(MessageType).name();
            if constexpr (requires { handler_.OnUnhandledMessage(typeName, message, std::forward<Args>(args)...); }) {
                handler_.OnUnhandledMessage(typeName, message, std::forward<Args>(args)...);
            }
        }

        return true;
    }

    template <std::size_t Index>
    bool CanDecode(std::span<std::byte> data) {
        using MessageType = std::variant_alternative_t<Index, Message>;

        MessageType message;
        zpp::bits::in testIn(data, zpp::bits::size_varint{});
        auto result = testIn(message);

        return !zpp::bits::failure(result);
    }

    Handler &handler_;
};

template <typename BaseClass>
class MessageHandlerMixin {
public:
    template <typename T>
    void OnMessage(const T &message) {
        static_cast<BaseClass *>(this)->OnUnhandledMessage(typeid(T).name(), message);
    }

    void OnUnhandledMessage(const char *typeName, const auto &message) {
        // Default: do nothing
    }
};