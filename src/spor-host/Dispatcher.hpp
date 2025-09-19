#pragma once

// This header previously provided a zpp-bits based dispatcher.
// The new wire format is decoded directly in Decoder.cpp.

template <typename Handler, typename... Args>
class MessageDispatcher {
public:
    explicit MessageDispatcher(Handler &handler) {}
};

template <typename BaseClass>
class MessageHandlerMixin {
public:
    template <typename T>
    void OnMessage(const T &message) {
        static_cast<BaseClass *>(this)->OnUnhandledMessage(typeid(T).name(), message);
    }

    void OnUnhandledMessage(const char *, const auto &) {}
};
