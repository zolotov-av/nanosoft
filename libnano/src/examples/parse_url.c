
#include "nanourl.h"
#include <stdio.h>

int main(int argc, char **argv)
{
  if ( argc < 2 )
  {
    fprintf(stderr, "parse_url <URL>\n");
    return 1;
  }
  url_p purl = url_parse(argv[1]);
  if ( purl == 0 )
  {
    fprintf(stderr, "incorrect url\n");
    return 2;
  }
  printf("scheme:\t%s\nhost:\t%s\nport:\t%s\nuser:\t%s\npass:\t%s\npath:\t%s\nquery:\t%s\nfragment:\t%s\n",
    purl->scheme,
    purl->host,
    purl->port,
    purl->user,
    purl->pass,
    purl->path,
    purl->query,
    purl->fragment
    );
  url_free(purl);
  return 0;
}
