#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "curl.h"

#define URL_LEN 1024
#define DATA_LEN 1024

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
        printf("ERROR: Failed to expand buffer in curl_callback");

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

CRESULT fetchURL() {
    CRESULT cr = E_UNEXPECTED;

    return cr;

    CURL* curl;
    CURLcode rc;
    char url[URL_LEN];
    char data[DATA_LEN];
    struct curl_fetch_st curl_fetch;
    struct curl_fetch_st* fetch = &curl_fetch;

    // Prepare full url...
    strncpy(url, "https://localhost:8123/req", URL_LEN);
    printf("URL: %s\n", url);

    // Prepare Headers...
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Prepare input data...
    snprintf(data, DATA_LEN, "{\"user\":\"%s\",\"token\":\"%s\"}",
        "martin",
        "1234"
    );

    printf("JSON: %s\n", data);

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
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(data));

        /* Perform the request */
        if ((rc = curl_easy_perform(curl) ) == CURLE_OK) {
//             cr = S_OK;
        }
        else {
            printf("curl_easy_perform() failed: %s\n",
                curl_easy_strerror(rc));
        }

        /* check payload */
         if (fetch->payload != NULL) {
            printf("CURL Returned: \n%s\n", fetch->payload);
            free(fetch->payload);
         }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

//     return cr;
}
