/* see https://stackoverflow.com/questions/2410976/how-to-define-a-string-literal-in-gcc-command-line
 * and note that # is the CPP "stringizing" operator */
#define STR(x) #x
#define TOSTR(X) STR(x)

#define API_START_PATH "start"
#define API_START_RESPONSE_CODE 201

#define API_CHECK_PIN_PATH "check-pin"
#define API_CHECK_PIN_RESPONSE_CODE 201

#define DEFAULT_CONF_FILE "/etc/pam-weblogin.conf"

#define PROMPT_USERNAME "Username: "
#define PROMPT_PIN "Verification code: "

#define SERVER_ERROR "Server error!"

#ifndef GIT_COMMIT
#DEFINE GIT_COMMIT 0000
#endif

#ifndef JSONPARSER_GIT_COMMIT
#DEFINE JSONPARSER_GIT_COMMIT 0000
#endif
