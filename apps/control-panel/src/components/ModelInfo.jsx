import React, { useMemo, useState } from 'react';
import { Trash2, Link2, Pencil } from 'lucide-react';
import ExternalPortModal from './ExternalPortModal.jsx';

const normalizeMac = (value) => String(value || '').replace(/[^a-fA-F0-9]/g, '').toUpperCase();

const ModelInfo = ({
    modelData,
    onRemoveIntersection,
    onAddExternalPort,
    onUpdateExternalPort,
    onRemoveExternalPort,
    remoteDevices = [],
    remoteLoading = false,
    remoteError = '',
}) => {
    const [loadingIntersections, setLoadingIntersections] = useState({});
    const [loadingExternalPorts, setLoadingExternalPorts] = useState({});
    const [modalState, setModalState] = useState({
        isOpen: false,
        mode: 'add',
        intersection: null,
        slotIndex: 0,
        port: null,
    });

    const crossDevice = modelData?.capabilities?.crossDevice || { enabled: false, ready: false, transport: 'none' };
    const hasSchemaV2 = (modelData?.schemaVersion || 1) >= 2;

    const remoteLookup = useMemo(() => {
        const map = new Map();
        for (const remoteDevice of remoteDevices) {
            const macKey = normalizeMac(remoteDevice.mac);
            if (!macKey) continue;
            map.set(macKey, remoteDevice);
        }
        return map;
    }, [remoteDevices]);

    const getRemoteTargetLabel = (port) => {
        const macKey = normalizeMac(port?.device || port?.deviceMac);
        const remote = remoteLookup.get(macKey);
        if (!remote) {
            return 'Remote device unavailable';
        }
        const target = remote.ports.find((candidate) => candidate.portId === port.targetId);
        if (!target) {
            return `${remote.label} (${remote.host}) · target missing`;
        }
        return `${remote.label} (${remote.host}) · ${target.label}`;
    };

    const handleRemoveIntersection = async (intersection) => {
        const key = `${intersection.id}-${intersection.group}`;
        setLoadingIntersections((prev) => ({ ...prev, [key]: true }));

        try {
            if (onRemoveIntersection) {
                await onRemoveIntersection(intersection.id, intersection.group);
            }
        } catch (error) {
            console.error('Error removing intersection:', error);
        } finally {
            setLoadingIntersections((prev) => ({ ...prev, [key]: false }));
        }
    };

    const handleRemoveExternalPort = async (portId) => {
        setLoadingExternalPorts((prev) => ({ ...prev, [portId]: true }));
        try {
            await onRemoveExternalPort?.(portId);
        } catch (error) {
            console.error('Error removing external port:', error);
        } finally {
            setLoadingExternalPorts((prev) => ({ ...prev, [portId]: false }));
        }
    };

    const openAddExternalModal = (intersection, slotIndex) => {
        setModalState({
            isOpen: true,
            mode: 'add',
            intersection,
            slotIndex,
            port: null,
        });
    };

    const openEditExternalModal = (intersection, slotIndex, port) => {
        setModalState({
            isOpen: true,
            mode: 'edit',
            intersection,
            slotIndex,
            port,
        });
    };

    const closeExternalModal = () => {
        setModalState((prev) => ({ ...prev, isOpen: false }));
    };

    const handleExternalSubmit = async (payload) => {
        if (modalState.mode === 'edit') {
            await onUpdateExternalPort?.(payload);
            return;
        }
        await onAddExternalPort?.(payload);
    };

    if (!modelData) {
        return null;
    }

    return (
        <div className="bg-zinc-700 p-4 rounded-lg">
            <h3 className="text-xl font-semibold mb-4">Model Information</h3>

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
                    <div className="text-sm text-zinc-400">Schema</div>
                    <div className="text-lg font-bold text-white">v{modelData.schemaVersion || 1}</div>
                </div>
                <div className="bg-zinc-600 p-3 rounded">
                    <div className="text-sm text-zinc-400">Cross-device</div>
                    <div className="text-sm font-bold text-white">
                        {crossDevice.enabled ? `${crossDevice.transport} (${crossDevice.ready ? 'ready' : 'not ready'})` : 'disabled'}
                    </div>
                </div>
            </div>

            {!hasSchemaV2 && (
                <div className="mb-4 bg-amber-900/40 border border-amber-500 text-amber-100 px-3 py-2 rounded">
                    Firmware schemaVersion is below v2. External port editing requires schemaVersion 2.
                </div>
            )}

            {remoteError && (
                <div className="mb-4 bg-amber-900/40 border border-amber-500 text-amber-100 px-3 py-2 rounded">
                    {remoteError}
                </div>
            )}

            {modelData.intersections && modelData.intersections.length > 0 && (
                <div className="mb-6">
                    <h4 className="text-lg font-semibold mb-3 text-orange-400">Intersections ({modelData.intersections.length})</h4>
                    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-3 max-h-96 overflow-y-auto">
                        {modelData.intersections.map((intersection) => {
                            const isLoading = loadingIntersections[`${intersection.id}-${intersection.group}`];
                            return (
                                <div key={`${intersection.id}-${intersection.group}`} className="bg-zinc-600 p-3 rounded text-sm">
                                    <div className="flex justify-between items-center mb-1">
                                        <span className="font-semibold text-orange-300">ID: {intersection.id}</span>
                                        <div className="flex items-center gap-2">
                                            <span className="text-zinc-400">Group {intersection.group}</span>
                                            {onRemoveIntersection && (
                                                <button
                                                    onClick={() => handleRemoveIntersection(intersection)}
                                                    disabled={isLoading}
                                                    className="text-red-400 hover:text-red-300 disabled:opacity-50 disabled:cursor-not-allowed"
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

                                        <div className="mt-2 border-t border-zinc-500 pt-2">
                                            <div className="text-xs text-zinc-400 mb-1">Port Details:</div>
                                            {Array.from({ length: intersection.numPorts }, (_, slotIndex) => {
                                                const port = intersection.ports?.[slotIndex] ?? null;
                                                if (!port) {
                                                    return (
                                                        <div key={`slot-${slotIndex}`} className="text-xs bg-zinc-700 p-1 rounded mb-1 flex items-center justify-between">
                                                            <span className="text-zinc-400">Slot {slotIndex}: empty</span>
                                                            {hasSchemaV2 && crossDevice.enabled && onAddExternalPort && (
                                                                <button
                                                                    onClick={() => openAddExternalModal(intersection, slotIndex)}
                                                                    className="text-sky-300 hover:text-sky-200"
                                                                    title="Add external link"
                                                                >
                                                                    <Link2 size={13} />
                                                                </button>
                                                            )}
                                                        </div>
                                                    );
                                                }

                                                const isExternal = port.type === 'external';
                                                return (
                                                    <div key={`port-${port.id}`} className="text-xs bg-zinc-700 p-1 rounded mb-1">
                                                        <div className="flex justify-between">
                                                            <span className="text-zinc-300">Slot {slotIndex} · ID: {port.id}</span>
                                                            <span className={isExternal ? 'text-yellow-400' : 'text-cyan-400'}>{port.type}</span>
                                                        </div>
                                                        <div className="flex justify-between">
                                                            <span>Dir: {port.direction ? '→' : '←'}</span>
                                                            <span>G: {port.group}</span>
                                                        </div>
                                                        {isExternal && (
                                                            <>
                                                                <div className="text-yellow-300 text-[10px] mt-1">MAC: {port.device}</div>
                                                                <div className="text-yellow-300 text-[10px]">Target: {port.targetId}</div>
                                                                <div className="text-yellow-300 text-[10px]">{getRemoteTargetLabel(port)}</div>
                                                                <div className="mt-1 flex gap-2 justify-end">
                                                                    <button
                                                                        onClick={() => openEditExternalModal(intersection, slotIndex, port)}
                                                                        className="text-sky-300 hover:text-sky-200"
                                                                        title="Edit external link"
                                                                        disabled={!hasSchemaV2 || !crossDevice.enabled}
                                                                    >
                                                                        <Pencil size={12} />
                                                                    </button>
                                                                    <button
                                                                        onClick={() => handleRemoveExternalPort(port.id)}
                                                                        className="text-red-400 hover:text-red-300"
                                                                        title="Remove external link"
                                                                        disabled={loadingExternalPorts[port.id]}
                                                                    >
                                                                        <Trash2 size={12} />
                                                                    </button>
                                                                </div>
                                                            </>
                                                        )}
                                                    </div>
                                                );
                                            })}
                                        </div>
                                    </div>
                                </div>
                            );
                        })}
                    </div>
                </div>
            )}

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

            <ExternalPortModal
                isOpen={modalState.isOpen}
                mode={modalState.mode}
                intersection={modalState.intersection}
                slotIndex={modalState.slotIndex}
                port={modalState.port}
                remoteDevices={remoteDevices}
                loading={remoteLoading}
                onClose={closeExternalModal}
                onSubmit={handleExternalSubmit}
            />
        </div>
    );
};

export default ModelInfo;
