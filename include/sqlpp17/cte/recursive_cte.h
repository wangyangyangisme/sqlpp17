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

#include <sqlpp17/clause_fwd.h>
#include <sqlpp17/column.h>
#include <sqlpp17/member.h>
#include <sqlpp17/result_row.h>
#include <sqlpp17/table_spec.h>
#include <sqlpp17/type_traits.h>
#include <sqlpp17/wrapped_static_assert.h>

namespace sqlpp
{
  template <typename Cte, typename ResultRow>
  struct cte_columns_t
  {
    static_assert(wrong<cte_columns_t>, "Invalid arguments for cte_columns_t");
  };

  template <typename Cte, typename... ResultColumnSpecs>
  struct cte_columns_t<Cte, result_row_t<ResultColumnSpecs...>>
      : member_t<ResultColumnSpecs, column_t<table_spec<name_tag_of_t<Cte>, type_hash<Cte>()>, ResultColumnSpecs>>...
  {
  };

  template <typename Cte, typename... ResultColumnSpecs>
  [[nodiscard]] constexpr auto all_of(const cte_columns_t<Cte, ResultColumnSpecs...>& t)
  {
#warning : For sake of completeness, it might be good to provide overloads for cte and recursive_cte, too
    return multi_column_t{column_t<table_spec<name_tag_of_t<Cte>, type_hash<Cte>()>, ResultColumnSpecs>{}...};
  }

#warning : Maybe merge cte and recursive_cte, check in union calls if it already is recursive. Duplicating all functions for both versions of CTE is error prone
  template <typename NameTag, typename Statement>
  struct recursive_cte_t : cte_columns_t<recursive_cte_t<NameTag, Statement>, result_row_of_t<Statement>>
  {
    Statement _statement;

    recursive_cte_t(NameTag, Statement statement) : _statement(statement)
    {
    }
  };

  template <typename NameTag, typename Statement>
  [[nodiscard]] constexpr auto provided_tables_of([[maybe_unused]] type_t<recursive_cte_t<NameTag, Statement>>)
  {
    return type_set<table_spec<NameTag, type_hash<recursive_cte_t<NameTag, Statement>>()>>();
  }

  template <typename NameTag, typename Statement>
  [[nodiscard]] constexpr auto required_tables_of([[maybe_unused]] type_t<recursive_cte_t<NameTag, Statement>>)
  {
    return type_set<>();
  }

  template <typename NameTag, typename Statement>
  struct nodes_of<recursive_cte_t<NameTag, Statement>>
  {
    using type = type_vector<Statement>;
  };

  template <typename NameTag, typename Statement>
  struct name_tag_of<recursive_cte_t<NameTag, Statement>>
  {
    using type = NameTag;
  };

  template <typename NameTag, typename Statement>
  constexpr auto is_table_v<recursive_cte_t<NameTag, Statement>> = true;

  template <typename DbConnection, typename NameTag, typename Statement>
  [[nodiscard]] auto to_full_sql_string(const DbConnection& connection, const recursive_cte_t<NameTag, Statement>& t)
  {
    return to_sql_name(connection, t) + " AS (" + to_sql_string(connection, t._statement) + ")";
  }

  template <typename DbConnection, typename NameTag, typename Statement>
  [[nodiscard]] auto to_sql_string(const DbConnection& connection, const recursive_cte_t<NameTag, Statement>& t)
  {
    return to_sql_name(connection, t);
  }
}  // namespace sqlpp
