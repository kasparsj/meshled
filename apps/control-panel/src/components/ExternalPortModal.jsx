import React, { useEffect, useMemo, useState } from 'react';
import { X, Link2 } from 'lucide-react';

const directionFromPort = (port) => Boolean(port?.direction);
const GROUP_OPTIONS = [1, 2, 4, 8, 16];

const ExternalPortModal = ({
    isOpen,
    mode = 'add',
    intersection,
    slotIndex,
    port,
    remoteDevices = [],
    loading = false,
    onClose,
    onSubmit,
}) => {
    const [selectedHost, setSelectedHost] = useState('');
    const [selectedTargetPortId, setSelectedTargetPortId] = useState('');
    const [group, setGroup] = useState('1');
    const [direction, setDirection] = useState(false);
    const [error, setError] = useState('');

    const selectedRemote = useMemo(
        () => remoteDevices.find((device) => device.host === selectedHost) || null,
        [remoteDevices, selectedHost],
    );

    useEffect(() => {
        if (!isOpen) return;

        const resolvedGroup = Number(port?.group ?? intersection?.group ?? 1);
        const initialGroup = GROUP_OPTIONS.includes(resolvedGroup) ? String(resolvedGroup) : '1';
        setGroup(initialGroup);
        setDirection(directionFromPort(port));

        const currentMac = String(port?.device || '').toUpperCase();
        const preferred =
            remoteDevices.find((device) => device.mac === currentMac) || remoteDevices[0] || null;

        setSelectedHost(preferred?.host || '');

        const initialTarget = port?.targetId ?? preferred?.ports?.[0]?.portId ?? '';
        setSelectedTargetPortId(String(initialTarget));
        setError('');
    }, [isOpen, port, intersection, remoteDevices]);

    if (!isOpen) return null;

    const handleSubmit = async (event) => {
        event.preventDefault();
        setError('');

        if (!intersection) {
            setError('Missing intersection context');
            return;
        }
        if (!selectedRemote) {
            setError('Select a remote device');
            return;
        }
        if (!selectedRemote.mac) {
            setError('Selected device is missing a MAC address in /device_info');
            return;
        }

        const targetPortIdNum = Number(selectedTargetPortId);
        const groupNum = Number(group);
        if (!Number.isFinite(targetPortIdNum) || targetPortIdNum < 0 || targetPortIdNum > 255) {
            setError('Select a valid target internal port');
            return;
        }
        if (!Number.isFinite(groupNum) || !GROUP_OPTIONS.includes(groupNum)) {
            setError('Group must be one of 1, 2, 4, 8, or 16');
            return;
        }

        const payload = mode === 'edit'
            ? {
                portId: port.id,
                group: groupNum,
                direction,
                deviceMac: selectedRemote.mac,
                targetPortId: targetPortIdNum,
            }
            : {
                intersectionId: intersection.id,
                slotIndex,
                group: groupNum,
                direction,
                deviceMac: selectedRemote.mac,
                targetPortId: targetPortIdNum,
            };

        try {
            await onSubmit(payload);
            onClose();
        } catch (submitError) {
            setError(submitError.message || 'Failed to save external port');
        }
    };

    const targets = selectedRemote?.ports || [];

    return (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <div className="bg-zinc-800 p-6 rounded-lg w-full max-w-lg mx-4">
                <div className="flex justify-between items-center mb-4">
                    <h3 className="text-xl font-semibold text-white flex items-center gap-2">
                        <Link2 size={18} />
                        {mode === 'edit' ? 'Edit External Port' : 'Add External Port'}
                    </h3>
                    <button
                        onClick={onClose}
                        className="text-zinc-400 hover:text-white"
                        disabled={loading}
                    >
                        <X size={20} />
                    </button>
                </div>

                <div className="bg-zinc-700 p-3 rounded mb-4 text-sm text-zinc-200">
                    <div>Intersection: <span className="text-white font-semibold">{intersection?.id}</span></div>
                    <div>Slot: <span className="text-white font-semibold">{slotIndex}</span></div>
                </div>

                <form onSubmit={handleSubmit} className="space-y-4">
                    <div>
                        <label className="block text-sm font-medium text-zinc-300 mb-2">Remote Device</label>
                        <select
                            value={selectedHost}
                            onChange={(e) => {
                                const host = e.target.value;
                                setSelectedHost(host);
                                const nextRemote = remoteDevices.find((device) => device.host === host);
                                setSelectedTargetPortId(String(nextRemote?.ports?.[0]?.portId ?? ''));
                            }}
                            className="w-full bg-zinc-700 border border-zinc-600 text-white rounded px-3 py-2"
                            disabled={loading}
                        >
                            <option value="">Select remote device...</option>
                            {remoteDevices.map((device) => (
                                <option key={device.host} value={device.host}>
                                    {device.label} ({device.host})
                                </option>
                            ))}
                        </select>
                        {selectedRemote?.mac && (
                            <p className="text-xs text-zinc-400 mt-1">Remote MAC: {selectedRemote.mac}</p>
                        )}
                    </div>

                    <div>
                        <label className="block text-sm font-medium text-zinc-300 mb-2">Remote Target Port</label>
                        <select
                            value={selectedTargetPortId}
                            onChange={(e) => setSelectedTargetPortId(e.target.value)}
                            className="w-full bg-zinc-700 border border-zinc-600 text-white rounded px-3 py-2"
                            disabled={loading || targets.length === 0}
                        >
                            {targets.length === 0 && <option value="">No internal ports discovered</option>}
                            {targets.map((target) => (
                                <option key={`${target.portId}-${target.intersectionId}-${target.slotIndex}`} value={target.portId}>
                                    {target.label}
                                </option>
                            ))}
                        </select>
                    </div>

                    <div className="grid grid-cols-2 gap-3">
                        <div>
                            <label className="block text-sm font-medium text-zinc-300 mb-2">Group Bit</label>
                            <select
                                value={group}
                                onChange={(e) => setGroup(e.target.value)}
                                className="w-full bg-zinc-700 border border-zinc-600 text-white rounded px-3 py-2"
                                disabled={loading}
                            >
                                {GROUP_OPTIONS.map((groupValue) => (
                                    <option key={groupValue} value={groupValue}>
                                        {groupValue}
                                    </option>
                                ))}
                            </select>
                        </div>
                        <div>
                            <label className="block text-sm font-medium text-zinc-300 mb-2">Direction</label>
                            <select
                                value={direction ? '1' : '0'}
                                onChange={(e) => setDirection(e.target.value === '1')}
                                className="w-full bg-zinc-700 border border-zinc-600 text-white rounded px-3 py-2"
                                disabled={loading}
                            >
                                <option value="0">Inbound (←)</option>
                                <option value="1">Outbound (→)</option>
                            </select>
                        </div>
                    </div>

                    {error && (
                        <div className="bg-red-900/50 border border-red-500 text-red-200 px-3 py-2 rounded">
                            {error}
                        </div>
                    )}

                    <div className="flex gap-2 pt-2">
                        <button
                            type="button"
                            onClick={onClose}
                            className="flex-1 bg-zinc-600 hover:bg-zinc-700 text-white py-2 px-4 rounded"
                            disabled={loading}
                        >
                            Cancel
                        </button>
                        <button
                            type="submit"
                            className="flex-1 bg-blue-600 hover:bg-blue-700 text-white py-2 px-4 rounded"
                            disabled={loading}
                        >
                            {loading ? 'Saving...' : mode === 'edit' ? 'Save Changes' : 'Create Link'}
                        </button>
                    </div>
                </form>
            </div>
        </div>
    );
};

export default ExternalPortModal;
