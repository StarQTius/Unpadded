//! \file
//! \brief Order request storage and processing

#pragma once

#include <cstddef>
#include <type_traits>

#include <upd/type.hpp>

#include "detail/io.hpp"
#include "detail/signature.hpp"
#include "detail/type_traits/require.hpp"
#include "detail/typelist.hpp"
#include "dispatcher.hpp" // IWYU pragma: keep
#include "policy.hpp"

#include "detail/def.hpp"

namespace k2o {
namespace detail {

template<typename Keyring, order_features Order_Features>
using dispatcher_t = dispatcher<Keyring::size, Keyring::endianess, Keyring::signed_mode, Order_Features>;

template<typename Keyring>
using needed_input_buffer_size =
    std::integral_constant<std::size_t,
                           detail::max<detail::map<typename Keyring::signatures_t, detail::parameters_size>>::value +
                               sizeof(typename Keyring::index_t)>;

template<typename Keyring>
using needed_output_buffer_size = detail::max<detail::map<typename Keyring::signatures_t, detail::return_type_size>>;

}; // namespace detail

//! \brief Dispatcher with input / output storage
//! \details
//!   Instances of this class may store input and output byte sequences as they are received / sent. This allows the
//!   user to load and unload the dispatcher byte after byte, whereas plain dispatchers cannot buffer their input and
//!   ouput, therefore they must receive and send byte sequences all at once. Buffered dispatchers do not own their
//!   buffers directly. They must be provided through iterators. A buffered dispatcher state goes through the following
//!   states:
//!     -# The input buffer is empty, ready to load an order request in the input buffer.
//!     -# Once a full order request has been received, it is immediately fulfilled and the result is written in the
//!     output buffer. The input buffer is reset, thus it may receive a new request while the output buffer is unloaded.
//!     -# Once the output buffer is empty, it may be written again.
//!   \note Input buffer reset is soft, in other word, its content is not erased. As a result, it is possible to use a
//!   single buffer as input and output as long as byte sequence reading and writting does not occur at the same time.
//!   For that purpose, 'is_loaded' will indicate whether the output buffer is empty or not.
//! \tparam Dispatcher Number of stored orders
//! \tparam Input_Iterator Type of the iterator to the input buffer
//! \tparam Output_Iterator Type of the iterator to the output buffer
template<typename Dispatcher, typename Input_Iterator, typename Output_Iterator>
class buffered_dispatcher
    : public detail::reader<buffered_dispatcher<Dispatcher, Input_Iterator, Output_Iterator>, void>,
      public detail::writer<buffered_dispatcher<Dispatcher, Input_Iterator, Output_Iterator>> {
  using this_t = buffered_dispatcher<Dispatcher, Input_Iterator, Output_Iterator>;

public:
  //! \copydoc dispatcher::index_t
  using index_t = typename Dispatcher::index_t;

  //! \brief Initialize the underlying plain dispatcher with a keyring and hold iterators to the buffers
  //! \tparam Keyring Type of the keyring
  //! \param input_it Start of the input buffer
  //! \param output_it Start of the output buffer
  template<typename Keyring, order_features Order_Features, REQUIREMENT(is_keyring, Keyring)>
  buffered_dispatcher(Keyring, Input_Iterator input_it, Output_Iterator output_it, order_features_h<Order_Features>)
      : m_dispatcher{Keyring{}, order_features_h<Order_Features>{}}, m_is_index_loaded{false},
        m_load_count{sizeof(index_t)}, m_ibuf_begin{input_it}, m_ibuf_next{input_it}, m_obuf_begin{output_it},
        m_obuf_next{output_it}, m_obuf_bottom{output_it} {}

  //! \brief Indicates whether the output buffer contains data to send
  //! \return true if and only if the next call to 'write' or 'write_all' will have a visible effect
  bool is_loaded() const { return m_obuf_next != m_obuf_bottom; }

  using detail::immediate_reader<this_t, void>::read_all;

  //! \brief Put bytes into the input buffer until a full order request is stored
  //! \copydoc ImmediateReader_CRTP
  //! \param src_ftor Input functor to a byte sequence
  template<typename Src_F, REQUIREMENT(input_invocable, Src_F)>
  void read_all(Src_F &&src_ftor) {
    while (!m_is_index_loaded)
      read(src_ftor);
    while (m_is_index_loaded)
      read(src_ftor);
  }

  using detail::reader<this_t, void>::read;

  //! \brief Put one byte into the input buffer
  //! \copydoc Reader_CRTP
  //! \param src_ftor Input functor to a byte sequence
  template<typename Src_F, REQUIREMENT(input_invocable, Src_F)>
  void read(Src_F &&src_ftor) {
    *m_ibuf_next++ = FWD(src_ftor)();
    if (--m_load_count == 0) {
      auto ibuf_it = m_ibuf_begin;
      auto get_index_byte = [&]() { return *ibuf_it++; };
      if (m_is_index_loaded) {
        m_obuf_bottom = m_obuf_begin;
        m_obuf_next = m_obuf_begin;

        auto index = get_index(get_index_byte);
        if (index < m_dispatcher.size) {
          m_dispatcher[index]([&]() { return *ibuf_it++; }, [&](upd::byte_t byte) { *m_obuf_bottom++ = byte; });
        }

        m_is_index_loaded = false;
        m_load_count = sizeof(index_t);
        m_ibuf_next = m_ibuf_begin;
      } else {
        auto index = get_index(get_index_byte);
        if (index < m_dispatcher.size) {
          m_is_index_loaded = true;
          m_load_count = m_dispatcher[index].input_size();
        }
      }
    }
  }

  using detail::immediate_writer<this_t>::write_all;

  //! \brief Completely output the output buffer content
  //! \copydoc ImmediateWriter_CRTP
  //! \param dest_ftor Output functor for writing byte sequences
  template<typename Dest_F, REQUIREMENT(output_invocable, Dest_F)>
  void write_all(Dest_F &&dest_ftor) {
    while (is_loaded())
      write(dest_ftor);
  }

  using detail::writer<this_t>::write;

  //! \brief Output one byte from the output buffer
  //! \copydoc Writer_CRTP
  //! \param dest_ftor Output functor for writing byte sequences
  template<typename Dest_F, REQUIREMENT(output_invocable, Dest_F)>
  void write(Dest_F &&dest_ftor) {
    if (is_loaded())
      FWD(dest_ftor)(*m_obuf_next++);
  }

  //! \name replace
  //!
  //! Forward the arguments to \mgref{Dispatcher_Replace, dispatcher::replace}
  //! @{

  template<typename T>
  void replace(index_t index, T &&x) {
    m_dispatcher.replace(index, FWD(x));
  }

#if __cplusplus >= 201703L
  template<auto &Ftor>
  void replace(index_t index) {
    m_dispatcher.replace<Ftor>(index);
  }
#endif // __cplusplus >= 201703L

  //! @}

private:
  //! \copydoc dispatcher::get_index
  template<typename Src_F>
  index_t get_index(Src_F &&fetch_byte) const {
    return m_dispatcher.get_index(FWD(fetch_byte));
  }

  Dispatcher m_dispatcher;
  bool m_is_index_loaded;
  std::size_t m_load_count;
  Input_Iterator m_ibuf_begin, m_ibuf_next;
  Output_Iterator m_obuf_begin, m_obuf_next, m_obuf_bottom;
};

template<typename Keyring, typename Input_Iterator, typename Output_Iterator, order_features Order_Features>
buffered_dispatcher<detail::dispatcher_t<Keyring, Order_Features>, Input_Iterator, Output_Iterator>
make_buffered_dispatcher(Keyring,
                         Input_Iterator input_it,
                         Output_Iterator output_it,
                         order_features_h<Order_Features>) {
  return buffered_dispatcher<detail::dispatcher_t<Keyring, Order_Features>, Input_Iterator, Output_Iterator>(
      Keyring{}, input_it, output_it);
}

#if __cplusplus >= 201703L

template<typename Keyring, typename Input_Iterator, typename Output_Iterator, order_features Order_Features>
buffered_dispatcher(Keyring, Input_Iterator, Output_Iterator, order_features_h<Order_Features>)
    -> buffered_dispatcher<detail::dispatcher_t<Keyring, Order_Features>, Input_Iterator, Output_Iterator>;

#endif // __cplusplus >= 201703L

template<typename Dispatcher, std::size_t Buffer_Size>
class single_buffered_dispatcher : public buffered_dispatcher<Dispatcher, upd::byte_t *, upd::byte_t *> {
  using base_t = buffered_dispatcher<Dispatcher, upd::byte_t *, upd::byte_t *>;

public:
  constexpr static auto buffer_size = Buffer_Size;

