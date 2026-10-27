@echo off
REM Build the FFPR native screen reader (hid.dll proxy). Builds ALWAYS go through this script.
REM Treat negative Windows exception/stack-overflow exit codes as failures too.
REM Nothing is written to any game folder: installing is deploy.bat, a separate step.
setlocal
cd /d "%~dp0"

set "CMAKE=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not exist "%CMAKE%" (
    echo ERROR: bundled CMake not found at:
    echo   %CMAKE%
    exit /b 1
)
if not exist build mkdir build

echo Configuring...
python tools\openal_dependency.py check
if not "%errorlevel%"=="0" exit /b 1
"%CMAKE%" -S . -B build -A x64 > build_log.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Configure FAILED - see build_log.txt
    exit /b 1
)

echo Building...
set "CLEAN_BUILD="
if /I "%~1"=="--clean" set "CLEAN_BUILD=--clean-first"
"%CMAKE%" --build build --config Release %CLEAN_BUILD% -- /m:4 /nr:false /v:minimal >> build_log.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Build FAILED - see build_log.txt
    exit /b 1
)
if not exist "out\hid.dll" (
    echo Build reported success but out\hid.dll is missing.
    exit /b 1
)

echo Checking routes avoid unintended map transitions...
"out\checks\enterprise_flight_check.exe" > build_check\enterprise_flight_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Enterprise flight check FAILED - see build_check\enterprise_flight_report.txt
    exit /b 1
)
"out\checks\vehicle_transition_check.exe" > build_check\vehicle_transition_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Vehicle transition route check FAILED - see build_check\vehicle_transition_report.txt
    exit /b 1
)

echo Checking chocobo routing and Tower of Owen approaches...
"out\checks\chocobo_owen_check.exe" > build_check\chocobo_owen_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Chocobo/Owen check FAILED - see build_check\chocobo_owen_report.txt
    exit /b 1
)

echo Checking FFIII chocobo circuit controller and production routes...
"out\checks\chocobo_circuit_check.exe" > build_check\chocobo_circuit_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\chocobo_circuit_route_check.exe" > build_check\chocobo_circuit_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1

echo Checking exports and imports...
python tools\import_check.py out\hid.dll
if not "%errorlevel%"=="0" exit /b 1

echo Checking that hooks stack with MelonLoader / BepInEx style hooks...
"out\checks\timing_check.exe" > build_check\timing_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Timing diagnostics check FAILED - see build_check\timing_report.txt
    exit /b 1
)
"out\checks\coexist_check.exe" > build_check\coexist_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Coexistence check FAILED - see build_check\coexist_report.txt
    exit /b 1
)

echo Checking the baked-map router against FF2 and FF1 routes known from the dumps...
"out\checks\cursor_handoff_check.exe" > build_check\cursor_handoff_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Cursor handoff check FAILED - see build_check\cursor_handoff_report.txt
    exit /b 1
)
python tools\ur_locations_check.py > build_check\ur_locations_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Ur location check FAILED - see build_check\ur_locations_report.txt
    exit /b 1
)
python tools\map_entry_floors.py > build_check\map_entry_floors_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native entrance floor audit FAILED - see build_check\map_entry_floors_report.txt
    exit /b 1
)
"out\checks\map_names_check.exe" > build_check\map_names_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native floor resolver check FAILED - see build_check\map_names_report.txt
    exit /b 1
)
"out\checks\map_exit_check.exe" > build_check\map_exit_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Map exit check FAILED - see build_check\map_exit_report.txt
    exit /b 1
)
"out\checks\route_check.exe" "data\ff2\maps.bin" "data\ff1\maps.bin" > build_check\route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Route check FAILED - see build_check\route_report.txt
    exit /b 1
)
"out\checks\route_worker_check.exe" > build_check\route_worker_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Route worker check FAILED - see build_check\route_worker_report.txt
    exit /b 1
)
"out\checks\npc_geometry_check.exe" > build_check\npc_geometry_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo All-six NPC geometry audit FAILED - see build_check\npc_geometry_report.txt
    exit /b 1
)
"out\checks\locked_door_check.exe" > build_check\locked_door_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Locked-door interaction check FAILED - see build_check\locked_door_report.txt
    exit /b 1
)
"out\checks\warehouse_check.exe" > build_check\warehouse_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Warehouse accessibility check FAILED - see build_check\warehouse_report.txt
    exit /b 1
)
"out\checks\remote_exit_check.exe" > build_check\remote_exit_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Remote return-exit check FAILED - see build_check\remote_exit_report.txt
    exit /b 1
)
"out\checks\npc_floors_check.exe" > build_check\npc_floors_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Connected NPC floor check FAILED - see build_check\npc_floors_report.txt
    exit /b 1
)
"out\checks\npc_gate_check.exe" > build_check\npc_gate_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo NPC and story-gate route check FAILED - see build_check\npc_gate_report.txt
    exit /b 1
)

