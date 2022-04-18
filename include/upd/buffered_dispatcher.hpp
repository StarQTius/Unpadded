//! \file

#pragma once

#include <cstddef>
#include <type_traits>

#include "type.hpp"

#include "detail/io/immediate_reader.hpp"
#include "detail/io/immediate_writer.hpp"
#include "detail/io/reader.hpp"
#include "detail/io/writer.hpp"
#include "detail/static_error.hpp"
#include "detail/type_traits/is_byte_iterator.hpp"
#include "detail/type_traits/require.hpp"
#include "detail/type_traits/signature.hpp"
#include "detail/type_traits/typelist.hpp"
#include "dispatcher.hpp" // IWYU pragma: keep
#include "policy.hpp"
#include "unevaluated.hpp"

#include "detail/def.hpp"

namespace upd {
namespace detail {

template<typename Keyring>
using needed_input_buffer_size =
    std::integral_constant<std::size_t,
                           detail::max<detail::map<typename Keyring::signatures_t, detail::parameters_size>>::value +
                               sizeof(typename Keyring::index_t)>;

template<typename Keyring>
using needed_output_buffer_size = detail::max<detail::map<typename Keyring::signatures_t, detail::return_type_size>>;

}; // namespace detail

//! \brief Enumerates the possible status of a loading packet
//!
//! - `LOADING_PACKET`: The packet is currently being loaded and is not yet complete
//! - `DROPPED_PACKET`: The packet loading has been canceled before completion
//! - `RESOLVED_PACKET`: The packet loading has been completed and the corresponding action called
//!
enum class packet_status { LOADING_PACKET, DROPPED_PACKET, RESOLVED_PACKET };

//! \brief Dispatcher with input / output storage
//!
//! Instances of this class may store input and output byte sequences as they are received / sent. This allows the
//! user to load and unload the dispatcher byte after byte, whereas plain dispatchers cannot buffer their input and
//! ouput, therefore they must receive and send byte sequences all at once. Buffered dispatchers do not own their
//! buffers directly. They must be provided through iterators. A buffered dispatcher state goes through the following
//! states:
//!   -# The input buffer is empty, ready to load an action request in the input buffer.
//!   -# Once a full action request has been received, it is immediately fulfilled and the result is written in the
//!   output buffer. The input buffer is reset, thus it may receive a new request while the output buffer is unloaded.
//!   -# Once the output buffer is empty, it may be written again.
//! \note Input buffer reset is soft, in other word, its content is not erased. As a result, it is possible to use a
//! single buffer as input and output as long as byte sequence reading and writting does not occur at the same time.
//! For that purpose, `is_loaded` will indicate whether the output buffer is empty or not.
//!
//! \tparam Dispatcher Number of stored actions
//! \tparam Input_Iterator Type of the iterator to the input buffer
//! \tparam Output_Iterator Type of the iterator to the output buffer
template<typename Dispatcher, typename Input_Iterator, typename Output_Iterator>
class buffered_dispatcher
    : public detail::reader<buffered_dispatcher<Dispatcher, Input_Iterator, Output_Iterator>, packet_status>,
      public detail::writer<buffered_dispatcher<Dispatcher, Input_Iterator, Output_Iterator>> {
  using this_t = buffered_dispatcher<Dispatcher, Input_Iterator, Output_Iterator>;

  static_assert(detail::is_byte_iterator<Input_Iterator>::value, K2O_ERROR_NOT_BYTE_ITERATOR(Input_Iterator));
  static_assert(detail::is_byte_iterator<Output_Iterator>::value, K2O_ERROR_NOT_BYTE_ITERATOR(Output_Iterator));

public:
  //! \copydoc dispatcher::index_t
  using index_t = typename Dispatcher::index_t;

  //! \brief Initialize the underlying plain dispatcher with a keyring and hold iterators to the buffers
  //! \tparam Keyring Type of the keyring
  //! \param input_it Start of the input buffer
  //! \param output_it Start of the output buffer
  template<typename Keyring, action_features Action_Features>
  buffered_dispatcher(Keyring, Input_Iterator input_it, Output_Iterator output_it, action_features_h<Action_Features>)
      : m_dispatcher{Keyring{}, action_features_h<Action_Features>{}}, m_is_index_loaded{false},
        m_load_count{sizeof(index_t)}, m_ibuf_begin{input_it}, m_ibuf_next{input_it}, m_obuf_begin{output_it},
        m_obuf_next{output_it}, m_obuf_bottom{output_it} {}

  //! \brief Indicates whether the output buffer contains data to send
  //! \return true if and only if the next call to `write` or `write_all` will have a visible effect
  bool is_loaded() const { return m_obuf_next != m_obuf_bottom; }

  using detail::immediate_reader<this_t, packet_status>::read_all;

  //! \brief Put bytes into the input buffer until a full action request is stored
  //! \copydoc ImmediateReader_CRTP
  //! \param src Input functor to a byte sequence
  //! \return one of the following :
  //!   - `DROPPED_PACKET`: the received index was invalid and the input buffer content was therefore discarded
  //!   - `RESOLVED_PACKET`: the packet was fully loaded and the associated action has been called (the input buffer is
  //!   empty and the output buffer contains the result of the action invocation)
  template<typename Src, REQUIREMENT(input_invocable, Src)>
  packet_status read_all(Src &&src) {
    packet_status status = packet_status::LOADING_PACKET;
    while (!m_is_index_loaded && status == packet_status::LOADING_PACKET)
      status = read(src);
    while (m_is_index_loaded)
      status = read(src);
    return status;
  }

  K2O_SFINAE_FAILURE_MEMBER(read_all, K2O_ERROR_NOT_INPUT(src))

  using detail::reader<this_t, packet_status>::read;

  //! \brief Put one byte into the input buffer
  //! \copydoc Reader_CRTP
  //! \param src Input functor to a byte sequence
  //! \return one of the following :
  //!   - `LOADING_PACKET`: the packet is not yet fully loaded
  //!   - `DROPPED_PACKET`: the received index was invalid and the input buffer content was therefore discarded
  //!   - `RESOLVED_PACKET`: the packet was fully loaded and the associated action has been called (the input buffer is
  //!   empty and the output buffer contains the result of the action invocation)
  template<typename Src, REQUIREMENT(input_invocable, Src)>
  packet_status read(Src &&src) {
    *m_ibuf_next++ = FWD(src)();

    if (--m_load_count > 0)
      return packet_status::LOADING_PACKET;

    if (m_is_index_loaded) {
      call();
      return packet_status::RESOLVED_PACKET;
    } else {
      auto ibuf_it = m_ibuf_begin;
      auto index = get_index([&]() { return *ibuf_it++; });
      if (index < m_dispatcher.size) {
        m_load_count = m_dispatcher[index].input_size();
        m_is_index_loaded = true;

        if (m_load_count == 0) {
          call();
          return packet_status::RESOLVED_PACKET;
        } else {
          return packet_status::LOADING_PACKET;
        }
      } else {
        m_load_count = sizeof(index_t);
        m_ibuf_next = m_ibuf_begin;
        return packet_status::DROPPED_PACKET;
      }
    }
  }

  K2O_SFINAE_FAILURE_MEMBER(read, K2O_ERROR_NOT_INPUT(src))

  using detail::immediate_writer<this_t>::write_all;

  //! \brief Completely output the output buffer content
  //! \copydoc ImmediateWriter_CRTP
  //! \param dest Output functor for writing byte sequences
  template<typename Dest, REQUIREMENT(output_invocable, Dest)>
  void write_all(Dest &&dest) {
    while (is_loaded())
      write(dest);
  }

  K2O_SFINAE_FAILURE_MEMBER(write_all, K2O_ERROR_NOT_OUTPUT(dest))

  using detail::writer<this_t>::write;

  //! \brief Output one byte from the output buffer
  //! \copydoc Writer_CRTP
  //! \param dest Output functor for writing byte sequences
  template<typename Dest, REQUIREMENT(output_invocable, Dest)>
  void write(Dest &&dest) {
    if (is_loaded())
      FWD(dest)(*m_obuf_next++);
  }

  K2O_SFINAE_FAILURE_MEMBER(write, K2O_ERROR_NOT_OUTPUT(dest))

  //! \copydoc dispatcher::replace(unevaluated<F,Ftor>)
  template<index_t Index, typename F, F Ftor>
  void replace(unevaluated<F, Ftor>) {
    m_dispatcher.template replace<Index>(unevaluated<F, Ftor>{});
  }

#if __cplusplus >= 201703L
  //! \copydoc dispatcher::replace()
  template<index_t Index, auto &Ftor>
  void replace() {
    m_dispatcher.template replace<Index, Ftor>();
  }
#endif // __cplusplus >= 201703L

  //! \copydoc dispatcher::replace(F&&)
  template<index_t Index, typename F>
  void replace(F &&ftor) {
    m_dispatcher.template replace<Index>(FWD(ftor));
  }

private:
  //! \brief Provided that the input buffer does contain a full action request, invoke the corresponding action
  //! \warning If the input buffer does not contain a valid action request, the behavior is undefined.
  void call() {
    m_obuf_bottom = m_obuf_begin;
    m_obuf_next = m_obuf_begin;

    auto ibuf_it = m_ibuf_begin;
    auto index = get_index([&]() { return *ibuf_it++; });
    m_dispatcher[index]([&]() { return *ibuf_it++; }, [&](byte_t byte) { *m_obuf_bottom++ = byte; });

    m_is_index_loaded = false;
    m_load_count = sizeof(index_t);
    m_ibuf_next = m_ibuf_begin;
  }

  //! \copydoc dispatcher::get_index
  template<typename Src>
  index_t get_index(Src &&fetch_byte) const {
    return m_dispatcher.get_index(FWD(fetch_byte));
  }

  Dispatcher m_dispatcher;
  bool m_is_index_loaded;
  std::size_t m_load_count;
  Input_Iterator m_ibuf_begin, m_ibuf_next;
  Output_Iterator m_obuf_begin, m_obuf_next, m_obuf_bottom;
};

//! \brief Make a buffered dispatcher
//! \related buffered_dispatcher
template<typename Keyring, typename Input_Iterator, typename Output_Iterator, action_features Action_Features>
buffered_dispatcher<dispatcher<Keyring, Action_Features>, Input_Iterator, Output_Iterator> make_buffered_dispatcher(
    Keyring, Input_Iterator input_it, Output_Iterator output_it, action_features_h<Action_Features>) {
  return buffered_dispatcher<dispatcher<Keyring, Action_Features>, Input_Iterator, Output_Iterator>(
      Keyring{}, input_it, output_it, action_features_h<Action_Features>{});
}

#if __cplusplus >= 201703L

template<typename Keyring, typename Input_Iterator, typename Output_Iterator, action_features Action_Features>
buffered_dispatcher(Keyring, Input_Iterator, Output_Iterator, action_features_h<Action_Features>)
    -> buffered_dispatcher<dispatcher<Keyring, Action_Features>, Input_Iterator, Output_Iterator>;

#endif // __cplusplus >= 201703L

//! \brief Implements a dispatcher using a single buffer for input and output
//!
//! The buffer is allocated statically as a plain array. If created using CTAD or `make_single_buffered_dispatcher`, its
//! size is as small as possible for holding any action request and any action response.
//! \warning It is not possible to read a request and write a response at the same time. If you need that, use
//! `double_buffered_dispatcher` instead.
//!
//! \tparam Dispatcher Underlying dispatcher type
//! \tparam Buffer_Size Size of the internal buffer
template<typename Dispatcher, std::size_t Buffer_Size>
class single_buffered_dispatcher : public buffered_dispatcher<Dispatcher, byte_t *, byte_t *> {
  using base_t = buffered_dispatcher<Dispatcher, byte_t *, byte_t *>;

public:
  //! \brief Equals the `Buffer_Size` template parameter
  constexpr static auto buffer_size = Buffer_Size;

