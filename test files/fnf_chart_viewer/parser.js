"use strict";

var strum;
var tex_notes = new Array(4);
var tex_notes_c = new Array(4);
var last_pos = new Array(8);

function log(msg) {
    document.getElementById("info").textContent += "\n"+msg;
    console.log.apply(undefined, arguments);
}

function error(e) {
    document.getElementById("info").textContent += "\n"+e.message;
    console.error.apply(undefined, arguments);
}

async function main() {
    await load();
    
    let chart = document.getElementById("chart");
    let voices = document.getElementById("voices");
    let del_voices = document.getElementById("del-voices");
    let instruments = document.getElementById("instruments");
    let del_instruments = document.getElementById("del-instruments");
    
    chart.addEventListener("change", parse_chart, false);
    del_voices.addEventListener("click", function() {
        player_voices = null;
        voices.value = "";
    }, false);
    voices.addEventListener("change", load_voices, false);
    del_instruments.addEventListener("click", function() {
        player_ins = null;
        instruments.value = "";
    }, false);
    instruments.addEventListener("change", load_ins, false);
    
    player_start_px = placeholder.offsetTop;
    player_time_dom = document.getElementById("time");
    
    document.addEventListener("scroll", function(e) {
        if (player_playing) return;
        let value = math2d_inverselerp(player_start_px, player_end_px, document.body.scrollTop);
        let player_time = math2d_lerp(0, player_duration, value);
        if (player_time < 0) player_time = 0;
        player_time_dom.value = millisecondsToTime(player_time); 
    });

    if (chart.files[0]) parse_chart({target: chart});
    if (voices.files[0]) load_voices({target: voices});
    if (instruments.files[0]) load_ins({target: instruments});
}

async function load_ins(e) {
    player_voices = null;
    player_voices = await player_load(e.target);
}

async function load_voices(e) {
    player_ins = null;
    player_ins = await player_load(e.target);
}

async function parse_chart(e) {
        player_duration = 0;
        document.getElementById("info").textContent = "";

        for (let i=0 ; i<strum.length ; i++)
            strum[i].clearRect(0,0, strum[i].canvas.width, strum[i].canvas.height);

        if (e.target.files.length < 1) return;

        log("file: " + e.target.files[0].name);
        
        let url = URL.createObjectURL(e.target.files[0]);
        let res;

        try {
            res = await (await fetch(url)).text();
        } catch (error) {
            error(error);
            alert("Can not read the file '" + e.target.files[0].name + "':\n" + error.message);
        } finally {
            URL.revokeObjectURL(url);
        }

        let json;
        try {
            json = JSON.parse(res);
        } catch (e) {
            error(e);
            alert("Can not parse the file:\n" + e.message);
            return;
        }

        try {
            render(json);
        } catch (e) {
            error(e);
            alert("Can not render the song\n" + e.message);
            return;
        }
    }

function render(json) {
    let song;

    if (typeof(json.song) == "string")
        song = json;
    else
        song = json.song;

    log("bpm : " + song.bpm);
    log("notes : " + song.notes.length);
    log("sections : " + song.sections);
    log("speed : " + song.speed);
    log("--------------------");

    for (let i=0 ; i<strum.length ; i++) {
        strum[i].fillStyle = "gold";
        strum[i].font = "15px Consolas";
    }

    for (let i=0 ; i<song.notes.length ; i++) {
        if (song.notes[i].typeOfSection != null && song.notes[i].typeOfSection != 0)
            log("song.notes[" + i + "].typeOfSection " + song.notes[i].typeOfSection);

        if (song.notes[i].lengthInSteps != 16)
            log("song.notes[" + i + "].lengthInSteps " + song.notes[i].lengthInSteps);

        for (let j=0 ; j<song.notes[i].sectionNotes.length ; j++) {
            for (let k=0 ; k<song.notes[i].sectionNotes[j].length ; k++) {
                add_note(
                    song.notes[i].sectionNotes[j][0],
                    song.notes[i].sectionNotes[j][1],
                    song.notes[i].sectionNotes[j][2],
                    song.notes[i].mustHitSection
                );
            }
        }
    }

    let max_pos = 0;
    for (let i=0 ; i<last_pos.length ; i++) {
        if (last_pos[i] > max_pos)
            max_pos = last_pos[i];
    }

    max_pos += 75;
    for (let i=0 ; i<strum.length ; i++) {
        strum[i].fillStyle = "black";
        strum[i].font = "20px Consolas";
        strum[i].fillText("-END-", -2, max_pos);
    }
}

