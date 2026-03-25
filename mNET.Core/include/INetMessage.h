#pragma once

#ifndef _H_NET_MESSAGE_
#define _H_NET_MESSAGE_

#include <vector>
#include "mnet.h"

namespace mnet
{
    class Socket;


    INTERFACE INetMessage
    {
    public:
        static constexpr int MESSAGE_HEADER_LEN = sizeof(size_t) * 2;

        INetMessage(size_t type)
            : m_Type {type} { }

        virtual ~INetMessage() = default;

        size_t Type() const { return m_Type; }

        virtual bool Decode(const std::vector<char>& data) = 0;
        virtual std::vector<char> Encode() = 0;

    protected:
        size_t m_Type;

        void EncodeBaseMessageData(std::vector<char>& container) const;
		std::vector<char> CreateEncodingBuffer() const;
    private:
    };

}

#endif // !_H_NET_MESSAGE_
