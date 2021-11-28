#include "common.hpp"

#include "model.hpp"

#include <unordered_set>

using std::string;
using std::pair;
using std::vector;
namespace chrono = std::chrono;

namespace cloudy
{
beltpp::void_unique_ptr get_putl()
{
    beltpp::message_loader_utility utl;
    Model::detail::extension_helper(utl);

    auto ptr_utl =
        beltpp::new_void_unique_ptr<beltpp::message_loader_utility>(std::move(utl));

    return ptr_utl;
}

namespace detail
{
wait_result_item wait_and_receive_one(wait_result& wait_result_info,
                                      beltpp::event_handler& eh,
                                      beltpp::stream& event_stream,
                                      beltpp::stream* on_demand_stream)
{
    auto& info = wait_result_info.m_wait_result;

    if (info == beltpp::event_handler::wait_result::nothing)
    {
        assert(wait_result_info.on_demand_packets.second.empty());
        if (false == wait_result_info.on_demand_packets.second.empty())
            throw std::logic_error("false == wait_result_info.on_demand_packets.second.empty()");
        assert(wait_result_info.event_packets.second.empty());
        if (false == wait_result_info.event_packets.second.empty())
            throw std::logic_error("false == wait_result_info.event_packets.second.empty()");

        std::unordered_set<beltpp::event_item const*> wait_streams;

        info = eh.wait(wait_streams);

        if (info & beltpp::event_handler::event)
        {
            for (auto& pevent_item : wait_streams)
            {
                B_UNUSED(pevent_item);

                beltpp::socket::packets received_packets;
                beltpp::socket::peer_id peerid;
                received_packets = event_stream.receive(peerid);

                wait_result_info.event_packets = std::make_pair(peerid,
                                                                std::move(received_packets));
            }
        }

        /*if (wait_result & beltpp::event_handler::timer_out)
        {
        }*/

        if (on_demand_stream && (info & beltpp::event_handler::on_demand))
        {
            beltpp::socket::packets received_packets;
            beltpp::socket::peer_id peerid;
            received_packets = on_demand_stream->receive(peerid);

            wait_result_info.on_demand_packets = std::make_pair(peerid,
                                                                std::move(received_packets));
        }
    }

    auto result = wait_result_item::empty_result();

    if (info & beltpp::event_handler::event)
    {
        if (false == wait_result_info.event_packets.second.empty())
        {
            auto packet = std::move(wait_result_info.event_packets.second.front());
            auto peerid = wait_result_info.event_packets.first;

            wait_result_info.event_packets.second.pop_front();

            result = wait_result_item::event_result(peerid, std::move(packet));
        }

        if (wait_result_info.event_packets.second.empty())
            info = beltpp::event_handler::wait_result(info & ~beltpp::event_handler::event);

        return result;
    }

    if (info & beltpp::event_handler::timer_out)
    {
        info = beltpp::event_handler::wait_result(info & ~beltpp::event_handler::timer_out);
        result = wait_result_item::timer_result();
        return result;
    }

    if (info & beltpp::event_handler::on_demand)
    {
        if (false == wait_result_info.on_demand_packets.second.empty())
        {
            auto packet = std::move(wait_result_info.on_demand_packets.second.front());
            auto peerid = wait_result_info.on_demand_packets.first;

            wait_result_info.on_demand_packets.second.pop_front();

            result = wait_result_item::on_demand_result(peerid, std::move(packet));
        }

        if (wait_result_info.on_demand_packets.second.empty())
            info = beltpp::event_handler::wait_result(info & ~beltpp::event_handler::on_demand);

        return result;
    }

    return result;
}
} // end detail
} // end cloudy