function add_note(timestamp, id, duration, alt) {

    let pos = timestamp/5;
    let dur = parseInt(duration/5);
    let chour = id > 3;

    if (chour) {
        if (id >= 8) {
            log(`EVENT: timestamp=${timestamp} id=${id} duration=${duration} mustHitSection=${alt}`);
            for (let i=0 ; i<strum.length ; i++) {
                strum[i].fillText("event:", 0, pos);
                strum[i].fillText(id, 0, pos + 25);
            }
            return;
        }

        id -= 4;
        alt = !alt;
    }

    let strum_id = draw_note(id, pos, alt);

    player_duration = timestamp + duration;
    player_end_px = pos + 25;
    
    if (duration != 0) {
        strum[strum_id].drawImage(tex_notes_c[id], 12.5, pos + 25, 25, dur);
        strum[strum_id].fillText(duration, 0, pos + 10);
        player_end_px += dur;
    }

    if (chour) {
        strum[strum_id].fillText("CHOUR!", 0, pos + (duration != 0 ? 25 : 0));
    }
}

function draw_note(id, pos, alt) {
    let strum_id = id;
    if (alt) strum_id += 4;
    last_pos[strum_id] = pos;

    strum[strum_id].drawImage(tex_notes[id], 12.5, pos, 25, 25);
    return strum_id;
}

async function read_img(src) {
    return await createImageBitmap(document.querySelector(`img[src='${src}.png']`));
}

async function load() {
    tex_notes[0] = await read_img("left");
    tex_notes[1] = await read_img("down");
    tex_notes[2] = await read_img("up");
    tex_notes[3] = await read_img("right");
    tex_notes_c[0] = await read_img("left_c");
    tex_notes_c[1] = await read_img("down_c");
    tex_notes_c[2] = await read_img("up_c");
    tex_notes_c[3] = await read_img("right_c");

    strum = [
        document.getElementById("strum_a_0").getContext("2d"),
        document.getElementById("strum_a_1").getContext("2d"),
        document.getElementById("strum_a_2").getContext("2d"),
        document.getElementById("strum_a_3").getContext("2d"),

        document.getElementById("strum_b_0").getContext("2d"),
        document.getElementById("strum_b_1").getContext("2d"),
        document.getElementById("strum_b_2").getContext("2d"),
        document.getElementById("strum_b_3").getContext("2d")
    ];
    
    document.getElementById("play").addEventListener("click", player_start, false);
    document.getElementById("stop").addEventListener("click", player_stop,false);
}


function math2d_lerp(start, end, step) {
    return start + (end - start) * step;
}

function math2d_inverselerp(start, end, value) {
    return (value - start) / (end - start);
}


var player_ins = null;
var player_voices = null;
var player_last = 0;
var player_duration = 0;
var player_start_px;
var player_end_px;
var player_playing = 0;
var player_time_dom = null;

function player_start() {
    if (player_playing ) return;
    if (player_duration <= 0) return alert("no chart loaded");
    requestAnimationFrame(player_loop);
    player_playing = 1;
    player_last = 0;
}

function player_loop(timestamp) {
    if (!player_playing) return;
    if (player_last == 0) {
        player_last = timestamp;

        if (player_voices) {
            player_voices.play();
            player_voices.currentTime = 0;
        }

        if (player_ins) {
            player_ins.play();
            player_ins.currentTime = 0;
        }

        requestAnimationFrame(player_loop);
        return;
    }

    let player_time = timestamp - player_last;
    player_time_dom.value = millisecondsToTime(player_time); 
    
    if (player_time >= player_duration) {
        player_playing = 0;
        return;
    }

    let value = math2d_inverselerp(0, player_duration, player_time);

    let progress = math2d_lerp(player_start_px, player_end_px, value);
    document.body.scrollTop = progress;

    requestAnimationFrame(player_loop);
}

function player_stop() {
    player_playing = 0;
    player_last = Infinity;
    if (player_ins) player_ins.pause();
    if (player_voices) player_voices.pause();
    cancelAnimationFrame(player_loop);
}


function player_load(input_file) {
    return new Promise(function(resolve, reject) {
        if (input_file.files.length < 1) {
            log("unloaded "+ input_file.name);
            return resolve(null);
        }

        let url = URL.createObjectURL(input_file.files[0]);
        
        let audio = new Audio(url);
        audio.preload = "auto";
        audio.volume = 0.5;
        audio.oncanplay = function (evt) {
            this.currentTime = 0;
            this.oncanplay = null;
            this.onerror = null;
            log("loaded " + input_file.files[0].name);
            URL.revokeObjectURL(url);
            resolve(audio);
        };
        audio.onerror = function (evt) {
            this.oncanplay = null;
            this.onerror = null;
            URL.revokeObjectURL(url);
            log("cannot load " + input_file.files[0].name);
            error(this.error);
            resolve(null);
        }
    });
}

function millisecondsToTime(milli) {
      var milliseconds = milli % 1000;
      var seconds = Math.floor((milli / 1000) % 60);
      var minutes = Math.floor((milli / (60 * 1000)) % 60);
      if (isNaN(seconds)) return "--:--.--";

      minutes = minutes.toString();
      seconds = seconds.toString();
      if (minutes.length < 2) minutes = "0" + minutes;
      if (seconds.length < 2) seconds = "0" + seconds;

      return minutes + ":" + seconds + "." + milliseconds.toString().substring(0, 2);
}


