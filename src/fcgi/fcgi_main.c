#include "fcgi_main.h"

#include "fcgi_stdio.h"

 int32_t fcgi_main(void)
{
    while (FCGI_Accept() >= 0)
    {
        printf("Status: 200 OK\r\n\r\nHello World!\n");
    }
    return 0;
}

int32_t main (void)
{
    fcgi_main();
    return 0;
}