  template<typename Keyring, order_features Order_Features>
  explicit single_buffered_dispatcher(Keyring, order_features_h<Order_Features>)
      : base_t{Keyring{}, m_buf, m_buf, order_features_h<Order_Features>{}} {}

private:
  upd::byte_t m_buf[buffer_size];
};

#if __cplusplus >= 201703L
template<typename Keyring, order_features Order_Features>
single_buffered_dispatcher(Keyring, order_features_h<Order_Features>) -> single_buffered_dispatcher<
    detail::dispatcher_t<Keyring, Order_Features>,
    detail::max_p<detail::needed_input_buffer_size<Keyring>, detail::needed_output_buffer_size<Keyring>>::value>;
#endif // __cplusplus >= 201703L

template<typename Keyring, order_features Order_Features>
single_buffered_dispatcher<
    detail::dispatcher_t<Keyring, Order_Features>,
    detail::max_p<detail::needed_input_buffer_size<Keyring>, detail::needed_output_buffer_size<Keyring>>::value>
make_single_buffered_dispatcher(Keyring, order_features_h<Order_Features>) {
  return single_buffered_dispatcher<
      detail::dispatcher_t<Keyring, Order_Features>,
      detail::max_p<detail::needed_input_buffer_size<Keyring>, detail::needed_output_buffer_size<Keyring>>::value>{
      Keyring{}, order_features_h<Order_Features>{}};
}

template<typename Dispatcher, std::size_t Input_Buffer_Size, std::size_t Output_Buffer_Size>
class double_buffered_dispatcher : public buffered_dispatcher<Dispatcher, upd::byte_t *, upd::byte_t *> {
  using base_t = buffered_dispatcher<Dispatcher, upd::byte_t *, upd::byte_t *>;

public:
  constexpr static auto input_buffer_size = Input_Buffer_Size;
  constexpr static auto output_buffer_size = Output_Buffer_Size;

