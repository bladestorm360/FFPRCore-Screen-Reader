@echo off
REM Full FFPR regression build. Use build_local.bat for scoped local delivery.
REM Treat negative Windows exception/stack-overflow exit codes as failures too.
REM Nothing is written to any game folder: installing is deploy.bat, a separate step.
setlocal
cd /d "%~dp0"

REM Two machine-wide slots; two GB for each COMPLETE tree, never per compiler.
REM Re-entry is permitted only after inspecting the actual Windows job limits.
if defined CODEX_BUILD_JOB_NAME goto verify_memory_job
python scripts\run-build.py --receipt build_check\build_resource_receipt.json -- cmd /d /c "%~f0" %*
exit /b %errorlevel%
:verify_memory_job
python scripts\run-build.py --verify
if not "%errorlevel%"=="0" exit /b 1

REM Hard rule: reject recovered game source before compiling any payload.
if not exist build_check mkdir build_check
python tools\repository_content_check.py > build_check\repository_content_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Game-source boundary check FAILED - see build_check\repository_content_report.txt
    exit /b 1
)

REM Focused development check; normal build remains required for delivery.
if /I "%~1"=="--check" goto one_check

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
REM User preference: one project and one compiler at a time, even on large PCs.
REM Together with CMake's /MP1 this prevents multiplied compiler RAM pressure.
set "CMAKE_BUILD_PARALLEL_LEVEL=1"
REM Release MSBuild's retained project state between targets within the same job.
python tools\build_projects_serial.py "%CMAKE%" %CLEAN_BUILD% >> build_log.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Build FAILED - see build_log.txt
    exit /b 1
)
if not exist "out\hid.dll" (
    echo Build reported success but out\hid.dll is missing.
    exit /b 1
)

echo Checking routes avoid unintended map transitions...
"out\checks\cross_floor_npc_check.exe" > build_check\cross_floor_npc_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Cross-floor NPC guidance check FAILED - see build_check\cross_floor_npc_report.txt
    exit /b 1
)
"out\checks\map_geometry_check.exe" --verify-relays > build_check\map_geometry_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native counter geometry check FAILED - see build_check\map_geometry_report.txt
    exit /b 1
)
"out\checks\saronia_entry_check.exe" > build_check\saronia_entry_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Saronia entry check FAILED - see build_check\saronia_entry_report.txt
    exit /b 1
)
"out\checks\enterprise_landing_check.exe" > build_check\enterprise_landing_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Enterprise landing check FAILED - see build_check\enterprise_landing_report.txt
    exit /b 1
)
"out\checks\continent_transition_check.exe" > build_check\continent_transition_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Continent transition check FAILED - see build_check\continent_transition_report.txt
    exit /b 1
)
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

echo Checking native Invincible mountain actions and Falgabard access...
"out\checks\invincible_route_check.exe" > build_check\invincible_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Invincible crossing check FAILED - see build_check\invincible_route_report.txt
    exit /b 1
)

echo Checking ordinary-flight preference...
"out\checks\invincible_preference_check.exe" > build_check\invincible_preference_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Invincible preference check FAILED - see build_check\invincible_preference_report.txt
    exit /b 1
)

echo Checking chocobo routing and Tower of Owen approaches...
"out\checks\invincible_artillery_check.exe" > build_check\invincible_artillery_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Invincible artillery check FAILED - see build_check\invincible_artillery_report.txt
    exit /b 1
)
"out\checks\command_commit_check.exe" > build_check\command_commit_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Command commit focus check FAILED - see build_check\command_commit_report.txt
    exit /b 1
)
"out\checks\torna_canal_check.exe" > build_check\torna_canal_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Torna Canal check FAILED - see build_check\torna_canal_report.txt
    exit /b 1
)
"out\checks\field_effect_check.exe" > build_check\field_effect_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Field effect collision check FAILED - see build_check\field_effect_report.txt
    exit /b 1
)
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
"out\checks\eureka_destination_check.exe" > build_check\eureka_destination_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Eureka remote weapon check FAILED - see build_check\eureka_destination_report.txt
    exit /b 1
)

"out\checks\vehicle_journey_check.exe" > build_check\vehicle_journey_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo vehicle_journey check FAILED - see build_check\vehicle_journey_report.txt
    exit /b 1
)

"out\checks\ff1_vehicle_goal_check.exe" > build_check\ff1_vehicle_goal_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF1 selected vehicle goal check FAILED - see build_check\ff1_vehicle_goal_report.txt
    exit /b 1
)

"out\checks\ff1_sunken_shrine_check.exe" > build_check\ff1_sunken_shrine_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF1 Sunken Shrine check FAILED - see build_check\ff1_sunken_shrine_report.txt
    exit /b 1
)

