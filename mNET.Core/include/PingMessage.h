#pragma once

#ifndef _H_PING_MESSAGE_
#define _H_PING_MESSAGE_

#include "mnet.h"
#include "INetMessage.h"
#include <vector>

namespace mnet 
{
    class DLL_API PingMessage
        : public INetMessage
    {
    public:
        static constexpr size_t TYPE = 0;

        PingMessage();

        virtual bool Decode(const std::vector<char>& data) override;
        virtual std::vector<char> Encode() override;
    };

    class DLL_API PingReply
        : public INetMessage
    {
    public:
        static constexpr size_t TYPE = 1;

        PingReply();

        virtual bool Decode(const std::vector<char>& data) override;
        virtual std::vector<char> Encode() override;

        short GetCode() const { return m_Code; }
        void SetCode(short code) { m_Code = code; }

    private:
        short m_Code;
    };
}

#endif