  template<typename Keyring, order_features Order_Features>
  explicit double_buffered_dispatcher(Keyring, order_features_h<Order_Features>)
      : base_t{Keyring{}, m_ibuf, m_obuf, order_features_h<Order_Features>{}} {}

private:
  upd::byte_t m_ibuf[input_buffer_size], m_obuf[output_buffer_size];
};

#if __cplusplus >= 201703L
template<typename Keyring, order_features Order_Features>
double_buffered_dispatcher(Keyring, order_features_h<Order_Features>)
    -> double_buffered_dispatcher<detail::dispatcher_t<Keyring, Order_Features>,
                                  detail::needed_input_buffer_size<Keyring>::value,
                                  detail::needed_output_buffer_size<Keyring>::value>;
#endif // __cplusplus >= 201703L

template<typename Keyring, order_features Order_Features>
double_buffered_dispatcher<detail::dispatcher_t<Keyring, Order_Features>,
                           detail::needed_input_buffer_size<Keyring>::value,
                           detail::needed_output_buffer_size<Keyring>::value>
make_double_buffered_dispatcher(Keyring, order_features_h<Order_Features>) {
  return double_buffered_dispatcher<detail::dispatcher_t<Keyring, Order_Features>,
                                    detail::needed_input_buffer_size<Keyring>::value,
                                    detail::needed_output_buffer_size<Keyring>::value>{
      Keyring{}, order_features_h<Order_Features>{}};
}

} // namespace k2o

#include "detail/undef.hpp" // IWYU pragma: keep
