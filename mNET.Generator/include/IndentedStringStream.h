#pragma once

#ifndef _H_INDENTED_STREAM_
#define _H_INDENTED_STREAM_

#include <sstream>
#include <string>

namespace mnet
{
    class IndentedStringStream {
    public:
        IndentedStringStream(int indentSize = 4)
            : indentLevel(0), indentSize(indentSize), atLineStart(true) {
        }

        // Increase/decrease indentation
        void indent() { indentLevel++; }
        void dedent() { if (indentLevel > 0) indentLevel--; }

        // Write any type
        template <typename T>
        IndentedStringStream& operator<<(const T& value) {
            std::ostringstream temp;
            temp << value;
            WriteString(temp.str());
            return *this;
        }

        // Handle manipulators like std::endl
        IndentedStringStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            if (manip == static_cast<std::ostream & (*)(std::ostream&)>(std::endl)) {
                buffer << '\n';
                atLineStart = true;
            }
            else {
                manip(buffer);
            }
            return *this;
        }

        std::string str() const {
            return buffer.str();
        }

    private:
        std::stringstream buffer;
        int indentLevel;
        int indentSize;
        bool atLineStart;

        void WriteIndent() {
            for (int i = 0; i < indentLevel * indentSize; ++i)
                buffer.put(' ');
        }

        void WriteString(const std::string& text) {
            for (char c : text) {
                if (atLineStart) {
                    WriteIndent();
                    atLineStart = false;
                }

                buffer.put(c);

                if (c == '\n') {
                    atLineStart = true;
                }
            }
        }
    };
}

#endif // !_H_INDENTED_STREAM_
