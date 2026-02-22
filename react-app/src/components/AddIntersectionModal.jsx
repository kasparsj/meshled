import React, { useState } from 'react';
import { X, Plus } from 'lucide-react';
import { useIntersectionContext } from '../contexts/IntersectionContext';

const AddIntersectionModal = ({ onSuccess }) => {
    const { showAddModal, selectedPixel, closeAddModal, addIntersection, isLoading } = useIntersectionContext();
    const [numPorts, setNumPorts] = useState(4);
    const [group, setGroup] = useState(1);
    const [bottomPixel, setBottomPixel] = useState('');
    const [error, setError] = useState('');

    const handleSubmit = async (e) => {
        e.preventDefault();
        setError('');

        if (selectedPixel === null) {
            setError('No pixel selected');
            return;
        }

        try {
            const intersectionData = {
                numPorts: parseInt(numPorts),
                topPixel: selectedPixel,
                group: parseInt(group),
            };

            // Add bottomPixel if provided
            if (bottomPixel && bottomPixel !== '') {
                const bottomVal = parseInt(bottomPixel);
                if (bottomVal !== selectedPixel) {
                    intersectionData.bottomPixel = bottomVal;
                }
            }

            await addIntersection(intersectionData);
            
            // Reset form
            setNumPorts(4);
            setGroup(1);
            setBottomPixel('');
            
            closeAddModal();
            
            if (onSuccess) {
                onSuccess();
            }
        } catch (err) {
            setError(err.message);
        }
    };

    if (!showAddModal) return null;

    return (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <div className="bg-zinc-800 p-6 rounded-lg w-full max-w-md mx-4">
                <div className="flex justify-between items-center mb-4">
                    <h3 className="text-xl font-semibold text-white">Add Intersection</h3>
                    <button
                        onClick={closeAddModal}
                        className="text-zinc-400 hover:text-white"
                        disabled={isLoading}
                    >
                        <X size={20} />
                    </button>
                </div>

                {selectedPixel !== null && (
                    <div className="bg-zinc-700 p-3 rounded mb-4">
                        <span className="text-sm text-zinc-400">Selected Pixel:</span>
                        <span className="text-white font-semibold ml-2">{selectedPixel}</span>
                    </div>
                )}

                <form onSubmit={handleSubmit} className="space-y-4">
                    <div>
                        <label className="block text-sm font-medium text-zinc-300 mb-2">
                            Number of Ports
                        </label>
                        <select
                            value={numPorts}
                            onChange={(e) => setNumPorts(e.target.value)}
                            className="w-full bg-zinc-700 border border-zinc-600 text-white rounded px-3 py-2 focus:outline-none focus:border-blue-500"
                            disabled={isLoading}
                        >
                            <option value={2}>2 Ports</option>
                            <option value={4}>4 Ports</option>
                        </select>
                    </div>

                    <div>
                        <label className="block text-sm font-medium text-zinc-300 mb-2">
                            Group
                        </label>
                        <input
                            type="number"
                            min="1"
                            max="31"
                            value={group}
                            onChange={(e) => setGroup(e.target.value)}
                            className="w-full bg-zinc-700 border border-zinc-600 text-white rounded px-3 py-2 focus:outline-none focus:border-blue-500"
                            disabled={isLoading}
                        />
                    </div>

                    <div>
                        <label className="block text-sm font-medium text-zinc-300 mb-2">
                            Bottom Pixel (optional)
                        </label>
                        <input
                            type="number"
                            min="0"
                            value={bottomPixel}
                            onChange={(e) => setBottomPixel(e.target.value)}
                            placeholder="Leave empty for single pixel intersection"
                            className="w-full bg-zinc-700 border border-zinc-600 text-white rounded px-3 py-2 focus:outline-none focus:border-blue-500"
                            disabled={isLoading}
                        />
                        <p className="text-xs text-zinc-400 mt-1">
                            If set, creates a range intersection from topPixel to bottomPixel
                        </p>
                    </div>

                    {error && (
                        <div className="bg-red-900/50 border border-red-500 text-red-200 px-3 py-2 rounded">
                            {error}
                        </div>
                    )}

                    <div className="flex gap-2 pt-2">
                        <button
                            type="button"
                            onClick={closeAddModal}
                            className="flex-1 bg-zinc-600 hover:bg-zinc-700 text-white py-2 px-4 rounded"
                            disabled={isLoading}
                        >
                            Cancel
                        </button>
                        <button
                            type="submit"
                            className="flex-1 bg-blue-600 hover:bg-blue-700 text-white py-2 px-4 rounded flex items-center justify-center gap-2"
                            disabled={isLoading}
                        >
                            <Plus size={16} />
                            {isLoading ? 'Adding...' : 'Add Intersection'}
                        </button>
                    </div>
                </form>
            </div>
        </div>
    );
};

export default AddIntersectionModal;