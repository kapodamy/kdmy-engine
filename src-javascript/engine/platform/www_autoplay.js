"use strict";
async function www_autoplay() {
    const TEST = "data:audio/wav;base64,UklGRiQAAABXQVZFZm10IBAAAAABAAEARKwAAIhYAQACABAAZGF0YQAAAAA=";
    let audio = new Audio(TEST);
    let enabled = await (new Promise(function (resolve, reject) {
        audio = new Audio(TEST);
        audio.oncanplay = function () {
            audio.oncanplay = audio.onerror = null;
            audio.play().then(
                function () {
                    resolve(true)
                }, function (e) {
                    console.warn("media autoplay is probably disabled", e.name);
                    resolve(false);
                }
            );
        };
        audio.onerror = function (e) {
            audio.oncanplay = audio.onerror = null;
            console.error("www_autoplay()", audio.error);
            resolve(true);
        };
    }));

    if (enabled) return;

    await (new Promise(function (resolve, reject) {
        let button = document.createElement("button");
        button.style.display = "block";
        button.style.margin = "15px";
        button.textContent = "Your web browser was media autoplay was disabled. Continue";
        button.addEventListener("click", function (e) {
            button.remove();
            audio.play().catch(reject);
            audio.onplay = function (e) {
                resolve();
            }
        }, false);
        document.body.prepend(button);
    }));
}

