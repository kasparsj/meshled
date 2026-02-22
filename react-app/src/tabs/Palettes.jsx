import {Palette, Edit3} from "lucide-react";
import React, { useState, useEffect } from "react";
import usePalettes from "../hooks/usePalettes";

const PalettesTab = () => {
    const { palettes, loading, error, deletePalette } = usePalettes();
    const [editingPalette, setEditingPalette] = useState(null);

    // Handle ESC key to close modal
    useEffect(() => {
        const handleKeyDown = (event) => {
            if (event.key === 'Escape' && editingPalette) {
                setEditingPalette(null);
            }
        };

        if (editingPalette) {
            document.addEventListener('keydown', handleKeyDown);
            return () => document.removeEventListener('keydown', handleKeyDown);
        }
    }, [editingPalette]);

    const handleDeletePalette = async (paletteIndex) => {
        if (window.confirm('Are you sure you want to delete this palette?')) {
            await deletePalette(paletteIndex);
        }
    };

    const handleEditPalette = (palette, index) => {
        setEditingPalette({ ...palette, index });
    };

    const handleCloseEdit = () => {
        setEditingPalette(null);
    };

    const handleNewPalette = () => {
        const newPalette = {
            name: `Palette ${palettes.length + 1}`,
            colors: ['#FF0000'],
            positions: [0],
            colorRule: -1,
            interMode: 1,
            wrapMode: 0,
            segmentation: 0,
            index: -1 // -1 indicates this is a new palette
        };
        setEditingPalette(newPalette);
    };

    return (
        <div className="space-y-6">
            <div className="flex justify-between items-center">
                <h2 className="text-2xl font-bold">Color Palettes</h2>
                <button 
                    onClick={handleNewPalette}
                    className="bg-purple-600 hover:bg-purple-700 px-4 py-2 rounded-lg flex items-center gap-2"
                >
                    <Palette size={18}/>
                    New Palette
                </button>
            </div>

            {loading && <div className="text-zinc-400 animate-pulse">Loading palettes...</div>}
            {error && <div className="text-red-500">{error}</div>}

            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                {palettes.map((palette, index) => (
                    <div key={index} className="bg-zinc-700 p-4 rounded-lg">
                        <h3 className="text-lg font-semibold mb-3">{palette.name}</h3>
                        <div className="flex flex-wrap gap-2 mb-3">
                            {palette.colors && palette.colors.map((color, colorIndex) => (
                                <div
                                    key={colorIndex}
                                    className="w-8 h-8 rounded border-2 border-zinc-500"
                                    style={{backgroundColor: color}}
                                    title={color}
                                />
                            ))}
                        </div>
                        {palette.colorRule !== undefined && (
                            <div className="text-sm text-zinc-400 mb-2">
                                Rule: {palette.colorRule === -1 ? 'None' : palette.colorRule}
                            </div>
                        )}
                        <div className="flex gap-2">
                            <button 
                                onClick={() => handleEditPalette(palette, index)}
                                className="bg-sky-600 hover:bg-sky-700 px-3 py-1 rounded text-sm flex items-center gap-1"
                            >
                                <Edit3 size={14} />
                                Edit
                            </button>
                            <button 
                                onClick={() => handleDeletePalette(index)}
                                className="bg-red-600 hover:bg-red-700 px-3 py-1 rounded text-sm"
                            >
                                Delete
                            </button>
                        </div>
                    </div>
                ))}
            </div>

            {/* Edit Palette Modal */}
            {editingPalette && (
                <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
                    <div className="bg-zinc-800 p-6 rounded-lg max-w-2xl w-full mx-4 max-h-[90vh] overflow-y-auto">
                        <div className="flex justify-between items-center mb-4">
                            <h3 className="text-xl font-bold">
                                {editingPalette.index === -1 ? 'Create New Palette' : 'Edit Palette'}
                            </h3>
                            <button 
                                onClick={handleCloseEdit}
                                className="text-zinc-400 hover:text-white"
                            >
                                ✕
                            </button>
                        </div>
                        
                        <PaletteEditor palette={editingPalette} onClose={handleCloseEdit} />
                    </div>
                </div>
            )}
        </div>
    );
};

