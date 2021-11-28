#include <webcat/server.hpp>

#include <belt.pp/log.hpp>
#include <belt.pp/scope_helper.hpp>
#include <belt.pp/direct_stream.hpp>

#include <boost/program_options.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem/path.hpp>

#include <iostream>
#include <vector>
#include <exception>
#include <thread>
#include <string>

#include <csignal>

namespace program_options = boost::program_options;

using std::unique_ptr;
using std::string;
using std::cerr;
using std::endl;
using std::vector;
using std::runtime_error;
using std::thread;

bool process_command_line(int argc, char** argv,
                          beltpp::ip_address& bind_to_address);

static bool g_termination_handled = false;
static webcat::server* g_server = nullptr;
void termination_handler(int /*signum*/)
{
    cerr << "stopping..." << endl;

    g_termination_handled = true;
    if (g_server)
        g_server->wake();
}

template <typename SERVER>
void loop(SERVER& server, beltpp::ilog_ptr& plogger_exceptions, bool& termination_handled);

int main(int argc, char** argv)
{
    try
    {
        //  boost filesystem UTF-8 support
        std::locale::global(boost::locale::generator().generate(""));
        boost::filesystem::path::imbue(std::locale());
    }
    catch (...)
    {}  //  don't care for exception, for now
    //

    //meshpp::config::set_public_key_prefix("WebCat-");

    beltpp::ip_address bind_to_address;

    if (false == process_command_line(argc, argv,
                                      bind_to_address))
        return 1;

#ifdef B_OS_WINDOWS
    signal(SIGINT, termination_handler);
#else
    struct sigaction signal_handler;
    signal_handler.sa_handler = termination_handler;
    ::sigaction(SIGINT, &signal_handler, nullptr);
    ::sigaction(SIGTERM, &signal_handler, nullptr);
#endif

    beltpp::ilog_ptr plogger_exceptions;
    try
    {
        cerr << "bind address: " << bind_to_address.to_string() << endl;
        
        beltpp::ilog_ptr plogger = beltpp::console_logger("server", true);
        //plogger_admin->disable();
        
        beltpp::direct_channel direct_channel;

        webcat::server server(bind_to_address,
                              plogger.get(),
                              direct_channel);

        g_server = &server;


        {
            thread server_thread([&server, &plogger_exceptions]
            {
                loop(server, plogger_exceptions, g_termination_handled);
            });

            beltpp::finally join_admin_thread([&server_thread](){ server_thread.join(); });
        }
    }
    catch (std::exception const& ex)
    {
        if (plogger_exceptions)
            plogger_exceptions->message(ex.what());
        cerr << "exception cought: " << ex.what() << endl;
    }
    catch (...)
    {
        if (plogger_exceptions)
            plogger_exceptions->message("always throw std::exceptions");
        cerr << "always throw std::exceptions" << endl;
    }

    cerr << "quit." << endl;

    return 0;
}

template <typename SERVER>
void loop(SERVER& server, beltpp::ilog_ptr& plogger_exceptions, bool& termination_handled)
{
    bool stop_check = false;
    while (false == stop_check)
    {
        try
        {
            if (termination_handled)
                break;
            server.run(stop_check);
            if (stop_check)
            {
                termination_handler(0);
                break;
            }
        }
        catch (std::bad_alloc const& ex)
        {
            if (plogger_exceptions)
                plogger_exceptions->message(ex.what());
            cerr << "exception cought: " << ex.what() << endl;
            cerr << "will exit now" << endl;
            termination_handler(0);
            break;
        }
        catch (std::logic_error const& ex)
        {
            if (plogger_exceptions)
                plogger_exceptions->message(ex.what());
            cerr << "logic error cought: " << ex.what() << endl;
            cerr << "will exit now" << endl;
            termination_handler(0);
            break;
        }
        catch (std::exception const& ex)
        {
            if (plogger_exceptions)
                plogger_exceptions->message(ex.what());
            cerr << "exception cought: " << ex.what() << endl;
        }
        catch (...)
        {
            if (plogger_exceptions)
                plogger_exceptions->message("always throw std::exceptions, will exit now");
            cerr << "always throw std::exceptions, will exit now" << endl;
            termination_handler(0);
            break;
        }
    }
}

bool process_command_line(int argc, char** argv,
                          beltpp::ip_address& bind_to_address)
{
    string bind_interface;
    
    program_options::options_description options_description;
    try
    {
        auto desc_init = options_description.add_options()
            ("help,h", "print this help message and exit.")
            ("interface,i", program_options::value<string>(&bind_interface)->required(),
                            "server interface");
        (void)(desc_init);

        program_options::variables_map options;

        program_options::store(
                    program_options::parse_command_line(argc, argv, options_description),
                    options);

        program_options::notify(options);

        if (options.count("help"))
        {
            throw std::runtime_error("");
        }

        bind_to_address.from_string(bind_interface);
    }
    catch (std::exception const& ex)
    {
        std::stringstream ss;
        ss << options_description;

        string ex_message = ex.what();
        if (false == ex_message.empty())
            cerr << ex.what() << endl << endl;
        cerr << ss.str();
        return false;
    }
    catch (...)
    {
        cerr << "always throw std::exceptions" << endl;
        return false;
    }

    return true;
}
