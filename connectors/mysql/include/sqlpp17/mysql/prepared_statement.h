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

#include <memory>
#include <vector>

#include <mysql.h>

namespace sqlpp ::mysql
{
  class prepared_statement_t
  {
    std::unique_ptr<MYSQL_STMT, void (*)(MYSQL_STMT*)> _handle;
    std::size_t _number_of_parameters;
    std::size_t _number_of_columns;
    std::vector<MYSQL_BIND> _bind;
    std::function<void(std::string_view)> _debug;

  public:
    prepared_statement_t() = default;
    prepared_statement_t(std::unique_ptr<MYSQL_STMT, void (*)(MYSQL_STMT*)>&& handle,
                         std::size_t number_of_parameters,
                         std::size_t number_of_columns,
                         std::function<void(std::string_view)> debug)
        : _handle(std::move(handle)),
          _number_of_parameters(number_of_parameters),
          _number_of_columns(number_of_columns),
          _debug(debug)
    {
    }
    prepared_statement_t(const prepared_statement_t&) = delete;
    prepared_statement_t(prepared_statement_t&& rhs) = default;
    prepared_statement_t& operator=(const prepared_statement_t&) = delete;
    prepared_statement_t& operator=(prepared_statement_t&&) = default;
    ~prepared_statement_t() = default;

    auto* get() const
    {
      return _handle.get();
    }

    auto* get_bind_data()
    {
      return _bind.data();
    }

    auto debug() const
    {
      return _debug;
    }
  };
}  // namespace sqlpp::mysql
