#pragma once

/*
Copyright (c) 2016, Roland Bock
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

#include <tuple>
#include <variant>
#include <sqlpp17/all.h>
#include <sqlpp17/selected_fields/selected_fields.h>
#include <sqlpp17/statement.h>

namespace sqlpp
{
  SQLPP_WRAPPED_STATIC_ASSERT(assert_selected_fields_args_not_empty,
                              "selected_fields() must be called with at least one argument");
  SQLPP_WRAPPED_STATIC_ASSERT(assert_selected_fields_args_are_selectable,
                              "selected_fields() args must be selectable (i.e. named expressions)");
  SQLPP_WRAPPED_STATIC_ASSERT(assert_selected_fields_args_have_unique_names,
                              "selected_fields() args must have unique names");

  template <typename... T>
  constexpr auto check_selected_fields_arg(const T&...)
  {
    if
      constexpr(sizeof...(T) == 0)
      {
        return failed<assert_selected_fields_args_not_empty>{};
      }
    else if
      constexpr(!all<is_selectable_v<T>...>)
      {
        return failed<assert_selected_fields_args_are_selectable>{};
      }
    else if
      constexpr(type_set<char_sequence_of_t<T>...>().size() == sizeof...(T))
      {
        return failed<assert_selected_fields_args_have_unique_names>{};
      }
    else
      return succeeded{};
  }

  struct no_selected_fields_t
  {
  };

  template <typename Statement>
  class clause_base<no_selected_fields_t, Statement>
  {
  public:
    template <typename... Fields>
    [[nodiscard]] constexpr auto selected_fields(Fields... fields) const
    {
      constexpr auto check = check_selected_fields_arg(fields...);
      if
        constexpr(check)
        {
          return Statement::of(this).template replace_clause<no_selected_fields_t>(
              selected_fields_t<std::tuple<Fields...>>{std::make_tuple(fields...)});
        }
      else
      {
        return ::sqlpp::bad_statement_t<std::decay_t<decltype(check)>>{};
      }
    }

    template <typename... Fields>
    [[nodiscard]] constexpr auto selected_fields(std::tuple<Fields...> fields) const
    {
      constexpr auto check = check_selected_fields_arg(std::declval<Fields>()...);
      if
        constexpr(check)
        {
          return Statement::of(this).template replace_clause<no_selected_fields_t>(
              selected_fields_t<std::tuple<Fields...>>{fields});
        }
      else
      {
        return ::sqlpp::bad_statement_t<std::decay_t<decltype(check)>>{};
      }
    }

    template <typename... Fields>
    [[nodiscard]] constexpr auto selected_fields(std::vector<std::variant<Fields...>> fields) const
    {
      constexpr auto check = check_selected_fields_arg(std::declval<Fields>()...);
      if
        constexpr(check)
        {
          return Statement::of(this).template replace_clause<no_selected_fields_t>(selected_fields_t<Table>{t});
        }
      else
      {
        return ::sqlpp::bad_statement_t<std::decay_t<decltype(check)>>{};
      }
    }
  };

  template <typename Context, typename Statement>
  class interpreter_t<Context, clause_base<no_selected_fields_t, Statement>>
  {
    using T = clause_base<no_selected_fields_t, Statement>;

    static Context& _(const T&, Context& context)
    {
      return context;
    }
  };

  template <typename... Fields>
  [[nodiscard]] constexpr auto selected_fields(Fields&&... fields)
  {
    return statement<no_selected_fields_t>{}.selected_fields(std::forward<Fields>(fields)...);
  }
}
