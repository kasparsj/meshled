import React from 'react';
import { X, Trash2, AlertTriangle } from 'lucide-react';
import { useIntersectionContext } from '../contexts/IntersectionContext';

const RemoveIntersectionModal = ({ onSuccess }) => {
    const { 
        showRemoveModal, 
        selectedIntersection, 
        closeRemoveModal, 
        removeIntersection, 
        isLoading 
    } = useIntersectionContext();

    const handleConfirm = async () => {
        if (!selectedIntersection) return;

        try {
            await removeIntersection(selectedIntersection.id, selectedIntersection.group);
            closeRemoveModal();
            
            if (onSuccess) {
                onSuccess();
            }
        } catch (error) {
            console.error('Failed to remove intersection:', error);
        }
    };

    if (!showRemoveModal || !selectedIntersection) return null;

    return (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <div className="bg-zinc-800 p-6 rounded-lg w-full max-w-md mx-4">
                <div className="flex justify-between items-center mb-4">
                    <h3 className="text-xl font-semibold text-white flex items-center gap-2">
                        <Trash2 size={20} className="text-red-400" />
                        Remove Intersection
                    </h3>
                    <button
                        onClick={closeRemoveModal}
                        className="text-zinc-400 hover:text-white"
                        disabled={isLoading}
                    >
                        <X size={20} />
                    </button>
                </div>

                <div className="mb-4">
                    <div className="bg-red-900/30 border border-red-500/50 p-4 rounded-lg mb-4">
                        <div className="flex items-center gap-2 text-red-300 mb-2">
                            <AlertTriangle size={18} />
                            <span className="font-medium">Warning</span>
                        </div>
                        <p className="text-red-200 text-sm">
                            This will permanently remove the intersection and clean up all related model weights and connections.
                        </p>
                    </div>

                    <div className="bg-zinc-700 p-4 rounded-lg">
                        <h4 className="text-white font-medium mb-2">Intersection Details</h4>
                        <div className="space-y-1 text-sm">
                            <div className="flex justify-between">
                                <span className="text-zinc-400">ID:</span>
                                <span className="text-orange-300 font-semibold">{selectedIntersection.id}</span>
                            </div>
                            <div className="flex justify-between">
                                <span className="text-zinc-400">Group:</span>
                                <span className="text-white">{selectedIntersection.group}</span>
                            </div>
                            <div className="flex justify-between">
                                <span className="text-zinc-400">Ports:</span>
                                <span className="text-white">{selectedIntersection.numPorts}</span>
                            </div>
                            <div className="flex justify-between">
                                <span className="text-zinc-400">Top Pixel:</span>
                                <span className="text-white">{selectedIntersection.topPixel}</span>
                            </div>
                            <div className="flex justify-between">
                                <span className="text-zinc-400">Bottom Pixel:</span>
                                <span className="text-white">{selectedIntersection.bottomPixel}</span>
                            </div>
                        </div>
                    </div>
                </div>

                <div className="flex gap-2">
                    <button
                        type="button"
                        onClick={closeRemoveModal}
                        className="flex-1 bg-zinc-600 hover:bg-zinc-700 text-white py-2 px-4 rounded"
                        disabled={isLoading}
                    >
                        Cancel
                    </button>
                    <button
                        onClick={handleConfirm}
                        className="flex-1 bg-red-600 hover:bg-red-700 text-white py-2 px-4 rounded flex items-center justify-center gap-2"
                        disabled={isLoading}
                    >
                        <Trash2 size={16} />
                        {isLoading ? 'Removing...' : 'Remove Intersection'}
                    </button>
                </div>
            </div>
        </div>
    );
};

export default RemoveIntersectionModal;