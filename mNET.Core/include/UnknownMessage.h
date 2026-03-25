#pragma once

#ifndef _H_UNKNOWN_MESSAGE_
#define _H_UNKNOWN_MESSAGE_

#include "mnet.h"
#include "INetMessage.h"

namespace mnet
{
    class DLL_API UnknownMessage
        : public INetMessage
    {
    public:
        static constexpr size_t TYPE = -1;

        UnknownMessage();

        virtual bool Decode(const std::vector<char>& data) override;
        virtual std::vector<char> Encode() override;
    };
}

#endif

