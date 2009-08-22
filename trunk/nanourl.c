
#include "nanourl.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

url_p url_parse(const char *url)
{
  url_p purl = (url_p) calloc(1, sizeof(url_t));
  
  char *s1, *s2;
  int len1, len2;
  
  // scheme
  s1 = strstr(url, "://");
  if ( s1 == 0 ) {
    url_free(purl);
    return 0;
  }
  len1 = s1 - url;
  purl->scheme = (char*) malloc(len1 + 1);
  strncpy(purl->scheme, url, len1);
  url = s1 + 3;
  
  // username & password
  s1 = strchr(url, '@');
  if ( s1 )
  {
    s2 = strchr(url, ':');
    if ( s2 && s2 < s1 )
    { // username:password
      len1 = s2 - url;
      purl->user = (char *) malloc(len1 + 1);
      strncpy(purl->user, url, len1);
      
      len2 = s1 - s2;
      purl->pass = (char *) malloc(len2);
      strncpy(purl->pass, s2+1, len2 - 1);
    }
    else
    { // only username
      len1 = s1 - url;
      purl->user = (char *) malloc(len1 + 1);
      strncpy(purl->user, url, len1);
    }
    url = s1 + 1;
  }
  
  // host & port
  s1 = strchr(url, '/');
  if ( s1 == 0 )
  {
    url_free(purl);
    return 0;
  }
  s2 = strchr(url, ':');
  if ( s2 && s2 < s1 )
  { // host:port
    len1 = s2 - url;
    purl->host = (char *) malloc(len1 + 1);
    strncpy(purl->host, url, len1);
    
    len2 = s1 - s2;
    purl->port = (char *) malloc(len2);
    strncpy(purl->port, s2 + 1, len2 - 1);
  }
  else
  { // only host
    len1 = s1 - url;
    purl->host = (char *) malloc(len1 + 1);
    strncpy(purl->host, url, len1);
  }
  url = s1;
  
  // fragment
  s1 = strchr(url, '#');
  if ( s1 )
  {
    len1 = strlen(s1);
    purl->fragment = (char*) malloc(len1);
    strcpy(purl->fragment, s1 + 1);
    s2 = s1;
  }
  else
  {
    s2 = strchr(url, 0);
  }
  len2 = strlen(s2);
  
  s1 = strchr(url, '?');
  if ( s1 && s1 < s2 )
  {
    len1 = strlen(s1) - len2;
    purl->query = (char *) malloc(len1);
    strncpy(purl->query, s1+1, len1-1);
    s2 = s1;
    len2 += len1;
  }
  
  len1 = strlen(url) - len2;
  purl->path = (char *) malloc(len1 + 1);
  strncpy(purl->path, url, len1);
  
  return purl;
}

void url_free(url_p url)
{
  if ( url )
  {
    free(url->scheme);
    free(url->host);
    free(url->port);
    free(url->user);
    free(url->pass);
    free(url->path);
    free(url->query);
    free(url->fragment);
    free(url);
  }
}
