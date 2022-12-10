const MILF_ZOOMS_STAGE_INCREMENT:number = 0.018;
const MILF_ZOOMS_UI_INCREMENT:number = 0.0003;

let fastCarCanDrive: boolean = true;
let limo: Layout = null;
let cameraUI: Camera = null;
let cameraStage: Camera = null;
let isMILF: boolean = false;
let incrementalUIZoomMILF: number = 0;
let incrementalStageZoomMILF: number = 0;

function f_beforeready(): void {
    fastCarCanDrive = true;
    limo = week_get_stage_layout();
    cameraUI = week_ui_get_camera();
    cameraStage = limo.get_camera_helper();

    let [track_name, track_difficult, track_index] = week_get_current_track_info();
    print("track: " + track_name);
    isMILF = track_name == "Milf";
    incrementalUIZoomMILF = 1.05;
    incrementalStageZoomMILF = 1.09;
}

function f_beat(curBeat: number, since: number): void {
    if (isMILF && curBeat >= 168 && curBeat < 200 && incrementalUIZoomMILF < 1.35) {
        // NON HARDCODING FOR MILF ZOOMS! :)
        let oldZoom = incrementalStageZoomMILF;
        incrementalStageZoomMILF = incrementalStageZoomMILF + MILF_ZOOMS_STAGE_INCREMENT;
        cameraStage.slide_z(oldZoom, incrementalStageZoomMILF);

        cameraUI.slide_z(1.0, incrementalUIZoomMILF);
        incrementalUIZoomMILF = incrementalUIZoomMILF + MILF_ZOOMS_UI_INCREMENT;
    }

    if (math2d_random_boolean(10) && fastCarCanDrive) {
        let random_sound: string = math2d_random_boolean(50) ? "carPass0" : "carPass1";

        // animate car sprite and play a sound
        fastCarCanDrive = false;
        limo.trigger_action(null, random_sound);
        limo.trigger_action(null, "carPass");

        // 2 seconds cooldown
        timer_callback_timeout(2000, resetFastCar);
    }
}

function resetFastCar(): void {
    fastCarCanDrive = false;
}