  //! \brief Initialize the underlying dispatcher
  //!
  //! \tparam Keyring Keyring which holds the actions to be managed by the dispatcher
  //! \tparam Action_Features Features of the actions managed by the dispatcher
  template<typename Keyring, action_features Action_Features>
  explicit single_buffered_dispatcher(Keyring, action_features_h<Action_Features>)
      : base_t{Keyring{}, m_buf, m_buf, action_features_h<Action_Features>{}} {}

private:
  byte_t m_buf[buffer_size];
};

#if __cplusplus >= 201703L
template<typename Keyring, action_features Action_Features>
single_buffered_dispatcher(Keyring, action_features_h<Action_Features>) -> single_buffered_dispatcher<
    dispatcher<Keyring, Action_Features>,
    detail::max_p<detail::needed_input_buffer_size<Keyring>, detail::needed_output_buffer_size<Keyring>>::value>;
#endif // __cplusplus >= 201703L

//! \brief Make a single buffered dispatcher
//! \related single_buffered_dispatcher
#if defined(DOXYGEN)
template<typename Keyring, action_features Action_Features>
auto make_single_buffered_dispatcher(Keyring, action_features_h<Action_Features>);
#else  // defined(DOXYGEN)
template<typename Keyring, action_features Action_Features>
single_buffered_dispatcher<
    dispatcher<Keyring, Action_Features>,
    detail::max_p<detail::needed_input_buffer_size<Keyring>, detail::needed_output_buffer_size<Keyring>>::value>
make_single_buffered_dispatcher(Keyring, action_features_h<Action_Features>) {
  return single_buffered_dispatcher<
      dispatcher<Keyring, Action_Features>,
      detail::max_p<detail::needed_input_buffer_size<Keyring>, detail::needed_output_buffer_size<Keyring>>::value>{
      Keyring{}, action_features_h<Action_Features>{}};
}
#endif // defined(DOXYGEN)

//! \brief Implements a dispatcher using separate buffers for input and output
//!
//! The buffers are allocated statically as plain arrays. If created using CTAD or `make_double_buffered_dispatcher`,
//! their sizes are as small as possible for holding any action request and any action response. \note If you would
//! rather having a single buffer and do not mind reading and writing at different moment, consider using
//! `single_buffered_dispatcher` instead.
//!
//! \tparam Dispatcher Underlying dispatcher type
//! \tparam Input_Buffer_Size Size of the input internal buffer
//! \tparam Output_Buffer_Size Size of the output internal buffer
template<typename Dispatcher, std::size_t Input_Buffer_Size, std::size_t Output_Buffer_Size>
class double_buffered_dispatcher : public buffered_dispatcher<Dispatcher, byte_t *, byte_t *> {
  using base_t = buffered_dispatcher<Dispatcher, byte_t *, byte_t *>;

public:
  //! \brief Equals the `Input_Buffer_Size` template parameter
  constexpr static auto input_buffer_size = Input_Buffer_Size;

