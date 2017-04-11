#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include "uploadfile.h"

char IS_CURL_INIT = 0;

const size_t BUF_SIZE= 5000000;
size_t wr_index = 0;

static size_t write_data_func(char *ptr, size_t size, size_t nmemb, char* data)
{
    if(data==NULL || wr_index + size*nmemb > BUF_SIZE) return 0; // Если выход за размеры буфера - ошибка

    memcpy( &data[wr_index], ptr, size*nmemb);// дописываем данные в конец
    wr_index += size*nmemb;  // изменяем  текущее положение

    return size*nmemb;
}

int uploadFile(const char* fileName, const void* buffer, size_t bufferSize, char* buff_out, size_t outBufferSize)
{
    if (IS_CURL_INIT == 0) {
        IS_CURL_INIT = 1;
        curl_global_init(CURL_GLOBAL_ALL);
    }
    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;

    /* Fill in the file upload field */
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_BUFFER, fileName,
                 CURLFORM_BUFFERPTR, buffer,
                 CURLFORM_BUFFERLENGTH, bufferSize,
                 CURLFORM_END);


    /* Fill in the submit field too, even if this is rarely needed */
    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_COPYNAME, "submit",
                 CURLFORM_COPYCONTENTS, "send",
                 CURLFORM_END);

    CURL *curl = curl_easy_init();
    if (curl)
    {
        /* what URL that receives this POST */
        curl_easy_setopt(curl, CURLOPT_URL, "http://84.245.225.2:1080/fileupload.php");

        struct curl_slist *headerlist=NULL;
        headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        wr_index = 0;
        char   wr_buf[BUF_SIZE+1];  // char*   wr_buf[BUF_SIZE+1];
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, wr_buf);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_func);

        /* Perform the request, res will get the return code */
        CURLcode res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        else {
            if (strncmp(wr_buf, "{\"url\":\"", 8) == 0) {
                size_t len = strlen(wr_buf + 8);
                if (len > 2 && len < outBufferSize) {
                    len -= 2;
                    char* ptr = wr_buf + 8;

                    while (len-- > 0) {
                        if (*ptr != '\\') {
                            *buff_out++ = *ptr;
                        }
                        ++ptr;
                    }
                    *buff_out = 0;
                }
            }
        }

        /* always cleanup */
        curl_easy_cleanup(curl);

        /* then cleanup the formpost chain */
        curl_formfree(formpost);
        /* free slist */
        curl_slist_free_all (headerlist);

        return res == CURLE_OK;
    }
    return 1;
}
