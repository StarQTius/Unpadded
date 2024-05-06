#pragma once

namespace upd::detail {

template<typename It>
class range {
public:
  constexpr range(It begin, It end) : m_begin{begin}, m_end{end} {}

  [[nodiscard]] constexpr auto begin() const -> It { return m_begin; }

  [[nodiscard]] constexpr auto end() const -> It { return m_end; }

private:
  It m_begin;
  It m_end;
};

} // namespace upd::detail
