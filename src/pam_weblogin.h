/* see https://stackoverflow.com/questions/2410976/how-to-define-a-string-literal-in-gcc-command-line
 * and note that # is the CPP "stringizing" operator */
#define STR(x) #x
#define TOSTR(X) STR(x)

#define API_START_PATH "start"
#define API_CHECK_CODE_PATH "check-pin"

#define DEFAULT_CONF_FILE "/etc/security/pam-weblogin.conf"
#define ALTERNATE_CONF_FILE "/etc/pam-weblogin.conf"

#define MSG_GROUPS "\nWhat group are you logging in for?"

#define PROMPT_USERNAME "Username: "
#define PROMPT_CODE "Enter verification code: "
#define PROMPT_GROUP "\nSelect group: "
#define PROMPT_WRONG_NUMBER "Wrong number"

#ifndef GIT_COMMIT
#define GIT_COMMIT 0000
#endif

#ifndef JSONPARSER_GIT_COMMIT
#define JSONPARSER_GIT_COMMIT 0000
#endif

#define BUFSIZE 16384