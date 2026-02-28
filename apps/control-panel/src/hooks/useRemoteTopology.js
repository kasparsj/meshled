import { useEffect, useMemo, useState } from 'react';
import { useDevice } from '../contexts/DeviceContext.jsx';
import { parseModelDataResponse } from '../models/apiModels';
import { fetchDeviceJsonWithTimeout, sanitizeHost } from '../utils/deviceRequest';

const normalizeMac = (value) => {
    const raw = String(value || '').replace(/[^a-fA-F0-9]/g, '').toUpperCase();
    if (raw.length !== 12) return null;
    return raw.match(/.{1,2}/g).join(':');
};

const extractMac = (info) => normalizeMac(info?.mac || info?.wifi?.bssid || info?.wifi?.mac);

const collectInternalPorts = (modelData) => {
    const ports = [];
    for (const intersection of modelData?.intersections || []) {
        (intersection.ports || []).forEach((port, slotIndex) => {
            if (!port || port.type !== 'internal') return;
            ports.push({
                portId: port.id,
                intersectionId: intersection.id,
                slotIndex,
                group: port.group,
                label: `Port ${port.id} · I${intersection.id} · Slot ${slotIndex}`,
            });
        });
    }
    return ports;
};

export const useRemoteTopology = (devices = []) => {
    const { selectedDevice, apiToken } = useDevice();
    const [remoteDevices, setRemoteDevices] = useState([]);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState('');

    const candidateDevices = useMemo(() => {
        const deduped = Array.from(new Set((devices || []).map(sanitizeHost).filter(Boolean)));
        return deduped.filter((host) => host !== selectedDevice);
    }, [devices, selectedDevice]);

    useEffect(() => {
        let cancelled = false;

        const load = async () => {
            if (candidateDevices.length === 0) {
                setRemoteDevices([]);
                setError('');
                return;
            }

            setLoading(true);
            setError('');

            const loaded = await Promise.all(candidateDevices.map(async (host) => {
                try {
                    const [deviceInfoRaw, modelRaw] = await Promise.all([
                        fetchDeviceJsonWithTimeout(host, '/device_info', { timeoutMs: 1600, apiToken }),
                        fetchDeviceJsonWithTimeout(host, '/get_model', { timeoutMs: 2200, apiToken }),
                    ]);
                    const modelData = parseModelDataResponse(modelRaw);
                    const ports = collectInternalPorts(modelData);
                    if (ports.length === 0) {
                        return null;
                    }

                    const mac = extractMac(deviceInfoRaw);
                    const hostLabel = String(deviceInfoRaw?.name || deviceInfoRaw?.ip || host);
                    return {
                        host,
                        label: hostLabel,
                        mac,
                        ports,
                    };
                } catch {
                    return null;
                }
            }));

            if (cancelled) return;
            const filtered = loaded.filter(Boolean);
            setRemoteDevices(filtered);
            if (filtered.length === 0) {
                setError('No remote devices with topology data available.');
            }
            setLoading(false);
        };

        load().catch((err) => {
            if (cancelled) return;
            setError(err.message || 'Failed to load remote topology');
            setLoading(false);
        });

        return () => {
            cancelled = true;
        };
    }, [candidateDevices, apiToken]);

    return {
        remoteDevices,
        loading,
        error,
    };
};

export default useRemoteTopology;
