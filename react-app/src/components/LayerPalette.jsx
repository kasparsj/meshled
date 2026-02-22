import React from "react";
import { Plus, Minus } from "lucide-react";

const LayerPalette = ({ 
    layer, 
    predefinedPalettes,
    colorRuleOptions,
    interpolationModes,
    wrapModes,
    addColorToLayer,
    removeColorFromLayer,
    updateLayerColor,
    updateColorPosition,
    updatePalette,
    savePalette,
    loadPredefinedPalette,
}) => {
    const palette = layer.palette || { 
        colors: ['#FF0000'], 
        positions: [0], 
        colorRule: -1, 
        interMode: 1, 
        wrapMode: 0, 
        segmentation: 0 
    };

    return (
        <div className="space-y-4 mt-4 p-4 bg-zinc-800 rounded-lg">
            {/* Predefined Palette Selector */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Load Palette:</label>
                <select
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                    onChange={(e) => loadPredefinedPalette(layer.id, e.target.value)}
                    defaultValue=""
                >
                    <option value="">Select Palette</option>
                    <optgroup label="FastLED Palettes">
                        {predefinedPalettes.map((paletteName, index) => (
                            <option key={index} value={index}>
                                {paletteName}
                            </option>
                        ))}
                    </optgroup>
                </select>
            </div>
            
            {/* Color Controls */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">
                    Colors: {palette.colors.length}
                </label>
                <div className="space-y-3">
                    {palette.colors.map((color, index) => (
                        <div key={index} className="flex items-center gap-2">
                            <input
                                type="color"
                                value={color}
                                onChange={(e) => updateLayerColor(layer.id, index, e.target.value)}
                                className="w-12 h-10 rounded border border-zinc-500"
                            />
                            <input
                                type="number"
                                min="0"
                                max="1"
                                step="0.01"
                                value={palette.positions[index] || 0}
                                onChange={(e) => updateColorPosition(layer.id, index, parseFloat(e.target.value))}
                                className="flex-1 bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                            />
                            <button
                                onClick={() => removeColorFromLayer(layer.id, index)}
                                className="text-red-400 hover:text-red-300 p-2"
                            >
                                <Minus size={16} />
                            </button>
                        </div>
                    ))}
                </div>
                
                <button
                    onClick={() => addColorToLayer(layer.id)}
                    className="w-full mt-2 bg-slate-600 hover:bg-slate-700 text-white px-4 py-2 rounded-lg flex items-center justify-center gap-2"
                >
                    <Plus size={16} />
                    Add Color
                </button>
                
                <button
                    onClick={() => savePalette(layer.id)}
                    className="w-full mt-2 bg-green-600 hover:bg-green-700 text-white px-4 py-2 rounded-lg"
                >
                    Save Palette
                </button>
            </div>
            
            {/* Color Rule */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Color Rule:</label>
                <select
                    value={palette.colorRule}
                    onChange={(e) => updatePalette(layer.id, { colorRule: parseInt(e.target.value) })}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                >
                    {colorRuleOptions.map(option => (
                        <option key={option.value} value={option.value}>
                            {option.label}
                        </option>
                    ))}
                </select>
            </div>
            
            {/* Color Interpolation */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Color Interpolation:</label>
                <select
                    value={palette.interMode}
                    onChange={(e) => updatePalette(layer.id, { interMode: parseInt(e.target.value) })}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                >
                    {interpolationModes.map(option => (
                        <option key={option.value} value={option.value}>
                            {option.label}
                        </option>
                    ))}
                </select>
            </div>
            
            {/* Segmentation */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Segmentation:</label>
                <input
                    type="number"
                    min="0"
                    max="100"
                    step="0.1"
                    value={palette.segmentation}
                    onChange={(e) => updatePalette(layer.id, { segmentation: parseFloat(e.target.value) })}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                />
            </div>
            
            {/* Wrap Mode */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Wrap Mode:</label>
                <select
                    value={palette.wrapMode}
                    onChange={(e) => updatePalette(layer.id, { wrapMode: parseInt(e.target.value) })}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                >
                    {wrapModes.map(option => (
                        <option key={option.value} value={option.value}>
                            {option.label}
                        </option>
                    ))}
                </select>
            </div>
        </div>
    );
};

export default LayerPalette;