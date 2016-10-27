/*
 * Copyright 2016 Gary R. Van Sickle (grvs@users.sourceforge.net).
 *
 * This file is part of UniversalCodeGrep.
 *
 * UniversalCodeGrep is free software: you can redistribute it and/or modify it under the
 * terms of version 3 of the GNU General Public License as published by the Free
 * Software Foundation.
 *
 * UniversalCodeGrep is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * UniversalCodeGrep.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @file microstring.hpp
 * A std:string-like class for strings short enough to fit into an unsigned literal int type.
 */

#ifndef SRC_LIBEXT_MICROSTRING_HPP_
#define SRC_LIBEXT_MICROSTRING_HPP_

/**
 * Class for very short strings.  Basically a thin facade over a built-in integral type which allows very fast comparisons, copies, and moves.
 */
class microstring
{
public:

	using underlying_storage_type = uint32_t;

	microstring() = default;
	microstring(const std::string &other) : microstring(other.cbegin(), other.cend()) {};
	microstring(std::string::const_iterator b, std::string::const_iterator e) : microstring(&*b, &*e) {};

	microstring(const char * __restrict__ start, const char * __restrict__ end)
	{
		size_t num_chars = end - start;
		if(unlikely(num_chars > sizeof(underlying_storage_type)))
		{
			throw std::length_error("Length too long for a microstring");
		}

		// Put the first character in the MSB, so that microstrings sort the same as a regular string.
		m_storage = 0;
		if(num_chars > 0)
		{
			m_storage = (static_cast<underlying_storage_type>(*start) << (8*3));
		}
		if(num_chars > 1)
		{
			++start;
			m_storage |= (static_cast<underlying_storage_type>(*start) << (8*2));
		}
		if(num_chars > 2)
		{
			++start;
			m_storage |= (static_cast<underlying_storage_type>(*start) << (8*1));
		}
		if(num_chars > 3)
		{
			++start;
			m_storage |= static_cast<underlying_storage_type>(*start);
		}
	}

	~microstring() noexcept = default;

	/**
	 * Return the number of characters in the microstring.
	 *
	 * @todo Redo this function with sse2 or just bit-twiddling instead of strnlen etc.  Not used much at the moment, so this
	 * is ok for now.
	 *
	 * @return Length of string.
	 */
	size_t length() const noexcept
	{
		auto tmp = m_storage;

		// Make sure the bytes of the tmp var are in big-endian order.
		tmp = host_to_be(tmp);

		auto ptr = reinterpret_cast<const char *>(&tmp);

		return strnlen(ptr, 4);
	};

	inline bool operator <(const microstring other) const noexcept { return m_storage < other.m_storage; };

	/// Implicitly convert to a std::string.
	operator std::string() const
	{
		underlying_storage_type tmp = host_to_be(m_storage);
		return std::string(reinterpret_cast<const char *>(&tmp), length());
	};

	underlying_storage_type urep() const noexcept { return m_storage; };

private:
	underlying_storage_type m_storage;  // No member initializer, would make the class non-trivial.
};

#ifdef HAVE_IS_TRIVIAL
static_assert(std::is_trivial<microstring>::value, "microstring is not trivial");
#endif
#ifdef HAVE_IS_TRIVIALLY_COPYABLE // gcc/libstdc++ 4.8.4 doesn't have these for some reason.
static_assert(std::is_trivially_copyable<microstring>::value, "microstring is not trivially copyable");
static_assert(std::is_trivially_copy_constructible<microstring>::value, "microstring is not trivially copy-constructible");
static_assert(std::is_trivially_move_constructible<microstring>::value, "microstring is not trivially move-constructible");
static_assert(std::is_trivially_assignable<microstring, microstring>::value, "microstring is not trivially assignable to itself");
static_assert(std::is_trivially_copy_assignable<microstring>::value, "microstring is not trivially copy-assignable");
static_assert(std::is_trivially_move_assignable<microstring>::value, "microstring is not trivially move-assignable");
#endif
static_assert(sizeof(microstring) == sizeof(uint32_t), "microstring has a different size than its underlying storage");

static_assert(std::is_constructible<microstring, std::string>::value, "microstring is not constructible from std::string");
static_assert(std::is_trivially_destructible<microstring>::value, "microstring is not trivially destructible");

#endif /* SRC_LIBEXT_MICROSTRING_HPP_ */