"out\checks\field_targets_check.exe" > build_check\field_targets_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Field magic range check FAILED - see build_check\field_targets_report.txt
    exit /b 1
)
python tools\npc_patrol_check.py > build_check\npc_patrol_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo All-six NPC movement metadata check FAILED - see build_check\npc_patrol_report.txt
    exit /b 1
)
"out\checks\event_item_select_check.exe" > build_check\event_item_select_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Event-item Autodetail check FAILED - see build_check\event_item_select_report.txt
    exit /b 1
)
"out\checks\navigation_followup_check.exe" > build_check\navigation_followup_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Scripted exit, Ship and indoor graph check FAILED - see build_check\navigation_followup_report.txt
    exit /b 1
)

"out\checks\puzzle_guide_check.exe" > build_check\puzzle_guide_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Puzzle guide check FAILED - see build_check\puzzle_guide_report.txt
    exit /b 1
)
"out\checks\ff6_story_check.exe" > build_check\ff6_story_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF6 story guide check FAILED - see build_check\ff6_story_report.txt
    exit /b 1
)
"out\checks\ff6_story_runtime_check.exe" > build_check\ff6_story_runtime_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF6 story runtime check FAILED - see build_check\ff6_story_runtime_report.txt
    exit /b 1
)
"out\checks\party_entities_check.exe" > build_check\party_entities_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native parked-party check FAILED - see build_check\party_entities_report.txt
    exit /b 1
)
"out\checks\ff6_defense_check.exe" > build_check\ff6_defense_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF6 defense advice check FAILED - see build_check\ff6_defense_report.txt
    exit /b 1
)
"out\checks\ff6_opera_check.exe" > build_check\ff6_opera_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF6 opera producer check FAILED - see build_check\ff6_opera_report.txt
    exit /b 1
)
"out\checks\fade_ready_check.exe" > build_check\fade_ready_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Fade readiness check FAILED - see build_check\fade_ready_report.txt
    exit /b 1
)
"out\checks\trap_route_check.exe" > build_check\trap_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Trap routing check FAILED - see build_check\trap_route_report.txt
    exit /b 1
)
"out\checks\transition_route_check.exe" > build_check\transition_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Transition route check FAILED - see build_check\transition_route_report.txt
    exit /b 1
)
"out\checks\event_semantics_check.exe" > build_check\event_semantics_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Event semantics check FAILED - see build_check\event_semantics_report.txt
    exit /b 1
)

"out\checks\teleport_check.exe" > build_check\teleport_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Teleport check FAILED - see build_check\teleport_report.txt
    exit /b 1
)

