#pragma once

/*
Copyright (c) 2017, Roland Bock
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

#ifdef SQLPP_USE_SQLCIPHER
#include <sqlcipher/sqlite3.h>
#else
#include <sqlite3.h>
#endif

#include <sqlpp17/sqlite3/prepared_statement.h>

namespace sqlpp::sqlite3
{
  class prepared_statement_result_t;
}  // namespace sqlpp::sqlite3

namespace sqlpp::sqlite3::detail
{
  auto get_next_result_row(prepared_statement_result_t& result) -> bool;
}  // namespace sqlpp::sqlite3::detail

namespace sqlpp::sqlite3
{
  class direct_execution_result_t;

  class prepared_statement_result_t
  {
    ::sqlite3_stmt* _handle;
    std::function<void(std::string_view)> _debug;

  public:
    prepared_statement_result_t() = default;
    prepared_statement_result_t(prepared_statement_t& prepared_statement)
        : _handle(prepared_statement.get()), _debug(prepared_statement.debug())
    {
    }
    prepared_statement_result_t(const prepared_statement_result_t&) = delete;
    prepared_statement_result_t(prepared_statement_result_t&& rhs) = default;
    prepared_statement_result_t(direct_execution_result_t&& rhs) = delete;
    prepared_statement_result_t& operator=(const prepared_statement_result_t&) = delete;
    prepared_statement_result_t& operator=(prepared_statement_result_t&&) = default;
    prepared_statement_result_t& operator=(direct_execution_result_t&&) = delete;
    ~prepared_statement_result_t(){}

        [[nodiscard]]
        operator bool() const
    {
      return !!_handle;
    }

    [[nodiscard]] auto* get() const
    {
      return _handle;
    }

    [[nodiscard]] auto debug() const
    {
      return _debug;
    }

    auto reset() -> void
    {
      *this = prepared_statement_result_t{};
    }
  };

  template <typename Row>
  auto get_next_result_row(prepared_statement_result_t& result, Row& row) -> void
  {
    if (detail::get_next_result_row(result))
    {
      row.post_bind(result);
    }
    else
    {
      result.reset();
    }
  }

  auto post_bind_field(prepared_statement_result_t& result, bool& value, int index) -> void;
  auto post_bind_field(prepared_statement_result_t& result, std::int32_t& value, int index) -> void;
  auto post_bind_field(prepared_statement_result_t& result, std::int64_t& value, int index) -> void;
  auto post_bind_field(prepared_statement_result_t& result, float& value, int index) -> void;
  auto post_bind_field(prepared_statement_result_t& result, double& value, int index) -> void;
  auto post_bind_field(prepared_statement_result_t& result, std::string_view& value, int index) -> void;

  template <typename T>
  auto post_bind_field(prepared_statement_result_t& result, std::optional<T>& value, int index) -> void
  {
    if (result.debug())
      result.debug()("Binding optional result at index " + std::to_string(index));

    if (sqlite3_column_type(result.get(), index) == SQLITE_NULL)
    {
      value.reset();
    }
    else
    {
      value = T{};
      post_bind_field(result, *value, index);
    }
  }
}  // namespace sqlpp::sqlite3
