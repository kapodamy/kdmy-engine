
function test_timeout_error(arg: string) {
    error(arg);
}

function test_timeout(arg: string) {
    print("test_timeout, arg was: " + arg);
    timer_callback_timeout(1000, test_timeout_error, "this is a error");
    timer_callback_timeout(1000, function () { print("post-error function") }, "this is a error");
    let to_cancel = timer_callback_timeout(2000, function () { print("canceled function") });
    timer_callback_timeout(1500, function () {
        let result: boolean = timer_callback_cancel(to_cancel);
        print("last test cancel timeout, result: " + result);
    });
}


let test_timer_var = 1;
let test_timer_id = timer_callback_interval(1000, function () {
    print(`timer_callback_interval no args ${test_timer_var}/5`);
    if (test_timer_var >= 5) {
        print(`timer_callback_interval no args ${test_timer_var}/5  END`);
        timer_callback_cancel(test_timer_id);
        timer_callback_timeout(1000, test_timeout, "abc123");
        return;
    }
    test_timer_var++;
});