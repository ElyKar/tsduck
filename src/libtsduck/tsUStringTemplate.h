//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2018, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------

#pragma once


//----------------------------------------------------------------------------
// Assign from std::vector and std::array.
//----------------------------------------------------------------------------

// With Microsoft compiler:
// warning C4127: conditional expression is constant
// for expression: if (sizeof(CHARTYPE) == sizeof(UChar)) {
#if defined(TS_MSC)
    #pragma warning(push)
    #pragma warning(disable:4127)
#endif

template <typename CHARTYPE, typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
ts::UString& ts::UString::assign(const std::vector<CHARTYPE>& vec, INT count)
{
    // The character type must be 16 bits.
    assert(sizeof(CHARTYPE) == sizeof(UChar));
    if (sizeof(CHARTYPE) == sizeof(UChar)) {

        // Maximum number of characters to check.
        // Take care, carefully crafted expression.
        const size_t last = std::min<std::size_t>(vec.size(), static_cast<size_t>(std::max<INT>(0, count)));

        // Compute actual string length.
        size_type n = 0;
        while (n < last && vec[n] != static_cast<CHARTYPE>(0)) {
            ++n;
        }

        // Assign string.
        assign(reinterpret_cast<const UChar*>(vec.data()), n);
    }
    return *this;
}

template <typename CHARTYPE, std::size_t SIZE, typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
ts::UString& ts::UString::assign(const std::array<CHARTYPE, SIZE>& arr, INT count)
{
    // The character type must be 16 bits.
    assert(sizeof(CHARTYPE) == sizeof(UChar));
    if (sizeof(CHARTYPE) == sizeof(UChar)) {

        // Maximum number of characters to check.
        // Take care, carefully crafted expression.
        const std::size_t last = std::min<std::size_t>(arr.size(), static_cast<std::size_t>(std::max<INT>(0, count)));

        // Compute actual string length.
        size_type n = 0;
        while (n < last && arr[n] != static_cast<CHARTYPE>(0)) {
            ++n;
        }

        // Assign string.
        assign(reinterpret_cast<const UChar*>(arr.data()), n);
    }
    return *this;
}

#if defined(TS_MSC)
    #pragma warning(pop)
#endif

template <typename CHARTYPE>
ts::UString& ts::UString::assign(const std::vector<CHARTYPE>& vec)
{
    return assign(vec, vec.size());
}

template <typename CHARTYPE, std::size_t SIZE>
ts::UString& ts::UString::assign(const std::array<CHARTYPE, SIZE>& arr)
{
    return assign(arr, arr.size());
}


//----------------------------------------------------------------------------
// Template constructors.
//----------------------------------------------------------------------------

template <typename CHARTYPE, typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
ts::UString::UString(const std::vector<CHARTYPE>& vec, INT count, const allocator_type& alloc) :
    SuperClass(alloc)
{
    assign(vec, count);
}

template <typename CHARTYPE>
ts::UString::UString(const std::vector<CHARTYPE>& vec, const allocator_type& alloc) :
    SuperClass(alloc)
{
    assign(vec);
}

template <typename CHARTYPE, std::size_t SIZE, typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
ts::UString::UString(const std::array<CHARTYPE, SIZE>& arr, INT count, const allocator_type& alloc) :
    SuperClass(alloc)
{
    assign(arr, count);
}

template <typename CHARTYPE, std::size_t SIZE>
ts::UString::UString(const std::array<CHARTYPE, SIZE>& arr, const allocator_type& alloc) :
    SuperClass(alloc)
{
    assign(arr);
}


//----------------------------------------------------------------------------
// Split a string based on a separator character.
//----------------------------------------------------------------------------