echo Checking every game-code name against each game's dump.cs...
"out\checks\navigation_groups_check.exe" > build_check\navigation_groups_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Navigation groups check FAILED - see build_check\navigation_groups_report.txt
    exit /b 1
)
"out\checks\story_filter_check.exe" > build_check\story_filter_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Story filter check FAILED - see build_check\story_filter_report.txt
    exit /b 1
)
"out\checks\name_input_check.exe" > build_check\name_input_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native name-input check FAILED - see build_check\name_input_report.txt
    exit /b 1
)
"out\checks\enemy_parts_check.exe" > build_check\enemy_parts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Boss part identity check FAILED - see build_check\enemy_parts_report.txt
    exit /b 1
)
"out\checks\autowalk_check.exe" > build_check\autowalk_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Autowalk check FAILED - see build_check\autowalk_report.txt
    exit /b 1
)
"out\checks\autowalk_native_check.exe" > build_check\autowalk_native_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Autowalk native input check FAILED - see build_check\autowalk_native_report.txt
    exit /b 1
)
"out\checks\active_controller_check.exe" > build_check\active_controller_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Active controller check FAILED - see build_check\active_controller_report.txt
    exit /b 1
)
"out\checks\autowalk_steering_check.exe" > build_check\autowalk_steering_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native steering check FAILED - see build_check\autowalk_steering_report.txt
    exit /b 1
)
"out\checks\bindings_check.exe" > build_check\bindings_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Input binding check FAILED - see build_check\bindings_report.txt
    exit /b 1
)
"out\checks\equipment_party_check.exe" > build_check\equipment_party_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Equipment and party check FAILED - see build_check\equipment_party_report.txt
    exit /b 1
)
"out\checks\mapstate_check.exe" > build_check\mapstate_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Map-state check FAILED - see build_check\mapstate_report.txt
    exit /b 1
)
"out\checks\mapstate_native_check.exe" > build_check\mapstate_native_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native map-state check FAILED - see build_check\mapstate_native_report.txt
    exit /b 1
)
"out\checks\frame_check.exe" > build_check\frame_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Scheduler check FAILED - see build_check\frame_report.txt
    exit /b 1
)
"out\checks\log_worker_check.exe" > build_check\log_worker_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Logger worker check FAILED - see build_check\log_worker_report.txt
    exit /b 1
)
"out\checks\audio_worker_check.exe" > build_check\audio_worker_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Audio worker check FAILED - see build_check\audio_worker_report.txt
    exit /b 1
)
"out\checks\soundscape_check.exe" > build_check\soundscape_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Soundscape check FAILED - see build_check\soundscape_report.txt
    exit /b 1
)
"out\checks\beacon_check.exe" > build_check\beacon_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Beacon mode check FAILED - see build_check\beacon_report.txt
    exit /b 1
)

