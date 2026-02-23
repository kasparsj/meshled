import {useCallback, useEffect, useState} from "react";
import {useDevice} from "../contexts/DeviceContext.jsx";

const usePalettes = () => {
    const [palettes, setPalettes] = useState([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    // Fetch palettes from the server
    const fetchPalettes = useCallback(async (verbose = true, paletteName = null) => {
        try {
            setLoading(true);
            setError(null);
            
            let url = '/get_palettes';
            const params = new URLSearchParams();
            
            if (verbose) {
                params.append('v', 'true');
            }
            
            if (paletteName) {
                params.append('name', paletteName);
            }
            
            if (params.toString()) {
                url += '?' + params.toString();
            }
            
            const response = await deviceFetch(url);
            
            if (response.ok) {
                const data = await response.json();
                setPalettes(data);
                return data;
            } else {
                const errorText = await response.text();
                console.error("Failed to fetch palettes:", errorText);
                setError("Failed to fetch palettes: " + errorText);
                return null;
            }
        } catch (err) {
            console.error("Error fetching palettes:", err);
            setError("Error fetching palettes");
            return null;
        } finally {
            setLoading(false);
        }
    }, [deviceFetch]);

    // Delete a palette by index
    const deletePalette = useCallback(async (paletteIndex) => {
        try {
            const formData = new FormData();
            formData.append('index', paletteIndex.toString());
            const response = await deviceFetch('/delete_palette', {
                method: 'POST',
                body: formData,
            });
            
            if (response.ok) {
                const result = await response.json();
                console.log("Palette deleted successfully:", result.message);
                
                // Refresh palettes list after deletion
                await fetchPalettes();
                
                return result;
            } else {
                const error = await response.json();
                console.error("Failed to delete palette:", error.error);
                setError("Failed to delete palette: " + error.error);
                return null;
            }
        } catch (err) {
            console.error("Error deleting palette:", err);
            setError("Error deleting palette");
            return null;
        }
    }, [deviceFetch, fetchPalettes]);

    // Sync palettes with server (push/pull)
    const syncPalettes = useCallback(async (palettesData, push = false, pull = false) => {
        try {
            const params = new URLSearchParams();
            if (push) params.append('push', 'true');
            if (pull) params.append('pull', 'true');
            
            const response = await deviceFetch(`/sync_palettes?${params.toString()}`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(palettesData)
            });
            
            if (response.ok) {
                const result = await response.json();
                console.log("Palettes synced successfully");
                
                // If pulling, update local palettes with the result
                if (pull && result.length > 0) {
                    setPalettes(prev => [...prev, ...result]);
                }
                
                return result;
            } else {
                const error = await response.json();
                console.error("Failed to sync palettes:", error.error);
                setError("Failed to sync palettes: " + error.error);
                return null;
            }
        } catch (err) {
            console.error("Error syncing palettes:", err);
            setError("Error syncing palettes");
            return null;
        }
    }, [deviceFetch]);

    // Save/Create a palette
    const savePalette = useCallback(async (paletteData) => {
        try {
            const response = await deviceFetch('/save_palette', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(paletteData)
            });
            
            if (response.ok) {
                const result = await response.json();
                console.log("Palette saved successfully:", result.message);
                
                // Refresh palettes list after saving
                await fetchPalettes();
                
                return result;
            } else {
                const error = await response.json();
                console.error("Failed to save palette:", error.error);
                setError("Failed to save palette: " + error.error);
                return null;
            }
        } catch (err) {
            console.error("Error saving palette:", err);
            setError("Error saving palette");
            return null;
        }
    }, [deviceFetch, fetchPalettes]);

    // Auto-fetch palettes when device changes
    useEffect(() => {
        if (!selectedDevice) {
            setPalettes([]);
            setLoading(false);
            setError('No device selected');
            return;
        }

        fetchPalettes();
    }, [selectedDevice, fetchPalettes]);

    return {
        palettes,
        loading,
        error,
        fetchPalettes,
        savePalette,
        deletePalette,
        syncPalettes,
    };
};

export default usePalettes;