template <class CONTAINER>
void ts::UString::split(CONTAINER& container, UChar separator, bool trimSpaces, bool removeEmpty) const
{
    const UChar* sep = 0;
    const UChar* input = c_str();
    container.clear();

    do {
        // Locate next separator
        for (sep = input; *sep != separator && *sep != 0; ++sep) {
        }
        // Extract segment
        UString segment(input, sep - input);
        if (trimSpaces) {
            segment.trim();
        }
        if (!removeEmpty || !segment.empty()) {
            container.push_back(segment);
        }
        // Move to beginning of next segment
        input = *sep == 0 ? sep : sep + 1;
    } while (*sep != 0);
}


//----------------------------------------------------------------------------
// Split a string into segments by starting / ending characters.
//----------------------------------------------------------------------------

template <class CONTAINER>
void ts::UString::splitBlocks(CONTAINER& container, UChar startWith, UChar endWith, bool trimSpaces) const
{
    const UChar *sep = 0;
    const UChar* input = c_str();
    container.clear();

    do {
        int blocksStillOpen = 0;
        // Locate next block-opening character
        while (*input != startWith && *input != 0) {
            // Input now points to the first block opening character
            ++input;
        }

        // Locate the next block-ending character corresponding to the considered block
        for (sep = input; *sep != 0; ++sep) {
            if (*sep == startWith) {
                ++blocksStillOpen;
                continue;
            }
            if (*sep == endWith) {
                --blocksStillOpen;
                if (blocksStillOpen == 0) {
                    break;
                }
            }

        }
        // Extract segment
        UString segment(input, sep - input + (*sep == 0 ? 0 : 1));
        // trim spaces if needed
        if (trimSpaces) {
            segment.trim();
        }
        container.push_back(segment);
        // Move to beginning of next segment
        input = *sep == 0 ? sep : sep + 1;
    } while (*sep != 0 && *(sep + 1) != 0);
}


//----------------------------------------------------------------------------
// Split a string into multiple lines which are not larger than a maximum.
//----------------------------------------------------------------------------

template <class CONTAINER>
void ts::UString::splitLines(CONTAINER& lines, size_t maxWidth, const UString& otherSeparators, const UString& nextMargin, bool forceSplit) const
{
    // Cleanup container
    lines.clear();

    // If line smaller than max size or next margin too wide, return one line
    if (length() <= maxWidth || nextMargin.length() >= maxWidth) {
        lines.push_back(*this);
        return;
    }

    size_t marginLength = 0; // No margin on first line (supposed to be in str)
    size_t start = 0;        // Index in str of start of current line
    size_t eol = 0;          // Index in str of last possible end-of-line
    size_t cur = 0;          // Current index in str

    // Cut lines
    while (cur < length()) {
        if (IsSpace(at(cur)) || (cur > start && otherSeparators.find(at(cur-1)) != NPOS)) {
            // Possible end of line here
            eol = cur;
        }
        bool cut = false;
        if (marginLength + cur - start >= maxWidth) { // Reached max width
            if (eol > start) {
                // Found a previous possible end-of-line
                cut = true;
            }
            else if (forceSplit) {
                // No previous possible end-of-line but force cut
                eol = cur;
                cut = true;
            }
        }
        if (cut) {
            lines.push_back((marginLength == 0 ? UString() : nextMargin) + substr(start, eol - start));
            marginLength = nextMargin.length();
            // Start new line, skip leading spaces
            start = eol;
            while (start < length() && IsSpace(at(start))) {
                start++;
            }
            cur = eol = start;
        }
        else {
            cur++;
        }
    }

    // Rest of string on last line
    if (start < length()) {
        lines.push_back(nextMargin + substr(start));
    }
}


//----------------------------------------------------------------------------
// Join a part of a container of strings into one big string.
//----------------------------------------------------------------------------

template <class ITERATOR>
ts::UString ts::UString::Join(ITERATOR begin, ITERATOR end, const UString& separator)
{
    UString res;
    while (begin != end) {
        if (!res.empty()) {
            res.append(separator);
        }
        res.append(*begin);
        ++begin;
    }
    return res;
}


