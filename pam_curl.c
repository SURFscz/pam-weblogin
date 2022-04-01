#include <curl/curl.h>

struct curl_fetch_st {
    char* payload;
    size_t size;
};


/* callback for curl fetch */
size_t curl_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;                            /* calculate buffer size */
    struct curl_fetch_st* p = (struct curl_fetch_st*) userp;   /* cast pointer to fetch struct */

    /* expand buffer */
    p->payload = (char*)realloc(p->payload, p->size + realsize + 1);


    /* check buffer */
    if (p->payload == NULL) {
        /* this isn't good */
        debug("ERROR: Failed to expand buffer in curl_callback");

        /* free buffer */
        free(p->payload);

        /* return */
        return (size_t) -1;
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

CRESULT fetchURL()
{
    HRESULT hr = E_UNEXPECTED;
    CURL* curl;
    CURLcode rc;
    char url[URL_LEN];
    char data[DATA_LEN];
    struct curl_fetch_st curl_fetch;
    struct curl_fetch_st* fetch = &curl_fetch;

    // Prepare full url...
    url = "https://localhost:8123/req";
    debug("URL: %s\n", url);

    // Prepare Headers...
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Prepare input data...
    sprintf_s((char *)data, DATA_LEN, "{\"user\":\"%S\",\"token\":\"%S\"}",
        "martin",
        "1234"
    );

    debug("JSON: %s\n", data);

    // Prepare payload for response...
    fetch->payload = (char*)calloc(1, sizeof(fetch->payload));
    fetch->size = 0;

    // Prepare API request...
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)fetch);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "pam-websso");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data, strlen(data));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));

        /* Perform the request */
        if ((rc = curl_easy_perform(curl) ) == CURLE_OK) {
            hr = S_OK;
        }
        else {
            debug("curl_easy_perform() failed: %s\n",
                curl_easy_strerror(rc));
        }

        /* check payload */
         if (fetch->payload != NULL) {
            Document document;
            debug("CURL Returned: \n%s\n", fetch->payload);
            free(fetch->payload);
         }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return hr;
}