"out\checks\world_map_journey_check.exe" > build_check\world_map_journey_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo world_map_journey check FAILED - see build_check\world_map_journey_report.txt
    exit /b 1
)
"out\checks\ff3_world_request_check.exe" > build_check\ff3_world_request_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FFIII published world request check FAILED - see build_check\ff3_world_request_report.txt
    exit /b 1
)
"out\checks\world_flood_check.exe" > build_check\world_flood_report.txt 2>&1
if errorlevel 1 (
    echo World graph equivalence check FAILED - see build_check\world_flood_report.txt
    exit /b 1
)
"out\checks\mist_route_check.exe" > build_check\mist_route_report.txt 2>&1
if errorlevel 1 (
    echo Mist foot route check FAILED - see build_check\mist_route_report.txt
    exit /b 1
)
"out\checks\moving_object_goal_check.exe" > build_check\moving_object_goal_report.txt 2>&1
if errorlevel 1 (
    echo Moving native object destination check FAILED - see build_check\moving_object_goal_report.txt
    exit /b 1
)
"out\checks\world_mounted_hop_check.exe" > build_check\world_mounted_hop_report.txt 2>&1
if errorlevel 1 (
    echo Mounted world first-hop check FAILED - see build_check\world_mounted_hop_report.txt
    exit /b 1
)
"out\checks\world_mounted_distance_check.exe" > build_check\world_mounted_distance_report.txt 2>&1
if errorlevel 1 (
    echo Longer mounted first-hop check FAILED - see build_check\world_mounted_distance_report.txt
    exit /b 1
)
"out\checks\underwater_interior_check.exe" > build_check\underwater_interior_report.txt 2>&1
if errorlevel 1 (
    echo FFIII underwater interior continuation check FAILED - see build_check\underwater_interior_report.txt
    exit /b 1
)
"out\checks\world_player_hop_check.exe" > build_check\world_player_hop_report.txt 2>&1
if errorlevel 1 (
    echo Native Player first-hop check FAILED - see build_check\world_player_hop_report.txt
    exit /b 1
)

"out\checks\saved_vehicles_check.exe" > build_check\saved_vehicles_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo saved_vehicles check FAILED - see build_check\saved_vehicles_report.txt
    exit /b 1
)

echo Checking Floating Continent navigation cost and native capture ownership...
"out\checks\floating_navigation_check.exe" > build_check\floating_navigation_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Floating Continent navigation check FAILED - see build_check\floating_navigation_report.txt
    exit /b 1
)
"out\checks\floating_capture_check.exe" > build_check\floating_capture_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Fixed Floating Continent capture check FAILED - see build_check\floating_capture_report.txt
    exit /b 1
)
"out\checks\baked_string_check.exe" > build_check\baked_string_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Prepared map identity check FAILED - see build_check\baked_string_report.txt
    exit /b 1
)
python tools\floating_border_contract_check.py > build_check\floating_border_contract_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Fixed border source contract FAILED - see build_check\floating_border_contract_report.txt
    exit /b 1
)
"out\checks\entity_cursor_check.exe" > build_check\entity_cursor_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native collection cursor check FAILED - see build_check\entity_cursor_report.txt
    exit /b 1
)

python tools\re\arrival_contexts.py > build_check\arrival_contexts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native arrival context check FAILED - see build_check\arrival_contexts_report.txt
    exit /b 1
)
"out\checks\arrival_choice_check.exe" > build_check\arrival_choice_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native arrival choice check FAILED - see build_check\arrival_choice_report.txt
    exit /b 1
)
"out\checks\transport_guard_check.exe" > build_check\transport_guard_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native transportation selector and trigger check FAILED - see build_check\transport_guard_report.txt
    exit /b 1
)
"out\checks\contextual_scanner_check.exe" > build_check\contextual_scanner_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Contextual event native and remote output check FAILED - see build_check\contextual_scanner_report.txt
    exit /b 1
)
python tools\re\final_script_cases.py > build_check\final_script_cases_source_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native script source and continuation check FAILED - see build_check\final_script_cases_source_report.txt
    exit /b 1
)
"out\checks\final_script_cases_check.exe" > build_check\final_script_cases_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native script actor publication check FAILED - see build_check\final_script_cases_report.txt
    exit /b 1
)
"out\checks\conditional_transfer_check.exe" > build_check\conditional_transfer_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Conditional native door check FAILED - see build_check\conditional_transfer_report.txt
    exit /b 1
)
"out\checks\devil_road_check.exe" > build_check\devil_road_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Chained native teleport landing check FAILED - see build_check\devil_road_report.txt
    exit /b 1
)
"out\checks\contextual_submarine_check.exe" > build_check\contextual_submarine_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Contextual underwater vehicle entry check FAILED - see build_check\contextual_submarine_report.txt
    exit /b 1
)
"out\checks\npc_future_check.exe" > build_check\npc_future_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Future-floor native NPC check FAILED - see build_check\npc_future_report.txt
    exit /b 1
)
"out\checks\npc_region_check.exe" > build_check\npc_region_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native area and stair-route check FAILED - see build_check\npc_region_report.txt
    exit /b 1
)
"out\checks\npc_motion_check.exe" > build_check\npc_motion_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native moving-NPC and scene-control check FAILED - see build_check\npc_motion_report.txt
    exit /b 1
)
python tools\re\npc_remote_routes.py > build_check\npc_route_assets_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native patrol fact audit FAILED - see build_check\npc_route_assets_report.txt
    exit /b 1
)
"out\checks\npc_oscillation_check.exe" > build_check\npc_oscillation_oracle_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo NPC oscillation oracle check FAILED - see build_check\npc_oscillation_oracle_report.txt
    exit /b 1
)
"out\checks\npc_oscillation_check.exe" --verify > build_check\npc_oscillation_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo NPC oscillation retention check FAILED - see build_check\npc_oscillation_report.txt
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
"out\checks\npc_entry_check.exe" > build_check\npc_entry_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo NPC scripted-entry check FAILED - see build_check\npc_entry_report.txt
    exit /b 1
)


