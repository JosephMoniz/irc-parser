#ifndef irc_parser_h
#define irc_parser_h

/**
 * This is just the convenience typedef for the parser struct
 */
typedef struct irc_parser_s irc_parser;

/**
 * This is the typedef for the callback type that will be called on
 * state transitions. At the moment there is only one callback type
 * for each transition event but this may change in the future in
 * the direction of having more specialized callbacks for each event.
 * at the moment i like the simplicity of having a uniform callback
 * type though. So we'll see what happens as RethinkIRCd develops
 */
typedef int (*irc_parser_cb)(irc_parser*, const char *at, size_t len);

/**
 * These are the different parsing states that the parser can be in.
 * We represent them as a zero-indexed enum that will be stored in
 * the parser context so we can have a reentrant state machine.
 */
enum irc_parser_state {
  IRC_STATE_INIT = 0,
  IRC_STATE_NICK,
  IRC_STATE_NAME,
  IRC_STATE_HOST,
  IRC_STATE_COMMAND,
  IRC_STATE_PARAMS,
  IRC_STATE_TRAILING,
  IRC_STATE_END,
  IRC_STATE_ERROR
};

/**
 * This is the enum type we use within the irc_parser lib to identify
 * different parse and user errors that can be generated by the lib.
 * The string mappings for these errors can be found in `irc_parser.c`
 */
enum irc_parser_error {
  IRC_ERROR_NONE = 0,
  IRC_ERROR_PARSE,
  IRC_ERROR_LENGTH,
  IRC_ERROR_USER
};

/**
 * This is the struct that makes reentrancy into our parser clean and
 * possible. It doesn't matter how you allocate the parser as long as
 * you pass it through it's constructor `irc_parser_init()` before
 * using it. Once the parser has been initialized callbacks must be
 * assigned to it using the propper `irc_parser_on_*()` functions.
 * Once all callbacks are set you may begin parsing incoming IRC
 * messages in chunks as the data comes in by calling `irc_parser_execute()`
 * on the data. As the parser transitions states it will call the
 * associated callbacks the user assigned. Once the end of the IRC message
 * has been reached the "on_end" callback will be fired and the parser will
 * reset and be ready to parse the next incoming IRC message.
 */
struct irc_parser_s {
  int len;
  int last;
  enum irc_parser_state state;
  enum irc_parser_error error;
  char raw[513];
  irc_parser_cb on_nick;
  irc_parser_cb on_name;
  irc_parser_cb on_host;
  irc_parser_cb on_command;
  irc_parser_cb on_param;
  irc_parser_cb on_end;
};

/**
 * This function is the constructor for the `irc_parser` struct. It
 * should be called on all newly allocated `irc_parser`s once before
 * they are used.
 * 
 * @param irc_parser *parser - The parser to initialize
 * @returns void
 */
void irc_parser_init(irc_parser *parser);

/**
 * This function is used to reset the internal state of a parser
 * without reseting the assigned callbacks. It's useful for recovering
 * parsers from error states once user clean up has been done.
 *
 * @param irc_parser *parser - The parser to reset
 * @returns void
 */
void irc_parser_reset(irc_parser *parser);

/**
 * This is the main parsing function. You supply it with a parser
 * context, some data to parse and the length of that data and it
 * will parse it firing off assigned callbacks as it transitions
 * between states. It returns a `size_t` and when the returned value
 * is not identical to the `len` parameter this is indicative of an
 * error. To check for the type of error and the error string see
 * `irc_parser_get_error()` and `irc_parser_error_string()` respectively
 *
 * @param irc_parser *parser - The parser context to apply `data` to
 * @param const char *data   - The data to parse
 * @param size_t             - The length of `data`
 * @returns size_t           - The number of bytes parsed on success
 */
size_t irc_parser_execute(irc_parser *parser, const char *data, size_t len);

/**
 * This binds a callback to the nick state transformation
 *
 * @param irc_parser *parser - The parser to bind `cb` to
 * @param irc_parser_cb cb   - The callback to bind
 * @returns void
 */
void irc_parser_on_nick(irc_parser *parser, irc_parser_cb cb);

/**
 * This binds a callback to the name state transformation
 *
 * @param irc_parser *parser - The parser to bind `cb` to
 * @param irc_parser_cb cb   - The callback to bind
 * @returns void
 */
void irc_parser_on_name(irc_parser *parser, irc_parser_cb cb);

/**
 * This binds a callback to the host state transformation
 *
 * @param irc_parser *parser - The parser to bind `cb` to
 * @param irc_parser_cb cb   - The callback to bind
 * @returns void
 */
void irc_parser_on_host(irc_parser *parser, irc_parser_cb cb);

/**
 * This binds a callback to the command state transformation
 *
 * @param irc_parser *parser - The parser to bind `cb` to
 * @param irc_parser_cb cb   - The callback to bind
 * @returns void
 */
void irc_parser_on_command(irc_parser *parser, irc_parser_cb cb);

/**
 * This binds a callback to the param state transformation
 *
 * @param irc_parser *parser - The parser to bind `cb` to
 * @param irc_parser_cb cb   - The callback to bind
 * @returns void
 */
void irc_parser_on_param(irc_parser *parser, irc_parser_cb cb);

/**
 * This binds a callback to the message end state transformation
 *
 * @param irc_parser *parser - The parser to bind `cb` to
 * @param irc_parser_cb cb   - The callback to bind
 * @returns void
 */
void irc_parser_on_end(irc_parser *parser, irc_parser_cb cb);

/**
 * This function takes a parser and returns non zero if the 
 * parser is currently in an error state.
 *
 * @param irc_parser *parser - The parser to check for errors
 * @returns void
 */
int irc_parser_has_error(irc_parser *parser);

/**
 * This function returns the enumerated value for the current
 * error state of the supplied parser.
 *
 * @param irc_parser *parser       - The parser to pull the error code from
 * @returns enum irc_parser_errors - The enumerated error code
 */
enum irc_parser_error irc_parser_get_error(irc_parser *parser);

/**
 * The function returns the error string associated to the current
 * error in the supplied parser. If no error is present it returns
 * `NULL` instead.
 *
 * @param irc_parser *parser - The parser to get the error string from
 * @returns const char*      - The associated error string.  
 */
const char* irc_parser_error_string(irc_parser *parser);

#endif irc_parser_h