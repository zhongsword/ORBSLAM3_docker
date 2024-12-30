//
// Copyright (c) 2019-2024 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/mysql/client_errc.hpp>
#include <boost/mysql/common_server_errc.hpp>
#include <boost/mysql/error_categories.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/is_fatal_error.hpp>
#include <boost/mysql/mariadb_server_errc.hpp>
#include <boost/mysql/mysql_server_errc.hpp>
#include <boost/mysql/string_view.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/test/unit_test.hpp>

#include <system_error>

using namespace boost::mysql;
namespace asio = boost::asio;

BOOST_AUTO_TEST_CASE(test_is_fatal_error)
{
    // Helpers
    auto make_mysql_code = [](int value) { return error_code(value, get_mysql_server_category()); };
    auto make_mariadb_code = [](int value) { return error_code(value, get_mysql_server_category()); };
    auto make_openssl_code = [](int value) { return error_code(value, asio::error::get_ssl_category()); };

    struct
    {
        string_view name;
        error_code input;
        bool expected;
    } test_cases[] = {
        // No error
        {"success",                         error_code(),                                                   false},

        // Network errors
        {"net_eof",                         asio::error::connection_aborted,                                true },
        {"net_cancelled",                   asio::error::operation_aborted,                                 true },
        {"net_reset",                       asio::error::network_reset,                                     true },

        // SSL, generated by asio
        {"ssl_stream",                      asio::ssl::error::stream_errors::stream_truncated,              true },

        // SSL, generated by openssl (any numeric value may be produced)
        {"ssl_openssl",                     make_openssl_code(1623),                                        true },

        // Other system errors
        {"system",                          make_error_code(std::errc::broken_pipe),                        true },

        // Client errors affecting frame parsing
        {"incomplete_message",              client_errc::incomplete_message,                                true },
        {"protocol_value_error",            client_errc::protocol_value_error,                              true },
        {"extra_bytes",                     client_errc::extra_bytes,                                       true },
        {"sequence_number_mismatch",        client_errc::sequence_number_mismatch,                          true },
        {"max_buffer_size_exceeded",        client_errc::max_buffer_size_exceeded,                          true },

        // Client errors affecting the static interface
        {"metadata_check_failed",           client_errc::metadata_check_failed,                             true },
        {"num_resultsets_mismatch",         client_errc::num_resultsets_mismatch,                           true },
        {"row_type_mismatch",               client_errc::row_type_mismatch,                                 true },
        {"static_row_parsing_error",        client_errc::static_row_parsing_error,                          true },

        // Client errors affecting handshake
        {"server_unsupported",              client_errc::server_unsupported,                                true },
        {"unknown_auth_plugin",             client_errc::unknown_auth_plugin,                               true },
        {"auth_plugin_requires_ssl",        client_errc::auth_plugin_requires_ssl,                          true },
        {"server_doesnt_support_ssl",       client_errc::server_doesnt_support_ssl,                         true },

        // Other client errors
        {"wrong_num_params",                client_errc::wrong_num_params,                                  false},
        {"pool_not_running",                client_errc::pool_not_running,                                  false},
        {"invalid_encoding",                client_errc::invalid_encoding,                                  false},
        {"unformattable_value",             client_errc::unformattable_value,                               false},
        {"format_string_invalid_syntax",    client_errc::format_string_invalid_syntax,                      false},
        {"format_string_invalid_encoding",  client_errc::format_string_invalid_encoding,                    false},
        {"format_string_manual_auto_mix",   client_errc::format_string_manual_auto_mix,                     false},
        {"format_string_invalid_specifier", client_errc::format_string_invalid_specifier,                   false},
        {"format_arg_not_found",            client_errc::format_arg_not_found,                              false},
        {"unknown_character_set",           client_errc::unknown_character_set,                             false},

        // Fatal server errors
        {"ER_UNKNOWN_COM_ERROR",            common_server_errc::er_unknown_com_error,                       true },
        {"ER_ABORTING_CONNECTION",          common_server_errc::er_aborting_connection,                     true },
        {"ER_NET_PACKET_TOO_LARGE",         common_server_errc::er_net_packet_too_large,                    true },
        {"ER_NET_PACKET_TOO_LARGE",         common_server_errc::er_net_packet_too_large,                    true },
        {"ER_NET_READ_ERROR_FROM_PIPE",     common_server_errc::er_net_read_error_from_pipe,                true },
        {"ER_NET_FCNTL_ERROR",              common_server_errc::er_net_fcntl_error,                         true },
        {"ER_NET_PACKETS_OUT_OF_ORDER",     common_server_errc::er_net_packets_out_of_order,                true },
        {"ER_NET_UNCOMPRESS_ERROR",         common_server_errc::er_net_uncompress_error,                    true },
        {"ER_NET_READ_ERROR",               common_server_errc::er_net_read_error,                          true },
        {"ER_NET_READ_INTERRUPTED",         common_server_errc::er_net_read_interrupted,                    true },
        {"ER_NET_ERROR_ON_WRITE",           common_server_errc::er_net_error_on_write,                      true },
        {"ER_NET_WRITE_INTERRUPTED",        common_server_errc::er_net_write_interrupted,                   true },
        {"ER_MALFORMED_PACKET",             common_server_errc::er_malformed_packet,                        true },
        {"ER_ZLIB_Z_MEM_ERROR",             common_server_errc::er_zlib_z_mem_error,                        true },
        {"ER_ZLIB_Z_BUF_ERROR",             common_server_errc::er_zlib_z_buf_error,                        true },
        {"ER_ZLIB_Z_DATA_ERROR",            common_server_errc::er_zlib_z_data_error,                       true },

        // Non-fatal server errors
        {"ER_NO_SUCH_TABLE",                common_server_errc::er_no_such_table,                           false},
        {"ER_BAD_DB_ERROR",                 common_server_errc::er_bad_db_error,                            false},

        // Server-specific or user-defined errors
        {"mysql_specific",                  make_mysql_code(mysql_server_errc::er_invalid_cast),            false},
        {"mariadb_specific",                make_mariadb_code(mariadb_server_errc::er_gis_different_srids), false},
        {"mysql_user_defined",              make_mysql_code(9812),                                          false},
        {"mariadb_user_defined",            make_mariadb_code(9812),                                        false},
    };

    for (const auto& tc : test_cases)
    {
        BOOST_TEST_CONTEXT(tc.name)
        {  // Call fn and check
            BOOST_TEST(is_fatal_error(tc.input) == tc.expected);
        }
    }
}
