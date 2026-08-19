# Copyright (c) 2026 Helen Allien Poe. See LICENSE.
"""Creates the M2 editor assets: the three cue Blueprints, the regen curve, and BP_Grove.

Run headless:
  UnrealEditor-Cmd.exe <project>.uproject -run=pythonscript -script="<this file>" -unattended -nopause -nosplash

Cue Blueprint names are load-bearing: the cue manager derives the tag from the asset name
(GC_Mana_Drain -> GameplayCue.Mana.Drain). Do not rename them.

Idempotent: safe to run more than once. Every asset is checked for existence before creation,
so a second run only re-saves already-correct state instead of duplicating anything.

Placing the grove in the level is deliberately NOT here. EditorActorSubsystem.spawn_actor_from_object
crashes this commandlet (access violation inside EditorFramework, immediately after load_level) —
actor placement wants a level-editor context that -run=pythonscript never builds. Placement is a
manual step in docs/issues/2026-08-11-m2-hud-handoff.md, alongside the HUD work that needs the
editor anyway.
"""

import os

import unreal

CUE_PATH = "/Game/GAS/Cues"
WORLD_PATH = "/Game/World"

# Footprint area in square metres -> mana per second. Starting values; tune in-editor.
REGEN_CURVE_KEYS = [(50.0, 4.0), (200.0, 10.0)]

assets = unreal.AssetToolsHelpers.get_asset_tools()


def make_blueprint(name, package_path, parent_class):
    full = "{}/{}".format(package_path, name)
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.log("exists, skipping: {}".format(full))
        return unreal.EditorAssetLibrary.load_asset(full)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = assets.create_asset(name, package_path, unreal.Blueprint, factory)
    unreal.EditorAssetLibrary.save_asset(full)
    unreal.log("created: {}".format(full))
    return asset


def make_regen_curve():
    # UCurveFloat.FloatCurve is a plain UPROPERTY() with no edit specifiers, so it is not
    # reachable via get_editor_property/set_editor_property from Python (the scripting layer
    # only exposes CPF_Edit properties). Importing from an in-memory CSV through the same
    # CSVImportFactory the editor's own "Import" button uses is the supported route to key data.
    full = "{}/{}".format(WORLD_PATH, "Curve_GroveRegen")
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.log("exists, skipping: {}".format(full))
        return unreal.EditorAssetLibrary.load_asset(full)

    csv_path = os.path.join(unreal.Paths.project_intermediate_dir(), "tae_grove_regen_curve.csv")
    csv_path = os.path.normpath(csv_path)
    with open(csv_path, "w") as f:
        for time, value in REGEN_CURVE_KEYS:
            f.write("{},{}\n".format(time, value))

    factory = unreal.CSVImportFactory()
    settings = factory.get_editor_property("automated_import_settings")
    settings.set_editor_property("import_type", unreal.CSVImportType.ECSV_CURVE_FLOAT)
    factory.set_editor_property("automated_import_settings", settings)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", csv_path)
    task.set_editor_property("destination_path", WORLD_PATH)
    task.set_editor_property("destination_name", "Curve_GroveRegen")
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", False)
    task.set_editor_property("save", True)
    task.set_editor_property("factory", factory)

    assets.import_asset_tasks([task])
    os.remove(csv_path)

    if not unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.log_error("failed to create: {}".format(full))
        return None

    unreal.log("created: {}".format(full))
    return unreal.EditorAssetLibrary.load_asset(full)


def ensure_grove_component(grove_bp, regen_curve):
    # Add a UTaeGroveComponent as BP_Grove's root, sized and curved per the M2 spec, unless
    # one is already there (idempotent re-run).
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary

    handles = sds.k2_gather_subobject_data_for_blueprint(grove_bp)
    grove_handle = None
    root_handle = handles[0] if handles else None
    for h in handles:
        data = sds.k2_find_subobject_data_from_handle(h)
        obj = lib.get_associated_object(data)
        if obj is not None and obj.get_class().get_name() == "TaeGroveComponent":
            grove_handle = h
            break

    if grove_handle is None:
        params = unreal.AddNewSubobjectParams(
            parent_handle=root_handle,
            new_class=unreal.TaeGroveComponent,
            blueprint_context=grove_bp,
        )
        grove_handle, fail_reason = sds.add_new_subobject(params)
        if not lib.is_handle_valid(grove_handle):
            unreal.log_error("failed to add TaeGroveComponent to BP_Grove: {}".format(fail_reason))
            return False
        if not sds.make_new_scene_root(root_handle, grove_handle, grove_bp):
            unreal.log_error("failed to make TaeGroveComponent the root of BP_Grove")
            return False
        unreal.log("added TaeGroveComponent as BP_Grove root")
    else:
        unreal.log("BP_Grove already has a TaeGroveComponent, skipping add")

    data = sds.k2_find_subobject_data_from_handle(grove_handle)
    comp = lib.get_associated_object(data)
    comp.set_editor_property("box_extent", unreal.Vector(700.0, 700.0, 200.0))
    comp.set_editor_property("regen_curve", regen_curve)

    unreal.BlueprintEditorLibrary.compile_blueprint(grove_bp)
    unreal.EditorAssetLibrary.save_asset("{}/{}".format(WORLD_PATH, "BP_Grove"))
    unreal.log("BP_Grove: box_extent=(700,700,200), regen_curve=Curve_GroveRegen, compiled and saved")
    return True


def main():
    make_blueprint("GC_Mana_Drain", CUE_PATH, unreal.TaeCueNotify_ManaDrain)
    make_blueprint("GC_Mana_Regen", CUE_PATH, unreal.TaeCueNotify_ManaRegen)
    make_blueprint("GC_Arcane_Exhausted", CUE_PATH, unreal.TaeCueNotify_ArcaneExhausted)

    regen_curve = make_regen_curve()
    grove_bp = make_blueprint("BP_Grove", WORLD_PATH, unreal.Actor)
    ensure_grove_component(grove_bp, regen_curve)

    unreal.log("M2 assets done")


main()
