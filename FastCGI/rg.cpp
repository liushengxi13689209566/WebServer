#include <regex>
#include <iostream>
#include <string>
#include "fcgi.h"
using namespace std;

int main()
{
    const char *regex_str = R"(.+\.php$)";
    // const char *regex_str = R"((.php)$)";
    const char *addr = "127.0.0.1";
    int port = 9000;

    regex reg(regex_str);

    char requesturi[] = "/home/cwh/WebServer/src/FastCGI/index.php";
    if (regex_match(requesturi, reg)) {
        cout << "is_dynamtic" << endl;

        // FastCgi_t *c;
        // c = (FastCgi_t *)malloc(sizeof(FastCgi_t));
        // FastCgi_init(c);
        // setRequestId(c,1);
        // startConnect(c);
        // sendStartRequestRecord(c);
        // sendParams(c, "SCRIPT_FILENAME", requesturi);
        // sendParams(c, "REQUEST_METHOD", "GET");
        // sendEndRequestRecord(c);
        // readFromPhp(c);
        // FastCgi_finit(c);
    }

}