//----------------------------------------------------------------------------
// Check if a container of strings contains something similar to this string.
//----------------------------------------------------------------------------

template <class CONTAINER>
bool ts::UString::containSimilar(const CONTAINER& container) const
{
    for (typename CONTAINER::const_iterator it = container.begin(); it != container.end(); ++it) {
        if (similar(*it)) {
            return true;
        }
    }
    return false;
}


//----------------------------------------------------------------------------
// Locate into a map an element with a similar string.
//----------------------------------------------------------------------------

template <class CONTAINER>
typename CONTAINER::const_iterator ts::UString::findSimilar(const CONTAINER& container) const
{
    typename CONTAINER::const_iterator it = container.begin();
    while (it != container.end() && !similar(it->first)) {
        ++it;
    }
    return it;
}


//----------------------------------------------------------------------------
// Save strings from a container into a file, one per line.
//----------------------------------------------------------------------------

template <class ITERATOR>
bool ts::UString::Save(ITERATOR begin, ITERATOR end, const UString& fileName, bool append)
{
    std::ofstream file(fileName.toUTF8().c_str(), append ? (std::ios::out | std::ios::app) : std::ios::out);
    Save(begin, end, file);
    file.close();
    return !file.fail();
}

template <class ITERATOR>
bool ts::UString::Save(ITERATOR begin, ITERATOR end, std::ostream& strm)
{
    while (strm && begin != end) {
        strm << *begin << std::endl;
        ++begin;
    }
    return !strm.fail();
}

template <class CONTAINER>
bool ts::UString::Save(const CONTAINER& container, std::ostream& strm)
{
    return Save(container.begin(), container.end(), strm);
}

template <class CONTAINER>
bool ts::UString::Save(const CONTAINER& container, const UString& fileName, bool append)
{
    return Save(container.begin(), container.end(), fileName, append);
}


//----------------------------------------------------------------------------
// Load strings from a file, one per line, and insert them in a container.
//----------------------------------------------------------------------------

template <class CONTAINER>
bool ts::UString::LoadAppend(CONTAINER& container, std::istream& strm)
{
    UString line;
    while (line.getLine(strm)) {
        container.push_back(line);
        // Weird behaviour (bug?) with gcc 4.8.5: When we read 2 consecutive lines
        // with the same length, the storage of the previous string *in the container*
        // if overwritten by the new line. Maybe not in all cases. No problem with
        // other versions or compilers. As a workaround, we clear the string
        // between read operations.
        line.clear();
    }
    return strm.eof();
}

template <class CONTAINER>
bool ts::UString::Load(CONTAINER& container, std::istream& strm)
{
    container.clear();
    return LoadAppend(container, strm);
}

template <class CONTAINER>
bool ts::UString::LoadAppend(CONTAINER& container, const UString& fileName)
{
    std::ifstream file(fileName.toUTF8().c_str());
    return LoadAppend(container, file);
}

template <class CONTAINER>
bool ts::UString::Load(CONTAINER& container, const UString& fileName)
{
    container.clear();
    return LoadAppend(container, fileName);
}


//----------------------------------------------------------------------------
// Convert a string into an integer.
//----------------------------------------------------------------------------