"out\checks\istory_entry_check.exe" > build_check\istory_entry_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Istory prerequisite check FAILED - see build_check\istory_entry_report.txt
    exit /b 1
)
"out\checks\mask_projection_check.exe" > build_check\mask_projection_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Imperial Camp projection check FAILED - see build_check\mask_projection_report.txt
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
"out\checks\doga_surface_check.exe" > build_check\doga_surface_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Nautilus surfacing marker check FAILED - see build_check\doga_surface_report.txt
    exit /b 1
)
"out\checks\crystal_tower_check.exe" > build_check\crystal_tower_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Crystal Tower world return check FAILED - see build_check\crystal_tower_report.txt
    exit /b 1
)
"out\checks\nautilus_route_check.exe" > build_check\nautilus_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Nautilus underwater entry and landing check FAILED - see build_check\nautilus_route_report.txt
    exit /b 1
)
python tools\submarine_entrances.py > build_check\submarine_entrance_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Underwater source-to-bake check FAILED - see build_check\submarine_entrance_report.txt
    exit /b 1
)

python tools\modtext_puzzle_verbosity.py --check > build_check\puzzle_verbosity_locale_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_palace.py --check > build_check\palace_semantic_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_solitary_guide.py --check > build_check\author_solitary_guide_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_solitary_passage.py --check > build_check\author_solitary_passage_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_solitary.py --check > build_check\modtext_solitary_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_solitary.py --check > build_check\review_ff6_solitary_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_imperial_state.py --check > build_check\author_imperial_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_imperial_guides.py --check > build_check\author_imperial_guides_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_imperial.py --check > build_check\modtext_imperial_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_imperial.py --check > build_check\review_ff6_imperial_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_esper_passages.py --check > build_check\author_esper_passages_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_esper_guide.py --check > build_check\author_esper_guide_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_esper.py --check > build_check\modtext_esper_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_esper.py --check > build_check\review_ff6_esper_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_crescent_state.py --check > build_check\author_crescent_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_crescent_guide.py --check > build_check\author_crescent_guide_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_serpent_arrival.py --check > build_check\author_serpent_arrival_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_crescent.py --check > build_check\modtext_crescent_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_crescent.py --check > build_check\review_ff6_crescent_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\re\crescent_contract.py --check > build_check\crescent_contract_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_mobliz.py --check > build_check\mobliz_semantic_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_mobliz_guides.py --check > build_check\mobliz_recipe_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_mobliz.py --check > build_check\mobliz_locale_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_palace_guide.py --check > build_check\palace_recipe_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_palace.py --check > build_check\palace_locale_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\palace_contract.py > build_check\palace_contract_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_puzzle_types.py --check > build_check\puzzle_types_metadata_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_puzzle_types.py --check > build_check\puzzle_types_locale_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_puzzle_entry.py --check > build_check\puzzle_entry_locale_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_gerad_crossings.py --check > build_check\author_gerad_crossings_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_gerad_guide.py --check > build_check\author_gerad_guide_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_gerad.py --check > build_check\modtext_gerad_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_gerad.py --check > build_check\review_gerad_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_stables.py --check > build_check\review_ff6_stables_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_duncan_state.py --check > build_check\author_duncan_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_duncan.py --check > build_check\review_ff6_duncan_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_returners.py --check > build_check\review_ff6_returners_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_continuations_ff6.py --check > build_check\review_continuations_ff6_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_surgate_continuations.py --check > build_check\review_surgate_continuations_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_continuations_ii_iv.py --check > build_check\review_continuations_ii_iv_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\inventory_puzzle_continuations.py --check > build_check\inventory_puzzle_continuations_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_pyramid.py --check > build_check\author_ff5_pyramid_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_pyramid_arrival.py --check > build_check\author_pyramid_arrival_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_ff5_pyramid.py --check > build_check\modtext_ff5_pyramid_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_pyramid.py --check > build_check\review_ff5_pyramid_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_jachol.py --check > build_check\author_ff5_jachol_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_jachol_state.py --check > build_check\author_jachol_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_ff5_jachol.py --check > build_check\modtext_ff5_jachol_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_jachol.py --check > build_check\review_ff5_jachol_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_istory.py --check > build_check\author_ff5_istory_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_ff5_istory.py --check > build_check\modtext_ff5_istory_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_istory.py --check > build_check\review_ff5_istory_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_ordinary_early.py --check > build_check\review_ff5_ordinary_early_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_pianos.py --check > build_check\review_ff5_pianos_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_gogo.py --check > build_check\author_ff5_gogo_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_gogo_state.py --check > build_check\author_gogo_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_gogo_arrivals.py --check > build_check\author_gogo_arrivals_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_ff5_gogo.py --check > build_check\modtext_ff5_gogo_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_gogo.py --check > build_check\review_ff5_gogo_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_fire_ship_state.py --check > build_check\author_fire_ship_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\re\ff5_fire_ship_facts.py --check > build_check\ff5_fire_ship_facts_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_fire_ship.py --check > build_check\author_ff5_fire_ship_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_ff5_fire_ship.py --check > build_check\modtext_ff5_fire_ship_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_fire_ship.py --check > build_check\review_ff5_fire_ship_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_desert_state.py --check > build_check\author_desert_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_desert.py --check > build_check\author_ff5_desert_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_ff5_desert.py --check > build_check\modtext_ff5_desert_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_desert.py --check > build_check\review_ff5_desert_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_gohn.py --check > build_check\author_ff5_gohn_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_gohn.py --check > build_check\review_ff5_gohn_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_town_contacts.py --check > build_check\review_ff5_town_contacts_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_tycoon_exit_state.py --check > build_check\author_tycoon_exit_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_tycoon.py --check > build_check\review_ff5_tycoon_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_fork.py --check > build_check\author_ff5_fork_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_ff5_fork.py --check > build_check\modtext_ff5_fork_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_fork.py --check > build_check\review_ff5_fork_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_exdeath.py --check > build_check\author_ff5_exdeath_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_ff5_exdeath.py --check > build_check\modtext_ff5_exdeath_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_exdeath.py --check > build_check\review_ff5_exdeath_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_istory_falls.py --check > build_check\author_istory_falls_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_istory_falls.py --check > build_check\review_istory_falls_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_karnak.py --check > build_check\author_ff5_karnak_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_karnak.py --check > build_check\review_ff5_karnak_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\karnak_state.py --check > build_check\karnak_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_remaining_ordinary.py --check > build_check\review_ff5_remaining_ordinary_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_ship_graveyard.py --check > build_check\author_ff5_ship_graveyard_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\walse_jail_state.py --check > build_check\walse_jail_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_short_passages.py --check > build_check\review_ff5_short_passages_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_rewards_and_lap.py --check > build_check\review_ff5_rewards_and_lap_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_forest.py --check > build_check\review_ff5_forest_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_death_valley.py --check > build_check\author_ff5_death_valley_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_trench.py --check > build_check\author_ff5_trench_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff5_island_vents.py --check > build_check\author_ff5_island_vents_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff5_story_contacts.py --check > build_check\review_ff5_story_contacts_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\puzzle_semantic_review_check.py --check-sources > build_check\puzzle_semantic_review_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_lethe.py --check > build_check\author_lethe_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_lethe.py --check > build_check\modtext_lethe_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_lethe.py --check > build_check\review_lethe_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_mt_kolts.py --check > build_check\author_mt_kolts_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_mt_kolts.py --check > build_check\modtext_mt_kolts_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_mt_kolts.py --check > build_check\review_mt_kolts_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_late_opera.py --check > build_check\author_late_opera_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_late_opera.py --check > build_check\modtext_late_opera_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_late_opera.py --check > build_check\review_late_opera_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_veldt_cave.py --check > build_check\author_veldt_cave_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_veldt_cave.py --check > build_check\modtext_veldt_cave_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_veldt_cave.py --check > build_check\review_veldt_cave_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_zozo_pickup_state.py --check > build_check\author_zozo_pickup_state_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_zozo_jidoor.py --check > build_check\review_zozo_jidoor_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_cyan_zozo.py --check > build_check\author_cyan_zozo_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_cyan_zozo.py --check > build_check\modtext_cyan_zozo_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_cyan_zozo.py --check > build_check\review_ff6_cyan_zozo_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_tzen_guide.py --check > build_check\author_tzen_guide_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_tzen.py --check > build_check\modtext_tzen_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_tzen.py --check > build_check\review_ff6_tzen_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ordinary_ff6.py --check > build_check\review_ordinary_ff6_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_yeti_guide.py --check > build_check\author_yeti_guide_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_yeti.py --check > build_check\modtext_yeti_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_yeti.py --check > build_check\review_ff6_yeti_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_dream_episode.py --check > build_check\author_dream_episode_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\modtext_dream_episode.py --check > build_check\modtext_dream_episode_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\review_ff6_dream.py --check > build_check\review_ff6_dream_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\re\dream_episode_native_check.py --check > build_check\dream_episode_native_check_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\build_check_flow_check.py > build_check\build_check_flow_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\puzzle_guide_check.exe" > build_check\puzzle_guide_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Puzzle guide check FAILED - see build_check\puzzle_guide_report.txt
    exit /b 1
)
"out\checks\static_puzzle_route_check.exe" > build_check\static_puzzle_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Static puzzle route check FAILED - see build_check\static_puzzle_route_report.txt
    exit /b 1
)
"out\checks\ff1_slab_guide_check.exe" > build_check\ff1_slab_guide_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF1 prerequisite and slab guide check FAILED - see build_check\ff1_slab_guide_report.txt
    exit /b 1
)
python tools\author_ff1_slab_guides.py --check > build_check\ff1_slab_receipts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF1 native puzzle receipts FAILED - see build_check\ff1_slab_receipts_report.txt
    exit /b 1
)
python tools\modtext_ff1_slabs.py --check > build_check\ff1_slab_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF1 puzzle localization FAILED - see build_check\ff1_slab_locale_report.txt
    exit /b 1
)
"out\checks\deist_guide_check.exe" > build_check\deist_guide_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Deist complete guide journeys FAILED - see build_check\deist_guide_report.txt
    exit /b 1
)
"out\checks\ff2_gate_journey_check.exe" > build_check\ff2_gate_journey_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF2 complete gate journeys FAILED - see build_check\ff2_gate_journey_report.txt
    exit /b 1
)
python tools\author_ff2_gate_guides.py --check > build_check\ff2_gate_receipts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF2 gate native receipts FAILED - see build_check\ff2_gate_receipts_report.txt
    exit /b 1
)
python tools\re\ff2_escape_contract.py > build_check\ff2_escape_contract_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF2 native escape contract FAILED - see build_check\ff2_escape_contract_report.txt
    exit /b 1
)
python tools\author_ff2_puzzle_guides.py --check > build_check\ff2_puzzle_receipts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF2 native puzzle receipts FAILED - see build_check\ff2_puzzle_receipts_report.txt
    exit /b 1
)
python tools\modtext_ff2_puzzles.py --check > build_check\ff2_puzzle_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF2 puzzle localization FAILED - see build_check\ff2_puzzle_locale_report.txt
    exit /b 1
)
python tools\re\ff2_keyword_bindings.py --check > build_check\ff2_keyword_bindings_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF2 keyword native contract FAILED - see build_check\ff2_keyword_bindings_report.txt
    exit /b 1
)
"out\checks\ff3_puzzle_model_check.exe" > build_check\ff3_puzzle_model_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 puzzle model FAILED - see build_check\ff3_puzzle_model_report.txt
    exit /b 1
)
"out\checks\ff3_form_guide_check.exe" > build_check\ff3_form_guide_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 form guide ownership FAILED - see build_check\ff3_form_guide_report.txt
    exit /b 1
)
"out\checks\ff3_puzzle_journey_check.exe" > build_check\ff3_puzzle_journey_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 complete puzzle journeys FAILED - see build_check\ff3_puzzle_journey_report.txt
    exit /b 1
)
python tools\author_ff3_puzzle_guides.py --check > build_check\ff3_puzzle_receipts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 puzzle native receipts FAILED - see build_check\ff3_puzzle_receipts_report.txt
    exit /b 1
)
python tools\modtext_ff3_puzzles.py --check > build_check\ff3_puzzle_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 puzzle localization FAILED - see build_check\ff3_puzzle_locale_report.txt
    exit /b 1
)
"out\checks\ff4_puzzle_journey_check.exe" > build_check\ff4_puzzle_journey_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF4 puzzle journeys FAILED - see build_check\ff4_puzzle_journey_report.txt
    exit /b 1
)
"out\checks\ff4_event_item_select_check.exe" > build_check\ff4_event_item_select_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF4 puzzle item menu FAILED - see build_check\ff4_event_item_select_report.txt
    exit /b 1
)
python tools\author_ff4_key_gates.py --check > build_check\ff4_key_gates_report.txt 2>&1
if errorlevel 1 exit /b 1
python tools\author_ff4_puzzle_guides.py --check > build_check\ff4_puzzle_receipts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF4 native puzzle receipts FAILED - see build_check\ff4_puzzle_receipts_report.txt
    exit /b 1
)
python tools\modtext_ff4_puzzles.py --check > build_check\ff4_puzzle_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF4 puzzle localization FAILED - see build_check\ff4_puzzle_locale_report.txt
    exit /b 1
)
python tools\re\ff4_event_item_contract.py > build_check\ff4_event_item_contract_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF4 native item-menu contract FAILED - see build_check\ff4_event_item_contract_report.txt
    exit /b 1
)
python tools\re\ff5_library_contract.py --check > build_check\ff5_library_contract_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\puppet_ready_contract.py --check > build_check\puppet_ready_contract_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\puppet_ready_check.exe" > build_check\puppet_ready_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff5_library_guide_check.exe" > build_check\ff5_library_guide_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_ff5_library_guide.py --check > build_check\ff5_library_author_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff5_library_owner_check.exe" > build_check\ff5_library_owner_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff5_library_resource_check.exe" > build_check\ff5_library_resource_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff5_library_entry_check.exe" > build_check\ff5_library_entry_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\ff5_catapult_native.py --entry-ui --check > build_check\ff5_catapult_native_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\ff5_catapult_fixture.py --check > build_check\ff5_catapult_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_ff5_catapult.py --check > build_check\ff5_catapult_author_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff5_catapult_check.exe" > build_check\ff5_catapult_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\ff5_stone_contract.py --check > build_check\ff5_stone_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\ff5_stone_fixture.py --check > build_check\ff5_stone_fixture_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_ff5_stone.py --check > build_check\ff5_stone_author_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff5_stone_route_check.exe" > build_check\ff5_stone_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\ff5_books_contract.py --check > build_check\ff5_books_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\ff5_books_fixture.py --check > build_check\ff5_books_fixture_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_ff5_books.py --check > build_check\ff5_books_author_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff5_books_route_check.exe" > build_check\ff5_books_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\palace_choice_check.exe" > build_check\palace_choice_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff6_gau_reader_check.exe" > build_check\ff6_gau_reader_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\serpent_choice_check.exe" > build_check\serpent_choice_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff6_terra_reader_check.exe" > build_check\ff6_terra_reader_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_gogo_route_check.exe" > build_check\ff5_gogo_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_fire_ship_policy_check.exe" > build_check\ff5_fire_ship_policy_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_fire_ship_route_check.exe" > build_check\ff5_fire_ship_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_fire_ship_owner_check.exe" > build_check\ff5_fire_ship_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_desert_policy_check.exe" > build_check\ff5_desert_policy_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_desert_route_check.exe" > build_check\ff5_desert_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_desert_owner_check.exe" > build_check\ff5_desert_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_gohn_route_check.exe" > build_check\ff5_gohn_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_gohn_owner_check.exe" > build_check\ff5_gohn_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_tycoon_exit_check.exe" > build_check\ff5_tycoon_exit_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_fork_route_check.exe" > build_check\ff5_fork_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_fork_owner_check.exe" > build_check\ff5_fork_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_exdeath_route_check.exe" > build_check\ff5_exdeath_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_exdeath_owner_check.exe" > build_check\ff5_exdeath_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_istory_falls_route_check.exe" > build_check\ff5_istory_falls_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_karnak_route_check.exe" > build_check\ff5_karnak_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_karnak_owner_check.exe" > build_check\ff5_karnak_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_ship_graveyard_route_check.exe" > build_check\ff5_ship_graveyard_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_walse_jail_check.exe" > build_check\ff5_walse_jail_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_short_passages_check.exe" > build_check\ff5_short_passages_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_forest_route_check.exe" > build_check\ff5_forest_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_death_valley_route_check.exe" > build_check\ff5_death_valley_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_trench_route_check.exe" > build_check\ff5_trench_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_island_vents_route_check.exe" > build_check\ff5_island_vents_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_story_contacts_route_check.exe" > build_check\ff5_story_contacts_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_gogo_owner_check.exe" > build_check\ff5_gogo_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_istory_route_check.exe" > build_check\ff5_istory_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_istory_owner_check.exe" > build_check\ff5_istory_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_jachol_route_check.exe" > build_check\ff5_jachol_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_jachol_owner_check.exe" > build_check\ff5_jachol_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_pyramid_route_check.exe" > build_check\ff5_pyramid_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff5_pyramid_owner_check.exe" > build_check\ff5_pyramid_owner_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\lethe_route_check.exe" > build_check\lethe_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\mt_kolts_route_check.exe" > build_check\mt_kolts_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\late_opera_route_check.exe" > build_check\late_opera_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\veldt_cave_route_check.exe" > build_check\veldt_cave_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\zozo_jidoor_route_check.exe" > build_check\zozo_jidoor_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\cyan_zozo_route_check.exe" > build_check\cyan_zozo_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\tzen_route_check.exe" > build_check\tzen_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\yeti_route_check.exe" > build_check\yeti_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff6_mog_reader_check.exe" > build_check\ff6_mog_reader_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\dream_episode_check.exe" > build_check\dream_episode_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\ff6_cyan_reader_check.exe" > build_check\ff6_cyan_reader_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\solitary_route_check.exe" > build_check\solitary_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\imperial_route_check.exe" > build_check\imperial_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\esper_route_check.exe" > build_check\esper_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\crescent_route_check.exe" > build_check\crescent_route_report.txt 2>&1
if errorlevel 1 exit /b 1
"out\checks\mobliz_route_check.exe" > build_check\mobliz_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\palace_route_check.exe" > build_check\palace_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff6_story_check.exe" > build_check\ff6_story_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF6 story guide check FAILED - see build_check\ff6_story_report.txt
    exit /b 1
)
"out\checks\multiparty_guide_check.exe" > build_check\multiparty_guide_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Multi-party guide episode FAILED - see build_check\multiparty_guide_report.txt
    exit /b 1
)
"out\checks\opera_route_check.exe" > build_check\opera_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Opera complete route FAILED - see build_check\opera_route_report.txt
    exit /b 1
)
python tools\layer_contract_check.py > build_check\layer_contract_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native layer contract FAILED - see build_check\layer_contract_report.txt
    exit /b 1
)
"out\checks\narshe_light_check.exe" > build_check\narshe_light_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Narshe light puzzle check FAILED - see build_check\narshe_light_report.txt
    exit /b 1
)
python tools\modtext_light_path.py --check > build_check\light_path_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Light puzzle localization check FAILED - see build_check\light_path_locale_report.txt
    exit /b 1
)
"out\checks\dream_train_check.exe" > build_check\dream_train_report.txt 2>&1
if errorlevel 1 (
    echo Dream train maze episodes FAILED - see build_check\dream_train_report.txt
    exit /b 1
)
"out\checks\dream_train_first_car_check.exe" > build_check\dream_train_first_car_report.txt 2>&1
if errorlevel 1 (
    echo Dream train first-car and handoff check FAILED - see build_check\dream_train_first_car_report.txt
    exit /b 1
)
python tools\re\dream_train_native_check.py > build_check\dream_train_native_summary.txt 2>&1
if errorlevel 1 (
    echo Dream train native contract FAILED - see build_check\dream_train_native_summary.txt
    exit /b 1
)
python tools\modtext_dream_train.py --check > build_check\dream_train_locale_report.txt 2>&1
if errorlevel 1 (
    echo Dream train localization FAILED - see build_check\dream_train_locale_report.txt
    exit /b 1
)
"out\checks\ff6_story_runtime_check.exe" > build_check\ff6_story_runtime_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF6 story runtime check FAILED - see build_check\ff6_story_runtime_report.txt
    exit /b 1
)
"out\checks\ff6_roof_route_check.exe" > build_check\ff6_roof_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Zone Eater roof route check FAILED - see build_check\ff6_roof_route_report.txt
    exit /b 1
)
python tools\author_zone_eater_roof.py --check > build_check\ff6_roof_source_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Zone Eater roof source check FAILED - see build_check\ff6_roof_source_report.txt
    exit /b 1
)
python tools\modtext_zone_eater_roof.py --check > build_check\ff6_roof_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Zone Eater roof localization FAILED - see build_check\ff6_roof_locale_report.txt
    exit /b 1
)
"out\checks\ff6_zone_journey_check.exe" > build_check\ff6_zone_journey_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Zone Eater crossing journey FAILED - see build_check\ff6_zone_journey_report.txt
    exit /b 1
)
python tools\author_zone_eater_crossings.py --check > build_check\ff6_zone_source_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Zone Eater crossing source FAILED - see build_check\ff6_zone_source_report.txt
    exit /b 1
)
python tools\modtext_zone_eater_crossings.py --check > build_check\ff6_zone_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Zone Eater crossing localization FAILED - see build_check\ff6_zone_locale_report.txt
    exit /b 1
)
"out\checks\phantom_train_journey_check.exe" > build_check\phantom_train_journey_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Phantom Train journey FAILED - see build_check\phantom_train_journey_report.txt
    exit /b 1
)
python tools\phantom_train_source_check.py > build_check\phantom_train_source_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Phantom Train source contract FAILED - see build_check\phantom_train_source_report.txt
    exit /b 1
)
"out\checks\ancient_castle_route_check.exe" > build_check\ancient_castle_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Ancient Castle objective routes FAILED - see build_check\ancient_castle_route_report.txt
    exit /b 1
)
python tools\author_ancient_castle_guide.py --check > build_check\ancient_castle_source_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Ancient Castle source FAILED - see build_check\ancient_castle_source_report.txt
    exit /b 1
)
python tools\modtext_ancient_castle.py --check > build_check\ancient_castle_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Ancient Castle locale FAILED - see build_check\ancient_castle_locale_report.txt
    exit /b 1
)
"out\checks\darills_tomb_route_check.exe" > build_check\darills_tomb_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Darill routes FAILED - see build_check\darills_tomb_route_report.txt
    exit /b 1
)
python tools\author_darills_tomb_guide.py --check > build_check\darills_tomb_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_darill_crossings.py --check > build_check\darills_tomb_crossings_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_darills_tomb.py --check > build_check\darills_tomb_locale_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_darills_tomb.py --check > build_check\darills_tomb_semantic_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\cultists_tower_route_check.exe" > build_check\cultists_tower_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Cultists Tower routes FAILED - see build_check\cultists_tower_route_report.txt
    exit /b 1
)
python tools\author_cultists_tower_guide.py --check > build_check\cultists_tower_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_cultists_tower_state.py --check > build_check\cultists_tower_state_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_cultists_tower.py --check > build_check\cultists_tower_semantic_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\owzers_mansion_route_check.exe" > build_check\owzers_mansion_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_owzers_mansion_guide.py --check > build_check\owzers_mansion_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_owzer_passages.py --check > build_check\owzers_mansion_passages_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_owzer_state.py --check > build_check\owzers_mansion_state_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_owzers_mansion.py --check > build_check\owzers_mansion_locale_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_owzers_mansion.py --check > build_check\owzers_mansion_semantic_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_ff6_cinematic_world_cohort.py --check > build_check\ff6_cinematic_world_cohort_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\zozo_clock_route_check.exe" > build_check\zozo_clock_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_zozo_clock_guide.py --check > build_check\zozo_clock_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\zozo_clock_choice_check.py > build_check\zozo_clock_choice_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_zozo_clock.py --check > build_check\zozo_clock_locale_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_zozo_clock.py --check > build_check\zozo_clock_semantic_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\sealed_gate_route_check.exe" > build_check\sealed_gate_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_sealed_gate_guide.py --check > build_check\sealed_gate_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_sealed_gate_passages.py --check > build_check\sealed_gate_passages_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_sealed_gate.py --check > build_check\sealed_gate_locale_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_sealed_gate.py --check > build_check\sealed_gate_semantic_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\floating_continent_route_check.exe" > build_check\floating_continent_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_floating_continent_guide.py --check > build_check\author_floating_continent_guide_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_floating_continent_state.py --check > build_check\author_floating_continent_state_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_floating_continent_passages.py --check > build_check\author_floating_continent_passages_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_floating_continent.py --check > build_check\modtext_floating_continent_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_floating_continent.py --check > build_check\review_floating_continent_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff6_locke_reader_check.exe" > build_check\ff6_locke_reader_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\narshe_party_gate_check.exe" > build_check\narshe_party_gate_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_narshe_party_gates.py --check > build_check\author_narshe_party_gates_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_narshe_state.py --check > build_check\author_narshe_state_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_narshe_gates.py --check > build_check\modtext_narshe_gates_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_narshe_party_gates.py --check > build_check\review_narshe_party_gates_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\narshe_chase_route_check.exe" > build_check\narshe_chase_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_narshe_chase_guide.py --check > build_check\author_narshe_chase_guide_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_narshe_chase.py --check > build_check\modtext_narshe_chase_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_narshe_interactions.py --check > build_check\review_narshe_interactions_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_sabin_scenario.py --check > build_check\review_sabin_scenario_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\magitek_transfer_check.exe" > build_check\magitek_transfer_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_magitek_passages.py --check > build_check\magitek_passages_source_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_magitek_factory.py --check > build_check\magitek_semantic_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff6_coral_reader_check.exe" > build_check\ff6_coral_reader_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ebots_rock_route_check.exe" > build_check\ebots_rock_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_ebots_rock_guide.py --check > build_check\author_ebots_rock_guide_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_ebot_warp_state.py --check > build_check\author_ebot_warp_state_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_ebots_rock.py --check > build_check\modtext_ebots_rock_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_ebots_rock.py --check > build_check\review_ebots_rock_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\south_figaro_route_check.exe" > build_check\south_figaro_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_south_figaro_guide.py --check > build_check\author_south_figaro_guide_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\south_figaro_passages.py --check > build_check\south_figaro_passages_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_south_figaro.py --check > build_check\modtext_south_figaro_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\review_south_figaro.py --check > build_check\review_south_figaro_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\keyboard_native_check.exe" > build_check\keyboard_native_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_keyboard_keycodes.py --check > build_check\author_keyboard_keycodes_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\modtext_keyboard_remapping.py --check > build_check\modtext_keyboard_remapping_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
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
python tools\modtext_opera_guide.py --check > build_check\opera_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Opera language check FAILED - see build_check\opera_locale_report.txt
    exit /b 1
)
python tools\opera_contract_check.py > build_check\opera_contract_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Opera source contract FAILED - see build_check\opera_contract_report.txt
    exit /b 1
)
python tools\opera_backdrop_contract_check.py > build_check\opera_backdrop_contract_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Opera backdrop contract FAILED - see build_check\opera_backdrop_contract_report.txt
    exit /b 1
)
python tools\author_static_puzzle_receipts.py --check > build_check\static_puzzle_receipts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Static puzzle operation receipts FAILED - see build_check\static_puzzle_receipts_report.txt
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
"out\checks\ff3_key_reader_check.exe" > build_check\ff3_key_reader_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 key capability reader FAILED - see build_check\ff3_key_reader_report.txt
    exit /b 1
)
"out\checks\ff3_key_topology_check.exe" > build_check\ff3_key_topology_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 consumable prerequisites FAILED - see build_check\ff3_key_topology_report.txt
    exit /b 1
)
"out\checks\ff3_key_route_check.exe" > build_check\ff3_key_route_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 manual key journeys FAILED - see build_check\ff3_key_route_report.txt
    exit /b 1
)
python tools\author_magic_key_gates.py --check > build_check\ff3_key_declarations_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 key identity declarations FAILED - see build_check\ff3_key_declarations_report.txt
    exit /b 1
)
python tools\re\ff3_key_capability_contract.py --check > build_check\ff3_key_capability_contract_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo FF3 native key contract FAILED - see build_check\ff3_key_capability_contract_report.txt
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

