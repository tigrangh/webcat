#pragma once

#include "global.hpp"

#include <belt.pp/isocket.hpp>
#include <belt.pp/ilog.hpp>
#include <belt.pp/direct_stream.hpp>
#include <mesh.pp/cryptoutility.hpp>

#include <boost/filesystem/path.hpp>

#include <memory>

namespace webcat
{
namespace detail
{
    class server_internals;
}

class WEBCATSERVERSHARED_EXPORT server
{
public:
    server(beltpp::ip_address const& bind_to_address,
           //meshpp::private_key const& pv_key,
           beltpp::ilog* plogger,
           beltpp::direct_channel& stream);
    server(server&& other) noexcept;
    ~server();

    void wake();
    void run(bool& stop);

private:
    std::unique_ptr<detail::server_internals> m_pimpl;
};

}

