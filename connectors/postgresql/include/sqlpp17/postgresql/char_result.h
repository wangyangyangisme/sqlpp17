#pragma once

/*
Copyright (c) 2017 - 2018, Roland Bock
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this
   list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <functional>
#include <memory>
#include <optional>
#include <string_view>

#include <libpq-fe.h>

namespace sqlpp::postgresql::detail
{
  struct result_cleanup_t
  {
    auto operator()(PGresult* handle) const -> void
    {
      if (handle)
        PQclear(handle);
    }
  };
  using unique_result_ptr = std::unique_ptr<PGresult, detail::result_cleanup_t>;
}  // namespace sqlpp::postgresql::detail

namespace sqlpp::postgresql
{
  class char_result_t
  {
    detail::unique_result_ptr _handle;
    std::function<void(std::string_view)> _debug;
    int _row_index = -1;
    int _row_count;

  public:
    char_result_t() = default;
    char_result_t(detail::unique_result_ptr handle, std::function<void(std::string_view)> debug)
        : _handle(std::move(handle)), _debug(debug)
    {
      _row_count = PQntuples(_handle.get());
    }

    char_result_t(const char_result_t&) = delete;
    char_result_t(char_result_t&& rhs) = default;
    char_result_t& operator=(const char_result_t&) = delete;
    char_result_t& operator=(char_result_t&&) = default;
    ~char_result_t() = default;

    [[nodiscard]] operator bool() const
    {
      return !!_handle;
    }

    auto* get() const
    {
      return _handle.get();
    }

    auto increase_row_index()
    {
      if (_handle)
        ++_row_index;
    }

    auto get_row_index() const
    {
      return _row_index;
    }

    auto get_row_count() const
    {
      return _row_count;
    }

    auto debug() const
    {
      return _debug;
    }

    auto reset() -> void
    {
      *this = char_result_t{};
    }
  };

  template <typename Row>
  auto get_next_result_row(char_result_t& result, Row& row) -> void
  {
    result.increase_row_index();
    if (result.get_row_index() < result.get_row_count())
    {
      row.bind(result);
    }
    else
    {
      result.reset();
    }
  }

  auto bind_field(char_result_t& result, std::int32_t& value, int index) -> void;
  auto bind_field(char_result_t& result, std::int64_t& value, int index) -> void;
  auto bind_field(char_result_t& result, float& value, int index) -> void;
  auto bind_field(char_result_t& result, double& value, int index) -> void;
  auto bind_field(char_result_t& result, std::string_view& value, int index) -> void;

  auto bind_field(char_result_t& result, std::optional<std::string_view>& value, int index) -> void;

}  // namespace sqlpp::postgresql

