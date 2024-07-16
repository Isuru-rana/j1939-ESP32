#include <estd/string.h>

#include <esp_console.h>
#include <esp_log.h>
#include <argtable3/argtable3.h>

#include <j1939/state-machines/transport_protocol.hpp>

#include "nca.h"
#include "streambuf.h"
#include "tp.h"

using namespace embr::j1939;

esp_idf::log_ostream clog;   // Coming along well, almost ready
bool dca_enabled = true;

extern transport_type t;

#define PROMPT_STR "j1939"

const char* TAG = "j1939::console::pri";

static struct
{
    struct arg_str* abbrev;
    struct arg_int* da;
    struct arg_end* end;

}   emit_args;


static struct
{
    //struct arg_str* abbrev;
    struct arg_int* da;
    struct arg_int* pgn;
    struct arg_end* end;

}   emit_rqst_args;


static struct
{
    struct arg_str* command;
    struct arg_end* end;

}   addr_args;


static struct
{
    struct arg_str* command;
    struct arg_end* end;

}   log_args;


static int emit(int argc, char** argv)
{
    return -1;
}


static int emit_rqst(int argc, char** argv)
{
    using traits = transport_traits<transport_type>;

    int nerrors = arg_parse(argc, argv, (void**) &emit_rqst_args);

    if(nerrors) return -1;

    bool da_present = emit_rqst_args.da->count;
    int da = da_present ? emit_rqst_args.da->ival[0] : addresses::null;
    uint8_t sa;
    uint32_t pgn = emit_rqst_args.pgn->ival[0];

    // DEBT: Check for da range validity

    if(nca.state() == sm::v1::network_base::states::claimed)
        sa = nca.address().value();
    else
        sa = 0; // DEBT

    pdu<pgns::request> p(sa, da, pgn);

    traits::send(t, p);

    if(tp.state() == tp_type::IDLE)
    {
        // Reserve transport protocol state machine, in case response is > 8 bytes
        // TODO: Still need to unreserve/release
        tp.initiate_responder(da);
    }
    else
        ESP_LOGI(TAG, "Unable to reserve transport protocol for receipt");

    return 0;
}


static int list(int argc, char** argv)
{
    return -1;
}


static int addr(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &addr_args);

    if(nerrors) return -1;

    estd::layer2::const_string cmd = addr_args.command->sval[0];

    if(cmd == "set")
    {

    }
    else if(cmd == "claim")
    {
        nca.start(t);
    }
    else if(cmd == "release")
    {

    }
    else if(cmd == "show")
    {
        clog << "address: ";

        if(nca.state() == sm::v1::network_base::states::claimed)
        {
            clog << estd::hex << (unsigned) nca.address().value();
            clog << " (claimed)";
        }
        // TODO: Do set override too
        else
        {
            clog << "unset";
        }

        clog << estd::endl;
    }
    else if(cmd == "dest")
    {
        // default destination
    }

    return 0;
}

static int log(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &addr_args);

    if(nerrors) return -1;

    estd::layer2::const_string cmd = addr_args.command->sval[0];

    if(cmd == "on")
    {
        dca_enabled = true;
    }
    else if(cmd == "off")
    {
        dca_enabled = false;
    }

    return 0;
}

static void register_emit()
{
    const esp_console_cmd_t cmd = {
        .command = "emit",
        .help = "Send message with defaults",
        .hint = nullptr,
        .func = &emit,
        .argtable = &emit_args
    };

    emit_args.abbrev = arg_str1(nullptr, nullptr, "<cmd>", "Abbreviated command name (i.e. CM1, BJM1, etc)");
    emit_args.da = arg_int1(nullptr, nullptr, "<da>", "Destination Address");
    emit_args.end = arg_end(2);

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}


// DEBT: Would prefer a smart enough 'emit' to handle all this
static void register_emit_rqst()
{
    const esp_console_cmd_t cmd = {
        .command = "emit-rqst",
        .help = "Send request message with defaults",
        .hint = nullptr,
        .func = &emit_rqst,
        .argtable = &emit_rqst_args
    };

    //emit_rqst_args.abbrev = arg_str1(nullptr, nullptr, "<cmd>", "Abbreviated command name (i.e. CM1, BJM1, etc)");
    emit_rqst_args.da = arg_int0(nullptr, nullptr, "<da>", "Destination Address");
    emit_rqst_args.pgn = arg_int1(nullptr, nullptr, "<pgn>", "Particular PGN requested");
    emit_rqst_args.end = arg_end(2);

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}



static void register_list()
{
    const esp_console_cmd_t cmd = {
        .command = "list",
        .help = "List known messages (TBD)",
        .hint = nullptr,
        .func = &list,
        .argtable = nullptr
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}


static void register_addr()
{
    const esp_console_cmd_t cmd = {
        .command = "addr",
        .help = "network address control",
        .hint = nullptr,
        .func = &addr,
        .argtable = &addr_args
    };

    addr_args.command = arg_str1(nullptr, nullptr, "<set|dest|show|claim|release>", nullptr);
    addr_args.end = arg_end(2);

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}


static void register_log()
{
    const esp_console_cmd_t cmd = {
        .command = "log",
        .help = "logging on or off",
        .hint = nullptr,
        .func = &log,
        .argtable = &log_args
    };

    log_args.command = arg_str1(nullptr, nullptr, "<on|off>", nullptr);
    log_args.end = arg_end(2);

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}


static esp_console_repl_t* init_repl()
{
    esp_console_repl_t* repl = nullptr;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    repl_config.prompt = PROMPT_STR ">";
    repl_config.max_cmdline_length = 80;

    esp_console_register_help_command();

#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));
#elif defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));
#else
#error
#endif

    return repl;
}


void init_console()
{
    esp_console_repl_t* repl = init_repl();

    register_emit();
    register_emit_rqst();
    register_list();
    register_addr();
    register_log();

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}