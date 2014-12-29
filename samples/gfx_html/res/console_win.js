// Remap console functions if the console object doesn't exist in the correct
// location
//

if (!(window.console)) {
    console = {};

    console.log = function (message) {
        window.external.log(message.toString());
    };

    console.debug = function (message) {
        window.external.debug(message.toString());
    };

    console.info = function (message) {
        window.external.info(message.toString());
    };

    console.warn = function (message) {
        window.external.warn(message.toString());
    };

    console.error = function (message) {
        window.external.error(message.toString());
    };
}
