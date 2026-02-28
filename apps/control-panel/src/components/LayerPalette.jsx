import React from "react";
import { Plus, Minus } from "lucide-react";
import { useToast } from "../contexts/ToastContext.jsx";

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
    const { showToast } = useToast();
    const [showSaveModal, setShowSaveModal] = React.useState(false);
    const [paletteName, setPaletteName] = React.useState('');

    const palette = layer.palette || { 
        colors: ['#FF0000'], 
        positions: [0], 
        colorRule: -1, 
        interMode: 1, 
        wrapMode: 0, 
        segmentation: 0 
    };

    const runMutation = async (operation, fallbackMessage) => {
        try {
            await operation();
        } catch (error) {
            showToast(error.message || fallbackMessage, 'error');
        }
    };

    const openSaveModal = () => {
        setPaletteName(`Layer ${layer.id + 1} Palette`);
        setShowSaveModal(true);
    };

    const handleSavePalette = async () => {
        await runMutation(
            async () => {
                await savePalette(layer.id, paletteName);
                showToast('Palette saved.', 'success');
                setShowSaveModal(false);
            },
            'Failed to save palette',
        );
    };

    return (
        <div className="space-y-4 mt-4 p-4 bg-zinc-800 rounded-lg">
            {/* Predefined Palette Selector */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Load Palette:</label>
                <select
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                    onChange={(event) => runMutation(
                        () => loadPredefinedPalette(layer.id, event.target.value),
                        'Failed to load predefined palette',
                    )}
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
                                onChange={(event) => runMutation(
                                    () => updateLayerColor(layer.id, index, event.target.value),
                                    'Failed to update color',
                                )}
                                className="w-12 h-10 rounded border border-zinc-500"
                            />
                            <input
                                type="number"
                                min="0"
                                max="1"
                                step="0.01"
                                value={palette.positions[index] || 0}
                                onChange={(event) => runMutation(
                                    () => updateColorPosition(layer.id, index, parseFloat(event.target.value)),
                                    'Failed to update color position',
                                )}
                                className="flex-1 bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                            />
                            <button
                                onClick={() => runMutation(
                                    () => removeColorFromLayer(layer.id, index),
                                    'Failed to remove color',
                                )}
                                className="text-red-400 hover:text-red-300 p-2"
                            >
                                <Minus size={16} />
                            </button>
                        </div>
                    ))}
                </div>
                
                <button
                    onClick={() => runMutation(
                        () => addColorToLayer(layer.id),
                        'Failed to add color',
                    )}
                    className="w-full mt-2 bg-slate-600 hover:bg-slate-700 text-white px-4 py-2 rounded-lg flex items-center justify-center gap-2"
                >
                    <Plus size={16} />
                    Add Color
                </button>
                
                <button
                    onClick={openSaveModal}
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
                    onChange={(event) => runMutation(
                        () => updatePalette(layer.id, { colorRule: parseInt(event.target.value, 10) }),
                        'Failed to update color rule',
                    )}
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
                    onChange={(event) => runMutation(
                        () => updatePalette(layer.id, { interMode: parseInt(event.target.value, 10) }),
                        'Failed to update interpolation mode',
                    )}
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
                    onChange={(event) => runMutation(
                        () => updatePalette(layer.id, { segmentation: parseFloat(event.target.value) }),
                        'Failed to update segmentation',
                    )}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                />
            </div>
            
            {/* Wrap Mode */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Wrap Mode:</label>
                <select
                    value={palette.wrapMode}
                    onChange={(event) => runMutation(
                        () => updatePalette(layer.id, { wrapMode: parseInt(event.target.value, 10) }),
                        'Failed to update wrap mode',
                    )}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                >
                    {wrapModes.map(option => (
                        <option key={option.value} value={option.value}>
                            {option.label}
                        </option>
                    ))}
                </select>
            </div>

            {showSaveModal && (
                <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
                    <div className="bg-zinc-800 rounded-lg p-5 w-full max-w-md mx-4">
                        <h4 className="text-lg font-semibold mb-3">Save Palette</h4>
                        <input
                            type="text"
                            value={paletteName}
                            onChange={(event) => setPaletteName(event.target.value)}
                            className="w-full bg-zinc-700 border border-zinc-600 rounded px-3 py-2 mb-4"
                            placeholder="Palette name"
                        />
                        <div className="flex gap-2 justify-end">
                            <button
                                onClick={() => setShowSaveModal(false)}
                                className="px-4 py-2 rounded bg-zinc-700 hover:bg-zinc-600"
                            >
                                Cancel
                            </button>
                            <button
                                onClick={handleSavePalette}
                                className="px-4 py-2 rounded bg-green-600 hover:bg-green-700"
                            >
                                Save
                            </button>
                        </div>
                    </div>
                </div>
            )}
        </div>
    );
};

export default LayerPalette;
