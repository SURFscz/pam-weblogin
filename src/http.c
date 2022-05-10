#include "defs.h"
#include "utils.h"
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct curl_fetch_st
{
	char *payload;
	size_t size;
};

/* callback for curl fetch */
static size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;							 /* calculate buffer size */
	struct curl_fetch_st *p = (struct curl_fetch_st *)userp; /* cast pointer to fetch struct */

	/* expand buffer */
	p->payload = (char *)realloc(p->payload, p->size + realsize + 1);

	/* check buffer */
	if (p->payload == NULL)
	{
		/* this isn't good */
		printf("ERROR: Failed to expand buffer in curl_callback");

		/* free buffer */
		free(p->payload);

		/* return */
		return (size_t)-1;
	}

	/* copy contents to buffer */
	memcpy(&(p->payload[p->size]), contents, realsize);

	/* set new buffer size */
	p->size += realsize;

	/* ensure null termination */
	p->payload[p->size] = 0;

	/* return size */
	return realsize;
}

/*
 * postURL POSTS data to URL url and returns the result
 */
int postURL(const char *url, const char *token, const char *data, char **result)
{
	int rc = 0;
	CURL *curl;
	CURLcode cc;
	long response_code;
	struct curl_fetch_st curl_fetch;
	struct curl_fetch_st *fetch = &curl_fetch;

	/* default return value */
	*result = NULL;

	/* Prepare Headers... */
	struct curl_slist *headers = NULL;
	char *authorization = NULL;

	asprintf(&authorization, "Authorization: %s", token);
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, authorization);
	free(authorization);
	/*
		printf("http URL: %s\n", url);
		printf("http data: %s\n", data);
	*/

	/* Prepare API request... */
	curl = curl_easy_init();
	if (curl)
	{
		/* Prepare payload for response... */
		fetch->payload = (char *)calloc(1, sizeof(fetch->payload));
		fetch->size = 0;

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)fetch);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "pam-websso");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));

		/* Perform the request */
		if ((cc = curl_easy_perform(curl)) != CURLE_OK)
		{
			// printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(cc));
			goto cleanup;
		}

		/* Check response */
		if ((cc = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code)) != CURLE_OK || response_code != 200)
		{
			// printf("Invalid response\n");
			goto cleanup;
		}

		/* Assign payload */
		if (fetch->payload != NULL)
		{
			rc = 1;
			*result = fetch->payload;
		}
	}

cleanup:
	if (curl)
		curl_easy_cleanup(curl);
	if (rc == 0 && fetch->payload)
	{
		free(fetch->payload);
	}
	return rc;
}
