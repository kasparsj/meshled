import { Layers, Trash2, RotateCcw } from "lucide-react";
import React, { useState, useEffect } from "react";
import useLayers from "../hooks/useLayers";
import LayerPalette from "../components/LayerPalette";
import ColorPreviewStripe from "../components/ColorPreviewStripe";
import { useToast } from "../contexts/ToastContext.jsx";

const LayersTab = () => {
    const { showToast } = useToast();
    const {
        layers,
        loading,
        error,
        toggleLayerVisibility,
        updateLayerBrightness,
        updateLayerSpeed,
        updateLayerOffset,
        removeLayer,
        updatePalette,
        savePalette,
        loadPredefinedPalette,
        addLayer,
        addColorToLayer,
        removeColorFromLayer,
        updateLayerColor,
        updateColorPosition,
        updateEasing,
        updateFadeSpeed,
        updateBlendMode,
        updateBehaviourFlags,
        resetLayer
    } = useLayers();
    
    // Load expanded layers from localStorage
    const [expandedLayers, setExpandedLayers] = useState(() => {
        try {
            const saved = localStorage.getItem('expandedLayers');
            return saved ? new Set(JSON.parse(saved)) : new Set();
        } catch {
            return new Set();
        }
    });

    // Save expanded layers to localStorage whenever it changes
    useEffect(() => {
        try {
            localStorage.setItem('expandedLayers', JSON.stringify([...expandedLayers]));
        } catch (error) {
            console.error('Failed to save expanded layers to localStorage:', error);
        }
    }, [expandedLayers]);
    
    // Predefined palettes from the C++ code
    const predefinedPalettes = [
        "Sunset Real", "ES Rivendell", "Ocean Breeze", "RGI 15", "Retro 2",
        "Analogous 1", "Pink Splash 08", "Coral Reef", "Ocean Breeze 068",
        "Pink Splash 07", "Vintage 01", "Departure", "Landscape 64",
        "Landscape 33", "Rainbow Sherbet", "Green Hult 65", "Green Hult 64",
        "Dry Wet", "July 01", "Vintage 57", "IB 15", "Fuschia 7",
        "Emerald Dragon", "Lava", "Fire", "Colorful", "Magenta Evening",
        "Pink Purple", "Autumn 19", "Black Blue Magenta White",
        "Black Magenta Red", "Black Red Magenta Yellow", "Blue Cyan Yellow"
    ];
    
    const colorRuleOptions = [
        { value: -1, label: "None" },
        { value: 0, label: "Analogous" },
        { value: 1, label: "Complementary" },
        { value: 2, label: "Split Complementary" },
        { value: 3, label: "Compound" },
        { value: 4, label: "Flipped Compound" },
        { value: 5, label: "Monochrome" },
        { value: 6, label: "Triad" },
        { value: 7, label: "Tetrad" }
    ];
    
    const interpolationModes = [
        { value: -1, label: "None" },
        { value: 0, label: "RGB" },
        { value: 1, label: "HSB" },
        { value: 2, label: "CIELCh" }
    ];
    
    const wrapModes = [
        { value: -1, label: "Nowrap" },
        { value: 0, label: "Clamp to Edge" },
        { value: 1, label: "Repeat" },
        { value: 2, label: "Repeat Mirror" }
    ];
    
    const toggleLayerExpanded = (layerId) => {
        setExpandedLayers(prev => {
            const newSet = new Set(prev);
            if (newSet.has(layerId)) {
                newSet.delete(layerId);
            } else {
                newSet.add(layerId);
            }
            return newSet;
        });
    };

    const toInteger = (rawValue, fallback) => {
        const parsed = Number.parseInt(rawValue, 10);
        return Number.isFinite(parsed) ? parsed : fallback;
    };

    const toFloat = (rawValue, fallback) => {
        const parsed = Number.parseFloat(rawValue);
        return Number.isFinite(parsed) ? parsed : fallback;
    };

    const runMutation = async (operation, fallbackMessage) => {
        try {
            await operation();
        } catch (mutationError) {
            showToast(mutationError.message || fallbackMessage, 'error');
        }
    };

    return (
        <div className="space-y-6">
            <div className="flex justify-between items-center">
                <h2 className="text-2xl font-bold">Layer Control</h2>
                <button 
                    onClick={() => runMutation(addLayer, 'Failed to add layer')}
                    className="bg-green-600 hover:bg-green-700 px-4 py-2 rounded-lg flex items-center gap-2"
                >
                    <Layers size={18} />
                    Add Layer
                </button>
            </div>

            {loading && <div className="text-zinc-400 animate-pulse">Loading layers...</div>}
            {error && <div className="text-red-500">{error}</div>}

            <div className="space-y-4">
                {layers.map(layer => (
                    <div key={layer.id} className="bg-zinc-700 p-4 rounded-lg">
                        <div className="flex items-center justify-between mb-4">
                            <div className="flex items-center gap-2 cursor-pointer" onClick={() => toggleLayerExpanded(layer.id)}>
                                <button
                                    className="text-zinc-400 hover:text-white p-1"
                                    title="Toggle Layer Details"
                                >
                                    <span className={`inline-block transform transition-transform ${
                                        expandedLayers.has(layer.id) ? 'rotate-90' : ''
                                    }`}>
                                        ▶
                                    </span>
                                </button>
                                <div>
                                    <h3 className="text-lg font-semibold">Layer {layer.id + 1}</h3>
                                </div>
                            </div>
                            <div className="flex items-center gap-2">
                                <button
                                    onClick={() => runMutation(
                                        () => toggleLayerVisibility(layer.id),
                                        'Failed to toggle layer visibility',
                                    )}
                                    className={`px-3 py-1 rounded-full text-sm ${
                                        layer.visible
                                            ? 'bg-green-600 text-white'
                                            : 'bg-zinc-600 text-zinc-300'
                                    }`}
                                >
                                    {layer.visible ? 'Visible' : 'Hidden'}
                                </button>
                                <button
                                    onClick={() => runMutation(
                                        () => resetLayer(layer.id),
                                        'Failed to reset layer',
                                    )}
                                    className="text-blue-400 hover:text-blue-300 p-1"
                                    title="Reset Layer"
                                >
                                    <RotateCcw size={16} />
                                </button>
                                {layer.id > 0 && (
                                    <button
                                        onClick={() => runMutation(
                                            () => removeLayer(layer.id),
                                            'Failed to remove layer',
                                        )}
                                        className="text-red-400 hover:text-red-300 p-1"
                                        title="Remove Layer"
                                    >
                                        <Trash2 size={16} />
                                    </button>
                                )}
                            </div>
                        </div>
                        
                        {/* Color preview stripe */}
                        <ColorPreviewStripe 
                            palette={layer.palette} 
                            className="w-full h-2 bg-zinc-800 rounded mb-4 overflow-hidden" 
                        />

                        {expandedLayers.has(layer.id) && (
                            <>
                                <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mb-4">
                                    <div className="md:col-span-2">
                                        <label className="block text-sm text-zinc-300 mb-2">Brightness</label>
                                        <input
                                            type="range"
                                            min="1"
                                            max="255"
                                            value={layer.brightness}
                                            onChange={(event) => runMutation(
                                                () => updateLayerBrightness(
                                                    layer.id,
                                                    toInteger(event.target.value, layer.brightness),
                                                ),
                                                'Failed to update brightness',
                                            )}
                                            className="w-full"
                                        />
                                        <span className="text-sky-400 text-sm">{layer.brightness}</span>
                                    </div>

                                    <div>
                                        <label className="block text-sm text-zinc-300 mb-2">Speed</label>
                                        <input
                                            type="number"
                                            min="-10"
                                            max="10"
                                            step="0.1"
                                            value={layer.speed}
                                            onChange={(event) => runMutation(
                                                () => updateLayerSpeed(
                                                    layer.id,
                                                    toFloat(event.target.value, layer.speed),
                                                ),
                                                'Failed to update speed',
                                            )}
                                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                        />
                                    </div>

                                    <div>
                                        <label className="block text-sm text-zinc-300 mb-2">Offset</label>
                                        <input
                                            type="number"
                                            step="0.1"
                                            value={layer.offset || 0}
                                            onChange={(event) => runMutation(
                                                () => updateLayerOffset(
                                                    layer.id,
                                                    toFloat(event.target.value, layer.offset || 0),
                                                ),
                                                'Failed to update offset',
                                            )}
                                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                        />
                                    </div>

                                    <div>
                                        <label className="block text-sm text-zinc-300 mb-2">Fade Speed</label>
                                        <input
                                            type="number"
                                            min="0"
                                            max="255"
                                            step="1"
                                            value={layer.fadeSpeed || 0}
                                            onChange={(event) => runMutation(
                                                () => updateFadeSpeed(
                                                    layer.id,
                                                    toInteger(event.target.value, layer.fadeSpeed || 0),
                                                ),
                                                'Failed to update fade speed',
                                            )}
                                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                        />
                                    </div>

                                    <div>
                                        <label className="block text-sm text-zinc-300 mb-2">Easing</label>
                                        <select
                                            value={layer.easing || 0}
                                            onChange={(event) => runMutation(
                                                () => updateEasing(layer.id, toInteger(event.target.value, layer.easing || 0)),
                                                'Failed to update easing',
                                            )}
                                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                        >
                                            <option value="0">None</option>
                                            <optgroup label="Linear">
                                                <option value="1">Linear In</option>
                                                <option value="2">Linear Out</option>
                                                <option value="3">Linear InOut</option>
                                            </optgroup>
                                            <optgroup label="Sine">
                                                <option value="4">Sine In</option>
                                                <option value="5">Sine Out</option>
                                                <option value="6">Sine InOut</option>
                                            </optgroup>
                                            <optgroup label="Circular">
                                                <option value="7">Circular In</option>
                                                <option value="8">Circular Out</option>
                                                <option value="9">Circular InOut</option>
                                            </optgroup>
                                            <optgroup label="Quadratic">
                                                <option value="10">Quadratic In</option>
                                                <option value="11">Quadratic Out</option>
                                                <option value="12">Quadratic InOut</option>
                                            </optgroup>
                                            <optgroup label="Cubic">
                                                <option value="13">Cubic In</option>
                                                <option value="14">Cubic Out</option>
                                                <option value="15">Cubic InOut</option>
                                            </optgroup>
                                            <optgroup label="Quartic">
                                                <option value="16">Quartic In</option>
                                                <option value="17">Quartic Out</option>
                                                <option value="18">Quartic InOut</option>
                                            </optgroup>
                                            <optgroup label="Quintic">
                                                <option value="19">Quintic In</option>
                                                <option value="20">Quintic Out</option>
                                                <option value="21">Quintic InOut</option>
                                            </optgroup>
                                            <optgroup label="Exponential">
                                                <option value="22">Exponential In</option>
                                                <option value="23">Exponential Out</option>
                                                <option value="24">Exponential InOut</option>
                                            </optgroup>
                                            <optgroup label="Back">
                                                <option value="25">Back In</option>
                                                <option value="26">Back Out</option>
                                                <option value="27">Back InOut</option>
                                            </optgroup>
                                            <optgroup label="Bounce">
                                                <option value="28">Bounce In</option>
                                                <option value="29">Bounce Out</option>
                                                <option value="30">Bounce InOut</option>
                                            </optgroup>
                                            <optgroup label="Elastic">
                                                <option value="31">Elastic In</option>
                                                <option value="32">Elastic Out</option>
                                                <option value="33">Elastic InOut</option>
                                            </optgroup>
                                        </select>
                                    </div>

                                    <div>
                                        <label className="block text-sm text-zinc-300 mb-2">Blend Mode</label>
                                        <select
                                            value={layer.blendMode || 0}
                                            onChange={(event) => runMutation(
                                                () => updateBlendMode(layer.id, toInteger(event.target.value, layer.blendMode || 0)),
                                                'Failed to update blend mode',
                                            )}
                                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                        >
                                            <optgroup label="Basic">
                                                <option value="0">Normal</option>
                                                <option value="1">Add</option>
                                                <option value="5">Replace</option>
                                                <option value="6">Subtract</option>
                                            </optgroup>
                                            <optgroup label="Photoshop-style">
                                                <option value="2">Multiply</option>
                                                <option value="3">Screen</option>
                                                <option value="4">Overlay</option>
                                                <option value="7">Difference</option>
                                                <option value="8">Exclusion</option>
                                            </optgroup>
                                            <optgroup label="Advanced">
                                                <option value="9">Color Dodge</option>
                                                <option value="10">Color Burn</option>
                                                <option value="11">Hard Light</option>
                                                <option value="12">Soft Light</option>
                                                <option value="13">Linear Light</option>
                                                <option value="14">Vivid Light</option>
                                                <option value="15">Pin Light</option>
                                            </optgroup>
                                        </select>
                                    </div>

                                    <div>
                                        <label className="block text-sm text-zinc-300 mb-2">Behaviour</label>
                                        <select
                                            value={layer.behaviourFlags || 0}
                                            onChange={(event) => runMutation(
                                                () => updateBehaviourFlags(
                                                    layer.id,
                                                    toInteger(event.target.value, layer.behaviourFlags || 0),
                                                ),
                                                'Failed to update behaviour flags',
                                            )}
                                            className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                        >
                                            <option value="0">None</option>
                                            <option value="1">Position Change Fade</option>
                                            <option value="2">Brightness Constant Noise</option>
                                            <option value="4">Render Segment</option>
                                            <option value="8">Allow Bounce</option>
                                            <option value="16">Force Bounce</option>
                                            <option value="32">Expire Immediate</option>
                                            <option value="64">Emit From Connection</option>
                                            <option value="128">Fill Ease</option>
                                            <option value="256">Random Color</option>
                                            <option value="512">Mirror Flip</option>
                                            <option value="1024">Mirror Rotate</option>
                                            <option value="2048">Smooth Changes</option>
                                        </select>
                                    </div>
                                </div>
                                
                                {/* Palette Controls */}
                                <LayerPalette 
                                    layer={layer}
                                    predefinedPalettes={predefinedPalettes}
                                    colorRuleOptions={colorRuleOptions}
                                    interpolationModes={interpolationModes}
                                    wrapModes={wrapModes}
                                    addColorToLayer={addColorToLayer}
                                    removeColorFromLayer={removeColorFromLayer}
                                    updateLayerColor={updateLayerColor}
                                    updateColorPosition={updateColorPosition}
                                    updatePalette={updatePalette}
                                    savePalette={savePalette}
                                    loadPredefinedPalette={loadPredefinedPalette}
                                />
                            </>
                        )}
                    </div>
                ))}
            </div>
        </div>
    );
};

export default LayersTab;