  //! \brief Equals the `Output_Buffer_Size` template parameter
  constexpr static auto output_buffer_size = Output_Buffer_Size;

  //! \brief Initialize the underlying dispatcher
  //!
  //! \tparam Keyring Keyring which holds the actions to be managed by the dispatcher
  //! \tparam Action_Features Features of the actions managed by the dispatcher
  template<typename Keyring, action_features Action_Features>
  explicit double_buffered_dispatcher(Keyring, action_features_h<Action_Features>)
      : base_t{Keyring{}, m_ibuf, m_obuf, action_features_h<Action_Features>{}} {}

private:
  byte_t m_ibuf[input_buffer_size], m_obuf[output_buffer_size];
};

#if __cplusplus >= 201703L
template<typename Keyring, action_features Action_Features>
double_buffered_dispatcher(Keyring, action_features_h<Action_Features>)
    -> double_buffered_dispatcher<dispatcher<Keyring, Action_Features>,
                                  detail::needed_input_buffer_size<Keyring>::value,
                                  detail::needed_output_buffer_size<Keyring>::value>;
#endif // __cplusplus >= 201703L

//! \brief Make a double buffered dispatcher
//! \related double_buffered_dispatcher
#if defined(DOXYGEN)
template<typename Keyring, action_features Action_Features>
auto make_double_buffered_dispatcher(Keyring, action_features_h<Action_Features>);
#else  // defined(DOXYGEN)
template<typename Keyring, action_features Action_Features>
double_buffered_dispatcher<dispatcher<Keyring, Action_Features>,
                           detail::needed_input_buffer_size<Keyring>::value,
                           detail::needed_output_buffer_size<Keyring>::value>
make_double_buffered_dispatcher(Keyring, action_features_h<Action_Features>) {
  return double_buffered_dispatcher<dispatcher<Keyring, Action_Features>,
                                    detail::needed_input_buffer_size<Keyring>::value,
                                    detail::needed_output_buffer_size<Keyring>::value>{
      Keyring{}, action_features_h<Action_Features>{}};
}
#endif // defined(DOXYGEN)

} // namespace upd

#include "detail/undef.hpp" // IWYU pragma: keep
