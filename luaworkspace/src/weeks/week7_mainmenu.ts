
function f_modding_handle_custom_option(option_name: string): void {
    //if (option_name != "kickstarter") return;
    modding_spawn_screen(
        "/assets/weeks/week7/kickstarter/layout.xml", "/assets/weeks/week7/kickstarter/script.lua",
        null
    );
}
