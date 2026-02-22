import React, { useState } from 'react';
import { Trash2 } from 'lucide-react';

const ModelInfo = ({ modelData, onRemoveIntersection }) => {
    const [loadingIntersections, setLoadingIntersections] = useState({});

    const handleRemoveIntersection = async (intersection) => {
        const key = `${intersection.id}-${intersection.group}`;
        setLoadingIntersections(prev => ({ ...prev, [key]: true }));

        try {
            if (onRemoveIntersection) {
                await onRemoveIntersection(intersection.id, intersection.group);
            }
        } catch (error) {
            console.error('Error removing intersection:', error);
        } finally {
            setLoadingIntersections(prev => ({ ...prev, [key]: false }));
        }
    };

    if (!modelData) {
        return null;
    }

    return (
        <div className="bg-zinc-700 p-4 rounded-lg">
            <h3 className="text-xl font-semibold mb-4">Model Information</h3>
            
            {/* Basic Info */}
            <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-6">
                <div className="bg-zinc-600 p-3 rounded">
                    <div className="text-sm text-zinc-400">Total Pixels</div>
                    <div className="text-lg font-bold text-white">{modelData.pixelCount}</div>
                </div>
                <div className="bg-zinc-600 p-3 rounded">
                    <div className="text-sm text-zinc-400">Real Pixels</div>
                    <div className="text-lg font-bold text-white">{modelData.realPixelCount}</div>
                </div>
                <div className="bg-zinc-600 p-3 rounded">
                    <div className="text-sm text-zinc-400">Models</div>
                    <div className="text-lg font-bold text-white">{modelData.modelCount}</div>
                </div>
                <div className="bg-zinc-600 p-3 rounded">
                    <div className="text-sm text-zinc-400">Gaps</div>
                    <div className="text-lg font-bold text-white">{modelData.gapCount}</div>
                </div>
            </div>

            {/* Intersections */}
            {modelData.intersections && modelData.intersections.length > 0 && (
                <div className="mb-6">
                    <h4 className="text-lg font-semibold mb-3 text-orange-400">Intersections ({modelData.intersections.length})</h4>
                    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-3 max-h-64 overflow-y-auto">
                        {modelData.intersections.map((intersection, index) => {
                            const isLoading = loadingIntersections[`${intersection.id}-${intersection.group}`];
                            return (
                                <div key={index} className="bg-zinc-600 p-3 rounded text-sm">
                                    <div className="flex justify-between items-center mb-1">
                                        <span className="font-semibold text-orange-300">ID: {intersection.id}</span>
                                        <div className="flex items-center gap-2">
                                            <span className="text-zinc-400">Group {intersection.group}</span>
                                            {onRemoveIntersection && (
                                                <button
                                                    onClick={() => handleRemoveIntersection(intersection)}
                                                    disabled={isLoading}
                                                    className="text-red-400 hover:text-red-300 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
                                                    title={`Remove intersection ${intersection.id}`}
                                                >
                                                    <Trash2 size={14} />
                                                </button>
                                            )}
                                        </div>
                                    </div>
                                    <div className="text-zinc-300">
                                        <div>Ports: {intersection.numPorts}</div>
                                        <div>Top: {intersection.topPixel}</div>
                                        <div>Bottom: {intersection.bottomPixel}</div>
                                        {intersection.ports && intersection.ports.length > 0 && (
                                            <div className="mt-2 border-t border-zinc-500 pt-2">
                                                <div className="text-xs text-zinc-400 mb-1">Port Details:</div>
                                                {intersection.ports.map((port, portIndex) => (
                                                    port ? (
                                                        <div key={portIndex} className="text-xs bg-zinc-700 p-1 rounded mb-1">
                                                            <div className="flex justify-between">
                                                                <span className="text-zinc-300">ID: {port.id}</span>
                                                                <span className={port.type === 'external' ? 'text-yellow-400' : 'text-cyan-400'}>
                                                                    {port.type}
                                                                </span>
                                                            </div>
                                                            <div className="flex justify-between">
                                                                <span>Dir: {port.direction ? '→' : '←'}</span>
                                                                <span>G: {port.group}</span>
                                                            </div>
                                                            {port.type === 'external' && (
                                                                <>
                                                                    <div className="text-yellow-300 text-[10px] mt-1">
                                                                        MAC: {port.device}
                                                                    </div>
                                                                    <div className="text-yellow-300 text-[10px]">
                                                                        Target: {port.targetId}
                                                                    </div>
                                                                </>
                                                            )}
                                                        </div>
                                                    ) : (
                                                        <div key={portIndex} className="text-xs text-zinc-500">
                                                            Port {portIndex}: null
                                                        </div>
                                                    )
                                                ))}
                                            </div>
                                        )}
                                    </div>
                                </div>
                            );
                        })}
                    </div>
                </div>
            )}

            {/* Connections */}
            {modelData.connections && modelData.connections.length > 0 && (
                <div className="mb-6">
                    <h4 className="text-lg font-semibold mb-3 text-blue-400">Connections ({modelData.connections.length})</h4>
                    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-3 max-h-64 overflow-y-auto">
                        {modelData.connections.map((connection, index) => (
                            <div key={index} className="bg-zinc-600 p-3 rounded text-sm">
                                <div className="flex justify-between items-center mb-1">
                                    <span className="font-semibold text-blue-300">Group {connection.group}</span>
                                    <span className="text-zinc-400">Dir: {connection.pixelDir ? <>&raquo;</> : <>&laquo;</>}</span>
                                </div>
                                <div className="text-zinc-300">
                                    <div>Range: {connection.fromPixel} → {connection.toPixel}</div>
                                    <div>LEDs: {connection.numLeds}</div>
                                </div>
                            </div>
                        ))}
                    </div>
                </div>
            )}

            {/* Models */}
            {modelData.models && modelData.models.length > 0 && (
                <div className="mb-6">
                    <h4 className="text-lg font-semibold mb-3 text-green-400">Models ({modelData.models.length})</h4>
                    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-3 max-h-48 overflow-y-auto">
                        {modelData.models.map((model, index) => (
                            model ? (
                                <div key={index} className="bg-zinc-600 p-3 rounded text-sm">
                                    <div className="flex justify-between items-center mb-1">
                                        <span className="font-semibold text-green-300">ID: {model.id}</span>
                                        <span className="text-zinc-400">W: {model.defaultW}</span>
                                    </div>
                                    <div className="text-zinc-300">
                                        <div>Emit Groups: {model.emitGroups}</div>
                                        <div>Max Length: {model.maxLength}</div>
                                    </div>
                                </div>
                            ) : (
                                <div key={index} className="bg-zinc-600 p-3 rounded text-sm text-zinc-500">
                                    Model {index}: null
                                </div>
                            )
                        ))}
                    </div>
                </div>
            )}

            {/* Gaps */}
            {modelData.gaps && modelData.gaps.length > 0 && (
                <div className="mb-6">
                    <h4 className="text-lg font-semibold mb-3 text-red-400">Pixel Gaps ({modelData.gaps.length})</h4>
                    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-3 max-h-32 overflow-y-auto">
                        {modelData.gaps.map((gap, index) => (
                            <div key={index} className="bg-zinc-600 p-2 rounded text-sm">
                                <div className="text-red-300 font-medium">
                                    {gap.fromPixel} → {gap.toPixel}
                                </div>
                                <div className="text-zinc-400 text-xs">
                                    Gap {index + 1}
                                </div>
                            </div>
                        ))}
                    </div>
                </div>
            )}
        </div>
    );
};

export default ModelInfo;