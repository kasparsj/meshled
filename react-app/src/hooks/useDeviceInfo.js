// hooks/useDeviceInfo.js
import { useEffect, useState } from "react";
import { useDevice } from "../contexts/DeviceContext";

const useDeviceInfo = () => {
    const [deviceInfo, setDeviceInfo] = useState(null);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    useEffect(() => {
        if (!selectedDevice) {
            setDeviceInfo(null);
            setLoading(false);
            setError('No device selected');
            return;
        }

        const fetchDeviceInfo = async () => {
            try {
                setLoading(true);
                setError(null);
                const res = await deviceFetch('/device_info');
                if (!res.ok) throw new Error(`HTTP ${res.status}`);
                const data = await res.json();
                setDeviceInfo(data);
            } catch (err) {
                setError('Failed to load device info.');
                console.error(err);
            } finally {
                setLoading(false);
            }
        };

        fetchDeviceInfo();
    }, [selectedDevice, deviceFetch]);

    return { deviceInfo, loading, error };
};

export default useDeviceInfo;