template <typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
bool ts::UString::toInteger(INT& value, const UString& thousandSeparators) const
{
    // In this function, we work on formal integer types INT. We use std::numeric_limits<INT> to test the
    // capabilities of the type (is_signed, etc.) But, for each instantiation of INT, some expression
    // may not make sense and the Microsoft compiler complains about that. Disable specific warnings
    #if defined(TS_MSC)
        #pragma warning(push)
        #pragma warning(disable:4127)
        #pragma warning(disable:4146)
    #endif

    typedef typename std::numeric_limits<INT> limits;

    // Initial value, up to decode error
    value = static_cast<INT>(0);

    // Reject non-integer type (floating points, etc.) and invalid parameters
    if (!limits::is_integer) {
        return false;
    }

    // Locate actual begin and end of integer value
    const UChar* start = data();
    const UChar* end = start + length();
    while (start < end && IsSpace(*start)) {
        ++start;
    }
    while (start < end && IsSpace(*(end-1))) {
        --end;
    }

    // Skip optional sign
    bool negative = false;
    if (start < end) {
        if (*start == '+') {
            ++start;
        }
        else if (*start == '-') {
            if (!limits::is_signed) {
                // INT type is unsigned, invalid signed value
                return false;
            }
            ++start;
            negative = true;
        }
    }

    // Look for hexadecimal prefix
    int base = 10;
    if (start + 1 < end && start[0] == UChar('0') && (start[1] == UChar('x') || start[1] == UChar('X'))) {
        start += 2;
        base = 16;
    }

    // Filter empty string
    if (start == end) {
        return false;
    }
    assert(start < end);

    // Decode the string
    while (start < end) {
        const int digit = ToDigit(*start, base);
        if (digit >= 0) {
            // Character is a valid digit
            value = value * static_cast<INT>(base) + static_cast<INT>(digit);
        }
        else if (thousandSeparators.find(*start) == NPOS) {
            // Character is not a possible thousands separator
            break;
        }
        ++start;
    }

    // Apply sign
    if (negative) {
        value = -value;
    }

    // Success only if we went down to the end of string
    return start == end;

    #if defined(TS_MSC)
        #pragma warning(pop)
    #endif
}


//----------------------------------------------------------------------------
// Convert a string containing a list of integers into a container of integers.
//----------------------------------------------------------------------------

template <class CONTAINER, typename std::enable_if<std::is_integral<typename CONTAINER::value_type>::value>::type*>
bool ts::UString::toIntegers(CONTAINER& container, const UString& thousandSeparators, const UString& listSeparators) const
{
    // Let's name INT the integer type.
    // In all STL standard containers, value_type is a typedef for the element type.
    typedef typename CONTAINER::value_type INT;

    // Reset the content of the container
    container.clear();

    // Locate segments in the string
    size_type start = 0;
    size_type farEnd = length();

    // Loop on segments
    while (start < farEnd) {
        // Skip spaces and list separators
        while (start < farEnd && (IsSpace((*this)[start]) || listSeparators.find((*this)[start]) != NPOS)) {
            ++start;
        }
        // Locate end of segment
        size_type end = start;
        while (end < farEnd && listSeparators.find((*this)[end]) == NPOS) {
            ++end;
        }
        // Exit at end of string
        if (start >= farEnd) {
            break;
        }
        // Decode segment
        INT value = static_cast<INT>(0);
        if (!substr(start, end - start).toInteger<INT>(value, thousandSeparators)) {
            return false;
        }
        container.push_back(value);

        // Move to next segment
        start = end;
    }

    return true;
}


//----------------------------------------------------------------------------
// Append an array of C-strings to a container of strings.
//----------------------------------------------------------------------------

template <class CONTAINER>
CONTAINER& ts::UString::Append(CONTAINER& container, int argc, const char* const argv[])
{
    const size_type size = argc < 0 ? 0 : size_type(argc);
    for (size_type i = 0; i < size; ++i) {
        container.push_back(UString::FromUTF8(argv[i]));
    }
    return container;
}


//----------------------------------------------------------------------------
// Format a string containing a decimal value.
//----------------------------------------------------------------------------

