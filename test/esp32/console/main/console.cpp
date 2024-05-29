#include <estd/string.h>

#include <esp_console.h>
#include <esp_log.h>
#include <argtable3/argtable3.h>

#include "nca.h"
#include "streambuf.h"

using namespace embr::j1939;

static esp_idf::log_ostream clog;   // Coming along well, almost ready

extern transport_type t;

#define PROMPT_STR "j1939"

static struct
{
    struct arg_str* abbrev;
    struct arg_int* da;
    struct arg_end* end;

}   emit_args;


static struct
{
    struct arg_str* command;
    struct arg_end* end;

}   addr_args;


static int emit(int argc, char** argv)
{
    return -1;
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

        if(nca.state == impl::network_ca_base::states::claimed)
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

    addr_args.command = arg_str1(nullptr, nullptr, "<set|show|claim|release>", nullptr);
    addr_args.end = arg_end(2);

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

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    return repl;
}

void init_console()
{
    esp_console_repl_t* repl = init_repl();

    register_emit();
    register_list();
    register_addr();

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}