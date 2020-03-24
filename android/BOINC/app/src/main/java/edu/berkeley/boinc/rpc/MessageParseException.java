package edu.berkeley.boinc.rpc;

/**
 * Exception to be thrown by MessagesParser if an issue occurs while parsing.
 */
class MessageParseException extends Exception {
    MessageParseException() {
        super("Can't parse messages");
    }
}
