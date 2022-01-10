#pragma once

namespace REL
{
	namespace detail
	{
		struct optional_byte
		{
			constexpr optional_byte() {}
			constexpr optional_byte(std::nullopt_t) {}
			constexpr optional_byte(std::uint8_t a_value) : has_value(true), value(a_value) {}

			optional_byte& operator=(std::nullopt_t)
			{
				has_value = false;
				return *this;
			}

			optional_byte& operator=(std::uint8_t a_value)
			{
				has_value = true;
				value = a_value;
				return *this;
			}

			bool has_value = false;
			std::uint8_t value = 0;
		};

		template <std::size_t N> requires (N % 3 == 0)
		struct pattern_impl
		{
			static constexpr auto Size = N / 3;

			constexpr pattern_impl(const char(&a_str)[N])
			{
				for (std::size_t i = 0; i < Size; i++) {
					data[i] = parse_byte(a_str[i * 3], a_str[i * 3 + 1]);
				}
			}

			[[nodiscard]] bool match(const void* a_address) const
			{
				for (std::size_t i = 0; i < Size; i++) {
					if (!data[i].has_value) {
						continue;
					}

					if (data[i].value != static_cast<const std::uint8_t*>(a_address)[i]) {
						return false;
					}
				}

				return true;
			}

			[[nodiscard]] static constexpr optional_byte parse_byte(char a_char1, char a_char2)
			{
				if (a_char1 == '?' && a_char2 == '?') {
					return std::nullopt;
				}

				return parse_hex(a_char1) * 0x10 + parse_hex(a_char2);
			}

			[[nodiscard]] static constexpr std::uint8_t parse_hex(char a_char)
			{
				if (a_char >= '0' && a_char <= '9') {
					return a_char - '0';
				}
				else if (a_char >= 'A' && a_char <= 'F') {
					return a_char + 0xA - 'A';
				}
				else if (a_char >= 'a' && a_char <= 'f') {
					return a_char + 0xA - 'a';
				}
			}

			optional_byte data[Size];
		};
	}

	template <detail::pattern_impl Impl>
	struct Pattern
	{
		[[nodiscard]] bool match(std::uintptr_t a_address) const
		{
			return Impl.match(reinterpret_cast<const void*>(a_address));
		}
	};
}
