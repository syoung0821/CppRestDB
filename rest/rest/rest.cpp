#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#pragma comment(lib, "cpprest_2_10")

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <mysql.h>

using namespace std;

#define TRACE(msg)            wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

map<utility::string_t, utility::string_t> dictionary;
int arg1;
char** argv1;

enum MethodType
{
    GET,
    POST,
    PUT,
    DEL
};


bool mysql_ckeck(int argc, char** argv)
{
  
    return true;
}

string mysql_connect(MethodType type, string id, string name, string pwd, string desc)
{
    MYSQL* mysql = NULL;
    if (mysql_library_init(arg1, argv1, NULL)) {
        fprintf(stderr, "could not initialize MySQL client library\n");
        exit(1);
    }

    mysql = mysql_init(mysql);
    string query;
    if (!mysql) {
        puts("Init faild, out of memory?");
        return "EXIT_FAILURE";
    }

    mysql_options(mysql, MYSQL_READ_DEFAULT_FILE, (void*)"./my.cnf");

    if (!mysql_real_connect(mysql,       /* MYSQL structure to use */
        "127.0.0.1",  /* server hostname or IP address */
        "root",  /* mysql user */
        "rud0303",   /* password */
        "test",    /* default database to use, NULL for none */
        0,           /* port number, 0 for default */
        NULL,        /* socket file or named pipe name */
        CLIENT_FOUND_ROWS /* connection flags */)) {
        puts("Connect failed\n");
    }
    else {
        switch (type)
        {
        case GET:
            query = "SELECT * FROM account WHERE `id` = '" + id + "';";
            break;
        case POST:
            query = "INSERT INTO account (`id`, `name`, `pwd`, `desc`) VALUES('" + id + "', '" + name + "', '" + pwd + "', '" + desc + "');";
            break;
        case PUT:
            query = "UPDATE account SET `name` = '" + name + "', `pwd` = '" + pwd + "', `desc` = '" + desc + "', `mod_date` = now();";
            break;
        case DEL:
            query = "DELETE FROM account WHERE `id` = '" + id + "';";
            break;
        default:
            break;
        }
        if (mysql_query(mysql, query.c_str())) {
            printf("Query failed: %s\n", mysql_error(mysql));
        }
        else {
            MYSQL_RES* result = mysql_store_result(mysql);

            if (!result) {
                printf("Couldn't get results set: %s\n", mysql_error(mysql));
            }
            else {
                MYSQL_ROW row;
                int i;
                unsigned int num_fields = mysql_num_fields(result);

                while ((row = mysql_fetch_row(result))) {
                    for (i = 0; i < num_fields; i++) {
                        printf("%s, ", row[i]);
                    }
                    putchar('\n');
                }

                mysql_free_result(result);
            }
        }
    }

    mysql_close(mysql);

    mysql_library_end();

    return "EXIT_SUCCESS";
}

void display_json(
    json::value const& jvalue,
    utility::string_t const& prefix)
{
    wcout << prefix << jvalue.serialize() << endl;
}

void handle_get(http_request request)
{
    TRACE(L"\nhandle GET\n");

    auto answer = json::value::object();

    for (auto const& p : dictionary)
    {
        answer[p.first] = json::value::string(p.second);
    }
    mysql_connect(MethodType(GET), "test1", "test3", "1234", "test3");

    display_json(json::value::null(), L"R: ");
    display_json(answer, L"S: ");

    request.reply(status_codes::OK, answer);
}

void handle_request(
    http_request request,
    function<void(json::value const&, json::value&)> action)
{
    auto answer = json::value::object();

    request
        .extract_json()
        .then([&answer, &action](pplx::task<json::value> task) {
        try
        {
            auto const& jvalue = task.get();
            display_json(jvalue, L"R: ");

            if (!jvalue.is_null())
            {
                action(jvalue, answer);
            }
        }
        catch (http_exception const& e)
        {
            wcout << e.what() << endl;
        }
            })
        .wait();


    display_json(answer, L"S: ");

    request.reply(status_codes::OK, answer);
}

void handle_post(http_request request)
{
    TRACE("\nhandle POST\n");

    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer)
        {
            for (auto const& e : jvalue.as_array())
            {
                if (e.is_string())
                {
                    auto key = e.as_string();
                    auto pos = dictionary.find(key);

                    if (pos == dictionary.end())
                    {
                        answer[key] = json::value::string(L"<nil>");
                    }
                    else
                    {
                        answer[pos->first] = json::value::string(pos->second);
                    }
                }
            }

            mysql_connect(MethodType(POST), "test2", "test2", "1234", "test2");
        });
}

void handle_put(http_request request)
{
    TRACE("\nhandle PUT\n");

    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer)
        {
            for (auto const& e : jvalue.as_object())
            {
                if (e.second.is_string())
                {
                    auto key = e.first;
                    auto value = e.second.as_string();

                    if (dictionary.find(key) == dictionary.end())
                    {
                        TRACE_ACTION(L"added", key, value);
                        answer[key] = json::value::string(L"<put>");
                    }
                    else
                    {
                        TRACE_ACTION(L"updated", key, value);
                        answer[key] = json::value::string(L"<updated>");
                    }

                    dictionary[key] = value;
                }
            }
            mysql_connect(MethodType(PUT), "test1", "update1", "1111", "update1");

        });
}

void handle_del(http_request request)
{
    TRACE("\nhandle DEL\n");

    handle_request(
        request,
        [](json::value const& jvalue, json::value& answer)
        {
            set<utility::string_t> keys;
            for (auto const& e : jvalue.as_array())
            {
                if (e.is_string())
                {
                    auto key = e.as_string();

                    auto pos = dictionary.find(key);
                    if (pos == dictionary.end())
                    {
                        answer[key] = json::value::string(L"<failed>");
                    }
                    else
                    {
                        TRACE_ACTION(L"deleted", pos->first, pos->second);
                        answer[key] = json::value::string(L"<deleted>");
                        keys.insert(key);
                    }
                }
            }

            for (auto const& key : keys)
                dictionary.erase(key);

            mysql_connect(MethodType(DEL), "test2", "update1", "1111", "update1");
        });
}
int main(int argc, char **argv)
{
    arg1 = argc;
    argv1 = argv;

    mysql_ckeck(argc, argv);

    http_listener listener(L"http://localhost/restdemo");

    listener.support(methods::GET, handle_get);
    listener.support(methods::POST, handle_post);
    listener.support(methods::PUT, handle_put);
    listener.support(methods::DEL, handle_del);

    try
    {
        listener
            .open()
            .then([&listener]() {TRACE(L"\nstarting to listen\n"); })
            .wait();

        while (true);
    }
    catch (exception const& e)
    {
        wcout << e.what() << endl;
    }

    return 0;
}