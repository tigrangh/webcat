#include "server.hpp"

#include "common.hpp"
#include "http.hpp"
#include "model.hpp"

#include <belt.pp/socket.hpp>
#include <belt.pp/packet.hpp>
#include <belt.pp/timer.hpp>
#include <belt.pp/scope_helper.hpp>

#include <memory>
#include <chrono>
#include <vector>
#include <utility>
#include <unordered_set>

namespace webcat
{
using beltpp::event_handler;
using beltpp::socket;
using beltpp::event_handler_ptr;
using beltpp::socket_ptr;
using beltpp::packet;
using beltpp::ip_address;
using beltpp::ilog;
using beltpp::timer;

using std::unordered_set;
using std::unique_ptr;
namespace chrono = std::chrono;
using std::vector;

namespace detail
{
using rpc_sf = beltpp::socket_family_t<&http::message_list_load<&Model::message_list_load>>;

class server_internals
{
public:
    ilog* plogger;
    event_handler_ptr ptr_eh;
    socket_ptr ptr_socket;

    beltpp::stream_ptr ptr_direct_stream;

    wait_result wait_result_info;

    server_internals(ip_address const& bind_to_address,
                     ilog* _plogger,
                     beltpp::direct_channel& stream)
        : plogger(_plogger)
        , ptr_eh(beltpp::libsocket::construct_event_handler())
        , ptr_socket(beltpp::libsocket::getsocket<rpc_sf>(*ptr_eh))
        , ptr_direct_stream(beltpp::construct_direct_stream(server_peerid, *ptr_eh, stream))
    {
        ptr_eh->set_timer(event_timer_period);

        if (bind_to_address.local.empty())
            throw std::logic_error("bind_to_address.local.empty()");

        ptr_socket->listen(bind_to_address);

        ptr_eh->add(*ptr_socket);
    }

    void writeln_node(string const& value)
    {
        if (plogger)
            plogger->error(value);
    }
};

} // end namespace detail

using namespace Model;

server::server(ip_address const& bind_to_address,
               ilog* plogger,
               beltpp::direct_channel& stream)
    : m_pimpl(new detail::server_internals(bind_to_address,
                                           plogger,
                                           stream))
{}
server::server(server&& other) noexcept = default;
server::~server() = default;

void server::wake()
{
    m_pimpl->ptr_eh->wake();
}

void server::run(bool& stop_check)
{
    stop_check = false;

    auto wait_result = detail::wait_and_receive_one(m_pimpl->wait_result_info,
                                                    *m_pimpl->ptr_eh,
                                                    *m_pimpl->ptr_socket,
                                                    m_pimpl->ptr_direct_stream.get());

    if (wait_result.et == detail::wait_result_item::event)
    {
        auto peerid = wait_result.peerid;
        auto received_packet = std::move(wait_result.packet);

        //auto& stream = *m_pimpl->ptr_socket;

        try
        {
            switch (received_packet.type())
            {
            default:
            {
                m_pimpl->writeln_node("peer: " + peerid);
                m_pimpl->writeln_node("server can't handle: " + received_packet.to_string());

                break;
            }
            }   // switch received_packet.type()
        }
        catch (std::exception const& e)
        {
            // RemoteError msg;
            // msg.message = e.what();
            // stream.send(peerid, beltpp::packet(msg));
            throw;
        }
        catch (...)
        {
            // RemoteError msg;
            // msg.message = "unknown exception";
            // stream.send(peerid, beltpp::packet(msg));
            throw;
        }
    }
    else if (wait_result.et == detail::wait_result_item::timer)
    {
        m_pimpl->ptr_socket->timer_action();
    }
    else if (m_pimpl->ptr_direct_stream && wait_result.et == detail::wait_result_item::on_demand)
    {
        //auto peerid = wait_result.peerid;
        auto received_packet = std::move(wait_result.packet);

    }
}

}