"out\checks\custom_sound_check.exe" > build_check\custom_sound_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Custom sound check FAILED - see build_check\custom_sound_report.txt
    exit /b 1
)
"out\checks\custom_settings_check.exe" > build_check\custom_settings_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Custom settings check FAILED - see build_check\custom_settings_report.txt
    exit /b 1
)
python tools\modtext_custom_sounds.py --check > build_check\custom_sound_locale_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Custom sound localization FAILED - see build_check\custom_sound_locale_report.txt
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
"out\checks\bootstrap_singleton_check.exe" > build_check\bootstrap_singleton_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Early singleton producer check FAILED - see build_check\bootstrap_singleton_report.txt
    exit /b 1
)
"out\checks\scan_feedback_check.exe" > build_check\scan_feedback_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Scan feedback check FAILED - see build_check\scan_feedback_report.txt
    exit /b 1
)
"out\checks\action_effect_check.exe" > build_check\action_effect_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Action effect feedback check FAILED - see build_check\action_effect_report.txt
    exit /b 1
)
"out\checks\boost_use_check.exe" > build_check\boost_use_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Boost use producer check FAILED - see build_check\boost_use_report.txt
    exit /b 1
)
python tools\re\boost_guard_facts.py > build_check\boost_guard_facts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Boost and Guard visual source check FAILED - see build_check\boost_guard_facts_report.txt
    exit /b 1
)
"out\checks\unfiltered_air_check.exe" > build_check\unfiltered_air_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Unfiltered and parked aircraft discovery check FAILED - see build_check\unfiltered_air_report.txt
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
python tools\functional_events_check.py > build_check\functional_events_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Functional event audit FAILED - see build_check\functional_events_report.txt
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
python tools\review_field_event_contacts.py --check > build_check\field_event_contacts_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Reviewed boss and spring check FAILED - see build_check\field_event_contacts_report.txt
    exit /b 1
)
python tools\functional_map_events_check.py > build_check\functional_map_events_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Functional map source and contextual scene check FAILED - see build_check\functional_map_events_report.txt
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
"out\checks\ff1_class_description_check.exe" > build_check\ff1_class_description_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\key_door_route_check.exe" > build_check\key_door_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff1_key_doors_check.exe" > build_check\ff1_key_doors_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff1_trench_route_check.exe" > build_check\ff1_trench_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff1_giant_route_check.exe" > build_check\ff1_giant_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff1_boarding_route_check.exe" > build_check\ff1_boarding_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff1_ice_route_check.exe" > build_check\ff1_ice_route_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\author_ff1_story_actor_gates.py --check > build_check\ff1_story_actor_gates_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\bake_loop_metadata.py --check > build_check\native_loop_metadata_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Native loop metadata check FAILED - see build_check\native_loop_metadata_report.txt
    exit /b 1
)
python tools\check_ff1_precision.py > build_check\ff1_precision_bake_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
python tools\re\ff1_class_description_contract.py > build_check\ff1_class_native_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
"out\checks\ff1_animated_goal_check.exe" > build_check\ff1_animated_goal_report.txt 2>&1
if not "%errorlevel%"=="0" exit /b 1
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
python tools\service_npc_labels.py --check > build_check\service_npc_labels_report.txt 2>&1
if not "%errorlevel%"=="0" (
    echo Service NPC naming check FAILED - see build_check\service_npc_labels_report.txt
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
exit /b 0

:one_check
if /I "%~3"=="--debug" goto one_check_debug
python tools\build_one_check.py "%~2"
exit /b %errorlevel%
:one_check_debug
python tools\build_one_check.py "%~2" --debug
exit /b %errorlevel%