// Palette Editor Component
const PaletteEditor = ({ palette, onClose }) => {
    const { savePalette } = usePalettes();
    const [name, setName] = useState(palette.name || 'New Palette');
    const [colors, setColors] = useState(palette.colors || ['#FF0000']);
    const [positions, setPositions] = useState(palette.positions || [0]);
    const [colorRule, setColorRule] = useState(palette.colorRule || -1);
    const [interMode, setInterMode] = useState(palette.interMode || 1);
    const [wrapMode, setWrapMode] = useState(palette.wrapMode || 0);
    const [segmentation, setSegmentation] = useState(palette.segmentation || 0);

    const addColor = () => {
        const newColors = [...colors, '#FF0000'];
        const newPositions = [...positions];
        
        // Calculate position for new color
        const newPosition = newColors.length === 1 ? 0 : 1;
        newPositions.push(newPosition);
        
        // Redistribute positions evenly
        if (newColors.length > 1) {
            for (let i = 0; i < newColors.length; i++) {
                newPositions[i] = i / (newColors.length - 1);
            }
        }
        
        setColors(newColors);
        setPositions(newPositions);
    };

    const removeColor = (index) => {
        if (colors.length <= 1) return; // Don't allow removing the last color
        
        const newColors = colors.filter((_, i) => i !== index);
        const newPositions = positions.filter((_, i) => i !== index);
        
        // Redistribute positions evenly
        if (newColors.length > 1) {
            for (let i = 0; i < newColors.length; i++) {
                newPositions[i] = i / (newColors.length - 1);
            }
        } else {
            newPositions[0] = 0;
        }
        
        setColors(newColors);
        setPositions(newPositions);
    };

    const updateColor = (index, newColor) => {
        const newColors = [...colors];
        newColors[index] = newColor;
        setColors(newColors);
    };

    const updatePosition = (index, newPosition) => {
        const clampedPosition = Math.max(0, Math.min(1, parseFloat(newPosition) || 0));
        const newPositions = [...positions];
        newPositions[index] = clampedPosition;
        setPositions(newPositions);
    };

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

    const handleSave = async () => {
        const paletteData = {
            name,
            colors,
            positions,
            colorRule,
            interMode,
            wrapMode,
            segmentation
        };
        
        const result = await savePalette(paletteData);
        if (result) {
            onClose();
        }
    };

    return (
        <div className="space-y-4">
            {/* Palette Name */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Palette Name:</label>
                <input
                    type="text"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                    placeholder="Enter palette name"
                />
            </div>

            {/* Colors */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">
                    Colors: {colors.length}
                </label>
                <div className="space-y-2">
                    {colors.map((color, index) => (
                        <div key={index} className="flex items-center gap-2">
                            <input
                                type="color"
                                value={color}
                                onChange={(e) => updateColor(index, e.target.value)}
                                className="w-12 h-10 rounded border border-zinc-500"
                            />
                            <input
                                type="number"
                                min="0"
                                max="1"
                                step="0.01"
                                value={positions[index] || 0}
                                onChange={(e) => updatePosition(index, e.target.value)}
                                className="flex-1 bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                                placeholder="Position"
                            />
                            <button
                                onClick={() => removeColor(index)}
                                disabled={colors.length <= 1}
                                className="text-red-400 hover:text-red-300 disabled:text-zinc-500 p-2"
                            >
                                ✕
                            </button>
                        </div>
                    ))}
                </div>
                
                <button
                    onClick={addColor}
                    className="w-full mt-2 bg-slate-600 hover:bg-slate-700 text-white px-4 py-2 rounded-lg"
                >
                    + Add Color
                </button>
            </div>

            {/* Color Rule */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Color Rule:</label>
                <select
                    value={colorRule}
                    onChange={(e) => setColorRule(parseInt(e.target.value))}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                >
                    {colorRuleOptions.map(option => (
                        <option key={option.value} value={option.value}>
                            {option.label}
                        </option>
                    ))}
                </select>
            </div>

            {/* Interpolation Mode */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Color Interpolation:</label>
                <select
                    value={interMode}
                    onChange={(e) => setInterMode(parseInt(e.target.value))}
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
                    value={segmentation}
                    onChange={(e) => setSegmentation(parseFloat(e.target.value))}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                />
            </div>

            {/* Wrap Mode */}
            <div>
                <label className="block text-sm text-zinc-300 mb-2">Wrap Mode:</label>
                <select
                    value={wrapMode}
                    onChange={(e) => setWrapMode(parseInt(e.target.value))}
                    className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2"
                >
                    {wrapModes.map(option => (
                        <option key={option.value} value={option.value}>
                            {option.label}
                        </option>
                    ))}
                </select>
            </div>

            {/* Action Buttons */}
            <div className="flex gap-2 pt-4">
                <button
                    onClick={onClose}
                    className="flex-1 bg-zinc-600 hover:bg-zinc-700 text-white px-4 py-2 rounded-lg"
                >
                    Cancel
                </button>
                <button
                    onClick={handleSave}
                    className="flex-1 bg-green-600 hover:bg-green-700 text-white px-4 py-2 rounded-lg"
                >
                    {palette.index === -1 ? 'Create Palette' : 'Save Changes'}
                </button>
            </div>
        </div>
    );
};

export default PalettesTab;