"out\checks\sound_player_check.exe" > build_check\sound_player_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Cue producer check FAILED - see build_check\sound_player_report.txt
    exit /b 1
)
"out\checks\controller_worker_check.exe" > build_check\controller_worker_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Controller worker check FAILED - see build_check\controller_worker_report.txt
    exit /b 1
)
"out\checks\controller_worker_check.exe" failure >> build_check\controller_worker_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Controller failure check FAILED - see build_check\controller_worker_report.txt
    exit /b 1
)
"out\checks\battle_context_check.exe" > build_check\battle_context_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Battle context check FAILED - see build_check\battle_context_report.txt
    exit /b 1
)
"out\checks\battle_entry_check.exe" > build_check\battle_entry_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Battle entry check FAILED - see build_check\battle_entry_report.txt
    exit /b 1
)
"out\checks\battle_formation_check.exe" > build_check\battle_formation_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Battle formation check FAILED - see build_check\battle_formation_report.txt
    exit /b 1
)
"out\checks\battle_focus_check.exe" > build_check\battle_focus_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Battle focus check FAILED - see build_check\battle_focus_report.txt
    exit /b 1
)
"out\checks\save_load_check.exe" > build_check\save_load_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Save path check FAILED - see build_check\save_load_report.txt
    exit /b 1
)
python tools\save_paths_audit.py > build_check\save_paths_native_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo All-six native save path audit FAILED - see build_check\save_paths_native_report.txt
    exit /b 1
)
"out\checks\command_bar_check.exe" > build_check\command_bar_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Command preload context check FAILED - see build_check\command_bar_report.txt
    exit /b 1
)
"out\checks\job_menu_check.exe" > build_check\job_menu_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FFIII job equipment detail check FAILED - see build_check\job_menu_report.txt
    exit /b 1
)
"out\checks\party_esper_check.exe" > build_check\party_esper_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Party and Esper check FAILED - see build_check\party_esper_report.txt
    exit /b 1
)
"out\checks\shop_party_check.exe" > build_check\shop_party_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Shop portrait comparison check FAILED - see build_check\shop_party_report.txt
    exit /b 1
)
"out\checks\vehicle_landing_check.exe" > build_check\vehicle_landing_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Vehicle landing check FAILED - see build_check\vehicle_landing_report.txt
    exit /b 1
)
"out\checks\startup_diagnostics_check.exe" > build_check\startup_diagnostics_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Startup diagnostics check FAILED - see build_check\startup_diagnostics_report.txt
    exit /b 1
)
"out\checks\battle_feedback_check.exe" > build_check\battle_feedback_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Battle feedback check FAILED - see build_check\battle_feedback_report.txt
    exit /b 1
)
"out\checks\battle_results_check.exe" > build_check\battle_results_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Battle results check FAILED - see build_check\battle_results_report.txt
    exit /b 1
)
"out\checks\native_cues_check.exe" > build_check\native_cues_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native presentation cues check FAILED - see build_check\native_cues_report.txt
    exit /b 1
)
"out\checks\interception_check.exe" > build_check\interception_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Control interception check FAILED - see build_check\interception_report.txt
    exit /b 1
)
"out\checks\map_panels_check.exe" > build_check\map_panels_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Map panels check FAILED - see build_check\map_panels_report.txt
    exit /b 1
)
"out\checks\key_help_check.exe" > build_check\key_help_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Screen-owned controls check FAILED - see build_check\key_help_report.txt
    exit /b 1
)
"out\checks\bestiary_check.exe" > build_check\bestiary_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Bestiary check FAILED - see build_check\bestiary_report.txt
    exit /b 1
)
python tools\enemy_status_audit.py --check > build_check\enemy_status_audit_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Enemy status data check FAILED - see build_check\enemy_status_audit_report.txt
    exit /b 1
)
python tools\condition_labels_check.py > build_check\condition_labels_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Condition label audit FAILED - see build_check\condition_labels_report.txt
    exit /b 1
)
python tools\oct24_navigation_check.py > build_check\oct24_navigation_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo October24 navigation catalog/data check FAILED - see build_check\oct24_navigation_report.txt
    exit /b 1
)
python tools\oct22_catalog_check.py > build_check\oct22_catalog_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo October22 catalog check FAILED - see build_check\oct22_catalog_report.txt
    exit /b 1
)
python tools\event_identity_check.py > build_check\event_identity_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Object identity coverage check FAILED - see build_check\event_identity_report.txt
    exit /b 1
)
python tools\event_warp_check.py > build_check\event_warp_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Reviewed warp label check FAILED - see build_check\event_warp_report.txt
    exit /b 1
)
"out\checks\prism_check.exe" > build_check\prism_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Prism check FAILED - see build_check\prism_report.txt
    exit /b 1
)
"out\checks\speech_worker_check.exe" > build_check\speech_worker_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Speech worker check FAILED - see build_check\speech_worker_report.txt
    exit /b 1
)
"out\checks\navigation_speech_check.exe" > build_check\navigation_speech_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Navigation speech check FAILED - see build_check\navigation_speech_report.txt
    exit /b 1
)
"out\checks\browse_route_check.exe" > build_check\browse_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Browse route check FAILED - see build_check\browse_route_report.txt
    exit /b 1
)
"out\checks\event_tracking_check.exe" > build_check\event_tracking_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Event tracking check FAILED - see build_check\event_tracking_report.txt
    exit /b 1
)
"out\checks\modal_speech_check.exe" > build_check\modal_speech_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Modal speech check FAILED - see build_check\modal_speech_report.txt
    exit /b 1
)
"out\checks\creation_fields_check.exe" > build_check\creation_fields_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Character creation field check FAILED - see build_check\creation_fields_report.txt
    exit /b 1
)
"out\checks\title_prompt_check.exe" > build_check\title_prompt_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Title prompt check FAILED - see build_check\title_prompt_report.txt
    exit /b 1
)
"out\checks\settings_worker_check.exe" > build_check\settings_worker_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Settings worker check FAILED - see build_check\settings_worker_report.txt
    exit /b 1
)
"out\checks\entity_filter_check.exe" > build_check\entity_filter_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Entity filter check FAILED - see build_check\entity_filter_report.txt
    exit /b 1
)
"out\checks\gysahl_elder_check.exe" > build_check\gysahl_elder_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Gysahl Elder check FAILED - see build_check\gysahl_elder_report.txt
    exit /b 1
)
"out\checks\entity_scanner_check.exe" > build_check\entity_scanner_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Entity scanner check FAILED - see build_check\entity_scanner_report.txt
    exit /b 1
)
"out\checks\transition_input_check.exe" > build_check\transition_input_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Transition input check FAILED - see build_check\transition_input_report.txt
    exit /b 1
)
"out\checks\navigation_worker_check.exe" > build_check\navigation_worker_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Navigation worker check FAILED - see build_check\navigation_worker_report.txt
    exit /b 1
)
"out\checks\navigation_perf_check.exe" > build_check\navigation_perf_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Navigation performance check FAILED - see build_check\navigation_perf_report.txt
    exit /b 1
)
"out\checks\component_cache_check.exe" > build_check\component_cache_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Component cache check FAILED - see build_check\component_cache_report.txt
    exit /b 1
)
"out\checks\vehicle_state_check.exe" > build_check\vehicle_state_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Vehicle state check FAILED - see build_check\vehicle_state_report.txt
    exit /b 1
)
python tools\prism_package_check.py > build_check\prism_package_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Prism package check FAILED - see build_check\prism_package_report.txt
    exit /b 1
)
python tools\prism_dependency_check.py > build_check\prism_dependency_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Prism dependency check FAILED - see build_check\prism_dependency_report.txt
    exit /b 1
)
"out\checks\modal_input_check.exe" > build_check\modal_input_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Modal input check FAILED - see build_check\modal_input_report.txt
    exit /b 1
)
"out\checks\npc_names_check.exe" > build_check\npc_names_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo NPC naming check FAILED - see build_check\npc_names_report.txt
    exit /b 1
)
for %%G in (ff1 ff2 ff3 ff4 ff5 ff6) do (
    "out\checks\npc_label_audit_check.exe" %%G > build_check\%%G_npc_labels_report.txt 2>&1
    if errorlevel 1 (
        echo NPC label audit FAILED for %%G - see build_check\%%G_npc_labels_report.txt
        exit /b 1
    )
    if not errorlevel 0 (
        echo NPC label audit CRASHED for %%G - see build_check\%%G_npc_labels_report.txt
        exit /b 1
    )
)
python tools\name_check.py
if not "%errorlevel%"=="0" (
    echo Name check FAILED - see build_check\name_report.txt
    exit /b 1
)
REM Report only: a few names are optional by design (FF1's default when undefined).
python tools\wiring_check.py
python tools\soundscape_assets_check.py > build_check\soundscape_assets_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Embedded soundscape asset check FAILED - see build_check\soundscape_assets_report.txt
    exit /b 1
)

echo.
echo Build OK: out\hid.dll
for %%F in ("out\hid.dll") do echo   %%~zF bytes
echo NOT deployed. Run smoketest.bat, then deploy.bat.
endlocal