template <typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
ts::UString ts::UString::Decimal(INT value,
                                 size_type min_width,
                                 bool right_justified,
                                 const UString& separator,
                                 bool force_sign,
                                 UChar pad)
{
    // We build the result string in s IN REVERSE ORDER
    UString s;
    s.reserve(32); // avoid reallocating (most of the time)

    // So, we need the separator in reverse order too.
    UString sep(separator);
    sep.reverse();

    // If the value is negative, format the absolute value.
    // The test "value != 0 && value < 1" means "value < 0"
    // but avoid GCC warning when the type is unsigned.
    const bool negative = value != 0 && value < 1;

    INT ivalue;
    if (negative) {
        // If the type is unsigned, "ivalue = -value" will never be executed
        // but Visual C++ complains. Suppress the warning.
        #if defined(TS_MSC)
            #pragma warning(push)
            #pragma warning(disable:4146)
        #endif

        ivalue = -value;

        #if defined(TS_MSC)
            #pragma warning(pop)
        #endif
    }
    else {
        ivalue = value;
    }

    // Format the value
    if (ivalue == 0) {
        s.push_back(u'0');
    }
    else {
        int count = 0;
        while (ivalue != 0) {
            s.push_back(u'0' + UChar(ivalue % 10));
            ivalue /= 10;
            if (++count % 3 == 0 && ivalue != 0) {
                s += sep;
            }
        }
    }
    if (negative) {
        s.push_back(u'-');
    }
    else if (force_sign) {
        s.push_back(u'+');
    }

    // Reverse characters in string
    s.reverse();

    // Adjust string width.
    if (s.size() < min_width) {
        if (right_justified) {
            s.insert(0, min_width - s.size(), pad);
        }
        else {
            s.append(min_width - s.size(), pad);
        }
    }

    // Return the formatted result
    return s;
}


//----------------------------------------------------------------------------
// Format a string containing an hexadecimal value.
//----------------------------------------------------------------------------

template <typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
ts::UString ts::UString::Hexa(INT value,
                              size_type width,
                              const UString& separator,
                              bool use_prefix,
                              bool use_upper)
{
    // We build the result string in s IN REVERSE ORDER
    UString s;
    s.reserve(32); // avoid reallocating (most of the time)

    // So, we need the separator in reverse order too.
    UString sep(separator);
    sep.reverse();

    // Default to the natural size of the type.
    if (width == 0) {
        width = 2 * sizeof(INT);
    }

    // Format the value
    int count = 0;
    while (width != 0) {
        const int nibble = int(value & 0xF);
        value >>= 4;
        --width;
        if (nibble < 10) {
            s.push_back(u'0' + UChar(nibble));
        }
        else if (use_upper) {
            s.push_back(u'A' + UChar(nibble - 10));
        }
        else {
            s.push_back(u'a' + UChar(nibble - 10));
        }
        if (++count % 4 == 0 && width > 0) {
            s += sep;
        }
    }

    // Add the optional prefix, still in reverse order.
    if (use_prefix) {
        s.push_back(u'x');
        s.push_back(u'0');
    }

    // Reverse characters in string
    return s.toReversed();
}


//----------------------------------------------------------------------------
// Format a percentage string.
//----------------------------------------------------------------------------

template <typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
ts::UString ts::UString::Percentage(INT value, INT total)
{
    if (total < 0) {
        return u"?";
    }
    if (total == 0) {
        return u"0.00%";
    }
    else {
        // Integral percentage
        const int p1 = int((100 * uint64_t(value)) / uint64_t(total));
        // Percentage first 2 decimals
        const int p2 = int(((10000 * uint64_t(value)) / uint64_t(total)) % 100);
        return Format(u"%d.%02d%%", {p1, p2});
    }
}


//----------------------------------------------------------------------------
// Reduce the size of the string to a given length from an alien integer type.
//----------------------------------------------------------------------------

template <typename INT, typename std::enable_if<std::is_integral<INT>::value>::type*>
void ts::UString::trimLength(INT length, bool trimTrailingSpaces)
{
    // We assume here that UString::size_type is the largest unsigned type
    // and that it is safe to convert any positive value into this type.
    resize(std::min<size_type>(size(), size_type(std::max<INT>(0, length))));
    trim(false, trimTrailingSpaces);
}
