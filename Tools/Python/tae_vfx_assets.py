# Copyright (c) 2026 Helen Allien Poe. See LICENSE.
"""Creates the arcane presentation assets: MPC_Arcane and DA_ArcanePalette.

Run headless:
  UnrealEditor-Cmd.exe <project>.uproject -run=pythonscript -script="<this file>" -unattended -nopause -nosplash

The four MPC parameter names are load-bearing: they must match TaeArcaneParams in
Public/Core/TaeArcanePalette.h character for character. SetVectorParameterValue returns false for
a name the collection does not declare, so a mismatch surfaces as ApplyPaletteToCollection
returning fewer than 4 and a "[Arcane] No palette colours written" warning in the log.

Idempotent: safe to run more than once. Every asset is checked for existence before creation.

Niagara systems are deliberately NOT here. Emitter graphs are not exposed to Python, so a scripted
NS_ asset would be an empty shell rather than a starting point. NS_GroveBloom and NS_GrowthFront
are authored by hand - see docs/issues/2026-08-15-arcane-presentation-handoff.md.

Unlike UCurveFloat.FloatCurve (see the CSV workaround in tae_m2_assets.py),
UMaterialParameterCollection.VectorParameters is UPROPERTY(EditAnywhere) and therefore reachable
from Python directly. No import workaround needed here.
"""

import unreal

MATERIALS_PATH = "/Game/Locations/Materials"
CORE_PATH = "/Game/Core"

# Must match TaeArcaneParams in Public/Core/TaeArcanePalette.h, and the defaults should match the
# UPROPERTY initialisers on UTaeArcanePalette so an unassigned palette still looks deliberate.
ARCANE_PARAMS = [
    ("SpectralEdge", unreal.LinearColor(0.2, 0.8, 1.0, 1.0)),
    ("CubeTint", unreal.LinearColor(0.1, 0.4, 0.6, 1.0)),
    ("GroveBloom", unreal.LinearColor(0.4, 0.9, 0.35, 1.0)),
    ("GrowthFront", unreal.LinearColor(0.6, 1.0, 0.5, 1.0)),
]

assets = unreal.AssetToolsHelpers.get_asset_tools()


def make_parameter_collection():
    full = "{}/{}".format(MATERIALS_PATH, "MPC_Arcane")
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.log("exists, skipping: {}".format(full))
        return unreal.EditorAssetLibrary.load_asset(full)

    factory = unreal.MaterialParameterCollectionFactoryNew()
    collection = assets.create_asset(
        "MPC_Arcane", MATERIALS_PATH, unreal.MaterialParameterCollection, factory)

    params = []
    for name, default in ARCANE_PARAMS:
        param = unreal.CollectionVectorParameter()
        param.set_editor_property("parameter_name", name)
        param.set_editor_property("default_value", default)
        params.append(param)

    collection.set_editor_property("vector_parameters", params)
    unreal.EditorAssetLibrary.save_asset(full)
    unreal.log("created: {} with {} vector parameters".format(full, len(params)))
    return collection


def make_palette():
    full = "{}/{}".format(CORE_PATH, "DA_ArcanePalette")
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.log("exists, skipping: {}".format(full))
        return unreal.EditorAssetLibrary.load_asset(full)

    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.TaeArcanePalette)
    palette = assets.create_asset(
        "DA_ArcanePalette", CORE_PATH, unreal.TaeArcanePalette, factory)

    # Left at the C++ defaults deliberately - this asset is the artist's tuning surface
    unreal.EditorAssetLibrary.save_asset(full)
    unreal.log("created: {}".format(full))
    return palette


def main():
    make_parameter_collection()
    make_palette()
    unreal.log("arcane presentation assets done")


main()
